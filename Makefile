# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Turn on increased build verbosity by using V=1 on the make command line.
ifeq ("$(origin V)", "command line")
BUILD_VERBOSE=$(V)
endif
ifndef BUILD_VERBOSE
BUILD_VERBOSE = 0
endif
ifeq ($(BUILD_VERBOSE),0)
Q = @
else
Q =
endif
ifeq ($(BUILD_VERBOSE),0)
$(info Use make V=1 or set BUILD_VERBOSE in your environment to increase build verbosity.)
endif

TARGET ?= sensorweb-nrf51

RM = rm
ECHO = @echo

comma := ,
empty :=
space := $(empty) $(empty)

# SDK 11.0.0 expects gcc 4.9.3
CROSS_COMPILE = arm-none-eabi-

AS = $(CROSS_COMPILE)as
CC = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
OBJCOPY = $(CROSS_COMPILE)objcopy
SIZE = $(CROSS_COMPILE)size

SDK = sdk-11.0.0
SDK_ZIP = nRF5_SDK_11.0.0_89a8197.zip
SDK_URL = https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v11.x.x/$(SDK_ZIP)

INC  = -Iconfig
INC += -I$(SDK)/components/drivers_nrf/config
INC += -I$(SDK)/components/libraries/timer
INC += -I$(SDK)/components/libraries/fstorage/config
INC += -I$(SDK)/components/softdevice/s130/headers
INC += -I$(SDK)/components/drivers_nrf/delay
INC += -I$(SDK)/components/libraries/util
INC += -I$(SDK)/components/ble/device_manager
INC += -I$(SDK)/components/drivers_nrf/uart
INC += -I$(SDK)/components/ble/common
INC += -I$(SDK)/components/libraries/sensorsim
INC += -I$(SDK)/components/drivers_nrf/pstorage
INC += -I$(SDK)/components/libraries/uart
INC += -I$(SDK)/components/libraries/fifo
INC += -I$(SDK)/components/device
INC += -I$(SDK)/components/libraries/button
INC += -I$(SDK)/components/libraries/fstorage
INC += -I$(SDK)/components/libraries/experimental_section_vars
INC += -I$(SDK)/components/drivers_nrf/gpiote
INC += -I$(SDK)/external/segger_rtt
INC += -I$(SDK)/examples/bsp
INC += -I$(SDK)/components/toolchain/CMSIS/Include
INC += -I$(SDK)/components/drivers_nrf/hal
INC += -I$(SDK)/components/toolchain/gcc
INC += -I$(SDK)/components/toolchain
INC += -I$(SDK)/components/drivers_nrf/common
INC += -I$(SDK)/components/ble/ble_advertising
INC += -I$(SDK)/components/softdevice/s130/headers/nrf51
INC += -I$(SDK)/components/libraries/trace
INC += -I$(SDK)/components/softdevice/common/softdevice_handler
INC += -Icrypto

#CPPFLAGS  = -DNRF_LOG_USES_RAW_UART=1
CPPFLAGS  = -DNRF_LOG_USES_UART=1
CPPFLAGS += -DBOARD_CUSTOM
#CPPFLAGS += -DBOARD_PCA10028
CPPFLAGS += -DSOFTDEVICE_PRESENT
CPPFLAGS += -DNRF51
CPPFLAGS += -DS130
CPPFLAGS += -DBLE_STACK_SUPPORT_REQD
CPPFLAGS += -DSWI_DISABLE0

CFLAGS  = $(CPPFLAGS)
CFLAGS += -mcpu=cortex-m0
CFLAGS += -mthumb -mabi=aapcs

# Note: -O3 causes the flash image size to increase by 80K
CFLAGS += -Wall -Werror -Os -g3

CFLAGS += -mfloat-abi=soft
# keep every function in separate section. This will allow linker to dump unused functions
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin --short-enums 

CXXFLAGS := $(CFLAGS)

# Using -fnoexceptions drops 6K off the flash image
CXXFLAGS += -fno-exceptions
# Using -fno-rtti drops another 6K off the flash image
CXXFLAGS += -fno-rtti

CFLAGS += --std=gnu99

# keep every function in separate section. This will allow linker to dump unused functions
LDFLAGS += -Xlinker -Map=$(OBJDIR)/$(TARGET).map -Wl,--cref
LDFLAGS += -mthumb -mabi=aapcs -L $(SDK)/components/toolchain/gcc -Tnrf51.ld
LDFLAGS += -mcpu=cortex-m0
# let linker to dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
#LDFLAGS += --specs=nano.specs -lc -lnosys
LDFLAGS += -lc -lnosys

AFLAGS  = $(CPPFLAGS)
AFLAGS += -x assembler-with-cpp
AFLAGS += -D__STACK_SIZE=4096

OBJDIR ?= obj

