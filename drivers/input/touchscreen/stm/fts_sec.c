#ifdef SEC_TSP_FACTORY_TEST

#define TSP_FACTEST_RESULT_PASS		2
#define TSP_FACTEST_RESULT_FAIL		1
#define TSP_FACTEST_RESULT_NONE		0

#define BUFFER_MAX					(256 * 1024) - 16
#define READ_CHUNK_SIZE				128 // (2 * 1024) - 16

enum {
	TYPE_RAW_DATA = 0,
	TYPE_FILTERED_DATA = 2,
	TYPE_STRENGTH_DATA = 4,
	TYPE_BASELINE_DATA = 6
};

enum {
	BUILT_IN = 0,
	UMS,
};

enum CMD_STATUS {
	CMD_STATUS_WAITING = 0,
	CMD_STATUS_RUNNING,
	CMD_STATUS_OK,
	CMD_STATUS_FAIL,
	CMD_STATUS_NOT_APPLICABLE,
};

#ifdef FTS_SUPPORT_TOUCH_KEY
enum {
	TYPE_TOUCHKEY_RAW			= 0x34,
	TYPE_TOUCHKEY_STRENGTH	= 0x36,
	TYPE_TOUCHKEY_THRESHOLD	= 0x48,
};
#endif

static void fw_update(void *device_data);
static void get_fw_ver_bin(void *device_data);
static void get_fw_ver_ic(void *device_data);
static void get_config_ver(void *device_data);
static void get_threshold(void *device_data);
static void module_off_master(void *device_data);
static void module_on_master(void *device_data);
static void get_chip_vendor(void *device_data);
static void get_chip_name(void *device_data);
static void get_x_num(void *device_data);
static void get_y_num(void *device_data);
static void run_reference_read(void *device_data);
static void get_reference(void *device_data);
static void run_rawcap_read(void *device_data);
static void get_rawcap(void *device_data);
static void run_delta_read(void *device_data);
static void get_delta(void *device_data);
static void run_abscap_read(void *device_data);
static void run_absdelta_read(void *device_data);
static void run_trx_short_test(void *device_data);
static void get_cx_data(void *device_data);
static void run_cx_data_read(void *device_data);
static void set_tsp_test_result(void *device_data);
static void get_tsp_test_result(void *device_data);
static void hover_enable(void *device_data);
/* static void hover_no_sleep_enable(void *device_data); */
static void glove_mode(void *device_data);
static void get_glove_sensitivity(void *device_data);
static void clear_cover_mode(void *device_data);
static void fast_glove_mode(void *device_data);
static void report_rate(void *device_data);
#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
static void interrupt_control(void *device_data);
#endif
static void set_lowpower_mode(void *device_data);
static void set_deepsleep_mode(void *device_data);
static void active_sleep_enable(void *device_data);
#ifdef TSP_BOOSTER
static void boost_level(void *device_data);
#endif
static void quick_shot_enable(void *device_data);
static void second_screen_enable(void *device_data);
static void scrub_enable(void *device_data);
static void delay(void *device_data);
static void not_support_cmd(void *device_data);

static ssize_t store_cmd(struct device *dev, struct device_attribute *devattr,
			   const char *buf, size_t count);
static ssize_t show_cmd_status(struct device *dev,
				struct device_attribute *devattr, char *buf);
static ssize_t show_cmd_result(struct device *dev,
				struct device_attribute *devattr, char *buf);
static ssize_t cmd_list_show(struct device *dev,
				struct device_attribute *attr, char *buf);
static ssize_t fts_scrub_position(struct device *dev,
				struct device_attribute *attr, char *buf);

#define STM_FT_CMD(name, func)	.cmd_name = name, .cmd_func = func

struct stm_ft_cmd {
	struct list_head list;
	const char *cmd_name;
	void (*cmd_func) (void *device_data);
};

struct stm_ft_cmd stm_ft_cmds[] = {
	{STM_FT_CMD("fw_update", fw_update),},
	{STM_FT_CMD("get_fw_ver_bin", get_fw_ver_bin),},
	{STM_FT_CMD("get_fw_ver_ic", get_fw_ver_ic),},
	{STM_FT_CMD("get_config_ver", get_config_ver),},
	{STM_FT_CMD("get_threshold", get_threshold),},
	{STM_FT_CMD("module_off_master", module_off_master),},
	{STM_FT_CMD("module_on_master", module_on_master),},
	{STM_FT_CMD("module_off_slave", not_support_cmd),},
	{STM_FT_CMD("module_on_slave", not_support_cmd),},
	{STM_FT_CMD("get_chip_vendor", get_chip_vendor),},
	{STM_FT_CMD("get_chip_name", get_chip_name),},
	{STM_FT_CMD("get_x_num", get_x_num),},
	{STM_FT_CMD("get_y_num", get_y_num),},
	{STM_FT_CMD("run_reference_read", run_reference_read),},
	{STM_FT_CMD("get_reference", get_reference),},
	{STM_FT_CMD("run_rawcap_read", run_rawcap_read),},
	{STM_FT_CMD("get_rawcap", get_rawcap),},
	{STM_FT_CMD("run_delta_read", run_delta_read),},
	{STM_FT_CMD("get_delta", get_delta),},
	{STM_FT_CMD("run_abscap_read" , run_abscap_read),},
	{STM_FT_CMD("run_absdelta_read", run_absdelta_read),},
	{STM_FT_CMD("run_trx_short_test", run_trx_short_test),},
	{STM_FT_CMD("get_cx_data", get_cx_data),},
	{STM_FT_CMD("run_cx_data_read", run_cx_data_read),},
	{STM_FT_CMD("set_tsp_test_result", set_tsp_test_result),},
	{STM_FT_CMD("get_tsp_test_result", get_tsp_test_result),},
	{STM_FT_CMD("hover_enable", hover_enable),},
	{STM_FT_CMD("hover_no_sleep_enable", not_support_cmd),},
	{STM_FT_CMD("glove_mode", glove_mode),},
	{STM_FT_CMD("get_glove_sensitivity", get_glove_sensitivity),},
	{STM_FT_CMD("clear_cover_mode", clear_cover_mode),},
	{STM_FT_CMD("fast_glove_mode", fast_glove_mode),},
	{STM_FT_CMD("report_rate", report_rate),},
#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
	{STM_FT_CMD("interrupt_control", interrupt_control),},
#endif
	{STM_FT_CMD("set_lowpower_mode", set_lowpower_mode),},
	{STM_FT_CMD("set_deepsleep_mode", set_deepsleep_mode),},
	{STM_FT_CMD("active_sleep_enable", active_sleep_enable),},
#ifdef TSP_BOOSTER
	{STM_FT_CMD("boost_level", boost_level),},
#endif
	{STM_FT_CMD("quick_shot_enable", quick_shot_enable),},
	{STM_FT_CMD("second_screen_enable", second_screen_enable),},
	{STM_FT_CMD("scrub_enable", scrub_enable),},
	{STM_FT_CMD("delay", delay),},
	{STM_FT_CMD("not_support_cmd", not_support_cmd),},
};

static DEVICE_ATTR(cmd, S_IWUSR | S_IWGRP, NULL, store_cmd);
static DEVICE_ATTR(cmd_status, S_IRUGO, show_cmd_status, NULL);
static DEVICE_ATTR(cmd_result, S_IRUGO, show_cmd_result, NULL);
static DEVICE_ATTR(cmd_list, S_IRUGO, cmd_list_show, NULL);
static DEVICE_ATTR(scrub_pos, S_IRUGO, fts_scrub_position, NULL);

static struct attribute *sec_touch_facotry_attributes[] = {
	&dev_attr_cmd.attr,
	&dev_attr_cmd_status.attr,
	&dev_attr_cmd_result.attr,
	&dev_attr_cmd_list.attr,
	&dev_attr_scrub_pos.attr,
	NULL,
};

static struct attribute_group sec_touch_factory_attr_group = {
	.attrs = sec_touch_facotry_attributes,
};

static int fts_check_index(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };
	int node;
	if (info->cmd_param[0] < 0
	  || info->cmd_param[0] >= info->SenseChannelLength
	  || info->cmd_param[1] < 0
	  || info->cmd_param[1] >= info->ForceChannelLength) {
		snprintf(buff, sizeof(buff), "%s", "NG");
		strncat(info->cmd_result, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = 3;
		tsp_debug_info(true, &info->client->dev, "%s: parameter error: %u,%u\n",
			   __func__, info->cmd_param[0], info->cmd_param[1]);
		node = -1;
		return node;
	}
	node =
	info->cmd_param[1] * info->SenseChannelLength + info->cmd_param[0];
	tsp_debug_info(true, &info->client->dev, "%s: node = %d\n", __func__, node);
	return node;
}

static ssize_t fts_scrub_position(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct fts_ts_info *info = dev_get_drvdata(dev);
	char buff[CMD_STR_LEN] = { 0 };

	if (!info) {
		printk(KERN_ERR "%s: No platform data found\n",
				__func__);
		return -EINVAL;
	}

	if (!info->input_dev) {
		printk(KERN_ERR "%s: No input_dev data found\n",
				__func__);
		return -EINVAL;
	}

	tsp_debug_info(true, &info->client->dev, "%s: %d %d %d\n",
				__func__, info->scrub_id, info->scrub_x, info->scrub_y);
	snprintf(buff, sizeof(buff), "%d %d %d", info->scrub_id, info->scrub_x, info->scrub_y);

	info->scrub_id = 0;
	info->scrub_x = 0;
	info->scrub_y = 0;

	return snprintf(buf, TSP_BUF_SIZE, "%s\n", buff);

}

