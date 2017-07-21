/* ----------------------------------------------------------------------------
 * Copyright (c) 2015-2016 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * This code is the property of ON Semiconductor and may not be redistributed
 * in any form without prior written permission from ON Semiconductor.
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 * ----------------------------------------------------------------------------
 * app.c
 * - Main application file
 * ----------------------------------------------------------------------------
 * $Revision: 1.21 $
 * $Date: 2016/09/27 13:54:16 $
 * ------------------------------------------------------------------------- */

#include "include/app.h"

/* Global variables for notification simulation */
static uint16_t cnt_notifc = 0;

int main()
{
    App_Initialize();

    SystemCoreClockUpdate();
    unsigned int result;

    /* Enable writing to NVR2 where the bonding information would be saved */
    result = WriteEnable();

    /* This line should be run when erasing flash is desirable
     * result = Flash_EraseSector(FLASH_NVR2_BASE); */

    app_env.bonded = false;

    ke_msg_send_basic(GAPM_DEVICE_READY_IND, TASK_APP, TASK_GAPM);

    /* Main application loop:
     * - Run the kernel scheduler
     * - Update the battery voltage
     * - Refresh the watchdog and wait for an event before continuing
     * - Check for the custom services
     */

    //app_env.state = APPM_ENABLED;

    while (1)
    {
        Kernel_Schedule();
        //Test_SPI();
        app_env.state = APPM_ENABLED;
        if (app_env.state == APPM_ENABLED)
        {
            if (app_env.send_batt_ntf)
            {
                app_env.send_batt_ntf = 0;
                Batt_LevelUpdateSend(app_env.batt_lvl, 0);
            }

            /* Check characteristic configuration value of custom service
             * and send a notification if notification is enabled and rx_value
             * (that is the first characteristic) value has been changed.
             * The value sent indicates to master device that this device
             * is bonded to the master device */
            if ((app_env.cccd_value & 1) && app_env.uart_rx_value_changed)
            {
                if(app_env.bonded == true)
                {
                    //app_env.uart_tx_value[3] = APP_BONDED;
                }
                else
                {
                    //app_env.uart_tx_value[3] = APP_NOT_BONDED;
                }
                app_env.uart_rx_value_changed = 0;
                /*
                CustomService_SendNotification(app_env.conidx,
                                               app_env.start_hdl + 2,
                                               &app_env.uart_tx_value[0], 1);
                */
                CustomService_SendNotification_BCD(app_env.conidx, app_env.start_hdl + 2,
                		&app_env.uart_tx_value[0],10);// msg_Len);
            }

        	if((app_env.spi1_tx_size>0)&&(app_env.spi1_tx_size==app_env.bytes)){
        		if(app_env.spi1_tx_value_changed==0){
        		SPI1->TX_DATA=(uint32_t) SPI1R_MSG_DONE;
        		Sys_GPIO_Set_Low(6);
        		app_env.spi1_tx_value_changed=1;
        		app_env.bytes=0;
        		app_env.spi1_tx_size=0;
        		uint32_t w2z = 200;
        		while(w2z>0){
        			w2z--;
        			if(app_env.spi1_tx_value_changed==0){
        				break;
        			}
        		}
        		}
        	}
            if(app_env.spi1_tx_size>app_env.bytes){
            	if(app_env.spi1_tx_value_changed==0){
            		SPI1->TX_DATA=(uint32_t) app_env.spi1_tx_value[app_env.bytes++];
            		Sys_GPIO_Set_Low(6);
            		app_env.spi1_tx_value_changed=1;

            		uint32_t w2z = 200;
            		while(w2z>0){
            			w2z--;
            			if(app_env.spi1_tx_value_changed==0){
    					break;
            			}
            		}
    			}
            }

        /* Refresh the watchdog timer */
        Sys_Watchdog_Refresh();

        /* Wait for an event before executing the scheduler again */
        SYS_WAIT_FOR_EVENT;
        }
    }
}

/* ----------------------------------------------------------------------------
 * Function      : void TIMER0_IRQHandler(void)
 * ----------------------------------------------------------------------------
 * Description   : Read the battery level using LSAD, calculate the average
 *                 over 16 timer intervals, and if complete set the
 *                 notification flag for the battery service.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void TIMER0_IRQHandler(void)
{
    uint16_t level;

    cnt_notifc++;
    if (cnt_notifc == 30)
    {
        cnt_notifc = 0;
        app_env.uart_rx_value_changed=1;
    }

    /* Calculate the battery level as a percentage, scaling the battery
     * voltage between 1.4V (max) and 1.1V (min) */
    level = ((LSAD->DATA_TRIM_CH[0] - VBAT_1p1V_MEASURED) * 100
             / (VBAT_1p4V_MEASURED - VBAT_1p1V_MEASURED));
    level = ((level >= 100) ? 100 : level);

    /* Add to the current sum and increment the number of reads */
    app_env.sum_batt_lvl += level;
    app_env.num_batt_read++;

    /* Calculate the average over the past 16 voltage reads */
    if (app_env.num_batt_read == 16)
    {
        if ((app_env.sum_batt_lvl >> 4) != app_env.batt_lvl)
        {
            app_env.send_batt_ntf = 1;
        }

        if (app_env.state == APPM_ENABLED)
        {
            app_env.batt_lvl = (app_env.sum_batt_lvl >> 4);
        }
        app_env.num_batt_read = 0;
        app_env.sum_batt_lvl = 0;
    }
}

void SPI0_TX_IRQHandler(void)
{


}

void SPI0_ERROR_IRQHandler (void)
{

}

void SPI1_RX_IRQHandler(void)
{
	switch ((uint32_t) SPI1->RX_DATA)
	{
		case SPI1R_DUMMY:
		    Sys_GPIO_Set_High(6);
			uint32_t w2z = 20000;
			while(w2z>0){
				w2z--;
			}
			app_env.spi1_tx_value_changed=0;
		    break;
		case SPI1R_MSG_SEND:
			SPI1R_STATE=MSG_SEND_REQ;
			for(int i=59;i>0;i--)
			{
				app_env.spi1_rx_value[i]='0';
			}
			msg_Len=0;
			break;
		case SPI1R_MSG_DONE:
			SPI1R_STATE=MSG_NO_REQ;
			app_env.spi1_rx_value_changed=1;
			app_env.uart_rx_value_changed=1;
			memcpy(app_env.uart_tx_value, app_env.spi1_rx_value, 10);
			break;
		case SPI1R_INT2BCD:
			msg_Len=10;
			SPI1R_STATE=MSG_INT2BCD;
		break;
		default:
			switch(SPI1R_STATE)
			{
			case MSG_SEND_REQ:
				app_env.spi1_rx_value[msg_Len++]=(int8_t) SPI1->RX_DATA;
				SPI1->TX_DATA=SPI1->RX_DATA;
				break;
			case MSG_INT2BCD:
				msg_Len=10;
				uint322bcd((uint32_t) SPI1->RX_DATA);
				app_env.spi1_rx_value_changed=1;
				app_env.uart_rx_value_changed=1;
				break;
			default:
				break;
			}
			break;
	}
}

void SPI1_ERROR_IRQHandler(void)
{

}

void TIMER1_IRQHandler (void)
{

}
