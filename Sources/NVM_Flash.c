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
 * @ 名称: NVM_Flash.c
 * @ 描述:
 * @ 作者: Tomy
 * @ 日期: 2021年2月2日
 * @ 版本: V1.0
 * @ 历史: V1.0 2021年2月2日 Summary
 */

#include "NVM_Flash.h"

/*
 * Flash Driver based on:
 * SDK: S32K1xx RTM SDK 3.0.0
 * IDE: S32 Design Studio for ARM 2.2
 *
 * TODO S32K144_FlashDrv: #01 在 C/C++ Build -> Settings -> Build Steps ->
 * Post-build steps -> Command 编辑框中粘贴以下命令，在编译后可自动以 <ProjName>Extracted.hex 的形式命名，
 * 在工程的根目录输出单独提取的 Flash Driver
 *
 * 例如 Flash Driver 加载到 Bootloader RAM 中的地址为 0x1FFF8010，则以下命令中的地址则为 0x1FFE8010，
 * objcopy 会给 0x1FFE + 1，尚不清楚原因，具体请以最终输出的 Hex 文件为准
 *
 * arm-none-eabi-objcopy.exe -O ihex --change-addresses 0x1FFE8010 -j .NVM_Driver "${BuildArtifactFileBaseName}.elf" "../${ProjName}Extracted.hex"
 */

//uint32_t *g_pFlashDriverRAMSection = (uint32_t *)0x20006FF0u;

tFlashDriverAPIInfo *g_pFlashDriverAPIRAM = NULL;

/* TODO S32K144_FlashDrv: #02 本偏移量需与 .ld 链接文件中的 m_NVM_Driver 起始地址保持一致 */
#define FLASH_DRV_OFFSET    (0x10000u)
#define CAL_OFFSET(funcPtr) ((uint32_t)(funcPtr) - FLASH_DRV_OFFSET)

/* TODO S32K144_FlashDrv: #03 本偏移结构体内成员必须与对应芯片型号的 Bootloader 工程中 flash.h 文件的 tFlashOptInfo 结构体类型的 Flash API 成员一一对应并且数量相等 */
#pragma GCC diagnostic ignored "-Wunused-const-variable="
static const tFlashDriverAPIInfo gs_FlashDriverAPI NVM_DRIVER_SECTION = {
#if defined (S32K116) || defined (S32K142)
    (tpfFLASH_DRV_EraseSector)      CAL_OFFSET(FLASH_DRV_EraseSector),
    (tpfFLASH_DRV_Program)          CAL_OFFSET(FLASH_DRV_Program),
    (tpfFLASH_DRV_VerifySection)    CAL_OFFSET(FLASH_DRV_VerifySection),
    (tpfFLASH_DRV_GetDefaultConfig) CAL_OFFSET(FLASH_DRV_GetDefaultConfig),
#elif defined (S32K144)
//    (tpfFLASH_DRV_EraseAllBlock)    CAL_OFFSET(FLASH_DRV_EraseAllBlock),
//    (tpfFLASH_DRV_VerifyAllBlock)   CAL_OFFSET(FLASH_DRV_VerifyAllBlock),
    (tpfFLASH_DRV_EraseSector)      CAL_OFFSET(FLASH_DRV_EraseSector),
    (tpfFLASH_DRV_VerifySection)    CAL_OFFSET(FLASH_DRV_VerifySection),
//    (tpfFLASH_DRV_EraseSuspend)     CAL_OFFSET(FLASH_DRV_EraseSuspend),
//    (tpfFLASH_DRV_EraseResume)      CAL_OFFSET(FLASH_DRV_EraseResume),
    (tpfFLASH_DRV_Program)          CAL_OFFSET(FLASH_DRV_Program),
    (tpfFLASH_DRV_ProgramCheck)     CAL_OFFSET(FLASH_DRV_ProgramCheck),
//    (tpfFLASH_DRV_ProgramSection)   CAL_OFFSET(FLASH_DRV_ProgramSection),
//    (tpfFLASH_DRV_EraseBlock)       CAL_OFFSET(FLASH_DRV_EraseBlock),
//    (tpfFLASH_DRV_CommandSequence)  CAL_OFFSET(FLASH_DRV_CommandSequence),
//    (tpfFLASH_DRV_VerifyBlock)      CAL_OFFSET(FLASH_DRV_VerifyBlock),
#endif
};

/* TODO S32K144_FlashDrv: #04 Flash 驱动函数表：包含前16（0x10）字节的指针映射偏移表以及相应的4个驱动函数
 * 编译后，通过 J-Flash 打开 Hex 文件提取对应地址处的内容，粘贴到以下数组即可测试
 */
