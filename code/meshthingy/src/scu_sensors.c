/*
***************************************************************
* @file oslib/scu_drivers/scu_sensors/scu_sensors.c
* @author Thanh Do - 45062147
* @date 1/4/2022
***************************************************************
* EXTERNAL FUNCTIONS
***************************************************************
* int scu_ccs811_init(void)
* double scu_ccs811_read(void)
* int scu_hts221_init(void)
* double scu_hts221_read_temp(void)
* double scu_hts221_read_hum(void)
* int scu_lis2dh_init(void)
* double scu_lis2dh_read(int axis)
* int scu_lps22hb_init(void)
* double scu_lps22hb_read(void)
***************************************************************
*/

#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include <sys/printk.h>
#include <drivers/sensor/ccs811.h>
#include <stdio.h>
#include <sys/util.h>
#include <stdint.h>

static bool app_fw_2;

// Device structures for sensors
const struct device *ccs811_dev;
const struct device *hts221_dev;
const struct device *lis2dh_dev;
const struct device *lps22hb_dev;

/**
 * @brief  reads CCS811 sensor value
 * 
 * @return returns CCS811 sensor value
**/
double scu_ccs811_read_voc(void)
{
	static struct sensor_value tvoc;
	int rc = 0;

	if (rc == 0) {
		rc = sensor_sample_fetch(ccs811_dev);
	}
	
	if (rc == 0) {
		const struct ccs811_result_type *rp = ccs811_result(ccs811_dev);

		sensor_channel_get(ccs811_dev, SENSOR_CHAN_VOC, &tvoc);

		if (app_fw_2 && !(rp->status & CCS811_STATUS_DATA_READY)) {
			printk("STALE DATA\n");
		}

		if (rp->status & CCS811_STATUS_ERROR) {
			printk("ERROR: %02x\n", rp->error);
		}
	}

	if (rc == 0) {
		printk("Timed fetch got %d\n", rc);
	} else if (-EAGAIN == rc) {
		printk("Timed fetch got stale data\n");
	} else {
		printk("Timed fetch failed: %d\n", rc);
	}
	
	return (double) tvoc.val1;
}

/**
 * @brief  reads CCS811 sensor value
 * 
 * @return returns CCS811 sensor value
**/
double scu_ccs811_read_eco2(void)
{
	static struct sensor_value eco2;
	int rc = 0;

	if (rc == 0) {
		rc = sensor_sample_fetch(ccs811_dev);
	}
	
	if (rc == 0) {
		const struct ccs811_result_type *rp = ccs811_result(ccs811_dev);

		sensor_channel_get(ccs811_dev, SENSOR_CHAN_CO2, &eco2);

		if (app_fw_2 && !(rp->status & CCS811_STATUS_DATA_READY)) {
			printk("STALE DATA\n");
		}

		if (rp->status & CCS811_STATUS_ERROR) {
			printk("ERROR: %02x\n", rp->error);
		}
	}

	if (rc == 0) {
		printk("Timed fetch got %d\n", rc);
	} else if (-EAGAIN == rc) {
		printk("Timed fetch got stale data\n");
	} else {
		printk("Timed fetch failed: %d\n", rc);
	}
	
	return (double) eco2.val1;
}

/**
 * @brief  Initilise CCS811 sensor
 * 
 * @return error status
**/
int scu_ccs811_init(void)
{
	ccs811_dev = device_get_binding(DT_LABEL(DT_INST(0, ams_ccs811)));
	struct ccs811_configver_type cfgver;
	int rc;

	if (!ccs811_dev) {
		printk("Failed to get device binding");
		return -1;
	}

	printk("device is %p, name is %s\n", ccs811_dev, ccs811_dev->name);

	rc = ccs811_configver_fetch(ccs811_dev, &cfgver);

	if (rc == 0) {
		printk("HW %02x; FW Boot %04x App %04x ; mode %02x\n",
		       cfgver.hw_version, cfgver.fw_boot_version,
		       cfgver.fw_app_version, cfgver.mode);
		app_fw_2 = (cfgver.fw_app_version >> 8) > 0x11;
	}

    return 0;
}

