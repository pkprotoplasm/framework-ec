/* Copyright 2020 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* Elemi board-specific configuration */
#include "bb_retimer.h"
#include "button.h"
#include "common.h"
#include "accelgyro.h"
#include "cbi_ec_fw_config.h"
#include "driver/accel_bma2x2.h"
#include "driver/accelgyro_bmi260.h"
#include "driver/als_tcs3400.h"
#include "driver/bc12/pi3usb9201.h"
#include "driver/ppc/sn5s330.h"
#include "driver/ppc/syv682x.h"
#include "driver/retimer/bb_retimer.h"
#include "driver/sync.h"
#include "driver/tcpm/ps8xxx.h"
#include "driver/tcpm/rt1715.h"
#include "driver/tcpm/tcpci.h"
#include "driver/tcpm/tusb422.h"
#include "extpower.h"
#include "fan.h"
#include "fan_chip.h"
#include "gpio.h"
#include "hooks.h"
#include "keyboard_raw.h"
#include "keyboard_scan.h"
#include "lid_switch.h"
#include "power.h"
#include "power_button.h"
#include "pwm.h"
#include "pwm_chip.h"
#include "switch.h"
#include "system.h"
#include "task.h"
#include "tablet_mode.h"
#include "throttle_ap.h"
#include "uart.h"
#include "usb_mux.h"
#include "usb_pd.h"
#include "usb_pd_tbt.h"
#include "usb_pd_tcpm.h"
#include "usbc_ppc.h"
#include "util.h"

#include "gpio_list.h" /* Must come after other header files. */

#define CPRINTS(format, args...) cprints(CC_CHIPSET, format, ## args)

/* Keyboard scan setting */
struct keyboard_scan_config keyscan_config = {
	/* Increase from 50 us, because KSO_02 passes through the H1. */
	.output_settle_us = 80,
	/* Other values should be the same as the default configuration. */
	.debounce_down_us = 9 * MSEC,
	.debounce_up_us = 30 * MSEC,
	.scan_period_us = 3 * MSEC,
	.min_post_scan_delay_us = 1000,
	.poll_timeout_us = 100 * MSEC,
	.actual_key_mask = {
		0x14, 0xff, 0xff, 0xff, 0xff, 0xf5, 0xff,
		0xa4, 0xff, 0xfe, 0x55, 0xfa, 0xca  /* full set */
	},
};

/******************************************************************************/
/*
 * FW_CONFIG defaults for Volteer if the CBI data is not initialized.
 */
union volteer_cbi_fw_config fw_config_defaults = {
	.usb_db = DB_USB4_GEN2,
};

__override enum tbt_compat_cable_speed board_get_max_tbt_speed(int port)
{
	enum ec_cfg_usb_db_type usb_db = ec_cfg_usb_db_type();

	if (port == USBC_PORT_C1) {
		if (usb_db == DB_USB4_GEN2) {
			/*
			 * Older boards violate 205mm trace length prior
			 * to connection to the re-timer and only support up
			 * to GEN2 speeds.
			 */
			return TBT_SS_U32_GEN1_GEN2;
		} else if (usb_db == DB_USB4_GEN3) {
			return TBT_SS_TBT_GEN3;
		}
	}

	/*
	 * Thunderbolt-compatible mode not supported
	 *
	 * TODO (b/147726366): All the USB-C ports need to support same speed.
	 * Need to fix once USB-C feature set is known for Volteer.
	 */
	return TBT_SS_RES_0;
}

__override bool board_is_tbt_usb4_port(int port)
{
	enum ec_cfg_usb_db_type usb_db = ec_cfg_usb_db_type();

	/*
	 * Volteer reference design only supports TBT & USB4 on port 1
	 * if the USB4 DB is present.
	 *
	 * TODO (b/147732807): All the USB-C ports need to support same
	 * features. Need to fix once USB-C feature set is known for Volteer.
	 */
	return ((port == USBC_PORT_C1)
		&& ((usb_db == DB_USB4_GEN2) || (usb_db == DB_USB4_GEN3)));
}

