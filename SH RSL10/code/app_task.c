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
 * app_task.c
 * - Application task handler
 * ----------------------------------------------------------------------------
 * $Revision: 1.30 $
 * $Date: 2016/12/08 22:16:21 $
 * ------------------------------------------------------------------------- */

#include "../include/app.h"
#include <rsl10_map_nvr.h>

BondInfo_Type currentMaster;

/* State and event handler definition */
const struct ke_msg_handler appm_default_state[] =
{
    /* Note: The first message is the last message checked by the kernel
     * so put the default handler on top */
    { KE_MSG_DEFAULT_HANDLER, (ke_msg_func_t) Msg_Handler },
    { APP_ADV_TIMEOUT_TIMER, (ke_msg_func_t) Advertising_Timeout },
    { GAPM_DEVICE_READY_IND, (ke_msg_func_t) GAPM_DeviceReadyInd },
    { GAPM_CMP_EVT, (ke_msg_func_t) GAPM_CmpEvt },
    { GAPM_PROFILE_ADDED_IND, (ke_msg_func_t) GAPM_ProfileAddedInd },
    { GAPC_CONNECTION_REQ_IND, (ke_msg_func_t) GAPC_ConnectionReqInd },
    { GAPC_CMP_EVT, (ke_msg_func_t) GAPC_CmpEvt },
    { GAPC_DISCONNECT_IND, (ke_msg_func_t) GAPC_DisconnectInd },
    { GAPC_GET_DEV_INFO_REQ_IND, (ke_msg_func_t) GAPC_GetDevInfoReqInd },
    { GAPC_PARAM_UPDATED_IND, (ke_msg_func_t) GAPC_ParamUpdatedInd },
    { GATTM_ADD_SVC_RSP, (ke_msg_func_t) GATTM_AddSvcRsp },
    { GATTC_READ_REQ_IND, (ke_msg_func_t) GATTC_ReadReqInd },
    { GATTC_WRITE_REQ_IND, (ke_msg_func_t) GATTC_WriteReqInd },
    { BASS_ENABLE_RSP, (ke_msg_func_t) Batt_EnableRsp },
    { BASS_BATT_LEVEL_NTF_CFG_IND, (ke_msg_func_t) Batt_LevelNtfCfgInd },
    { BASS_BATT_LEVEL_UPD_RSP, (ke_msg_func_t) Batt_LevelUpdateRsp },
    { GAPC_BOND_REQ_IND, (ke_msg_func_t) GAPC_BondReqInd },
    { GAPC_BOND_IND, (ke_msg_func_t) GAPC_BondInd },
    { GAPC_ENCRYPT_REQ_IND, (ke_msg_func_t) GAPC_EncryptReqInd },
    { GAPC_ENCRYPT_IND, (ke_msg_func_t) GAPC_EncryptInd }
};

/* Specifies the message handlers that are common to all states. */
const struct ke_state_handler appm_default_handler =
             KE_STATE_HANDLER(appm_default_state);

/* Defines the place holder for the states of all the task instances. */
ke_state_t appm_state[APP_IDX_MAX];

