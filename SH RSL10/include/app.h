/* ----------------------------------------------------------------------------
 * Copyright (c) 2016 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * This module is derived in part from example code provided by RivieraWaves
 * and as such the underlying code is the property of RivieraWaves [a member
 * of the CEVA, Inc. group of companies], together with additional code which
 * is the property of ON Semiconductor. The code (in whole or any part) may not
 * be redistributed in any form without prior written permission from
 * ON Semiconductor.
 *
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 * ----------------------------------------------------------------------------
 * app.h
 * - Main application header
 * ----------------------------------------------------------------------------
 * $Revision: 1.31 $
 * $Date: 2017/02/03 20:27:59 $
 * ------------------------------------------------------------------------- */

#ifndef APP_H
#define APP_H

/* ----------------------------------------------------------------------------
 * If building with a C++ compiler, make all of the definitions in this header
 * have a C binding.
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif

/* ----------------------------------------------------------------------------
 * Include files
 * --------------------------------------------------------------------------*/
#include <rsl10.h>
#include <rsl10_ke.h>
#include <rsl10_ble.h>
#include <rsl10_profiles.h>
#include <rsl10_flash_rom.h>
#include <diss.h>
#include <rsl10_map_nvr.h>
#include "gpio_util.h"

/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/
#define TIMER_200MS_SETTING             781
#define TIMER_1S_SETTING				3905

/* List of service to add in the database */
enum appm_svc_list
{
    APPM_SVC_BATT, APPM_SVC_CS, APPM_SVC_LIST_STOP,
};

/* Maximal length of the Device Name value  */
#define APP_DEVICE_NAME_LENGTH_MAX      20
#define APP_DFLT_DEVICE_NAME            "BB_BLE_New_Libs"

/* vendor specific advertising type (ON SEMICONDUCTOR Company ID) */
#define APP_SCNRSP_DATA                 {0x4, 0xff, 0x62, 0x3, 0x3}
#define APP_SCNRSP_DATA_LEN             (0x4 + 1)

/* ON SEMICONDUCTOR Company ID and device ID */
#define APP_COMPANY_ID_DATA             {0x4, 0xff, 0x62, 0x3, 0x3}
#define APP_COMPANY_ID_DATA_LEN         (0x4 + 1)

#define CS_SVC_UUID                     {0x24,0xdc,0x0e,0x6e,0x01,0x40,0xca,0x9e,0xe5,0xa9,0xa3,0x00,0xb5,0xf3,0x93,0xe0}

/* Characteristic */
#define CS_CHARACTERISTIC_UUID          {0x03,0x28}

/* RX value */
#define CS_RX_VALUE_UUID                {0x24,0xdc,0x0e,0x6e,0x02,0x40,0xca,0x9e,0xe5,0xa9,0xa3,0x00,0xb5,0xf3,0x93,0xe0}

/* Client characteristic configuration descriptor */
#define CS_CCC_UUID                     {0x02,0x29}

/* TX value */
#define CS_TX_VALUE_UUID                {0x24,0xdc,0x0e,0x6e,0x03,0x40,0xca,0x9e,0xe5,0xa9,0xa3,0x00,0xb5,0xf3,0x93,0xe0}

/* Advertising channel map - 37, 38, 39 */
#define APP_ADV_CHMAP                   0x07

/* Advertising minimum interval - 40ms (64*0.625ms) */
#define APP_ADV_INT_MIN                 64

/* Advertising maximum interval - 40ms (64*0.625ms) */
#define APP_ADV_INT_MAX                 64

/* Non-resolvable Private Bluetooth device address.
 * If public address does not exist, the two MSBs must be zero. */
#ifndef PRIVATE_BDADDR
    #define PRIVATE_BDADDR              {0x94,0x11,0x11,0xff,0xff,0x27}
#endif

/* Number of APP Task Instances */
#define APP_IDX_MAX                     1

/* Minimum and maximum VBAT measurements */
#define VBAT_1p1V_MEASURED              0x1200
#define VBAT_1p4V_MEASURED              0x16cc

/* Bonding status sent to the master device */
#define APP_BONDED                      0x01
#define APP_NOT_BONDED                  0X00

/* Variables those indicate the space availability of the list */
#define LIST_FULL                       0
#define LIST_NOT_FULL                   1

/* Indication of the failure of searching for an open index in a list */
#define INDEX_SEARCH_FAIL               0
#define ERRNO_FLASH_UNRESTRICT_FAILED   1
#define ERRNO_FLASHLIB_ERROR_MASK       0x80

/* Error msg from the flash */
//#define ERRNO_NO_ERROR                  0

/* Length of Bluetooth Address */
#define BDADDR_LENGTH                   6

