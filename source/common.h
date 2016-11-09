#pragma once

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#include "menu.h"
#include "LED.h"

#define VERSION "1.3" // Version string
#define FB_SIZE 230400 // Bottom framebuffer size

static SwkbdState swkbd;
char input_str[4];

int MenuIndex;

u32 amiibo_appid = 0x10110E00; // Hardcoded for Super Smash Bros. See https://www.3dbrew.org/wiki/Amiibo for more details

static char *fb_buffer = NULL;


u8 attack=201;
u8 defense=201;
u8 speed=201;