/* ----------------------------------------------------------------------------
 * Function      : int Advertising_Timeout(ke_msg_id_t const msg_id,
 *                                         void const *param,
 *                                         ke_task_id_t const dest_id,
 *                                         ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle a timeout received for application advertising
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameter (unused)
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int Advertising_Timeout(ke_msg_id_t const msg_id, void const *param,
                        ke_task_id_t const dest_id,
                        ke_task_id_t const src_id)
{
    /* Stop advertising */
    Advertising_Stop();

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GAPM_DeviceReadyInd(ke_msg_id_t const msg_id,
 *                                         void const *param,
 *                                         ke_task_id_t const dest_id,
 *                                         ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle reception of device ready indication
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameter (unused)
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GAPM_DeviceReadyInd(ke_msg_id_t const msg_id, void const *param,
                        ke_task_id_t const dest_id,
                        ke_task_id_t const src_id)
{
    struct gapm_reset_cmd* cmd;

    /* Application has not been initialized */
    ASSERT_ERR(app_env.state == APPM_INIT);

    /* Reset the stack */
    cmd = KE_MSG_ALLOC(GAPM_RESET_CMD, TASK_GAPM, TASK_APP, gapm_reset_cmd);

    cmd->operation = GAPM_RESET;

    ke_msg_send(cmd);

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GAPM_ProfileAddedInd(ke_msg_id_t const msg_id,
 *                                          struct gapm_profile_added_ind
 *                                          const *param,
 *                                          ke_task_id_t const dest_id,
 *                                          ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle the received result of adding a profile to attributes
 *                 database
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gapm_profile_added_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GAPM_ProfileAddedInd(ke_msg_id_t const msg_id,
                         struct gapm_profile_added_ind const *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    /* Retrieve identifier of the task from received message */
    ke_task_id_t src_task_id = MSG_T(msg_id);

    if (app_env.state == APPM_CREATE_DB)
    {
        /* Add the next requested service  */
        if (!Service_Add())
        {
            /* Go to the ready state */
            app_env.state = APPM_READY;

            /* No more service to add, start advertising */
            Advertising_Start();
        }
    }
    else
    {
        ke_msg_send_basic(GAPM_DEVICE_READY_IND, TASK_APP, TASK_GAPM);
    }

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GAPM_CmpEvt(ke_msg_id_t const msg_id,
 *                                 struct gapm_cmp_evt
 *                                 const *param,
 *                                 ke_task_id_t const dest_id,
 *                                 ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle the reception of GAPM complete event
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gapm_cmp_evt
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GAPM_CmpEvt(ke_msg_id_t const msg_id, struct gapm_cmp_evt const *param,
                ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gapm_set_dev_config_cmd* cmd;

    switch (param->operation)
    {
        case (GAPM_RESET):
        {

            if (param->status == GAP_ERR_NO_ERROR)
            {
                /* Set Device configuration */
                cmd = KE_MSG_ALLOC(GAPM_SET_DEV_CONFIG_CMD, TASK_GAPM, TASK_APP,
                                   gapm_set_dev_config_cmd);
                cmd->operation = GAPM_SET_DEV_CONFIG;

                cmd->role = GAP_ROLE_PERIPHERAL;
                cmd->renew_dur = 15000;
                cmd->addr_type = GAPM_CFG_ADDR_PRIVATE;

                memcpy(cmd->addr.addr, bdaddr, BDADDR_LENGTH);

                memset(&cmd->irk.key[0], 0, KEY_LEN);

                cmd->pairing_mode = GAPM_PAIRING_SEC_CON | GAPM_PAIRING_LEGACY;
                cmd->gatt_start_hdl = 0;
                cmd->max_mtu = 0x200;
                cmd->max_mps = 0x200;
                cmd->att_cfg = 0x80;
                cmd->sugg_max_tx_octets = 0x1b;
                cmd->sugg_max_tx_time = 0x148;

                #if RSL10_CID != 8101
                cmd->tx_pref_rates = 0;
                cmd->rx_pref_rates = 0;
                cmd->max_nb_lecb = 0x0;
                cmd->audio_cfg = 1;
                #endif

                /* Send message */
                ke_msg_send(cmd);
            }
            else
            {
                ASSERT_ERR(0);
            }
        } break;

        /* Device configuration updated */
        case (GAPM_SET_DEV_CONFIG):
        {
            ASSERT_ERR(param->status == GAP_ERR_NO_ERROR);

            /* Go to the create database state */
            app_env.state = APPM_CREATE_DB;

            /* Add the first required service in the database */
            if (!Service_Add())
            {
                /* Go to the ready state */
                app_env.state = APPM_READY;

                /* Start advertising since there is no service to add
                 * to the attribute database */
                Advertising_Start();
            }
        } break;

        default:
        {
            /* Drop the message */
        } break;

    }

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GAPC_GetDevInfoReqInd(ke_msg_id_t const msg_id,
 *                                           struct gapc_get_dev_info_req_ind
 *                                           const *param,
 *                                           ke_task_id_t const dest_id,
 *                                           ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle message device info request received
 *                    from GAP controller
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gapc_get_dev_info_req_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GAPC_GetDevInfoReqInd(ke_msg_id_t const msg_id,
                          struct gapc_get_dev_info_req_ind const *param,
                          ke_task_id_t const dest_id, ke_task_id_t const src_id)
{

    /* Allocate message */
    struct gapc_get_dev_info_cfm *cfm;
    cfm = KE_MSG_ALLOC_DYN(GAPC_GET_DEV_INFO_CFM,
                           src_id, dest_id, gapc_get_dev_info_cfm, 30);
    uint8_t len;

    switch (param->req)
    {
        case GAPC_DEV_NAME:
        {
            cfm->req = GAPC_DEV_NAME;
            len = strlen(APP_DFLT_DEVICE_NAME);
            memcpy(&cfm->info.name.value[0], APP_DFLT_DEVICE_NAME, len);
            cfm->info.name.length = len;
        } break;

        case GAPC_DEV_APPEARANCE:
        {
            /* Set the device appearance (No appearance) */
            cfm->info.appearance = 0;
            cfm->req = GAPC_DEV_APPEARANCE;
            cfm->info.appearance = GAPM_WRITE_DISABLE;
        } break;

        case GAPC_DEV_SLV_PREF_PARAMS:
        {
            /* Slave preferred connection interval (minimum) */
            cfm->info.slv_params.con_intv_min = 8;
            /* Slave preferred connection interval (maximum) */
            cfm->info.slv_params.con_intv_max = 10;
            /* Slave preferred connection latency */
            cfm->info.slv_params.slave_latency = 0;
            /* Slave preferred link supervision timeout */
            cfm->info.slv_params.conn_timeout = 200; /* 200 * 10 ms */
            cfm->req = GAPC_DEV_SLV_PREF_PARAMS;
        } break;

        default: /* Do Nothing */
          break;
    }
        /* Send message */
    ke_msg_send(cfm);
    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GAPC_ConnectionReqInd(ke_msg_idd_t const msg_id,
 *                                           struct gapc_connection_req_ind
 *                                           const *param,
 *                                           ke_task_id_t const dest_id,
 *                                           ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle connection indication message
 *                    received from GAP controller
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gapc_connection_req_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GAPC_ConnectionReqInd(ke_msg_id_t const msg_id,
                          struct gapc_connection_req_ind const *param,
                          ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    struct gapc_connection_cfm *cfm;
    int result;
    int checker = 0;
    int written = 0;
    int flashAddr;
    uint8_t addr[BDADDR_LENGTH];
    app_env.conidx = KE_IDX_GET(src_id);

    /* Check if the received connection handle was valid */
    if (app_env.conidx != GAP_INVALID_CONIDX)
    {
        app_env.state = APPM_CONNECTED;
        /* Retrieve the connection info from the parameters */
        app_env.conidx = param->conhdl;

        /* Clear the advertising timeout timer */
        if (ke_timer_active(APP_ADV_TIMEOUT_TIMER, TASK_APP))
        {
            ke_timer_clear(APP_ADV_TIMEOUT_TIMER, TASK_APP);
        }

        /* Send connection confirmation */
        cfm = KE_MSG_ALLOC(GAPC_CONNECTION_CFM,
                           KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                           gapc_connection_cfm);

        /* Copy the address of the master that is requesting a connection */
        memcpy(&currentMaster.ADDR, &param->peer_addr.addr,
               sizeof(param->peer_addr.addr));

        /* Check if the master device is already bonded */
        currentMaster.STATE = (checkBonded(&currentMaster.ADDR[0]));

        /* If there is a no previous bonded information with the master
         * master device, set the index to 0xFF. The correct value will be
         * determined in bondInd() function */
        if(currentMaster.STATE == APP_NOT_BONDED)
        {
            currentMaster.STATE = 0xFF;
        }

        /* If the master device is already bonded, load the data from the
         * flash */
        else
        {
            flashAddr = (FLASH_NVR2_BASE +
                         sizeof(BondInfo_Type) *(currentMaster.STATE - 1));
            memcpy(&currentMaster,(uint8_t *)flashAddr,
                   sizeof(BondInfo_Type));
            app_env.bonded = true;
            cfm->auth = GAP_AUTH_REQ_NO_MITM_BOND;
            cfm->ltk_present = true;
        }

        cfm->svc_changed_ind_enable = 0;

        /* Send the message */
        ke_msg_send(cfm);
    }
    else
    {
        Advertising_Start();
    }

    return(KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GAPC_CmpEvt(ke_msg_id_t const msg_id,
 *                                 struct gapc_cmp_evt
 *                                 const *param,
 *                                 ke_task_id_t const dest_id,
 *                                 ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle received GAPC complete event
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gapc_cmp_evt
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GAPC_CmpEvt(ke_msg_id_t const msg_id, struct gapc_cmp_evt const *param,
                ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    switch (param->operation)
    {
        case (GAPC_UPDATE_PARAMS):
        {
            if (param->status != GAP_ERR_NO_ERROR)
            {
                Connection_Disconnect();
            }
        } break;

        default:
        {

        } break;
    }

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GAPC_DisconnectInd(ke_msg_id_t const msg_id,
 *                                        struct gapc_disconnect_ind
 *                                        const *param,
 *                                        ke_task_id_t const dest_id,
 *                                        ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle disconnect indication message from GAP controller
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gapc_disconnect_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GAPC_DisconnectInd(ke_msg_id_t const msg_id,
                       struct gapc_disconnect_ind const *param,
                       ke_task_id_t const dest_id,
                       ke_task_id_t const src_id)
{
    /* Go to the ready state */
    app_env.state = APPM_READY;

    /* When not connected to any device, it is not bonded
     * Bonded status is set upon connection */
    app_env.bonded = false;
    Advertising_Start();

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GAPC_ParamUpdatedInd(ke_msg_id_t const msg_id,
 *                                          struct gapc_param_updated_ind
 *                                          const *param,
 *                                          ke_task_id_t const dest_id,
 *                                          ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle message parameter updated indication received
 *                    from GAP controller
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gapc_param_updated_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GAPC_ParamUpdatedInd(ke_msg_id_t const msg_id,
                         struct gapc_param_updated_ind const *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    app_env.updated_con_interval = param->con_interval;
    app_env.updated_latency = param->con_latency;
    app_env.updated_suo_to = param->sup_to;

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int Msg_Handler(ke_msg_id_t const msg_id,
 *                                 void const *param,
 *                                 ke_task_id_t const dest_id,
 *                                 ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle any message received from kernel that doesn't have
 *                 a dedicated handler
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameter (unused)
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int Msg_Handler(ke_msg_id_t const msg_id, void *param,
                ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GATTC_ReadReqInd(ke_msg_id_t const msg_id,
 *                                      struct gattc_read_req_ind
 *                                      const *param,
 *                                      ke_task_id_t const dest_id,
 *                                      ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle received read request indication from GATT Controller
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gattc_read_req_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GATTC_ReadReqInd(ke_msg_id_t const msg_id,
                     struct gattc_read_req_ind const *param,
                     ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gattc_read_cfm *cfm;
    uint8_t length = 0;
    uint8_t status = GAP_ERR_NO_ERROR;
    uint8_t value[40];

    if (param->handle == (app_env.start_hdl + 2))
    {
        length = 1;
        memcpy(value, app_env.uart_tx_value, length);
    }
    else if (param->handle == (app_env.start_hdl + 3))
    {
        length = 2;
        value[0] = app_env.cccd_value;
        value[1] = 0;
    }
    else if (param->handle == (app_env.start_hdl + 5))
    {
        length = 0x14;
        memcpy(value, app_env.uart_rx_value, length);
    }
    else
    {
        status = ATT_ERR_INVALID_HANDLE;
    }

    cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM,
                           KE_BUILD_ID(TASK_GATTC, app_env.conidx),
                           TASK_APP, gattc_read_cfm, length);

    if (status == GAP_ERR_NO_ERROR)
    {
        memcpy(cfm->value, value, length);
        cfm->length = length;
    }

    cfm->handle = param->handle;
    cfm->status = status;

    /* Send the message */
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GATTC_WriteReqInd(ke_msg_id_t const msg_id,
 *                                       struct gattc_write_req_ind
 *                                       const *param,
 *                                       ke_task_id_t const dest_id,
 *                                       ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle received write request indication from GATT controller
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gattc_write_req_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GATTC_WriteReqInd(ke_msg_id_t const msg_id,
        struct gattc_write_req_ind const *param, ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    uint8_t status = GAP_ERR_NO_ERROR;

    struct gattc_write_cfm *cfm;
    cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM,KE_BUILD_ID(TASK_GATTC, app_env.conidx),
                       TASK_APP, gattc_write_cfm);

    if (param->handle == (app_env.start_hdl + 2))
    {
        /* TX handle, it is not to be written */
        status = ATT_ERR_WRITE_NOT_PERMITTED;
    }

    else if (param->handle == (app_env.start_hdl + 3))
    {
        /* It is client characteristic configuration descriptor (CCCD) */
        app_env.cccd_value = param->value[0];
    }

    else if (param->handle == (app_env.start_hdl + 5))
    {
        /* RX handle */
        memcpy(app_env.uart_rx_value, param->value, param->length);
        memcpy(app_env.spi1_tx_value, 0, 10);
        app_env.spi1_tx_size=param->length;
        memcpy(app_env.spi1_tx_value, param->value, param->length);
        app_env.bytes=0;
    }

    else
    {
        status = ATT_ERR_INVALID_HANDLE;
    }

    cfm->handle = param->handle;
    cfm->status = status;

    /* Send the message */
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GATTM_AddSvcRsp(ke_msg_id_t const msg_id,
 *                                     struct gattm_add_svc_rsp
 *                                     const *param,
 *                                     ke_task_id_t const dest_id,
 *                                     ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle the response of adding a service in attribute database
 *                 from GATT manager
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gattm_add_svc_rsp
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GATTM_AddSvcRsp(ke_msg_id_t const msg_id,
                    struct gattm_add_svc_rsp const *param,
                    ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    app_env.start_hdl = param->start_hdl;

    if (!Service_Add())
    {
        /* Go to the ready state */
        app_env.state = APPM_READY;

        /* No more service to add, start advertising */
        Advertising_Start();
    }

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : void Batt_LevelUpdateRsp(ke_msg_id_t const msg_id,
 *                                          struct bass_batt_level_upd_rsp
 *                                          const *param,
 *                                          ke_task_id_t const dest_id,
 *                                          ke_task_id_t const src_id)
 * ----------------------------------------------------
 * Description   : Receive status of sending battery notification
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct bass_batt_level_upd_rsp
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int Batt_LevelUpdateRsp(ke_msg_id_t const msg_id,
                        struct bass_batt_level_upd_rsp const *param,
                        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    /* A notification has been sent to peer */
    uint8_t status;

    status = param->status;

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : void Batt_LevelNtfCfgInd(ke_msg_id_t const msg_id,
 *                                          struct bass_batt_level_ntf_cfg_ind
 *                                          const *param,
 *                                          ke_task_id_t const dest_id,
 *                                          ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Receive notification config indication for battery level
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct bass_batt_level_ntf_cfg_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int Batt_LevelNtfCfgInd(ke_msg_id_t const msg_id,
                        struct bass_batt_level_ntf_cfg_ind const *param,
                        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    uint8_t conidx, ntf_cfg;

    /* Notification config has been changed for one
     * of the battery level characteristics */
    app_env.batt_ntf_cfg = param->ntf_cfg;

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : void Batt_EnableRsp(ke_msg_id_t const msg_id,
 *                                     struct bass_enable_rsp
 *                                     const *param,
 *                                     ke_task_id_t const dest_id,
 *                                     ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Receive battery service enable from battery task
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct bass_enable_rsp
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int Batt_EnableRsp(ke_msg_id_t const msg_id,
                   struct bass_enable_rsp const *param,
                   ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    if (param->status == GAP_ERR_NO_ERROR)
    {
        app_env.state = APPM_ENABLED;
    }

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GAPC_BondReqInd(ke_msg_id_t const msg_id,
                                       struct gapc_bond_req_ind const *param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Receive bond request and information exchange indication
 *                 from master, peripheral is storing the values that it
 *                 generates.IRK and LTK are generated and are sent to
 *                 master device
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gapc_bond_req_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GAPC_BondReqInd(ke_msg_id_t const msg_id,
                    struct gapc_bond_req_ind const *param,
                    ke_task_id_t const dest_id, ke_task_id_t const src_id)
{

    /* Prepare message to be sent to smpc */
    struct gapc_bond_cfm *cfm;
    cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, src_id, TASK_APP, gapc_bond_cfm);
    /* Slave device address */
    uint8_t slave_address[BDADDR_LENGTH] = PRIVATE_BDADDR;
    int result;

    /* Key and Random number has a different length
     * so the variable is used in two different loops
     * This variable is used in LTK_EXCH_REQ case */
    uint8_t counter;

    uint32_t pin_code;

    /* Ensure that the current master is not already bonded
     * If the bonding information already exist, handle the request
     * differently */
    if (!app_env.bonded)
    {
        switch (param->request)
        {
            /* Prepare pairing information to be sent to the master device */
            case (GAPC_PAIRING_REQ):
            {

            	if(checkSpaceAvailable()!= LIST_FULL)
                {
                cfm->request = GAPC_PAIRING_RSP;
                cfm->accept  = 1;
                cfm->data.pairing_feat.auth      = GAP_AUTH_REQ_NO_MITM_BOND;
                cfm->data.pairing_feat.iocap     =
                                                  GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
                cfm->data.pairing_feat.key_size  = 16;
                cfm->data.pairing_feat.oob       =
                                                  GAP_OOB_AUTH_DATA_NOT_PRESENT;
                cfm->data.pairing_feat.sec_req   = GAP_SEC1_NOAUTH_PAIR_ENC;
                cfm->data.pairing_feat.ikey_dist = GAP_KDIST_ENCKEY  |
                                                   GAP_KDIST_IDKEY   |
                                                   GAP_KDIST_SIGNKEY |
                                                   GAP_KDIST_LINKKEY;
                cfm->data.pairing_feat.rkey_dist = GAP_KDIST_ENCKEY  |
                                                   GAP_KDIST_IDKEY   |
                                                   GAP_KDIST_SIGNKEY |
                                                   GAP_KDIST_LINKKEY;
                }

                else
                {
                    cfm->accept = false;
                }
            } break;

            /* Prepare LTK to be sent to the master device */
            case (GAPC_LTK_EXCH):
            {
                cfm->accept  = true;
                cfm->request = GAPC_LTK_EXCH;
                /* Generate EDIV, LTK, and RAND for an encryption */
                cfm->data.ltk.ediv = (uint16_t)co_rand_word();

                for (counter = 0; counter < RAND_NB_LEN; counter++)
                {
                    cfm->data.ltk.ltk.key[counter]    = (uint8_t)co_rand_word();
                    cfm->data.ltk.randnb.nb[counter] = (uint8_t)co_rand_word();
                }

                for (counter = RAND_NB_LEN; counter < KEY_LEN; counter++)
                {
                    cfm->data.ltk.ltk.key[counter]    = (uint8_t)co_rand_word();
                }

                /* Store the generated keys into the local list */
                memcpy(&currentMaster.LTK, &cfm->data.ltk.ltk.key,
                       sizeof(cfm->data.ltk.ltk.key));
                memcpy(&currentMaster.RAND,&cfm->data.ltk.randnb.nb,
                       sizeof(cfm->data.ltk.randnb.nb));
                memcpy(&currentMaster.EDIV,&cfm->data.ltk.ediv,
                       sizeof(cfm->data.ltk.ediv));

            } break;

            /* Prepare TK to be sent to the master device */
            case (GAPC_TK_EXCH):
            {
                cfm->accept  = true;
                cfm->request = GAPC_TK_EXCH;

                pin_code = (100000 + (co_rand_word()%900000));

                /* Set the TK value */
                memset(cfm->data.tk.key, 0, KEY_LEN);

                cfm->data.tk.key[0] = (uint8_t)((pin_code & 0x000000FF) >>  0);
                cfm->data.tk.key[1] = (uint8_t)((pin_code & 0x0000FF00) >>  8);
                cfm->data.tk.key[2] = (uint8_t)((pin_code & 0x00FF0000) >> 16);
                cfm->data.tk.key[3] = (uint8_t)((pin_code & 0xFF000000) >> 24);

            } break;

            /* Prepare IRK to be sent to the master device */
            case (GAPC_IRK_EXCH):
            {
                cfm->accept = true;
                cfm->request = GAPC_IRK_EXCH;

                /* Store the slave device address in the message */
                for(int j = 0; j < BDADDR_LENGTH; j++)
                {
                    cfm->data.irk.addr.addr.addr[j] = slave_address[j];
                }

                /* Generate IRK */
                for (counter = 0; counter < 16; counter++)
                {
                    cfm->data.irk.irk.key[counter] = (uint8_t)co_rand_word();
                }

                memcpy(&currentMaster.IRK,&cfm->data.irk.irk.key,
                       sizeof(cfm->data.irk.irk.key));

            } break;

            default:
            {
                ASSERT_ERR(0);
            } break;
        }
    }

    /* This block of code handles the case when the master device that is
     * already bonded is requesting for a new pairing. If there is an available
     * space accept the new request */
    else
    {
        /* Using the used bits to mark invalidated connection.
         * Logical OR operation sets a bit */
        currentMaster.STATE = currentMaster.STATE | 0x80;

        cfm->accept = false;
        app_env.bonded = false;

        /* If there is an open space in the list that could be written
         * so accept the bonding request */
        if(checkSpaceAvailable()!= LIST_FULL)
        {
            cfm->accept  = true;
            cfm->request = GAPC_PAIRING_RSP;
            cfm->data.pairing_feat.auth      = GAP_AUTH_REQ_NO_MITM_BOND;
            cfm->data.pairing_feat.iocap     = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
            cfm->data.pairing_feat.key_size  = 16;
            cfm->data.pairing_feat.oob       = GAP_OOB_AUTH_DATA_NOT_PRESENT;
            cfm->data.pairing_feat.sec_req   = GAP_SEC1_NOAUTH_PAIR_ENC;
            cfm->data.pairing_feat.ikey_dist = GAP_KDIST_ENCKEY  |
                                               GAP_KDIST_IDKEY   |
                                               GAP_KDIST_SIGNKEY |
                                               GAP_KDIST_LINKKEY;
            cfm->data.pairing_feat.rkey_dist = GAP_KDIST_ENCKEY  |
                                               GAP_KDIST_IDKEY   |
                                               GAP_KDIST_SIGNKEY |
                                               GAP_KDIST_LINKKEY;
        }
    }

    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GAPC_BondInd(ke_msg_id_t const msg_id,
                                    struct gapc_bond_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Receives bond procedure status and CSRK value
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gapc_bond_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GAPC_BondInd(ke_msg_id_t const msg_id, struct gapc_bond_ind const *param,
                 ke_task_id_t const dest_id, ke_task_id_t const src_id)

{
    int result;
    switch (param->info)
    {
        case (GAPC_PAIRING_SUCCEED):
            {

            /* Ideally, this statement is never true */
            if(currentMaster.STATE & 0x80 == 0)
            {
                /* Do Nothing */
            }

            /* The function that determines the available index in the flash
             * is called */
            else
            {
                currentMaster.STATE = searchAvailableIndex(currentMaster.STATE);

                if(currentMaster.STATE != INDEX_SEARCH_FAIL)
                {
                    /* Update the bonding status in the environment */
                    app_env.bonded = true;

                    /* Save the current master device's bonding information */
                    result = WordWrite(currentMaster,currentMaster.STATE);
                    /* Enable battery service */
                    Batt_ServiceEnable(app_env.conidx);
                }
            }
        } break;

        case (GAPC_REPEATED_ATTEMPT):
        {
            app_env.state = APPM_READY;
        } break;

        case (GAPC_PAIRING_FAILED):
        {

        } break;

        case (GAPC_IRK_EXCH):
        {
            memcpy(&currentMaster.IRK, &param->data.irk.irk.key,
                   sizeof(param->data.irk.irk.key));
            memcpy(&currentMaster.ADDR, &param->data.irk.addr.addr.addr,
                   sizeof(param->data.irk.addr.addr.addr));
        } break;

        default:
        {
            ASSERT_ERR(0);
        } break;

    }

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GAPC_EncryptReqInd(ke_msg_id_t const msg_id,
 *                                        struct gapc_encrypt_req_ind
 *                                        const *param,
 *                                        ke_task_id_t const dest_id,
 *                                        ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Receives encryption request from master device and sends
 *                 the encrpytion status after comparing the values from the
 *                 master device. Master device sends EDIV and RAND values
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gapc_encrypt_req_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GAPC_EncryptReqInd(ke_msg_id_t const msg_id,
                       struct gapc_encrypt_req_ind const *param,
                       ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    /* Struct that will contain the bonding information from the slave device */
    struct gapc_ltk ltk;

    memcpy(&ltk.ltk.key, &currentMaster.LTK, sizeof(currentMaster.LTK));
    memcpy(&ltk.randnb.nb, &currentMaster.RAND, sizeof(currentMaster.RAND));
    memcpy(&ltk.ediv,&currentMaster.EDIV, sizeof(currentMaster.EDIV));

    /* Prepare the GAPC_ENCRYPT_CFM message to be sent to SMPC */
    struct gapc_encrypt_cfm *cfm;
    cfm = KE_MSG_ALLOC(GAPC_ENCRYPT_CFM, src_id, TASK_APP, gapc_encrypt_cfm);

    /* If the devices are not bonded, do not accept the encryption request */
    cfm->found = false;

    /* If the devices are bonded, checked the EDIV and RAND values sent
     * from the master */
    if (app_env.bonded == true)
    {

        if ((param->ediv == ltk.ediv)
             && !memcmp(&param->rand_nb.nb[0], &ltk.randnb.nb[0],
             sizeof(struct rand_nb)))
        {
            cfm->found = true;
            cfm->key_size = 16;
            memcpy(&cfm->ltk, &ltk.ltk, sizeof(struct gap_sec_key));
        }
    }

    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}
/* ----------------------------------------------------------------------------
* Function       : int GAPC_EncryptInd(ke_msg_id_t const msg_id,
                                       struct gapc_encrypt_ind const *param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
* ----------------------------------------------------------------------------
* Description   : Receives this flag if encryption was successful
* Inputs        : - msg_id     - Kernel message ID number
*                 - param      - Message parameters in format of
*                                struct gapc_encrypt_ind
*                 - dest_id    - Destination task ID number
*                 - src_id     - Source task ID number
* Outputs       : return value - Indicate if the message was consumed;
*                                compare with KE_MSG_CONSUMED
* Assumptions   : None
* ------------------------------------------------------------------------- */
int GAPC_EncryptInd(ke_msg_id_t const msg_id,
        struct gapc_encrypt_ind const *param, ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    return (KE_MSG_CONSUMED);
}