#define SPI1R_DUMMY					0xF0000005
#define SPI1R_MSG_SEND				0xF0000006
#define SPI1R_INT2BCD				0xF0000007
#define SPI1R_MSG_DONE				0xF0000008
enum SPI1R_STATE_t{
	MSG_NO_REQ,
	MSG_SEND_REQ,
	MSG_INT2BCD,
	//MSG_DONE
};
enum SPI1R_STATE_t SPI1R_STATE;
uint8_t msg_Len;
/* ----------------------------------------------------------------------------
 * Global variables and types
 * --------------------------------------------------------------------------*/
typedef void (*appm_add_svc_func_t)(void);

/* States of APP task */
enum appm_state {
    /* Initialization state */
    APPM_INIT,
    /* Database create state */
    APPM_CREATE_DB,
    /* Ready State */
    APPM_READY,
    /* Advertising state */
    APPM_ADVERTISING,
    /* Connected state */
    APPM_CONNECTED,
    /* Enabled state */
    APPM_ENABLED,
    /* Number of defined states */
    APPM_STATE_MAX
};

/* APP Task messages */
enum appm_msg {
    #if RSL10_CID==8101
        APPM_DUMMY_MSG = KE_FIRST_MSG(TASK_ID_APP),
    #else
        APPM_DUMMY_MSG = TASK_FIRST_MSG(TASK_ID_APP),
    #endif

    /* Timer used to automatically stop advertising */
    APP_ADV_TIMEOUT_TIMER,

    APP_CS_PARAM_UPDATE_TIMER
};

struct app_env_tag {
    /* Connection handle */
    uint16_t conhdl;

    /* Connection index */
    uint8_t conidx;

    /* Last initialized profile */
    uint8_t next_svc;

    /* Bonding status */
    bool bonded;

    uint8_t state;

    uint8_t batt_lvl;
    uint8_t batt_ntf_cfg;
    uint32_t sum_batt_lvl;
    uint16_t num_batt_read;
    uint8_t send_batt_ntf;

    uint16_t start_hdl;

    uint16_t con_interval;
    uint16_t time_out;
    uint16_t updated_con_interval;
    uint16_t updated_latency;
    uint16_t updated_suo_to;

    uint8_t cccd_value;
    uint8_t uart_rx_value_changed;
    uint8_t uart_rx_value[60];
    uint8_t uart_tx_value[60];
    uint8_t spi1_rx_value[60];
    uint8_t spi1_tx_value[60];
    uint8_t spi1_rx_value_changed;
    uint8_t spi1_tx_value_changed;
    uint16_t spi1_rx_size;
    uint16_t spi1_tx_size;
    uint16_t bytes;
};

/* Support for the application manager and the application environment */
extern const struct ke_state_handler appm_default_handler;
extern ke_state_t appm_state[APP_IDX_MAX];
extern struct app_env_tag app_env;

/* Bluetooth Device Address */
extern uint8_t bdaddr[BDADDR_LENGTH];

/* ----------------------------------------------------------------------------
 * Function prototype definitions
 * --------------------------------------------------------------------------*/

/* Application support functions */
extern void App_Initialize(void);
extern void TIMER0_IRQHandler(void);
extern void SPI0_TX_IRQHandler(void);
extern void SPI1_RX_IRQHandler(void);
extern void SPI0_ERROR_IRQHandler (void);
extern void SPI1_ERROR_IRQHandler(void);
extern void TIMER1_IRQHandler (void);

/* Bluetooth baseband application support functions */
extern void App_Init(void);
extern bool Service_Add(void);
extern void Advertising_Start(void);
extern void Advertising_Stop(void);
extern void Connection_ParamUpdate(struct gapc_conn_param *conn_param);
extern void Connection_Disconnect(void);

