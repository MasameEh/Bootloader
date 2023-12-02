
// File Name: bootloader.c
// Author:		 Mohamed Sameh
// Date:			 Nov 23, 2023

/* ----------------- Includes ----------------- */
#include "bootloader.h"

/* ----------------- Static Functions Decleration ----------------- */
static void Bootloader_Get_Version(uint8_t *Host_Buffer);
static void Bootloader_Get_Help(uint8_t *Host_Buffer);
static void Bootloader_Get_Chip_Identification_Number(uint8_t *Host_Buffer);
static void Bootloader_Read_Protection_Level(uint8_t *Host_Buffer);
static void Bootloader_Jump_To_Address(uint8_t *Host_Buffer);
static void Bootloader_Erase_Flash(uint8_t *Host_Buffer);
static void Bootloader_Memory_Write(uint8_t *Host_Buffer);
static void Bootloader_Enable_RW_Protection(uint8_t *Host_Buffer);
static void Bootloader_Memory_Read(uint8_t *Host_Buffer);
static void Bootloader_Get_Sector_Protection_Status(uint8_t *Host_Buffer);
static void Bootloader_Read_OTP(uint8_t *Host_Buffer);
static void Bootloader_Change_Read_Protection_Level(uint8_t *Host_Buffer);

static uint8_t Bootloader_CRC_Verify(uint8_t *pData, uint32_t Data_Len, uint32_t Host_CRC);
static void Bootloader_Send_ACK(uint8_t Replay_Len);
static void Bootloader_Send_NACK(void);
static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer, uint32_t Data_Len);
static uint8_t Host_Address_Verification(uint32_t Jump_Address);
static uint8_t Perform_Flash_Erase(uint8_t Sector_Numebr, uint8_t Number_Of_Sectors);
static uint8_t Flash_Memory_Write_Payload(uint8_t *Host_Payload, uint32_t Start_Addr, uint8_t Payload_Len);

/* ----------------- Global Variables Definitions ----------------- */
static uint8_t BL_Host_Buffer[BL_HOST_BUFFER_RX_SIZE];

static uint8_t Bootloader_Supported_CMDs[12] = 
{
	CBL_GET_VER_CMD,
	CBL_GET_HELP_CMD,
	CBL_GET_CID_CMD,
	CBL_GET_RDP_STATUS_CMD,
	CBL_GO_TO_ADDR_CMD,
	CBL_FLASH_ERASE_CMD,
	CBL_MEM_WRITE_CMD,
	CBL_ED_W_PROTECT_CMD,
	CBL_MEM_READ_CMD,
	CBL_READ_SECTOR_STATUS_CMD,
	CBL_OTP_READ_CMD,
	CBL_CHANGE_ROP_Level_CMD,
};

/* -----------------  Software Interfaces Definitions ------------- */
static void BL_Jump_To_App(void)
{
	// Value of the main stack pointer of the application
	uint32_t Msp_Value = (*((volatile uint32_t *)APP_START_ADD_FLASH_SECTOR2));
	// Reset handler function of the application
	uint32_t App_Reset_Handler = (*((volatile uint32_t *)(APP_START_ADD_FLASH_SECTOR2 + 4)));
	
	pfun pResetHandler = (pfun)App_Reset_Handler;
	
	// Set the main stack pointer
	__set_MSP(Msp_Value);
	
	// De-Initialize the Modules
	HAL_RCC_DeInit();
	
	// Jump to application
	pResetHandler();
}

