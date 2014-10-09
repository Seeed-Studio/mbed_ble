/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "dfu_app_handler.h"
#include "bootloader_util.h"
#include "nrf_sdm.h"
#include "app_error.h"

static void                     dfu_app_reset_prepare(void);                    /**< Forward declare of default reset handler. */
static dfu_app_reset_prepare_t  m_reset_prepare = dfu_app_reset_prepare;        /**< Callback function to application to prepare for system reset. Allows application to cleanup of service and memory prior to reset. */


/**@brief Default reset prepare handler if application hasn't registered a handler.
 */
static void dfu_app_reset_prepare(void)
{
    // Reset prepare should be handled by application.
    // This function can be extended to include default handling if application does not implement
    // own handler.
}


/**@brief Function for preparing the reset, disabling SoftDevice and jump to the bootloader.
 */
void bootloader_start(void)
{
    m_reset_prepare();

    uint32_t err_code = sd_power_gpregret_set(BOOTLOADER_DFU_START);
    APP_ERROR_CHECK(err_code);

    NVIC_SystemReset();
}



void dfu_app_reset_prepare_set(dfu_app_reset_prepare_t reset_prepare_func)
{
    m_reset_prepare = reset_prepare_func;
}