uint8_t g_flashDriverRAM[] = {
    0x11, 0x00, 0x00, 0x00, 0x49, 0x01, 0x00, 0x00, 0x65, 0x02, 0x00, 0x00, 0xB1, 0x03, 0x00, 0x00,
    0x80, 0xB5, 0x88, 0xB0, 0x00, 0xAF, 0xF8, 0x60, 0xB9, 0x60, 0x7A, 0x60, 0x00, 0x23, 0x7B, 0x82,
    0x7B, 0x68, 0xBB, 0x61, 0xFB, 0x68, 0x9B, 0x68, 0x7B, 0x61, 0xBA, 0x68, 0x7B, 0x69, 0x9A, 0x42,
    0x10, 0xD3, 0xFB, 0x68, 0xDA, 0x68, 0x7B, 0x69, 0x1A, 0x44, 0xBB, 0x68, 0x9A, 0x42, 0x09, 0xD9,
    0xBA, 0x68, 0x7B, 0x69, 0xD3, 0x1A, 0x03, 0xF5, 0x00, 0x03, 0xBB, 0x60, 0x4F, 0xF4, 0x00, 0x63,
    0xFB, 0x61, 0x1B, 0xE0, 0xFB, 0x68, 0x1B, 0x68, 0x7B, 0x61, 0xBA, 0x68, 0x7B, 0x69, 0x9A, 0x42,
    0x0E, 0xD3, 0xFB, 0x68, 0x5A, 0x68, 0x7B, 0x69, 0x1A, 0x44, 0xBB, 0x68, 0x9A, 0x42, 0x07, 0xD9,
    0xBA, 0x68, 0x7B, 0x69, 0xD3, 0x1A, 0xBB, 0x60, 0x4F, 0xF4, 0x80, 0x53, 0xFB, 0x61, 0x05, 0xE0,
    0x01, 0x23, 0x7B, 0x82, 0x00, 0x23, 0xBB, 0x61, 0x00, 0x23, 0xFB, 0x61, 0xFB, 0x69, 0x5A, 0x1E,
    0xBB, 0x69, 0x13, 0x40, 0x00, 0x2B, 0x49, 0xD0, 0x01, 0x23, 0x7B, 0x82, 0x46, 0xE0, 0x29, 0x4B,
    0x1B, 0x78, 0xDB, 0xB2, 0x5B, 0xB2, 0x00, 0x2B, 0x02, 0xDB, 0x02, 0x23, 0x7B, 0x82, 0x3D, 0xE0,
    0x24, 0x4B, 0x70, 0x22, 0x1A, 0x70, 0x23, 0x4B, 0x09, 0x22, 0xDA, 0x71, 0x21, 0x4A, 0xBB, 0x68,
    0x1B, 0x0C, 0xDB, 0xB2, 0x93, 0x71, 0x1F, 0x4A, 0xBB, 0x68, 0x1B, 0x0A, 0xDB, 0xB2, 0x53, 0x71,
    0x1C, 0x4B, 0xBA, 0x68, 0xD2, 0xB2, 0x1A, 0x71, 0x00, 0x23, 0x7B, 0x82, 0x19, 0x4A, 0x19, 0x4B,
    0x1B, 0x78, 0xDB, 0xB2, 0x63, 0xF0, 0x7F, 0x03, 0xDB, 0xB2, 0x13, 0x70, 0x07, 0xE0, 0xFB, 0x68,
    0x9B, 0x69, 0xB3, 0xF1, 0xFF, 0x3F, 0x02, 0xD0, 0xFB, 0x68, 0x9B, 0x69, 0x98, 0x47, 0x11, 0x4B,
    0x1B, 0x78, 0xDB, 0xB2, 0x5B, 0xB2, 0x00, 0x2B, 0xF1, 0xDA, 0x0E, 0x4B, 0x1B, 0x78, 0xDB, 0xB2,
    0x03, 0xF0, 0x71, 0x03, 0x00, 0x2B, 0x01, 0xD0, 0x01, 0x23, 0x7B, 0x82, 0xBA, 0x69, 0xFB, 0x69,
    0xD3, 0x1A, 0xBB, 0x61, 0xBA, 0x68, 0xFB, 0x69, 0x13, 0x44, 0xBB, 0x60, 0xBB, 0x69, 0x00, 0x2B,
    0x02, 0xD0, 0x7B, 0x8A, 0x00, 0x2B, 0xB2, 0xD0, 0x7B, 0x8A, 0x18, 0x46, 0x20, 0x37, 0xBD, 0x46,
    0x80, 0xBD, 0x00, 0xBF, 0x00, 0x00, 0x02, 0x40, 0x80, 0xB5, 0x86, 0xB0, 0x00, 0xAF, 0xF8, 0x60,
    0xB9, 0x60, 0x11, 0x46, 0x1A, 0x46, 0x0B, 0x46, 0xFB, 0x80, 0x13, 0x46, 0x7B, 0x71, 0x00, 0x23,
    0x7B, 0x82, 0xFB, 0x68, 0x9B, 0x68, 0x7B, 0x61, 0xBA, 0x68, 0x7B, 0x69, 0x9A, 0x42, 0x0D, 0xD3,
    0xFB, 0x68, 0xDA, 0x68, 0x7B, 0x69, 0x1A, 0x44, 0xBB, 0x68, 0x9A, 0x42, 0x06, 0xD9, 0xBA, 0x68,
    0x7B, 0x69, 0xD3, 0x1A, 0x03, 0xF5, 0x00, 0x03, 0xBB, 0x60, 0x14, 0xE0, 0xFB, 0x68, 0x1B, 0x68,
    0x7B, 0x61, 0xBA, 0x68, 0x7B, 0x69, 0x9A, 0x42, 0x0B, 0xD3, 0xFB, 0x68, 0x5A, 0x68, 0x7B, 0x69,
    0x1A, 0x44, 0xBB, 0x68, 0x9A, 0x42, 0x04, 0xD9, 0xBA, 0x68, 0x7B, 0x69, 0xD3, 0x1A, 0xBB, 0x60,
    0x01, 0xE0, 0x01, 0x23, 0x7B, 0x82, 0x7B, 0x8A, 0x00, 0x2B, 0x4B, 0xD1, 0x28, 0x4B, 0x1B, 0x78,
    0xDB, 0xB2, 0x5B, 0xB2, 0x00, 0x2B, 0x02, 0xDB, 0x02, 0x23, 0x7B, 0x82, 0x42, 0xE0, 0x24, 0x4B,
    0x70, 0x22, 0x1A, 0x70, 0x22, 0x4B, 0x01, 0x22, 0xDA, 0x71, 0x21, 0x4A, 0xBB, 0x68, 0x1B, 0x0C,
    0xDB, 0xB2, 0x93, 0x71, 0x1E, 0x4A, 0xBB, 0x68, 0x1B, 0x0A, 0xDB, 0xB2, 0x53, 0x71, 0x1C, 0x4B,
    0xBA, 0x68, 0xD2, 0xB2, 0x1A, 0x71, 0x1A, 0x4A, 0xFB, 0x88, 0x1B, 0x0A, 0x9B, 0xB2, 0xDB, 0xB2,
    0xD3, 0x72, 0x17, 0x4B, 0xFA, 0x88, 0xD2, 0xB2, 0x9A, 0x72, 0x15, 0x4A, 0x7B, 0x79, 0x53, 0x72,
    0x00, 0x23, 0x7B, 0x82, 0x12, 0x4A, 0x12, 0x4B, 0x1B, 0x78, 0xDB, 0xB2, 0x63, 0xF0, 0x7F, 0x03,
    0xDB, 0xB2, 0x13, 0x70, 0x07, 0xE0, 0xFB, 0x68, 0x9B, 0x69, 0xB3, 0xF1, 0xFF, 0x3F, 0x02, 0xD0,
    0xFB, 0x68, 0x9B, 0x69, 0x98, 0x47, 0x0A, 0x4B, 0x1B, 0x78, 0xDB, 0xB2, 0x5B, 0xB2, 0x00, 0x2B,
    0xF1, 0xDA, 0x07, 0x4B, 0x1B, 0x78, 0xDB, 0xB2, 0x03, 0xF0, 0x71, 0x03, 0x00, 0x2B, 0x01, 0xD0,
    0x01, 0x23, 0x7B, 0x82, 0x7B, 0x8A, 0x18, 0x46, 0x18, 0x37, 0xBD, 0x46, 0x80, 0xBD, 0x00, 0xBF,
    0x00, 0x00, 0x02, 0x40, 0x80, 0xB5, 0x88, 0xB0, 0x00, 0xAF, 0xF8, 0x60, 0xB9, 0x60, 0x7A, 0x60,
    0x3B, 0x60, 0x00, 0x23, 0xFB, 0x82, 0x7B, 0x68, 0x03, 0xF0, 0x07, 0x03, 0x00, 0x2B, 0x02, 0xD0,
    0x01, 0x23, 0xFB, 0x82, 0x8B, 0xE0, 0xFB, 0x68, 0x9B, 0x68, 0xBB, 0x61, 0xBA, 0x68, 0xBB, 0x69,
    0x9A, 0x42, 0x0D, 0xD3, 0xFB, 0x68, 0xDA, 0x68, 0xBB, 0x69, 0x1A, 0x44, 0xBB, 0x68, 0x9A, 0x42,
    0x06, 0xD9, 0xBA, 0x68, 0xBB, 0x69, 0xD3, 0x1A, 0x03, 0xF5, 0x00, 0x03, 0xBB, 0x60, 0x14, 0xE0,
    0xFB, 0x68, 0x1B, 0x68, 0xBB, 0x61, 0xBA, 0x68, 0xBB, 0x69, 0x9A, 0x42, 0x0B, 0xD3, 0xFB, 0x68,
    0x5A, 0x68, 0xBB, 0x69, 0x1A, 0x44, 0xBB, 0x68, 0x9A, 0x42, 0x04, 0xD9, 0xBA, 0x68, 0xBB, 0x69,
    0xD3, 0x1A, 0xBB, 0x60, 0x01, 0xE0, 0x01, 0x23, 0xFB, 0x82, 0x5A, 0xE0, 0x32, 0x4B, 0x1B, 0x78,
    0xDB, 0xB2, 0x5B, 0xB2, 0x00, 0x2B, 0x02, 0xDB, 0x02, 0x23, 0xFB, 0x82, 0x51, 0xE0, 0x2E, 0x4B,
    0x70, 0x22, 0x1A, 0x70, 0x2C, 0x4B, 0x07, 0x22, 0xDA, 0x71, 0x2B, 0x4A, 0xBB, 0x68, 0x1B, 0x0C,
    0xDB, 0xB2, 0x93, 0x71, 0x28, 0x4A, 0xBB, 0x68, 0x1B, 0x0A, 0xDB, 0xB2, 0x53, 0x71, 0x26, 0x4B,
    0xBA, 0x68, 0xD2, 0xB2, 0x1A, 0x71, 0x00, 0x23, 0xFB, 0x77, 0x0C, 0xE0, 0xFA, 0x7F, 0x23, 0x4B,
    0x13, 0x44, 0xBB, 0x61, 0xBB, 0x69, 0xFA, 0x7F, 0x39, 0x68, 0x0A, 0x44, 0x12, 0x78, 0x1A, 0x70,
    0xFB, 0x7F, 0x01, 0x33, 0xFB, 0x77, 0xFB, 0x7F, 0x07, 0x2B, 0xEF, 0xD9, 0x00, 0x23, 0xFB, 0x82,
    0x19, 0x4A, 0x19, 0x4B, 0x1B, 0x78, 0xDB, 0xB2, 0x63, 0xF0, 0x7F, 0x03, 0xDB, 0xB2, 0x13, 0x70,
    0x07, 0xE0, 0xFB, 0x68, 0x9B, 0x69, 0xB3, 0xF1, 0xFF, 0x3F, 0x02, 0xD0, 0xFB, 0x68, 0x9B, 0x69,
    0x98, 0x47, 0x11, 0x4B, 0x1B, 0x78, 0xDB, 0xB2, 0x5B, 0xB2, 0x00, 0x2B, 0xF1, 0xDA, 0x0E, 0x4B,
    0x1B, 0x78, 0xDB, 0xB2, 0x03, 0xF0, 0x71, 0x03, 0x00, 0x2B, 0x01, 0xD0, 0x01, 0x23, 0xFB, 0x82,
    0xBB, 0x68, 0x08, 0x33, 0xBB, 0x60, 0x7B, 0x68, 0x08, 0x3B, 0x7B, 0x60, 0x3B, 0x68, 0x08, 0x33,
    0x3B, 0x60, 0x7B, 0x68, 0x00, 0x2B, 0x02, 0xD0, 0xFB, 0x8A, 0x00, 0x2B, 0x9E, 0xD0, 0xFB, 0x8A,
    0x18, 0x46, 0x20, 0x37, 0xBD, 0x46, 0x80, 0xBD, 0x00, 0x00, 0x02, 0x40, 0x08, 0x00, 0x02, 0x40,
    0x80, 0xB5, 0x8A, 0xB0, 0x00, 0xAF, 0xF8, 0x60, 0xB9, 0x60, 0x7A, 0x60, 0x3B, 0x60, 0x00, 0x23,
    0xFB, 0x82, 0x7B, 0x68, 0x03, 0xF0, 0x03, 0x03, 0x00, 0x2B, 0x02, 0xD0, 0x01, 0x23, 0xFB, 0x82,
    0xA8, 0xE0, 0x7B, 0x68, 0x3B, 0x62, 0xFB, 0x68, 0x9B, 0x68, 0x7B, 0x62, 0xBA, 0x68, 0x7B, 0x6A,
    0x9A, 0x42, 0x0D, 0xD3, 0xFB, 0x68, 0xDA, 0x68, 0x7B, 0x6A, 0x1A, 0x44, 0xBB, 0x68, 0x9A, 0x42,
    0x06, 0xD9, 0xBA, 0x68, 0x7B, 0x6A, 0xD3, 0x1A, 0x03, 0xF5, 0x00, 0x03, 0xBB, 0x60, 0x16, 0xE0,
    0xFB, 0x68, 0x1B, 0x68, 0x7B, 0x62, 0xBA, 0x68, 0x7B, 0x6A, 0x9A, 0x42, 0x0B, 0xD3, 0xFB, 0x68,
    0x5A, 0x68, 0x7B, 0x6A, 0x1A, 0x44, 0xBB, 0x68, 0x9A, 0x42, 0x04, 0xD9, 0xBA, 0x68, 0x7B, 0x6A,
    0xD3, 0x1A, 0xBB, 0x60, 0x03, 0xE0, 0x01, 0x23, 0xFB, 0x82, 0x00, 0x23, 0x3B, 0x62, 0x73, 0xE0,
    0x3F, 0x4B, 0x1B, 0x78, 0xDB, 0xB2, 0x5B, 0xB2, 0x00, 0x2B, 0x02, 0xDB, 0x02, 0x23, 0xFB, 0x82,
    0x6A, 0xE0, 0x3B, 0x4B, 0x70, 0x22, 0x1A, 0x70, 0x39, 0x4B, 0x02, 0x22, 0xDA, 0x71, 0x38, 0x4A,
    0xBB, 0x68, 0x1B, 0x0C, 0xDB, 0xB2, 0x93, 0x71, 0x35, 0x4A, 0xBB, 0x68, 0x1B, 0x0A, 0xDB, 0xB2,
    0x53, 0x71, 0x33, 0x4B, 0xBA, 0x68, 0xD2, 0xB2, 0x1A, 0x71, 0x31, 0x4A, 0x97, 0xF8, 0x34, 0x30,
    0xD3, 0x72, 0x00, 0x23, 0xFB, 0x77, 0x0C, 0xE0, 0xFA, 0x7F, 0x2E, 0x4B, 0x13, 0x44, 0xBB, 0x61,
    0xBB, 0x69, 0xFA, 0x7F, 0x39, 0x68, 0x0A, 0x44, 0x12, 0x78, 0x1A, 0x70, 0xFB, 0x7F, 0x01, 0x33,
    0xFB, 0x77, 0xFB, 0x7F, 0x03, 0x2B, 0xEF, 0xD9, 0x00, 0x23, 0xFB, 0x82, 0x24, 0x4A, 0x24, 0x4B,
    0x1B, 0x78, 0xDB, 0xB2, 0x63, 0xF0, 0x7F, 0x03, 0xDB, 0xB2, 0x13, 0x70, 0x07, 0xE0, 0xFB, 0x68,
    0x9B, 0x69, 0xB3, 0xF1, 0xFF, 0x3F, 0x02, 0xD0, 0xFB, 0x68, 0x9B, 0x69, 0x98, 0x47, 0x1C, 0x4B,
    0x1B, 0x78, 0xDB, 0xB2, 0x5B, 0xB2, 0x00, 0x2B, 0xF1, 0xDA, 0x19, 0x4B, 0x1B, 0x78, 0xDB, 0xB2,
    0x03, 0xF0, 0x71, 0x03, 0x00, 0x2B, 0x01, 0xD0, 0x01, 0x23, 0xFB, 0x82, 0xFB, 0x8A, 0x00, 0x2B,
    0x11, 0xD0, 0xBB, 0x68, 0xB3, 0xF5, 0x00, 0x0F, 0x07, 0xD3, 0xBA, 0x68, 0x7B, 0x6A, 0x13, 0x44,
    0xA3, 0xF5, 0x00, 0x02, 0x3B, 0x6B, 0x1A, 0x60, 0x0E, 0xE0, 0xBA, 0x68, 0x7B, 0x6A, 0x1A, 0x44,
    0x3B, 0x6B, 0x1A, 0x60, 0x08, 0xE0, 0x3B, 0x6A, 0x04, 0x3B, 0x3B, 0x62, 0x3B, 0x68, 0x04, 0x33,
    0x3B, 0x60, 0xBB, 0x68, 0x04, 0x33, 0xBB, 0x60, 0x3B, 0x6A, 0x00, 0x2B, 0x02, 0xD0, 0xFB, 0x8A,
    0x00, 0x2B, 0x85, 0xD0, 0xFB, 0x8A, 0x18, 0x46, 0x28, 0x37, 0xBD, 0x46, 0x80, 0xBD, 0x00, 0xBF,
    0x00, 0x00, 0x02, 0x40, 0x0C, 0x00, 0x02, 0x40
};

