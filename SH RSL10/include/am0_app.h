/**
 ****************************************************************************************
 *
 * @file am0_app.h
 *
 * @brief Audio Mode 0 application example
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 ****************************************************************************************
 */


#ifndef _AM0_APP_H_
#define _AM0_APP_H_

/**
 ****************************************************************************************
 * @addtogroup AM0_APP
 * @brief AUDIO MODE 0 APPLICATION EXAMPLE
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration
//#include <am0_has_task.h>		// Unresolved inclusion

#ifdef BLE_APP_AM0

/*
 * DEFINES
 ****************************************************************************************
 */

///////////////////////////////////////////////////////////////////////////////
// Dzung: moved to here from am0_app.c

/// Advertising minimum interval - 40ms (64*0.625ms)
#define AM0_ADV_INT_MIN                 (64)
/// Advertising maximum interval - 40ms (64*0.625ms)
#define AM0_ADV_INT_MAX                 (64)

/// Maximal length of the Device Name value
#define AM0_DEVICE_NAME_MAX_LEN         (22)
#define AM0_IDENTIFIER_MAX_LEN          (20)
/// Default Device Name if no value can be found in NVDS
#define AM0_DFLT_DEVICE_NAME            ("RW-BLE Left Ear")
/// Default program name
#define AM0_APP_PROG_DFLT_NAME          "Default"

/// certificate information
#define AM0_NVDS_KEY_MAX_LEN            (64*3)

#define AM0_CERT_DATA_MAX_LEN           (1024)
#define AM0_CERT_PRIV_KEY_MAX_LEN       (64*5)

/// Number of available programs
#define AM0_NB_PROGS                    (2)

#define HAS_HEARING_AID_SERVICE_UUID    "\x92\x15\x37\x84\xE8\xCC\x2C\x86\x31\x44\x4A\xC7\xBD\xF4\x74\x7D"

#define AM0_APP_DFLT_ID                 "RW-BLE LEFT"
#define AM0_APP_DFLT_OTHER_ID           "RW-BLE RIGHT"

///////////////////
// Dzung
// Length of certificate data
#define AM0_CERT_DATA_LEN               (910)
// Length of private key (compopsed of 5 private sub-keys)
#define AM0_CERT_PRIV_KEY_LEN           (64*5)
///////////////////

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

//C:\Repository\swt_components\vendor\ceva\rw-ble-sw\src\modules\app\api\app.h (2 hits)
//	Line 99:     NVDS_TAG_AM0_FIRST                  = NVDS_TAG_APP_SPECIFIC_FIRST,
//	Line 100:     NVDS_TAG_AM0_LAST                   = NVDS_TAG_APP_SPECIFIC_FIRST+16,

//C:\Repository\swt_components\vendor\ceva\rw-ble-sw\src\modules\nvds\api\nvds.h (1 hit)
//	Line 188:     NVDS_TAG_APP_SPECIFIC_FIRST         = 0x90,
//	Line 189:     NVDS_TAG_APP_SPECIFIC_LAST          = 0xAF,

/// audio mode 0 NVDS
//enum am0_app_nvds_tag
//{
//    /// Hearing aid identifier
//    NVDS_TAG_AM0_ID               = NVDS_TAG_AM0_FIRST,
//    /// Other Hearing aid identifier
//    NVDS_TAG_AM0_OTHER_ID,
//    /// private key partitioning
//    NVDS_TAG_AM0_PRIV_KEY_FIRST,
//    NVDS_TAG_AM0_PRIV_KEY_LAST    = NVDS_TAG_AM0_PRIV_KEY_FIRST+1,
//    /// certificate partitioning
//    NVDS_TAG_AM0_CERT_FIRST,
//    NVDS_TAG_AM0_CERT_LAST        = NVDS_TAG_AM0_CERT_FIRST    +5,
//
//    NVDS_TAG_AM0_MAX,
//
//    /// Parameter length
//    NVDS_LEN_AM0_ID       = AM0_IDENTIFIER_MAX_LEN,
//    NVDS_LEN_AM0_OTHER_ID = AM0_IDENTIFIER_MAX_LEN,
//    NVDS_LEN_AM0_CERT     = AM0_NVDS_KEY_MAX_LEN,
//    NVDS_LEN_AM0_PRIV_KEY = AM0_NVDS_KEY_MAX_LEN,
//};


/// Audio program information
struct am0_app_prog
{
    /// Program name
    uint8_t name_len;
    uint8_t name[AM0_PROG_NAME_MAX_LEN];
    /// Program category
    uint8_t category;
};

/// Audio Mode 0 Application environment
struct am0_app_env_tag
{
    // ------------------- Audio Mode 0 ---------------
    #if (DISPLAY_SUPPORT)
    uint8_t state;
    uint8_t volume;
    #endif // (DISPLAY_SUPPORT)

    // ------------------- Hearing Aid Service ---------------
    uint16_t prf_task;

    /// Hearing Aid identifier
    uint8_t id_len;
    uint8_t id[AM0_IDENTIFIER_MAX_LEN];

    /// Other Hearing Aid identifier
    uint8_t other_id_len;
    uint8_t other_id[AM0_IDENTIFIER_MAX_LEN];

    /// Certificate data length
    uint16_t cert_data_len;
    /// Certificate data payload
    uint8_t  cert_data[AM0_CERT_DATA_MAX_LEN];

    /// Private key length
    uint16_t cert_priv_key_len;
    /// Private key
    uint8_t  cert_priv_key[AM0_CERT_PRIV_KEY_MAX_LEN];

    /// Current active program identifier
    uint8_t active_prog_id;

    uint8_t selected_prog_id;
    /// programs info
    struct am0_app_prog progs[AM0_NB_PROGS];
};

///////////////////////////////////////////////////////////////////////////////

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

enum am0_app_state
{
    AM0_APP_STATE_OFF,
    AM0_APP_STATE_INIT,
    AM0_APP_STATE_CONNECTED,
    AM0_APP_STATE_AUTH,
    AM0_APP_STATE_CODEC_SEL,
    AM0_APP_STATE_STREAM_ON,
    AM0_APP_STATE_STREAM_OFF,
};



/*
 * MACROS
 ****************************************************************************************
 */


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */
/// Message handler table for AM0 messages
//Dzung: commented out the line below
//extern const struct ke_state_handler am0_app_table_handler;

/// Message handler table for AM0_HAS messages
//Dzung: commented out the line below
//extern const struct ke_state_handler am0_app_has_table_handler;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @brief Initialize Audio Mode 0 Application Module
 ****************************************************************************************
 */
void am0_app_init(void);

/**
 ****************************************************************************************
 * @brief Add a Hearing Aid Service instance in the DB
 ****************************************************************************************
 */
void am0_app_add_has(void);

/**
 ****************************************************************************************
 * @brief Restore bond data of Hearing Aid Service
 *
 * @param[in]:  conidx - Connection index of the connection
 ****************************************************************************************
 */
void am0_app_restore_bond_data(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Start advertising
 ****************************************************************************************
 */
void am0_app_start_advertising(void);

/**
 ****************************************************************************************
 * @brief update the profile task number
 *
 * @param[in] task Profile task number.
 ****************************************************************************************
 */
void am0_app_set_prf_task(uint16_t task);


/**
 * Initializes audio protocol
 *
 * @param conidx connection index
 */
void am0_app_send_audio_init(uint8_t conidx);

#endif // BLE_APP_AM0

/// @} am0_app

#endif /* _AM0_APP_H_ */
