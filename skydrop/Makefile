######################################
TOPDIR := $(shell pwd)
######################################
# target
######################################
TARGET = SkyDrop

######################################
# Posibilities
######################################
GPS_SUPPORT=n


######################################
# MCU
######################################
MCU = -mthumb -mcpu=cortex-m3

######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Os

#######################################
# pathes
#############################/src##########
# source path

######################################
# source
######################################
C_SOURCES :=
CPP_SOURCES :=

#hal
C_SOURCES += src/hal/Src/system_stm32f1xx.c
C_SOURCES += src/hal/Src/stm32f1xx_hal.c
C_SOURCES += src/hal/Src/stm32f1xx_hal_cortex.c
C_SOURCES += src/hal/Src/stm32f1xx_hal_rcc.c
C_SOURCES += src/hal/Src/stm32f1xx_hal_rcc_ex.c
C_SOURCES += src/hal/Src/stm32f1xx_hal_uart.c
C_SOURCES += src/hal/Src/stm32f1xx_hal_gpio.c
C_SOURCES += src/hal/Src/stm32f1xx_hal_gpio_ex.c
C_SOURCES += src/hal/Src/stm32f1xx_hal_spi.c
C_SOURCES += src/hal/Src/stm32f1xx_hal_spi_ex.c
C_SOURCES += src/hal/Src/stm32f1xx_hal_tim.c
C_SOURCES += src/hal/Src/stm32f1xx_hal_tim_ex.c
C_SOURCES += src/hal/Src/stm32f1xx_hal_i2c.c
C_SOURCES += src/hal/Src/stm32f1xx_hal_flash.c
C_SOURCES += src/hal/Src/stm32f1xx_hal_flash_ex.c
C_SOURCES += src/hal/Src/stm32f1xx_hal_rtc.c
C_SOURCES += src/hal/Src/stm32f1xx_hal_rtc_ex.c
C_SOURCES += src/hal/Src/stm32f1xx_hal_pwr.c
C_SOURCES += src/hal/Src/stm32f1xx_hal_adc.c
C_SOURCES += src/hal/Src/stm32f1xx_hal_adc_ex.c
C_SOURCES += src/hal/Src/stm32f1xx_hal_dma.c

C_SOURCES += src/hal/Src/stm32f1xx_ll_usb.c
C_SOURCES += src/hal/Src/stm32f1xx_hal_pcd.c
C_SOURCES += src/hal/Src/stm32f1xx_hal_pcd_ex.c

C_SOURCES += src/hal/stm32f1xx_hal_msp.c
C_SOURCES += src/hal/stm32f1xx_it.c
C_SOURCES += src/hal/syscalls.c

C_SOURCES += src/hal/clock.c
C_SOURCES += src/drivers/storage/spi_flash.c
C_SOURCES += src/drivers/storage/spi_flash_io.c
C_SOURCES += src/drivers/storage/spi_ctrl.c
C_SOURCES += src/drivers/storage/usbd_storage.c

C_SOURCES += src/tasks/task_usb/STM32_USB_Device_Library/Core/Src/usbd_core.c
C_SOURCES += src/tasks/task_usb/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc.c
C_SOURCES += src/tasks/task_usb/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc_bot.c
C_SOURCES += src/tasks/task_usb/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c
C_SOURCES += src/tasks/task_usb/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c
C_SOURCES += src/tasks/task_usb/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc_scsi.c
C_SOURCES += src/tasks/task_usb/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc_data.c

C_SOURCES += src/tasks/task_usb/usbd_conf.c
C_SOURCES += src/tasks/task_usb/usbd_desc.c

#cpp
CPP_SOURCES += src/hal/timers_irq_handlers.cpp
CPP_SOURCES += src/hal/eeprom.cpp

CPP_SOURCES += src/skydrop.cpp
CPP_SOURCES += src/common.cpp
CPP_SOURCES += src/debug.cpp

CPP_SOURCES += src/fc/fc.cpp
CPP_SOURCES += src/fc/conf.cpp
CPP_SOURCES += src/fc/vario.cpp
CPP_SOURCES += src/fc/kalman.cpp
CPP_SOURCES += src/fc/logger/logger.cpp

CPP_SOURCES += src/tasks/tasks.cpp
CPP_SOURCES += src/tasks/task_active.cpp
CPP_SOURCES += src/tasks/task_powerdown.cpp

CPP_SOURCES += src/tasks/task_usb/task_usb.cpp

CPP_SOURCES += src/drivers/uart.cpp

CPP_SOURCES += src/drivers/sensors/devices.cpp
CPP_SOURCES += src/drivers/sensors/ms5611.cpp
CPP_SOURCES += src/drivers/buttons.cpp
CPP_SOURCES += src/drivers/lcd_disp.cpp
#CPP_SOURCES += src/drivers/ILI9163C.cpp
CPP_SOURCES += src/drivers/battery.cpp
CPP_SOURCES += src/drivers/audio/audio.cpp
CPP_SOURCES += src/drivers/audio/buzzer.cpp
CPP_SOURCES += src/drivers/audio/sequencer.cpp
CPP_SOURCES += src/drivers/audio/vario.cpp
CPP_SOURCES += src/drivers/time.cpp
CPP_SOURCES += src/drivers/storage/storage.cpp

