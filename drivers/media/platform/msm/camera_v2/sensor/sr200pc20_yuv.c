/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
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

#include "sr200pc20.h"
#include "sr200pc20_yuv_t8.h"
#include "msm_sd.h"
#include "camera.h"
#include "msm_cci.h"
#include "msm_camera_dt_util.h"

#define sr200pc20_WRT_LIST(s_ctrl,A)\
    s_ctrl->sensor_i2c_client->i2c_func_tbl->\
    i2c_write_conf_tbl(\
    s_ctrl->sensor_i2c_client, A,\
    ARRAY_SIZE(A),\
    MSM_CAMERA_I2C_BYTE_DATA);\
	pr_err("sr200pc20_WRT_LIST - %s\n", #A);

#define SR200PC20_VALUE_EXPOSURE 1
#define SR200PC20_VALUE_WHITEBALANCE 2
#define SR200PC20_VALUE_EFFECT 4

static struct yuv_ctrl sr200pc20_ctrl;
static exif_data_t sr200pc20_exif;

static int32_t streamon = 0;
//static int32_t recording = 0;
static int32_t resolution = MSM_SENSOR_RES_FULL;
static unsigned int value_mark_ev = 0;
static unsigned int value_mark_wb = 0;
static unsigned int value_mark_effect = 0;

int32_t sr200pc20_sensor_config(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp);
int32_t sr200pc20_sensor_native_control(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp);
int sr200pc20_get_exif_data(struct msm_sensor_ctrl_t *s_ctrl, void __user *argp);
void sr200pc20_get_exif(struct msm_sensor_ctrl_t *s_ctrl);
static int sr200pc20_exif_shutter_speed(struct msm_sensor_ctrl_t *s_ctrl);
static int sr200pc20_exif_iso(struct msm_sensor_ctrl_t *s_ctrl);

int32_t sr200pc20_sensor_match_id(struct msm_camera_i2c_client *sensor_i2c_client,
    struct msm_camera_slave_info *slave_info, const char *sensor_name)
{
    int32_t rc = 0;
    uint16_t chipid = 0;
    if (!sensor_i2c_client || !slave_info || !sensor_name) {
        pr_err("%s:%d failed: %p %p %p\n",__func__, __LINE__,
            sensor_i2c_client, slave_info,sensor_name);
        return -EINVAL;
    }
    CDBG("%s sensor_name =%s sid = 0x%X sensorid = 0x%X DATA TYPE = %d\n",
        __func__, sensor_name, sensor_i2c_client->cci_client->sid,
        slave_info->sensor_id, sensor_i2c_client->data_type);
    rc = sensor_i2c_client->i2c_func_tbl->i2c_read(sensor_i2c_client,
        slave_info->sensor_id_reg_addr, &chipid, sensor_i2c_client->data_type);
    if (chipid != slave_info->sensor_id) {
        pr_err("%s: chip id %x does not match read id: %x\n",
            __func__, chipid, slave_info->sensor_id);
    }
    return rc;
}

int32_t sr200pc20_set_exposure_compensation(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
	int32_t rc = 0;
	CDBG("E\n");
	CDBG("CAM-SETTING -- EV is %d", mode);
	switch (mode) {
	case CAMERA_EV_M4:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_brightness_M4);
		break;
	case CAMERA_EV_M3:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_brightness_M3);
		break;
	case CAMERA_EV_M2:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_brightness_M2);
		break;
	case CAMERA_EV_M1:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_brightness_M1);
		break;
	case CAMERA_EV_DEFAULT:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_brightness_default);
		break;
	case CAMERA_EV_P1:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_brightness_P1);
		break;
	case CAMERA_EV_P2:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_brightness_P2);
		break;
	case CAMERA_EV_P3:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_brightness_P3);
		break;
	case CAMERA_EV_P4:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_brightness_P4);
		break;
	default:
		pr_err("%s: Setting %d is invalid\n", __func__, mode);
		rc = 0;
	}
	return rc;
}

