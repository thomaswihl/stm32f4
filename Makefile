# where can I find the STM library
STM_STD_PERIPH=$(HOME)/UbuntuOne/stm/lib/STM32F4-Discovery_FW_V1.1.0/Libraries

# where can I find the toolchain
# get it from https://launchpad.net/gcc-arm-embedded/+milestone/4.6-2012-q2-update
TOOLCHAIN=$(HOME)/stm/gcc-arm-none-eabi-4_6-2012q2/bin

# our executable (base) name
TARGET=example

# DONE with the config
CC      = $(TOOLCHAIN)/arm-none-eabi-gcc
OBJCOPY = $(TOOLCHAIN)/arm-none-eabi-objcopy
OBJDUMP = $(TOOLCHAIN)/arm-none-eabi-objdump
GDB     = $(TOOLCHAIN)/arm-none-eabi-gdb

CFLAGS  = -g -O0 -Wall -Tstm32f407vg.ld
CFLAGS += -mthumb -mcpu=cortex-m4
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -Wl,-Map,$(TARGET).map
CFLAGS += -nostartfiles
CFLAGS += -std=c++0x
CPPFLAGS = -fno-rtti -fno-exceptions

#LDFLAGS = -lstdc++

VERSION_FILE = version.cpp


SRC  = $(wildcard *.cpp)
SRC += $(wildcard hw/*.cpp)
# make sure we have the version file included (in case of a clean build it doesn't exist right now)
ifeq (,$(findstring $(VERSION_FILE),$(SRC)))
  SRC  += $(VERSION_FILE)
endif
OBJ    = $(SRC:.cpp=.o)

INCLUDE_FILES = $(wildcard *.h)

.PHONY: proj
	make elf-dump

all: proj

proj: $(VERSION_FILE) $(TARGET).elf

.PHONY: $(VERSION_FILE)
$(VERSION_FILE):
	echo > $@
	echo '#include "version.h"' >> $@
	echo >> $@
	echo -n 'const char* const BUILD_DATE = "' >> $@
	date | tr -d "\n" >> $@
	echo "\";" >> $@
	echo -n 'const char* const GIT_VERSION = "' >> $@
	git branch --abbrev=100 -v | tr -d "\n" >> $@
	echo "\";" >> $@
	echo >> $@

$(TARGET).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET).elf $(OBJ) $(LDFLAGS)
	$(OBJCOPY) -O binary $(TARGET).elf $(TARGET).bin

%.o: %.S $(INCLUDE_FILES)
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c $(INCLUDE_FILES)
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp $(INCLUDE_FILES)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o
	rm -f $(TARGET).map
	rm -f $(TARGET).elf
	rm -f $(TARGET).bin
	rm -f elf-dump
	rm -f $(VERSION_FILE)

.PHONY: flash
flash:
	st-flash write $(TARGET).bin 0x8000000

.PHONY: debug
debug:
	killall -q st-util || true
	sleep 0.5
	st-util >/dev/null 2>&1 &
	$(GDB) $(TARGET).elf
	
.PHONY: elf-dump
elf-dump: proj
	$(OBJDUMP) -d $(TARGET).elf > elf-dump
	
