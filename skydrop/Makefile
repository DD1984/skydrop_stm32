######################################
TOPDIR := $(shell pwd)
######################################
# target
######################################
TARGET = SkyDrop

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
#######################################
# source path
# Build path
BUILD_DIR = build

######################################
# source
######################################
C_SOURCES :=
CPP_SOURCES :=

#hal
C_SOURCES += src/hal/system_stm32f1xx.c
C_SOURCES += src/hal/stm32f1xx_hal.c
C_SOURCES += src/hal/stm32f1xx_hal_cortex.c
C_SOURCES += src/hal/stm32f1xx_hal_rcc.c
C_SOURCES += src/hal/stm32f1xx_hal_rcc_ex.c
C_SOURCES += src/hal/stm32f1xx_hal_uart.c
C_SOURCES += src/hal/stm32f1xx_hal_msp.c
C_SOURCES += src/hal/stm32f1xx_hal_gpio.c
C_SOURCES += src/hal/stm32f1xx_hal_gpio_ex.c
C_SOURCES += src/hal/stm32f1xx_it.c
C_SOURCES += src/hal/syscalls.c


C_SOURCES += src/hal/clock.c
#c

#cpp
CPP_SOURCES += src/skydrop.cpp
CPP_SOURCES += src/common.cpp

CPP_SOURCES += src/fc/fc.cpp
CPP_SOURCES += src/fc/conf.cpp
CPP_SOURCES += src/fc/vario.cpp
CPP_SOURCES += src/fc/kalman.cpp

CPP_SOURCES += src/tasks/tasks.cpp
CPP_SOURCES += src/tasks/task_active.cpp
CPP_SOURCES += src/tasks/task_powerdown.cpp

CPP_SOURCES += src/drivers/sensors/devices.cpp
CPP_SOURCES += src/drivers/buttons.cpp
CPP_SOURCES += src/drivers/uart.cpp
CPP_SOURCES += src/drivers/n5110_disp.cpp

CPP_SOURCES += src/data/fonts.cpp

CPP_SOURCES += src/gui/gui.cpp
CPP_SOURCES += src/gui/pages.cpp
CPP_SOURCES += src/gui/gui_dialog.cpp
CPP_SOURCES += src/gui/gui_value.cpp
CPP_SOURCES += src/gui/splash.cpp
CPP_SOURCES += src/gui/factory_test.cpp
CPP_SOURCES += src/gui/gui_list.cpp

CPP_SOURCES += src/gui/widgets/widgets.cpp
CPP_SOURCES += src/gui/widgets/dummy.cpp
CPP_SOURCES += src/gui/widgets/flight.cpp
CPP_SOURCES += src/gui/widgets/temperature.cpp
#CPP_SOURCES += src/gui/widgets/battery.cpp
CPP_SOURCES += src/gui/widgets/vario.cpp
CPP_SOURCES += src/gui/widgets/altitude.cpp


CPP_SOURCES += src/gui/settings/set_widgets.cpp
CPP_SOURCES += src/gui/settings/settings.cpp
CPP_SOURCES += src/gui/settings/set_system.cpp
CPP_SOURCES += src/gui/settings/set_autostart.cpp
CPP_SOURCES += src/gui/settings/set_layout.cpp
CPP_SOURCES += src/gui/settings/set_audio.cpp
CPP_SOURCES += src/gui/settings/set_altimeter.cpp
CPP_SOURCES += src/gui/settings/set_weaklift.cpp
CPP_SOURCES += src/gui/settings/set_vario.cpp



#asm
ASM_SOURCES = src/hal/startup_stm32f100xb.s

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
C_DEFS = -DSTM32F100xB
# includes for gcc
AS_INCLUDES = 
C_INCLUDES = -I src/ -I src/hal
# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections
CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -fdata-sections -ffunction-sections
ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif
# Generate dependency information
CFLAGS += -MD -MP -MF .dep/$(@F).d

CFLAGS += -DSTM32
CFLAGS += -DUART_SUPPORT
CFLAGS += -DDISPLAY_SUPPORT

#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = src/hal/STM32F100XB_FLASH.ld
# libraries
LIBS = -lc -lm -lstdc++
LIBDIR =
# --specs=nano.specs
# --specs=nosys.specs
LDFLAGS = $(MCU) --specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

#######################################
# build the application
#######################################
# list of objects
#c
#OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS = $(C_SOURCES:.c=.o)
vpath %.c $(sort $(dir $(C_SOURCES)))
#cpp
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(CPP_SOURCES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CPP_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.cpp $(BUILD_DIR)
	zxfg
	$(CC) -c $(CFLAGS) -x c++ -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.cpp=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.c $(BUILD_DIR)
#	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@
	$(CC) -c $(CFLAGS) $< -o $@


$(BUILD_DIR)/%.o: %.s $(BUILD_DIR)
	sakjdhf
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	dsgfh
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir -p $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR .dep $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

# *** EOF ***