/******************************************************************************/
/* Physical fans. These are logically separate from pwm_channels. */

const struct fan_conf fan_conf_0 = {
	.flags = FAN_USE_RPM_MODE,
	.ch = MFT_CH_0,	/* Use MFT id to control fan */
	.pgood_gpio = -1,
	.enable_gpio = GPIO_EN_PP5000_FAN,
};

/*
 * Fan specs from datasheet:
 * Max speed 5900 rpm (+/- 7%), minimum duty cycle 30%.
 * Minimum speed not specified by RPM. Set minimum RPM to max speed (with
 * margin) x 30%.
 *    5900 x 1.07 x 0.30 = 1894, round up to 1900
 */
const struct fan_rpm fan_rpm_0 = {
	.rpm_min = 1900,
	.rpm_start = 1900,
	.rpm_max = 5900,
};

const struct fan_t fans[FAN_CH_COUNT] = {
	[FAN_CH_0] = {
		.conf = &fan_conf_0,
		.rpm = &fan_rpm_0,
	},
};

/******************************************************************************/
/* EC thermal management configuration */

/*
 * Tiger Lake specifies 100 C as maximum TDP temperature.  THRMTRIP# occurs at
 * 130 C.  However, sensor is located next to DDR, so we need to use the lower
 * DDR temperature limit (85 C)
 */
const static struct ec_thermal_config thermal_cpu = {
	.temp_host = {
		[EC_TEMP_THRESH_HIGH] = C_TO_K(70),
		[EC_TEMP_THRESH_HALT] = C_TO_K(80),
	},
	.temp_host_release = {
		[EC_TEMP_THRESH_HIGH] = C_TO_K(65),
	},
	.temp_fan_off = C_TO_K(35),
	.temp_fan_max = C_TO_K(50),
};

/*
 * Inductor limits - used for both charger and PP3300 regulator
 *
 * Need to use the lower of the charger IC, PP3300 regulator, and the inductors
 *
 * Charger max recommended temperature 100C, max absolute temperature 125C
 * PP3300 regulator: operating range -40 C to 145 C
 *
 * Inductors: limit of 125c
 * PCB: limit is 80c
 */
const static struct ec_thermal_config thermal_inductor = {
	.temp_host = {
		[EC_TEMP_THRESH_HIGH] = C_TO_K(75),
		[EC_TEMP_THRESH_HALT] = C_TO_K(80),
	},
	.temp_host_release = {
		[EC_TEMP_THRESH_HIGH] = C_TO_K(65),
	},
	.temp_fan_off = C_TO_K(40),
	.temp_fan_max = C_TO_K(55),
};


struct ec_thermal_config thermal_params[] = {
	[TEMP_SENSOR_1_CHARGER]			= thermal_inductor,
	[TEMP_SENSOR_2_PP3300_REGULATOR]	= thermal_inductor,
	[TEMP_SENSOR_3_DDR_SOC]			= thermal_cpu,
	[TEMP_SENSOR_4_FAN]			= thermal_cpu,
};
BUILD_ASSERT(ARRAY_SIZE(thermal_params) == TEMP_SENSOR_COUNT);

/******************************************************************************/
/* MFT channels. These are logically separate from pwm_channels. */
const struct mft_t mft_channels[] = {
	[MFT_CH_0] = {
		.module = NPCX_MFT_MODULE_1,
		.clk_src = TCKC_LFCLK,
		.pwm_id = PWM_CH_FAN,
	},
};
BUILD_ASSERT(ARRAY_SIZE(mft_channels) == MFT_CH_COUNT);

