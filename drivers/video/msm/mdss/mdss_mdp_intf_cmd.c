/* Copyright (c) 2013-2014, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/bootmem.h>
#include <linux/memblock.h>

#include "mdss_mdp.h"
#include "mdss_panel.h"
#include "mdss_debug.h"

#define VSYNC_EXPIRE_TICK 6

#define MAX_SESSIONS 2

/* wait for at most 2 vsync for lowest refresh rate (24hz) */
#define KOFF_TIMEOUT msecs_to_jiffies(200)

#define STOP_TIMEOUT msecs_to_jiffies(300)
#define POWER_COLLAPSE_TIME msecs_to_jiffies(100)

struct mdss_mdp_cmd_ctx {
	struct mdss_mdp_ctl *ctl;
#if defined (CONFIG_FB_MSM_MDSS_DSI_DBG)
	u32 panel_ndx;
#endif
	u32 pp_num;
	u8 ref_cnt;
	struct completion pp_comp;
	struct completion stop_comp;
	struct list_head vsync_handlers;
	int panel_on;
	atomic_t koff_cnt;
	int clk_enabled;
	int vsync_enabled;
	int rdptr_enabled;
	int do_notifier;
	struct mutex clk_mtx;
	spinlock_t clk_lock;
#if defined(CONFIG_FB_MSM_MIPI_SAMSUNG_OCTA_CMD_FHD_FA2_PT_PANEL)
	spinlock_t te_lock;
#endif
	spinlock_t koff_lock;
	struct work_struct clk_work;
	struct delayed_work pc_work;
	struct work_struct pp_done_work;
	atomic_t pp_done_cnt;
	struct mdss_panel_recovery recovery;
	struct mdss_mdp_cmd_ctx *sync_ctx;	/* for left + right, partial update */
	bool idle_pc;
};

struct mdss_mdp_cmd_ctx mdss_mdp_cmd_ctx_list[MAX_SESSIONS];

static int mdss_mdp_cmd_do_notifier(struct mdss_mdp_cmd_ctx *ctx);
int get_lcd_attached(void);

static inline u32 mdss_mdp_cmd_line_count(struct mdss_mdp_ctl *ctl)
{
	struct mdss_mdp_mixer *mixer;
	u32 cnt = 0xffff;	/* init it to an invalid value */
	u32 init;
	u32 height;

	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON, false);
	mdss_bus_bandwidth_ctrl(1);

	mixer = mdss_mdp_mixer_get(ctl, MDSS_MDP_MIXER_MUX_LEFT);
	if (!mixer) {
		mixer = mdss_mdp_mixer_get(ctl, MDSS_MDP_MIXER_MUX_RIGHT);
		if (!mixer) {
			mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF, false);
			mdss_bus_bandwidth_ctrl(0);
			goto exit;
		}
	}

	init = mdss_mdp_pingpong_read
		(mixer, MDSS_MDP_REG_PP_VSYNC_INIT_VAL) & 0xffff;

	height = mdss_mdp_pingpong_read
		(mixer, MDSS_MDP_REG_PP_SYNC_CONFIG_HEIGHT) & 0xffff;

	if (height < init) {
		mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF, false);
		mdss_bus_bandwidth_ctrl(0);		
		goto exit;
	}

	cnt = mdss_mdp_pingpong_read
		(mixer, MDSS_MDP_REG_PP_INT_COUNT_VAL) & 0xffff;

	if (cnt < init)		/* wrap around happened at height */
		cnt += (height - init);
	else
		cnt -= init;

	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF, false);
	mdss_bus_bandwidth_ctrl(0);	

	pr_debug("cnt=%d init=%d height=%d\n", cnt, init, height);
exit:
	return cnt;
}


static int mdss_mdp_cmd_tearcheck_cfg(struct mdss_mdp_ctl *ctl,
				      struct mdss_mdp_mixer *mixer)
{
	struct mdss_mdp_pp_tear_check *te;
	struct mdss_panel_info *pinfo;
	u32 vsync_clk_speed_hz, total_lines, vclks_line, cfg;

	if (IS_ERR_OR_NULL(ctl->panel_data)) {
		pr_err("no panel data\n");
		return -ENODEV;
	}

	pinfo = &ctl->panel_data->panel_info;
	te = &ctl->panel_data->panel_info.te;

	mdss_mdp_vsync_clk_enable(1);

	vsync_clk_speed_hz =
		mdss_mdp_get_clk_rate(MDSS_CLK_MDP_VSYNC);

	total_lines = mdss_panel_get_vtotal(pinfo);

	total_lines *= pinfo->mipi.frame_rate;

	vclks_line = (total_lines) ? vsync_clk_speed_hz / total_lines : 0;

	cfg = BIT(19);
	pr_debug("%s : cfg1 = %d\n", __func__, cfg);
	if (pinfo->mipi.hw_vsync_mode) {
		cfg |= BIT(20);
		pr_debug("%s : cfg2 = %d\n", __func__, cfg);
	}

	if (te->refx100)
		vclks_line = vclks_line * pinfo->mipi.frame_rate *
			100 / te->refx100;
	else {
		pr_warn("refx100 cannot be zero! Use 6000 as default\n");
		vclks_line = vclks_line * pinfo->mipi.frame_rate *
			100 / 6000;
	}

	cfg |= vclks_line;

	pr_debug("%s: cfg=%d yres=%d vclks=%x height=%d init=%d rd=%d start=%d ",
		__func__, cfg, pinfo->yres, vclks_line, te->sync_cfg_height,
		 te->vsync_init_val, te->rd_ptr_irq, te->start_pos);
	pr_debug("thrd_start =%d thrd_cont=%d tear_ckeck_en=%d\n",
		te->sync_threshold_start, te->sync_threshold_continue,
		te->tear_check_en);

	mdss_mdp_pingpong_write(mixer, MDSS_MDP_REG_PP_SYNC_CONFIG_VSYNC, cfg);
	mdss_mdp_pingpong_write(mixer, MDSS_MDP_REG_PP_SYNC_CONFIG_HEIGHT,
				te->sync_cfg_height);
	mdss_mdp_pingpong_write(mixer, MDSS_MDP_REG_PP_VSYNC_INIT_VAL,
				te->vsync_init_val);
	mdss_mdp_pingpong_write(mixer, MDSS_MDP_REG_PP_RD_PTR_IRQ,
				te->rd_ptr_irq);
	mdss_mdp_pingpong_write(mixer, MDSS_MDP_REG_PP_START_POS,
				te->start_pos);
	mdss_mdp_pingpong_write(mixer, MDSS_MDP_REG_PP_SYNC_THRESH,
				((te->sync_threshold_continue << 16) |
				 te->sync_threshold_start));
	mdss_mdp_pingpong_write(mixer, MDSS_MDP_REG_PP_TEAR_CHECK_EN,
				te->tear_check_en);
	return 0;
}

static int mdss_mdp_cmd_tearcheck_setup(struct mdss_mdp_ctl *ctl)
{
	struct mdss_mdp_mixer *mixer;
	int rc = 0;
	mixer = mdss_mdp_mixer_get(ctl, MDSS_MDP_MIXER_MUX_LEFT);
	if (mixer) {
		rc = mdss_mdp_cmd_tearcheck_cfg(ctl, mixer);
		if (rc)
			goto err;
	}
	mixer = mdss_mdp_mixer_get(ctl, MDSS_MDP_MIXER_MUX_RIGHT);
	if (mixer)
		rc = mdss_mdp_cmd_tearcheck_cfg(ctl, mixer);
 err:
	return rc;
}

static inline void mdss_mdp_cmd_clk_on(struct mdss_mdp_cmd_ctx *ctx)
{
	unsigned long flags;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	int irq_en;

	if (!ctx->panel_on)
		return;

	mutex_lock(&ctx->clk_mtx);
	MDSS_XLOG(ctx->pp_num, atomic_read(&ctx->koff_cnt), ctx->clk_enabled,
						ctx->rdptr_enabled);
#if defined (CONFIG_FB_MSM_MDSS_DSI_DBG)
	xlog(__func__, ctx->panel_ndx, atomic_read(&ctx->koff_cnt), ctx->clk_enabled, ctx->rdptr_enabled, 0, 0);
#endif
	if (!ctx->clk_enabled) {
		ctx->clk_enabled = 1;
		if (cancel_delayed_work_sync(&ctx->pc_work))
			pr_debug("deleted pending power collapse work\n");

		if (ctx->idle_pc) {
			mdss_mdp_footswitch_ctrl_idle_pc(1,
				&ctx->ctl->mfd->pdev->dev);
			mdss_mdp_ctl_restore(ctx->ctl);
			mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON, false);
			mdss_bus_bandwidth_ctrl(1);

			if (mdss_mdp_cmd_tearcheck_setup(ctx->ctl))
				pr_warn("tearcheck setup failed\n");
			ctx->idle_pc = false;
		} else {
			mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON, false);
			mdss_bus_bandwidth_ctrl(1);			
		}

		mdss_mdp_ctl_intf_event
			(ctx->ctl, MDSS_EVENT_PANEL_CLK_CTRL, (void *)1);

		mdss_mdp_hist_intr_setup(&mdata->hist_intr, MDSS_IRQ_RESUME);
	}
	spin_lock_irqsave(&ctx->clk_lock, flags);
	irq_en =  !ctx->rdptr_enabled;
	ctx->rdptr_enabled = VSYNC_EXPIRE_TICK;
	spin_unlock_irqrestore(&ctx->clk_lock, flags);

	if (irq_en)
		mdss_mdp_irq_enable(MDSS_MDP_IRQ_PING_PONG_RD_PTR, ctx->pp_num);

	mutex_unlock(&ctx->clk_mtx);
}

static inline void mdss_mdp_cmd_clk_off(struct mdss_mdp_cmd_ctx *ctx)
{
	unsigned long flags;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	int set_clk_off = 0;

	mutex_lock(&ctx->clk_mtx);
	MDSS_XLOG(ctx->pp_num, atomic_read(&ctx->koff_cnt), ctx->clk_enabled,
						ctx->rdptr_enabled);
#if defined (CONFIG_FB_MSM_MDSS_DSI_DBG)
	xlog(__func__,ctx->panel_ndx, atomic_read(&ctx->koff_cnt), ctx->clk_enabled, ctx->rdptr_enabled, 0, 0);
#endif
	spin_lock_irqsave(&ctx->clk_lock, flags);
	if (!ctx->rdptr_enabled)
		set_clk_off = 1;
	spin_unlock_irqrestore(&ctx->clk_lock, flags);

	if ((ctx->clk_enabled && set_clk_off) || (get_lcd_attached() == 0)) {
		ctx->clk_enabled = 0;
		mdss_mdp_hist_intr_setup(&mdata->hist_intr, MDSS_IRQ_SUSPEND);
		mdss_mdp_ctl_intf_event
			(ctx->ctl, MDSS_EVENT_PANEL_CLK_CTRL, (void *)0);
		mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF, false);
		mdss_bus_bandwidth_ctrl(0);
		if ((ctx->panel_on) && (mdata->idle_pc_enabled))
			schedule_delayed_work(&ctx->pc_work,
				POWER_COLLAPSE_TIME);
	}
	mutex_unlock(&ctx->clk_mtx);
}

#if defined(CONFIG_LCD_HMT)
int skip_te_enable = 0;
static unsigned int skip_te = 0;
#endif

#if defined(CONFIG_FB_MSM_MIPI_SAMSUNG_OCTA_CMD_FHD_FA2_PT_PANEL)
int te;
int te_cnt;
int te_set_done;
struct completion te_check_comp;
int get_lcd_ldi_info(void);
#endif

static void mdss_mdp_cmd_readptr_done(void *arg)
{
	struct mdss_mdp_ctl *ctl = arg;
	struct mdss_mdp_cmd_ctx *ctx = ctl->priv_data;
	struct mdss_mdp_vsync_handler *tmp;
	ktime_t vsync_time;

#if defined(CONFIG_FB_MSM_MIPI_SAMSUNG_OCTA_CMD_FHD_FA2_PT_PANEL)
	static ktime_t vsync_time1;
	static ktime_t vsync_time2;
	static int i = 0;
	static int time1 = 0, time2 = 0;
#endif

	if (!ctx) {
		pr_err("invalid ctx\n");
		return;
	}

#if defined(CONFIG_LCD_HMT)
	if (skip_te_enable) {
		if (skip_te) {
			pr_debug("%s : Skip TE Signal \n",__func__);
			skip_te = 0;
			return;
		}
		skip_te = 1;
	}
#endif

	vsync_time = ktime_get();
	ctl->vsync_cnt++;
	MDSS_XLOG(ctl->num, atomic_read(&ctx->koff_cnt), ctx->clk_enabled,
				ctx->rdptr_enabled);

#if defined(CONFIG_FB_MSM_MIPI_SAMSUNG_OCTA_CMD_FHD_FA2_PT_PANEL)
	if (te_set_done == TE_SET_START) {

		pr_debug("%s : TE_SET_START...",__func__);

		if (i % 2 == 0) {
			vsync_time1 = ktime_get();
			time1 = (int)ktime_to_us(vsync_time1);
			te = time1 && time2 ? time1 - time2 : 0;
			pr_debug("[%s] : ktime = %d\n",__func__, te);
		} else {
			vsync_time2 = ktime_get();
			time2 = (int)ktime_to_us(vsync_time2);
			te = time1 && time2 ? time2 - time1 : 0;
			pr_debug("[%s] : ktime = %d\n",__func__, te);
		}
		i++;

		pr_debug("[%s] TE = %d\n",__func__, te);

		spin_lock(&ctx->te_lock);
		te_cnt++;
		if (te_cnt >= 2) { // check TE using only two signal..
			pr_debug(">>>> te_check_comp COMPLETE (%d) <<<< \n", te_cnt);
			complete(&te_check_comp);
		}
		spin_unlock(&ctx->te_lock);
	} else {
		pr_debug("%s : not TE_SET_START...",__func__);
	}
#endif

#if defined (CONFIG_FB_MSM_MDSS_DSI_DBG)
	xlog(__func__,ctl->num, atomic_read(&ctx->koff_cnt), ctx->clk_enabled, ctx->rdptr_enabled, 0, 0x88888);
#endif

	spin_lock(&ctx->clk_lock);
	list_for_each_entry(tmp, &ctx->vsync_handlers, list) {
		if (tmp->enabled && !tmp->cmd_post_flush)
			tmp->vsync_handler(ctl, vsync_time);
	}

	if (!ctx->vsync_enabled) {
		if (ctx->rdptr_enabled)
			ctx->rdptr_enabled--;

		/* keep clk on during kickoff */
#if defined(CONFIG_FB_MSM_MIPI_SAMSUNG_OCTA_CMD_FHD_FA2_PT_PANEL)
		/* K_CAT6 should keep clk on during TE gathering */
		if (ctx->rdptr_enabled == 0 && (atomic_read(&ctx->koff_cnt) ||
			(te_set_done != TE_SET_DONE && te_set_done != TE_SET_FAIL)))
#else
		if (ctx->rdptr_enabled == 0 && atomic_read(&ctx->koff_cnt))
#endif
			ctx->rdptr_enabled++;
	}

	if (ctx->rdptr_enabled == 0) {
		mdss_mdp_irq_disable_nosync
			(MDSS_MDP_IRQ_PING_PONG_RD_PTR, ctx->pp_num);
		complete(&ctx->stop_comp);
		schedule_work(&ctx->clk_work);
	}

	spin_unlock(&ctx->clk_lock);
}

static void mdss_mdp_cmd_underflow_recovery(void *data)
{
	struct mdss_mdp_cmd_ctx *ctx = data;
	unsigned long flags;

	if (!data) {
		pr_err("%s: invalid ctx\n", __func__);
		return;
	}

	if (!ctx->ctl)
		return;
	spin_lock_irqsave(&ctx->koff_lock, flags);
	if (atomic_read(&ctx->koff_cnt)) {
		mdss_mdp_ctl_reset(ctx->ctl);
		pr_debug("%s: intf_num=%d\n", __func__,
					ctx->ctl->intf_num);
		atomic_dec(&ctx->koff_cnt);
		mdss_mdp_irq_disable_nosync(MDSS_MDP_IRQ_PING_PONG_COMP,
						ctx->pp_num);
		complete_all(&ctx->pp_comp);
	}
	spin_unlock_irqrestore(&ctx->koff_lock, flags);
}