static ssize_t store_cmd(struct device *dev, struct device_attribute *devattr,
			   const char *buf, size_t count)
{
	struct fts_ts_info *info = dev_get_drvdata(dev);
	char *cur, *start, *end;
	char buff[CMD_STR_LEN] = { 0 };
	int len, i;
	struct stm_ft_cmd *ft_cmd_ptr = NULL;
	char delim = ',';
	bool cmd_found = false;
	int param_cnt = 0;

	if (!info) {
		printk(KERN_ERR "%s: No platform data found\n",
				__func__);
		return -EINVAL;
	}

	if (!info->input_dev) {
		printk(KERN_ERR "%s: No input_dev data found\n",
				__func__);
		return -EINVAL;
	}

	if (info->cmd_is_running == true) {
		tsp_debug_err(true, &info->client->dev, "ft_cmd: other cmd is running.\n");
		goto err_out;
	}

	/* check lock   */
	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = true;
	mutex_unlock(&info->cmd_lock);
	info->cmd_state = 1;
	for (i = 0; i < ARRAY_SIZE(info->cmd_param); i++)
		info->cmd_param[i] = 0;
	len = (int)count;
	if (*(buf + len - 1) == '\n')
		len--;
	memset(info->cmd, 0x00, ARRAY_SIZE(info->cmd));
	memcpy(info->cmd, buf, len);
	cur = strchr(buf, (int)delim);
	if (cur)
		memcpy(buff, buf, cur - buf);

	else
		memcpy(buff, buf, len);
	tsp_debug_info(true, &info->client->dev, "COMMAND : %s\n", buff);

	/* find command */
	list_for_each_entry(ft_cmd_ptr, &info->cmd_list_head, list) {
		if (!strncmp(buff, ft_cmd_ptr->cmd_name, CMD_STR_LEN)) {
			cmd_found = true;
			break;
		}
	}

	/* set not_support_cmd */
	if (!cmd_found) {
		list_for_each_entry(ft_cmd_ptr, &info->cmd_list_head, list) {
			if (!strncmp
			 ("not_support_cmd", ft_cmd_ptr->cmd_name,
			  CMD_STR_LEN))
				break;
		}
	}

	/* parsing parameters */
	if (cur && cmd_found) {
		cur++;
		start = cur;
		memset(buff, 0x00, ARRAY_SIZE(buff));

		do {
			if (*cur == delim || cur - buf == len) {
				end = cur;
				memcpy(buff, start, end - start);
				*(buff + strnlen(buff, ARRAY_SIZE(buff))) =
				'\0';
				if (kstrtoint
				 (buff, 10,
				  info->cmd_param + param_cnt) < 0)
					goto err_out;
				start = cur + 1;
				memset(buff, 0x00, ARRAY_SIZE(buff));
				param_cnt++;
			}
			cur++;
		} while (cur - buf <= len);
	}
	tsp_debug_info(true, &info->client->dev, "cmd = %s\n", ft_cmd_ptr->cmd_name);
	for (i = 0; i < param_cnt; i++)
		tsp_debug_info(true, &info->client->dev, "cmd param %d= %d\n", i,
			  info->cmd_param[i]);
	ft_cmd_ptr->cmd_func(info);

err_out:
	return count;
}

static ssize_t show_cmd_status(struct device *dev,
				 struct device_attribute *devattr, char *buf)
{
	struct fts_ts_info *info = dev_get_drvdata(dev);
	char buff[16] = { 0 };

	if (!info) {
		printk(KERN_ERR "%s: No platform data found\n",
				__func__);
		return -EINVAL;
	}

	if (!info->input_dev) {
		printk(KERN_ERR "%s: No input_dev data found\n",
				__func__);
		return -EINVAL;
	}

	tsp_debug_info(true, &info->client->dev, "tsp cmd: status:%d\n", info->cmd_state);
	if (info->cmd_state == CMD_STATUS_WAITING)
		snprintf(buff, sizeof(buff), "WAITING");

	else if (info->cmd_state == CMD_STATUS_RUNNING)
		snprintf(buff, sizeof(buff), "RUNNING");

	else if (info->cmd_state == CMD_STATUS_OK)
		snprintf(buff, sizeof(buff), "OK");

	else if (info->cmd_state == CMD_STATUS_FAIL)
		snprintf(buff, sizeof(buff), "FAIL");

	else if (info->cmd_state == CMD_STATUS_NOT_APPLICABLE)
		snprintf(buff, sizeof(buff), "NOT_APPLICABLE");
	return snprintf(buf, TSP_BUF_SIZE, "%s\n", buff);
}

static ssize_t show_cmd_result(struct device *dev,
				 struct device_attribute *devattr, char *buf)
{
	struct fts_ts_info *info = dev_get_drvdata(dev);

	if (!info) {
		printk(KERN_ERR "%s: No platform data found\n",
				__func__);
		return -EINVAL;
	}

	if (!info->input_dev) {
		printk(KERN_ERR "%s: No input_dev data found\n",
				__func__);
		return -EINVAL;
	}

	tsp_debug_info(true, &info->client->dev, "tsp cmd: result: %s\n",
		   info->cmd_result);
	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);
	info->cmd_state = 0;
	return snprintf(buf, TSP_BUF_SIZE, "%s\n", info->cmd_result);
}

static ssize_t cmd_list_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct fts_ts_info *info = dev_get_drvdata(dev);
	int ii = 0;
	char buffer[info->cmd_buffer_size];
	char buffer_name[CMD_STR_LEN];

	snprintf(buffer, 30, "++factory command list++\n");
	while (strncmp(stm_ft_cmds[ii].cmd_name, "not_support_cmd", 16) != 0) {
		snprintf(buffer_name, CMD_STR_LEN, "%s\n", stm_ft_cmds[ii].cmd_name);
		strcat(buffer, buffer_name);
		ii++;
	}

	dev_info(&info->client->dev,
		"%s: length : %u / %d\n", __func__,
		strlen(buffer), info->cmd_buffer_size);
	return snprintf(buf, TSP_BUF_SIZE, "%s\n", buffer);
}

static void set_default_result(struct fts_ts_info *info)
{
	char delim = ':';
	memset(info->cmd_result, 0x00, info->cmd_buffer_size);
	memcpy(info->cmd_result, info->cmd, strnlen(info->cmd, CMD_STR_LEN));
	strncat(info->cmd_result, &delim, 1);
}

static void set_cmd_result(struct fts_ts_info *info, char *buff, int len)
{
	strncat(info->cmd_result, buff, len);
}

static void not_support_cmd(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[16] = { 0 };
	set_default_result(info);
	snprintf(buff, sizeof(buff), "%s", "NA");
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = 4;
	tsp_debug_info(true, &info->client->dev, "%s: \"%s(%d)\"\n", __func__, buff,
		  strnlen(buff, sizeof(buff)));
}

static void fw_update(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[64] = { 0 };
	int retval = 0;

	set_default_result(info);
	if (info->touch_stopped) {
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Touch is stopped\n",
			__func__);
		snprintf(buff, sizeof(buff), "%s", "TSP turned off");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;
		return;
	}

	retval = fts_fw_update_on_hidden_menu(info, info->cmd_param[0]);

	if (retval < 0) {
		sprintf(buff, "%s", "NA");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_FAIL;
		tsp_debug_info(true, &info->client->dev, "%s: failed [%d]\n", __func__, retval);
	} else {
		sprintf(buff, "%s", "OK");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_OK;
		tsp_debug_info(true, &info->client->dev, "%s: success [%d]\n", __func__, retval);
	}

	return;
}

static int fts_get_channel_info(struct fts_ts_info *info)
{
	int rc;
	unsigned char cmd[4] =
		{ 0xB2, 0x00, 0x14, 0x02 };
	unsigned char data[FTS_EVENT_SIZE];
	int retry = 0;

	memset(data, 0x0, FTS_EVENT_SIZE);

	rc = -1;
	fts_write_reg(info, &cmd[0], 4);
	cmd[0]=READ_ONE_EVENT;
	while (fts_read_reg
	       (info, &cmd[0], 1, (unsigned char *)data, FTS_EVENT_SIZE)) {

		if (data[0] == EVENTID_RESULT_READ_REGISTER) {
			if ((data[1] == cmd[1]) && (data[2] == cmd[2]))
			{
				info->SenseChannelLength = data[3];
				info->ForceChannelLength = data[4];

				rc = 0;
				break;
			}
		}

		if (retry++ > 30) {
			rc = -1;
			tsp_debug_info(true, &info->client->dev, "Time over - wait for channel info\n");
			break;
		}
		mdelay(5);
	}

	dev_info(&info->client->dev, "%s: SenseChannel: %02d, Force Channel: %02d\n",
			__func__, info->SenseChannelLength, info->ForceChannelLength);

	return rc;
}