CPP_SOURCES += src/data/fonts.cpp

CPP_SOURCES += src/gui/gui.cpp
CPP_SOURCES += src/gui/pages.cpp
CPP_SOURCES += src/gui/gui_dialog.cpp
CPP_SOURCES += src/gui/gui_value.cpp
CPP_SOURCES += src/gui/splash.cpp
CPP_SOURCES += src/gui/factory_test.cpp
CPP_SOURCES += src/gui/gui_list.cpp
CPP_SOURCES += src/gui/usb.cpp

CPP_SOURCES += src/gui/widgets/layouts/layouts.cpp
CPP_SOURCES += src/gui/widgets/widgets.cpp
CPP_SOURCES += src/gui/widgets/dummy.cpp
CPP_SOURCES += src/gui/widgets/temperature.cpp
CPP_SOURCES += src/gui/widgets/battery.cpp
CPP_SOURCES += src/gui/widgets/vario.cpp
CPP_SOURCES += src/gui/widgets/altitude.cpp
CPP_SOURCES += src/gui/widgets/time.cpp
CPP_SOURCES += src/gui/widgets/ctrl.cpp

CPP_SOURCES += src/gui/settings/set_widgets.cpp
CPP_SOURCES += src/gui/settings/settings.cpp
CPP_SOURCES += src/gui/settings/set_system.cpp
CPP_SOURCES += src/gui/settings/set_autostart.cpp
CPP_SOURCES += src/gui/settings/layouts.cpp
CPP_SOURCES += src/gui/settings/set_layout.cpp
CPP_SOURCES += src/gui/settings/set_audio.cpp
CPP_SOURCES += src/gui/settings/set_menu_audio.cpp
CPP_SOURCES += src/gui/settings/set_altimeter.cpp
CPP_SOURCES += src/gui/settings/set_weaklift.cpp
CPP_SOURCES += src/gui/settings/set_vario.cpp
CPP_SOURCES += src/gui/settings/set_display.cpp
CPP_SOURCES += src/gui/settings/set_debug.cpp
CPP_SOURCES += src/gui/settings/set_altimeters.cpp
CPP_SOURCES += src/gui/settings/set_logger.cpp
CPP_SOURCES += src/gui/settings/set_time.cpp


#asm
#ASM_SOURCES = src/hal/startup_stm32f100xb.s
ASM_SOURCES = src/hal/startup_stm32f103xe.s

#######################################
# binaries
#######################################
CC = arm-none-eabi-gcc
AS = arm-none-eabi-gcc -x assembler-with-cpp
CP = arm-none-eabi-objcopy
AR = arm-none-eabi-ar
SZ = arm-none-eabi-size
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# macros for gcc
AS_DEFS = 
#C_DEFS = -DSTM32F100xB
C_DEFS = -DSTM32F103xE
# includes for gcc
AS_INCLUDES = 
C_INCLUDES = -I src/ -I src/hal/Inc -I src/hal

#for usb
C_INCLUDES += -I src/tasks/task_usb/STM32_USB_Device_Library/Core/Inc -I src/tasks/task_usb/ -I src/tasks/task_usb/STM32_USB_Device_Library/Class/MSC/Inc/
 
# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections
CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -fdata-sections -ffunction-sections
ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif
# Generate dependency information
CFLAGS += -MD -MP -MF .dep/$(@F).d

CFLAGS += -DSTM32
CFLAGS += -DMS5611_SUPPORT
CFLAGS += -DSTORAGE_SUPPORT
CFLAGS += -DUSB_SUPPORT
#CFLAGS += -DILI9163C

#######################################
# LDFLAGS
#######################################
# link script
#LDSCRIPT = src/hal/STM32F100XB_FLASH.ld
LDSCRIPT = src/hal/STM32F103XE_FLASH.ld
# libraries
LIBS = -lc -lm -lstdc++
LIBDIR =
LDFLAGS = $(MCU) -u _printf_float --specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(TARGET).elf $(TARGET).hex $(TARGET).bin

#######################################
# build the application
#######################################
# list of objects
#c
OBJECTS = $(C_SOURCES:.c=.o)
vpath %.c $(sort $(dir $(C_SOURCES)))
#cpp
OBJECTS += $(CPP_SOURCES:.cpp=.o)
vpath %.cpp $(sort $(dir $(CPP_SOURCES)))
# list of ASM program objects
OBJECTS += $(ASM_SOURCES:.s=.o)
vpath %.s $(sort $(dir $(ASM_SOURCES)))

%.o: %.cpp
	$(CC) -c $(CFLAGS) -x c++ $< -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) -c $(CFLAGS) $< -o $@

$(TARGET).elf: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

%.hex: %.elf
	$(HEX) $< $@
	
%.bin: %.elf
	$(BIN) $< $@	
	
#######################################
# clean up
#######################################
clean:
	-rm -fR .dep $(OBJECTS) $(TARGET).elf $(TARGET).bin $(TARGET).map $(TARGET).hex
  
#######################################
# dependencies
#######################################
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

# *** EOF ***
