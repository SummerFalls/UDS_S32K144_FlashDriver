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
 * @ 名称: NVM_Flash.h
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月2日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月2日 Summary
 */

#ifndef NVM_FLASH_H_
#define NVM_FLASH_H_

#include "flash_driver.h"
#include "interrupt_manager.h"

/* TODO S32K_FlashDrv: #00 MCU 型号选择，影响生成的 Flash Driver 所包含的 API，与对应芯片型号的 Bootloader 工程对应 */
//#define S32K116
//#define S32K118
//#define S32K142
#define S32K144


typedef status_t (*tpfFLASH_DRV_EraseAllBlock)(const flash_ssd_config_t *pSSDConfig);
typedef status_t (*tpfFLASH_DRV_VerifyAllBlock)(const flash_ssd_config_t *pSSDConfig, uint8_t marginLevel);
typedef status_t (*tpfFLASH_DRV_EraseSector)(const flash_ssd_config_t *pSSDConfig, uint32_t dest, uint32_t size);
typedef status_t (*tpfFLASH_DRV_VerifySection)(const flash_ssd_config_t *pSSDConfig, uint32_t dest, uint16_t number, uint8_t marginLevel);
typedef void     (*tpfFLASH_DRV_EraseSuspend)(void);
typedef void     (*tpfFLASH_DRV_EraseResume)(void);
typedef status_t (*tpfFLASH_DRV_Program)(const flash_ssd_config_t *pSSDConfig, uint32_t dest, uint32_t size, const uint8_t *pData);
typedef status_t (*tpfFLASH_DRV_ProgramCheck)(const flash_ssd_config_t *pSSDConfig, uint32_t dest, uint32_t size, const uint8_t *pExpectedData, uint32_t *pFailAddr, uint8_t marginLevel);
typedef status_t (*tpfFLASH_DRV_ProgramSection)(const flash_ssd_config_t *pSSDConfig, uint32_t dest, uint16_t number);
typedef status_t (*tpfFLASH_DRV_EraseBlock)(const flash_ssd_config_t *pSSDConfig, uint32_t dest);
typedef status_t (*tpfFLASH_DRV_CommandSequence)(const flash_ssd_config_t *pSSDConfig);
typedef status_t (*tpfFLASH_DRV_VerifyBlock)(const flash_ssd_config_t *pSSDConfig, uint32_t dest, uint8_t marginLevel);
typedef status_t (*tpfFLASH_DRV_Init)(const flash_user_config_t *const pUserConf, flash_ssd_config_t *const pSSDConfig);
typedef void     (*tpfFLASH_DRV_GetDefaultConfig)(flash_user_config_t *const config);

typedef struct {
#if defined (S32K116) || defined (S32K142)
    tpfFLASH_DRV_EraseSector pfFLASH_DRV_EraseSector;           /* erase sector flash memory */
    tpfFLASH_DRV_Program pfFLASH_DRV_Program;                   /* program flash */
    tpfFLASH_DRV_VerifySection pfFLASH_DRV_VerifySection;       /* verify flash section */
    tpfFLASH_DRV_GetDefaultConfig pfFLASH_DRV_GetDefaultConfig; /* get default config */
#elif defined (S32K144) || defined (S32K118)
//    tpfFLASH_DRV_EraseAllBlock pfFLASH_DRV_EraseAllBlock;
//    tpfFLASH_DRV_VerifyAllBlock pfFLASH_DRV_VerifyAllBlock;
    tpfFLASH_DRV_EraseSector pfFLASH_DRV_EraseSector;
    tpfFLASH_DRV_VerifySection pfFLASH_DRV_VerifySection;
//    tpfFLASH_DRV_EraseSuspend pfFLASH_DRV_EraseSuspend;
//    tpfFLASH_DRV_EraseResume pfFLASH_DRV_EraseResume;
    tpfFLASH_DRV_Program pfFLASH_DRV_Program;
    tpfFLASH_DRV_ProgramCheck pfFLASH_DRV_ProgramCheck;
//    tpfFLASH_DRV_ProgramSection pfFLASH_DRV_ProgramSection;
//    tpfFLASH_DRV_EraseBlock pfFLASH_DRV_EraseBlock;
//    tpfFLASH_DRV_CommandSequence pfFLASH_DRV_CommandSequence;
//    tpfFLASH_DRV_VerifyBlock pfFLASH_DRV_VerifyBlock;
#else
#error "请选择对应的芯片型号，即取消注释对应宏定义..."
#endif
} tFlashDriverAPIInfo;

#endif /* NVM_FLASH_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
