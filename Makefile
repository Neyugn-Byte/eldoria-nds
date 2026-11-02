# Build with devkitPro/devkitARM and libnds installed.
TARGET  := eldoria_v01
BUILD   := build
SOURCES := source
INCLUDES :=

ARCH := -mthumb -mthumb-interwork
CFLAGS := -g -Wall -O2 -ffunction-sections -fdata-sections $(ARCH) $(INCLUDES)
CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions
LDFLAGS := -specs=ds_arm9.specs -g $(ARCH) -Wl,--gc-sections

LIBS := -lnds9

include $(DEVKITPRO)/libnds/Makefile
