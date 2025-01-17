/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      main.c
   * @brief     Source file for BLE scatternet project, mainly used for initialize modules
   * @author    jane
   * @date      2017-06-12
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include "board.h"
#include <os_sched.h>
#include <string.h>
#include <app_task.h>
#include <trace.h>
#include <gap.h>
#include <gap_bond_le.h>
#include <gap_scan.h>
#include <gap_msg.h>
#include <profile_client.h>
#include <gaps_client.h>
#include <gap_adv.h>
#include <profile_server.h>
#include <gatt_builtin_services.h>
#include <platform_utils.h>

#if DLPS_EN
#include "dlps.h"
#include "rtl876x_io_dlps.h"
#endif

#include "mesh_api.h"
#include "health.h"
#include "ping.h"
#include "ping_app.h"
#include "switch_app.h"
#include "dimmable_light.h"
#include "light_cwrgb_server_app.h"
#include "light_hsl_server_app.h"

#include "ota_server.h"
#include "dfu_server.h"
#include "dfu_client.h"
#include "mem_config.h"
#include "dfu_updater_app.h"

#include "user_data.h"
#include "switch_io.h"
#include "switch_server_app.h"
#include "switch_flash_mgr.h"
#include "switch_swtimer.h"

static mesh_model_info_t health_server_model;

/******************************************************************
 * @fn          Initial gap parameters
 * @brief      Initialize peripheral and gap bond manager related parameters
 *
 * @return     void
 */
void mesh_stack_init(void)
{
    /** set ble stack log level, disable nonsignificant log */
    log_module_bitmap_trace_set(0xFFFFFFFFFFFFFFFF, LEVEL_TRACE, 0);
    log_module_bitmap_trace_set(0xFFFFFFFFFFFFFFFF, LEVEL_INFO, 0);
    log_module_trace_set(MODULE_LOWERSTACK, LEVEL_ERROR, 0);
    log_module_trace_set(MODULE_SNOOP, LEVEL_ERROR, 0);

    /** set mesh stack log level, default all on, disable the log of level LEVEL_TRACE */
    uint32_t module_bitmap[MESH_LOG_LEVEL_SIZE] = {0};
    diag_level_set(LEVEL_TRACE, module_bitmap);

    /** mesh stack needs rand seed */
    plt_srand(platform_random(0xffffffff));

    /** set device name and appearance */
    char *dev_name = "Mesh Ali Switch";
    uint16_t appearance = GAP_GATT_APPEARANCE_UNKNOWN;
    gap_sched_params_set(GAP_SCHED_PARAMS_DEVICE_NAME, dev_name, GAP_DEVICE_NAME_LEN);
    gap_sched_params_set(GAP_SCHED_PARAMS_APPEARANCE, &appearance, sizeof(appearance));

    /** set device uuid */
    uint32_t product_id = ALI_PRODUCT_ID;
    if (user_data_contains_ali_data())
    {
        product_id = user_data_read_ali_product_id();
    }

    ali_uuid_t dev_uuid;
    dev_uuid.cid = 0x01A8; //!< taobao
    dev_uuid.pid.adv_ver = 1;
    dev_uuid.pid.sec = 1;
    dev_uuid.pid.ota = 0;
    dev_uuid.pid.bt_ver = 1;
    dev_uuid.product_id = product_id;
    uint8_t bt_addr[6];
    gap_get_param(GAP_PARAM_BD_ADDR, bt_addr);
    memcpy(dev_uuid.mac_addr, bt_addr, sizeof(bt_addr));
    memset(dev_uuid.rfu, 0, sizeof(dev_uuid.rfu));
    device_uuid_set((uint8_t *)&dev_uuid);

    /** configure provisioning parameters */
    prov_capabilities_t prov_capabilities =
    {
        .algorithm = PROV_CAP_ALGO_FIPS_P256_ELLIPTIC_CURVE,
        .public_key = 0,
        .static_oob = PROV_CAP_STATIC_OOB,
        .output_oob_size = 0,
        .output_oob_action = 0,
        .input_oob_size = 0,
        .input_oob_action = 0
    };
    prov_params_set(PROV_PARAMS_CAPABILITIES, &prov_capabilities, sizeof(prov_capabilities_t));
    prov_params_set(PROV_PARAMS_CALLBACK_FUN, prov_cb, sizeof(prov_cb_pf));

    /** config node parameters */
    mesh_node_features_t features =
    {
        .role = MESH_ROLE_DEVICE,
        .relay = 1,
        .proxy = 1,
        .fn = 0,
        .lpn = 0,
        .prov = 1,
        .udb = 1,
        .snb = 1,
        .bg_scan = 1,
        .flash = 1,
        .flash_rpl = 0
    };
    mesh_node_cfg_t node_cfg =
    {
        .dev_key_num = 1,
        .net_key_num = 3,
        .app_key_num = 3,
        .vir_addr_num = 3,
        .rpl_num = 100,
        .sub_addr_num = 10,
        .proxy_num = 1,
        .udb_interval = 5,
        .prov_interval = 10,
        .proxy_interval = 10,
        .identity_interval = 20
    };
    mesh_node_cfg(features, &node_cfg);
    mesh_node.net_trans_count = 5;
    mesh_node.relay_retrans_count = 5;
    mesh_node.trans_retrans_count = 7;

    /** create elements and register models */
    mesh_element_create(GATT_NS_DESC_UNKNOWN);
    health_server_reg(0, &health_server_model);
    health_server_set_company_id(&health_server_model, COMPANY_ID);
    //dfu_updater_models_init();
    //ping_control_reg(ping_app_ping_cb, NULL);
    //trans_ping_pong_init(ping_app_ping_cb, NULL);
    switch_server_models_init();
    compo_data_page0_header_t compo_data_page0_header = {COMPANY_ID, PRODUCT_ID, VERSION_ID};
    compo_data_page0_gen(&compo_data_page0_header);

    /** restore switch ahead since it may restore the fatory setting */
    switch_flash_restore();

    /** init mesh stack */
    mesh_init();
}