static void mdss_mdp_cmd_pingpong_done(void *arg)
{
	struct mdss_mdp_ctl *ctl = arg;
	struct mdss_mdp_cmd_ctx *ctx = ctl->priv_data;
	struct mdss_mdp_vsync_handler *tmp;
	ktime_t vsync_time;

	if (!ctx) {
		pr_err("%s: invalid ctx\n", __func__);
		return;
	}

	mdss_mdp_ctl_perf_set_transaction_status(ctl,
		PERF_HW_MDP_STATE, PERF_STATUS_DONE);

	spin_lock(&ctx->clk_lock);
	list_for_each_entry(tmp, &ctx->vsync_handlers, list) {
		if (tmp->enabled && tmp->cmd_post_flush)
			tmp->vsync_handler(ctl, vsync_time);
	}
	spin_unlock(&ctx->clk_lock);

	spin_lock(&ctx->koff_lock);
	mdss_mdp_irq_disable_nosync(MDSS_MDP_IRQ_PING_PONG_COMP, ctx->pp_num);
	complete_all(&ctx->pp_comp);
	MDSS_XLOG(ctl->num, atomic_read(&ctx->koff_cnt), ctx->clk_enabled,
					ctx->rdptr_enabled);
#if defined (CONFIG_FB_MSM_MDSS_DSI_DBG)
	xlog(__func__, ctl->num, atomic_read(&ctx->koff_cnt), ctx->clk_enabled, ctx->rdptr_enabled, 0, 0);
#endif

	if (atomic_read(&ctx->koff_cnt)) {
		if (atomic_dec_return(&ctx->koff_cnt)) {
			pr_err("%s: too many kickoffs=%d!\n", __func__,
			       atomic_read(&ctx->koff_cnt));
			atomic_set(&ctx->koff_cnt, 0);
		}
		if (mdss_mdp_cmd_do_notifier(ctx)) {
			atomic_inc(&ctx->pp_done_cnt);
			schedule_work(&ctx->pp_done_work);
		}
	} else
		pr_err("%s: should not have pingpong interrupt!\n", __func__);

	pr_debug("%s: ctl_num=%d intf_num=%d ctx=%d kcnt=%d\n", __func__,
		ctl->num, ctl->intf_num, ctx->pp_num, atomic_read(&ctx->koff_cnt));

	spin_unlock(&ctx->koff_lock);
}

static void pingpong_done_work(struct work_struct *work)
{
	struct mdss_mdp_cmd_ctx *ctx =
		container_of(work, typeof(*ctx), pp_done_work);

	if (ctx->ctl) {
		while (atomic_add_unless(&ctx->pp_done_cnt, -1, 0))
			mdss_mdp_ctl_notify(ctx->ctl, MDP_NOTIFY_FRAME_DONE);
#if !defined(CONFIG_FB_MSM_MIPI_SAMSUNG_OCTA_CMD_FHD_FA2_PT_PANEL)
		mdss_mdp_ctl_perf_release_bw(ctx->ctl);
#endif
	}
}

static void clk_ctrl_work(struct work_struct *work)
{
	struct mdss_mdp_cmd_ctx *ctx =
		container_of(work, typeof(*ctx), clk_work);

	if (!ctx) {
		pr_err("%s: invalid ctx\n", __func__);
		return;
	}
	mdss_mdp_cmd_clk_off(ctx);
}

static void __mdss_mdp_cmd_pc_work(struct work_struct *work)
{
	struct delayed_work *dw = to_delayed_work(work);
	struct mdss_mdp_cmd_ctx *ctx =
		container_of(dw, struct mdss_mdp_cmd_ctx, pc_work);
		return;
	if (!ctx) {
		pr_err("%s: invalid ctx\n", __func__);
		return;
	}

	if (!ctx->panel_on) {
		pr_err("Panel is off. skipping power collapse\n");
		return;
	}

	ctx->idle_pc = true;
	ctx->ctl->play_cnt = 0;
	mdss_mdp_footswitch_ctrl_idle_pc(0, &ctx->ctl->mfd->pdev->dev);
}

static int mdss_mdp_cmd_add_vsync_handler(struct mdss_mdp_ctl *ctl,
		struct mdss_mdp_vsync_handler *handle)
{
	struct mdss_mdp_ctl *sctl = NULL;
	struct mdss_mdp_cmd_ctx *ctx, *sctx = NULL;
	unsigned long flags;
	bool enable_rdptr = false;

	ctx = (struct mdss_mdp_cmd_ctx *) ctl->priv_data;
	if (!ctx) {
		pr_err("%s: invalid ctx\n", __func__);
		return -ENODEV;
	}

	MDSS_XLOG(ctl->num, atomic_read(&ctx->koff_cnt), ctx->clk_enabled,
					ctx->rdptr_enabled);
#if defined (CONFIG_FB_MSM_MDSS_DSI_DBG)
	xlog(__func__, ctl->num, atomic_read(&ctx->koff_cnt), ctx->clk_enabled, ctx->rdptr_enabled, ctl->mfd->shutdown_pending, 0);
#endif
	sctl = mdss_mdp_get_split_ctl(ctl);
	if (sctl)
		sctx = (struct mdss_mdp_cmd_ctx *)sctl->priv_data;

	spin_lock_irqsave(&ctx->clk_lock, flags);
	if (!handle->enabled) {
		handle->enabled = true;
		list_add(&handle->list, &ctx->vsync_handlers);

		enable_rdptr = !handle->cmd_post_flush;
		if (enable_rdptr) {
 			ctx->vsync_enabled++;
			if (sctx)
				sctx->vsync_enabled++;
		}
	}
	spin_unlock_irqrestore(&ctx->clk_lock, flags);
	if (enable_rdptr) {
 		mdss_mdp_cmd_clk_on(ctx);
		if (sctx)
			mdss_mdp_cmd_clk_on(sctx);
	}
	return 0;
}

static int mdss_mdp_cmd_remove_vsync_handler(struct mdss_mdp_ctl *ctl,
		struct mdss_mdp_vsync_handler *handle)
{
	struct mdss_mdp_ctl *sctl;
	struct mdss_mdp_cmd_ctx *ctx, *sctx = NULL;
	unsigned long flags;

