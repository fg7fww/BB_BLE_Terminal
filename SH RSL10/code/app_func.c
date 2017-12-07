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
 * app_func.c
 * - Application task scheduler
 * ----------------------------------------------------------------------------
 * $Revision: 1.31 $
 * $Date: 2016/12/08 22:16:21 $
 * ------------------------------------------------------------------------- */

#include "../include/app.h"

const struct ke_task_desc TASK_DESC_APP = { NULL, &appm_default_handler,
        appm_state, APPM_STATE_MAX, APP_IDX_MAX };

/* List of functions used to create the database */
const appm_add_svc_func_t appm_add_svc_func_list[APPM_SVC_LIST_STOP] = {
                          (appm_add_svc_func_t) Batt_ServiceAdd,
                          (appm_add_svc_func_t) CustomService_ServiceAdd, };

/* Application Environment Structure */
struct app_env_tag app_env;

/* Bluetooth Device Address */
uint8_t bdaddr[BDADDR_LENGTH];

/* ----------------------------------------------------------------------------
 * Function      : void App_Init(void)
 * ----------------------------------------------------------------------------
 * Description   : Initialize application environment
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void App_Init(void)
{
    uint8_t *ptr = (uint8_t *) DEVICE_INFO_BLUETOOTH_ADDR;
    uint8_t tmp[2][BDADDR_LENGTH] = {
            {0xff,0xff,0xff,0xff,0xff,0xff},
            {0x00,0x00,0x00,0x00,0x00,0x00}
            };
    uint8_t default_addr[BDADDR_LENGTH] = PRIVATE_BDADDR;

    /* Reset the application manager environment */
    memset(&app_env, 0, sizeof(app_env));

    /* Create application task */
    ke_task_create(TASK_APP, &TASK_DESC_APP);

    /* Initialize task state */
    app_env.state = APPM_INIT;

    app_env.batt_lvl = 0;
    app_env.num_batt_read = 0;
    app_env.send_batt_ntf = 0;

    /* 8*1.25 ms  */
    app_env.con_interval = 8;

    /* 3 seconds  */
    app_env.time_out = 300;

    app_env.uart_tx_value[0] = 0xEF;
    memset(app_env.uart_rx_value, 0, 20);
    memset(app_env.spi1_rx_value, 0, 20);
    app_env.start_hdl = 0;
    app_env.uart_rx_value_changed = 0;
    app_env.cccd_value = 1;
    app_env.spi1_rx_value_changed = 0;
    app_env.spi1_tx_value_changed = 0;

    /* Use the device's public address if an address is available at
     * DEVICE_INFO_BLUETOOTH_ADDR (located in NVR3). If this address is
     * not defined (all ones) use a pre-defined private address for this
     * application */

    if(memcmp(ptr, &tmp[0][0] , BDADDR_LENGTH) == 0 || memcmp(ptr, &tmp[1][0] , BDADDR_LENGTH) == 0)
    {
        memcpy(bdaddr, default_addr, sizeof(uint8_t) * BDADDR_LENGTH);
    }
    else
    {
        memcpy(bdaddr, ptr, sizeof(uint8_t) * BDADDR_LENGTH);
    }
}

/* ----------------------------------------------------------------------------
 * Function      : bool Service_Add(void)
 * ----------------------------------------------------------------------------
 * Description   : Add the next service in the service list,
 *                 calling the appropriate add service function
 * Inputs        : None
 * Outputs       : return value - Indicates if any service is remaining
 *                    to be added
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
bool Service_Add(void)
{
    /* Indicate if more services need to be added in the database */
    bool more_svc = false;

    /* Check if another should be added in the database */
    if (app_env.next_svc != APPM_SVC_LIST_STOP)
    {
        ASSERT_ERR(appm_add_svc_func_list[app_env.next_svc] != NULL);

        /* Call the function used to add the required service */
        appm_add_svc_func_list[app_env.next_svc]();

        /* Select the next service to add */
        app_env.next_svc++;
        more_svc = true;
    }

    return more_svc;
}