/**
  * @brief  Initialize gap related parameters
  * @return void
  */
void app_le_gap_init(void)
{
    /* GAP Bond Manager parameters */
    uint8_t  auth_pair_mode = GAP_PAIRING_MODE_PAIRABLE;
    uint16_t auth_flags = GAP_AUTHEN_BIT_BONDING_FLAG;
    uint8_t  auth_io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
    uint8_t  auth_oob = false;
    uint8_t  auth_use_fix_passkey = false;
    uint32_t auth_fix_passkey = 0;
    uint8_t  auth_sec_req_enable = false;
    uint16_t auth_sec_req_flags = GAP_AUTHEN_BIT_BONDING_FLAG;

    /* Setup the GAP Bond Manager */
    gap_set_param(GAP_PARAM_BOND_PAIRING_MODE, sizeof(auth_pair_mode), &auth_pair_mode);
    gap_set_param(GAP_PARAM_BOND_AUTHEN_REQUIREMENTS_FLAGS, sizeof(auth_flags), &auth_flags);
    gap_set_param(GAP_PARAM_BOND_IO_CAPABILITIES, sizeof(auth_io_cap), &auth_io_cap);
    gap_set_param(GAP_PARAM_BOND_OOB_ENABLED, sizeof(auth_oob), &auth_oob);
    le_bond_set_param(GAP_PARAM_BOND_FIXED_PASSKEY, sizeof(auth_fix_passkey), &auth_fix_passkey);
    le_bond_set_param(GAP_PARAM_BOND_FIXED_PASSKEY_ENABLE, sizeof(auth_use_fix_passkey),
                      &auth_use_fix_passkey);
    le_bond_set_param(GAP_PARAM_BOND_SEC_REQ_ENABLE, sizeof(auth_sec_req_enable), &auth_sec_req_enable);
    le_bond_set_param(GAP_PARAM_BOND_SEC_REQ_REQUIREMENT, sizeof(auth_sec_req_flags),
                      &auth_sec_req_flags);

    /* register gap message callback */
    le_register_app_cb(app_gap_callback);

#if F_BT_LE_5_0_SET_PHY_SUPPORT
    uint8_t  phys_prefer = GAP_PHYS_PREFER_ALL;
    uint8_t  tx_phys_prefer = GAP_PHYS_PREFER_1M_BIT | GAP_PHYS_PREFER_2M_BIT |
                              GAP_PHYS_PREFER_CODED_BIT;
    uint8_t  rx_phys_prefer = GAP_PHYS_PREFER_1M_BIT | GAP_PHYS_PREFER_2M_BIT |
                              GAP_PHYS_PREFER_CODED_BIT;
    le_set_gap_param(GAP_PARAM_DEFAULT_PHYS_PREFER, sizeof(phys_prefer), &phys_prefer);
    le_set_gap_param(GAP_PARAM_DEFAULT_TX_PHYS_PREFER, sizeof(tx_phys_prefer), &tx_phys_prefer);
    le_set_gap_param(GAP_PARAM_DEFAULT_RX_PHYS_PREFER, sizeof(rx_phys_prefer), &rx_phys_prefer);
#endif
}