/**
 * @brief  reads HTS221 temperature sensor value
 * 
 * @return returns HTS221 temperature sensor value
**/
double scu_hts221_read_temp(void)
{
	static struct sensor_value temp;

	if (sensor_sample_fetch(hts221_dev) < 0) {
		printf("Sensor sample update errortemp\n");
		return -1;
	}

	if (sensor_channel_get(hts221_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp) < 0) {
		printf("Cannot read HTS221 temperature channel\n");
		return -1;
	}
    return sensor_value_to_double(&temp);
}

/**
 * @brief  reads HTS221 humidity sensor value
 * 
 * @return returns HTS221 humidity sensor value
**/
double scu_hts221_read_hum(void) 
{
	static struct sensor_value hum;

	if (sensor_sample_fetch(hts221_dev) < 0) {
		printf("Sensor sample update errorhum\n");
		return -1;
	}

	if (sensor_channel_get(hts221_dev, SENSOR_CHAN_HUMIDITY, &hum) < 0) {
		printf("Cannot read HTS221 humidity channel\n");
		return -1;
	}

    return sensor_value_to_double(&hum);
}

/**
 * @brief  Initilise HTS221 sensor
 * 
 * @return error status
**/
int scu_hts221_init(void)
{
    hts221_dev = device_get_binding("HTS221");

	if (hts221_dev == NULL) {
		printf("Could not get HTS221 device\n");
		return -1;
	}

    return 0;
}

/**
 * @brief  reads LIS2DH accelleration sensor value
 * 
 * @param axis Selects which axis to return
 * @return returns LIS2DH accelleration sensor value for selected axis
**/
double scu_lis2dh_read(int axis)
{
	struct sensor_value accel[3];
	const char *overrun = "";
	int rc = sensor_sample_fetch(lis2dh_dev);

	if (rc == -EBADMSG) {
		/* Sample overrun.  Ignore in polled mode. */
		if (IS_ENABLED(CONFIG_LIS2DH_TRIGGER)) {
			overrun = "[OVERRUN] ";
		}
		rc = 0;
	}

	if (rc == 0) {
		rc = sensor_channel_get(lis2dh_dev,
					SENSOR_CHAN_ACCEL_XYZ,
					accel);
	}

	if (rc < 0) {
		printf("ERROR: Update failed: %d\n", rc);
        return -1;
	} else {
		return sensor_value_to_double(&accel[axis]);
	}
}

/**
 * @brief  Initilise LIS2DH sensor
 * 
 * @return error status
**/
int scu_lis2dh_init(void)
{
	lis2dh_dev = DEVICE_DT_GET_ANY(st_lis2dh);

	if (lis2dh_dev == NULL) {
		printf("No device found\n");
		return -1;
	}

	if (!device_is_ready(lis2dh_dev)) {
		printf("Device %s is not ready\n", lis2dh_dev->name);
		return -1;
	}
    
    return 0;
}

/**
 * @brief  reads LPS22HB pressure sensor value
 * 
 * @return returns LPS22HB pressure sensor value
**/
double scu_lps22hb_read(void)
{
	static struct sensor_value pressure;

	if (sensor_sample_fetch(lps22hb_dev) < 0) {
		printf("Sensor sample update errorpress\n");
		return -1;
	}

	if (sensor_channel_get(lps22hb_dev, SENSOR_CHAN_PRESS, &pressure) < 0) {
		printf("Cannot read LPS22HB pressure channel\n");
		return -1;
	}

    return sensor_value_to_double(&pressure);
}

/**
 * @brief  Initilise LPS22HB sensor
 * 
 * @return error status
**/
int scu_lps22hb_init(void)
{
	lps22hb_dev = device_get_binding(DT_LABEL(DT_INST(0, st_lps22hb_press)));

	if (lps22hb_dev == NULL) {
		printf("Could not get LPS22HB device\n");
		return -1;
	}

    return 0;
}