/* Bluetooth event and message handlers */
extern int Msg_Handler(ke_msg_id_t const msgid, void *param,
                       ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int Advertising_Timeout(ke_msg_id_t const msgid, void const *param,
                               ke_task_id_t const dest_id,
                               ke_task_id_t const src_id);
extern int GAPM_DeviceReadyInd(ke_msg_id_t const msgid,
                               void const *param, ke_task_id_t const dest_id,
                               ke_task_id_t const src_id);
extern int GAPM_ProfileAddedInd(ke_msg_id_t const msgid,
                                struct gapm_profile_added_ind const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id);
extern int GAPM_CmpEvt(ke_msg_id_t const msgid,
                       struct gapm_cmp_evt const *param,
                       ke_task_id_t const dest_id,
                       ke_task_id_t const src_id);
extern int GAPC_CmpEvt(ke_msg_id_t const msgid,
                       struct gapc_cmp_evt const *param,
                       ke_task_id_t const dest_id,
                       ke_task_id_t const src_id);
extern int GAPC_GetDevInfoReqInd(ke_msg_id_t const msgid,
                                 struct gapc_get_dev_info_req_ind const *param,
                                 ke_task_id_t const dest_id,
                                 ke_task_id_t const src_id);
extern int Gapc_SetDevInfoReqInd(ke_msg_id_t const msgid,
                                 struct gapc_set_dev_info_req_ind const *param,
                                 ke_task_id_t const dest_id,
                                 ke_task_id_t const src_id);
extern int GAPC_DisconnectInd(ke_msg_id_t const msgid,
                              struct gapc_disconnect_ind const *param,
                              ke_task_id_t const dest_id,
                              ke_task_id_t const src_id);
extern int GAPC_ParamUpdatedInd(ke_msg_id_t const msgid,
                                struct gapc_param_updated_ind const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id);
extern int GAPC_ConnectionReqInd(ke_msg_id_t const msgid,
                                  struct gapc_connection_req_ind const *param,
                                  ke_task_id_t const dest_id,
                                  ke_task_id_t const src_id);
extern int GATTM_AddSvcRsp(ke_msg_id_t const msgid,
                           struct gattm_add_svc_rsp const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id);
extern int GATTC_ReadReqInd(ke_msg_id_t const msgid,
                            struct gattc_read_req_ind const *param,
                            ke_task_id_t const dest_id,
                            ke_task_id_t const src_id);
extern int GATTC_WriteReqInd(ke_msg_id_t const msgid,
                             struct gattc_write_req_ind const *param,
                             ke_task_id_t const dest_id,
                             ke_task_id_t const src_id);

extern int GAPC_BondReqInd(ke_msg_id_t const msgid,
                           struct gapc_bond_req_ind const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id);

extern int GAPC_BondInd(ke_msg_id_t const msgid,
                        struct gapc_bond_ind const *param,
                        ke_task_id_t const dest_id,
                        ke_task_id_t const src_id);

extern int GAPC_BondCmd(ke_msg_id_t const msg_id,
                        struct gapc_bond_cmd const *param,
                        ke_task_id_t const dest_id,
                        ke_task_id_t const src_id);
extern int GAPC_BondCfm(ke_msg_id_t const msg_id,
                        struct gapc_bond_cfm const *param,
                        ke_task_id_t const dest_id,
                        ke_task_id_t const src_id);

extern int GAPC_EncryptReqInd(ke_msg_id_t const msg_id,
                              struct gapc_encrypt_req_ind const *param,
                              ke_task_id_t const dest_id,
                              ke_task_id_t const src_id);
extern int GAPC_EncryptInd(ke_msg_id_t const msg_id,
                           struct gapc_encrypt_ind const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id);

extern int GAPM_ResolvAddrCmd(ke_msg_id_t const msg_id,
                              struct gapm_resolv_addr_cmd const *param,
                              ke_task_id_t const dest_id,
                              ke_task_id_t const src_id);

/* Bluetooth bonding support functions */
extern void Whitelist_Full(void);

extern unsigned int Empty_Whitelist(void);

extern unsigned int WriteEnable(void);

extern unsigned int WordWrite(BondInfo_Type list, unsigned int index);

extern unsigned int checkBonded(uint8_t *addr);

extern unsigned int checkSpaceAvailable(void);

extern unsigned int searchAvailableIndex(uint8_t index);

extern void refreshList(uint8_t index);

extern void uint322bcd(uint32_t data_var);

/* Bluetooth battery service support functions */
extern int Batt_EnableRsp(ke_msg_id_t const msgid,
                          struct bass_enable_rsp const *param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id);
extern int Batt_LevelNtfCfgInd(ke_msg_id_t const msgid,
                               struct bass_batt_level_ntf_cfg_ind const *param,
                               ke_task_id_t const dest_id,
                               ke_task_id_t const src_id);
extern int Batt_LevelUpdateRsp(ke_msg_id_t const msgid,
                               struct bass_batt_level_upd_rsp const *param,
                               ke_task_id_t const dest_id,
                               ke_task_id_t const src_id);
extern void Batt_ServiceAdd(void);
extern void Batt_ServiceEnable(uint8_t conidx);
extern void Batt_LevelUpdateSend(uint8_t batt_lvl, uint8_t svcidx);

/* Bluetooth custom service support functions */
extern void CustomService_ServiceAdd(void);
extern void CustomService_SendNotification(uint8_t conidx, uint16_t handle,
                                           uint8_t *value, uint8_t length);
extern void CustomService_SendNotification_BCD(uint8_t conidx, uint16_t handle,
        uint8_t *value, uint8_t length);
/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif

#endif /* APP_H */