/* ----------------------------------------------------------------------------
 * Function      : void Connection_Disconnect(void)
 * ----------------------------------------------------------------------------
 * Description   : Disconnect the link
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Connection_Disconnect(void)
{
    struct gapc_disconnect_cmd *cmd;
    cmd=KE_MSG_ALLOC(GAPC_DISCONNECT_CMD,
                     KE_BUILD_ID(TASK_GAPC, app_env.conidx),
                     TASK_APP, gapc_disconnect_cmd);

    app_env.state = APPM_READY;

    cmd->operation = GAPC_DISCONNECT;
    cmd->reason = CO_ERROR_REMOTE_USER_TERM_CON;

   /* Send the message */
    ke_msg_send(cmd);
}

/* ----------------------------------------------------------------------------
 * Function      : void Advertising_Start(void)
 * ----------------------------------------------------------------------------
 * Description   : Send a start advertising
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Advertising_Start(void)
{
    /* Check if the advertising procedure is already is progress */
    /*  Device Name Length */
    uint8_t device_name_length;
    uint8_t device_name_avail_space;
    uint8_t device_name_temp_buf[APP_DEVICE_NAME_LENGTH_MAX];
    uint8_t scan_rsp[SCAN_RSP_DATA_LEN] = APP_SCNRSP_DATA;

    /* Prepare the GAPM_START_ADVERTISE_CMD message */
    struct gapm_start_advertise_cmd *cmd;

    if (app_env.state == APPM_READY)
    {
        cmd = KE_MSG_ALLOC(GAPM_START_ADVERTISE_CMD, TASK_GAPM, TASK_APP,
                           gapm_start_advertise_cmd);
        cmd->op.addr_src = GAPM_STATIC_ADDR;
        cmd->channel_map = APP_ADV_CHMAP;

        cmd->intv_min = APP_ADV_INT_MIN;
        cmd->intv_max = APP_ADV_INT_MAX;

        cmd->op.code = GAPM_ADV_UNDIRECT;
        cmd->op.state = 0;
        cmd->info.host.mode = GAP_GEN_DISCOVERABLE;

        /* Set the advertising Data and the Scan Response Data */
        /* Flag value is set by the GAP manager  */
        cmd->info.host.adv_data_len = ADV_DATA_LEN - 3;
        cmd->info.host.scan_rsp_data_len = SCAN_RSP_DATA_LEN;

        cmd->info.host.adv_filt_policy = 0;
        cmd->info.host.adv_data_len = 0;

        /* Scan Response Data  */
        cmd->info.host.scan_rsp_data_len = 0;

        memcpy(&cmd->info.host.scan_rsp_data[cmd->info.host.scan_rsp_data_len],
               scan_rsp, APP_SCNRSP_DATA_LEN);
        cmd->info.host.scan_rsp_data_len += APP_SCNRSP_DATA_LEN;

        /* Get remaining space in the advertising Data -
         * 2 bytes are used for name length/flag */
        device_name_avail_space = (ADV_DATA_LEN - 3)
                                  - cmd->info.host.adv_data_len - 2;

        /* Check if data can be added to the advertising data */
        if (device_name_avail_space > 0)
        {
            /* Get the device name to add in the advertising Data
             * (default one or from FLASH one) */
            device_name_length = strlen(APP_DFLT_DEVICE_NAME);

            /* Get default device name (no name if not enough space) */
            memcpy(&device_name_temp_buf[0],
            APP_DFLT_DEVICE_NAME, device_name_length);

            if (device_name_length > 0)
            {
                /* Check available space */
                device_name_length = co_min(device_name_length,
                        device_name_avail_space);
                cmd->info.host.adv_data[cmd->info.host.adv_data_len] =
                        device_name_length + 1;

                /* Fill device name flag */
                cmd->info.host.adv_data[cmd->info.host.adv_data_len + 1] =
                        '\x09';

                /* Copy device name  */
                memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len
                       + 2], device_name_temp_buf, device_name_length);

                /* Update advertising data length */
                cmd->info.host.adv_data_len += (device_name_length + 2);
            }
        }

        /* Send the message */
        ke_msg_send(cmd);

        /* Set the state of the task to APPM_ADVERTISING  */
        app_env.state = APPM_ADVERTISING;
    }
}

/* ----------------------------------------------------------------------------
 * Function      : void Advertising_Stop(void)
 * ----------------------------------------------------------------------------
 * Description   : Sends a stop advertising request
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Advertising_Stop(void)
{
    struct gapm_cancel_cmd *cmd;

    if (app_env.state == APPM_ADVERTISING)
    {
        /* Go into ready state */
        app_env.state = APPM_READY;

        /* Prepare the GAPM_CANCEL_CMD message  */
        cmd = KE_MSG_ALLOC(GAPM_CANCEL_CMD, TASK_GAPM,
                           TASK_APP, gapm_cancel_cmd);

        cmd->operation = GAPM_CANCEL;

        /* Send the message */
        ke_msg_send(cmd);
    }
}

/* ----------------------------------------------------------------------------
 * Function      : void Connection_ParamUpdate(struct gapc_conn_param *conn_param)
 * ----------------------------------------------------------------------------
 * Description   : Send a parameter update request to GAP
 * Inputs        : input parameters in format of struct gapc_conn_param
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Connection_ParamUpdate(struct gapc_conn_param *conn_param)
{
    struct gapc_param_update_cmd *cmd;
    cmd = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CMD,
                       KE_BUILD_ID(TASK_GAPC,app_env.conidx), TASK_APP,
                       gapc_param_update_cmd);

    cmd->operation = GAPC_UPDATE_PARAMS;
    cmd->intv_min = conn_param->intv_min;
    cmd->intv_max = conn_param->intv_max;
    cmd->latency = conn_param->latency;
    cmd->time_out = conn_param->time_out;

    /* Not used by a slave device */
    cmd->ce_len_min = 0xffff;
    cmd->ce_len_max = 0xffff;

   /* Send the message */
    ke_msg_send(cmd);
}

/* ----------------------------------------------------------------------------
 * Function      : void Batt_ServiceAdd(void)
 * ----------------------------------------------------------------------------
 * Description   : Send request to add battery profile in kernel and DB
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Batt_ServiceAdd(void)
{
    struct bass_db_cfg* db_cfg;
    struct gapm_profile_task_add_cmd *req;
    req = KE_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD, TASK_GAPM,
                           TASK_APP, gapm_profile_task_add_cmd,
                           sizeof(struct bass_db_cfg));

    /* Fill message */
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl = PERM(SVC_AUTH, DISABLE);
    req->prf_task_id = TASK_ID_BASS;
    req->app_task = TASK_APP;
    req->start_hdl = 0;

    /* Set parameters  */
    db_cfg = (struct bass_db_cfg*) req->param;
    db_cfg->bas_nb = 1;
    db_cfg->features[0] = BAS_BATT_LVL_NTF_SUP;
    db_cfg->batt_level_pres_format[0].description = 0;
    db_cfg->batt_level_pres_format[0].exponent = 0;
    db_cfg->batt_level_pres_format[0].format = 0x4;
    db_cfg->batt_level_pres_format[0].name_space = 1;
    db_cfg->batt_level_pres_format[0].unit = ATT_UNIT_PERCENTAGE;

   /* Send the message */
    ke_msg_send(req);
}