/******************************************************************************/
/* I2C port map configuration */
const struct i2c_port_t i2c_ports[] = {
	{
		.name = "sensor",
		.port = I2C_PORT_SENSOR,
		.kbps = 400,
		.scl = GPIO_EC_I2C0_SENSOR_SCL,
		.sda = GPIO_EC_I2C0_SENSOR_SDA,
	},
	{
		.name = "usb_c0",
		.port = I2C_PORT_USB_C0,
		.kbps = 1000,
		.scl = GPIO_EC_I2C1_USB_C0_SCL,
		.sda = GPIO_EC_I2C1_USB_C0_SDA,
	},
	{
		.name = "usb_c1",
		.port = I2C_PORT_USB_C1,
		.kbps = 1000,
		.scl = GPIO_EC_I2C2_USB_C1_SCL,
		.sda = GPIO_EC_I2C2_USB_C1_SDA,
	},
	{
		.name = "usb_1_mix",
		.port = I2C_PORT_USB_1_MIX,
		.kbps = 100,
		.scl = GPIO_EC_I2C3_USB_1_MIX_SCL,
		.sda = GPIO_EC_I2C3_USB_1_MIX_SDA,
	},
	{
		.name = "power",
		.port = I2C_PORT_POWER,
		.kbps = 100,
		.scl = GPIO_EC_I2C5_BATTERY_SCL,
		.sda = GPIO_EC_I2C5_BATTERY_SDA,
	},
	{
		.name = "eeprom",
		.port = I2C_PORT_EEPROM,
		.kbps = 400,
		.scl = GPIO_EC_I2C7_EEPROM_PWR_SCL_R,
		.sda = GPIO_EC_I2C7_EEPROM_PWR_SDA_R,
	},
};
const unsigned int i2c_ports_used = ARRAY_SIZE(i2c_ports);

/******************************************************************************/
/* PWM configuration */
const struct pwm_t pwm_channels[] = {
	[PWM_CH_FAN] = {
		.channel = 5,
		.flags = PWM_CONFIG_OPEN_DRAIN,
		.freq = 25000
	},
	[PWM_CH_KBLIGHT] = {
		.channel = 3,
		.flags = 0,
		/*
		 * Set PWM frequency to multiple of 50 Hz and 60 Hz to prevent
		 * flicker. Higher frequencies consume similar average power to
		 * lower PWM frequencies, but higher frequencies record a much
		 * lower maximum power.
		 */
		.freq = 2400,
	},
};
BUILD_ASSERT(ARRAY_SIZE(pwm_channels) == PWM_CH_COUNT);

/******************************************************************************/
/* Volteer specific USB daughter-board configuration */

/* USBC TCPC configuration for USB3 daughter board */
static const struct tcpc_config_t tcpc_config_p1_usb3 = {
	.bus_type = EC_BUS_TYPE_I2C,
	.i2c_info = {
		.port = I2C_PORT_USB_C1,
		.addr_flags = PS8751_I2C_ADDR1_FLAGS,
	},
	.flags = TCPC_FLAGS_TCPCI_REV2_0 | TCPC_FLAGS_TCPCI_REV2_0_NO_VSAFE0V,
	.drv = &ps8xxx_tcpm_drv,
	.usb23 = USBC_PORT_1_USB2_NUM | (USBC_PORT_1_USB3_NUM << 4),
};

/*
 * USB3 DB mux configuration - the top level mux still needs to be set to the
 * virtual_usb_mux_driver so the AP gets notified of mux changes and updates
 * the TCSS configuration on state changes.
 */
static const struct usb_mux usbc1_usb3_db_retimer = {
	.usb_port = USBC_PORT_C1,
	.driver = &tcpci_tcpm_usb_mux_driver,
	.hpd_update = &ps8xxx_tcpc_update_hpd_status,
	.next_mux = NULL,
};

static const struct usb_mux mux_config_p1_usb3_active = {
	.usb_port = USBC_PORT_C1,
	.driver = &virtual_usb_mux_driver,
	.hpd_update = &virtual_hpd_update,
	.next_mux = &usbc1_usb3_db_retimer,
};

static const struct usb_mux mux_config_p1_usb3_passive = {
	.usb_port = USBC_PORT_C1,
	.driver = &virtual_usb_mux_driver,
	.hpd_update = &virtual_hpd_update,
};

/******************************************************************************/
/* USB-A charging control */

const int usb_port_enable[USB_PORT_COUNT] = {
	GPIO_EN_PP5000_USBA,
};

static enum gpio_signal ps8xxx_rst_odl = GPIO_USB_C1_RT_RST_ODL;

static void ps8815_reset(void)
{
	int val;

	gpio_set_level(ps8xxx_rst_odl, 0);
	msleep(GENERIC_MAX(PS8XXX_RESET_DELAY_MS,
			   PS8815_PWR_H_RST_H_DELAY_MS));
	gpio_set_level(ps8xxx_rst_odl, 1);
	msleep(PS8815_FW_INIT_DELAY_MS);

	/*
	 * b/144397088
	 * ps8815 firmware 0x01 needs special configuration
	 */

	CPRINTS("%s: patching ps8815 registers", __func__);

	if (i2c_read8(I2C_PORT_USB_C1,
		      PS8751_I2C_ADDR1_P2_FLAGS, 0x0f, &val) == EC_SUCCESS)
		CPRINTS("ps8815: reg 0x0f was %02x", val);

	if (i2c_write8(I2C_PORT_USB_C1,
		       PS8751_I2C_ADDR1_P2_FLAGS, 0x0f, 0x31) == EC_SUCCESS)
		CPRINTS("ps8815: reg 0x0f set to 0x31");

	if (i2c_read8(I2C_PORT_USB_C1,
		      PS8751_I2C_ADDR1_P2_FLAGS, 0x0f, &val) == EC_SUCCESS)
		CPRINTS("ps8815: reg 0x0f now %02x", val);
}

void board_reset_pd_mcu(void)
{
	enum ec_cfg_usb_db_type usb_db = ec_cfg_usb_db_type();

	/* No reset available for TCPC on port 0 */
	/* Daughterboard specific reset for port 1 */
	if (usb_db == DB_USB3_ACTIVE) {
		ps8815_reset();
		usb_mux_hpd_update(USBC_PORT_C1, 0, 0);
	}
}

/*
 * Set up support for the USB3 daughterboard:
 *   Parade PS8815 TCPC (integrated retimer)
 *   Diodes PI3USB9201 BC 1.2 chip (same as USB4 board)
 *   Silergy SYV682A PPC (same as USB4 board)
 *   Virtual mux with stacked retimer
 */
static void config_db_usb3_active(void)
{
	tcpc_config[USBC_PORT_C1] = tcpc_config_p1_usb3;
	usb_muxes[USBC_PORT_C1] = mux_config_p1_usb3_active;
}

/*
 * Set up support for the passive USB3 daughterboard:
 *   TUSB422 TCPC (already the default)
 *   PI3USB9201 BC 1.2 chip (already the default)
 *   Silergy SYV682A PPC (already the default)
 *   Virtual mux without stacked retimer
 */

static void config_db_usb3_passive(void)
{
	usb_muxes[USBC_PORT_C1] = mux_config_p1_usb3_passive;
}

static void config_port_discrete_tcpc(int port)
{
	/*
	 * Support 2 Pin-to-Pin compatible parts: TUSB422 and RT1715, for
	 * simplicity allow either and decide at runtime which we are using.
	 * Default to TUSB422, and switch to RT1715 if it is on the I2C bus and
	 * the VID matches.
	 */

	int regval;

	if (i2c_read16(port ? I2C_PORT_USB_C1 : I2C_PORT_USB_C0,
		       RT1715_I2C_ADDR_FLAGS, TCPC_REG_VENDOR_ID,
		       &regval) == EC_SUCCESS) {
		if (regval == RT1715_VENDOR_ID) {
			CPRINTS("C%d: RT1715 detected", port);
			tcpc_config[port].i2c_info.addr_flags =
				RT1715_I2C_ADDR_FLAGS;
			tcpc_config[port].drv = &rt1715_tcpm_drv;
			return;
		}
	}
	CPRINTS("C%d: Default to TUSB422", port);
}

static const char *db_type_prefix = "USB DB type: ";
__override void board_cbi_init(void)
{
	enum ec_cfg_usb_db_type usb_db = ec_cfg_usb_db_type();

	/* Reconfigure Volteer GPIOs based on the board ID */
	if (get_board_id() == 0) {
		CPRINTS("Configuring GPIOs for board ID 0");
		CPRINTS("VOLUME_UP button disabled");

		/* Reassign USB_C1_RT_RST_ODL */
		bb_controls[USBC_PORT_C1].retimer_rst_gpio =
			GPIO_USB_C1_RT_RST_ODL_BOARDID_0;
		ps8xxx_rst_odl = GPIO_USB_C1_RT_RST_ODL_BOARDID_0;
	}
	config_port_discrete_tcpc(0);
	switch (usb_db) {
	case DB_USB_ABSENT:
		CPRINTS("%sNone", db_type_prefix);
		break;
	case DB_USB4_GEN2:
		config_port_discrete_tcpc(1);
		CPRINTS("%sUSB4 Gen1/2", db_type_prefix);
		break;
	case DB_USB4_GEN3:
		config_port_discrete_tcpc(1);
		CPRINTS("%sUSB4 Gen3", db_type_prefix);
		break;
	case DB_USB3_ACTIVE:
		config_db_usb3_active();
		CPRINTS("%sUSB3 Active", db_type_prefix);
		break;
	case DB_USB3_PASSIVE:
		config_db_usb3_passive();
		config_port_discrete_tcpc(1);
		CPRINTS("%sUSB3 Passive", db_type_prefix);
		break;
	default:
		CPRINTS("%sID %d not supported", db_type_prefix, usb_db);
	}

	if ((!IS_ENABLED(TEST_BUILD) && !ec_cfg_has_numeric_pad()) ||
	    get_board_id() <= 2)
		keyboard_raw_set_cols(KEYBOARD_COLS_NO_KEYPAD);
}

/******************************************************************************/
/* USBC PPC configuration */
struct ppc_config_t ppc_chips[] = {
	[USBC_PORT_C0] = {
		.i2c_port = I2C_PORT_USB_C0,
		.i2c_addr_flags = SN5S330_ADDR0_FLAGS,
		.drv = &sn5s330_drv,
	},
	[USBC_PORT_C1] = {
		.i2c_port = I2C_PORT_USB_C1,
		.i2c_addr_flags = SYV682X_ADDR0_FLAGS,
		.drv = &syv682x_drv,
	},
};
BUILD_ASSERT(ARRAY_SIZE(ppc_chips) == USBC_PORT_COUNT);
unsigned int ppc_cnt = ARRAY_SIZE(ppc_chips);

/******************************************************************************/
/* PPC support routines */
void ppc_interrupt(enum gpio_signal signal)
{
	switch (signal) {
	case GPIO_USB_C0_PPC_INT_ODL:
		sn5s330_interrupt(USBC_PORT_C0);
		break;
	case GPIO_USB_C1_PPC_INT_ODL:
		syv682x_interrupt(USBC_PORT_C1);
	default:
		break;
	}
}

/******************************************************************************/
/* BC1.2 charger detect configuration */
const struct pi3usb9201_config_t pi3usb9201_bc12_chips[] = {
	[USBC_PORT_C0] = {
		.i2c_port = I2C_PORT_USB_C0,
		.i2c_addr_flags = PI3USB9201_I2C_ADDR_3_FLAGS,
	},
	[USBC_PORT_C1] = {
		.i2c_port = I2C_PORT_USB_C1,
		.i2c_addr_flags = PI3USB9201_I2C_ADDR_3_FLAGS,
	},
};
BUILD_ASSERT(ARRAY_SIZE(pi3usb9201_bc12_chips) == USBC_PORT_COUNT);

/******************************************************************************/
/* USBC TCPC configuration */
struct tcpc_config_t tcpc_config[] = {
	[USBC_PORT_C0] = {
		.bus_type = EC_BUS_TYPE_I2C,
		.i2c_info = {
			.port = I2C_PORT_USB_C0,
			.addr_flags = TUSB422_I2C_ADDR_FLAGS,
		},
		.drv = &tusb422_tcpm_drv,
		.usb23 = USBC_PORT_0_USB2_NUM | (USBC_PORT_0_USB3_NUM << 4),
	},
	[USBC_PORT_C1] = {
		.bus_type = EC_BUS_TYPE_I2C,
		.i2c_info = {
			.port = I2C_PORT_USB_C1,
			.addr_flags = TUSB422_I2C_ADDR_FLAGS,
		},
		.drv = &tusb422_tcpm_drv,
		.usb23 = USBC_PORT_1_USB2_NUM | (USBC_PORT_1_USB3_NUM << 4),
	},
};
BUILD_ASSERT(ARRAY_SIZE(tcpc_config) == USBC_PORT_COUNT);
BUILD_ASSERT(CONFIG_USB_PD_PORT_MAX_COUNT == USBC_PORT_COUNT);

/******************************************************************************/
/* USBC mux configuration - Tiger Lake includes internal mux */
struct usb_mux usbc1_usb4_db_retimer = {
	.usb_port = USBC_PORT_C1,
	.driver = &bb_usb_retimer,
	.i2c_port = I2C_PORT_USB_1_MIX,
	.i2c_addr_flags = USBC_PORT_C1_BB_RETIMER_I2C_ADDR,
};
struct usb_mux usb_muxes[] = {
	[USBC_PORT_C0] = {
		.usb_port = USBC_PORT_C0,
		.driver = &virtual_usb_mux_driver,
		.hpd_update = &virtual_hpd_update,
	},
	[USBC_PORT_C1] = {
		.usb_port = USBC_PORT_C1,
		.driver = &virtual_usb_mux_driver,
		.hpd_update = &virtual_hpd_update,
		.next_mux = &usbc1_usb4_db_retimer,
	},
};
BUILD_ASSERT(ARRAY_SIZE(usb_muxes) == USBC_PORT_COUNT);

struct bb_usb_control bb_controls[] = {
	[USBC_PORT_C0] = {
		/* USB-C port 0 doesn't have a retimer */
	},
	[USBC_PORT_C1] = {
		.usb_ls_en_gpio = GPIO_USB_C1_LS_EN,
		.retimer_rst_gpio = GPIO_USB_C1_RT_RST_ODL,
	},
};
BUILD_ASSERT(ARRAY_SIZE(bb_controls) == USBC_PORT_COUNT);

static void board_tcpc_init(void)
{
	/* Don't reset TCPCs after initial reset */
	if (!system_jumped_late())
		board_reset_pd_mcu();

	/* Enable PPC interrupts. */
	gpio_enable_interrupt(GPIO_USB_C0_PPC_INT_ODL);
	gpio_enable_interrupt(GPIO_USB_C1_PPC_INT_ODL);

	/* Enable TCPC interrupts. */
	gpio_enable_interrupt(GPIO_USB_C0_TCPC_INT_ODL);
	gpio_enable_interrupt(GPIO_USB_C1_TCPC_INT_ODL);

	/* Enable BC1.2 interrupts. */
	gpio_enable_interrupt(GPIO_USB_C0_BC12_INT_ODL);
	gpio_enable_interrupt(GPIO_USB_C1_BC12_INT_ODL);
}
DECLARE_HOOK(HOOK_INIT, board_tcpc_init, HOOK_PRIO_INIT_CHIPSET);

/******************************************************************************/
/* TCPC support routines */
uint16_t tcpc_get_alert_status(void)
{
	uint16_t status = 0;

	/*
	 * Check which port has the ALERT line set
	 */
	if (!gpio_get_level(GPIO_USB_C0_TCPC_INT_ODL))
		status |= PD_STATUS_TCPC_ALERT_0;
	if (!gpio_get_level(GPIO_USB_C1_TCPC_INT_ODL))
		status |= PD_STATUS_TCPC_ALERT_1;

	return status;
}

int ppc_get_alert_status(int port)
{
	if (port == USBC_PORT_C0)
		return gpio_get_level(GPIO_USB_C0_PPC_INT_ODL) == 0;
	else
		return gpio_get_level(GPIO_USB_C1_PPC_INT_ODL) == 0;
}
