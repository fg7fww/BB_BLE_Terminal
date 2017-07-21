/* ----------------------------------------------------------------------------
 * Copyright (c) 2016 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * This code is the property of ON Semiconductor and may not be redistributed
 * in any form without prior written permission from ON Semiconductor.
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 * ----------------------------------------------------------------------------
 * app_init.c
 * - Application initialization
 * ----------------------------------------------------------------------------
 * $Revision: 1.15 $
 * $Date: 2016/11/03 14:27:42 $
 * ------------------------------------------------------------------------- */

#include "../include/app.h"

/* ----------------------------------------------------------------------------
 * Function      : void App_Initialize(void)
 * ----------------------------------------------------------------------------
 * Description   : Initialize the system for proper application execution
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void App_Initialize(void)
{
    /* Mask all interrupts */
    __set_PRIMASK(PRIMASK_DISABLE_INTERRUPTS);

    /* Disable all interrupts and clear any pending interrupts */
    Sys_NVIC_DisableAllInt();
    Sys_NVIC_ClearAllPendingInt();

    /* Configure the current trim settings for VCC, VDDA */
    ACS_VCC_CTRL->ICH_TRIM_BYTE = VCC_ICHTRIM_16MA_BYTE;
    ACS_VDDA_CP_CTRL->PTRIM_BYTE = VDDA_PTRIM_16MA_BYTE;

    /* Start 48 MHz XTAL oscillator */
    ACS_VDDRF_CTRL->ENABLE_ALIAS = VDDRF_ENABLE_BITBAND;
    ACS_VDDRF_CTRL->CLAMP_ALIAS = VDDRF_DISABLE_HIZ_BITBAND;

    /* Wait until VDDRF supply has powered up */
    while (ACS_VDDRF_CTRL->READY_ALIAS != VDDRF_READY_BITBAND);

    ACS_VDDPA_CTRL->ENABLE_ALIAS = VDDPA_DISABLE_BITBAND;
    ACS_VDDPA_CTRL->VDDPA_SW_CTRL_ALIAS = VDDPA_SW_VDDRF_BITBAND;

    /* Enable RF power switches */
    SYSCTRL_RF_POWER_CFG->RF_POWER_ALIAS = RF_POWER_ENABLE_BITBAND;

    /* Remove RF isolation */
    SYSCTRL_RF_ACCESS_CFG->RF_ACCESS_ALIAS = RF_ACCESS_ENABLE_BITBAND;

    /* Start the 48 MHz oscillator without changing the other register bits */
    RF_XTAL_CTRL->XTAL_CTRL_XO_EN_B_REG_ALIAS =
            XTAL_CTRL_ENABLE_OSCILLATOR_BITBAND;
    RF_XTAL_CTRL->XTAL_CTRL_REG_VALUE_SEL_ALIAS =
            XTAL_CTRL_REG_VALUE_SEL_INTERNAL_BITBAND;

    /* Enable 48 MHz oscillator divider at desired prescale value */
    RF_REG2F->CK_DIV_1_6_CK_DIV_1_6_BYTE = CK_DIV_1_6_PRESCALE_6_BYTE;

    /* Wait until 48 MHz oscillator is started */
    while (RF_REG39->ANALOG_INFO_CLK_DIG_READY_ALIAS !=
           ANALOG_INFO_CLK_DIG_READY_BITBAND);

    /* Switch to (divided 48 MHz) oscillator clock */
    Sys_Clocks_SystemClkConfig(JTCK_PRESCALE_1   |
                               EXTCLK_PRESCALE_1 |
                               SYSCLK_CLKSRC_RFCLK);

    /* Configure clock dividers */
    CLK->DIV_CFG0 = (SLOWCLK_PRESCALE_8 | BBCLK_PRESCALE_1 |
                     USRCLK_PRESCALE_1);
    CLK->DIV_CFG2 = (CPCLK_PRESCALE_8 | DCCLK_PRESCALE_2);

    BBIF->CTRL = (BB_CLK_ENABLE | BBCLK_DIVIDER_8 | BB_WAKEUP);

    /* Seed the random number generator */
    srand(1);

    /* Configure LSAD channel 0 to measure VBAT/2 */
    Sys_LSAD_Set_Config(LSAD_VBAT_DIV2_NORMAL | LSAD_NORMAL |
                        LSAD_PRESCALE_6400);
    Sys_LSAD_InputSelectConfig(0,
                               (LSAD_NEG_INPUT_GND |
                                LSAD_POS_INPUT_VBAT_DIV2));

    /* Configure and start timer 0 with a period of 200 ms */
    Sys_Timer_Set_Control(0, TIMER_FREE_RUN | TIMER_PRESCALE_8 |
                          TIMER_200MS_SETTING);
    Sys_Timers_Start(SELECT_TIMER0);

    /* Configure and start timer 1 with a period of 1 s */
    Sys_Timer_Set_Control(1, TIMER_FREE_RUN | TIMER_PRESCALE_8 |
                          TIMER_1S_SETTING);
    Sys_Timers_Start(SELECT_TIMER1);

    /* Configure SPI1 */
    msg_Len=1;
    SPI1R_STATE=MSG_NO_REQ;
    SPI1_SlaveCfg();

    /*SPI1 request slave for master service*/
    Sys_DIO_Config(6, DIO_WEAK_PULL_UP | DIO_MODE_GPIO_OUT_1);
    Sys_GPIO_Set_High(6);

    GPIO_4_5_to_Input();

    /* Enable timer interrupt */
    NVIC_EnableIRQ(TIMER0_IRQn);
    NVIC_EnableIRQ(TIMER1_IRQn);

    NVIC_EnableIRQ(SPI0_TX_IRQn);
    NVIC_EnableIRQ(SPI0_ERROR_IRQn);
    NVIC_EnableIRQ(SPI1_RX_IRQn);
    NVIC_EnableIRQ(SPI1_ERROR_IRQn);

    /* Enable the needed Bluetooth interrupts, and timer 0 interrupt */
    NVIC_EnableIRQ(BLE_EVENT_IRQn);
    NVIC_EnableIRQ(BLE_RX_IRQn);
    NVIC_EnableIRQ(BLE_CRYPT_IRQn);
    NVIC_EnableIRQ(BLE_ERROR_IRQn);
    NVIC_EnableIRQ(BLE_SW_IRQn);
    NVIC_EnableIRQ(BLE_GROSSTGTIM_IRQn);
    NVIC_EnableIRQ(BLE_FINETGTIM_IRQn);
    NVIC_EnableIRQ(BLE_CSCNT_IRQn);
    NVIC_EnableIRQ(BLE_SLP_IRQn);

    //Test_GPIO8_Init();

    /* Initialize the baseband and application manager */
    Kernel_Init(0);
    BLE_InitNoTL(0);
    App_Init();
    BLE_Reset();

    /* Stop masking interrupts */
    __set_PRIMASK(PRIMASK_ENABLE_INTERRUPTS);
    __set_FAULTMASK(FAULTMASK_ENABLE_INTERRUPTS);


}
