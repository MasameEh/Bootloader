
// File Name: bootloader.h
// Author:		 Mohamed Sameh
// Date:			 Nov 23, 2023


#ifndef _BOOTLOADER_H
#define _BOOTLOADER_H


/* ------------------ Includes ------------------------------------- */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "crc.h"

/* ------------------ Macro Declarations --------------------------- */			 				
#define BL_DEBUG_UART					   				 (&huart1)
#define BL_HOST_COMMUNICATION_UART		   (&huart2)

#define CRC_ENGINE											 (&hcrc)

#define DEBUG_INFO_DISABLE							 0x00
#define DEBUG_INFO_ENABLE							   0x01
#define BL_DEBUG_INFO_CONTROLL				   DEBUG_INFO_ENABLE

#define BL_ENABLE_UART_DEBUG_MSG 			   0x00
#define BL_ENABLE_SPI_DEBUG_MSG 			   0x01
#define BL_ENABLE_I2C_DEBUG_MSG 			   0x02
#define BL_DEBUG_METHOD				 			 (BL_ENABLE_UART_DEBUG_MSG)

#define BL_HOST_BUFFER_RX_SIZE						200


#define CBL_GET_VER_CMD               0x10
#define CBL_GET_HELP_CMD              0x11
#define CBL_GET_CID_CMD               0x12
/* Get Read Protection Status */
#define CBL_GET_RDP_STATUS_CMD        0x13 
#define CBL_GO_TO_ADDR_CMD            0x14
#define CBL_FLASH_ERASE_CMD           0x15
#define CBL_MEM_WRITE_CMD             0x16
/* Enable/Disable Write Protection */
#define CBL_ED_W_PROTECT_CMD          0x17
#define CBL_MEM_READ_CMD              0x18
/* Get Sector Read/Write Protection Status */
#define CBL_READ_SECTOR_STATUS_CMD   	0x19
#define CBL_OTP_READ_CMD             	0x20
/* Change Read Out Protection Level */
#define CBL_CHANGE_ROP_Level_CMD     	0x21

#define CBL_SEND_ACK  								0xAB
#define CBL_SEND_NACK  								0xCD

#define BL_VEDNOR_ID									7
#define BL_SW_MAJOR_VERSION						1
#define BL_SW_MINOR_VERSION						0
#define BL_SW_PATCH_VERSION						0

#define CRC_VERIFICATION_FAILED				0x00
#define CRC_VERIFICATION_PASSED				0x01

#define CRC_SIZE_BYTE									4

#define ADDRESS_IS_INVALID						0x00
#define ADDRESS_IS_VALID							0x01
/*  CBL_FLASH_ERASE_CMD  */
#define ERASE_FAILED									0x00
#define ERASE_SUCCEEDED							  0x01
#define INVALID_SECTOR_NUMBER					0x02
#define VALID_SECTOR_NUMBER						0x03

// If a user wants to mass erase the flash
#define CBL_FLASH_MASS_ERASE					0xFF

#define FLASH_MAX_SECTOR_NUMBERS			8	

#define APP_START_ADD_FLASH_SECTOR2		0x08008000U


#define STM32F401xx_FLASH_SIZE				(256 * 1024)
#define STM32F401xx_SRAM_SIZE					(64 * 1024)

#define STM32F401xx_FLASH_END				  (STM32F401xx_FLASH_SIZE + FLASH_BASE)
#define STM32F401xx_SRAM_END					(STM32F401xx_SRAM_SIZE + SRAM1_BASE)

#define HAL_SUCCESSFUL_ERASE					0xFFFFFFFFU

/* CBL_MEM_WRITE_CMD */
#define FLASH_MEMORY_WRITE_FAILED			0x00
#define FLASH_MEMORY_WRITE_PASSED			0x01	

/* CBL_GET_RDP_STATUS_CMD */
#define CBL_GET_RDP_FAILED						0x00	
#define CBL_GET_RDP_PASSED						0x01

/* CBL_CHANGE_ROP_Level_CMD */
#define CBL_CHANGE_RDP_FAILED						0x00	
#define CBL_CHANGE_RDP_PASSED						0x01
/* ------------------ Macro Functions Declarations ----------------- */


/* ------------------ Data Types Declarations ---------------------- */
typedef enum
{
	BL_ERROR=0,
	BL_OK
}BL_Status;

typedef void (*pfun)(void);
/* ------------------ Software Interfaces Declarations ------------- */
void BL_Print_Message(char *format, ...);
BL_Status BL_UART_Fetch_Host_Command(void);

#endif /*_BOOTLOADER_H*/