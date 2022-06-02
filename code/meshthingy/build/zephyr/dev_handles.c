#include <device.h>
#include <toolchain.h>

/* 1 : /soc/clock@40000000:
 * Direct Dependencies:
 *   - (/soc)
 *   - (/soc/interrupt-controller@e000e100)
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_DT_N_S_soc_S_clock_40000000[] = { DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };

/* 2 : /soc/gpio@50000000:
 * Direct Dependencies:
 *   - (/soc)
 * Supported:
 *   - /spk-pwr-ctrl
 *   - /vdd-pwr-ctrl
 *   - (/buttons/button_0)
 *   - /soc/i2c@40003000/ccs811@5a
 *   - /soc/i2c@40003000/hts221@5f
 *   - /soc/i2c@40004000/lis2dh12@19
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_DT_N_S_soc_S_gpio_50000000[] = { DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, 13, 16, 18, 14, 17, DEVICE_HANDLE_ENDS };

/* 3 : /soc/random@4000d000:
 * Direct Dependencies:
 *   - (/soc)
 *   - (/soc/interrupt-controller@e000e100)
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_DT_N_S_soc_S_random_4000d000[] = { DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };

/* 4 : /soc/uart@40002000:
 * Direct Dependencies:
 *   - (/soc)
 *   - (/soc/interrupt-controller@e000e100)
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_DT_N_S_soc_S_uart_40002000[] = { DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };

/* 5 : sysinit:
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_vdd_pwr_ctrl_init[] = { DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };

/* 6 : /soc/flash-controller@4001e000:
 * Direct Dependencies:
 *   - (/soc)
 * Supported:
 *   - (/soc/flash-controller@4001e000/flash@0)
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_DT_N_S_soc_S_flash_controller_4001e000[] = { DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };

/* 7 : /soc/i2c@40004000:
 * Direct Dependencies:
 *   - (/soc)
 *   - (/soc/interrupt-controller@e000e100)
 * Supported:
 *   - /soc/i2c@40004000/lis2dh12@19
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_DT_N_S_soc_S_i2c_40004000[] = { DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, 18, DEVICE_HANDLE_ENDS };

/* 8 : /soc/i2c@40003000:
 * Direct Dependencies:
 *   - (/soc)
 *   - (/soc/interrupt-controller@e000e100)
 * Supported:
 *   - /soc/i2c@40003000/ccs811@5a
 *   - /soc/i2c@40003000/hts221@5f
 *   - /soc/i2c@40003000/lps22hb_press@5c
 *   - /soc/i2c@40003000/sx1509b@3e
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_DT_N_S_soc_S_i2c_40003000[] = { DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, 16, 19, 17, 9, DEVICE_HANDLE_ENDS };

/* 9 : /soc/i2c@40003000/sx1509b@3e:
 * Direct Dependencies:
 *   - /vdd-pwr-ctrl
 *   - /soc/i2c@40003000
 * Supported:
 *   - /ccs-pwr-ctrl
 *   - /mic-pwr-ctrl
 *   - /mpu-pwr-ctrl
 *   - (/vbatt)
 *   - (/leds/led_0)
 *   - (/leds/led_1)
 *   - (/leds/led_2)
 *   - /soc/i2c@40003000/ccs811@5a
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_DT_N_S_soc_S_i2c_40003000_S_sx1509b_3e[] = { 8, 14, DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, 12, 16, 11, 10, DEVICE_HANDLE_ENDS };

/* 10 : /ccs-pwr-ctrl:
 * Direct Dependencies:
 *   - (/)
 *   - /soc/i2c@40003000/sx1509b@3e
 * Supported:
 *   - /soc/i2c@40003000/ccs811@5a
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_DT_N_S_ccs_pwr_ctrl[] = { 9, DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, 16, DEVICE_HANDLE_ENDS };

/* 11 : /mic-pwr-ctrl:
 * Direct Dependencies:
 *   - (/)
 *   - /soc/i2c@40003000/sx1509b@3e
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_DT_N_S_mic_pwr_ctrl[] = { 9, DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };

/* 12 : /mpu-pwr-ctrl:
 * Direct Dependencies:
 *   - (/)
 *   - /soc/i2c@40003000/sx1509b@3e
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_DT_N_S_mpu_pwr_ctrl[] = { 9, DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };

/* 13 : /spk-pwr-ctrl:
 * Direct Dependencies:
 *   - (/)
 *   - /soc/gpio@50000000
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_DT_N_S_spk_pwr_ctrl[] = { 2, DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };

/* 14 : /vdd-pwr-ctrl:
 * Direct Dependencies:
 *   - (/)
 *   - /soc/gpio@50000000
 * Supported:
 *   - /soc/i2c@40003000/hts221@5f
 *   - /soc/i2c@40003000/lps22hb_press@5c
 *   - /soc/i2c@40003000/sx1509b@3e
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_DT_N_S_vdd_pwr_ctrl[] = { 2, DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, 17, 19, 9, DEVICE_HANDLE_ENDS };

/* 15 : sysinit:
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_ccs_vdd_pwr_ctrl_init[] = { DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };

/* 16 : /soc/i2c@40003000/ccs811@5a:
 * Direct Dependencies:
 *   - /ccs-pwr-ctrl
 *   - /soc/gpio@50000000
 *   - /soc/i2c@40003000
 *   - /soc/i2c@40003000/sx1509b@3e
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_DT_N_S_soc_S_i2c_40003000_S_ccs811_5a[] = { 9, 8, 2, 10, DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };

/* 17 : /soc/i2c@40003000/hts221@5f:
 * Direct Dependencies:
 *   - /vdd-pwr-ctrl
 *   - /soc/gpio@50000000
 *   - /soc/i2c@40003000
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_DT_N_S_soc_S_i2c_40003000_S_hts221_5f[] = { 14, 8, 2, DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };

/* 18 : /soc/i2c@40004000/lis2dh12@19:
 * Direct Dependencies:
 *   - /soc/gpio@50000000
 *   - /soc/i2c@40004000
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_DT_N_S_soc_S_i2c_40004000_S_lis2dh12_19[] = { 7, 2, DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };

/* 19 : /soc/i2c@40003000/lps22hb_press@5c:
 * Direct Dependencies:
 *   - /vdd-pwr-ctrl
 *   - /soc/i2c@40003000
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_DT_N_S_soc_S_i2c_40003000_S_lps22hb_press_5c[] = { 8, 14, DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };
