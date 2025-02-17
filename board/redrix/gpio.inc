/* -*- mode:c -*-
 *
 * Copyright 2021 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define MODULE_KB		MODULE_KEYBOARD_SCAN

/* INTERRUPT GPIOs: */
GPIO_INT(ACOK_OD,                        PIN(0, 0), GPIO_INT_BOTH | GPIO_HIB_WAKE_HIGH, extpower_interrupt)
GPIO_INT(EC_ALS_RGB_INT_R_L,             PIN(D, 4), GPIO_INT_FALLING, tcs3400_interrupt)
GPIO_INT(EC_IMU_INT_R_L,                 PIN(5, 6), GPIO_SEL_1P8V | GPIO_INT_FALLING, lsm6dsm_interrupt)
GPIO_INT(EC_PROCHOT_IN_L,                PIN(F, 0), GPIO_INT_BOTH, throttle_ap_prochot_input_interrupt)
GPIO_INT(EC_VOLDN_BTN_ODL,               PIN(9, 3), GPIO_INT_BOTH | GPIO_PULL_UP, button_interrupt)
GPIO_INT(EC_VOLUP_BTN_ODL,               PIN(9, 7), GPIO_INT_BOTH | GPIO_PULL_UP, button_interrupt)
GPIO_INT(EC_WP_ODL,                      PIN(A, 1), GPIO_INT_BOTH, switch_interrupt)
GPIO_INT(GSC_EC_PWR_BTN_ODL,             PIN(0, 1), GPIO_INT_BOTH | GPIO_HIB_WAKE_LOW, power_button_interrupt)
GPIO_INT(LID_OPEN,                       PIN(D, 2), GPIO_INT_BOTH | GPIO_HIB_WAKE_HIGH, lid_interrupt)
GPIO_INT(SEQ_EC_ALL_SYS_PG,              PIN(F, 4), GPIO_INT_BOTH, power_signal_interrupt)
GPIO_INT(SEQ_EC_DSW_PWROK,               PIN(C, 7), GPIO_INT_BOTH, power_signal_interrupt)
GPIO_INT(SEQ_EC_RSMRST_ODL,              PIN(E, 2), GPIO_INT_BOTH, power_signal_interrupt)
GPIO_INT(SLP_S3_L,                       PIN(A, 5), GPIO_INT_BOTH, power_signal_interrupt)
GPIO_INT(SLP_SUS_L,                      PIN(F, 1), GPIO_INT_BOTH, power_signal_interrupt)
GPIO_INT(SYS_SLP_S0IX_L,                 PIN(D, 5), GPIO_INT_BOTH, power_signal_interrupt)
GPIO_INT(TABLET_MODE_L,                  PIN(9, 5), GPIO_INT_BOTH, gmr_tablet_switch_isr)
GPIO_INT(USB_C0_BC12_INT_ODL,            PIN(C, 6), GPIO_INT_FALLING, bc12_interrupt)
GPIO_INT(USB_C0_TCPC_INT_ODL,            PIN(E, 0), GPIO_INT_FALLING, tcpc_alert_event)
GPIO_INT(USB_C0_PPC_INT_ODL,             PIN(6, 2), GPIO_INT_FALLING, ppc_interrupt)
GPIO_INT(USB_C0_RT_INT_ODL,              PIN(B, 1), GPIO_INT_FALLING, retimer_interrupt)
GPIO_INT(USB_C1_TCPC_INT_ODL,            PIN(A, 2), GPIO_INT_FALLING, tcpc_alert_event)
GPIO_INT(USB_C1_BC12_INT_ODL,            PIN(8, 3), GPIO_INT_FALLING, bc12_interrupt)
GPIO_INT(USB_C1_PPC_INT_ODL,             PIN(7, 0), GPIO_INT_FALLING, ppc_interrupt)
GPIO_INT(USB_C1_RT_INT_ODL,              PIN(4, 1), GPIO_INT_FALLING, retimer_interrupt)

/* WLC interrupt. GPIO_PULL_DOWN ensures no IRQ when WLC chip is off. */
#ifdef SECTION_IS_RW
GPIO_INT(PEN_INT_ODL, PIN(F, 5), GPIO_INT_RISING | GPIO_PULL_DOWN, pchg_irq)
#endif

