
# our executable (base) name
TARGET=test

# DONE with the config
CC      = g++
OBJCOPY = objcopy
OBJDUMP = objdump
GDB     = gdb

CFLAGS  = -g -O0 -Wall
CFLAGS += -std=c++0x

#LDFLAGS = -lstdc++

CSRC   = $(wildcard *.c)
CPPSRC = $(wildcard *.cpp)
SSRC   = $(wildcard *.S)
OBJ    = $(CSRC:.c=.o) $(CPPSRC:.cpp=.o) $(SSRC:.S=.o)

INCLUDE_FILES = $(wildcard *.h)

.PHONY: proj

all: proj

proj: 	$(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

%.o: %.S $(INCLUDE_FILES)
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c $(INCLUDE_FILES)
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp $(INCLUDE_FILES)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o
	rm -f $(TARGET)

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
elf-dump:
	$(OBJDUMP) -sSd $(TARGET).elf
	
