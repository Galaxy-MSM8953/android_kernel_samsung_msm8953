/*
 * lm3632a-panel_power.c - Platform data for lm3632a panel_power driver
 *
 * Copyright (C) 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/kernel.h>
#include <asm/unaligned.h>
#include <linux/input/mt.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/regulator/consumer.h>
#include <linux/device.h>
#include <linux/of_gpio.h>

#include "../ss_dsi_panel_common.h"


struct lm3632a_panel_power_platform_data {
	int gpio_en;
	u32 gpio_en_flags;

	int gpio_enn;
	u32 gpio_enn_flags;

	int gpio_enp;
	u32 gpio_enp_flags;
};

struct lm3632a_panel_power_info {
	struct i2c_client			*client;
	struct lm3632a_panel_power_platform_data	*pdata;
};

static struct lm3632a_panel_power_info *pinfo;

static int panel_power_i2c_write(struct i2c_client *client,
		u8 reg,  u8 val, unsigned int len)
{
	int err = 0;
	int retry = 3;
	u8 temp_val = val;

	while (retry--) {
		err = i2c_smbus_write_i2c_block_data(client,
				reg, len, &temp_val);
		if (err >= 0)
			return err;

		LCD_ERR("i2c transfer error. %d\n", err);
	}
	return err;
}

static void panel_power_request_gpio(struct lm3632a_panel_power_platform_data *pdata)
{
	int ret;

	if (gpio_is_valid(pdata->gpio_en)) {
		ret = gpio_request(pdata->gpio_en, "gpio_en");
		if (ret) {
			LCD_ERR("unable to request gpio_en [%d]\n", pdata->gpio_en);
			return;
		}
	}

	if (gpio_is_valid(pdata->gpio_enn)) {
		ret = gpio_request(pdata->gpio_enn, "gpio_enn");
		if (ret) {
			LCD_ERR("unable to request gpio_enn [%d]\n", pdata->gpio_enn);
			return;
		}
	}

	if (gpio_is_valid(pdata->gpio_enp)) {
		ret = gpio_request(pdata->gpio_enp, "gpio_enp");
		if (ret) {
			LCD_ERR("unable to request gpio_enp [%d]\n", pdata->gpio_enp);
			return;
		}
	}
}

static int lm3632a_panel_power_parse_dt(struct device *dev,
			struct lm3632a_panel_power_platform_data *pdata)
{
	struct device_node *np = dev->of_node;

	/* enable, enn, enp */
	pdata->gpio_en = of_get_named_gpio_flags(np, "lm3632a_en_gpio",
				0, &pdata->gpio_en_flags);
	pdata->gpio_enn = of_get_named_gpio_flags(np, "lm3632a_enn_gpio",
				0, &pdata->gpio_enn_flags);
	pdata->gpio_enp = of_get_named_gpio_flags(np, "lm3632a_enp_gpio",
				0, &pdata->gpio_enp_flags);

	LCD_ERR("gpio_en : %d gpio_enn: %d gpio_enp: %d\n",
		pdata->gpio_en,pdata->gpio_enn,pdata->gpio_enp);
	return 0;
}

int lm3632a_panel_power(struct mdss_dsi_ctrl_pdata *ctrl, int enable)
{
	struct lm3632a_panel_power_info *info = pinfo;

	if (!info) {
		LCD_ERR("error pinfo\n");
		return 0;
	}

	if (enable) {
		gpio_set_value(info->pdata->gpio_en, 1);
		msleep(5);
		panel_power_i2c_write(info->client, 0x09, 0x41, 1);
		panel_power_i2c_write(info->client, 0x02, 0x50, 1);
		panel_power_i2c_write(info->client, 0x03, 0x8D, 1);
		panel_power_i2c_write(info->client, 0x04, 0x07, 1);
		panel_power_i2c_write(info->client, 0x05, 0xFF, 1);
		panel_power_i2c_write(info->client, 0x0A, 0x19, 1);
		panel_power_i2c_write(info->client, 0x0D, 0x1C, 1);
		panel_power_i2c_write(info->client, 0x0E, 0x1E, 1);
		panel_power_i2c_write(info->client, 0x0F, 0x1E, 1);
		panel_power_i2c_write(info->client, 0x0C, 0x1F, 1);
		panel_power_i2c_write(info->client, 0x11, 0x0D, 1);

		gpio_set_value(info->pdata->gpio_enp, 1);
		msleep(5);
		gpio_set_value(info->pdata->gpio_enn, 1);


	} else {
		gpio_set_value(info->pdata->gpio_enn, 0);
		msleep(5);
		gpio_set_value(info->pdata->gpio_enp, 0);
		gpio_set_value(info->pdata->gpio_en, 0);
		msleep(5);

	}
	LCD_INFO(" enable : %d \n", enable);

	return 0;
}

static int lm3632a_panel_power_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct lm3632a_panel_power_platform_data *pdata;
	struct lm3632a_panel_power_info *info;

	int error = 0;

	LCD_INFO("\n");

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -EIO;

	if (client->dev.of_node) {
		pdata = devm_kzalloc(&client->dev,
			sizeof(struct lm3632a_panel_power_platform_data),
				GFP_KERNEL);
		if (!pdata) {
			LCD_INFO("Failed to allocate memory\n");
			return -ENOMEM;
		}

		error = lm3632a_panel_power_parse_dt(&client->dev, pdata);
		if (error)
			return error;
	} else
		pdata = client->dev.platform_data;

	panel_power_request_gpio(pdata);

	pinfo = info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info) {
		LCD_INFO("Failed to allocate memory\n");
		return -ENOMEM;
	}

	info->client = client;
	info->pdata = pdata;

	i2c_set_clientdata(client, info);

	return error;
}

static int lm3632a_panel_power_remove(struct i2c_client *client)
{
	return 0;
}

static const struct i2c_device_id lm3632a_panel_power_id[] = {
	{"lm3632a_panel_power", 0},
};
MODULE_DEVICE_TABLE(i2c, lm3632a_panel_power_id);

static struct of_device_id lm3632a_panel_power_match_table[] = {
	{ .compatible = "lm3632a,panel_power-control",},
};

MODULE_DEVICE_TABLE(of, lm3632a_panel_power_id);

struct i2c_driver lm3632a_panel_power_driver = {
	.probe = lm3632a_panel_power_probe,
	.remove = lm3632a_panel_power_remove,
	.driver = {
		.name = "lm3632a_panel_power",
		.owner = THIS_MODULE,
		.of_match_table = lm3632a_panel_power_match_table,
		   },
	.id_table = lm3632a_panel_power_id,
};

static int __init lm3632a_panel_power_init(void)
{

	int ret = 0;

	ret = i2c_add_driver(&lm3632a_panel_power_driver);
	if (ret) {
		LCD_ERR("lm3632a_panel_power_init registration failed. ret= %d\n",
			ret);
	}

	return ret;
}

static void __exit lm3632a_panel_power_exit(void)
{
	i2c_del_driver(&lm3632a_panel_power_driver);
}

module_init(lm3632a_panel_power_init);
module_exit(lm3632a_panel_power_exit);

MODULE_DESCRIPTION("lm3632a panel_power driver");
MODULE_LICENSE("GPL");