static void procedure_cmd_event(struct fts_ts_info *info, unsigned char *data)
{
	char buff[16] = { 0 };

	if ((data[1] == 0x00) && (data[2] == 0x62))
	{
		snprintf(buff, sizeof(buff), "%d",
					*(unsigned short *)&data[3]);
		tsp_debug_info(true, &info->client->dev, "%s: %s(%d)\n", "get_threshold", buff,
					strnlen(buff, sizeof(buff)));
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_OK;

	}
	else if ((data[1] == 0x01) && (data[2] == 0xC6))
	{
		snprintf(buff, sizeof(buff), "%d",
					*(unsigned short *)&data[3]);
		tsp_debug_info(true, &info->client->dev, "%s: %s(%d)\n", "get_glove_sensitivity", buff,
					strnlen(buff, sizeof(buff)));
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_OK;

	}
	else if ((data[1] == 0x07) && (data[2] == 0xE7))
	{
		if (data[3] <= TSP_FACTEST_RESULT_PASS) {
			sprintf(buff, "%s",
					data[3] == TSP_FACTEST_RESULT_PASS ? "PASS" :
					data[3] == TSP_FACTEST_RESULT_FAIL ? "FAIL" : "NONE");
			tsp_debug_info(true, &info->client->dev, "%s: success [%s][%d]", "get_tsp_test_result",
                                        data[3] == TSP_FACTEST_RESULT_PASS ? "PASS" :
                                        data[3] == TSP_FACTEST_RESULT_FAIL ? "FAIL" :
                                        "NONE", data[3]);
			set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
			info->cmd_state = CMD_STATUS_OK;
		}
		else
		{
			snprintf(buff, sizeof(buff), "%s", "NG");
			set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
			info->cmd_state = CMD_STATUS_FAIL;
			tsp_debug_info(true, &info->client->dev, "%s: %s(%d)\n",
							"get_tsp_test_result",
							buff,
							strnlen(buff, sizeof(buff)));
		}

	}
}

void fts_print_frame(struct fts_ts_info *info, short *min, short *max)
{
	int i = 0;
	int j = 0;
	unsigned char *pStr = NULL;
	unsigned char pTmp[16] = { 0 };

	pStr = kzalloc(6 * (info->SenseChannelLength + 1), GFP_KERNEL);
	if (pStr == NULL) {
		tsp_debug_info(true, &info->client->dev, "FTS pStr kzalloc failed\n");
		return;
	}

	memset(pStr, 0x0, 6 * (info->SenseChannelLength + 1));
	snprintf(pTmp, sizeof(pTmp), "    ");
	strncat(pStr, pTmp, 6 * info->SenseChannelLength);

	for (i = 0; i < info->SenseChannelLength; i++) {
		snprintf(pTmp, sizeof(pTmp), "Rx%02d  ", i);
		strncat(pStr, pTmp, 6 * info->SenseChannelLength);
	}

	tsp_debug_info(true, &info->client->dev, "FTS %s\n", pStr);
	memset(pStr, 0x0, 6 * (info->SenseChannelLength + 1));
	snprintf(pTmp, sizeof(pTmp), " +");
	strncat(pStr, pTmp, 6 * info->SenseChannelLength);

	for (i = 0; i < info->SenseChannelLength; i++) {
		snprintf(pTmp, sizeof(pTmp), "------");
		strncat(pStr, pTmp, 6 * info->SenseChannelLength);
	}

	tsp_debug_info(true, &info->client->dev, "FTS %s\n", pStr);

	for (i = 0; i < info->ForceChannelLength; i++) {
		memset(pStr, 0x0, 6 * (info->SenseChannelLength + 1));
		snprintf(pTmp, sizeof(pTmp), "Tx%02d | ", i);
		strncat(pStr, pTmp, 6 * info->SenseChannelLength);

		for (j = 0; j < info->SenseChannelLength; j++) {
			snprintf(pTmp, sizeof(pTmp), "%5d ", info->pFrame[(i * info->SenseChannelLength) + j]);

			if (i > 0) {
				if (info->pFrame[(i * info->SenseChannelLength) + j] < *min)
					*min = info->pFrame[(i * info->SenseChannelLength) + j];

				if (info->pFrame[(i * info->SenseChannelLength) + j] > *max)
					*max = info->pFrame[(i * info->SenseChannelLength) + j];
			}
			strncat(pStr, pTmp, 6 * info->SenseChannelLength);
		}
		tsp_debug_info(true, &info->client->dev, "FTS %s\n", pStr);
	}

	kfree(pStr);
}

int fts_read_frame(struct fts_ts_info *info, unsigned char type, short *min,
		 short *max)
{
	unsigned char pFrameAddress[8] =
	{ 0xD0, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00 };
	unsigned int FrameAddress = 0;
	unsigned int writeAddr = 0;
	unsigned int start_addr = 0;
	unsigned int end_addr = 0;
	unsigned int totalbytes = 0;
	unsigned int remained = 0;
	unsigned int readbytes = 0xFF;
	unsigned int dataposition = 0;
	unsigned char *pRead = NULL;
	int rc = 0;
	int ret = 0;
	int i = 0;
	pRead = kzalloc(BUFFER_MAX, GFP_KERNEL);

	if (pRead == NULL) {
		tsp_debug_info(true, &info->client->dev, "FTS pRead kzalloc failed\n");
		rc = 1;
		goto ErrorExit;
	}

	pFrameAddress[2] = type;
	totalbytes = info->SenseChannelLength * info->ForceChannelLength * 2;
	ret = fts_read_reg(info, &pFrameAddress[0], 3, pRead, pFrameAddress[3]);

	if (ret >= 0) {
		if (info->digital_rev == FTS_DIGITAL_REV_1)
			FrameAddress = pRead[0] + (pRead[1] << 8);
		else if (info->digital_rev == FTS_DIGITAL_REV_2)
			FrameAddress = pRead[1] + (pRead[2] << 8);

		start_addr = FrameAddress+info->SenseChannelLength * 2;
		end_addr = start_addr + totalbytes;
	} else {
		tsp_debug_info(true, &info->client->dev, "FTS read failed rc = %d \n", ret);
		rc = 2;
		goto ErrorExit;
	}

#ifdef DEBUG_MSG
	tsp_debug_info(true, &info->client->dev, "FTS FrameAddress = %X \n", FrameAddress);
	tsp_debug_info(true, &info->client->dev, "FTS start_addr = %X, end_addr = %X \n", start_addr, end_addr);
#endif

	remained = totalbytes;
	for (writeAddr = start_addr; writeAddr < end_addr; writeAddr += READ_CHUNK_SIZE) {
		pFrameAddress[1] = (writeAddr >> 8) & 0xFF;
		pFrameAddress[2] = writeAddr & 0xFF;

		if (remained >= READ_CHUNK_SIZE)
			readbytes = READ_CHUNK_SIZE;
		else
			readbytes = remained;

		memset(pRead, 0x0, readbytes);

#ifdef DEBUG_MSG
		tsp_debug_info(true, &info->client->dev, "FTS %02X%02X%02X readbytes=%d\n",
			   pFrameAddress[0], pFrameAddress[1],
			   pFrameAddress[2], readbytes);

#endif
		if (info->digital_rev == FTS_DIGITAL_REV_1) {
			fts_read_reg(info, &pFrameAddress[0], 3, pRead, readbytes);
			remained -= readbytes;

			for (i = 0; i < readbytes; i += 2) {
				info->pFrame[dataposition++] =
				pRead[i] + (pRead[i + 1] << 8);
			}
		} else if (info->digital_rev == FTS_DIGITAL_REV_2) {
			fts_read_reg(info, &pFrameAddress[0], 3, pRead, readbytes + 1);
			remained -= readbytes;

			for (i = 1; i < (readbytes+1); i += 2) {
				info->pFrame[dataposition++] =
				pRead[i] + (pRead[i + 1] << 8);
			}
		}
	}
	kfree(pRead);

#ifdef DEBUG_MSG
	tsp_debug_info(true, &info->client->dev,
		   "FTS writeAddr = %X, start_addr = %X, end_addr = %X \n",
		   writeAddr, start_addr, end_addr);
#endif

	switch (type) {
	case TYPE_RAW_DATA:
		tsp_debug_info(true, &info->client->dev, "FTS [Raw Data : 0x%X%X] \n", pFrameAddress[0],
			FrameAddress);
		break;
	case TYPE_FILTERED_DATA:
		tsp_debug_info(true, &info->client->dev, "FTS [Filtered Data : 0x%X%X] \n",
			pFrameAddress[0], FrameAddress);
		break;
	case TYPE_STRENGTH_DATA:
		tsp_debug_info(true, &info->client->dev, "FTS [Strength Data : 0x%X%X] \n",
			pFrameAddress[0], FrameAddress);
		break;
	case TYPE_BASELINE_DATA:
		tsp_debug_info(true, &info->client->dev, "FTS [Baseline Data : 0x%X%X] \n",
			pFrameAddress[0], FrameAddress);
		break;
	}
	fts_print_frame(info, min, max);

ErrorExit:
	return rc;
}

