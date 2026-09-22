TARGET = msp432_app

TOOLCHAIN = C:/ti/arm-gnu-toolchain-15.3.rel1-mingw-w64-x86_64-arm-none-eabi/bin
SDK = C:/ti/simplelink_msp432p4_sdk_3_40_01_02

CC = $(TOOLCHAIN)/arm-none-eabi-gcc.exe
CXX = $(TOOLCHAIN)/arm-none-eabi-g++.exe
SIZE = $(TOOLCHAIN)/arm-none-eabi-size.exe

CPU_FLAGS = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

INCLUDES = \
-Iinclude \
-Icommon \
-I$(SDK)/source \
-I$(SDK)/source/ti/devices/msp432p4xx/inc \
-I$(SDK)/source/third_party/CMSIS/Include

CFLAGS = $(CPU_FLAGS) \
-D__MSP432P401R__ \
-Wall \
-Wextra \
-g \
-O0 \
-MMD \
-MP \
$(INCLUDES)

CXXFLAGS = $(CFLAGS) -std=c++17

LDFLAGS = $(CPU_FLAGS) \
-Tlinker/msp432p401r.lds \
-Wl,-Map=build/$(TARGET).map \
-Wl,--gc-sections \
-specs=nosys.specs \
-specs=nano.specs

C_SOURCES = \
startup/startup_msp432p401r_gcc.c \
startup/system_msp432p401r.c

CPP_SOURCES = src/main.cpp

C_OBJECTS = $(patsubst %.c,build/%.o,$(C_SOURCES))
CPP_OBJECTS = $(patsubst %.cpp,build/%.o,$(CPP_SOURCES))

OBJECTS = $(C_OBJECTS) $(CPP_OBJECTS)
DEPS = $(OBJECTS:.o=.d)

all: build/$(TARGET).elf

build/$(TARGET).elf: $(OBJECTS) linker/msp432p401r.lds Makefile
	@mkdir -p $(dir $@)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@

build/%.o: %.c Makefile
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: %.cpp Makefile
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf build

-include $(DEPS)

.PHONY: all clean