/* ----------------------------------------------------------------------------
 * Function      : void Batt_ServiceEnable(uint8_t conidx)
 * ----------------------------------------------------------------------------
 * Description   : Send request for enabling battery service to battery server
 * Inputs        : connection index
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Batt_ServiceEnable(uint8_t conidx)
{
    /* Allocate the GATT notification message  */
    struct bass_enable_req * req;
    struct bass_env_tag* bass_env;
    bass_env = PRF_ENV_GET(BASS, bass);

    app_env.batt_lvl = 40;

    /* Allocate the message  */
    req = KE_MSG_ALLOC(BASS_ENABLE_REQ,
                       prf_src_task_get(&(bass_env->prf_env), conidx),
                       TASK_APP, bass_enable_req);

    /* Fill in the parameter structure */
    req->conidx = conidx;
    req->ntf_cfg = 0;
    req->old_batt_lvl[0] = 90;
    req->old_batt_lvl[1] = 10;

   /* Send the message */
    ke_msg_send(req);
}

/* ----------------------------------------------------------------------------
 * Function      : void Batt_LevelUpdateSend(uint8_t batt_lvl, uint8_t bas_nb)
 * ----------------------------------------------------------------------------
 * Description   : Send request for sending battery level notification
 * Inputs        : - batt_lvl       - battery level
 *                 - bas_nb         - battery server instance number
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Batt_LevelUpdateSend(uint8_t batt_lvl, uint8_t bas_nb)
{
    struct bass_env_tag* bass_env;
    struct bass_batt_level_upd_req * req;
    uint32_t msg_size;

    ASSERT_ERR(batt_lvl <= BAS_BATTERY_LVL_MAX);

    bass_env = PRF_ENV_GET(BASS, bass);

    /* Allocate the message */
    req = KE_MSG_ALLOC(BASS_BATT_LEVEL_UPD_REQ,
                       prf_src_task_get(&(bass_env->prf_env), 0), TASK_APP,
                       bass_batt_level_upd_req);

    /* Fill in the parameter structure */
    req->bas_instance = bas_nb;
    req->batt_level = batt_lvl;

   /* Send the message */
    ke_msg_send(req);
}

/* ----------------------------------------------------------------------------
 * Function      : void CustomService_ServiceAdd(void)
 * ----------------------------------------------------------------------------
 * Description   : Send request to add custom profile into the attribute database
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void CustomService_ServiceAdd(void)
{
    struct gattm_add_svc_req *req;
    uint8_t svc_uuid[ATT_UUID_128_LEN] = CS_SVC_UUID;
    uint8_t att_uuid[5][ATT_UUID_128_LEN] = {
                                             CS_CHARACTERISTIC_UUID,
                                             CS_RX_VALUE_UUID,
                                             CS_CCC_UUID,
                                             CS_CHARACTERISTIC_UUID,
                                             CS_TX_VALUE_UUID, };

    req = KE_MSG_ALLOC_DYN(GATTM_ADD_SVC_REQ, TASK_GATTM, TASK_APP,
                           gattm_add_svc_req,
                           5 * sizeof(struct gattm_att_desc));

    /* Fill message */

    /* Set a service with 5 attributes and a 16-byte UUID and flag that it is
     * a primary service */
    req->svc_desc.start_hdl = 0;
    req->svc_desc.task_id = TASK_APP;
    req->svc_desc.perm = 0x40;
    req->svc_desc.nb_att = 5;
    memcpy(&req->svc_desc.uuid[0], &svc_uuid[0], ATT_UUID_128_LEN);

    /* Set characteristic declaration attribute with read enabled,
     * its value must always exist in the next attribute */
    req->svc_desc.atts[0].max_len = 0;
    req->svc_desc.atts[0].perm = 0x200;
    req->svc_desc.atts[0].ext_perm = 0;
    memcpy(&req->svc_desc.atts[0].uuid, &att_uuid[0][0], ATT_UUID_16_LEN);

    /* Set characteristic value attribute with maximum length of 1 byte and
     * permission set to read and notification enabled, ext_perm
     * is set to indicate that UUID length is 128 bits and it is not saved
     * in database */
    req->svc_desc.atts[1].max_len = 1;
    req->svc_desc.atts[1].perm = 0x1200;
    req->svc_desc.atts[1].ext_perm = 0xc000;
    memcpy(&req->svc_desc.atts[1].uuid, &att_uuid[1][0], ATT_UUID_128_LEN);

    /* Set client characteristic configuration attribute
     * with read and write request enabled, its value is not saved in database
     * and application should keep its value and sends response once it
     * receives a read value message from GATT controller  */
    req->svc_desc.atts[2].max_len = 0;
    req->svc_desc.atts[2].perm = 0xa00;
    req->svc_desc.atts[2].ext_perm = 0x000;
    memcpy(&req->svc_desc.atts[2].uuid, &att_uuid[2][0], ATT_UUID_16_LEN);

    /* Set characteristic declaration attribute, its value must always exist
     * in the next attribute */
    req->svc_desc.atts[3].max_len = 0;
    req->svc_desc.atts[3].perm = 0x0;
    req->svc_desc.atts[3].ext_perm = 0x0;
    memcpy(&req->svc_desc.atts[3].uuid, &att_uuid[3][0], ATT_UUID_16_LEN);

    /* Set characteristic value attribute with maximum length of 20 byte and
     * permission set to read, write, and write without response enabled,
     * ext_perm is set to indicate that UUID length id 128 bits and it is not
     * saved in database */
    req->svc_desc.atts[4].max_len = 20;
    req->svc_desc.atts[4].perm = 0x0e00;
    req->svc_desc.atts[4].ext_perm = 0xc000;
    memcpy(&req->svc_desc.atts[4].uuid, &att_uuid[4][0], ATT_UUID_128_LEN);

   /* Send the message */
    ke_msg_send(req);
}

