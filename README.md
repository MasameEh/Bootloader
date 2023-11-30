# STM32F4xx Bootloader
This bootloader is designed for the STM32F401 microcontroller. It provides functionality to perform various operations such as fetching commands from the host, erasing and writing to Flash memory, reading protection levels, and more.

## Files

- `bootloader.h`: Header file containing definitions, macros, and function prototypes for the bootloader.
- `bootloader.c`: Source file containing function implementations.
## Included Libraries

- `stdarg.h`: For variable arguments handling in functions.
- `stdio.h`: For standard input/output operations.
- `string.h`: For string manipulation functions.
- `usart.h`: Header file related to USART communication.
- `crc.h`: Header file for CRC engine.

## Macros

Several macros are defined in `bootloader.h`:
- **Debug Settings**: Controls debug information output and debug method (UART/SPI/I2C).
- **Command Definitions**: Definitions for various commands supported by the bootloader.
- **Version Information**: Vendor ID and software version information.
- **Status and Verification Codes**: Error, verification, and status codes used by the bootloader.
- **Memory Addresses**: Definitions related to Flash and SRAM memory regions.

## Functions

1. **Bootloader_Get_Version**: Retrieves the version information.
2. **Bootloader_Get_Help**: Fetches help information about supported commands.
3. **Bootloader_Get_Chip_Identification_Number**: Retrieves the chip identification number.
4. **Bootloader_Read_Protection_Level**: Reads the protection level.
5. **Bootloader_Jump_To_Address**: Jumps to a specified memory address.
6. **Bootloader_Erase_Flash**: Erases Flash memory.
7. **Bootloader_Memory_Write**: Writes data to memory.
8. **Bootloader_Enable_RW_Protection**: Enables read/write protection.
9. **Bootloader_Memory_Read**: Reads data from memory.
10. **Bootloader_Get_Sector_Protection_Status**: Retrieves sector protection status.
11. **Bootloader_Read_OTP**: Reads data from OTP memory.
12. **Bootloader_Change_Read_Protection_Level**: Changes the read protection level.

*Note: The README provides an overview and structure of the bootloader. Additional documentation and comments within the code may contain more detailed information.*
