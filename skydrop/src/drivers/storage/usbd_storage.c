/**
  ******************************************************************************
  * @file    USB_Device/MSC_Standalone/Src/usbd_storage.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    05-June-2015
  * @brief   Memory management layer
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbd_storage.h"
#include "spi_flash.h"
//#include "stm3210e_eval_sd.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define STORAGE_LUN_NBR                  1  
#define STORAGE_BLK_SIZ                  0x200

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* USB Mass storage Standard Inquiry Data */
int8_t STORAGE_Inquirydata[] = { /* 36 */
  /* LUN 0 */
  0x00,		
  0x80,		
  0x02,		
  0x02,
  (STANDARD_INQUIRY_DATA_LEN - 5),
  0x00,
  0x00,	
  0x00,
  'S', 'T', 'M', ' ', ' ', ' ', ' ', ' ', /* Manufacturer: 8 bytes  */
  'P', 'r', 'o', 'd', 'u', 'c', 't', ' ', /* Product     : 16 Bytes */
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  '0', '.', '0','1',                      /* Version     : 4 Bytes  */
}; 

/* Private function prototypes -----------------------------------------------*/
int8_t STORAGE_Init(uint8_t lun);
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num, uint16_t *block_size);
int8_t STORAGE_IsReady(uint8_t lun);
int8_t STORAGE_IsWriteProtected(uint8_t lun);
int8_t STORAGE_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t STORAGE_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t STORAGE_GetMaxLun(void);

USBD_StorageTypeDef USBD_DISK_fops = {
  STORAGE_Init,
  STORAGE_GetCapacity,
  STORAGE_IsReady,
  STORAGE_IsWriteProtected,
  STORAGE_Read,
  STORAGE_Write,
  STORAGE_GetMaxLun,
  STORAGE_Inquirydata, 
};
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initailizes the storage unit (medium)       
  * @param  lun: Logical unit number
  * @retval Status (0 : Ok / -1 : Error)
  */
int8_t STORAGE_Init(uint8_t lun)
{
#if 0
  BSP_SD_Init();
#endif

  BSP_SERIAL_FLASH_Init();

  /* Get SPI Flash ID */
  uint32_t flash_id = BSP_SERIAL_FLASH_ReadID();

  printf("flash_id: 0x08%x\n", flash_id);
  return 0;
}

/**
  * @brief  Returns the medium capacity.      
  * @param  lun: Logical unit number
  * @param  block_num: Number of total block number
  * @param  block_size: Block size
  * @retval Status (0: Ok / -1: Error)
  */
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
  //HAL_SD_CardInfoTypedef info;
  int8_t ret = 0;
  
#if 0
  if(BSP_SD_IsDetected() != SD_NOT_PRESENT)
  {
    BSP_SD_GetCardInfo(&info);
    
    *block_num = (info.CardCapacity)/STORAGE_BLK_SIZ  - 1;
    *block_size = STORAGE_BLK_SIZ;
    ret = 0;
  }
#endif

  *block_num = (8 * 1024 *1024 / 8) / STORAGE_BLK_SIZ  - 1;
  *block_size = STORAGE_BLK_SIZ;

  return ret;
}

/**
  * @brief  Checks whether the medium is ready.  
  * @param  lun: Logical unit number
  * @retval Status (0: Ok / -1: Error)
  */
int8_t STORAGE_IsReady(uint8_t lun)
{
  static int8_t prev_status = 0;
  int8_t ret = 0;

#if 0
  if(BSP_SD_IsDetected() != SD_NOT_PRESENT)
  {
    if(prev_status < 0)
    {
      BSP_SD_Init();
      prev_status = 0;
      
    }
    if(BSP_SD_GetStatus() == SD_TRANSFER_OK)
    {
      ret = 0;
    }
  }
  else if(prev_status == 0)
  {
    prev_status = -1;
  }
#endif
  return ret;
}

