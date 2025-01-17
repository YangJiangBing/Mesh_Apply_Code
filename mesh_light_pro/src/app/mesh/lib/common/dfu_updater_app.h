/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      dfu_updater_app.h
* @brief     Smart mesh dfu application
* @details
* @author    bill
* @date      2018-6-5
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef _DFU_UPDATER_APP_H
#define _DFU_UPDATER_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mesh_api.h"
#include "firmware_update.h"
#include "object_transfer.h"

/**
 * @addtogroup DFU_UPDATER_APP
 * @{
 */
#define DFU_UPDATER_SUPPORT_POWER_OFF_GO_ON     1

/* Attention: shall not be overlapped with other datas */
#define DFU_UPDATER_NVM_OFFSET                  2000

/**
 * @defgroup Dfu_Updater_Exported_Functions Dfu Updater Exported Functions
 * @brief
 * @{
 */
void dfu_updater_models_init(void);

#if DFU_UPDATER_SUPPORT_POWER_OFF_GO_ON
void dfu_updater_load(void);
void dfu_updater_clear(void);
#endif

/** @} */
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* _DFU_UPDATER_APP_H */