/* ----------------------------------------------------------------------------
 * Function      : void CustomService_SendNotification(uint8_t conidx,
 *                                                     uint16_t handle,
 *                                                     uint8_t *value,
 *                                                       uint8_t length)
 * ----------------------------------------------------------------------------
 * Description   : Send a notification to peer
 * Inputs        : - conidx       - connection index
 *                 - handle       - handle value
 *                 - value        - pointer to value
 *                 - length       - length of value
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void CustomService_SendNotification(uint8_t conidx, uint16_t handle,
                                    uint8_t *value, uint8_t length)
{
    struct gattc_send_evt_cmd *cmd;

    cmd = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                           KE_BUILD_ID(TASK_GATTC, app_env.conidx), TASK_APP,
                           gattc_send_evt_cmd,
                           length * sizeof(uint8_t));

    cmd->handle = handle;
    cmd->length = length;
    cmd->operation = GATTC_NOTIFY;
    cmd->seq_num = 0;
    memcpy(cmd->value, value, length);

    /* Send the message */
     ke_msg_send(cmd);
}

void CustomService_SendNotification_BCD(uint8_t conidx, uint16_t handle,
                                    uint8_t *value, uint8_t length)
{
    struct gattc_send_evt_cmd *cmd;

    cmd = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                           KE_BUILD_ID(TASK_GATTC, app_env.conidx), TASK_APP,
                           gattc_send_evt_cmd,
                           length * sizeof(uint8_t));

    cmd->handle = handle;
    cmd->length = length;
    cmd->operation = GATTC_NOTIFY;
    cmd->seq_num = 0;
    memcpy(cmd->value, value, length);

    /* Send the message */
     ke_msg_send(cmd);
}

/* ----------------------------------------------------------------------------
 * Function      : void Empty_Whitelist(void)
 * ----------------------------------------------------------------------------
 * Description   : Resets entire NVR2 to high
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : NVR2 write is enabled
 * ------------------------------------------------------------------------- */
unsigned int Empty_Whitelist(void)
{
    FlashStatus result;

    result = Flash_EraseSector(0x00080800);
    if (result != FLASH_ERR_NONE)
    {
        return ((unsigned int) result | ERRNO_FLASHLIB_ERROR_MASK);
    }

    return ERRNO_NO_ERROR;
}

