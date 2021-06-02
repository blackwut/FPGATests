# You must configure INTELFPGAOCLSDKROOT to point the root directory of the
# Intel(R) FPGA SDK for OpenCL(TM) software installation.
# See http://www.altera.com/literature/hb/opencl-sdk/aocl_getting_started.pdf

# Where is the Intel(R) FPGA SDK for OpenCL(TM) software?
ifeq ($(wildcard $(INTELFPGAOCLSDKROOT)),)
$(error Set INTELFPGAOCLSDKROOT to the directory of the Intel(R) FPGA SDK for OpenCL)
endif
ifeq ($(wildcard $(INTELFPGAOCLSDKROOT)/host/include/CL/opencl.h),)
$(error Set INTELFPGAOCLSDKROOT to the directory of the Intel(R) FPGA SDK for OpenCL)
endif

# ------------------------------------------------------------------------------
# Host
# ------------------------------------------------------------------------------
CXX := arm-linux-gnueabihf-g++
CXXFLAGS = --std=c++11 -pedantic -Wall -Wextra

# Host Files
INCS := $(wildcard includes/*.hpp)
SRCS := $(wildcard host/*.cpp)
LIBS := rt pthread

# Host Directories
INC_DIRS := common includes
LIB_DIRS :=

# Target
TARGET_HOST := host
TARGET_HOST_DIR := bin


# ------------------------------------------------------------------------------
# Device
# ------------------------------------------------------------------------------
AOC := aoc
AOC_FLAGS := -v -report -incremental
AOC_BOARD := -board=a10s_ddr
# OpenCL compile and link flags.
AOCL_COMPILE_CONFIG := $(shell aocl compile-config)
AOCL_LINK_CONFIG := -L/opt/intelFPGA_pro/19.1/hld/board/terasic/a10s/arm32/lib \
					-L/opt/intelFPGA_pro/19.1/hld/host/arm32/lib \
					-L/opt/intelFPGA_pro/19.1/hld/host/linux64/lib \
					-Wl,--no-as-needed -lalteracl -lintel_a10soc32_mmd \
					-lstdc++ -lelf -lacl_emulator_kernel_rt

# Device Files
DEVICE_CODE := device
DEVICE_CODE_DIR := device

DEVICE_INCLUDES := $(DEVICE_CODE)/nodes includes

# Target
TARGET_DEVICE := device.aocx
TARGET_DEVICE_DIR := bin


# ------------------------------------------------------------------------------
# Emulator
# ------------------------------------------------------------------------------
ifeq ($(EMULATOR),1)
# Host compiler
CXX := g++
TARGET_HOST_DIR = bin/emulator

# Device Compiler
AOC_FLAGS += -emulator-channel-depth-model=strict
AOC_BOARD := -march=emulator
AOCL_LINK_CONFIG := $(shell aocl link-config) -lalteracl -lacl_emulator_kernel_rt
TARGET_DEVICE_DIR := bin/emulator
endif


# ------------------------------------------------------------------------------
# Fast-compile, Debug and Verbose
# ------------------------------------------------------------------------------
ifeq ($(FASTCOMPILE),1)
AOC_FLAGS += -fast-compile
else
endif

ifeq ($(DEBUG),1)
CXXFLAGS += -g
AOC_FLAGS += -g -profile=all
else
CXXFLAGS += -O2
AOC_FLAGS += -g0
endif

ifeq ($(VERBOSE),1)
ECHO :=
else
ECHO := @
endif


.PHONY: host device cleanhost cleandevice clean

$(shell mkdir -p $(TARGET_HOST_DIR) $(TARGET_DEVICE_DIR))

host: $(SRCS) $(INCS) $(TARGET_HOST_DIR)
	$(ECHO)$(CXX) $(CXXFLAGS) \
		$(foreach D,$(INC_DIRS),-I$D) \
		$(AOCL_COMPILE_CONFIG) \
		$(SRCS) \
		$(AOCL_LINK_CONFIG) \
		$(foreach L,$(LIBS),-l$L) \
		-o $(TARGET_HOST_DIR)/$(TARGET_HOST)

device: $(DEVICE_CODE_DIR)/$(DEVICE_CODE).cl $(TARGET_DEVICE_DIR)
	$(ECHO)$(AOC) $(AOC_BOARD) $(AOC_FLAGS) \
		$(DEVICE_CODE_DIR)/$(DEVICE_CODE).cl \
		-I $(DEVICE_INCLUDES) \
		-o $(TARGET_DEVICE_DIR)/$(TARGET_DEVICE)

cleanhost :
	$(ECHO)rm -f $(TARGET_HOST_DIR)/$(TARGET_HOST)

cleandevice :
	$(ECHO)rm -rf $(TARGET_DEVICE_DIR)/$(DEVICE_CODE)
	$(ECHO)rm -f $(TARGET_DEVICE_DIR)/$(DEVICE_CODE).aoco
	$(ECHO)rm -f $(TARGET_DEVICE_DIR)/$(DEVICE_CODE).aocr
	$(ECHO)rm -if $(TARGET_DEVICE_DIR)/$(DEVICE_CODE).aocx

clean : cleanhost cleandevice
