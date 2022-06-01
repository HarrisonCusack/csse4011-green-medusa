/* main.c - Application main entry point */

/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/printk.h>

#include <settings/settings.h>
#include <devicetree.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/hwinfo.h>
#include <sys/byteorder.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh.h>

#include "board.h"
#include "scu_sensors.h"

#define MY_STACK_SIZE 1024
#define MY_PRIORITY 5

// Time for sleep mode
uint32_t time = 0;

/**
 * @brief timer that increase every second
 * 
 * @param argv thread arguments
**/
void timer_func(void* argv) 
{    
    // Increase time by 1 second
    while(1) {
        k_msleep(1);
        time++;
    }
}

K_THREAD_DEFINE(timer_thread, MY_STACK_SIZE,
                timer_func, NULL, NULL, NULL,
                MY_PRIORITY, 0, 0);

#define PREAMBLE 0xAF
#define REQUEST 0xFF
#define PRESSURE 0x01
#define ECO2 0x02
#define TEMPERATURE 0x03
#define HUMIDITY 0x04
#define VOC 0x05
#define PM1_0 0x06
#define PM2_5 0x07
#define PM4_0 0x08
#define PM10_0 0x09
#define NOX 0x0a
#define RANDOM_8 (k_uptime_get_32() & 0xFF)

#define NODE_ADDR 0x0003

#define OP_ONOFF_GET       BT_MESH_MODEL_OP_2(0x82, 0x01)
#define OP_ONOFF_SET       BT_MESH_MODEL_OP_2(0x82, 0x02)
#define OP_ONOFF_SET_UNACK BT_MESH_MODEL_OP_2(0x82, 0x03)
#define OP_ONOFF_STATUS    BT_MESH_MODEL_OP_2(0x82, 0x04)

//Message OP-Codes (7.1-Message Summary table - MeshModel) 
#define BT_MESH_MODEL_OP_SENSOR_GET BT_MESH_MODEL_OP_2(0x82, 0x31)
#define BT_MESH_MODEL_OP_SENSOR_STATUS BT_MESH_MODEL_OP_2(0x00, 0x52)
#define BT_MESH_MODEL_OP_SENSOR_COLUMN_GET BT_MESH_MODEL_OP_2(0x82, 0x32)
#define BT_MESH_MODEL_OP_SENSOR_COLUMN_STATUS BT_MESH_MODEL_OP_2(0x00, 0x53)
#define BT_MESH_MODEL_OP_SENSOR_SERIES_GET BT_MESH_MODEL_OP_2(0x82, 0x33)
#define BT_MESH_MODEL_OP_SENSOR_SERIES_STATUS BT_MESH_MODEL_OP_2(0x00, 0x54)

static const uint8_t net_key[16] = {
	0xb2, 0xf1, 0xc5, 0x33, 0xeb, 0x04, 0x82, 0x35,
	0xa2, 0xe3, 0x71, 0x5c, 0xe0, 0x68, 0xf0, 0x7e
};
static const uint8_t dev_key[16] = {
	0xbd, 0x89, 0xf0, 0xfb, 0x05, 0xcb, 0xe7, 0x45, 
	0xce, 0xa7, 0xf2, 0x30, 0x20, 0x73, 0x77, 0xe3, 
};
static const uint8_t app_key[16] = {
	0xd1, 0xe8, 0xda, 0x61, 0x16, 0x60, 0x42, 0x06, 
	0x74, 0x14, 0xbd, 0x49, 0xed, 0x6d, 0x05, 0xcb, 
};

static const uint16_t net_idx;
static const uint16_t app_idx;
static const uint32_t iv_index;
static uint8_t flags;
static uint16_t addr = NODE_ADDR;

static int send_sensor_data(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, uint8_t device, float data);

static void attention_on(struct bt_mesh_model *mod)
{
	board_led_set(true);
}

static void attention_off(struct bt_mesh_model *mod)
{
	board_led_set(false);
}

static const struct bt_mesh_health_srv_cb health_cb = {
	.attn_on = attention_on,
	.attn_off = attention_off,
};