#if 0 /* 此表用途未知，故预编译条件注释 */
uint32_t g_codeRAMBackup[] = {
#if 1
    0xB084B580, 0x6078AF00, 0x18FB230E, 0x801A2200, 0x4B144914, 0xB2DB781B, 0x42522280, 0xB2DB4313, 0xE006700B, 0x699B687B,
    0xD0023301, 0x699B687B, 0x4B0C4798, 0xB2DB781B, 0x2B00B25B, 0x4B09DAF2, 0xB2DB781B, 0x2371001A, 0xD0034013, 0x18FB230E,
    0x801A2201, 0x18FB230E, 0x0018881B, 0xB00446BD, 0x46C0BD80, 0x40020000
#else
    0xB580, 0xB084, 0xAF00, 0x6078, 0x230E, 0x18FB, 0x2200, 0x801A, 0x4914, 0x4B14,
    0x781B, 0xB2DB, 0x2280, 0x4252, 0x4313, 0xB2DB, 0x700B, 0xE006, 0x687B, 0x699B,
    0x3301, 0xD002, 0x687B, 0x699B, 0x4798, 0x4B0C, 0x781B, 0xB2DB, 0xB25B, 0x2B00,
    0xDAF2, 0x4B09, 0x781B, 0xB2DB, 0x001A, 0x2371, 0x4013, 0xD003, 0x230E, 0x18FB,
    0x2201, 0x801A, 0x230E, 0x18FB, 0x881B, 0x0018, 0x46BD, 0xB004, 0xBD80, 0x46C0,
    0x0000, 0x4002
#endif
};
#endif

