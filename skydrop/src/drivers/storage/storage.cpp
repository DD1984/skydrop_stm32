#include "storage.h"
#include "usbd_storage.h"
FATFS FatFs;		/* FatFs work area needed for each volume */

#ifndef STM32
extern Usart sd_spi_usart;
#else
#include "spi_flash.h"
#include "../../dump.h"
char ff_work_buf[STORAGE_BLK_SIZ];
#endif

uint32_t storage_space = 0;
uint32_t storage_free_space = 0;

bool sd_avalible = false;
bool sd_error = false;

#define SD_CARD_DETECT	(GpioRead(SD_IN) == LOW)

bool storage_init()
{
#ifndef STM32
	uint8_t res;

	GpioSetPull(SD_IN, gpio_pull_up);
	GpioSetDirection(SD_IN, INPUT);

	DEBUG("SD_IN %d\n", GpioRead(SD_IN));

	if (!SD_CARD_DETECT)
		return false;

	//power spi & sdcard
	SD_EN_ON;
	SD_SPI_PWR_ON;

	DEBUG("Mounting SD card ... ");

	for (uint8_t i = 0; i < 5; i++)
	{

		res = f_mount(&FatFs, "", 1);		/* Give a work area to the default drive */
		DEBUG("%d ", i + 1);
		if (res == RES_OK)
			break;

		for (uint8_t j = 0; j < i +1; j++)
			_delay_ms(10);
	}

	if (res != RES_OK)
	{
		DEBUG("Error %02X\n", res);

		sd_spi_usart.Stop();

		GpioSetDirection(SD_SS, INPUT);
		GpioSetPull(SD_IN, gpio_totem);

		//power spi & sdcard
		SD_EN_OFF;
		SD_SPI_PWR_OFF;

		sd_error = true;
		sd_avalible = false;

		task_irqh(TASK_IRQ_MOUNT_ERROR, 0);

		return false;
	}

	DEBUG("OK\n");

	uint32_t size;

	FATFS * FatFs1;

	res = f_getfree("", &size, &FatFs1);

	uint32_t sector_count;

	res = disk_ioctl(0, GET_SECTOR_COUNT, &sector_count);

	uint16_t sector_size;

	res = disk_ioctl(0, GET_SECTOR_SIZE, &sector_size);

	storage_space = sector_count / 2;
	storage_free_space = size * 4 * 1024;

	DEBUG("Disk info\n");
	DEBUG(" sector size  %12u B\n", sector_size);
	DEBUG(" sector count %12lu\n", sector_count);
	DEBUG(" total space  %12lu kB\n", storage_space);
	DEBUG(" free space   %12lu\n", storage_free_space);
#else
	uint8_t ret;
	FATFS *FatFs_p;

	BSP_SERIAL_FLASH_Init();

	ret = f_mount(&FatFs, "", 1);

	if (ret != RES_OK) {
		//TOD: add gui dialog
		printf("Mounting err: %d - formating...", ret);
		ret = f_mkfs("", FM_EXFAT | FM_SFD, STORAGE_BLK_SIZ, ff_work_buf, sizeof(ff_work_buf));
		if (ret == RES_OK) {
			printf("OK\n");
			ret = f_mount(&FatFs, "", 1);
		}
		else {
			printf("FAIL err:%d\n", ret);
		}
	}

	printf("Mounting ");
	if (ret == RES_OK)
		printf("OK\n");
	else
		printf("FAIL\n");

#if 0
	FIL fp;

	//for (i = 0; i < strlen(file); i++)
	//	path[i] = file[i];
	//path[i] = 0;

	//hex_dump(path_p, 32);

	//memset(&fp, 0, sizeof(fp));

	ret = f_open(&fp, "/flash1.txt", FA_READ | FA_CREATE_ALWAYS | FA_WRITE);
	f_puts("flash work\n", &fp);

	f_close(&fp);

	//char buf[32];
	//f_gets(buf, 32, &fp);
	//hex_dump(buf, 32);

	printf("ret: %d\n", ret);
#endif
#endif
	sd_avalible = true;
	sd_error = false;

	return true;
}

void storage_deinit()
{
#ifndef STM32
	DEBUG("storage_deinit\n");

	if (!sd_avalible)
		return;

	assert(f_mount(NULL, "", 1) == FR_OK); //unmount

	sd_spi_usart.Stop();

	sd_avalible = false;

	GpioSetPull(SD_IN, gpio_totem);
	GpioSetDirection(SD_SS, INPUT);

	//power spi & sdcard
	SD_EN_OFF;
	SD_SPI_PWR_OFF;

	//let it cool down :)
	_delay_ms(100);
#endif
}

void storage_step()
{
#ifndef STM32
	if (SD_CARD_DETECT)
	{
		if (!sd_avalible && !sd_error)
			storage_init();
	}
	else
	{
		if (sd_avalible)
			storage_deinit();
		sd_error = false;
	}
#endif
}

bool storage_card_in()
{
	return sd_avalible;
}

bool storage_ready()
{
	return sd_avalible && !sd_error;
}