static struct bt_mesh_health_srv health_srv = {
	.cb = &health_cb,
};

BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);

static struct {
	uint8_t tid;
	uint16_t src;
} onoff;

void sensor_get(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct net_buf_simple *buf)
{
	printk("sensor_get()\n");

	printk("Got ");
	for (int i = 0; i < buf->len; i++) {
		printk("%02x ", buf->data[i]);
	}
	printk("\n");
	
	uint8_t tid = net_buf_simple_pull_u8(buf);
	uint8_t random = net_buf_simple_pull_u8(buf);
	uint8_t address = net_buf_simple_pull_u8(buf);
	uint8_t preamble = net_buf_simple_pull_u8(buf);
	uint8_t type = net_buf_simple_pull_u8(buf);
	uint8_t length = net_buf_simple_pull_u8(buf);
	time = net_buf_simple_pull_le32(buf);
	uint8_t device = net_buf_simple_pull_u8(buf);
	printk("%02x %02x\n", onoff.src, ctx->addr);
	if (tid == onoff.tid && onoff.src == ctx->addr) {
		/* Duplicate */
		return 0;
	}
	board_led_set(true);
	k_sleep(K_MSEC(50));
	board_led_set(false);
	//printk("%02x %02x %02x %02x %02x %08x\n", tid, address, preamble, type, length, data);

	if (preamble == PREAMBLE) {
		printk("correct preamble\n");
		// Checks which device id was receieved
		switch(device) {
			case 1:
				send_sensor_data(model, ctx, device, scu_lps22hb_read());
				break;
			case 2:
				send_sensor_data(model, ctx, device, scu_ccs811_read_eco2());
				break;
			case 3:
				send_sensor_data(model, ctx, device, scu_hts221_read_temp());
				break;
			case 4:
				send_sensor_data(model, ctx, device, scu_hts221_read_hum());
				break;
			case 5:
				send_sensor_data(model, ctx, device, scu_ccs811_read_voc());
				break;
			
		}
	}

	onoff.tid = tid;
	onoff.src = ctx->addr;
}

void sensor_status(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct net_buf_simple *buf)
{
	printk("sensor_status\n");
}

void sensor_column_get(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct net_buf_simple *buf)
{
	printk("sensor_column_get\n");
}

void sensor_column_status(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct net_buf_simple *buf)
{
	printk("sensor_column_status\n");
}

void sensor_series_get(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct net_buf_simple *buf)
{
	printk("sensor_series_get\n");
}

void sensor_series_status(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct net_buf_simple *buf)
{
	printk("sensor_series_status\n");
}

//4.2 Sensor messages
const struct bt_mesh_model_op sensor_srv_op[] = {
	{BT_MESH_MODEL_OP_SENSOR_GET, 0, sensor_get},
	{BT_MESH_MODEL_OP_SENSOR_STATUS, 0, sensor_status},
	{BT_MESH_MODEL_OP_SENSOR_COLUMN_GET, 2, sensor_column_get},
	{BT_MESH_MODEL_OP_SENSOR_COLUMN_STATUS, 2, sensor_column_status},
	{BT_MESH_MODEL_OP_SENSOR_SERIES_GET, 2, sensor_series_get},
	{BT_MESH_MODEL_OP_SENSOR_SERIES_STATUS, 2, sensor_series_status},
	BT_MESH_MODEL_OP_END,
};

static struct bt_mesh_model models[] = {
	BT_MESH_MODEL_CFG_SRV,
	BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
	BT_MESH_MODEL(BT_MESH_MODEL_ID_SENSOR_SRV, sensor_srv_op, NULL, NULL),
};

static struct bt_mesh_elem elements[] = {
	BT_MESH_ELEM(0, models, BT_MESH_MODEL_NONE),
};