static int fts_panel_ito_test(struct fts_ts_info *info)
{
	unsigned char cmd = READ_ONE_EVENT;
	unsigned char data[FTS_EVENT_SIZE];
	unsigned char regAdd[4] = {0xB0, 0x03, 0x60, 0xFB};
	int retry = 0;
	int result = -1;

	info->fts_systemreset(info);
	info->fts_wait_for_ready(info);

	fts_command(info, SLEEPOUT);
	fts_delay(20);
	fts_interrupt_set(info, INT_DISABLE);
	fts_write_reg(info, &regAdd[0], 4);
	fts_command(info, FLUSHBUFFER);
	fts_command(info, 0xA7);
	fts_delay(200);
	memset(data, 0x0, FTS_EVENT_SIZE);
	while (fts_read_reg
			(info, &cmd, 1, (unsigned char *)data, FTS_EVENT_SIZE)) {

		if ((data[0] == 0x0F) && (data[1] == 0x05)) {
			switch (data[2]) {
			case 0x00 :
				result = 0;
				break;
			case 0x01 :
				tsp_debug_info(true, &info->client->dev, "[FTS] ITO Test result : Force channel [%d] open.\n",
					data[3]);
				break;
			case 0x02 :
				tsp_debug_info(true, &info->client->dev, "[FTS] ITO Test result : Sense channel [%d] open.\n",
					data[3]);
				break;
			case 0x03 :
				tsp_debug_info(true, &info->client->dev, "[FTS] ITO Test result : Force channel [%d] short to GND.\n",
					data[3]);
				break;
			case 0x04 :
				tsp_debug_info(true, &info->client->dev, "[FTS] ITO Test result : Sense channel [%d] short to GND.\n",
					data[3]);
				break;
			case 0x07 :
				tsp_debug_info(true, &info->client->dev, "[FTS] ITO Test result : Force channel [%d] short to force.\n",
					data[3]);
				break;
			case 0x0E :
				tsp_debug_info(true, &info->client->dev, "[FTS] ITO Test result : Sennse channel [%d] short to sense.\n",
					data[3]);
				break;
			default:
				break;
			}

			break;
		}

		if (retry++ > 30) {
			tsp_debug_info(true, &info->client->dev, "Time over - wait for result of ITO test\n");
			break;
		}
		fts_delay(10);
	}

	fts_systemreset(info);

	/* wait for ready event */
	info->fts_wait_for_ready(info);

#ifdef FTS_SUPPORT_NOISE_PARAM
	fts_set_noise_param(info);
#endif

	fts_command(info, SLEEPOUT);
	fts_command(info, SENSEON);

#ifdef FTS_SUPPORT_TOUCH_KEY
	if (info->dt_data->support_mskey)
		fts_command(info, FTS_CMD_KEY_SENSE_ON);
#endif

	if (info->hover_enabled)
		fts_command(info, FTS_CMD_HOVER_ON);

	if (info->flip_enable) {
		fts_enable_feature(info, FTS_FEATURE_COVER_GLASS, true);
	} else {
		if (info->mshover_enabled)
			fts_command(info, FTS_CMD_MSHOVER_ON);
	}
#ifdef FTS_SUPPORT_TA_MODE
	if (info->TA_Pluged)
		fts_command(info, FTS_CMD_CHARGER_PLUGGED);
#endif

	info->touch_count = 0;

	fts_command(info, FLUSHBUFFER);
	fts_interrupt_set(info, INT_ENABLE);

	return result;
}

static void get_fw_ver_bin(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[16] = { 0 };
	set_default_result(info);

	sprintf(buff, "ST%02X%04X",
			info->panel_revision,
			info->fw_main_version_of_bin);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = CMD_STATUS_OK;
	tsp_debug_info(true, &info->client->dev, "%s: %s(%d)\n", __func__, buff,
		  strnlen(buff, sizeof(buff)));
}

static void get_fw_ver_ic(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[16] = { 0 };
	set_default_result(info);

	sprintf(buff, "ST%02X%04X",
			info->panel_revision,
			info->fw_main_version_of_ic);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = CMD_STATUS_OK;
	tsp_debug_info(true, &info->client->dev, "%s: %s(%d)\n", __func__, buff,
		  strnlen(buff, sizeof(buff)));
}

static void get_config_ver(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[20] = { 0 };

	snprintf(buff, sizeof(buff), "%s_ST_%04X",
		info->dt_data->project ? FTS_MODEL_CONFIG_NAME : FTS_DEVICE_NAME,
		info->config_version_of_ic);

	set_default_result(info);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = CMD_STATUS_OK;
	tsp_debug_info(true, &info->client->dev, "%s: %s(%d)\n", __func__, buff,
		  strnlen(buff, sizeof(buff)));
}

static void get_threshold(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	unsigned char cmd[4] =
		{ 0xB2, 0x00, 0x62, 0x02 };
	int timeout=0;

	set_default_result(info);

	if (info->touch_stopped) {
		char buff[CMD_STR_LEN] = { 0 };
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Touch is stopped\n",
			__func__);
		snprintf(buff, sizeof(buff), "%s", "TSP turned off");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;
		return;
	}

	fts_write_reg(info, &cmd[0], 4);
	info->cmd_state = CMD_STATUS_RUNNING;

	while (info->cmd_state == CMD_STATUS_RUNNING) {
		if (timeout++>30) {
			info->cmd_state = CMD_STATUS_FAIL;
			break;
		}
		msleep(10);
	}
}

static void module_off_master(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[3] = { 0 };
	int ret = 0;

	mutex_lock(&info->lock);
	if (info->enabled) {
		disable_irq(info->irq);
		info->enabled = false;
	}
	mutex_unlock(&info->lock);
	info->fts_power_ctrl(info, false);

	if (ret == 0)
		snprintf(buff, sizeof(buff), "%s", "OK");
	else
		snprintf(buff, sizeof(buff), "%s", "NG");

	set_default_result(info);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	if (strncmp(buff, "OK", 2) == 0)
		info->cmd_state = CMD_STATUS_OK;
	else
		info->cmd_state = CMD_STATUS_FAIL;
	tsp_debug_info(true, &info->client->dev, "%s: %s\n", __func__, buff);
}

static void module_on_master(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[3] = { 0 };
	int ret = 0;

	mutex_lock(&info->lock);
	if (!info->enabled) {
		enable_irq(info->irq);
		info->enabled = true;
	}
	mutex_unlock(&info->lock);
	info->fts_power_ctrl(info, true);

	if (ret == 0)
		snprintf(buff, sizeof(buff), "%s", "OK");
	else
		snprintf(buff, sizeof(buff), "%s", "NG");

	set_default_result(info);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	if (strncmp(buff, "OK", 2) == 0)
		info->cmd_state = CMD_STATUS_OK;
	else
		info->cmd_state = CMD_STATUS_FAIL;
	tsp_debug_info(true, &info->client->dev, "%s: %s\n", __func__, buff);
}

static void get_chip_vendor(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[16] = { 0 };
	strncpy(buff, "STM", sizeof(buff));
	set_default_result(info);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = CMD_STATUS_OK;
	tsp_debug_info(true, &info->client->dev, "%s: %s(%d)\n", __func__, buff,
		  strnlen(buff, sizeof(buff)));
}

static void get_chip_name(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[16] = { 0 };

	if (info->digital_rev == FTS_DIGITAL_REV_1)
		strncpy(buff, "FTS3A048", sizeof(buff));
	else
		strncpy(buff, "FTS3B062", sizeof(buff));

	set_default_result(info);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = CMD_STATUS_OK;
	tsp_debug_info(true, &info->client->dev, "%s: %s(%d)\n", __func__, buff,
		  strnlen(buff, sizeof(buff)));
}

static void get_x_num(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[16] = { 0 };

	set_default_result(info);
	snprintf(buff, sizeof(buff), "%d", info->SenseChannelLength);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = 2;
	tsp_debug_info(true, &info->client->dev, "%s: %s(%d)\n", __func__, buff,
		  strnlen(buff, sizeof(buff)));
}

static void get_y_num(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[16] = { 0 };

	set_default_result(info);
	snprintf(buff, sizeof(buff), "%d", info->ForceChannelLength);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = CMD_STATUS_OK;
	tsp_debug_info(true, &info->client->dev, "%s: %s(%d)\n", __func__, buff,
		  strnlen(buff, sizeof(buff)));
}

static void run_reference_read(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };
	short min = 0x7FFF;
	short max = 0x8000;

	set_default_result(info);
	if (info->touch_stopped) {
		char buff[CMD_STR_LEN] = { 0 };
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Touch is stopped\n",
			__func__);
		snprintf(buff, sizeof(buff), "%s", "TSP turned off");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;
		return;
	}

	fts_read_frame(info, TYPE_BASELINE_DATA, &min, &max);
	snprintf(buff, sizeof(buff), "%d,%d", min, max);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = CMD_STATUS_OK;
	tsp_debug_info(true, &info->client->dev, "%s: %s\n", __func__, buff);
}

static void get_reference(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };
	short min = 0x7FFF;
	short max = 0x8000;
	short val = 0;
	int node = 0;

	set_default_result(info);
	if (info->touch_stopped) {
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Touch is stopped\n",
			__func__);
		snprintf(buff, sizeof(buff), "%s", "TSP turned off");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;
		return;
	}

	node = fts_check_index(info);
	if (node < 0)
		return;
	fts_read_frame(info, TYPE_BASELINE_DATA, &min, &max);
	val = info->pFrame[node];
	snprintf(buff, sizeof(buff), "%d", val);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = CMD_STATUS_OK;
	tsp_debug_info(true, &info->client->dev, "%s: %s(%d)\n", __func__, buff,
		   strnlen(buff, sizeof(buff)));
}

