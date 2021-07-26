/* Copyright 2020 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "clock-f.h"
#include "test_util.h"

static uint32_t rtc_fired;
static struct rtc_time_reg rtc_irq;
static const int rtc_delay_ms = 500;

/*
 * We will be testing that the RTC interrupt timestamp occurs
 * within +/- delay_tol_us (tolerance) of the above rtc_delay_ms.
 */
static const int delay_tol_us = MSEC / 2;

/* Override default RTC interrupt handler */
void __rtc_alarm_irq(void)
{
	atomic_add(&rtc_fired, 1);
	reset_rtc_alarm(&rtc_irq);
}

test_static int test_rtc_alarm(void)
{
	struct rtc_time_reg rtc;
	uint32_t rtc_diff_us;
	const int delay_us = rtc_delay_ms * MSEC;

	set_rtc_alarm(0, delay_us, &rtc, 0);

	msleep(2 * rtc_delay_ms);

	/* Make sure the interrupt fired exactly once. */
	TEST_EQ(1, atomic_clear(&rtc_fired), "%d");

	rtc_diff_us = get_rtc_diff(&rtc, &rtc_irq);

	ccprintf("Target delay was %dus\n", delay_us);
	ccprintf("Actual delay was %dus\n", rtc_diff_us);
	ccprintf("The delays are expected to be within +/- %dus\n", MSEC / 2);

	/* Assume we'll always fire within 500us. May need to be adjusted if
	 * this doesn't hold.
	 *
	 * delay_us-delay_tol_us < rtc_diff_us < delay_us+delay_tol_us
	 */
	TEST_LT(delay_us - delay_tol_us, rtc_diff_us, "%dus");
	TEST_LT(rtc_diff_us, delay_us + delay_tol_us, "%dus");

	return EC_SUCCESS;
}

void run_test(int argc, char **argv)
{
	RUN_TEST(test_rtc_alarm);

	test_print_result();
}