	ctx = (struct mdss_mdp_cmd_ctx *) ctl->priv_data;
	if (!ctx) {
		pr_err("%s: invalid ctx\n", __func__);
		return -ENODEV;
	}
#if defined (CONFIG_FB_MSM_MDSS_DSI_DBG)
	xlog(__func__, ctl->num, atomic_read(&ctx->koff_cnt), ctx->clk_enabled, ctx->rdptr_enabled, 0, 0x88888);
#endif
	MDSS_XLOG(ctl->num, atomic_read(&ctx->koff_cnt), ctx->clk_enabled,
				ctx->rdptr_enabled, 0x88888);
	sctl = mdss_mdp_get_split_ctl(ctl);
	if (sctl)
		sctx = (struct mdss_mdp_cmd_ctx *) sctl->priv_data;

	spin_lock_irqsave(&ctx->clk_lock, flags);
	if (handle->enabled) {
		handle->enabled = false;
		list_del_init(&handle->list);

		if (!handle->cmd_post_flush) {
			if (ctx->vsync_enabled) {
 				ctx->vsync_enabled--;
				if (sctx)
					sctx->vsync_enabled--;
			}
			else
				WARN(1, "unbalanced vsync disable");
		}
	}
	spin_unlock_irqrestore(&ctx->clk_lock, flags);
	return 0;
}

int mdss_mdp_cmd_reconfigure_splash_done(struct mdss_mdp_ctl *ctl, bool handoff)
{
	struct mdss_panel_data *pdata;
	int ret = 0;

	pdata = ctl->panel_data;

	pdata->panel_info.cont_splash_enabled = 0;

	ret = mdss_mdp_ctl_intf_event(ctl, MDSS_EVENT_CONT_SPLASH_FINISH,
			NULL);

	mdss_mdp_ctl_intf_event(ctl, MDSS_EVENT_PANEL_CLK_CTRL, (void *)0);

	return ret;
}

static void mdp_print_reg(char *name, int off, int len)
{
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	u32 addr;
	u32 x0,x4,x8,xc;
	int i;

	addr = off - 0x100;
	if (len % 16)
	len += 16;
	len /= 16;

	pr_err("--------------- %s -------------------------------\n", name);

	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON, false);
	for (i=0; i < len; i++) {
		x0 = readl_relaxed(mdata->mdp_base + addr + 0x0);
		x4 = readl_relaxed(mdata->mdp_base + addr + 0x4);
		x8 = readl_relaxed(mdata->mdp_base + addr + 0x8);
		xc = readl_relaxed(mdata->mdp_base + addr + 0xc);
		pr_err("%08x : %08x %08x %08x %08x\n",addr+0x100, x0,x4,x8,xc);
		addr += 16;
	}
	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF, false);
}

void mdp5_dump_regs(void)
{
	pr_err("%s: =============MDP Reg DUMP==============\n", __func__);

	mdp_print_reg("mdp", 0x0, 0x1100);
	mdp_print_reg("vg0", 0x1200, 0x100); /* vg0 */
	mdp_print_reg("vg1", 0x1600, 0x100); /* vg1 */
	mdp_print_reg("vg2", 0x1a00, 0x100); /* vg2 */
	mdp_print_reg("vg3", 0x1e00, 0x100); /* vg3 */
	mdp_print_reg("rgb0", 0x2200, 0x100); /* rgb0 */
	mdp_print_reg("rgb1", 0x2600, 0x100); /* rgb1 */
	mdp_print_reg("rgb2", 0x2a00, 0x100); /* rgb2 */
	mdp_print_reg("rgb3", 0x2e00, 0x100); /* rgb3 */
	mdp_print_reg("dma0", 0x3200, 0x100); /* dma0 */
	mdp_print_reg("dma1", 0x3600, 0x100); /* dma0 */
	mdp_print_reg("layer0", 0x3a00, 0x100); /* layer0 */
	mdp_print_reg("layer1", 0x3e00, 0x100); /* layer1 */
	mdp_print_reg("layer2", 0x4200, 0x100); /* layer2 */
	mdp_print_reg("layer3", 0x4600, 0x100); /* layer3 */
	mdp_print_reg("layer4", 0x4a00, 0x100); /* layer4 */
	mdp_print_reg("layer5", 0x4e00, 0x100); /* layer5 */
	mdp_print_reg("intf0", 0x12500, 0x100); /* intf0 */
	mdp_print_reg("intf1", 0x12700, 0x100); /* intf1 */
	mdp_print_reg("intf2", 0x12900, 0x100); /* intf2 */
	mdp_print_reg("intf3", 0x12b00, 0x100); /* intf3 */
	mdp_print_reg("intf4", 0x12d00, 0x100); /* intf4 */
	mdp_print_reg("pp0", 0x12f00, 0x40); /* pp0 */
	mdp_print_reg("pp1", 0x13000, 0x40); /* pp1 */
	mdp_print_reg("pp2", 0x13100, 0x40); /* pp2 */
	mdp_print_reg("pp3", 0x13200, 0x40); /* pp3 */
}