static void run_rawcap_read(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };
	short min = 0x7FFF;
	short max = 0x8000;

	set_default_result(info);

	if (info->touch_stopped) {
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Touch is stopped\n",
			__func__);
		snprintf(buff, sizeof(buff), "%s", "TSP turned off");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;
		return;
	}

	fts_delay(500);
	fts_read_frame(info, TYPE_FILTERED_DATA, &min, &max);

	snprintf(buff, sizeof(buff), "%d,%d", min, max);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = CMD_STATUS_OK;
	tsp_debug_info(true, &info->client->dev, "%s: %s\n", __func__, buff);
}

static void get_rawcap(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };
	short val = 0;
	int node = 0;

	set_default_result(info);
	if (info->touch_stopped) {
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Touch is stopped\n",
			__func__);
		snprintf(buff, sizeof(buff), "%s", "TSP turned off");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;
		return;
	}

	node = fts_check_index(info);
	if (node < 0)
		return;

	val = info->pFrame[node];
	snprintf(buff, sizeof(buff), "%d", val);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = CMD_STATUS_OK;
	tsp_debug_info(true, &info->client->dev, "%s: %s(%d)\n", __func__, buff,
		   strnlen(buff, sizeof(buff)));
}

static void run_delta_read(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };
	short min = 0x7FFF;
	short max = 0x8000;

	set_default_result(info);
	if (info->touch_stopped) {
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Touch is stopped\n",
			__func__);
		snprintf(buff, sizeof(buff), "%s", "TSP turned off");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;
		return;
	}

	fts_read_frame(info, TYPE_STRENGTH_DATA, &min, &max);
	snprintf(buff, sizeof(buff), "%d,%d", min, max);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = CMD_STATUS_OK;
	tsp_debug_info(true, &info->client->dev, "%s: %s\n", __func__, buff);
}

static void get_delta(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };
	short val = 0;
	int node = 0;

	set_default_result(info);

	if (info->touch_stopped) {
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Touch is stopped\n",
			__func__);
		snprintf(buff, sizeof(buff), "%s", "TSP turned off");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;
		return;
	}

	node = fts_check_index(info);
	if (node < 0)
		return;

	val = info->pFrame[node];
	snprintf(buff, sizeof(buff), "%d", val);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = CMD_STATUS_OK;
	tsp_debug_info(true, &info->client->dev, "%s: %s(%d)\n", __func__, buff,
		   strnlen(buff, sizeof(buff)));
}

void fts_read_self_frame(struct fts_ts_info *info, unsigned short oAddr)
{
	char buff[64] = {0, };
	short *data = 0;
	char temp[9] = {0, };
	char temp2[512] = {0, };
	int i;
	int rc;
	int retry=1;
	unsigned char regAdd[6] = {0xD0, 0x00, 0x00, 0xD0, 0x00, 0x00};

	if (info->touch_stopped) {
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Touch is stopped\n",
			__func__);
		snprintf(buff, sizeof(buff), "%s", "TSP turned off");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;
		return;
	}

	if (!info->hover_enabled) {
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Hover is disabled\n",
			__func__);
		snprintf(buff, sizeof(buff), "%s", "TSP Hover disabled");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;
		return;
	}

	while (!info->hover_ready) {
		if (retry++ > 500) {
			tsp_debug_info(true, &info->client->dev, "%s: [FTS] Timeout - Abs Raw Data Ready Event\n",
					  __func__);
			break;
		}
		fts_delay(10);
	}

	regAdd[1] = (oAddr >> 8) & 0xff;
	regAdd[2] = oAddr & 0xff;
	rc = info->fts_read_reg(info, &regAdd[0], 3, (unsigned char *)&buff[0], 5);
	if (!rc) {
		info->cmd_state = CMD_STATUS_FAIL;
		return;
	}

	if (info->digital_rev == FTS_DIGITAL_REV_1) {
		tsp_debug_info(true, &info->client->dev, "%s: Force Address : %02x%02x\n",
				__func__, buff[1], buff[0]);
		tsp_debug_info(true, &info->client->dev, "%s: Sense Address : %02x%02x\n",
				__func__, buff[3], buff[2]);
		regAdd[1] = buff[3];
		regAdd[2] = buff[2];
		regAdd[4] = buff[1];
		regAdd[5] = buff[0];
	} else if (info->digital_rev == FTS_DIGITAL_REV_2) {
		tsp_debug_info(true, &info->client->dev, "%s: Force Address : %02x%02x\n",
				__func__, buff[2], buff[1]);
		tsp_debug_info(true, &info->client->dev, "%s: Sense Address : %02x%02x\n",
				__func__, buff[4], buff[3]);
		regAdd[1] = buff[4];
		regAdd[2] = buff[3];
		regAdd[4] = buff[2];
		regAdd[5] = buff[1];
	}

	rc = info->fts_read_reg(info, &regAdd[0], 3,
				(unsigned char *)&buff[0],
				info->SenseChannelLength * 2 + 1);
	if (!rc) {
		info->cmd_state = CMD_STATUS_FAIL;
		return;
	}

	if (info->digital_rev == FTS_DIGITAL_REV_1)
		data = (short *)&buff[0];
	else if (info->digital_rev == FTS_DIGITAL_REV_2)
		data = (short *)&buff[1];

	memset(temp2, 0, 512);

	for (i = 0; i < info->SenseChannelLength; i++) {
		tsp_debug_info(true, &info->client->dev,
				"%s: Rx [%d] = %d\n", __func__,
				i,
				*data);
		memset(temp, 0, 9);
		sprintf(temp, "%d,", *data);
		strncat(temp2, temp, 9);
		data++;
	}

	rc = info->fts_read_reg(info, &regAdd[3], 3,
				(unsigned char *)&buff[0],
				info->ForceChannelLength*2 + 1);
	if (!rc) {
		info->cmd_state = CMD_STATUS_FAIL;
		return;
	}

	if (info->digital_rev == FTS_DIGITAL_REV_1)
		data = (short *)&buff[0];
	else if (info->digital_rev == FTS_DIGITAL_REV_2)
		data = (short *)&buff[1];

	for (i = 0; i < info->ForceChannelLength; i++) {
		tsp_debug_info(true, &info->client->dev,
				"%s: Tx [%d] = %d\n", __func__, i, *data);
		sprintf(temp, "%d,", *data);
		strncat(temp2, temp, 9);
		data++;
	}

	set_cmd_result(info, temp2, strnlen(temp2, sizeof(temp2)));

	info->cmd_state = CMD_STATUS_OK;
}

static void run_abscap_read(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;

	set_default_result(info);
	fts_read_self_frame(info, 0x000E);
}

static void run_absdelta_read(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;

	set_default_result(info);
	fts_read_self_frame(info, 0x0012);
}

static void run_trx_short_test(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };
	int ret = 0;

	set_default_result(info);
	if (info->touch_stopped) {
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Touch is stopped\n",
			__func__);
		snprintf(buff, sizeof(buff), "%s", "TSP turned off");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;
		return;
	}

	disable_irq(info->irq);
	ret = fts_panel_ito_test(info);
	if (ret == 0)
		snprintf(buff, sizeof(buff), "%s", "OK");
	else
		snprintf(buff, sizeof(buff), "%s", "FAIL");
	enable_irq(info->irq);

	info->cmd_state = CMD_STATUS_OK;
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	tsp_debug_info(true, &info->client->dev, "%s: %s\n", __func__, buff);
}

#define FTS_MAX_TX_LENGTH		44
#define FTS_MAX_RX_LENGTH		64

#define FTS_CX2_READ_LENGTH		4
#define FTS_CX2_ADDR_OFFSET		3
#define FTS_CX2_TX_START		0
#define FTS_CX2_BASE_ADDR		0x1000

static void get_cx_data(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };
	short val = 0;
	int node = 0;

	set_default_result(info);
	if (info->touch_stopped || !info->cx_data) {
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Touch is stopped\n",
			__func__);
		snprintf(buff, sizeof(buff), "%s", "TSP turned off");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;
		return;
	}

	node = fts_check_index(info);
	if (node < 0)
		return;

	val = info->cx_data[node];
	snprintf(buff, sizeof(buff), "%d", val);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = CMD_STATUS_OK;
	tsp_debug_info(true, &info->client->dev, "%s: %s(%d)\n", __func__, buff,
		   strnlen(buff, sizeof(buff)));

}

