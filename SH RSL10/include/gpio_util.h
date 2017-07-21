/* ----------------------------------------------------------------------------
 * Copyright (c) 2016 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * This code is the property of ON Semiconductor and may not be redistributed
 * in any form without prior written permission from ON Semiconductor.
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 * ----------------------------------------------------------------------------
 * gpio_util.h
 * - GPIO pin utility functions
 * ----------------------------------------------------------------------------
 * $Revision: 1.5 $
 * $Date: 2016/10/11 23:39:12 $
 * ------------------------------------------------------------------------- */

#ifndef GPIO_UTIL_H
#define GPIO_UTIL_H

/* ----------------------------------------------------------------------------
 * If building with a C++ compiler, make all of the definitions in this header
 * have a C binding.
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif

#include <rsl10.h>
#include <rsl10_hw_cid101.h>
#include <rsl10_sys_spi.h>

/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/

#define APP_INIT_PIN                    1
#define BUZZ_APP_INIT_TOGGLE_NUM        10
#define BUZZ_APP_INIT_TOGGLE_DURATION   100000
/* --------------------------------------------------------------------------*/
#define SLEEP_WK_PIN                    1
#define BUZZ_SLEEP_WK_TOGGLE_NUM        100
#define BUZZ_SLEEP_WK_TOGGLE_DURATION   50
/* --------------------------------------------------------------------------*/
#define SVC_NOTIF_PIN                   3
#define BUZZ_SVC_NOTIF_TOGGLE_NUM       100
#define BUZZ_SVC_NOTIF_TOGGLE_DURATION  50
/* --------------------------------------------------------------------------*/
#define TIMER0_ISR_PIN                  5


/* ----------------------------------------------------------------------------
 * Function prototype definitions
 * --------------------------------------------------------------------------*/

void Buzz_GPIOs_Init(void);

void Buzz_GPIO(uint32_t gpio_pin, unsigned int toggle_num,
               unsigned int toggle_duration);

void Test_GPIO8_Init(void);
void SPI1_SlaveCfg(void);
void GPIO_4_5_to_Input(void);

/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif

#endif