static int mdss_mdp_cmd_wait4pingpong(struct mdss_mdp_ctl *ctl, void *arg)
{
	struct mdss_mdp_cmd_ctx *ctx;
	struct mdss_panel_data *pdata;
	unsigned long flags;
	int need_wait = 0;
	int rc = 0;

	ctx = (struct mdss_mdp_cmd_ctx *) ctl->priv_data;
	if (!ctx) {
		pr_err("invalid ctx\n");
		return -ENODEV;
	}

	pdata = ctl->panel_data;

	spin_lock_irqsave(&ctx->koff_lock, flags);
	if (atomic_read(&ctx->koff_cnt) > 0)
		need_wait = 1;
	spin_unlock_irqrestore(&ctx->koff_lock, flags);

	ctl->roi_bkup.w = ctl->width;
	ctl->roi_bkup.h = ctl->height;

	MDSS_XLOG(ctl->num, atomic_read(&ctx->koff_cnt), ctx->clk_enabled,
			ctx->rdptr_enabled, ctl->roi_bkup.w,
			ctl->roi_bkup.h);

#if defined (CONFIG_FB_MSM_MDSS_DSI_DBG)
	xlog(__func__, ctl->num, atomic_read(&ctx->koff_cnt), ctx->clk_enabled, ctx->rdptr_enabled, 0, 0);
#endif
	pr_debug("%s: need_wait=%d  intf_num=%d ctx=%p\n",
			__func__, need_wait, ctl->intf_num, ctx);

	if (need_wait) {
		rc = wait_for_completion_timeout(
				&ctx->pp_comp, msecs_to_jiffies(1000));

		if (rc <= 0) {
			WARN(1, "cmd kickoff timed out (%d) ctl=%d\n",
						rc, ctl->num);
			mdss_dsi_debug_check_te(pdata);
			MDSS_XLOG_TOUT_HANDLER("mdp", "dsi0", "dsi1",
						"edp", "hdmi", "panic");
#if defined (CONFIG_FB_MSM_MDSS_DSI_DBG)
			dumpreg();
			mdp5_dump_regs();
			mdss_mdp_debug_bus();
			xlog_dump();
			pr_err("mdp clk rate=%ld\n", mdss_mdp_get_clk_rate(MDSS_CLK_MDP_SRC));
			panic("Pingpong Timeout");
#endif
			rc = -EPERM;
			mdss_mdp_ctl_notify(ctl, MDP_NOTIFY_FRAME_TIMEOUT);
		} else {
			rc = 0;
		}
	}
#if defined (CONFIG_FB_MSM_MDSS_DSI_DBG)
	xlog(__func__,ctl->num, atomic_read(&ctx->koff_cnt), ctx->clk_enabled, ctx->rdptr_enabled, 0, rc);
#endif

	MDSS_XLOG(ctl->num, atomic_read(&ctx->koff_cnt), ctx->clk_enabled,
					ctx->rdptr_enabled, rc);
	return rc;
}

static int mdss_mdp_cmd_do_notifier(struct mdss_mdp_cmd_ctx *ctx)
{
	struct mdss_mdp_cmd_ctx *sctx;

	sctx = ctx->sync_ctx;
	if (!sctx || atomic_read(&sctx->koff_cnt) == 0)
		return 1;

	return 0;
}

static void mdss_mdp_cmd_set_sync_ctx(
		struct mdss_mdp_ctl *ctl, struct mdss_mdp_ctl *sctl)
{
	struct mdss_mdp_cmd_ctx *ctx, *sctx;

	ctx = (struct mdss_mdp_cmd_ctx *)ctl->priv_data;

	if (!ctl->panel_data->panel_info.partial_update_enabled || !sctl) {
		/* not partial or right only at partial update */
		ctx->sync_ctx = NULL;
		return;
	}

	sctx = (struct mdss_mdp_cmd_ctx *)sctl->priv_data;

	if (!sctl->roi.w && !sctl->roi.h) {
		/* left only */
		ctx->sync_ctx = NULL;
		sctx->sync_ctx = NULL;
	} else  {
		/* left + right */
		ctx->sync_ctx = sctx;
		sctx->sync_ctx = ctx;
	}
}

static int mdss_mdp_cmd_set_partial_roi(struct mdss_mdp_ctl *ctl)
{
	int rc = 0;

	/* set panel col and page addr */
	if (!ctl->panel_data->panel_info.partial_update_enabled)
		return rc;
	/* set panel col and page addr */
	rc = mdss_mdp_ctl_intf_event(ctl,
			MDSS_EVENT_ENABLE_PARTIAL_ROI, NULL);
	return rc;
}

static int mdss_mdp_cmd_set_stream_size(struct mdss_mdp_ctl *ctl)
{
	int rc = 0;

	if (!ctl->panel_data->panel_info.partial_update_enabled)
		return rc;

	/* set dsi controller stream size */
	rc = mdss_mdp_ctl_intf_event(ctl,
			MDSS_EVENT_DSI_STREAM_SIZE, NULL);
	return rc;
}

/*
 * There are 3 partial update possibilities
 * left only ==> enable left pingpong_done
 * left + right ==> enable both pingpong_done
 * right only ==> enable right pingpong_done
 *
 * notification is triggered at pingpong_done which will
 * signal timeline to release source buffer
 *
 * for left+right case, pingpong_done is enabled for both and
 * only the last pingpong_done should trigger the notification
 */