static void run_cx_data_read(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };
	unsigned char ReadData[info->ForceChannelLength][info->SenseChannelLength];
	unsigned char regAdd[8];
	unsigned char buf[8];
	unsigned char r_addr = READ_ONE_EVENT;
	unsigned int addr, rx_num, tx_num;
	int i, j, cx_rx_length, address_offset = 0, start_tx_offset = 0, retry = 100;

	set_default_result(info);

	if (info->touch_stopped) {
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Touch is stopped\n",
			__func__);
		snprintf(buff, sizeof(buff), "%s", "TSP turned off");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;
		return;
	}

	disable_irq(info->irq);

	tx_num = info->ForceChannelLength;
	rx_num = info->SenseChannelLength;

	start_tx_offset = FTS_CX2_TX_START * FTS_MAX_RX_LENGTH / FTS_CX2_READ_LENGTH * FTS_CX2_ADDR_OFFSET;
	address_offset = FTS_MAX_RX_LENGTH /FTS_CX2_READ_LENGTH;

	for(j = 0; j < tx_num; j++) {
		addr = FTS_CX2_BASE_ADDR + (j * address_offset * FTS_CX2_ADDR_OFFSET) + start_tx_offset;

		if(rx_num % FTS_CX2_READ_LENGTH != 0)
			cx_rx_length = rx_num / FTS_CX2_READ_LENGTH + 1;
		else
			cx_rx_length = rx_num / FTS_CX2_READ_LENGTH;

		for(i = 0; i < cx_rx_length; i++) {
			regAdd[0] = 0xB2;
			regAdd[1] = (addr >> 8) & 0xff;
			regAdd[2] = (addr & 0xff);
			regAdd[3] = 0x04;
			fts_write_reg(info, &regAdd[0], 4);

			retry = 100;
			do {
				if (retry < 0) {
					dev_err(&info->client->dev,
							"%s: failed to compare buf, break!\n", __func__);
					break;
				}

				fts_read_reg(info, &r_addr, 1, &buf[0], FTS_EVENT_SIZE);
				retry--;
			}while(buf[1] != regAdd[1] || buf[2] != regAdd[2]);

			ReadData[j][i * 4] = buf[3] & 0x3F;
			ReadData[j][i * 4 + 1] = (buf[3] & 0xC0) >> 6 | (buf[4] & 0x0F) << 2;
			ReadData[j][i * 4 + 2] = ((buf[4] & 0xF0)>> 4) | ((buf[5] & 0x03) << 4);
			ReadData[j][i * 4 + 3] = buf[5] >> 2;
			addr = addr + 3;

			dev_info(&info->client->dev, "%s: [Tx:%d][Rx:%d] :[%d][%d][%d][%d]\n", __func__,
				i, j, ReadData[j][i*4], ReadData[j][i*4+1], ReadData[j][i*4+2], ReadData[j][i*4+3]);
		}
	}

	if (info->cx_data) {
		for (j = 0; j < tx_num; j++) {
			for(i = 0; i < rx_num; i++)
				info->cx_data[(j * rx_num) + i] = ReadData[j][i];
		}
	}
	snprintf(buff, sizeof(buff), "%s", "OK");
	enable_irq(info->irq);

	info->cmd_state = CMD_STATUS_OK;
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	tsp_debug_info(true, &info->client->dev, "%s: %s\n", __func__, buff);
}

static void set_tsp_test_result(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };
	unsigned char regAdd[4] = {0xB0, 0x07, 0xE7, 0x00};

	set_default_result(info);

	if (info->cmd_param[0] < TSP_FACTEST_RESULT_NONE
				|| info->cmd_param[0] > TSP_FACTEST_RESULT_PASS) {
		snprintf(buff, sizeof(buff), "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
		return;
	}

	if (info->touch_stopped) {
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Touch is stopped\n",
			__func__);
		snprintf(buff, sizeof(buff), "%s", "TSP turned off");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;
		return;
	}

	regAdd[3] = info->cmd_param[0];
	fts_write_reg(info, &regAdd[0], 4);
	fts_delay(100);
	fts_command(info, FTS_CMD_SAVE_FWCONFIG);

	snprintf(buff, sizeof(buff), "%s", "OK");
	info->cmd_state = CMD_STATUS_OK;
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	tsp_debug_info(true, &info->client->dev, "%s: %s\n", __func__, buff);
}

static void get_tsp_test_result(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	unsigned char cmd[4] = {0xB2, 0x07, 0xE7, 0x01};
	int timeout = 0;

	set_default_result(info);

	if (info->touch_stopped) {
		char buff[CMD_STR_LEN] = { 0 };
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Touch is stopped\n",
			__func__);
		snprintf(buff, sizeof(buff), "%s", "TSP turned off");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;
		return;
	}

	fts_command(info, FLUSHBUFFER);
	fts_write_reg(info, &cmd[0], 4);
	info->cmd_state = CMD_STATUS_RUNNING;

	while (info->cmd_state == CMD_STATUS_RUNNING) {
		if (timeout++>30) {
			info->cmd_state = CMD_STATUS_FAIL;
			break;
		}
		fts_delay(10);
	}

	info->cmd_state = CMD_STATUS_OK;
}

static void hover_enable(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };

	set_default_result(info);

	if (info->touch_stopped || !(info->reinit_done) || (info->fts_power_mode==FTS_POWER_STATE_LOWPOWER)) {
		tsp_debug_info(true, &info->client->dev,
			"%s: [ERROR] Touch is stopped:%d, reinit_done:%d, power mode:%d\n",
			__func__, info->touch_stopped, info->reinit_done, info->fts_power_mode);
		snprintf(buff, sizeof(buff), "%s", "TSP turned off");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;

		if(info->cmd_param[0]==1){
			info->retry_hover_enable_after_wakeup = 1;
			tsp_debug_info(true, &info->client->dev, "%s: retry_hover_on_after_wakeup \n", __func__);
		}
			
		goto out;
	}

	if (info->cmd_param[0] < 0 || info->cmd_param[0] > 1) {
		snprintf(buff, sizeof(buff), "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
	} else {
		int enables;
		enables = info->cmd_param[0];
		if (enables) {
			unsigned char regAdd[4] = {0xB0, 0x01, 0x29, 0x41};
			fts_write_reg(info, &regAdd[0], 4);
			fts_command(info, FTS_CMD_HOVER_ON);
			info->hover_ready = false;
			info->hover_enabled = true;
		} else {
			fts_command(info, FTS_CMD_HOVER_OFF);
			info->hover_enabled = false;
			info->hover_ready = false;
			input_mt_slot(info->input_dev, 0);
			input_mt_report_slot_state(info->input_dev,
						MT_TOOL_FINGER, 0);
			input_sync(info->input_dev);
		}

		snprintf(buff, sizeof(buff), "%s", "OK");
		info->cmd_state = CMD_STATUS_OK;
	}
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = CMD_STATUS_WAITING;

out:
	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);

	tsp_debug_info(true, &info->client->dev, "%s: %s\n", __func__, buff);
}

/*
static void hover_no_sleep_enable(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	unsigned char regAdd[4] = {0xB0, 0x01, 0x18, 0x00};
	char buff[CMD_STR_LEN] = { 0 };

	set_default_result(info);

	if (info->touch_stopped) {
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Touch is stopped\n",
			__func__);
		snprintf(buff, sizeof(buff), "%s", "TSP turned off");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;
		return;
	}

	if (info->cmd_param[0] < 0 || info->cmd_param[0] > 1) {
		snprintf(buff, sizeof(buff), "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
	} else {
		if (info->cmd_param[0]) {
			regAdd[3]=0x0F;
		} else {
			regAdd[3]=0x08;
		}
		fts_write_reg(info, &regAdd[0], 4);

		snprintf(buff, sizeof(buff), "%s", "OK");
		info->cmd_state = CMD_STATUS_OK;
	}
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	tsp_debug_info(true, &info->client->dev, "%s: %s\n", __func__, buff);
}
*/

static void glove_mode(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };

	set_default_result(info);

	if (info->cmd_param[0] < 0 || info->cmd_param[0] > 1) {
		snprintf(buff, sizeof(buff), "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
	} else {
		info->mshover_enabled = info->cmd_param[0];

		if (!info->touch_stopped && info->reinit_done) {
			if (info->mshover_enabled)
				fts_command(info, FTS_CMD_MSHOVER_ON);
			else
				fts_command(info, FTS_CMD_MSHOVER_OFF);
		}

		snprintf(buff, sizeof(buff), "%s", "OK");
		info->cmd_state = CMD_STATUS_OK;
	}
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = CMD_STATUS_WAITING;

	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);

	tsp_debug_info(true, &info->client->dev, "%s: %s\n", __func__, buff);
}

static void get_glove_sensitivity(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	unsigned char cmd[4] =
		{ 0xB2, 0x01, 0xC6, 0x02 };
	int timeout=0;

	set_default_result(info);

	if (info->touch_stopped) {
		char buff[CMD_STR_LEN] = { 0 };
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Touch is stopped\n",
			__func__);
		snprintf(buff, sizeof(buff), "%s", "TSP turned off");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;
		return;
	}

	fts_write_reg(info, &cmd[0], 4);
	info->cmd_state = CMD_STATUS_RUNNING;

	while (info->cmd_state == CMD_STATUS_RUNNING) {
		if (timeout++>30) {
			info->cmd_state = CMD_STATUS_FAIL;
			break;
		}
		msleep(10);
	}
}

static void clear_cover_mode(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };

	set_default_result(info);

	if (info->cmd_param[0] < 0 || info->cmd_param[0] > 3) {
		snprintf(buff, sizeof(buff), "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
	} else {
		if (info->cmd_param[0] > 1)
			info->flip_enable = true;
		else
			info->flip_enable = false;

		if (!info->touch_stopped && info->reinit_done) {
			if (info->flip_enable) {
				if (info->mshover_enabled)
					fts_command(info, FTS_CMD_MSHOVER_OFF);

				fts_enable_feature(info, FTS_FEATURE_COVER_GLASS, true);
			} else {
				fts_enable_feature(info, FTS_FEATURE_COVER_GLASS, false);

				if (info->fast_mshover_enabled)
					fts_command(info, FTS_CMD_SET_FAST_GLOVE_MODE);
				else if (info->mshover_enabled)
					fts_command(info, FTS_CMD_MSHOVER_ON);
			}
		}

		snprintf(buff, sizeof(buff), "%s", "OK");
		info->cmd_state = CMD_STATUS_OK;
	}
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = CMD_STATUS_WAITING;

	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);

	tsp_debug_info(true, &info->client->dev, "%s: %s\n", __func__, buff);
};