/* USED GPIOs: */
GPIO(CCD_MODE_ODL,                   PIN(E, 5), GPIO_INPUT)
GPIO(CHARGER_VAP_OTG_EN,             PIN(9, 6), GPIO_OUT_LOW)
GPIO(CPU_C10_GATE_L,                 PIN(6, 7), GPIO_INPUT)
GPIO(EC_BATT_PRES_ODL,               PIN(A, 3), GPIO_INPUT)
GPIO(EC_ENTERING_RW,                 PIN(0, 3), GPIO_OUT_LOW)
GPIO(EC_EN_EDP_BL,                   PIN(D, 3), GPIO_OUT_HIGH)
GPIO(EC_GSC_PACKET_MODE,             PIN(7, 5), GPIO_OUT_LOW)
GPIO(EC_I2C_BAT_SCL,                 PIN(3, 3), GPIO_INPUT)
GPIO(EC_I2C_BAT_SDA,                 PIN(3, 6), GPIO_INPUT)
GPIO(EC_I2C_MISC_SCL_R,              PIN(B, 3), GPIO_INPUT)
GPIO(EC_I2C_MISC_SDA_R,              PIN(B, 2), GPIO_INPUT)
GPIO(EC_I2C_SENSOR_SCL,              PIN(B, 5), GPIO_INPUT | GPIO_SEL_1P8V)
GPIO(EC_I2C_SENSOR_SDA,              PIN(B, 4), GPIO_INPUT | GPIO_SEL_1P8V)
GPIO(EC_I2C_USB_C0_PPC_BC_SCL,       PIN(9, 2), GPIO_INPUT)
GPIO(EC_I2C_USB_C0_PPC_BC_SDA,       PIN(9, 1), GPIO_INPUT)
GPIO(EC_I2C_USB_C0_RT_SCL,           PIN(D, 1), GPIO_INPUT)
GPIO(EC_I2C_USB_C0_RT_SDA,           PIN(D, 0), GPIO_INPUT)
GPIO(EC_I2C_USB_C0_TCPC_SCL,         PIN(9, 0), GPIO_INPUT)
GPIO(EC_I2C_USB_C0_TCPC_SDA,         PIN(8, 7), GPIO_INPUT)
GPIO(EC_I2C_USB_C1_MIX_SCL,          PIN(E, 4), GPIO_INPUT)
GPIO(EC_I2C_USB_C1_MIX_SDA,          PIN(E, 3), GPIO_INPUT)
GPIO(EC_I2C_USB_C1_TCPC_SCL,         PIN(F, 3), GPIO_INPUT)
GPIO(EC_I2C_USB_C1_TCPC_SDA,         PIN(F, 2), GPIO_INPUT)
GPIO(EC_KB_BL_EN,                    PIN(8, 6), GPIO_OUT_LOW)
GPIO(EC_PCHHOT_ODL,                  PIN(7, 4), GPIO_INPUT)
GPIO(EC_PCH_INT_ODL,                 PIN(B, 0), GPIO_ODR_HIGH)
GPIO(EC_PCH_PWR_BTN_ODL,             PIN(C, 1), GPIO_ODR_HIGH)
GPIO(EC_PCH_RSMRST_L,                PIN(A, 6), GPIO_OUT_LOW)
GPIO(EC_PCH_RTCRST,                  PIN(7, 6), GPIO_OUT_LOW)
GPIO(EC_PCH_SYS_PWROK,               PIN(3, 7), GPIO_OUT_LOW)
GPIO(EC_PCH_WAKE_R_ODL,              PIN(C, 0), GPIO_ODR_HIGH)
GPIO(EC_PROCHOT_ODL,                 PIN(6, 3), GPIO_ODR_HIGH)
GPIO(EN_PP5000_FAN,                  PIN(6, 1), GPIO_OUT_HIGH)
GPIO(EN_PP5000_FAN2,                 PIN(5, 0), GPIO_OUT_HIGH)
GPIO(EN_PP5000_USBA_R,               PIN(D, 7), GPIO_OUT_LOW)
GPIO(EN_S5_RAILS,                    PIN(B, 6), GPIO_OUT_LOW)
GPIO(IMVP9_VRRDY_OD,                 PIN(4, 3), GPIO_INPUT)
GPIO(PCH_PWROK,                      PIN(7, 2), GPIO_OUT_LOW)
GPIO(SYS_RST_ODL,                    PIN(C, 5), GPIO_ODR_HIGH)
GPIO(USB_C0_TCPC_RST_ODL,            PIN(A, 7), GPIO_ODR_LOW)
GPIO(USB_C1_TCPC_RST_ODL,            PIN(A, 0), GPIO_ODR_LOW)
GPIO(VCCST_PWRGD_OD,                 PIN(A, 4), GPIO_ODR_LOW)
GPIO(PEN_RST_L,                      PIN(0, 2), GPIO_ODR_HIGH)
GPIO(LRA_DIS_ODL,                    PIN(0, 4), GPIO_ODR_HIGH)	/* Reserved for disable haptic pad LRA */

/* LED */
GPIO(C0_CHARGE_LED_AMBER_L,          PIN(C, 4), GPIO_OUT_HIGH)	/* Amber C0 port */
GPIO(C0_CHARGE_LED_WHITE_L,          PIN(C, 3), GPIO_OUT_HIGH)	/* White C0 port */
GPIO(C1_CHARGE_LED_AMBER_L,          PIN(5, 7), GPIO_OUT_HIGH)	/* Amber C1 port */
GPIO(C1_CHARGE_LED_WHITE_L,          PIN(9, 4), GPIO_OUT_HIGH)	/* White C1 port */
GPIO(PWR_LED_WHITE_L,                PIN(C, 2), GPIO_OUT_HIGH)	/* Power LED */

/* UART alternate functions */
ALTERNATE(PIN_MASK(6, 0x30), 0, MODULE_UART, 0)              /* GPIO64/CR_SIN1, GPO65/CR_SOUT1/FLPRG1_L */

/* I2C alternate functions */
ALTERNATE(PIN_MASK(3, 0x48), 0, MODULE_I2C, 0)               /* GPIO33/I2C5_SCL0/CTS_L, GPIO36/RTS_L/I2C5_SDA0 */
ALTERNATE(PIN_MASK(8, 0x80), 0, MODULE_I2C, 0)               /* GPIO87/I2C1_SDA0 */
ALTERNATE(PIN_MASK(9, 0x07), 0, MODULE_I2C, 0)               /* GPIO92/I2C2_SCL0, GPIO91/I2C2_SDA0, GPIO90/I2C1_SCL0 */
ALTERNATE(PIN_MASK(B, 0x0c), 0, MODULE_I2C, 0)               /* GPIOB3/I2C7_SCL0/DCD_L, GPIOB2/I2C7_SDA0/DSR_L */
ALTERNATE(PIN_MASK(B, 0x30), 0, MODULE_I2C, GPIO_SEL_1P8V)   /* GPIOB5/I2C0_SCL0, GPIOB4/I2C0_SDA0 */
ALTERNATE(PIN_MASK(D, 0x03), 0, MODULE_I2C, 0)               /* GPIOD1/I2C3_SCL0, GPIOD0/I2C3_SDA0 */
ALTERNATE(PIN_MASK(E, 0x18), 0, MODULE_I2C, 0)               /* GPIOE4/I2C6_SCL1/I3C_SCL, GPIOE3/I2C6_SDA1/I3C_SDA */
ALTERNATE(PIN_MASK(F, 0x0c), 0, MODULE_I2C, 0)               /* GPIOF3/I2C4_SCL1, GPIOF2/I2C4_SDA1 */

/* PWM alternate functions */
ALTERNATE(PIN_MASK(4, 0x01), 0, MODULE_PWM, 0)               /* GPIO40/TA1 */
ALTERNATE(PIN_MASK(7, 0x08), 0, MODULE_PWM, 0)               /* GPIO73/TA2 */
ALTERNATE(PIN_MASK(6, 0x01), 0, MODULE_PWM, 0)               /* GPIO60/PWM7 */
ALTERNATE(PIN_MASK(8, 0x01), 0, MODULE_PWM, 0)               /* GPIO80/PWM3 */
ALTERNATE(PIN_MASK(B, 0x80), 0, MODULE_PWM, 0)               /* GPIOB7/PWM5 */

/* ADC alternate functions */
ALTERNATE(PIN_MASK(3, 0x10), 0, MODULE_ADC, 0)               /* GPIO34/PS2_DAT2/ADC6 */
ALTERNATE(PIN_MASK(4, 0x34), 0, MODULE_ADC, 0)               /* GPIO42/ADC3/RI_L, GPIO45/ADC0, GPIO44/ADC1 */
ALTERNATE(PIN_MASK(E, 0x02), 0, MODULE_ADC, 0)               /* GPIOE1/ADC7 */