BL_Status BL_UART_Fetch_Host_Command(void)
{
	BL_Status status = BL_ERROR;
	HAL_StatusTypeDef UART_STATUS = HAL_ERROR;
	uint8_t dataLength = 0;
	
	memset(BL_Host_Buffer, 0,BL_HOST_BUFFER_RX_SIZE);
	UART_STATUS = HAL_UART_Receive(BL_HOST_COMMUNICATION_UART, BL_Host_Buffer, 1, HAL_MAX_DELAY);

	if(HAL_ERROR != UART_STATUS)
	{
		dataLength = BL_Host_Buffer[0];
		UART_STATUS = HAL_UART_Receive(BL_HOST_COMMUNICATION_UART, &BL_Host_Buffer[1], dataLength, HAL_MAX_DELAY);
		
		if(HAL_ERROR != UART_STATUS)
		{
			switch(BL_Host_Buffer[1])
			{
				case CBL_GET_VER_CMD:
					BL_Print_Message("CBL_GET_VER_CMD \r\n");
					Bootloader_Get_Version(BL_Host_Buffer);
					status = BL_OK;
					break;
				case CBL_GET_HELP_CMD:
					Bootloader_Get_Help(BL_Host_Buffer);
				  BL_Print_Message("CBL_GET_HELP_CMD \r\n");
					status = BL_OK;
					break;
				case CBL_GET_CID_CMD:
					Bootloader_Get_Chip_Identification_Number(BL_Host_Buffer);
					BL_Print_Message("CBL_GET_CID_CMD \r\n");
					status = BL_OK;
					break;
				case CBL_GET_RDP_STATUS_CMD:
					Bootloader_Read_Protection_Level(BL_Host_Buffer);
					BL_Print_Message("CBL_GET_RDP_STATUS_CMD \r\n");
					status = BL_OK;
					break;
				case CBL_GO_TO_ADDR_CMD:
					Bootloader_Jump_To_Address(BL_Host_Buffer);
					status = BL_OK;
					break;
				case CBL_FLASH_ERASE_CMD:
					Bootloader_Erase_Flash(BL_Host_Buffer);
					status = BL_OK;
					break;
				case CBL_MEM_WRITE_CMD:
					Bootloader_Memory_Write(BL_Host_Buffer);
					status = BL_OK;
					break;
				case CBL_ED_W_PROTECT_CMD:
					BL_Print_Message("Enable or Disable write protect on different sectors of the user flash \r\n");
					Bootloader_Enable_RW_Protection(BL_Host_Buffer);
					status = BL_OK;
					break;
				case CBL_MEM_READ_CMD:
					BL_Print_Message("Read data from different memories of the microcontroller \r\n");
					Bootloader_Memory_Read(BL_Host_Buffer);
					status = BL_OK;
					break;
				case CBL_READ_SECTOR_STATUS_CMD:
					BL_Print_Message("Read all the sector protection status \r\n");
					Bootloader_Get_Sector_Protection_Status(BL_Host_Buffer);
					status = BL_OK;
					break;
				case CBL_OTP_READ_CMD:
					BL_Print_Message("Read the OTP contents \r\n");
					Bootloader_Read_OTP(BL_Host_Buffer);
					status = BL_OK;
					break;
				case CBL_CHANGE_ROP_Level_CMD:
					Bootloader_Change_Read_Protection_Level(BL_Host_Buffer);
					status = BL_OK;
					break;
				default:
					BL_Print_Message("Invalid command code received from host !! \r\n");
					break;
			}
		}
		else
		{
			status = BL_ERROR;
		}
	}
	else
	{
		status = BL_ERROR;
	}
	
	return status;
}


void BL_Print_Message(char *format, ...)
{
	char message[100] = {0};
	va_list list;
	va_start(list, format);
	// Write the formatted data from variable argument list to String 
	vsprintf(message, format, list);
#if BL_DEBUG_METHOD == BL_ENABLE_UART_DEBUG_MSG
	// Transmit the formatted data through UART
	HAL_UART_Transmit(BL_DEBUG_UART, (uint8_t *)message, sizeof(message), HAL_MAX_DELAY);
#elif BL_DEBUG_METHOD == BL_ENABLE_SPI_DEBUG_MSG
	// Transmit the formatted data through SPI

#elif	BL_DEBUG_METHOD == BL_ENABLE_I2C_DEBUG_MSG01
	// Transmit the formatted data through I2C
	
#endif
	// Clean up the variable argument list
	va_end(list);
}