int mdss_mdp_cmd_kickoff(struct mdss_mdp_ctl *ctl, void *arg)
{
	struct mdss_mdp_ctl *sctl = NULL;
	struct mdss_mdp_cmd_ctx *ctx, *sctx = NULL;
	unsigned long flags;
	int rc;

	ctx = (struct mdss_mdp_cmd_ctx *) ctl->priv_data;
	if (!ctx) {
		pr_err("invalid ctx\n");
		return -ENODEV;
	}
	if (get_lcd_attached() == 0) {
		pr_err("%s : lcd is not attached..\n",__func__);
		return -ENODEV;
	}


	if (ctl->panel_data->panel_info.partial_update_enabled) {
		/* sctl will be null for right only */
		sctl = mdss_mdp_get_split_ctl(ctl);
	}

	if (sctl && (sctl->roi.w == 0 || sctl->roi.h == 0)) {
		/* left update only, set ssctl to null */
		sctl = NULL;
	}

	mdss_mdp_ctl_perf_set_transaction_status(ctl,
		PERF_HW_MDP_STATE, PERF_STATUS_BUSY);

	if (sctl) {
		sctx = (struct mdss_mdp_cmd_ctx *) sctl->priv_data;
		mdss_mdp_ctl_perf_set_transaction_status(sctl,
			PERF_HW_MDP_STATE, PERF_STATUS_BUSY);
	}

	if (ctx->panel_on == 0) {
		rc = mdss_mdp_ctl_intf_event(ctl, MDSS_EVENT_UNBLANK, NULL);
		WARN(rc, "intf %d unblank error (%d)\n", ctl->intf_num, rc);

		ctx->panel_on++;
		if (sctx)
			sctx->panel_on++;

		rc = mdss_mdp_ctl_intf_event(ctl, MDSS_EVENT_PANEL_ON, NULL);
		WARN(rc, "intf %d panel on error (%d)\n", ctl->intf_num, rc);
	}

	MDSS_XLOG(ctl->num, ctl->roi.x, ctl->roi.y, ctl->roi.w,
						ctl->roi.h);

	spin_lock_irqsave(&ctx->koff_lock, flags);
	atomic_inc(&ctx->koff_cnt);
	INIT_COMPLETION(ctx->pp_comp);
	if (sctx) {
		atomic_inc(&sctx->koff_cnt);
		INIT_COMPLETION(sctx->pp_comp);
	}
#if defined (CONFIG_FB_MSM_MDSS_DSI_DBG)
	xlog(__func__, ctl->num,  atomic_read(&ctx->koff_cnt), ctx->clk_enabled, ctx->rdptr_enabled, 0, 0);
#endif
	spin_unlock_irqrestore(&ctx->koff_lock, flags);

	mdss_mdp_cmd_clk_on(ctx);

	mdss_mdp_cmd_set_partial_roi(ctl);

	/*
	* tx dcs command if had any
	*/
	mdss_mdp_ctl_intf_event(ctl, MDSS_EVENT_DSI_CMDLIST_KOFF,
										(void *)&ctx->recovery);
	mdss_mdp_cmd_set_stream_size(ctl);

	mdss_mdp_cmd_set_sync_ctx(ctl, sctl);

	mdss_mdp_irq_enable(MDSS_MDP_IRQ_PING_PONG_COMP, ctx->pp_num);
	if (sctx)
		mdss_mdp_irq_enable(MDSS_MDP_IRQ_PING_PONG_COMP, sctx->pp_num);

	mdss_mdp_ctl_write(ctl, MDSS_MDP_REG_CTL_START, 1);	/* Kickoff */

	mdss_mdp_ctl_perf_set_transaction_status(ctl,
		PERF_SW_COMMIT_STATE, PERF_STATUS_DONE);

	if (sctl) {
		mdss_mdp_ctl_perf_set_transaction_status(sctl,
			PERF_SW_COMMIT_STATE, PERF_STATUS_DONE);
	}

	mb();
	MDSS_XLOG(ctl->num,  atomic_read(&ctx->koff_cnt), ctx->clk_enabled,
						ctx->rdptr_enabled);
#if defined (CONFIG_FB_MSM_MDSS_DSI_DBG)
	{
		void mdss_mdp_mixer_read(void);
		mdss_mdp_mixer_read();
	}
#endif
	return 0;
}

int mdss_mdp_cmd_stop(struct mdss_mdp_ctl *ctl)
{
	struct mdss_mdp_cmd_ctx *ctx;
	struct mdss_panel_info *pinfo = &ctl->panel_data->panel_info;
	struct mdss_panel_data *pdata;
	unsigned long flags;
	struct mdss_mdp_vsync_handler *tmp, *handle;
	int need_wait = 0;
	int ret = 0;
	pdata = ctl->panel_data;

	if (get_lcd_attached() == 0) {
		pr_err("%s : lcd is not attached..\n",__func__);
		ret = mdss_mdp_ctl_intf_event(ctl, MDSS_EVENT_PANEL_OFF, NULL);
		return 0;
	}

	ctx = (struct mdss_mdp_cmd_ctx *) ctl->priv_data;
	if (!ctx) {
		pr_err("invalid ctx\n");
		return -ENODEV;
	}

	list_for_each_entry_safe(handle, tmp, &ctx->vsync_handlers, list)
		mdss_mdp_cmd_remove_vsync_handler(ctl, handle);
	MDSS_XLOG(ctl->num, atomic_read(&ctx->koff_cnt), ctx->clk_enabled,
				ctx->rdptr_enabled, XLOG_FUNC_ENTRY);

#if defined (CONFIG_FB_MSM_MDSS_DSI_DBG)
	xlog(__func__, ctl->num, atomic_read(&ctx->koff_cnt), ctx->clk_enabled, ctx->rdptr_enabled, 0, 0x11111);
#endif
	spin_lock_irqsave(&ctx->clk_lock, flags);
	if (ctx->rdptr_enabled) {
		INIT_COMPLETION(ctx->stop_comp);
		need_wait = 1;
	}
	spin_unlock_irqrestore(&ctx->clk_lock, flags);

	if (need_wait)
		if (wait_for_completion_timeout(&ctx->stop_comp, STOP_TIMEOUT)
		    <= 0) {
			WARN(1, "stop cmd time out\n");
#if defined (CONFIG_FB_MSM_MDSS_DSI_DBG)
			mdss_dsi_debug_check_te(pdata);
			dumpreg();
			mdp5_dump_regs();
			mdss_mdp_debug_bus();
			xlog_dump();
			pr_err("mdp clk rate=%ld\n", mdss_mdp_get_clk_rate(MDSS_CLK_MDP_SRC));
			panic("cmd_stop Timeout");
#endif
			if (IS_ERR_OR_NULL(ctl->panel_data)) {
				pr_err("no panel data\n");
			} else {
				pinfo = &ctl->panel_data->panel_info;

				if (pinfo->panel_dead) {
					mdss_mdp_irq_disable
						(MDSS_MDP_IRQ_PING_PONG_RD_PTR,
								ctx->pp_num);
					ctx->rdptr_enabled = 0;
				}
			}
		}

	if (cancel_work_sync(&ctx->clk_work))
		pr_debug("no pending clk work\n");

	if (cancel_delayed_work_sync(&ctx->pc_work))
		pr_debug("deleted pending power collapse work\n");

	ctx->panel_on = 0;
	mdss_mdp_cmd_clk_off(ctx);

	flush_work(&ctx->pp_done_work);


	mdss_mdp_set_intr_callback(MDSS_MDP_IRQ_PING_PONG_RD_PTR, ctx->pp_num,
				   NULL, NULL);
	mdss_mdp_set_intr_callback(MDSS_MDP_IRQ_PING_PONG_COMP, ctx->pp_num,
				   NULL, NULL);

	if (ctl->num == 0) {
		ret = mdss_mdp_ctl_intf_event(ctl, MDSS_EVENT_BLANK, NULL);
		WARN(ret, "intf %d unblank error (%d)\n", ctl->intf_num, ret);

		ret = mdss_mdp_ctl_intf_event(ctl, MDSS_EVENT_PANEL_OFF, NULL);
		WARN(ret, "intf %d unblank error (%d)\n", ctl->intf_num, ret);
	}

	memset(ctx, 0, sizeof(*ctx));
	ctl->priv_data = NULL;

	ctl->stop_fnc = NULL;
	ctl->display_fnc = NULL;
	ctl->wait_pingpong = NULL;
	ctl->add_vsync_handler = NULL;
	ctl->remove_vsync_handler = NULL;

	MDSS_XLOG(ctl->num, atomic_read(&ctx->koff_cnt), ctx->clk_enabled,
				ctx->rdptr_enabled, XLOG_FUNC_EXIT);
#if defined (CONFIG_FB_MSM_MDSS_DSI_DBG)
	xlog(__func__, ctl->num, atomic_read(&ctx->koff_cnt), ctx->clk_enabled, ctx->rdptr_enabled, 0, 0x222222);
#endif
	pr_debug("%s:-\n", __func__);

	return 0;
}