/**
 * @brief  Add GATT services, clients and register callbacks
 * @return void
 */
void app_le_profile_init(void)
{
    server_init(MESH_GATT_SERVER_COUNT + 2);
    /* Add Server Module */
    ota_server_add(NULL);
    dfu_server_add(NULL);

    /* Register Server Callback */
    server_register_app_cb(app_profile_callback);

    client_init(1);
    /* Add Client Module */
    //dfu_client_add(NULL);

    /* Register Client Callback--App_ClientCallback to handle events from Profile Client layer. */
    client_register_general_client_cb(app_client_callback);
}

/**
 * @brief    Contains the initialization of pinmux settings and pad settings
 * @note     All the pinmux settings and pad settings shall be initiated in this function,
 *           but if legacy driver is used, the initialization of pinmux setting and pad setting
 *           should be peformed with the IO initializing.
 * @return   void
 */
void board_init(void)
{
    board_switch_io_init();
}

/**
 * @brief    Contains the initialization of peripherals
 * @note     Both new architecture driver and legacy driver initialization method can be used
 * @return   void
 */
void driver_init(void)
{
    driver_switch_io_init();

    mesh_node_state_t node_state = mesh_node_state_restore();
    if (node_state == UNPROV_DEVICE)
    {
        unprov_timer_init();
    }
    else
    {
        change_scan_param_timer_init();
    }
}

#if DLPS_EN
/**
 * @brief this function will be called before enter DLPS
 *
 *  set PAD and wakeup pin config for enterring DLPS
 *
 * @param none
 * @return none
 * @retval void
*/
void app_enter_dlps_config(void)
{
    switch_io_enter_dlps_config();
}

/**
 * @brief this function will be called after exit DLPS
 *
 *  set PAD and wakeup pin config for enterring DLPS
 *
 * @param none
 * @return none
 * @retval void
*/
void app_exit_dlps_config(void)
{
    switch_io_exit_dlps_config();
}

/**
 * @brief app_dlps_check_cb() contains the setting about app dlps callback.
*/
bool app_dlps_check_cb(void)
{
    return switch_check_dlps();
}
#endif

void System_Handler(void)
{
    APP_PRINT_INFO0("[app_system_on_interrupt_handler] sytem on interrupt");

    uint8_t tmpVal;
    NVIC_DisableIRQ(System_IRQn);
    tmpVal = btaon_fast_read_safe(0x2b);
    if (tmpVal & BIT7)
    {
        /* pad signal wake up event */
        btaon_fast_write_safe(0x2b, (tmpVal | BIT7));
        //switch_io_ctrl_dlps(false);
        switch_io_handle_msg_exit_dlps();
    }
    NVIC_ClearPendingIRQ(System_IRQn);
}

/**
 * @brief    Contains the power mode settings
 * @return   void
 */
void pwr_mgr_init(void)
{
#if DLPS_EN
    if (false == dlps_check_cb_reg(app_dlps_check_cb))
    {
        APP_PRINT_ERROR0("Error: dlps_check_cb_reg(app_dlps_check_cb) failed!");
    }
    DLPS_IORegUserDlpsEnterCb(app_enter_dlps_config);
    DLPS_IORegUserDlpsExitCb(app_exit_dlps_config);
    DLPS_IORegister();
    lps_mode_set(LPM_DLPS_MODE);
    switch_dlps_ctrl_init();
#endif
}

/**
 * @brief    Contains the initialization of all tasks
 * @note     There is only one task in BLE Scatternet APP, thus only one APP task is init here
 * @return   void
 */
void task_init(void)
{
    app_task_init();
}

/**
 * @brief    Entry of APP code
 * @return   int (To avoid compile warning)
 */
int main(void)
{
    board_init();
    le_gap_init(APP_MAX_LINKS);
    gap_lib_init();
    app_le_gap_init();
    app_le_profile_init();
    mesh_stack_init();
    pwr_mgr_init();
    task_init();
    os_sched_start();

    return 0;
}

#include "otp_config.h"
#ifdef BT_STACK_CONFIG_ENABLE
#include "app_section.h"
#include "gap_config.h"
APP_FLASH_TEXT_SECTION void bt_stack_config_init(void)
{
    gap_config_bt_report_buf_num(8);
}
#endif