static void Bootloader_Get_Version(uint8_t *Host_Buffer)
{
	uint8_t BL_Version[4] = {BL_VEDNOR_ID, BL_SW_MAJOR_VERSION, BL_SW_MINOR_VERSION, BL_SW_PATCH_VERSION};
	uint8_t Host_CMD_Length = 0;
	uint32_t CRC32 = 0;
	
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("Read Bootloader version from the MCU\r\n");
#endif
	// Extract the CRC sent by the Host
	Host_CMD_Length = Host_Buffer[0] + 1;
	CRC32 = *((uint32_t *)((Host_Buffer + Host_CMD_Length) - CRC_SIZE_BYTE));
	
	// CRC Verification
	if(CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify(Host_Buffer, Host_CMD_Length - CRC_SIZE_BYTE, CRC32))
	{
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("CRC VERIFICATION PASSED\r\n");
#endif
			Bootloader_Send_ACK(4);
			// Transmit the data through UART to the host
			Bootloader_Send_Data_To_Host((uint8_t *)&BL_Version[0], 4);
	}
	else
	{
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("CRC VERIFICATION FAILED\r\n");
#endif
			Bootloader_Send_NACK();
	}
}

static void Bootloader_Get_Help(uint8_t *Host_Buffer)
{
	uint8_t Host_CMD_Length = 0;
	uint32_t CRC32 = 0;
	
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("Read commands supported by the Bootloader \r\n");
#endif
	// Extract the CRC sent by the Host
	Host_CMD_Length = Host_Buffer[0] + 1;
	CRC32 = *((uint32_t *)((Host_Buffer + Host_CMD_Length) - CRC_SIZE_BYTE));
	
	// CRC Verification
	if(CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify((uint8_t *)&Host_Buffer[0], Host_CMD_Length - CRC_SIZE_BYTE, CRC32))
	{
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("CRC VERIFICATION PASSED\r\n");
#endif
			Bootloader_Send_ACK(12);
			// Transmit the data through UART to the host
			Bootloader_Send_Data_To_Host((uint8_t *)&Bootloader_Supported_CMDs[0], 12);
	}
	else
	{
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("CRC VERIFICATION FAILED\r\n");
#endif
			Bootloader_Send_NACK();
	}
}

static void Bootloader_Get_Chip_Identification_Number(uint8_t *Host_Buffer)
{
	uint8_t Host_CMD_Length = 0;
	uint32_t CRC32 = 0;
	uint16_t MCU_ID = 0;
	
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("Read the MCU identification number \r\n");
#endif
	// Extract the CRC sent by the Host
	Host_CMD_Length = Host_Buffer[0] + 1;
	CRC32 = *((uint32_t *)((Host_Buffer + Host_CMD_Length) - CRC_SIZE_BYTE));
	
	// CRC Verification
	if(CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify((uint8_t *)&Host_Buffer[0], Host_CMD_Length - CRC_SIZE_BYTE, CRC32))
	{
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("CRC VERIFICATION PASSED\r\n");
#endif
			// Get the MCU identification number
			MCU_ID = (uint16_t)((DBGMCU->IDCODE) & 0x00000FFF); 
			// Send the MCU identification number to the Host
			Bootloader_Send_ACK(2);
			// Transmit the data through UART to the host
			Bootloader_Send_Data_To_Host((uint8_t *)&MCU_ID, 2);
	}
	else
	{
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("CRC VERIFICATION FAILED\r\n");
#endif
			Bootloader_Send_NACK();
	}
}

static void Bootloader_Read_Protection_Level(uint8_t *Host_Buffer)
{


}

static void Bootloader_Jump_To_Address(uint8_t *Host_Buffer)
{
	uint8_t Host_CMD_Length = 0;
	uint32_t CRC32 = 0;
	uint32_t Host_Jump_Addr = 0;
	uint8_t Addr_Verifictaion = ADDRESS_IS_INVALID;
	
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("Bootloader Jumps to specific address \r\n");
#endif
	
	// Extract the CRC sent by the Host
	Host_CMD_Length = Host_Buffer[0] + 1;
	CRC32 = *((uint32_t *)((Host_Buffer + Host_CMD_Length) - CRC_SIZE_BYTE));
		
	// CRC Verification
	if(CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify(Host_Buffer, Host_CMD_Length - CRC_SIZE_BYTE, CRC32))
	{
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("CRC VERIFICATION PASSED\r\n");
#endif	
		Bootloader_Send_ACK(1);
		// Extract the address which sent by the Host
		Host_Jump_Addr = (*((uint32_t *)&Host_Buffer[2]));
		// Host Jump Address Verification
		Addr_Verifictaion = Host_Address_Verification(Host_Jump_Addr);
		if(ADDRESS_IS_VALID == Addr_Verifictaion)
		{
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("Jump Address is Valid\r\n");
#endif	
			// Report Address is valid
			Bootloader_Send_Data_To_Host(&Addr_Verifictaion, 1);
			pfun JumpAddress = (pfun)(Host_Jump_Addr + 1);
			JumpAddress();
			
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("Jump to : 0x%X \r\n", JumpAddress);
#endif
		}
		else
		{
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("Jump Address is Invalid\r\n");
#endif	
			// Report Address is invalid
			Bootloader_Send_Data_To_Host(&Addr_Verifictaion, 1);
		}
	}
	else
	{
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("CRC VERIFICATION FAILED\r\n");
#endif
			Bootloader_Send_NACK();
	}
}

