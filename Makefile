# TARGET #

TARGET ?= 3DS
LIBRARY := 0

ifeq ($(TARGET),3DS)
    ifeq ($(strip $(DEVKITPRO)),)
        $(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPro")
    endif

    ifeq ($(strip $(DEVKITARM)),)
        $(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
    endif
endif

# COMMON CONFIGURATION #

NAME := Smash Amiibo Cheat Tool

BUILD_DIR := build
OUTPUT_DIR := output
INCLUDE_DIRS := include
SOURCE_DIRS := source

EXTRA_OUTPUT_FILES :=

ifeq ($(TARGET),3DS)
	LIBRARY_DIRS := $(DEVKITPRO)/libctru
	LIBRARIES := citro3d ctru
else
	LIBRARY_DIRS :=
	LIBRARIES := ncurses SDL2
endif

BUILD_FLAGS := -O3 -Wno-unused-function -Wno-unused-result
ifeq ($(TARGET),3DS)
	BUILD_FLAGS += -DBACKEND_3DS
else
	BUILD_FLAGS += -DBACKEND_SDL
endif

RUN_FLAGS :=

VERSION_MAJOR := $(word 1, $(VERSION_PARTS))
VERSION_MINOR := $(word 2, $(VERSION_PARTS))
VERSION_MICRO := $(word 3, $(VERSION_PARTS))

# 3DS CONFIGURATION #

TITLE := $(NAME)
DESCRIPTION := Hack your smash amiibo in a tap
AUTHOR := Ordim3n
PRODUCT_CODE := CTR-P-SAHT
UNIQUE_ID := 0xA7FC8

SYSTEM_MODE := 64MB
SYSTEM_MODE_EXT := Legacy

ICON_FLAGS :=

ROMFS_DIR := romfs
BANNER_AUDIO := meta/banner.wav
BANNER_IMAGE := meta/banner.png
ICON := meta/icon.png

# INTERNAL #

include buildtools/make_base
