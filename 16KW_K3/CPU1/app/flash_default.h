/**
 * @file    flash_default.h
 * @brief   Factory defaults and SPI Flash persistence for DSP hold registers (0x6220~0x631F)
 * @date    2026-01-30
 * @author  Nova007457
 */

#ifndef FLASH_DEFAULT_H
#define FLASH_DEFAULT_H

#include "F28x_Project.h"
#include <stdbool.h>

/** Load hold parameters from Flash at boot; if invalid, write factory defaults and valid frame header */
void Init_System_Parameter(void);

/**
 * Save hold parameters to Flash after clearing transient command bits.
 * @param err_cnt  Incremented on save failure (may be NULL)
 * @return true if saved and verified
 */
bool ShutDown_SaveFlash(uint16_t *err_cnt);

/**
 * Factory reset: invalidate frame header, or write factory defaults when actual_reset_factory is true.
 * @param reset_cnt  Incremented on failure (may be NULL)
 * @param actual_reset_factory  true: write UdpPCS_DspHoldPara_Init to Flash
 * @return true on success
 */
bool Restore_Factory_Settings(uint16_t *reset_cnt, bool actual_reset_factory);

/**
 * Auto-save hold parameters to Flash when changed (transient fields excluded from compare).
 * @param err_cnt  Incremented on save failure (may be NULL)
 */
void UdpDspHoldPara_AutoSaveIfNeeded(uint16_t *err_cnt);

#endif /* FLASH_DEFAULT_H */