/* ----------------------------------------------------------------------------
 * Function      : unsigned WriteEnable(void)
 * ----------------------------------------------------------------------------
 * Description   : Enables writing to NVR2
 * Inputs        : None
 * Outputs       : Result of enabling
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
unsigned int WriteEnable()
{
    FLASH->NVR_CTRL = NVR2_WRITE_ENABLE;
    FLASH->NVR_WRITE_UNLOCK = FLASH_NVR_KEY;

    /* Confirm all of the flash memories unlocked */
    if ((FLASH->IF_STATUS & 0x3FF) != 0x377)
    {
        return ERRNO_FLASH_UNRESTRICT_FAILED;
    }

    return ERRNO_NO_ERROR;
}

/* ----------------------------------------------------------------------------
 * Function      : unsigned int WordWrite(BondInfo_Type list,
 *                                        unsigned int index)
 * ----------------------------------------------------------------------------
 * Description   : Writes the stored bonded data from the input struct, list
 * Inputs        : - list      - list of BondInfo_Type with bonding data,
 *                 - index     - location of where in the flash input struct
 *                                  will be written to
 * Outputs       : result of flash writing
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
unsigned int WordWrite(BondInfo_Type list, unsigned int index)
{
    FlashStatus result;
    unsigned int offSet;
    uint32_t *startAddr;
    uint32_t tmp32;
    uint32_t tmp32_2;

    /* Address where flash writing is calculated. Base address is NVR2 and has
     * an offset of 72 bytes (size of bonding information)
     * information*/
    offSet = ((index-1) * sizeof(BondInfo_Type));

    /* Start of the address where the bonding information will be written */
    startAddr = (uint32_t *) (FLASH_NVR2_BASE + offSet);

    result = Flash_WriteBuffer((unsigned int)startAddr,
                               (unsigned int)(sizeof(BondInfo_Type)/4),
                               (unsigned int*)&list);

    if (result != FLASH_ERR_NONE)
    {
        return ((unsigned int) result | ERRNO_FLASHLIB_ERROR_MASK);
    }
    return ERRNO_NO_ERROR;
}

/* ----------------------------------------------------------------------------
 * Function      : unsigned int checkBonded(uint8_t *addr)
 * ----------------------------------------------------------------------------
 * Description   : Checks if the bonding information for the peer device exists
 * Inputs        : addr      - Address of the peer device
 * Outputs       : Index of the peer device in the flash if it is bonded. If
 *                 not, return APP_NOT_BONDED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
unsigned int checkBonded(uint8_t *addr)
{
    unsigned int result;
    BondInfo_Type list[SIZEOF_BONDLIST];

    memcpy(&list[1],(uint8_t *)FLASH_NVR2_BASE, (SIZEOF_BONDLIST-1)
            * sizeof(BondInfo_Type));
    int checker = 0;
    for(int i = 1; i < SIZEOF_BONDLIST; i++)
    {
        if(list[i].STATE != 0 || ((list[i].STATE & 0x80) == 0x80))
        {
            /* length of addr is 6 byte */
            for(int j = 0; j < BDADDR_LENGTH; j++)
            {
                if(list[i].ADDR[j] == addr[j])
                {
                    checker ++;
                }

                /* The master device's address already exists in the list */
                if(checker == BDADDR_LENGTH)
                {
                    /* The master device is bonded already, and return it's
                     *  location in the flash */
                    return i;
                }
            }
        }
        checker = 0;
    }
    /* The master device is not bonded previously */
    return APP_NOT_BONDED;
}

/* ----------------------------------------------------------------------------
 * Function      : unsigned int checkSpaceAvailable(void)
 * ----------------------------------------------------------------------------
 * Description   : Checks if there is a space in the flash for a new bonding
 *                 information
 * Inputs        : None
 * Outputs       : Result of checking for a space
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
unsigned int checkSpaceAvailable(void)
{
    unsigned int result;
    BondInfo_Type list[SIZEOF_BONDLIST];

    memcpy(&list[1],(uint8_t *)FLASH_NVR2_BASE, (SIZEOF_BONDLIST-1)
           * sizeof(BondInfo_Type));

    for(int i = 1; i < SIZEOF_BONDLIST; i++)
    {
        /* Length of addr is 6 byte*/
        /* If there is an empty space, return */
        if(list[i].STATE == 0xFF || list[i].STATE == 0x00)
        {
            return LIST_NOT_FULL;
        }

    }
    return LIST_FULL;
}

