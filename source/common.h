#pragma once

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include <time.h>

#include "menu.h"
#include "LED.h"

#define VERSION "2.0" // Version string
#define FB_SIZE 230400 // Bottom framebuffer size

static SwkbdState swkbd;
char input_str[4];

int MenuIndex;

u32 amiibo_appid = 0x10110E00, bruteforce = 0x10000000, seconds;

static char *fb_buffer = NULL;


u8 attack=201, defense=201, speed=201;