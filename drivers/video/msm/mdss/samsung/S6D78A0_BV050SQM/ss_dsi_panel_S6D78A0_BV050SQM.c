/*
 * =================================================================
 *
 *
 *	Description:  samsung display panel file
 *
 *	Author: jb09.kim
 *	Company:  Samsung Electronics
 *
 * ================================================================
 */
/*
<one line to give the program's name and a brief idea of what it does.>
Copyright (C) 2012, Samsung Electronics. All rights reserved.

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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
*/
#include "ss_dsi_panel_S6D78A0_BV050SQM.h"
static int prev_level = BLIC_DEFAULT_SCALED_LEVEL;
static DEFINE_SPINLOCK(bg_gpio_lock);

static void ktd3102_set_brightness(int scaled_level, struct mdss_dsi_ctrl_pdata *ctrl)
{
	int pulse;
	int tune_level = 0;
	unsigned long irq_flags;
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);
	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return;
	}

	tune_level = scaled_level;

	if(scaled_level == 0){
		gpio_set_value((vdd->dtsi_data[ctrl->ndx].backlight_tft_gpio[0]), 0);
		mdelay(3);
		gpio_set_value(BLIC_ENABLE_PIN, 0);
		prev_level = tune_level;
		pr_info("level = %d Backlight IC turned off\n",scaled_level);
		return;
	}

	if (unlikely(prev_level < 0)) {
		int val = gpio_get_value(vdd->dtsi_data[ctrl->ndx].backlight_tft_gpio[0]);
		if (val) {
			prev_level = 0;
			gpio_set_value(vdd->dtsi_data[ctrl->ndx].backlight_tft_gpio[0], 0);
			mdelay(3);
			pr_info("LCD Backlight init in boot time on kernel\n");
		}
	}
	if (!prev_level) {
		gpio_set_value(BLIC_ENABLE_PIN, 1);
		mdelay(1);
		gpio_set_value(vdd->dtsi_data[ctrl->ndx].backlight_tft_gpio[0], 1);
		udelay(100);
		prev_level = 1;
		pr_info("scaled_level = %d prev_level = %d\n",scaled_level,prev_level);
	}

	pulse = (tune_level - prev_level + MAX_BRIGHTNESS_IN_BLU)
					% MAX_BRIGHTNESS_IN_BLU;

	pr_info("%s prev_level = %d, tune_level = %d,  pulse = %d\n", __func__,prev_level,tune_level,pulse);

	spin_lock_irqsave(&bg_gpio_lock, irq_flags);
	for (; pulse > 0; pulse--) {

		gpio_set_value(vdd->dtsi_data[ctrl->ndx].backlight_tft_gpio[0], 0);
		udelay(2);
		gpio_set_value(vdd->dtsi_data[ctrl->ndx].backlight_tft_gpio[0], 1);
		udelay(2);
	}
	spin_unlock_irqrestore(&bg_gpio_lock, irq_flags);

	prev_level = tune_level;

}

static int mdss_panel_on_pre(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);

	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return false;
	}

	pr_info("%s %d\n", __func__, ctrl->ndx);

	mdss_panel_attach_set(ctrl, true);

	return true;
}

static int mdss_panel_on_post(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);

	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return false;
	}

	pr_info("%s %d\n", __func__, ctrl->ndx);

	return true;
}


static int mdss_panel_off_pre(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);

	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return false;
	}
	/* Disable BLIC  */
	ssreg_enable_blic(false);

	pr_info("%s %d\n", __func__, ctrl->ndx);
	return true;
}

static int mdss_panel_off_post(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);

	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return false;
	}

	pr_info("%s %d\n", __func__, ctrl->ndx);

	return true;
}

static void backlight_tft_late_on(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);

	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return;
	}

	if (mdss_panel_attached(ctrl->ndx))
		ssreg_enable_blic(true);
	else /* For PBA BOOTING */
		ssreg_enable_blic(false);
}