/**
  * @brief  Checks whether the medium is write protected.
  * @param  lun: Logical unit number
  * @retval Status (0: write enabled / -1: otherwise)
  */
int8_t STORAGE_IsWriteProtected(uint8_t lun)
{
  return 0;
}

void dump_line(void *addr, int len, int line_len)
{
	if (len <= 0)
		return;
	printf(" %04x  ", addr);
	char *ptr = (char *)addr;
	while (ptr - (char *)addr < len) {
		printf("%02x ", (unsigned char)*ptr);
		ptr++;
	}

	int i;
	for (i = 0; i < line_len - len; i++)
		printf("   ");

	ptr = (char *)addr;
	while (ptr - (char *)addr < len) {
		printf("%c", ((unsigned char)*ptr < 0x20 || (unsigned char)*ptr > 0x7e) ? '.' : (unsigned char)*ptr);
		ptr++;
	}

	printf("\n");
}

void _hex_dump(void *addr, int len, int line_len)
{
	char *ptr = (char *)addr;
	while (ptr - (char *)addr < len) {
		dump_line(ptr, ((char *)addr + len - ptr > line_len) ? line_len : (char *)addr + len - ptr, line_len);
		ptr += line_len;
	}
}

/**
  * @brief  Reads data from the medium.
  * @param  lun: Logical unit number
  * @param  blk_addr: Logical block address
  * @param  blk_len: Blocks number
  * @retval Status (0: Ok / -1: Error)
  */
int8_t STORAGE_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
  int8_t ret = 0;

#if 0
  if(BSP_SD_IsDetected() != SD_NOT_PRESENT)
  {  
    BSP_SD_ReadBlocks_DMA((uint32_t *)buf, blk_addr * STORAGE_BLK_SIZ, STORAGE_BLK_SIZ, blk_len);
    ret = 0;
  }
#endif
  //printf("%s[%d] buf_ptr: 0x%x\n", __func__, __LINE__, buf);
  uint8_t read_stat = BSP_SERIAL_FLASH_ReadData(blk_addr * STORAGE_BLK_SIZ * 8, buf, STORAGE_BLK_SIZ);
  //printf("%s[%d] read: addr: 0x%x blk_addr: 0x%x rs: %d buf_ptr:0x%x\n", __func__, __LINE__, blk_addr * STORAGE_BLK_SIZ * 8, blk_addr, read_stat, buf);
  //_hex_dump(buf, 32, 16);
  return ret;
}

/**
  * @brief  Writes data into the medium.
  * @param  lun: Logical unit number
  * @param  blk_addr: Logical block address
  * @param  blk_len: Blocks number
  * @retval Status (0 : Ok / -1 : Error)
  */
int8_t STORAGE_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
  int8_t ret = 0;

#if 0
  if(BSP_SD_IsDetected() != SD_NOT_PRESENT)
  { 
    BSP_SD_WriteBlocks_DMA((uint32_t *)buf, blk_addr * STORAGE_BLK_SIZ, STORAGE_BLK_SIZ, blk_len);
    ret = 0;
  }
#endif
  uint8_t erase_stat = BSP_SERIAL_FLASH_EraseSector(blk_addr * STORAGE_BLK_SIZ * 8);

  uint8_t write_stat = BSP_SERIAL_FLASH_WriteData(blk_addr * STORAGE_BLK_SIZ * 8 , buf, STORAGE_BLK_SIZ);

  //printf("%s[%d] write: addr: 0x%x blk_addr: 0x%x es: %d, ws: %d\n", __func__, __LINE__, blk_addr * STORAGE_BLK_SIZ * 8, blk_addr, erase_stat, write_stat);
  //_hex_dump(buf, 32, 16);
  return ret;
}

/**
  * @brief  Returns the Max Supported LUNs.   
  * @param  None
  * @retval Lun(s) number
  */
int8_t STORAGE_GetMaxLun(void)
{
  return(STORAGE_LUN_NBR - 1);
}
 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