static void Bootloader_Erase_Flash(uint8_t *Host_Buffer)
{
	uint8_t Host_CMD_Length = 0;
	uint32_t CRC32 = 0;
	uint8_t Erase_Status = ERASE_FAILED;
	
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("Erase the MCU Flash Memory Sectors \r\n");
#endif
	// Extract the CRC sent by the Host
	Host_CMD_Length = Host_Buffer[0] + 1;
	CRC32 = *((uint32_t *)((Host_Buffer + Host_CMD_Length) - CRC_SIZE_BYTE));
	
	// CRC Verification
	if(CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify((uint8_t *)&Host_Buffer[0], Host_CMD_Length - CRC_SIZE_BYTE, CRC32))
	{
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("CRC VERIFICATION PASSED\r\n");
#endif
			Bootloader_Send_ACK(1);
			// Perform an erase from Flash memory 
			Erase_Status = Perform_Flash_Erase(Host_Buffer[2], Host_Buffer[3]);
		
			if(ERASE_SUCCEEDED == Erase_Status)
			{
				// Report Erase Succeeded 
				Bootloader_Send_Data_To_Host(&Erase_Status, 1);
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
				BL_Print_Message("Successful Erase \r\n");
#endif
			}
			else
			{
				// Report Erase Failed 
				Bootloader_Send_Data_To_Host(&Erase_Status, 1);
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
				BL_Print_Message("Failed Erase \r\n");
#endif
			}
	}
	else
	{
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("CRC VERIFICATION FAILED\r\n");
#endif
			Bootloader_Send_NACK();
	}
}


static void Bootloader_Memory_Write(uint8_t *Host_Buffer)
{
	uint8_t Host_CMD_Length = 0;
	uint32_t CRC32 = 0;
	uint8_t Payload_Len = 0;
	uint32_t Host_Addr = 0;
	uint8_t Addr_Verifictaion = ADDRESS_IS_INVALID;
	uint8_t Write_Status = FLASH_MEMORY_WRITE_FAILED;
	
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("Write in Flash Memory\r\n");
#endif
	// Extract the CRC sent by the Host
	Host_CMD_Length = Host_Buffer[0] + 1;
	CRC32 = *((uint32_t *)((Host_Buffer + Host_CMD_Length) - CRC_SIZE_BYTE));
	
	// CRC Verification
	if(CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify((uint8_t *)&Host_Buffer[0], Host_CMD_Length - CRC_SIZE_BYTE, CRC32))
	{
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
		BL_Print_Message("CRC VERIFICATION PASSED\r\n");
#endif
		Bootloader_Send_ACK(1);
		// Extract the start address
		Host_Addr = *((uint32_t *)&Host_Buffer[2]);
		// Extract the payload length
		Payload_Len = Host_Buffer[6];
		// Host start address Verification
		Addr_Verifictaion = Host_Address_Verification(Host_Addr);
		if(ADDRESS_IS_VALID == Addr_Verifictaion)
		{
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("Host Start Address is Valid \r\n");
#endif
			// Write data in the Flash
			Write_Status = Flash_Memory_Write_Payload((uint8_t *)&Host_Buffer[7], Host_Addr, Payload_Len);
			
			if(FLASH_MEMORY_WRITE_PASSED == Write_Status)
			{
				// Report writing passed
				Bootloader_Send_Data_To_Host(&Write_Status, 1);
			}
			else
			{
				// Report writing failed
				Bootloader_Send_Data_To_Host(&Write_Status, 1);
			}
		}
		else
		{
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("Host Start Address is Invalid\r\n");
#endif	
			// Report Address is invalid
			Bootloader_Send_Data_To_Host(&Addr_Verifictaion, 1);
		}
	}
	else
	{
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
		BL_Print_Message("CRC VERIFICATION FAILED\r\n");
#endif
		Bootloader_Send_NACK();
	}

}

