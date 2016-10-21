#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#include "menu.h"

#define VERSION "Beta 4.0" // Version string

#define FB_SIZE 230400 // Bottom framebuffer size

u32 amiibo_appid = 0x10110E00; // Hardcoded for Super Smash Bros. See https://www.3dbrew.org/wiki/Amiibo for more details

static char *fb_buffer = NULL;

void wait_for_start()
{
	printf("Press START to continue\n");
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		hidScanInput();

		u32 kDown = hidKeysDown();
		
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu
	}
}

int load_splash(char *buffer, const char *path)
{
	if (!buffer)
	{
		printf("Memory not allocated!\n");
		return -1;
	}
	FILE *f = fopen(path, "rb");
	if (!f) return -2;
	fseek(f, 0L, SEEK_END);
	size_t sz = ftell(f);
	rewind(f);
	size_t r = 0;
	if (buffer) r = fread(buffer, (sz > FB_SIZE) ? FB_SIZE : sz, 1, f); // Only read if memory has been properly allocated
	fclose(f);
	return ((r == 0) ? -3 : 0);
}

void WriteValTo(FILE *file, s32 address, u8 val)
{
	fseek(file, address, SEEK_SET);
	fwrite(&val, 1, 1, file);
}

const char *get_model_string(uint32_t id)
{
	const char *console_strings[] =
	{
		"O3DS",
		"O3DS XL",
		"N3DS",
		"2DS",
		"N3DS XL"
	};

	if (id > sizeof(console_strings))
		return "Unknown";

	else return console_strings[id];
}

