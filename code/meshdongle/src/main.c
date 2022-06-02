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

#include <usb/usb_device.h>
#include <drivers/uart.h>       //Include these libraries

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr.h>
#include <shell/shell.h>
#include <time.h>
#include <logging/log.h>

#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include <sys/util.h>
#include <inttypes.h>

#define PREAMBLE 0xAF
#define REQUEST 0xFF
#define RESET 0xFF
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

#define NODE_ADDR 0x0002

#define OP_ONOFF_GET       BT_MESH_MODEL_OP_2(0x82, 0x01)
#define OP_ONOFF_SET       BT_MESH_MODEL_OP_2(0x82, 0x02)
#define OP_ONOFF_SET_UNACK BT_MESH_MODEL_OP_2(0x82, 0x03)
#define OP_ONOFF_STATUS    BT_MESH_MODEL_OP_2(0x82, 0x04)

static int gen_onoff_send(uint8_t device);
static int gen_onoff_send2(uint8_t device);



//Message OP-Codes (7.1-Message Summary table - MeshModel) 
#define BT_MESH_MODEL_OP_SENSOR_GET BT_MESH_MODEL_OP_2(0x82, 0x31)
#define BT_MESH_MODEL_OP_SENSOR_STATUS BT_MESH_MODEL_OP_2(0x00, 0x52)
#define BT_MESH_MODEL_OP_SENSOR_COLUMN_GET BT_MESH_MODEL_OP_2(0x82, 0x32)
#define BT_MESH_MODEL_OP_SENSOR_COLUMN_STATUS BT_MESH_MODEL_OP_2(0x00, 0x53)
#define BT_MESH_MODEL_OP_SENSOR_SERIES_GET BT_MESH_MODEL_OP_2(0x82, 0x33)
#define BT_MESH_MODEL_OP_SENSOR_SERIES_STATUS BT_MESH_MODEL_OP_2(0x00, 0x54)

typedef struct communication_packet {
    uint8_t sensor;
	uint16_t time;
} communication_packet;

communication_packet data_recv;
uint8_t running = 0;

void print_func(void* argv) 
{    

    while(1) {
		
		while(running == 1) {
			gen_onoff_send(data_recv.sensor);
			k_msleep(data_recv.time);
		}
		k_msleep(1);
    }
}


K_THREAD_DEFINE(print_thread, 1024,
                print_func, NULL, NULL, NULL,
                2, 0, 0);


uint32_t currentTime = 0;

void timer_func(void* argv) 
{    
    // Increase time by 1 second
    while(1) {
        k_msleep(1);
        currentTime++;
    }
}

K_THREAD_DEFINE(timer_thread, 1024,
                timer_func, NULL, NULL, NULL,
                5, 0, 0);

void thread_cli(void);

K_THREAD_DEFINE(cli, 1024, thread_cli, NULL, NULL, NULL, 10, 0, 0);

LOG_MODULE_REGISTER(weather_station, LOG_LEVEL_DBG);

static int cmd_reset(const struct shell *, size_t, char **);
static int cmd_pressure(const struct shell *, size_t, char **);
static int cmd_humidity(const struct shell *, size_t, char **);
static int cmd_temperature(const struct shell *, size_t, char **);
static int cmd_voc(const struct shell *, size_t, char **);
static int cmd_eco2(const struct shell *, size_t, char **);
static int cmd_pm(const struct shell *, size_t, char **);
static int cmd_nox(const struct shell *, size_t, char **);
static int cmd_sync(const struct shell *, size_t, char **);



SHELL_CMD_REGISTER(reset, NULL, "reset devices", cmd_reset);
SHELL_CMD_REGISTER(pressure, NULL, "read pressure", cmd_pressure);
SHELL_CMD_REGISTER(humidity, NULL, "read humidity", cmd_humidity);
SHELL_CMD_REGISTER(temperature, NULL, "read temperature", cmd_temperature);
SHELL_CMD_REGISTER(voc, NULL, "read voc", cmd_voc);
SHELL_CMD_REGISTER(eco2, NULL, "read eco2", cmd_eco2);
SHELL_CMD_REGISTER(nox, NULL, "read nox", cmd_nox);
SHELL_CMD_REGISTER(pm, NULL, "read pm", cmd_pm);
SHELL_CMD_REGISTER(sync, NULL, "sync times", cmd_sync);


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