static int mdss_panel_revision(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);

	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return false;
	}

	vdd->panel_revision = 0;

	return true;
}
static void mdss_panel_tft_pwm_control(struct mdss_dsi_ctrl_pdata *ctrl, int level)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);
	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return;
	}

	if (level)
		vdd->bl_level = level;

	vdd->scaled_level = get_scaled_level(vdd, ctrl->ndx);
	pr_info("%s bl_level : %d scaled_level : %d\n", __func__, level, vdd->scaled_level);

	ktd3102_set_brightness(vdd->scaled_level, ctrl);

	return;
}
static void mdss_panel_tft_outdoormode_update(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);
	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return;
	}
	pr_info("%s: tft panel autobrightness update\n", __func__);

}
static void mdss_panel_init(struct samsung_display_driver_data *vdd)
{
	pr_info("%s : %s", __func__, vdd->panel_name);

	vdd->support_panel_max = S6D78A0_BV050SQM_SUPPORT_PANEL_COUNT;
	vdd->support_cabc = false;

	/* ON/OFF */
	vdd->panel_func.samsung_panel_on_pre = mdss_panel_on_pre;
	vdd->panel_func.samsung_panel_on_post = mdss_panel_on_post;
	vdd->panel_func.samsung_panel_off_pre = mdss_panel_off_pre;
	vdd->panel_func.samsung_panel_off_post = mdss_panel_off_post;
	vdd->panel_func.samsung_backlight_late_on = backlight_tft_late_on;

	/* DDI RX */
	vdd->panel_func.samsung_panel_revision = mdss_panel_revision;
	vdd->panel_func.samsung_manufacture_date_read = NULL;
	vdd->panel_func.samsung_ddi_id_read = NULL;
	vdd->panel_func.samsung_hbm_read = NULL;
	vdd->panel_func.samsung_mdnie_read = NULL;
	vdd->panel_func.samsung_smart_dimming_init = NULL;

	/* Brightness */
	vdd->panel_func.samsung_brightness_tft_pwm_ldi = NULL;
	vdd->panel_func.samsung_brightness_tft_pwm = mdss_panel_tft_pwm_control;
	vdd->panel_func.samsung_brightness_hbm_off = NULL;
	vdd->panel_func.samsung_brightness_aid = NULL;
	vdd->panel_func.samsung_brightness_acl_on = NULL;
	vdd->panel_func.samsung_brightness_acl_percent = NULL;
	vdd->panel_func.samsung_brightness_acl_off = NULL;
	vdd->panel_func.samsung_brightness_elvss = NULL;
	vdd->panel_func.samsung_brightness_elvss_temperature1 = NULL;
	vdd->panel_func.samsung_brightness_elvss_temperature2 = NULL;
	vdd->panel_func.samsung_brightness_vint = NULL;
	vdd->panel_func.samsung_brightness_gamma = NULL;

	vdd->brightness[0].brightness_packet_tx_cmds_dsi.link_state = DSI_HS_MODE;
	vdd->mdss_panel_tft_outdoormode_update=mdss_panel_tft_outdoormode_update;

	/* MDNIE */
	vdd->panel_func.samsung_mdnie_init = NULL;

	mdss_panel_attach_set(vdd->ctrl_dsi[DISPLAY_1], true);
}

static int __init samsung_panel_init(void)
{
	struct samsung_display_driver_data *vdd = samsung_get_vdd();
	char panel_string[] = "ss_dsi_panel_S6D78A0_BV050SQM_QHD";


	vdd->panel_name = mdss_mdp_panel + 8;
	pr_info("%s : %s\n", __func__, vdd->panel_name);

	if (!strncmp(vdd->panel_name, panel_string, strlen(panel_string)))
		vdd->panel_func.samsung_panel_init = mdss_panel_init;

	return 0;
}
early_initcall(samsung_panel_init);