#if 0
int32_t sr200pc20_set_contrast(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
	int32_t rc = 0;
	CDBG("E\n");
	CDBG("Contrast is %d", mode);
	switch (mode) {
	case CAMERA_CONTRAST_LV0:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_contrast_M4);
		break;
	case CAMERA_CONTRAST_LV1:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_contrast_M3);
		break;
	case CAMERA_CONTRAST_LV2:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_contrast_M2);
		break;
	case CAMERA_CONTRAST_LV3:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_contrast_M1);
		break;
	case CAMERA_CONTRAST_LV4:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_contrast_default);
		break;
	case CAMERA_CONTRAST_LV5:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_contrast_P1);
		break;
	case CAMERA_CONTRAST_LV6:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_contrast_P2);
		break;
	case CAMERA_CONTRAST_LV7:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_contrast_P3);
		break;
	case CAMERA_CONTRAST_LV8:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_contrast_P4);
		break;
	default:
		pr_err("%s: Setting %d is invalid\n", __func__, mode);
		rc = 0;
	}
	return rc;
}
#endif

int32_t sr200pc20_set_white_balance(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
	int32_t rc = 0;
	CDBG("E\n");
	CDBG("WB is %d", mode);
	switch (mode) {
	case CAMERA_WHITE_BALANCE_OFF:
	case CAMERA_WHITE_BALANCE_AUTO:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_WB_Auto);
		break;
	case CAMERA_WHITE_BALANCE_INCANDESCENT:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_WB_Incandescent);
		break;
	case CAMERA_WHITE_BALANCE_FLUORESCENT:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_WB_Fluorescent);
		break;
	case CAMERA_WHITE_BALANCE_DAYLIGHT:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_WB_Daylight);
		break;
	case CAMERA_WHITE_BALANCE_CLOUDY_DAYLIGHT:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_WB_Cloudy);
		break;
	default:
		pr_err("%s: Setting %d is invalid\n", __func__, mode);
		rc = 0;
	}
	return rc;
}

int32_t sr200pc20_set_resolution(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
	int32_t rc = 0;
	CDBG("E\n");
	switch (mode) {
	case MSM_SENSOR_RES_FULL:
		CDBG("In case MSM_SENSOR_RES_FULL");
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_Capture);
		//to get exif data
		sr200pc20_get_exif(s_ctrl);
		break;
	case MSM_SENSOR_RES_QTR:
		CDBG("In case MSM_SENSOR_RES_QTR");
		if(sr200pc20_ctrl.op_mode == CAMERA_MODE_RECORDING)
		{
			rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20m_recording_800x600_24fps);	
		}
		else
		{
			rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_800x600_Preview);
		}
		rc=0;
		break;
#if 1
	case MSM_SENSOR_RES_2:
		CDBG("In case MSM_SENSOR_RES_2");
		if(sr200pc20_ctrl.op_mode == CAMERA_MODE_RECORDING)
		{
			rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20m_recording_640x480_24fps);	
		}
		else
		{
			rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_640x480_Preview);
		}
		rc=0;
		break;
#endif
	default:
		rc=0;
		break;
	}
	return rc;
}

int32_t sr200pc20_set_effect(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
	int32_t rc = 0;
	CDBG("E\n");
	switch (mode) {
	case CAMERA_EFFECT_OFF:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_Effect_Normal);
		break;
	case CAMERA_EFFECT_MONO:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_Effect_Gray);
		break;
	case CAMERA_EFFECT_NEGATIVE:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_Effect_Negative);
		break;
	case CAMERA_EFFECT_SEPIA:
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_Effect_Sepia);
		break;
	default:
		pr_err("%s: Setting %d is invalid\n", __func__, mode);
	}
	return rc;
}

int32_t sr200pc20_sensor_config(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp)
{
	struct sensorb_cfg_data *cdata = (struct sensorb_cfg_data *)argp;
	int32_t rc = 0;
	int32_t i = 0;
	mutex_lock(s_ctrl->msm_sensor_mutex);

	CDBG("ENTER \n ");

	switch (cdata->cfgtype) {
	case CFG_GET_SENSOR_INFO:
		CDBG("CFG_GET_SENSOR_INFO\n");
		memcpy(cdata->cfg.sensor_info.sensor_name,
			s_ctrl->sensordata->sensor_name,
			sizeof(cdata->cfg.sensor_info.sensor_name));
		cdata->cfg.sensor_info.session_id =
			s_ctrl->sensordata->sensor_info->session_id;
		for (i = 0; i < SUB_MODULE_MAX; i++)
			cdata->cfg.sensor_info.subdev_id[i] =
				s_ctrl->sensordata->sensor_info->subdev_id[i];
		CDBG("%s:%d sensor name %s\n", __func__, __LINE__,
			cdata->cfg.sensor_info.sensor_name);
		CDBG("%s:%d session id %d\n", __func__, __LINE__,
			cdata->cfg.sensor_info.session_id);
		for (i = 0; i < SUB_MODULE_MAX; i++)
			CDBG("%s:%d subdev_id[%d] %d\n", __func__, __LINE__, i,
				cdata->cfg.sensor_info.subdev_id[i]);
		break;
	case CFG_SET_INIT_SETTING:
		CDBG("CFG_SET_INIT_SETTING\n");
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_Init_Reg);
#if 0
		rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_stop_stream);