static const uint16_t net_idx = 0;
static const uint16_t app_idx = 0;
static const uint32_t iv_index;
static uint8_t flags;
static uint16_t addr = NODE_ADDR;

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

static const char *const onoff_str[] = { "off", "on" };

static struct {
	bool val;
	uint8_t tid;
	uint16_t src;
	uint32_t transition_time;
	struct k_work_delayable work;
} onoff;

/* OnOff messages' transition time and remaining time fields are encoded as an
 * 8 bit value with a 6 bit step field and a 2 bit resolution field.
 * The resolution field maps to:
 * 0: 100 ms
 * 1: 1 s
 * 2: 10 s
 * 3: 20 min
 */
static const uint32_t time_res[] = {
	100,
	MSEC_PER_SEC,
	10 * MSEC_PER_SEC,
	10 * 60 * MSEC_PER_SEC,
};

static inline int32_t model_time_decode(uint8_t val)
{
	uint8_t resolution = (val >> 6) & BIT_MASK(2);
	uint8_t steps = val & BIT_MASK(6);

	if (steps == 0x3f) {
		return SYS_FOREVER_MS;
	}

	return steps * time_res[resolution];
}

static inline uint8_t model_time_encode(int32_t ms)
{
	if (ms == SYS_FOREVER_MS) {
		return 0x3f;
	}

	for (int i = 0; i < ARRAY_SIZE(time_res); i++) {
		if (ms >= BIT_MASK(6) * time_res[i]) {
			continue;
		}

		uint8_t steps = ceiling_fraction(ms, time_res[i]);

		return steps | (i << 6);
	}

	return 0x3f;
}

static int onoff_status_send(struct bt_mesh_model *model,
			     struct bt_mesh_msg_ctx *ctx)
{
	printk("onoff_status_send()\n");
	uint32_t remaining;

	BT_MESH_MODEL_BUF_DEFINE(buf, OP_ONOFF_STATUS, 3);
	bt_mesh_model_msg_init(&buf, OP_ONOFF_STATUS);

	remaining = k_ticks_to_ms_floor32(
			    k_work_delayable_remaining_get(&onoff.work)) +
		    onoff.transition_time;

	/* Check using remaining time instead of "work pending" to make the
	 * onoff status send the right value on instant transitions. As the
	 * work item is executed in a lower priority than the mesh message
	 * handler, the work will be pending even on instant transitions.
	 */
	if (remaining) {
		net_buf_simple_add_u8(&buf, !onoff.val);
		net_buf_simple_add_u8(&buf, onoff.val);
		net_buf_simple_add_u8(&buf, model_time_encode(remaining));
	} else {
		net_buf_simple_add_u8(&buf, onoff.val);
	}

	return bt_mesh_model_send(model, ctx, &buf, NULL, NULL);
}

static void onoff_timeout(struct k_work *work)
{
	if (onoff.transition_time) {
		/* Start transition.
		 *
		 * The LED should be on as long as the transition is in
		 * progress, regardless of the target value, according to the
		 * Bluetooth Mesh Model specification, section 3.1.1.
		 */
		board_led_set(true);

		k_work_reschedule(&onoff.work, K_MSEC(onoff.transition_time));
		onoff.transition_time = 0;
		return;
	}

	board_led_set(onoff.val);
}

/* Generic OnOff Server message handlers */

static int gen_onoff_get(struct bt_mesh_model *model,
			 struct bt_mesh_msg_ctx *ctx,
			 struct net_buf_simple *buf)
{
	printk("gen_onoff_get()\n");
	onoff_status_send(model, ctx);
	return 0;
}

static int gen_onoff_set_unack(struct bt_mesh_model *model,
			       struct bt_mesh_msg_ctx *ctx,
			       struct net_buf_simple *buf)
{