/* ----------------------------------------------------------------------------
 * Function      : unsigned int searchAvailableIndex(void)
 * ----------------------------------------------------------------------------
 * Description   : Determine the empty index where the new index can be saved.
 *                 If all the indexes are occupied, refresh the invalidated
 *                 indexes to use.
 *                 information can be saved
 * Inputs        : uint8_t index    - Index of the master device.
 *                                    It will be either 0xFF or the value where
 *                                    its previous information exists
 * Outputs       : Index where the new information can be saved or an error
 *                 code if no indexes are available
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
unsigned int searchAvailableIndex(uint8_t index)
{
    int invalidate = 0;

    unsigned int result;
    BondInfo_Type list[SIZEOF_BONDLIST];

    memcpy(&list[1],(uint8_t *)FLASH_NVR2_BASE, (SIZEOF_BONDLIST-1)
           * sizeof(BondInfo_Type));
    for(unsigned int i = 1; i < SIZEOF_BONDLIST; i++)
    {
        /* Search for an empty index */
        if(list[i].STATE == 0xFF)
        {
            /* This line checks if the master device was previously bonded.
             * If it was, it will invalidate the previous information */
            if(index != 0xFF && index != 0x00)
            {
                index = index & 0x7F;
                list[index].STATE = 0;
                result = WordWrite(list[index],index);
            }
            return i;
        }
    }

    /* The list is all occupied but there are some invalid entries
     * that could be removed, so remove and add the new data */
    refreshList(index);

    for(unsigned int i = 1; i < SIZEOF_BONDLIST; i++)
    {
        /* Search for an empty index */
        if(list[i].STATE == 0xFF)
        {
            return i;
        }
    }

    return INDEX_SEARCH_FAIL;
}

/* ----------------------------------------------------------------------------
 * Function      : void refreshList(uint8_t index)
 * ----------------------------------------------------------------------------
 * Description   : Deletes the NVR2, where bonding information is saved and
 *                 rewrite the valid information to make space
 * Inputs        : uint8_t index    - Index where the previous bonding
 *                                    information of master device is saved
 * Outputs       : None
 * Assumptions   : An index will be available after the function is called
 * ------------------------------------------------------------------------- */
void refreshList(uint8_t index)
{
    /* Erase the NVR2 to remove invalid bonding information and rewrite
     * the valid bonding information */
    unsigned int result;
    BondInfo_Type list[SIZEOF_BONDLIST];
    memcpy(&list[1],(uint8_t *)FLASH_NVR2_BASE, (SIZEOF_BONDLIST-1)
            * sizeof(BondInfo_Type));
    result = Flash_EraseSector(FLASH_NVR2_BASE);
    list[index].STATE = 0;

    for(int k = 1; k < SIZEOF_BONDLIST; k++)
    {
        /* Rewrite valid indexes*/
        if(list[k].STATE != 0)
        {
            result = WordWrite(list[k],k);
        }
    }
}
/* ----------------------------------------------------------------------------
 * Function      : void uint322bcd(uint32_t data_var)
 * ----------------------------------------------------------------------------
 * Description   : Conversion of binar to the BCD
 *
 * Inputs        : uint32_t     - binar value
 *
 * Outputs       : None
 * Assumptions   :
 * ------------------------------------------------------------------------- */
void uint322bcd(uint32_t data_var)
{
	uint32_t tmp;
	tmp=0;
	int i;
	for(i=9; data_var>0; i--){
		tmp=data_var/10;
		app_env.uart_tx_value[i]=(uint8_t)(data_var-10*tmp);
		data_var=tmp;
	}
	for(; i>=0; i--){
		app_env.uart_tx_value[i]=0;
	}
}