#endif
		break;
	case CFG_SET_RESOLUTION:
		resolution = *((int32_t  *)cdata->cfg.setting);
		CDBG("CFG_SET_RESOLUTION - res = %d\n" , resolution);
		CDBG("sr200pc20_ctrl.op_mode = %d\n " , sr200pc20_ctrl.op_mode);
		sr200pc20_set_resolution(s_ctrl , resolution );
		break;
	case CFG_SET_STOP_STREAM:
		CDBG("CFG_SET_STOP_STREAM - res = %d\n ", resolution);
		if(streamon == 1){
			CDBG("CFG_SET_STOP_STREAM *** res = %d - streamon == 1\n " , resolution);
#if 0
			CDBG(" CFG_SET_STOP_STREAM writing stop stream registers: sr200pc20_stop_stream \n");
			rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(
				s_ctrl->sensor_i2c_client, sr200pc20_stop_stream,
				ARRAY_SIZE(sr200pc20_stop_stream),
				MSM_CAMERA_I2C_BYTE_DATA);
			//rc = sr200pc20_WRT_LIST(s_ctrl,sr200pc20_stop_stream);
#endif
			rc=0;
			streamon = 0;
		}
		break;
	case CFG_SET_START_STREAM:
		CDBG("CFG_SET_START_STREAM\n");
		CDBG("sr200pc20_ctrl.op_mode = %d,sr200pc20_ctrl.prev_mode = %d \n", sr200pc20_ctrl.op_mode, sr200pc20_ctrl.prev_mode);
		if(sr200pc20_ctrl.op_mode != sr200pc20_ctrl.prev_mode)
		{
			sr200pc20_set_resolution(s_ctrl , resolution );
		}
		sr200pc20_set_effect( s_ctrl , sr200pc20_ctrl.settings.effect);
		sr200pc20_set_white_balance( s_ctrl, sr200pc20_ctrl.settings.wb);
		sr200pc20_set_exposure_compensation( s_ctrl , sr200pc20_ctrl.settings.exposure );
		streamon = 1;
		break;
	case CFG_SET_SLAVE_INFO: {
		struct msm_camera_sensor_slave_info sensor_slave_info;
		struct msm_camera_power_ctrl_t *p_ctrl;
		uint16_t size;
		int slave_index = 0;
		CDBG("CFG_SET_SLAVE_INFO\n");
		if (copy_from_user(&sensor_slave_info,
			(void *)cdata->cfg.setting,
				sizeof(sensor_slave_info))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		/* Update sensor slave address */
		if (sensor_slave_info.slave_addr) {
			s_ctrl->sensor_i2c_client->cci_client->sid =
				sensor_slave_info.slave_addr >> 1;
		}
		/* Update sensor address type */
		s_ctrl->sensor_i2c_client->addr_type =
			sensor_slave_info.addr_type;

		/* Update power up / down sequence */
		p_ctrl = &s_ctrl->sensordata->power_info;
		size = sensor_slave_info.power_setting_array.size;
		if (p_ctrl->power_setting_size < size) {
			struct msm_sensor_power_setting *tmp;
			tmp = kmalloc(sizeof(*tmp) * size, GFP_KERNEL);
			if (!tmp) {
				pr_err("%s: failed to alloc mem\n", __func__);
				rc = -ENOMEM;
				break;
			}
			kfree(p_ctrl->power_setting);
			p_ctrl->power_setting = tmp;
		}
		p_ctrl->power_setting_size = size;
		rc = copy_from_user(p_ctrl->power_setting, (void *)
			sensor_slave_info.power_setting_array.power_setting,
			size * sizeof(struct msm_sensor_power_setting));
		if (rc) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(sensor_slave_info.power_setting_array.
				power_setting);
			rc = -EFAULT;
			break;
		}
		CDBG("%s sensor id %x\n", __func__,
			sensor_slave_info.slave_addr);
		CDBG("%s sensor addr type %d\n", __func__,
			sensor_slave_info.addr_type);
		CDBG("%s sensor reg %x\n", __func__,
			sensor_slave_info.sensor_id_info.sensor_id_reg_addr);
		CDBG("%s sensor id %x\n", __func__,
			sensor_slave_info.sensor_id_info.sensor_id);
		for (slave_index = 0; slave_index <
			p_ctrl->power_setting_size; slave_index++) {
			CDBG("%s i %d power setting %d %d %ld %d\n", __func__,
				slave_index,
				p_ctrl->power_setting[slave_index].seq_type,
				p_ctrl->power_setting[slave_index].seq_val,
				p_ctrl->power_setting[slave_index].config_val,
				p_ctrl->power_setting[slave_index].delay);
		}
		break;
	}
	case CFG_WRITE_I2C_ARRAY: {
		struct msm_camera_i2c_reg_setting conf_array;
		struct msm_camera_i2c_reg_array *reg_setting = NULL;

		CDBG("CFG_WRITE_I2C_ARRAY\n");

		if (copy_from_user(&conf_array,
			(void *)cdata->cfg.setting,
			sizeof(struct msm_camera_i2c_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		reg_setting = kzalloc(conf_array.size *
			(sizeof(struct msm_camera_i2c_reg_array)), GFP_KERNEL);
		if (!reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(reg_setting, (void *)conf_array.reg_setting,
			conf_array.size *
			sizeof(struct msm_camera_i2c_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(reg_setting);
			rc = -EFAULT;
			break;
		}
		conf_array.reg_setting = reg_setting;
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write_table(
			s_ctrl->sensor_i2c_client, &conf_array);
		kfree(reg_setting);
		break;
	}
	case CFG_WRITE_I2C_SEQ_ARRAY: {
		struct msm_camera_i2c_seq_reg_setting conf_array;
		struct msm_camera_i2c_seq_reg_array *reg_setting = NULL;

		CDBG("CFG_WRITE_I2C_SEQ_ARRAY\n");

		if (copy_from_user(&conf_array,
			(void *)cdata->cfg.setting,
			sizeof(struct msm_camera_i2c_seq_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = kzalloc(conf_array.size *
			(sizeof(struct msm_camera_i2c_seq_reg_array)),
			GFP_KERNEL);
		if (!reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(reg_setting, (void *)conf_array.reg_setting,
			conf_array.size *
			sizeof(struct msm_camera_i2c_seq_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(reg_setting);
			rc = -EFAULT;
			break;
		}

		conf_array.reg_setting = reg_setting;
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_seq_table(s_ctrl->sensor_i2c_client,
			&conf_array);
		kfree(reg_setting);
		break;
	}
	case CFG_POWER_UP:
		CDBG("CFG_POWER_UP\n");
		streamon = 0;
		sr200pc20_ctrl.op_mode = CAMERA_MODE_INIT;
		sr200pc20_ctrl.prev_mode = CAMERA_MODE_INIT;
		sr200pc20_ctrl.settings.metering = CAMERA_CENTER_WEIGHT;
		sr200pc20_ctrl.settings.exposure = CAMERA_EV_DEFAULT;
		sr200pc20_ctrl.settings.wb = CAMERA_WHITE_BALANCE_AUTO;
		sr200pc20_ctrl.settings.iso = CAMERA_ISO_MODE_AUTO;
		sr200pc20_ctrl.settings.effect = CAMERA_EFFECT_OFF;
		sr200pc20_ctrl.settings.scenemode = CAMERA_SCENE_AUTO;
		if (s_ctrl->func_tbl->sensor_power_up) {
			CDBG("CFG_POWER_UP");
			rc = s_ctrl->func_tbl->sensor_power_up(s_ctrl,
				&s_ctrl->sensordata->power_info,
				s_ctrl->sensor_i2c_client,
				s_ctrl->sensordata->slave_info,
				s_ctrl->sensordata->sensor_name);
                } else
			rc = -EFAULT;
		break;
	case CFG_POWER_DOWN:
		CDBG("CFG_POWER_DOWN\n");
		if (s_ctrl->func_tbl->sensor_power_down) {
			CDBG("CFG_POWER_DOWN");
			rc = s_ctrl->func_tbl->sensor_power_down(
				s_ctrl,
				&s_ctrl->sensordata->power_info,
				s_ctrl->sensor_device_type,
				s_ctrl->sensor_i2c_client);
                } else
			rc = -EFAULT;
		break;
	case CFG_SET_STOP_STREAM_SETTING: {
		struct msm_camera_i2c_reg_setting *stop_setting =
			&s_ctrl->stop_setting;
		struct msm_camera_i2c_reg_array *reg_setting = NULL;

		CDBG("CFG_SET_STOP_STREAM_SETTING\n");

		if (copy_from_user(stop_setting, (void *)cdata->cfg.setting,
		    sizeof(struct msm_camera_i2c_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = stop_setting->reg_setting;
		stop_setting->reg_setting = kzalloc(stop_setting->size *
			(sizeof(struct msm_camera_i2c_reg_array)), GFP_KERNEL);
		if (!stop_setting->reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(stop_setting->reg_setting,
		    (void *)reg_setting, stop_setting->size *
		    sizeof(struct msm_camera_i2c_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(stop_setting->reg_setting);
			stop_setting->reg_setting = NULL;
			stop_setting->size = 0;
			rc = -EFAULT;
			break;
		}
		break;
	}
	default:
		break;
	}
	mutex_unlock(s_ctrl->msm_sensor_mutex);
	CDBG("EXIT \n ");
	return 0;
}

int32_t sr200pc20_sensor_native_control(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp)
{
	struct ioctl_native_cmd *cam_info = (struct ioctl_native_cmd *)argp;

	mutex_lock(s_ctrl->msm_sensor_mutex);

	CDBG("ENTER \n ");
	CDBG("cam_info values = %d : %d : %d : %d : %d\n", cam_info->mode, cam_info->address, cam_info->value_1, cam_info->value_2 , cam_info->value_3);
	/*Hal set more than 2 parms one time*/
	if ((cam_info->value_2 != 0 )&&
		(cam_info->value_2 != SR200PC20_VALUE_EXPOSURE )&&
		(cam_info->value_2 != SR200PC20_VALUE_WHITEBALANCE )&&
		(cam_info->value_2 != SR200PC20_VALUE_EFFECT )){
		if(cam_info->value_2 & SR200PC20_VALUE_EXPOSURE){
		sr200pc20_set_exposure_compensation(s_ctrl, value_mark_ev);
		sr200pc20_ctrl.settings.exposure=value_mark_ev;
		}
		 /*Hal set WB and Effect together */
		if ((cam_info->value_2 & (SR200PC20_VALUE_WHITEBALANCE | SR200PC20_VALUE_EFFECT)) == 6){
			sr200pc20_ctrl.settings.effect = cam_info->value_3;
			value_mark_effect = sr200pc20_ctrl.settings.effect;
			 sr200pc20_set_effect(s_ctrl, sr200pc20_ctrl.settings.effect);
			sr200pc20_ctrl.settings.wb = cam_info->value_1;
			value_mark_wb = sr200pc20_ctrl.settings.wb ;
			sr200pc20_set_white_balance(s_ctrl, sr200pc20_ctrl.settings.wb);
		}else{
			if(cam_info->value_2 & SR200PC20_VALUE_WHITEBALANCE){
				sr200pc20_set_white_balance(s_ctrl, value_mark_wb);
				sr200pc20_ctrl.settings.wb=value_mark_wb;
			}
			if(cam_info->value_2 & SR200PC20_VALUE_EFFECT){
				sr200pc20_set_effect(s_ctrl, value_mark_effect);
				sr200pc20_ctrl.settings.effect=value_mark_effect;
			}
		}
	}else{
		switch (cam_info->mode) {
		case EXT_CAM_EV:
			sr200pc20_ctrl.settings.exposure = cam_info->value_1;
			value_mark_ev = sr200pc20_ctrl.settings.exposure;
			sr200pc20_set_exposure_compensation(s_ctrl, sr200pc20_ctrl.settings.exposure);
			break;
		case EXT_CAM_WB:
			sr200pc20_ctrl.settings.wb = cam_info->value_1;
			value_mark_wb = sr200pc20_ctrl.settings.wb ;
			sr200pc20_set_white_balance(s_ctrl, sr200pc20_ctrl.settings.wb);
			break;
		case EXT_CAM_EFFECT:
			sr200pc20_ctrl.settings.effect = cam_info->value_1;
			value_mark_effect = sr200pc20_ctrl.settings.effect;
			sr200pc20_set_effect(s_ctrl, sr200pc20_ctrl.settings.effect);
			break;
		case EXT_CAM_SENSOR_MODE:
			sr200pc20_ctrl.prev_mode =  sr200pc20_ctrl.op_mode;
			sr200pc20_ctrl.op_mode = cam_info->value_1;
			break;
		case EXT_CAM_CONTRAST:
		#if 0
			sr200pc20_ctrl.settings.contrast = cam_info->value_1;
			sr200pc20_set_contrast(s_ctrl, sr200pc20_ctrl.settings.contrast);
		#endif
			break;
		default:
			break;
		}
	}
	mutex_unlock(s_ctrl->msm_sensor_mutex);
	CDBG("EXIT \n ");
	return 0;
}

void sr200pc20_set_default_settings(void)
{
	sr200pc20_ctrl.settings.metering = CAMERA_CENTER_WEIGHT;
	sr200pc20_ctrl.settings.exposure = CAMERA_EV_DEFAULT;
	sr200pc20_ctrl.settings.wb = CAMERA_WHITE_BALANCE_AUTO;
	sr200pc20_ctrl.settings.iso = CAMERA_ISO_MODE_AUTO;
	sr200pc20_ctrl.settings.effect = CAMERA_EFFECT_OFF;
	sr200pc20_ctrl.settings.scenemode = CAMERA_SCENE_AUTO;
}


int sr200pc20_get_exif_data(struct msm_sensor_ctrl_t *s_ctrl,
			void __user *argp)
{
	*((exif_data_t *)argp) = sr200pc20_exif;
	return 0;
}
void sr200pc20_get_exif(struct msm_sensor_ctrl_t *s_ctrl)
{
	CDBG("sr200pc20_get_exif: E");
	/*Exif data*/
	sr200pc20_exif_shutter_speed(s_ctrl);
	sr200pc20_exif_iso(s_ctrl);
	CDBG("exp_time : %d\niso_value : %d\n",sr200pc20_exif.shutterspeed, sr200pc20_exif.iso);
	return;
}

static int sr200pc20_exif_shutter_speed(struct msm_sensor_ctrl_t *s_ctrl)
{
	u16 read_value1 = 0;
	u16 read_value2 = 0;
	u16 read_value3 = 0;
	int OPCLK = 24000000;

	/* Exposure Time */
	s_ctrl->sensor_i2c_client->i2c_func_tbl->
		i2c_write(s_ctrl->sensor_i2c_client, 0x03,0x20,MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->
		i2c_read(s_ctrl->sensor_i2c_client, 0x80,&read_value1,MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->
		i2c_read(s_ctrl->sensor_i2c_client, 0x81,&read_value2,MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->
		i2c_read(s_ctrl->sensor_i2c_client, 0x82,&read_value3,MSM_CAMERA_I2C_BYTE_DATA);

	sr200pc20_exif.shutterspeed = OPCLK / ((read_value1 << 19)
		+ (read_value2 << 11) + (read_value3 << 3));
	CDBG("Exposure time = %d\n", sr200pc20_exif.shutterspeed);
	return 0;
}

static int sr200pc20_exif_iso(struct msm_sensor_ctrl_t *s_ctrl)
{
	u16 read_value4 = 0;
	u16 gain_value;

	/* ISO*/
	s_ctrl->sensor_i2c_client->i2c_func_tbl->
		i2c_write(s_ctrl->sensor_i2c_client, 0x03,0x20,MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->
		i2c_read(s_ctrl->sensor_i2c_client, 0xb0,&read_value4,MSM_CAMERA_I2C_BYTE_DATA);

	gain_value = (u16)(((read_value4 * 1000L)>>5) + 500);

	if (gain_value < 1140)
		sr200pc20_exif.iso = 50;
	else if (gain_value < 2140)
		sr200pc20_exif.iso = 100;
	else if (gain_value < 2640)
		sr200pc20_exif.iso = 200;
	else if (gain_value < 7520)
		sr200pc20_exif.iso = 400;
	else
		sr200pc20_exif.iso = 800;

	CDBG("ISO = %d\n", sr200pc20_exif.iso);
	return 0;
}