static void fast_glove_mode(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };

	set_default_result(info);

	if (info->cmd_param[0] < 0 || info->cmd_param[0] > 1) {
		snprintf(buff, sizeof(buff), "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
	} else {
		info->fast_mshover_enabled = info->cmd_param[0];

		if (!info->touch_stopped && info->reinit_done) {
			if (info->fast_mshover_enabled)
				fts_command(info, FTS_CMD_SET_FAST_GLOVE_MODE);
			else
				fts_command(info, FTS_CMD_SET_NOR_GLOVE_MODE);
		}

		snprintf(buff, sizeof(buff), "%s", "OK");
		info->cmd_state = CMD_STATUS_OK;
	}
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = CMD_STATUS_WAITING;

	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);

	tsp_debug_info(true, &info->client->dev, "%s: %s\n", __func__, buff);
};

static void report_rate(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };
	int ret;

	set_default_result(info);

#ifdef CONFIG_SEC_TBLTE_PROJECT
	/* This feature will fixed after firmware version 6684 */
	{
		snprintf(buff, sizeof(buff), "%s", "Not Support");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;
		goto out;

	}
#endif
	if (info->touch_stopped) {
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Touch is stopped\n",
			__func__);
		snprintf(buff, sizeof(buff), "%s", "TSP turned off");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;
		goto out;
	}

	if (info->cmd_param[0] < 0 || info->cmd_param[0] > 2) {
		snprintf(buff, sizeof(buff), "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
	} else {

		if (info->cmd_param[0] == REPORT_RATE_90HZ) {
			ret = info->fts_change_scan_rate(info, FTS_CMD_FAST_SCAN);
		} else if (info->cmd_param[0] == REPORT_RATE_60HZ) {
			ret = info->fts_change_scan_rate(info, FTS_CMD_SLOW_SCAN);
		} else if (info->cmd_param[0] == REPORT_RATE_30HZ) {
			/* will be fixed 30hz report rate command */
			ret = info->fts_change_scan_rate(info, FTS_CMD_USLOW_SCAN);
		}
		if (ret < 0) {
			dev_err(&info->client->dev, "%s: can not change report rate, %d\n", __func__, ret);
			snprintf(buff, sizeof(buff), "%s", "FAIL");
			info->cmd_state = CMD_STATUS_FAIL;
		} else {
			snprintf(buff, sizeof(buff), "%s", "OK");
			info->cmd_state = CMD_STATUS_OK;
		}
	}
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = CMD_STATUS_WAITING;

out:
	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);

	tsp_debug_info(true, &info->client->dev, "%s: %s\n", __func__, buff);
}

#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
static void interrupt_control(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };

	set_default_result(info);

	if (info->touch_stopped) {
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Touch is stopped\n",
			__func__);
		snprintf(buff, sizeof(buff), "%s", "TSP turned off");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;
		goto out;
	}

	if (info->cmd_param[0] < 0 || info->cmd_param[0] > 1) {
		snprintf(buff, sizeof(buff), "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
	} else {
		int enables;
		enables = info->cmd_param[0];
		if (enables)
			fts_irq_enable(info, true);
		else
			fts_irq_enable(info, false);

		snprintf(buff, sizeof(buff), "%s", "OK");
		info->cmd_state = CMD_STATUS_OK;
	}
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = CMD_STATUS_WAITING;

out:
	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);

	tsp_debug_info(true, &info->client->dev, "%s: %s\n", __func__, buff);
}
#endif

bool check_lowpower_mode(unsigned char flag)
{
	bool ret = 0;
	flag = flag & 0x0F;

	if(flag) ret = 1;
	
	pr_err("[tsp] %s: lowpower_mode flag : %d, ret:%d\n", __func__, flag, ret);

	
	if(flag & FTS_LOWP_FLAG_QUICK_CAM)
		pr_err("[tsp] %s: quick cam *- on\n", __func__);
	else if(flag & FTS_LOWP_FLAG_2ND_SCREEN)
		pr_err("[tsp] %s: 2nd screen on\n", __func__);
	else if(flag & FTS_LOWP_FLAG_BLACK_UI)
		pr_err("[tsp] %s: swipe finger on\n", __func__);
	else if(flag & FTS_LOWP_FLAG_TEMP_CMD)
		pr_err("[tsp] %s: known cmd on\n", __func__);

	return ret;

}

static void set_lowpower_mode(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };

	set_default_result(info);

	if (info->cmd_param[0] < 0 || info->cmd_param[0] > 1) {
	        snprintf(buff, sizeof(buff), "%s", "NG");
	        info->cmd_state = CMD_STATUS_FAIL;
	} else {
		if (info->cmd_param[0]) {
			info->lowpower_mode = true;
		} else {
			info->lowpower_mode = false;
		}
		snprintf(buff, sizeof(buff), "%s", "OK");
		info->cmd_state = CMD_STATUS_OK;
	}
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));

	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);
	info->cmd_state = CMD_STATUS_WAITING;

	tsp_debug_info(true, &info->client->dev, "%s: %s\n", __func__, buff);
};

static void set_deepsleep_mode(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };

	set_default_result(info);

	if (info->cmd_param[0] < 0 || info->cmd_param[0] > 1) {
		snprintf(buff, sizeof(buff), "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
	} else {
		info->deepsleep_mode = info->cmd_param[0];

		snprintf(buff, sizeof(buff), "%s", "OK");
		info->cmd_state = CMD_STATUS_OK;
	}

	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));

	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);
	info->cmd_state = CMD_STATUS_WAITING;

	tsp_debug_info(true, &info->client->dev, "%s: %s\n", __func__, buff);
};

static void active_sleep_enable(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };

	set_default_result(info);

	if (info->cmd_param[0] < 0 || info->cmd_param[0] > 1) {
		snprintf(buff, sizeof(buff), "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
	} else {
	/* To do here */
		snprintf(buff, sizeof(buff), "%s", "OK");
		info->cmd_state = CMD_STATUS_OK;
	}

	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));

	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);
	info->cmd_state = CMD_STATUS_WAITING;

	tsp_debug_info(true, &info->client->dev, "%s: %s\n", __func__, buff);
};

#ifdef TSP_BOOSTER
static void boost_level(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };
	int retval;
	int stage;

	set_default_result(info);

	stage = 1 << info->cmd_param[0];
	if (!(info->tsp_booster->dvfs_stage & stage)) {
		snprintf(buff, sizeof(buff), "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
		tsp_debug_err(true, &info->client->dev,
				"%s: %d is not supported(%04x != %04x).\n",
				__func__, info->cmd_param[0],
				stage, info->tsp_booster->dvfs_stage);

		goto boost_out;
	}

	info->tsp_booster->dvfs_boost_mode = stage;

	snprintf(buff, sizeof(buff), "%s", "OK");
	info->cmd_state = CMD_STATUS_OK;

	if (info->tsp_booster->dvfs_boost_mode == DVFS_STAGE_NONE) {
		retval = info->tsp_booster->dvfs_off(info->tsp_booster);
		if (retval < 0) {
			tsp_debug_err(true, &info->client->dev,
					"%s: booster stop failed(%d).\n",
					__func__, retval);
			snprintf(buff, sizeof(buff), "%s", "NG");
			info->cmd_state = CMD_STATUS_FAIL;
		}
	}

boost_out:
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));

	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);
	info->cmd_state = CMD_STATUS_WAITING;
	tsp_debug_info(true, &info->client->dev, "%s: %s\n", __func__, buff);
}
#endif

static void quick_shot_enable(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	unsigned short addr = FTS_CMD_STRING_ACCESS;
	int ret;
	char buff[CMD_STR_LEN] = { 0 };

	if (info->touch_stopped) {
		dev_info(&info->client->dev, "%s: [ERROR] Touch is stopped\n", __func__);

		snprintf(buff, sizeof(buff), "%s", "TSP turned off");

		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;
		goto out;
	}

	set_default_result(info);

	ret = info->fts_write_to_string(info, &addr, &info->fts_mode, sizeof(info->fts_mode));
	if (ret < 0) {
		dev_err(&info->client->dev, "%s: failed. ret: %d\n", __func__, ret);
		snprintf(buff, sizeof(buff), "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;

		goto out;
	}

	snprintf(buff, sizeof(buff), "%s", "OK");
	info->cmd_state = CMD_STATUS_OK;

	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));

out:
	if (info->cmd_param[0]) {
		info->fts_mode |= FTS_MODE_QUICK_SHOT;
		info->lowpower_flag = info->lowpower_flag | FTS_LOWP_FLAG_BLACK_UI;
	} else {
		info->fts_mode &= ~FTS_MODE_QUICK_SHOT;
		info->lowpower_flag = info->lowpower_flag & ~(FTS_LOWP_FLAG_BLACK_UI);
	}

	info->lowpower_mode = check_lowpower_mode(info->lowpower_flag);

	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);
	info->cmd_state = CMD_STATUS_WAITING;

	dev_info(&info->client->dev, "%s: %s\n", __func__, buff);
}