/*******************************************************************************
* @name   : DTek_TestFlashDriver
* @brief  : 用于测试 Flash API
* @param  : void
* @retval : void
*******************************************************************************/
#if defined (S32K116) || defined (S32K142)
void DTek_TestFlashDriver(void)
{
    flash_user_config_t FlashUserConfig;
    flash_ssd_config_t SSDFlashConfig;
    status_t result = STATUS_SUCCESS;
    uint32_t destAddr = 0x18000u;
    uint8_t errorCnt = 0u;

    /* TODO S32K144_FlashDrv: #05 注意不同MCU的Sector Size不同，需要进行相应调整，否则FLASH_DRV_EraseSector会执行失败 */
    uint32_t size = FEATURE_FLS_PF_BLOCK_SECTOR_SIZE;
    uint8_t aDataBuf[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

#ifdef EN_FLASH_DRIVER_DEBUG /* TODO S32K144_FlashDrv: #06 This macro is in the header file: flash_driver.h */
    g_pFlashDriverAPIRAM = (tFlashDriverAPIInfo *)g_flashDriverRAM;
    g_pFlashDriverAPIRAM->pfFLASH_DRV_EraseSector      = FLASH_DRV_EraseSector;
    g_pFlashDriverAPIRAM->pfFLASH_DRV_Program          = FLASH_DRV_Program;
    g_pFlashDriverAPIRAM->pfFLASH_DRV_VerifySection    = FLASH_DRV_VerifySection;
    g_pFlashDriverAPIRAM->pfFLASH_DRV_GetDefaultConfig = FLASH_DRV_GetDefaultConfig;
    g_pFlashDriverAPIRAM->pfFLASH_DRV_GetDefaultConfig((flash_user_config_t *const )&FlashUserConfig);
#else
    g_pFlashDriverAPIRAM = (tFlashDriverAPIInfo *)g_flashDriverRAM;
    g_pFlashDriverAPIRAM->pfFLASH_DRV_EraseSector      = (tpfFLASH_DRV_EraseSector)     ((uint32_t)g_flashDriverRAM + (uint32_t)(g_pFlashDriverAPIRAM->pfFLASH_DRV_EraseSector));
    g_pFlashDriverAPIRAM->pfFLASH_DRV_Program          = (tpfFLASH_DRV_Program)         ((uint32_t)g_flashDriverRAM + (uint32_t)(g_pFlashDriverAPIRAM->pfFLASH_DRV_Program));
    g_pFlashDriverAPIRAM->pfFLASH_DRV_VerifySection    = (tpfFLASH_DRV_VerifySection)   ((uint32_t)g_flashDriverRAM + (uint32_t)(g_pFlashDriverAPIRAM->pfFLASH_DRV_VerifySection));
    g_pFlashDriverAPIRAM->pfFLASH_DRV_GetDefaultConfig = (tpfFLASH_DRV_GetDefaultConfig)((uint32_t)g_flashDriverRAM + (uint32_t)(g_pFlashDriverAPIRAM->pfFLASH_DRV_GetDefaultConfig));

    g_pFlashDriverAPIRAM->pfFLASH_DRV_GetDefaultConfig((flash_user_config_t *const)&FlashUserConfig);
#endif

    MSCM->OCMDR[0u] |= MSCM_OCMDR_OCM1(0x3u);
    MSCM->OCMDR[1u] |= MSCM_OCMDR_OCM1(0x3u);

    INT_SYS_DisableIRQGlobal();

    result = FLASH_DRV_Init((flash_user_config_t *const)&FlashUserConfig, (flash_ssd_config_t *const)&SSDFlashConfig);

    if (STATUS_SUCCESS == result) {
        result = g_pFlashDriverAPIRAM->pfFLASH_DRV_EraseSector(&SSDFlashConfig, destAddr, size);
    } else {
        errorCnt++;
    }

    if (STATUS_SUCCESS == result) {
        result = g_pFlashDriverAPIRAM->pfFLASH_DRV_VerifySection(&SSDFlashConfig, destAddr, size / FTFx_DPHRASE_SIZE, 1);
    } else {
        errorCnt++;
    }

    if (STATUS_SUCCESS == result) {
        while ((0u != size) && (STATUS_SUCCESS == result)) {
            result = g_pFlashDriverAPIRAM->pfFLASH_DRV_Program(&SSDFlashConfig, destAddr, 8u, aDataBuf);
            size -= 8u;
            destAddr += 8u;
        }
    } else {
        errorCnt++;
    }

    INT_SYS_EnableIRQGlobal();

    for (;;) {
        ;
    }
}
#elif defined (S32K144)
#define BUFFER_SIZE         0x100u  /* Size of data source */
void DTek_TestFlashDriver(void)
{
    flash_user_config_t FlashUserConfig;
    flash_ssd_config_t SSDFlashConfig;
    status_t result = STATUS_SUCCESS;
    uint32_t destAddr = 20u * FEATURE_FLS_PF_BLOCK_SECTOR_SIZE; /* Erase the sixth PFlash sector */
    uint8_t errorCnt = 0u;
    uint32_t failAddr;

    /* TODO S32K144_FlashDrv: #05 注意不同MCU的Sector Size不同，需要进行相应调整，否则FLASH_DRV_EraseSector会执行失败 */
    uint32_t size = FEATURE_FLS_PF_BLOCK_SECTOR_SIZE;
    uint8_t sourceBuffer[BUFFER_SIZE];

    for (uint32_t i = 0u; i < BUFFER_SIZE; i++) {
        sourceBuffer[i] = i;
    }

#ifdef EN_FLASH_DRIVER_DEBUG /* TODO S32K144_FlashDrv: #06 This macro is in the header file: flash_driver.h */
    g_pFlashDriverAPIRAM = (tFlashDriverAPIInfo *)g_flashDriverRAM;
    g_pFlashDriverAPIRAM->pfFLASH_DRV_EraseSector      = FLASH_DRV_EraseSector;
    g_pFlashDriverAPIRAM->pfFLASH_DRV_VerifySection    = FLASH_DRV_VerifySection;
    g_pFlashDriverAPIRAM->pfFLASH_DRV_Program          = FLASH_DRV_Program;
    g_pFlashDriverAPIRAM->pfFLASH_DRV_ProgramCheck     = FLASH_DRV_ProgramCheck;
#else
    g_pFlashDriverAPIRAM = (tFlashDriverAPIInfo *)g_flashDriverRAM;
    g_pFlashDriverAPIRAM->pfFLASH_DRV_EraseSector      = (tpfFLASH_DRV_EraseSector)     ((uint32_t)g_flashDriverRAM + (uint32_t)(g_pFlashDriverAPIRAM->pfFLASH_DRV_EraseSector));
    g_pFlashDriverAPIRAM->pfFLASH_DRV_VerifySection    = (tpfFLASH_DRV_VerifySection)   ((uint32_t)g_flashDriverRAM + (uint32_t)(g_pFlashDriverAPIRAM->pfFLASH_DRV_VerifySection));
    g_pFlashDriverAPIRAM->pfFLASH_DRV_Program          = (tpfFLASH_DRV_Program)         ((uint32_t)g_flashDriverRAM + (uint32_t)(g_pFlashDriverAPIRAM->pfFLASH_DRV_Program));
    g_pFlashDriverAPIRAM->pfFLASH_DRV_ProgramCheck     = (tpfFLASH_DRV_ProgramCheck)    ((uint32_t)g_flashDriverRAM + (uint32_t)(g_pFlashDriverAPIRAM->pfFLASH_DRV_ProgramCheck));
#endif

    FLASH_DRV_GetDefaultConfig((flash_user_config_t *const )&FlashUserConfig);

    MSCM->OCMDR[0u] |= MSCM_OCMDR_OCM1(0x3u);
    MSCM->OCMDR[1u] |= MSCM_OCMDR_OCM1(0x3u);

    INT_SYS_DisableIRQGlobal();

    result = FLASH_DRV_Init((flash_user_config_t *const)&FlashUserConfig, (flash_ssd_config_t *const)&SSDFlashConfig);

    if (STATUS_SUCCESS == result) {
        result = g_pFlashDriverAPIRAM->pfFLASH_DRV_EraseSector(&SSDFlashConfig, destAddr, size);
    } else {
        errorCnt++;
    }


    if (STATUS_SUCCESS == result) {
        /* Verify the erase operation at margin level value of 1, user read */
        result = g_pFlashDriverAPIRAM->pfFLASH_DRV_VerifySection(&SSDFlashConfig, destAddr, size / FTFx_DPHRASE_SIZE, 1u);
    } else {
        errorCnt++;
    }

    size = BUFFER_SIZE;

    if (STATUS_SUCCESS == result) {
        /* Write some data to the erased PFlash sector */
        result = g_pFlashDriverAPIRAM->pfFLASH_DRV_Program(&SSDFlashConfig, destAddr, size, sourceBuffer);
    } else {
        errorCnt++;
    }


    if (STATUS_SUCCESS == result) {
        /* Verify the program operation at margin level value of 1, user margin */
        result = g_pFlashDriverAPIRAM->pfFLASH_DRV_ProgramCheck(&SSDFlashConfig, destAddr, size, sourceBuffer, &failAddr, 1u);
    } else {
        errorCnt++;
    }

    INT_SYS_EnableIRQGlobal();

#if (FEATURE_FLS_HAS_FLEX_NVM == 1u)

    destAddr = SSDFlashConfig.DFlashBase;
    size = FEATURE_FLS_DF_BLOCK_SECTOR_SIZE;

    if (STATUS_SUCCESS == result) {
        /* Erase a sector in DFlash */
        result = g_pFlashDriverAPIRAM->pfFLASH_DRV_EraseSector(&SSDFlashConfig, destAddr, size);
    } else {
        errorCnt++;
    }

    if (STATUS_SUCCESS == result) {
        /* Verify the erase operation at margin level value of 1, user read */
        result = g_pFlashDriverAPIRAM->pfFLASH_DRV_VerifySection(&SSDFlashConfig, destAddr, size / FTFx_PHRASE_SIZE, 1u);
    } else {
        errorCnt++;
    }


    destAddr = SSDFlashConfig.DFlashBase;
    size = BUFFER_SIZE;

    if (STATUS_SUCCESS == result) {
        /* Write some data to the erased DFlash sector */
        result = g_pFlashDriverAPIRAM->pfFLASH_DRV_Program(&SSDFlashConfig, destAddr, size, sourceBuffer);
    } else {
        errorCnt++;
    }

    if (STATUS_SUCCESS == result) {
        /* Verify the program operation at margin level value of 1, user margin */
        result = g_pFlashDriverAPIRAM->pfFLASH_DRV_ProgramCheck(&SSDFlashConfig, destAddr, size, sourceBuffer, &failAddr, 1u);
    } else {
        errorCnt++;
    }

#endif /* FEATURE_FLS_HAS_FLEX_NVM */

    for (;;) {
        ;
    }
}
#endif

/* -------------------------------------------- END OF FILE -------------------------------------------- */