	printk("gen_onoff_set_unack() %d %d\n", ctx->addr, addr);
	uint8_t tid = net_buf_simple_pull_u8(buf);
	uint8_t preamble = net_buf_simple_pull_u8(buf);
	uint8_t type = net_buf_simple_pull_u8(buf);
	uint8_t length = net_buf_simple_pull_u8(buf);
	uint32_t data = net_buf_simple_pull_le32(buf);
	
	
	// int32_t trans = 0;
	// int32_t delay = 0;

	// if (buf->len) {
	// 	trans = model_time_decode(net_buf_simple_pull_u8(buf));
	// 	delay = net_buf_simple_pull_u8(buf) * 5;
	// }

	/* Only perform change if the message wasn't a duplicate and the
	 * value is different.
	 */
	if (tid == onoff.tid) {
		/* Duplicate */
		return 0;
	}

	// if (val == onoff.val) {
	// 	/* No change */
	// 	return 0;
	// }

	// printk("set: %s delay: %d ms time: %d ms\n", onoff_str[val], delay,
	//        trans);

	onoff.tid = tid;
	onoff.src = ctx->addr;

	// /* Schedule the next action to happen on the delay, and keep
	//  * transition time stored, so it can be applied in the timeout.
	//  */
	// k_work_reschedule(&onoff.work, K_MSEC(delay));

	if (type != 0x0002) {
		//printk("%02x %02x %02x %02x %08x\n", tid, preamble, type, length, data); HARRISON COMMENTED
	}

	return 0;
}

static int gen_onoff_set(struct bt_mesh_model *model,
			 struct bt_mesh_msg_ctx *ctx,
			 struct net_buf_simple *buf)
{
	printk("gen_onoff_set()\n");
	(void)gen_onoff_set_unack(model, ctx, buf);
	onoff_status_send(model, ctx);

	return 0;
}

static const struct bt_mesh_model_op gen_onoff_srv_op[] = {
	{ OP_ONOFF_GET,       BT_MESH_LEN_EXACT(0), gen_onoff_get },
	{ OP_ONOFF_SET,       BT_MESH_LEN_MIN(2),   gen_onoff_set },
	{ OP_ONOFF_SET_UNACK, BT_MESH_LEN_MIN(2),   gen_onoff_set_unack },
	BT_MESH_MODEL_OP_END,
};

/* Generic OnOff Client */

static int cmd_reset(const struct shell *shell, size_t argc, char **argv) {
	ARG_UNUSED(argc);

	gen_onoff_send2(RESET);

	return 0;
}

static int cmd_pressure(const struct shell *shell, size_t argc, char **argv) {

	if (argc == 1) {
		gen_onoff_send(PRESSURE);
	} else{
		if (argv[1][2] == 'a') {
			running = 0;
			data_recv.sensor = PRESSURE;
			data_recv.time = atoi(argv[2]);
			running = 1;
		} else {
			running = 0;
		}
	}

	return 0;
}

static int cmd_humidity(const struct shell *shell, size_t argc, char **argv) {

	if (argc == 1) {
		gen_onoff_send(HUMIDITY);
	} else{
		if (argv[1][2] == 'a') {
			running = 0;
			data_recv.sensor = HUMIDITY;
			data_recv.time = atoi(argv[2]);
			running = 1;
		} else {
			running = 0;
		}
	}
	return 0;
}

static int cmd_temperature(const struct shell *shell, size_t argc, char **argv) {

	if (argc == 1) {
		gen_onoff_send(TEMPERATURE);
	} else{
		if (argv[1][2] == 'a') {
			running = 0;
			data_recv.sensor = TEMPERATURE;
			data_recv.time = atoi(argv[2]);
			running = 1;
		} else {
			running = 0;
		}
	}
	return 0;
}

static int cmd_voc(const struct shell *shell, size_t argc, char **argv) {

	if (argc == 1) {
		gen_onoff_send(VOC);
	} else{
		if (argv[1][2] == 'a') {
			running = 0;
			data_recv.sensor = VOC;
			data_recv.time = atoi(argv[2]);
			running = 1;
		} else {
			running = 0;
		}
	}
	return 0;
}

static int cmd_eco2(const struct shell *shell, size_t argc, char **argv) {

	if (argc == 1) {
		gen_onoff_send(ECO2);
	} else{
		if (argv[1][2] == 'a') {
			running = 0;
			data_recv.sensor = ECO2;
			data_recv.time = atoi(argv[2]);
			running = 1;
		} else {
			running = 0;
		}
	}
	return 0;
}