int mdss_mdp_cmd_start(struct mdss_mdp_ctl *ctl)
{
	struct mdss_mdp_cmd_ctx *ctx;
	struct mdss_mdp_mixer *mixer;
	int i, ret;

	pr_debug("%s:+\n", __func__);

	mixer = mdss_mdp_mixer_get(ctl, MDSS_MDP_MIXER_MUX_LEFT);
	if (!mixer) {
		pr_err("mixer not setup correctly\n");
		return -ENODEV;
	}

	for (i = 0; i < MAX_SESSIONS; i++) {
		ctx = &mdss_mdp_cmd_ctx_list[i];
		if (ctx->ref_cnt == 0) {
			ctx->ref_cnt++;
			break;
		}
	}
	if (i == MAX_SESSIONS) {
		pr_err("too many sessions\n");
		return -ENOMEM;
	}

	ctl->priv_data = ctx;
	if (!ctx) {
		pr_err("invalid ctx\n");
		return -ENODEV;
	}

	ctx->ctl = ctl;
#if defined (CONFIG_FB_MSM_MDSS_DSI_DBG)
	ctx->panel_ndx = ctl->panel_ndx;
#endif
	ctx->pp_num = mixer->num;
	init_completion(&ctx->pp_comp);
	init_completion(&ctx->stop_comp);
	spin_lock_init(&ctx->clk_lock);
	spin_lock_init(&ctx->koff_lock);
#if defined(CONFIG_FB_MSM_MIPI_SAMSUNG_OCTA_CMD_FHD_FA2_PT_PANEL)
	spin_lock_init(&ctx->te_lock);
#endif
	mutex_init(&ctx->clk_mtx);
	INIT_WORK(&ctx->clk_work, clk_ctrl_work);
	INIT_DELAYED_WORK(&ctx->pc_work, __mdss_mdp_cmd_pc_work);
	INIT_WORK(&ctx->pp_done_work, pingpong_done_work);
	atomic_set(&ctx->pp_done_cnt, 0);
	INIT_LIST_HEAD(&ctx->vsync_handlers);

	ctx->recovery.fxn = mdss_mdp_cmd_underflow_recovery;
	ctx->recovery.data = ctx;

	pr_debug("%s: ctx=%p num=%d mixer=%d\n", __func__,
				ctx, ctx->pp_num, mixer->num);
	MDSS_XLOG(ctl->num, atomic_read(&ctx->koff_cnt), ctx->clk_enabled,
					ctx->rdptr_enabled);

#if defined (CONFIG_FB_MSM_MDSS_DSI_DBG)
	xlog(__func__, ctl->num, atomic_read(&ctx->koff_cnt), ctx->clk_enabled, ctx->rdptr_enabled, 0, 0);
#endif

	mdss_mdp_set_intr_callback(MDSS_MDP_IRQ_PING_PONG_RD_PTR, ctx->pp_num,
				   mdss_mdp_cmd_readptr_done, ctl);

	mdss_mdp_set_intr_callback(MDSS_MDP_IRQ_PING_PONG_COMP, ctx->pp_num,
				   mdss_mdp_cmd_pingpong_done, ctl);

	ret = mdss_mdp_cmd_tearcheck_setup(ctl);

	if (ret) {
		pr_err("tearcheck setup failed\n");
		return ret;
	}

	ctl->stop_fnc = mdss_mdp_cmd_stop;
	ctl->display_fnc = mdss_mdp_cmd_kickoff;
	ctl->wait_pingpong = mdss_mdp_cmd_wait4pingpong;
	ctl->add_vsync_handler = mdss_mdp_cmd_add_vsync_handler;
	ctl->remove_vsync_handler = mdss_mdp_cmd_remove_vsync_handler;
	ctl->read_line_cnt_fnc = mdss_mdp_cmd_line_count;
	pr_debug("%s:-\n", __func__);

	return 0;
}

