/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file    mem_config.h
  * @brief   Memory Configuration
  * @date    2017.6.6
  * @version v1.0
  * *************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   * *************************************************************************************
  */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef MEM_CONFIG_H
#define MEM_CONFIG_H

#include "flash_map.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Code configuration
 *============================================================================*/
/** @brief set app bank to support OTA: 1 is ota bank1, 0 is ota bank0 */
#define APP_BANK                                  0

/** @brief ram code configuration: 1 is ram code, 0 is flash code */
#define FEATURE_RAM_CODE                          0

/** @brief encrypt app or not */
#define FEATURE_ENCRYPTION                        0

/* ali defined ids */
/**
 * @brief test 3-tuple for ali light
 *  --product id--product secret--mac address--

    on/off 3-tuple
    2 ed97034564d01aed3c91d1a1b03d0069 78da071127ea
    2 362bdeb7e820d40960ac45b5593e58c0 78da071127eb
    2 8b4eb878d14669f62553dc65befd6971 78da071127ec

    ctl 3-tuple //for control light lightness
    465 50e69b7d948d1bd7ba9ed7b008389010 78da07b89d7a
    465 089ef1b65c776ad518fcf5c3948b4a29 78da07b89d7b
    465 3afc7365ae9f9aa351285bc2151b60b4 78da07b89d7c

    ctl & scene 3-tuple
    956,9b66ff7685c573e2b9dbcf24867191f5 78da07efaeca
    956,2f75bcb966dc1745bd5716f995dbbb34 78da07efaecb
    956,15832ce80adfd2b473e2eb6875042dbc 78da07efaecc

    hsl 3-tuple
    973 517fe9795f0a62ff48f8b6726328b2ab 78da07f105b5
    973 7cae4412954c148d042a813c3f003e13 78da07f105b6
    973 477551018370affbde4ad15d9452fd6e 78da07f105b7

    ctl & hsl 3-tuple
    463 c33fc1c2b6a28b85d0933cc72e5c274c 78da07b87664
    463 6aa10b7132000cd062201d33d352ebd5 78da07b87665
    463 5ed4e03d9c314dab0404f7e2607c474a 78da07b87666
*/
#define ALI_PRODUCT_ID                          465
#define ALI_VERSION_ID                          0x00000001
//#define ALI_SECRET_KEY                        "c33fc1c2b6a28b85d0933cc72e5c274c"//"362bdeb7e820d40960ac45b5593e58c0"
#define ALI_SECRET_KEY                          "50e69b7d948d1bd7ba9ed7b008389010"

#define ALI_SECRET_LEN                          32
#define ALI_AIS_SUPPORT                         1

/* realtek defined ids */
#define DFU_AUTO_BETWEEN_DEVICES                0
#define DFU_PRODUCT_ID                          DFU_PRODUCT_ID_MESH_ALI_LIGHT
#define DFU_APP_VERSION                         0x00000000
#define DFU_PATCH_VERSION                       ({T_IMG_HEADER_FORMAT *p_header = (T_IMG_HEADER_FORMAT *)get_header_addr_by_img_id(RomPatch);\
        (p_header->git_ver.ver_info.sub_version._version_major << 28) + (p_header->git_ver.ver_info.sub_version._version_minor << 20)\
        + (p_header->git_ver.ver_info.sub_version._version_revision << 5) + p_header->git_ver.ver_info.sub_version._version_reserve;})

/* sig mesh defined ids */
#define COMPANY_ID                              0x01A8
#define PRODUCT_ID                              0x0000
#define VERSION_ID                              0x0000

/* mesh ota defined ids */
#define DFU_UPDATER_FW_ID                       BYTES4(ALI_VERSION_ID)

/*============================================================================*
 *                        data ram layout configuration
 *============================================================================*/
/* Data RAM layout:                   112K
example:
   1) reserved for rom and patch:      31K (fixed)
   2) app global + ram code:           35K (adjustable, config APP_GLOBAL_SIZE)
   3) Heap ON:                         30K (adjustable, config APP_GLOBAL_SIZE)
   6) patch ram code:                  16K (fixed)
*/

/** @brief data ram size for app global variables and code, could be changed, but (APP_GLOBAL_SIZE + HEAP_DATA_ON_SIZE) must be 65k */
#define APP_GLOBAL_SIZE                (30 * 1024)

/** @brief data ram size for heap, could be changed, but (APP_GLOBAL_SIZE + HEAP_DATA_ON_SIZE) must be 65k */
#define HEAP_DATA_ON_SIZE              (65 * 1024 - APP_GLOBAL_SIZE)

/** @brief shared cache ram size (adjustable, config SHARE_CACHE_RAM_SIZE: 0/8KB/16KB) */
#define SHARE_CACHE_RAM_SIZE           (0 * 1024)


#ifdef __cplusplus
}
#endif


/** @} */ /* End of group MEM_CONFIG */
#endif
