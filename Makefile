## Creator: Cameron Tacklind
##
## This is the main Makefile for building AVR projects. It should only be edited
## to add generic and project specific features. Ideally, there are no real targets
## here. Just variables, or overrides, that the included helper makefiles will use.

# List of C file basenames to build
C = 

# List of CPP file basenames to build
cppNames = main Board Debug Timer Clock

cppNames += ThreePhaseDriver
cppNames += MLX90363 ThreePhasePositionEstimator ThreePhaseController

cppNames += ServoController

cppNames += MLXDebug

cppNames += TripleBuffer-impl BlockBuffer-impl

cppNames += Demo
cppNames += Calibration

cppNames += HallWatcher

#cppNames += LookupTable/$(MotorID)

cppNames += LookupTable

cppNames += SerialInterface

#MotorID = Motor1

AVRpp_SRC = TimerTimeout USART gccGuard #ADC

libCameron_SRC = CRC8 DecPrintFormatter

# Select specific LUFA source files to compile like this
#LUFA_SRC = LUFA/Drivers/USB/Class/Common/HIDParser.c

F_CPU = 16000000UL

#BLD_STD_GCC ?= c11
#BLD_STD_GXX ?= c++11

TARGET = turnigy

MCU = atmega32u4

all: build-lss run
run: dfu-erase dfu-flash dfu-reset
#run: run-remote

REMOTE_HEX = $(TARGET).hex

#ASM = $(CPP:%=%.cpp.S)

# Load local settings
-include local.mk
-include local.$(shell hostname).mk

uMakerPath ?= uMaker/

include $(uMakerPath)tools/paths.mk

# Generate list of source files from basenames
include $(uMakerPath)tools/source.mk

# Force setting certain make flags
#include $(uMakerPath)tools/makeflags.mk

# Optional configuration testing for development
include $(uMakerPath)tools/checks.mk

# Defs for our setup
include $(uMakerPath)vars/AVR.mk


# Library targets
include $(uMakerPath)tools/AVR/lufa.mk
#include $(uMakerPath)tools/extlib.mk
include $(uMakerPath)tools/AVR/AVR++.mk
include $(uMakerPath)tools/libCameron.mk

# Build targets
include $(uMakerPath)tools/build.mk

# Intermediate assembly
include $(uMakerPath)tools/assembly.mk

# Programmer targets
include $(uMakerPath)tools/dfu.mk
#include $(uMakerPath)tools/nrfjprog.mk
#include $(uMakerPath)tools/AVR/avrdude.mk

# Directory creation targets
include $(uMakerPath)tools/mkdir.mk

run-remote: $(OUT_HEX)
	pscp -q $(OUT_HEX) pi@sleepypi:$(REMOTE_HEX)
	-plink pi@sleepypi sudo $(DFU_TARGETED) flash $(REMOTE_HEX)
	-plink pi@sleepypi sudo $(DFU_TARGETED) reset
	plink pi@sleepypi rm $(REMOTE_HEX)

.PHONY: all run run-remote