static void second_screen_enable(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };

	set_default_result(info);

	if (info->cmd_param[0] < 0 || info->cmd_param[0] > 1) {
	        snprintf(buff, sizeof(buff), "%s", "NG");
	        info->cmd_state = CMD_STATUS_FAIL;
	} else {
		if (info->cmd_param[0])
			info->lowpower_flag = info->lowpower_flag | FTS_LOWP_FLAG_2ND_SCREEN;
		else
			info->lowpower_flag = info->lowpower_flag & ~(FTS_LOWP_FLAG_2ND_SCREEN);

		info->lowpower_mode = check_lowpower_mode(info->lowpower_flag);

		snprintf(buff, sizeof(buff), "%s", "OK");
		info->cmd_state = CMD_STATUS_OK;
	}
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));

	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);
	info->cmd_state = CMD_STATUS_WAITING;

	tsp_debug_info(true, &info->client->dev, "%s: %s\n", __func__, buff);
};

static void scrub_enable(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	unsigned short addr = FTS_CMD_STRING_ACCESS;
	int ret;
	char buff[CMD_STR_LEN] = { 0 };

	if (info->touch_stopped) {
		dev_info(&info->client->dev, "%s: [ERROR] Touch is stopped\n", __func__);

		snprintf(buff, sizeof(buff), "%s", "TSP turned off");

		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = CMD_STATUS_NOT_APPLICABLE;
		goto out;
	}

	set_default_result(info);

	ret = info->fts_write_to_string(info, &addr, &info->fts_mode, sizeof(info->fts_mode));
	if (ret < 0) {
		dev_err(&info->client->dev, "%s: failed. ret: %d\n", __func__, ret);
		snprintf(buff, sizeof(buff), "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;

		goto out;
	}

	snprintf(buff, sizeof(buff), "%s", "OK");
	info->cmd_state = CMD_STATUS_OK;

	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));

out:
	if (info->cmd_param[0]) {
		info->fts_mode |= FTS_MODE_SCRUB;
		info->lowpower_flag = info->lowpower_flag | FTS_LOWP_FLAG_BLACK_UI;

	} else {
		info->fts_mode &= ~FTS_MODE_SCRUB;
		info->lowpower_flag = info->lowpower_flag & ~(FTS_LOWP_FLAG_BLACK_UI);
	}

	info->lowpower_mode = check_lowpower_mode(info->lowpower_flag);

	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);
	info->cmd_state = CMD_STATUS_WAITING;

	dev_info(&info->client->dev, "%s: %s\n", __func__, buff);
}

static void delay(void *device_data)
{
	struct fts_ts_info *info = (struct fts_ts_info *)device_data;
	char buff[CMD_STR_LEN] = { 0 };

	set_default_result(info);

	info->delay_time = info->cmd_param[0];

	dev_info(&info->client->dev, "%s: delay time is %d\n", __func__, info->delay_time);
	snprintf(buff, sizeof(buff), "%d", info->delay_time);

	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));

	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);
	info->cmd_state = CMD_STATUS_WAITING;

	dev_info(&info->client->dev, "%s: %s\n", __func__, buff);
}

#ifdef FTS_SUPPORT_TOUCH_KEY
int read_touchkey_data(struct fts_ts_info *info, unsigned char type, unsigned int keycode)
{
	unsigned char pCMD[3] = { 0xD0, 0x00, 0x00};
	unsigned char buf[9] = { 0 };
	int i;
	int ret = 0;

	pCMD[2] = type;

	ret = fts_read_reg(info, &pCMD[0], 3, buf, 3);
	if (ret >= 0) {
		if (info->digital_rev == FTS_DIGITAL_REV_1) {
			pCMD[1] = buf[1];
			pCMD[2] = buf[0];
		}
		else {
			pCMD[1] = buf[2];
			pCMD[2] = buf[1];
		}
	} else
		return -1;

	ret = fts_read_reg(info, &pCMD[0], 3, buf, 9);
	if (ret < 0)
		return -2;

	for (i = 0 ; i < info->dt_data->num_touchkey ; i++)
		if (info->dt_data->touchkey[i].keycode == keycode) {
			if (info->digital_rev == FTS_DIGITAL_REV_1)
				return *(unsigned short *)&buf[(info->dt_data->touchkey[i].value - 1) * 2];
			else
				return *(unsigned short *)&buf[(info->dt_data->touchkey[i].value - 1) * 2 + 1];
		}

	return -3;
}

static ssize_t touchkey_recent_strength(struct device *dev,
				       struct device_attribute *attr, char *buf) {
	struct fts_ts_info *info = dev_get_drvdata(dev);
	int value = 0;

	if (info->touch_stopped) {
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Touch is stopped\n", __func__);
		return sprintf(buf, "%d\n", value);
	}

	value = read_touchkey_data(info, TYPE_TOUCHKEY_STRENGTH, KEY_RECENT);

	return sprintf(buf, "%d\n", value);
}

static ssize_t touchkey_back_strength(struct device *dev,
				       struct device_attribute *attr, char *buf) {
	struct fts_ts_info *info = dev_get_drvdata(dev);
	int value = 0;

	if (info->touch_stopped) {
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Touch is stopped\n", __func__);
		return sprintf(buf, "%d\n", value);
	}

	value = read_touchkey_data(info, TYPE_TOUCHKEY_STRENGTH, KEY_BACK);

	return sprintf(buf, "%d\n", value);
}

static ssize_t touchkey_recent_raw(struct device *dev,
				       struct device_attribute *attr, char *buf) {
	struct fts_ts_info *info = dev_get_drvdata(dev);
	int value = 0;

	if (info->touch_stopped) {
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Touch is stopped\n", __func__);
		return sprintf(buf, "%d\n", value);
	}

	value = read_touchkey_data(info, TYPE_TOUCHKEY_RAW, KEY_RECENT);

	return sprintf(buf, "%d\n", value);
}

static ssize_t touchkey_back_raw(struct device *dev,
				       struct device_attribute *attr, char *buf) {
	struct fts_ts_info *info = dev_get_drvdata(dev);
	int value = 0;

	if (info->touch_stopped) {
		tsp_debug_info(true, &info->client->dev, "%s: [ERROR] Touch is stopped\n", __func__);
		return sprintf(buf, "%d\n", value);
	}

	value = read_touchkey_data(info, TYPE_TOUCHKEY_RAW, KEY_BACK);

	return sprintf(buf, "%d\n", value);
}

static ssize_t touchkey_threshold(struct device *dev,
				       struct device_attribute *attr, char *buf) {
	struct fts_ts_info *info = dev_get_drvdata(dev);
	unsigned char pCMD[3] = { 0xD0, 0x00, 0x00};
	int value;
	int ret = 0;

	value = -1;
	pCMD[2] = TYPE_TOUCHKEY_THRESHOLD;
	ret = fts_read_reg(info, &pCMD[0], 3, buf, 3);
	if (ret >= 0) {
		if (info->digital_rev == FTS_DIGITAL_REV_1)
			value = *(unsigned short *)&buf[0];
		else
			value = *(unsigned short *)&buf[1];
	}

	info->touchkey_threshold = value;
	return sprintf(buf, "%d\n", info->touchkey_threshold);
}

static ssize_t fts_touchkey_led_control(struct device *dev,
				 struct device_attribute *attr, const char *buf,
				 size_t size)
{
	struct fts_ts_info *info = dev_get_drvdata(dev);
	int data, ret;

	ret = sscanf(buf, "%d", &data);
	tsp_debug_dbg(true, &info->client->dev, "%s, %d\n", __func__, data);

	if (ret != 1) {
		tsp_debug_err(true, &info->client->dev, "%s, %d err\n",
			__func__, __LINE__);
		return size;
	}

	if (data != 0 && data != 1) {
		tsp_debug_err(true, &info->client->dev, "%s wrong cmd %x\n",
			__func__, data);
		return size;
	}

	if (info->led_power)
		ret = info->led_power(info, data);

	if (ret) {
		tsp_debug_err(true, &info->client->dev, "%s: Error turn on led %d\n",
			__func__, ret);

		goto out;
	}
	msleep(30);

out:
	return size;
}

static DEVICE_ATTR(brightness, S_IRUGO | S_IWUSR | S_IWGRP, NULL, fts_touchkey_led_control);
static DEVICE_ATTR(touchkey_recent, S_IRUGO, touchkey_recent_strength, NULL);
static DEVICE_ATTR(touchkey_back, S_IRUGO, touchkey_back_strength, NULL);
static DEVICE_ATTR(touchkey_recent_raw, S_IRUGO, touchkey_recent_raw, NULL);
static DEVICE_ATTR(touchkey_back_raw, S_IRUGO, touchkey_back_raw, NULL);
static DEVICE_ATTR(touchkey_threshold, S_IRUGO, touchkey_threshold, NULL);

static struct attribute *sec_touchkey_factory_attributes[] = {
	&dev_attr_touchkey_recent.attr,
	&dev_attr_touchkey_back.attr,
	&dev_attr_touchkey_recent_raw.attr,
	&dev_attr_touchkey_back_raw.attr,
	&dev_attr_touchkey_threshold.attr,
	&dev_attr_brightness.attr,
	NULL,
};

static struct attribute_group sec_touchkey_factory_attr_group = {
	.attrs = sec_touchkey_factory_attributes,
};
#endif

#endif