static const struct bt_mesh_comp comp = {
	.cid = BT_COMP_ID_LF,
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

/* Provisioning */

static uint8_t dev_uuid[16];

static const struct bt_mesh_prov prov = {
	.uuid = dev_uuid,
	// .output_size = 4,
	// .output_actions = BT_MESH_DISPLAY_NUMBER,
	// .output_number = output_number,
	// .complete = prov_complete,
	// .reset = prov_reset,
};

static void configure(void)
{
	int err;
	printk("Configuring...\n");

	/* Add Application Key */
	// err = bt_mesh_cfg_app_key_add(net_idx, addr, net_idx, app_idx, app_key, NULL);
	// if (err) {
	// 	printk("Cfg App key add failed (err: %d)\n", err);
	// 	return;
	// }

	/* Add an application key to both Generic OnOff models: */

	err = bt_mesh_app_key_add(app_idx, net_idx, app_key);
	if (err) {
		printk("Base key add failed (err: %d)\n", err);
		return;
	}

	/* Models must be bound to an app key to send and receive messages with
	 * it:
	 */
	models[2].keys[0] = app_idx;

	printk("Provisioned and configured!\n");

	printk("Configuration complete\n");
}

static int send_sensor_data(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, uint8_t device, float data)
{
	printk("send_sensor_data()\n");
	if (bt_mesh_is_provisioned()) {
		// struct bt_mesh_msg_ctx ctx = {
		// 	.app_idx = models[2].keys[0], /* Use the bound key */
		// 	.addr = BT_MESH_ADDR_ALL_NODES,
		// 	.send_ttl = BT_MESH_TTL_DEFAULT,
		// };

		static uint8_t tid;

		// if (ctx.app_idx == BT_MESH_KEY_UNUSED) {
		// 	printk("The Generic OnOff Client must be bound to a key before "
		// 		"sending.\n");
		// 	return -ENOENT;
		// }

		BT_MESH_MODEL_BUF_DEFINE(buf, OP_ONOFF_STATUS, 3 + 3 + 4 + 4);
		bt_mesh_model_msg_init(&buf, OP_ONOFF_STATUS);
		net_buf_simple_add_u8(&buf, tid++);
		net_buf_simple_add_u8(&buf, RANDOM_8);
		net_buf_simple_add_u8(&buf, NODE_ADDR);
		net_buf_simple_add_u8(&buf, PREAMBLE);
		net_buf_simple_add_u8(&buf, device);
		net_buf_simple_add_u8(&buf, 4);
		net_buf_simple_add_le32(&buf, time);
		net_buf_simple_add_le32(&buf, *((uint32_t*)(&data)));

		printk("Sending ");
		for (int i = 0; i < buf.len; i++) {
			printk("%02x ", buf.data[i]);
		}
		printk("\n");
		k_sleep(K_MSEC(100));
		board_led_set(true);
		k_sleep(K_MSEC(50));
		board_led_set(false);

		return bt_mesh_model_send(model, ctx, &buf, NULL, NULL);
	}
}

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	err = bt_mesh_init(&prov, &comp);
	if (err) {
		printk("Initializing mesh failed (err %d)\n", err);
		return;
	}

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		printk("Loading stored settings\n");
		settings_load();
	}

	err = bt_mesh_provision(net_key, net_idx, flags, iv_index, addr,
				dev_key);
	if (err == -EALREADY) {
		printk("Using stored settings\n");
	} else if (err) {
		printk("Provisioning failed (err %d)\n", err);
		return;
	} else {
		printk("Provisioning completed\n");
		configure();
	}

	/* This will be a no-op if settings_load() loaded provisioning info */
	//bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

	printk("Mesh initialized\n");
}

void sensors_init (void) 
{
    scu_ccs811_init();
    scu_hts221_init();
    scu_lis2dh_init();
    scu_lps22hb_init();
}    

void main(void)
{
	int err = -1;

	printk("Initializing...\n");

	sensors_init();

	if (IS_ENABLED(CONFIG_HWINFO)) {
		err = hwinfo_get_device_id(dev_uuid, sizeof(dev_uuid));
	}

	if (err < 0) {
		dev_uuid[0] = 0xdd;
		dev_uuid[1] = 0xdd;
	}

	err = board_init();
	if (err) {
		printk("Board init failed (err: %d)\n", err);
	}

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}
}
