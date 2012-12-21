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

CSRC   = $(wildcard *.c)
ifeq (,$(findstring version.c,$(CSRC)))
CSRC  += version.c
endif

CPPSRC = $(wildcard *.cpp)
SSRC   = $(wildcard *.S)
OBJ    = $(CSRC:.c=.o) $(CPPSRC:.cpp=.o) $(SSRC:.S=.o)

INCLUDE_FILES = $(wildcard *.h)

.PHONY: proj
	make elf-dump

all: proj

proj: version.c $(TARGET).elf

.PHONY: version.c
version.c:
	echo > version.c
	echo "#ifndef VERSION_H" >> version.c
	echo "#define VERSION_H" >> version.c
	echo >> version.c
	echo -n 'const char* const BUILD_DATE = "' >> version.c
	date | tr -d "\n" >> version.c
	echo "\";" >> version.c
	echo -n 'const char* const GIT_VERSION = "' >> version.c
	git branch --abbrev=100 -v | tr -d "\n" >> version.c
	echo "\";" >> version.c
	echo >> version.c
	echo "#endif" >> version.c

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
	
