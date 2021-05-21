/*
 *          ___                        ___                   _____          ___
 *         /__/\          ___         /__/\                 /  /::\        /  /\          ___
 *         \  \:\        /__/\       |  |::\               /  /:/\:\      /  /::\        /__/\
 *          \  \:\       \  \:\      |  |:|:\             /  /:/  \:\    /  /:/\:\       \  \:\
 *      _____\__\:\       \  \:\   __|__|:|\:\           /__/:/ \__\:|  /  /:/~/:/        \  \:\
 *     /__/::::::::\  ___  \__\:\ /__/::::| \:\          \  \:\ /  /:/ /__/:/ /:/___  ___  \__\:\
 *     \  \:\~~\~~\/ /__/\ |  |:| \  \:\~~\__\/           \  \:\  /:/  \  \:\/:::::/ /__/\ |  |:|
 *      \  \:\  ~~~  \  \:\|  |:|  \  \:\                  \  \:\/:/    \  \::/~~~~  \  \:\|  |:|
 *       \  \:\       \  \:\__|:|   \  \:\                  \  \::/      \  \:\       \  \:\__|:|
 *        \  \:\       \__\::::/     \  \:\                  \__\/        \  \:\       \__\::::/
 *         \__\/           ~~~~       \__\/                                \__\/           ~~~~
 *
 *
 * @ ����: NVM_Flash.h
 * @ ����:
 * @ ����: Tomy
 * @ ����: 2021��2��2��
 * @ �汾: V1.0
 * @ ��ʷ: V1.0 2021��2��2�� Summary
 */

#ifndef NVM_FLASH_H_
#define NVM_FLASH_H_

#include "flash_driver.h"
#include "interrupt_manager.h"

typedef status_t (*tpfFLASH_DRV_EraseSector)    (const flash_ssd_config_t *pSSDConfig, uint32_t dest, uint32_t size);
typedef status_t (*tpfFLASH_DRV_VerifySection)  (const flash_ssd_config_t *pSSDConfig, uint32_t dest, uint16_t number, uint8_t marginLevel);
typedef status_t (*tpfFLASH_DRV_Program)        (const flash_ssd_config_t *pSSDConfig, uint32_t dest, uint32_t size, const uint8_t *pData);
typedef status_t (*tpfFLASH_DRV_ProgramCheck)   (const flash_ssd_config_t *pSSDConfig, uint32_t dest, uint32_t size, const uint8_t *pExpectedData, uint32_t *pFailAddr, uint8_t marginLevel);

typedef struct
{
    tpfFLASH_DRV_EraseSector    pfFLASH_DRV_EraseSector;
    tpfFLASH_DRV_VerifySection  pfFLASH_DRV_VerifySection;
    tpfFLASH_DRV_Program        pfFLASH_DRV_Program;
    tpfFLASH_DRV_ProgramCheck   pfFLASH_DRV_ProgramCheck;
} tFlashDriverAPIInfo;

void NVM_TestFlashDriver(void);

#endif /* NVM_FLASH_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