static int cmd_nox(const struct shell *shell, size_t argc, char **argv) {

	if (argc == 1) {
		gen_onoff_send(NOX);
	} else{
		if (argv[1][2] == 'a') {
			running = 0;
			data_recv.sensor = NOX;
			data_recv.time = atoi(argv[2]);
			running = 1;
		} else {
			running = 0;
		}
	}
	return 0;
}

static int cmd_pm(const struct shell *shell, size_t argc, char **argv) {
	ARG_UNUSED(argc);

	if (argc == 2) {

		if (argv[1][0] == '1') {
			gen_onoff_send(PM1_0);
		} else if (argv[1][0] == '2') {
			gen_onoff_send(PM2_5);
		} else if (argv[1][0] == '3') {
			gen_onoff_send(PM4_0);
		} else {
			gen_onoff_send(PM10_0);
		}
	} else {
		if (argv[2][2] == 'a') {
			running = 0;
			if (argv[1][0] == '1') {
				data_recv.sensor = PM1_0;
			} else if (argv[1][0] == '2') {
				data_recv.sensor = PM2_5;
			} else if (argv[1][0] == '3') {
				data_recv.sensor = PM4_0;
			} else {
				data_recv.sensor = PM10_0;
			}
			data_recv.time = atoi(argv[3]);
			running = 1;
		} else {
			running = 0;
		}
	}

	return 0;
}

static int cmd_sync(const struct shell *shell, size_t argc, char **argv) {
	ARG_UNUSED(argc);

	board_led_set(true);
	k_sleep(K_MSEC(50));
	board_led_set(false);

	currentTime = 0;
	return 0;
}

static int gen_onoff_status(struct bt_mesh_model *model,
			    struct bt_mesh_msg_ctx *ctx,
			    struct net_buf_simple *buf)
{
	//printk("gen_onoff_status()\n"); HARRISON COMMENTED

	//printk("Got "); HARRISON COMMENTED
	for (int i = 0; i < buf->len; i++) {
		//printk("%02x ", buf->data[i]); HARRISON COMMENTED
	}
	//printk("\n"); HARRISON COMMENTED
	
	uint8_t tid = net_buf_simple_pull_u8(buf);
	uint8_t random = net_buf_simple_pull_u8(buf);
	uint8_t address = net_buf_simple_pull_u8(buf);
	uint8_t preamble = net_buf_simple_pull_u8(buf);
	uint8_t type = net_buf_simple_pull_u8(buf);
	uint8_t length = net_buf_simple_pull_u8(buf);
	uint32_t time = net_buf_simple_pull_le32(buf);
	uint32_t data = net_buf_simple_pull_le32(buf);
	printk("%d %d %d %f\n", address, time, type, *((float *)(&data)));
	//printk("Node: %d\tTime: %d\t Sensor: %d\tData: %f\n", address, time, type, *((float *)(&data))); HARRISON COMMENTED

	//printk("%02x %02x %02x %02x %02x %08x\n", tid, address, preamble, type, length, data);
	if (tid == onoff.tid && address != ctx->addr) {
		/* Duplicate */
		return 0;
	}

	onoff.tid = tid;
	onoff.src = ctx->addr;

	//printk("no dupe %02x %02x %02x %02x %02x %08x\n", tid, address, preamble, type, length, data);

	return 0;
}

static const struct bt_mesh_model_op gen_onoff_cli_op[] = {
	{OP_ONOFF_STATUS, BT_MESH_LEN_MIN(1), gen_onoff_status},
	BT_MESH_MODEL_OP_END,
};

/* This application only needs one element to contain its models */
static struct bt_mesh_model models[] = {
	BT_MESH_MODEL_CFG_SRV,
	BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
	BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_SRV, gen_onoff_srv_op, NULL,
		      NULL),
	BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_CLI, gen_onoff_cli_op, NULL,
		      NULL),
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

static int output_number(bt_mesh_output_action_t action, uint32_t number)
{
	printk("OOB Number: %u\n", number);

	board_output_number(action, number);

	return 0;
}

