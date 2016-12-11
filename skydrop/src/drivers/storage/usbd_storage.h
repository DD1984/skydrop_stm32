/**
  ******************************************************************************
  * @file    USB_Device/MSC_Standalone/Inc/usbd_storage.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    05-June-2015
  * @brief   Header for usbd_storage.c module
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
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_STORAGE_H_
#define __USBD_STORAGE_H_

#include "usbd_msc.h"
#include "spi_flash.h"

#define STORAGE_BLK_SIZ                  SPI_FLASH_SECTOR_SIZE

extern USBD_StorageTypeDef  USBD_DISK_fops;

int8_t _STORAGE_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t _STORAGE_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t _STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num, uint16_t *block_size);

#define STORAGE_Read(buff, sector, count) _STORAGE_Read(0, buff, sector, count)
#define STORAGE_Write(buff, sector, count) _STORAGE_Write(0, buff, sector, count)
#define STORAGE_GetCapacity(block_num, block_size) _STORAGE_GetCapacity(0, block_num, block_size);


#endif /* __USBD_STORAGE_H_ */
 
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