Result nfc_main()
{
	Result ret = 0;
	FILE *f = NULL, *backup = NULL;

	fb_buffer = malloc(FB_SIZE);
	if (!fb_buffer)
	{
		printf("Failed to allocate framebuffer memory!\n");
		return -1;
	}

	u8 model;
	CFGU_GetSystemModel(&model);

	if ((model == 2) || (model == 4))
	{
		if (load_splash(fb_buffer, "romfs:/scan_n3ds.bin"))
		{
			printf("Failed to load splash!\n");
			return -4;
		}
	}
	else
	{
		if (load_splash(fb_buffer, "romfs:/scan_o3ds.bin"))
		{
			printf("Failed to load splash!\n");
			return -4;
		}
	}


	NFC_TagState prevstate, curstate;
	NFC_TagInfo taginfo;
	NFC_AmiiboSettings amiibosettings;
	NFC_AmiiboConfig amiiboconfig;

	u32 pos;
	u32 appdata_initialized;

	u8 appdata[0xd8];

	char uidstr[16], tmpstr[262], backupstr[262], path[262], Info[512];
	const char *menu_entries[] =
	{
		"Hack",
		"Restore Backup",
		"Only dump appdata",
		"Custom file writing"
	};

	snprintf(Info, sizeof(Info) - 1,
		"Welcome to the Smash Amiibo Cheat tool "VERSION"\n" \
		"based off the libctru NFC example\nSelect Hack to hack your amiibo\n" \
		"Restore to restore a previous backup\nPlease note that the amiibo needs to:\n" \
		" - previously been setup for this tool to work\n" \
		"If you're on O3ds you need the NFC reader/writer\nto already been connected before you select\n"
		"Console: %s\n\n", get_model_string(model));

	ret = nfcStartScanning(NFC_STARTSCAN_DEFAULTINPUT);
	if(R_FAILED(ret))
	{
		printf("Failed init\n");
		free(fb_buffer);
		return ret;
	}

	printf("NFC initialized successfully, scanning...\n");

	ret = nfcGetTagState(&curstate);
	if(R_FAILED(ret))
	{
		printf("Failed to get NFC state\n");
		nfcStopScanning();
		free(fb_buffer);
		return ret;
	}

	prevstate = curstate;
	
	int MenuIndex = display_menu(menu_entries, 4, Info);
	consoleClear();
	
	while(aptMainLoop())
	{
		gspWaitForVBlank();
		memcpy(gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL), fb_buffer, FB_SIZE);
		gfxFlushBuffers();
		gfxSwapBuffers();
		
		hidScanInput();

		u32 kDown = hidKeysDown();

		if(kDown & KEY_B)break;
		
		memcpy(gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL), fb_buffer, FB_SIZE);
		nfcGetTagState(&curstate);
		if(curstate!=prevstate)//See nfc.h for the TagState values.
		{
			printf("TagState changed from %d to %d\n", prevstate, curstate);
			prevstate = curstate;

			if(curstate==NFC_TagState_InRange)
			{
				printf("A NFC tag has been found\n");
				
				memset(&taginfo, 0, sizeof(NFC_TagInfo));
				memset(&amiibosettings, 0, sizeof(NFC_AmiiboSettings));
				nfcGetTagInfo(&taginfo);

				printf("NFC tag UID:\n");

				memset(uidstr, 0, sizeof(uidstr));
				for(pos=0; pos<7; pos++)snprintf(&uidstr[pos*2], 3, "%02x", taginfo.id[pos]);
				printf("%s\n", uidstr);

				ret = nfcLoadAmiiboData();
				if(R_FAILED(ret))
				{
					printf("Failed to load the Amiibo's data.\n");
					break;
				}

				memset(&amiibosettings, 0, sizeof(NFC_AmiiboSettings));
				memset(&amiiboconfig, 0, sizeof(NFC_AmiiboConfig));

				ret = nfcGetAmiiboSettings(&amiibosettings);
				if(R_FAILED(ret))
				{
					if(ret==NFC_ERR_AMIIBO_NOTSETUP)printf("This amiibo wasn't setup by the amiibo Settings applet.\n");
					break;
				}
			
				ret = nfcGetAmiiboConfig(&amiiboconfig);
				if(R_FAILED(ret))
				{
					printf("Failed to get the Amiibo's configuration.\n");
					break;
				}

				printf("amiibo data successfully loaded.\n");

				printf("Opening appdata...\n");

				appdata_initialized = 1;

				ret = nfcOpenAppData(amiibo_appid);
				if(R_FAILED(ret))
				{
					printf("Failed to open the appdata.\n");
					if(ret==NFC_ERR_APPDATA_UNINITIALIZED)
					{
						printf("This appdata isn't initialized.\n");
						appdata_initialized = 0;
					}
					if(ret==NFC_ERR_APPID_MISMATCH)printf("This appdata is for a different appid(non-Super-Smash-Bros).\n");
					if(appdata_initialized)break;
				}

				memset(appdata, 0, sizeof(appdata));

				if(!appdata_initialized)
				{
					printf("Skipping appdata reading since it's uninitialized.\n");
				}
				else
				{
					printf("Reading appdata...\n");

					ret = nfcReadAppData(appdata, sizeof(appdata));
					if(R_FAILED(ret))
					{
						printf("nfcReadAppData() failed.\n");
						break;
					}

					memset(tmpstr, 0, sizeof(tmpstr));
					memset(backupstr, 0, sizeof(backupstr));
					memset(path, 0, sizeof(path));
					
					snprintf(path, sizeof(path)-1, "/Smash Amiibo Cheat Tool/%s", uidstr);
					snprintf(tmpstr, sizeof(tmpstr)-1, "%s/Modded.amiibo", path);
					snprintf(backupstr, sizeof(backupstr)-1, "%s/Backup.amiibo", path);
					mkdir("/Smash Amiibo Cheat Tool", 0777);
					mkdir(path, 0777);
					
					if(1)
					{
						
						if(MenuIndex==0)
						{
						printf("Modifying the amiibo's data\n");
						f = fopen(tmpstr, "w");
						backup = fopen(backupstr, "w");
						fwrite(appdata, 1, sizeof(appdata), f);
						fwrite(appdata, 1, sizeof(appdata), backup);
						fclose(f);
						fclose(backup);
						f = fopen(tmpstr, "r+b");
						u8 val = 0xC8;
						u8 nul = 0x00;
						WriteValTo(f, 0x10, nul);
						WriteValTo(f, 0x11, val);
						WriteValTo(f, 0x12, nul);
						WriteValTo(f, 0x13, val);
						WriteValTo(f, 0x14, nul);
						WriteValTo(f, 0x15, val);
						
						fclose(f);
						f = fopen(tmpstr, "r");
						}
						else if(MenuIndex==1)
						{
							printf("Restauring backup\n");
							f = fopen(backupstr, "r");
						}
						else if(MenuIndex==2)
						{
							backup = fopen(backupstr, "w");
							fwrite(appdata, 1, sizeof(appdata), backup);
							fclose(backup);
							printf("Finished, appdata dump is located at '%s'", backupstr);
						}
						else
						{
							printf("Writing '/Smash Amiibo Cheat Tool/Write.amiibo' to amiibo");
							f = fopen("/Smash Amiibo Cheat Tool/Write.amiibo", "r");
						}
						
						
						if(MenuIndex!=2)
						{
						fread(appdata, 1, sizeof(appdata), f);
						if(appdata_initialized)
						{
							printf("Writing the modifications...\n");

							ret = nfcWriteAppData(appdata, sizeof(appdata), &taginfo);
							if(R_FAILED(ret))
							{
								printf("Failed to write the modifications.\n");
								break;
							}

							printf("Writing to the amiibo NFC tag...\n");

							ret = nfcUpdateStoredAmiiboData();
							if(R_FAILED(ret))
							{
								printf("nfcUpdateStoredAmiiboData() failed.\n");
								break;
							}
						}
						else
						{
							printf("Initializing the appdata...\n");

							ret = nfcInitializeWriteAppData(amiibo_appid, appdata, sizeof(appdata));
							if(R_FAILED(ret))
							{
								printf("nfcInitializeWriteAppData() failed.\n");
								break;
							}
						}

						
					if(MenuIndex!=2)
					{
						printf("Writing finished.\n");
						fclose(f);
					}
					if(model==0) printf("You can now safely remove your amiibo from the NFC reader/writer");
					else printf("You can now safely remove your amiibo from the touchscreen");
					
					}
					}
				}
			
			}
		}
	}
	return ret;
	nfcStopScanning();
}

int main()
{
	Result ret=0;

	gfxInitDefault();
	cfguInit();
	romfsInit();

	consoleInit(GFX_TOP, NULL);
	
	ret = nfcInit(NFC_OpType_NFCTag);
	if(R_FAILED(ret))
	{
		printf("nfcInit() failed: 0x%08x.\n", (unsigned int)ret);
	}
	else
	{
		ret = nfc_main();
		printf("nfcInit() returned: 0x%08x.\n", (unsigned int)ret);

		nfcExit();
	}
	wait_for_start();

	romfsExit();
	cfguExit();
	gfxExit();
	return 0;
}