// static void prov_complete(uint16_t net_idx, uint16_t addr)
// {
// 	board_prov_complete();
// }

// static void prov_reset(void)
// {
// 	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);
// }

static uint8_t dev_uuid[16];

static const struct bt_mesh_prov prov = {
	.uuid = dev_uuid,
	// .output_size = 4,
	// .output_actions = BT_MESH_DISPLAY_NUMBER,
	// .output_number = output_number,
	// .complete = prov_complete,
	// .reset = prov_reset,
};

/** Send an OnOff Set message from the Generic OnOff Client to all nodes. */
static int gen_onoff_send(uint8_t device)
{
	//printk("gen_onoff_send()\n"); HARRISON COMMENTED
	struct bt_mesh_msg_ctx ctx = {
		.app_idx = models[3].keys[0], /* Use the bound key */
		.addr = BT_MESH_ADDR_ALL_NODES,
		.send_ttl = BT_MESH_TTL_DEFAULT,
	};
	static uint8_t tid;

	if (ctx.app_idx == BT_MESH_KEY_UNUSED) {
		printk("The Generic OnOff Client must be bound to a key before "
		       "sending.\n");
		return -ENOENT;
	}

	board_led_set(true);
	k_sleep(K_MSEC(50));
	board_led_set(false);

	BT_MESH_MODEL_BUF_DEFINE(buf, BT_MESH_MODEL_OP_SENSOR_GET, 3 + 3 + 4 + 1);
	bt_mesh_model_msg_init(&buf, BT_MESH_MODEL_OP_SENSOR_GET);
	net_buf_simple_add_u8(&buf, tid++);
	net_buf_simple_add_u8(&buf, RANDOM_8);
	net_buf_simple_add_u8(&buf, NODE_ADDR);
	net_buf_simple_add_u8(&buf, PREAMBLE);
	net_buf_simple_add_u8(&buf, REQUEST);
	net_buf_simple_add_u8(&buf, 5);
	net_buf_simple_add_le32(&buf, currentTime);
	net_buf_simple_add_u8(&buf, device);

	
	//printk("Sending "); HARRISON COMMENTED
	for (int i = 0; i < buf.len; i++) {
		//printk("%02x ", buf.data[i]); HARRISON COMMENTED
	}
	//printk("\n"); HARRISON COMMENTED

	return bt_mesh_model_send(&models[3], &ctx, &buf, NULL, NULL);
}

static int gen_onoff_send2(uint8_t device)
{
	//printk("gen_onoff_send()\n"); HARRISON COMMENTED
	struct bt_mesh_msg_ctx ctx = {
		.app_idx = models[3].keys[0], /* Use the bound key */
		.addr = BT_MESH_ADDR_ALL_NODES,
		.send_ttl = BT_MESH_TTL_DEFAULT,
	};
	static uint8_t tid;

	if (ctx.app_idx == BT_MESH_KEY_UNUSED) {
		printk("The Generic OnOff Client must be bound to a key before "
		       "sending.\n");
		return -ENOENT;
	}

	BT_MESH_MODEL_BUF_DEFINE(buf, BT_MESH_MODEL_OP_SENSOR_STATUS, 3 + 3 + 4 + 1);
	bt_mesh_model_msg_init(&buf, BT_MESH_MODEL_OP_SENSOR_STATUS);
	net_buf_simple_add_u8(&buf, tid++);
	net_buf_simple_add_u8(&buf, RANDOM_8);
	net_buf_simple_add_u8(&buf, NODE_ADDR);
	net_buf_simple_add_u8(&buf, PREAMBLE);
	net_buf_simple_add_u8(&buf, REQUEST);
	net_buf_simple_add_u8(&buf, 5);
	net_buf_simple_add_le32(&buf, currentTime);
	net_buf_simple_add_u8(&buf, device);

	
	//printk("Sending "); HARRISON COMMENTED
	for (int i = 0; i < buf.len; i++) {
		//printk("%02x ", buf.data[i]); HARRISON COMMENTED
	}
	//printk("\n"); HARRISON COMMENTED

	return bt_mesh_model_send(&models[3], &ctx, &buf, NULL, NULL);
}