static void Bootloader_Enable_RW_Protection(uint8_t *Host_Buffer)
{


}

static void Bootloader_Memory_Read(uint8_t *Host_Buffer)
{


}

static void Bootloader_Get_Sector_Protection_Status(uint8_t *Host_Buffer)
{


}

static void Bootloader_Read_OTP(uint8_t *Host_Buffer)
{


}

static void Bootloader_Change_Read_Protection_Level(uint8_t *Host_Buffer)
{

}

static uint8_t Bootloader_CRC_Verify(uint8_t *pData, uint32_t Data_Len, uint32_t Host_CRC)
{
	uint8_t CRC_Status = CRC_VERIFICATION_FAILED;
	uint32_t CRC_Calculated = 0;
	uint8_t Data_Counter = 0;
	uint32_t Data_Buffer = 0;
	
	// Calculate CRC
	for(Data_Counter = 0; Data_Counter < Data_Len; Data_Counter++)
	{		
			Data_Buffer = (uint32_t)pData[Data_Counter];
			CRC_Calculated = HAL_CRC_Accumulate(CRC_ENGINE, &Data_Buffer, 1);
	}
	
	// Resets the CRC calculation unit 
	__HAL_CRC_DR_RESET(CRC_ENGINE);
	BL_Print_Message("0x%x \r\n", CRC_Calculated);
	// Compare between the received CRC value and the calculated
	if(CRC_Calculated == Host_CRC)
	{
			CRC_Status = CRC_VERIFICATION_PASSED;
	}
	else
	{
		
	}	
	return CRC_Status;
}

static void Bootloader_Send_ACK(uint8_t Replay_Len)
{
	uint8_t ACK_Value[2] = {0};
	ACK_Value[0] = CBL_SEND_ACK;
	ACK_Value[1] = Replay_Len;
	HAL_UART_Transmit(BL_HOST_COMMUNICATION_UART, ACK_Value, 2, HAL_MAX_DELAY);	
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("Bootloader sends ACK\r\n");
#endif
}

static void Bootloader_Send_NACK(void)
{
		uint8_t NACK_Value = CBL_SEND_NACK;
		HAL_UART_Transmit(BL_HOST_COMMUNICATION_UART, &NACK_Value, 1, HAL_MAX_DELAY);	
}

static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer, uint32_t Data_Len)
{
	HAL_UART_Transmit(BL_HOST_COMMUNICATION_UART, Host_Buffer, Data_Len, HAL_MAX_DELAY);
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("Bootloader sends INFO\r\n");
#endif
}

static uint8_t Host_Address_Verification(uint32_t Jump_Address)
{
	uint8_t Addr_Verifictaion = ADDRESS_IS_INVALID;
	
	if(Jump_Address >= SRAM1_BASE &&  Jump_Address <= STM32F401xx_SRAM_END)
	{
		Addr_Verifictaion = ADDRESS_IS_VALID;
	}	
	else if(Jump_Address >= FLASH_BASE &&  Jump_Address <= STM32F401xx_FLASH_END)
	{
		Addr_Verifictaion = ADDRESS_IS_VALID;
	}
	return Addr_Verifictaion;
}


