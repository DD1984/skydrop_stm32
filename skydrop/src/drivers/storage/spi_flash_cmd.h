#ifndef __SPI_FLASH_CMD_H__
#define __SPI_FLASH_CMD_H__

/* @brief  MX25L FLASH SPI supported commands */

#define FLASH_SPI_CMD_WRITE          0x02  /*!< Write to Memory instruction */
#define FLASH_SPI_CMD_WRSR           0x01  /*!< Write Status Register instruction */
#define FLASH_SPI_CMD_WREN           0x06  /*!< Write enable instruction */
#define FLASH_SPI_CMD_READ           0x03  /*!< Read from Memory instruction */
#define FLASH_SPI_CMD_RDSR           0x05  /*!< Read Status Register instruction  */
#define FLASH_SPI_CMD_RDID           0x9F  /*!< Read identification */
#define FLASH_SPI_CMD_SE             0x20  /*!< Sector Erase instruction */
#define FLASH_SPI_CMD_BE             0xD8  /*!< Block Erase instruction */
#define FLASH_SPI_CMD_CE             0xC7  /*!< Chip Erase instruction */
#define FLASH_SPI_CMD_DP             0xB9  /*!< Deep power down */
#define FLASH_SPI_CMD_RDP            0xAB  /*!< Release from deep power down */

#define FLASH_SPI_WIP_FLAG           0x01  /*!< Write In Progress (WIP) flag */

#define FLASH_SPI_DUMMY_BYTE         0xA5
#define FLASH_SPI_PAGESIZE           0x100

#define FLASH_SPI_M25P128_ID         0x202018
#define FLASH_SPI_M25P64_ID          0x202017

#endif