static void configure(void)
{
	int err;
	//printk("Configuring...\n"); HARRISON COMMENTED

	/* Add Application Key */
	// err = bt_mesh_cfg_app_key_add(net_idx, addr, net_idx, app_idx, app_key, NULL);
	// if (err) {
	// 	printk("Cfg App key add failed (err: %d)\n", err);
	// 	return;
	// }

	/* Add an application key to both Generic OnOff models: */
	err = bt_mesh_app_key_add(app_idx, net_idx, app_key);
	if (err) {
		printk("Sensor key add failed (err: %d)\n", err);
		return;
	}

	/* Models must be bound to an app key to send and receive messages with
	 * it:
	 */
	models[2].keys[0] = app_idx;
	models[3].keys[0] = app_idx;

	//printk("Provisioned and configured!\n"); HARRISON COMMENTED

	//printk("Configuration complete\n"); HARRISON COMMENTED
}

static void button_pressed(struct k_work *work)
{
	if (bt_mesh_is_provisioned()) {
		(void)gen_onoff_send(2);
		return;
	}

	/* Self-provision with an arbitrary address.
	 *
	 * NOTE: This should never be done in a production environment.
	 *       Addresses should be assigned by a provisioner, and keys should
	 *       be generated from true random numbers. It is done in this
	 *       sample to allow testing without a provisioner.
	 */
	/*
	static uint8_t net_key[16];
	static uint8_t dev_key[16];
	static uint8_t app_key[16];
	uint16_t addr;
	int err;

	if (IS_ENABLED(CONFIG_HWINFO)) {
		addr = sys_get_le16(&dev_uuid[0]) & BIT_MASK(15);
	} else {
		addr = k_uptime_get_32() & BIT_MASK(15);
	}

	printk("Self-provisioning with address 0x%04x\n", addr);
	err = bt_mesh_provision(net_key, 0, 0, 0, addr, dev_key);
	if (err) {
		printk("Provisioning failed (err: %d)\n", err);
		return;
	}
	*/

	/* Add an application key to both Generic OnOff models: */
	/*
	err = bt_mesh_app_key_add(0, 0, app_key);
	if (err) {
		printk("App key add failed (err: %d)\n", err);
		return;
	}
	*/

	/* Models must be bound to an app key to send and receive messages with
	 * it:
	 */
	/*
	models[2].keys[0] = 0;
	models[3].keys[0] = 0;

	printk("Provisioned and configured!\n");
	*/
}

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	//printk("Bluetooth initialized\n"); HARRISON COMMENTED

	err = bt_mesh_init(&prov, &comp);
	if (err) {
		printk("Initializing mesh failed (err %d)\n", err);
		return;
	}

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		//printk("Loading stored settings\n"); HARRISON COMMENTED
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
		//printk("Provisioning completed\n"); HARRISON COMMENTED
		configure();
	}

	/* This will be a no-op if settings_load() loaded provisioning info */
	//bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

	//printk("Mesh initialized\n"); HARRISON COMMENTED
}

void thread_cli(void) {
	/* Setup DTR */
	const struct device *shell_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));
	uint32_t dtr = 0;
	
	while (!dtr) {
		uart_line_ctrl_get(shell_dev, UART_LINE_CTRL_DTR, &dtr);
		k_sleep(K_MSEC(100));
	} 
}

void main(void)
{
	/* Enable the USB Driver */
    if (usb_enable(NULL)){
        return;
	}

	static struct k_work button_work;
	int err = -1;

	//printk("Initializing...\n"); HARRISON COMMENTED

	if (IS_ENABLED(CONFIG_HWINFO)) {
		err = hwinfo_get_device_id(dev_uuid, sizeof(dev_uuid));
	}

	if (err < 0) {
		dev_uuid[0] = 0xdd;
		dev_uuid[1] = 0xdd;
	}

	k_work_init(&button_work, button_pressed);

	err = board_init(&button_work);
	if (err) {
		printk("Board init failed (err: %d)\n", err);
		return;
	}

	k_work_init_delayable(&onoff.work, onoff_timeout);

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}
	int device = 1;
	
}