static uint8_t Perform_Flash_Erase(uint8_t Sector_Numebr, uint8_t Number_Of_Sectors)
{
	uint8_t Sector_Validity = INVALID_SECTOR_NUMBER;
	FLASH_EraseInitTypeDef FLASH_Erase_Cfg = {0};
	uint8_t Remaining_Sectors = 0;
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	uint32_t SectorError = 0; 
	
	if(Number_Of_Sectors > FLASH_MAX_SECTOR_NUMBERS)
	{
		// Number of sectors is out of range
		Sector_Validity = INVALID_SECTOR_NUMBER;
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("Inavlid number of sectors \r\n");
#endif
	}
	else
	{
		Sector_Validity = VALID_SECTOR_NUMBER;
		
		if((Sector_Numebr <= (FLASH_MAX_SECTOR_NUMBERS - 1)) || (Sector_Numebr == CBL_FLASH_MASS_ERASE))
		{			
			if((Sector_Numebr == CBL_FLASH_MASS_ERASE))
			{
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("Performing Mass Erase \r\n");
#endif
				// Mass Erase
				FLASH_Erase_Cfg.TypeErase = FLASH_TYPEERASE_MASSERASE;
			}
			else
			{
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("Performing Sectors Erase \r\n");
#endif
				Remaining_Sectors = FLASH_MAX_SECTOR_NUMBERS - Sector_Numebr;
				if(Number_Of_Sectors > Remaining_Sectors)
				{
					Number_Of_Sectors = Remaining_Sectors;
				}
				else{/* Nothing */}
				
				// Sectors Erase
				FLASH_Erase_Cfg.TypeErase = FLASH_TYPEERASE_SECTORS;
				// Sector to start erase from
				FLASH_Erase_Cfg.Sector = Sector_Numebr;
				// Number of sectors to erase
				FLASH_Erase_Cfg.NbSectors = Number_Of_Sectors;
			}
			// One Bank
			FLASH_Erase_Cfg.Banks = FLASH_BANK_1;
			/* Device operating range: 2.7V to 3.6V */
			FLASH_Erase_Cfg.VoltageRange = FLASH_VOLTAGE_RANGE_3;
			
			// Unlock the flash memory
			HAL_Status = HAL_FLASH_Unlock();
			// Perform an erase from Flash memory
			HAL_Status = HAL_FLASHEx_Erase(&FLASH_Erase_Cfg, &SectorError);
			
			BL_Print_Message("0x%x \r\n", SectorError);
			
			if(HAL_SUCCESSFUL_ERASE == SectorError)
			{
				Sector_Validity = ERASE_SUCCEEDED;
			}
			else
			{
				Sector_Validity = ERASE_FAILED;
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
				BL_Print_Message("Problem with Sector: %x \r\n", SectorError);
#endif
			}
			// Lock the flash memory
			HAL_Status = HAL_FLASH_Lock();
		}
		else
		{
			// SECTOR Number is out of range
			Sector_Validity = INVALID_SECTOR_NUMBER;
#if BL_DEBUG_INFO_CONTROLL == DEBUG_INFO_ENABLE
			BL_Print_Message("Inavlid Start Sector \r\n");
#endif
		}
	}
	return Sector_Validity;
}

static uint8_t Flash_Memory_Write_Payload(uint8_t *Host_Payload, uint32_t Start_Addr, uint8_t Payload_Len)
{
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	uint8_t Payload_Counter = 0;
	uint8_t Write_Status = FLASH_MEMORY_WRITE_FAILED;
	// Unlock the flash memory
	HAL_Status = HAL_FLASH_Unlock();
	if(HAL_Status != HAL_OK)
	{
		Write_Status = FLASH_MEMORY_WRITE_FAILED;
	}
	else
	{
		for(Payload_Counter = 0; Payload_Counter < Payload_Len; Payload_Counter++)
		{
			HAL_Status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, (Start_Addr + Payload_Counter), Host_Payload[Payload_Counter]);
			if(HAL_Status != HAL_OK)
			{
				Write_Status = FLASH_MEMORY_WRITE_FAILED;
				break;
			}
			else
			{
				Write_Status = FLASH_MEMORY_WRITE_PASSED;
			}
		}
		// Lock the flash memory
		HAL_Status = HAL_FLASH_Lock();
		if(HAL_Status != HAL_OK)
		{
		Write_Status = FLASH_MEMORY_WRITE_FAILED;
		}
		else
		{
			Write_Status = FLASH_MEMORY_WRITE_PASSED;
		}
	}
	return Write_Status;
}