C_SRC = \
	main.c \
	our_service.c \
	system_nrf51.c \
	$(SDK)/components/libraries/button/app_button.c \
	$(SDK)/components/libraries/util/app_error.c \
	$(SDK)/components/libraries/util/app_error_weak.c \
	$(SDK)/components/libraries/timer/app_timer.c \
	$(SDK)/components/libraries/trace/app_trace.c \
	$(SDK)/components/libraries/util/app_util_platform.c \
	$(SDK)/components/libraries/fstorage/fstorage.c \
	$(SDK)/components/libraries/util/nrf_assert.c \
	$(SDK)/components/libraries/util/nrf_log.c \
	$(SDK)/components/libraries/uart/retarget.c \
	$(SDK)/components/libraries/sensorsim/sensorsim.c \
	$(SDK)/external/segger_rtt/RTT_Syscalls_GCC.c \
	$(SDK)/external/segger_rtt/SEGGER_RTT.c \
	$(SDK)/external/segger_rtt/SEGGER_RTT_printf.c \
	$(SDK)/components/libraries/fifo/app_fifo.c \
	$(SDK)/components/libraries/uart/app_uart_fifo.c \
	$(SDK)/components/drivers_nrf/delay/nrf_delay.c \
	$(SDK)/components/drivers_nrf/common/nrf_drv_common.c \
	$(SDK)/components/drivers_nrf/gpiote/nrf_drv_gpiote.c \
	$(SDK)/components/drivers_nrf/uart/nrf_drv_uart.c \
	$(SDK)/components/drivers_nrf/pstorage/pstorage.c \
	$(SDK)/examples/bsp/bsp.c \
	$(SDK)/components/ble/common/ble_advdata.c \
	$(SDK)/components/ble/ble_advertising/ble_advertising.c \
	$(SDK)/components/ble/common/ble_conn_params.c \
	$(SDK)/components/ble/common/ble_srv_common.c \
	$(SDK)/components/ble/device_manager/device_manager_peripheral.c \
	$(SDK)/components/softdevice/common/softdevice_handler/softdevice_handler.c \
	$(SDK)/examples/bsp/bsp_btn_ble.c \

CPP_SRC = \
	c_crypto.cpp \
	crypto/AES128.o \
	crypto/AESCommon.o \
	crypto/BlockCipher.o \
	crypto/Curve25519.o \
	crypto/Ed25519.o \
	crypto/BigNumberUtil.o \
	crypto/RNG.o \
	crypto/ChaCha.o \
	crypto/Cipher.o \
	crypto/Crypto.o \
	crypto/SHA512.o \
	crypto/Hash.o \
	crypto/Arduino.o \

A_SRC = $(SDK)/components/toolchain/gcc/gcc_startup_nrf51.s

C_PATHS = $(sort $(dir $(C_SRC)))
CPP_PATHS = $(sort $(dir $(CPP_SRC)))
A_PATHS = $(sort $(dir $(A_SRC)))

vpath %.cpp $(CPP_PATHS)
vpath %.c $(C_PATHS)
vpath %.s $(A_PATHS)

CPP_FILES = $(notdir $(CPP_SRC))
C_FILES = $(notdir $(C_SRC))
A_FILES = $(notdir $(A_SRC))

OBJ  = $(addprefix $(OBJDIR)/, $(CPP_FILES:.cpp=.o))
OBJ += $(addprefix $(OBJDIR)/, $(C_FILES:.c=.o))
OBJ += $(addprefix $(OBJDIR)/, $(A_FILES:.s=.o))

.PHONY: all
all: $(TARGET)

.PHONY: $(TARGET)
$(TARGET): $(OBJDIR)/$(TARGET).elf

OBJDIRS = $(sort $(dir $(OBJ)))
$(OBJ): | $(OBJDIRS) $(SDK)
$(OBJDIRS):
	mkdir -p $@

$(SDK_ZIP):
	wget $(SDK_URL)

.PHONY: sdk
sdk: $(SDK)
$(SDK): $(SDK_ZIP)
	mkdir -p $(SDK)-unzipped
	unzip -d $(SDK) $(SDK_ZIP)
	for p in sdk-11.0.0-patches/*.patch; do patch -p0 < $p; done
	mv $(SDK)-unzipped $(SDK)

define compile_c
$(ECHO) "CC $<"
$(Q)$(CC) $(CFLAGS) $(INC) -c -MD -o $@ $<
@# The following fixes the dependency file.
@# See http://make.paulandlesley.org/autodep.html for details.
@cp $(@:.o=.d) $(@:.o=.P); \
  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
      -e '/^$$/ d' -e 's/$$/ :/' < $(@:.o=.d) >> $(@:.o=.P); \
  rm -f $(@:.o=.d)
endef

define compile_cpp
$(ECHO) "C++ $<"
$(Q)$(CXX) $(CXXFLAGS) $(INC) -c -MD -o $@ $<
@# The following fixes the dependency file.
@# See http://make.paulandlesley.org/autodep.html for details.
@cp $(@:.o=.d) $(@:.o=.P); \
  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
      -e '/^$$/ d' -e 's/$$/ :/' < $(@:.o=.d) >> $(@:.o=.P); \
  rm -f $(@:.o=.d)
endef

$(OBJDIR)/%.o: %.c
	$(call compile_c)

$(OBJDIR)/%.o: %.cpp
	$(call compile_cpp)

$(OBJDIR)/%.o: %.s
	$(ECHO) "AS $<"
	$(Q)$(CC) $(AFLAGS) $(INC) -c -o $@ $<

$(OBJDIR)/$(TARGET).elf: $(OBJ)
	$(ECHO) "Linking $@"
	$(Q)$(CXX) $(LDFLAGS) $^ $(LIBS) -lm -o $@
	$(Q)$(SIZE) $@

%.bin: %.elf
	$(ECHO) "Creating $@"
	$(Q)$(OBJCOPY) -O binary $< $@

%.hex: %.elf
	$(ECHO) "Creating $@"
	$(Q)$(OBJCOPY) -O ihex $< $@

.PHONY: flash
flash: $(OBJDIR)/$(TARGET).hex
	$(ECHO) "Flashing $<"
	nrfjprog --program $< -f nrf51  --sectorerase
	nrfjprog --reset -f nrf51

.PHONY: flash_softdevice
flash_softdevice: $(SDK)/components/softdevice/s130/hex/s130_nrf51_2.0.0_softdevice.hex
	$(ECHO) "Flashing $<"
	nrfjprog --program $< -f nrf51 --chiperase
	nrfjprog --reset -f nrf51

.PHONY: clean
clean:
	$(Q)$(RM) -rf $(OBJDIR)

-include $(OBJ:.o=.P)