/* KB alternate functions */
ALTERNATE(PIN_MASK(0, 0xe0), 0, MODULE_KB, GPIO_ODR_HIGH)    /* KSO10&P80_CLK/GPIO07, KSO11&P80_DAT/GPIO06, KSO12/GPIO05 */
ALTERNATE(PIN_MASK(1, 0x7f), 0, MODULE_KB, GPIO_ODR_HIGH)    /* KSO06/GPO13/GP_SEL_L, KSO07/GPO12/JEN_L, KSO03/GPIO16/JTAG_TDO0_SWO, KSO04/GPIO15/XNOR, KSO05/GPIO14, KSO08/GPIO11/CR_SOUT1, KSO09/GPIO10/CR_SIN1 */
ALTERNATE(PIN_MASK(2, 0xfc), 0, MODULE_KB, GPIO_INPUT | GPIO_PULL_UP) /* KSI2/GPIO27/TRACEDATA1, KSI3/GPIO26/TRACEDATA0, KSI4/GPIO25/TRACECLK/GP_SCLK, KSI5/GPIO24/GP_MISO, KSI6/GPIO23/S_SBUB, KSI7/GPIO22/S_SBUA */
ALTERNATE(PIN_MASK(2, 0x03), 0, MODULE_KB, GPIO_ODR_HIGH)    /* KSO00/GPIO21/JTAG_TCK_SWCLK, KSO01/GPIO20/JTAG_TMS_SWIO */
ALTERNATE(PIN_MASK(3, 0x03), 0, MODULE_KB, GPIO_INPUT | GPIO_PULL_UP) /* KSI0/GPIO31/TRACEDATA3/GP_MOSI, KSI1/GPIO30/TRACEDATA2/GP_CS_L */

/* PMU alternate functions */
ALTERNATE(PIN_MASK(0, 0x01), 0, MODULE_PMU, GPIO_INT_BOTH | GPIO_HIB_WAKE_HIGH) /* PSL_IN2_L&GPI00/GPIO00 */
ALTERNATE(PIN_MASK(0, 0x02), 0, MODULE_PMU, GPIO_INT_BOTH | GPIO_HIB_WAKE_LOW) /* GPIO01/PSL_IN3_L&GPI01 */
ALTERNATE(PIN_MASK(D, 0x04), 0, MODULE_PMU, GPIO_INT_BOTH | GPIO_HIB_WAKE_HIGH) /* PSL_IN1_L&GPID2/GPIOD2 */

/* Unused Pins */
UNUSED(PIN(D, 6))       /* GPOD6/CR_SOUT3/SHDF_ESPI_L */
UNUSED(PIN(3, 2))       /* GPO32/TRIS_L */
UNUSED(PIN(3, 5))       /* GPO35/CR_SOUT4/TEST_L */
UNUSED(PIN(6, 6))       /* GPO66/ARM_L_x86 */
UNUSED(PIN(8, 2))       /* KSO14/GPIO82 */

/* Pre-configured PSL balls:  J8  K6 */

/*
 * The NPCX keyboard driver does not use named GPIOs to access
 * keyboard scan pins, so we do not list them in *gpio.inc. However, when
 * KEYBOARD_COL2_INVERTED is defined, this name is required.
 */
GPIO(EC_KSO_02_INV,               PIN(1, 7), GPIO_OUT_LOW)

IOEX(USB_C0_OC_ODL,          EXPIN(IOEX_C0_NCT38XX, 0, 4), GPIO_ODR_HIGH)
IOEX(USB_C0_FRS_EN,          EXPIN(IOEX_C0_NCT38XX, 0, 6), GPIO_LOW)
IOEX(USB_C0_RT_RST_ODL,      EXPIN(IOEX_C0_NCT38XX, 0, 7), GPIO_ODR_LOW)

IOEX(USB_C1_OC_ODL,          EXPIN(IOEX_C1_NCT38XX, 0, 4), GPIO_ODR_HIGH)
IOEX(USB_C1_FRS_EN,          EXPIN(IOEX_C1_NCT38XX, 0, 6), GPIO_LOW)
IOEX(USB_C1_RT_RST_ODL,      EXPIN(IOEX_C1_NCT38XX, 0, 7), GPIO_ODR_LOW)
