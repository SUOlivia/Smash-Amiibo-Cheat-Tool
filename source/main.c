#include "common.h"

uint16_t swap_uint16(uint16_t val)
{
	return (val << 8) | (val >> 8);
}

void Convert_Amiibo_Nickname(uint8_t *out, uint16_t *in)
{
	u32 i;
	
	uint16_t LE[0x16];
	
	for(i=0; i<22; i++)	LE[i]=swap_uint16(in[i]);

	utf16_to_utf8(out, LE, sizeof(LE));
}

void wait_for_start()
{
	stfuled();
	printf("Press START to continue\n");
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		hidScanInput();
		gfxFlushBuffers();
		
		u32 kDown = hidKeysDown();
		
		if (kDown & KEY_START)
			break;
	}
}

int load_splash(char *buffer, FILE *f)
{
	if (!buffer)
	{
		printf("Memory not allocated!\n");
		return -1;
	}
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
	rewind(file);
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
	FILE *f = NULL, *backup = NULL, *Scan = NULL;
	
	fb_buffer = malloc(FB_SIZE);
	if (!fb_buffer)
	{
		printf("Failed to allocate framebuffer memory!\n");
		return -1;
	}

	u8 model;
	CFGU_GetSystemModel(&model);

	Scan = fopen("/SACT/Scan.bin", "rb");
	
	if (!Scan)
	{
		if ((model == 2) || (model == 4)) 
			Scan = fopen("romfs:/scan_n3ds.bin", "rb");
		else
			Scan = fopen("romfs:/scan_o3ds.bin", "rb");
	}
	
	if (load_splash(fb_buffer, Scan))
	{
			printf("Failed to load splash!\n");
			return -4;
	}
	
	NFC_TagState prevstate, curstate;
	NFC_TagInfo taginfo;
	NFC_AmiiboSettings amiibosettings;
	NFC_AmiiboConfig amiiboconfig;

	u32 pos;
	u32 index = 0;
	u32 appdata_initialized;
	u32 secret_code[] =
	{
		KEY_UP,
		KEY_UP,
		KEY_DOWN,
		KEY_DOWN,
		KEY_LEFT,
		KEY_RIGHT,
		KEY_LEFT,
		KEY_RIGHT,
		KEY_START
	};

	u8 appdata[0xd8];
	
	uint8_t Name[0x16];
	
	char uidstr[16], tmpstr[262], backupstr[262], path[262], Info[512], bruteforce_appdata[262];
	const char *menu_entries[] =
	{
		"Hack", // Menuindex 0
		"Restore Backup / Bruteforce appdata writing", // MenuIndex 1, MenuIndex -6 if bruteforcing
		"Only dump appdata / Bruteforce appdata dump", // MenuIndex 2, MenuIndex -5 if bruteforcing
		"Custom file writing", // MenuIndex 3
		"Change custom moves", // MenuIndex 4
		"Appdata Randomizing"  //MenuIndex 5
	};

	snprintf(Info, sizeof(Info) - 1,
		"Welcome to the Smash Amiibo Cheat tool "VERSION"\n" \
		"based off the libctru NFC example\nSelect Hack to hack your amiibo\n" \
		"Restore to restore a previous backup\nPlease note that the amiibo needs to:\n" \
		" - previously been setup for this tool to work\n" \
		"You need the NFC reader/writer\nand a9lh/CFW for this tool to work on O3ds\n"
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
	
	MenuIndex = display_menu(menu_entries, 6, Info);
	consoleClear();
	if(MenuIndex==-1)return MenuIndex;
	
	fixcolor(255, 125, 0);
	
	while(aptMainLoop())
	{
		gspWaitForVBlank();
		gfxFlushBuffers();
		gfxSwapBuffers();
		
		hidScanInput();

		memcpy(gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL), fb_buffer, FB_SIZE);
		
		u32 kDown = hidKeysDown();

		if(kDown & KEY_B)break;
		if(kDown & KEY_Y)
		{
		nfcStopScanning();
		nfcStartScanning(NFC_STARTSCAN_DEFAULTINPUT);
		stfuled();
		index = 0;
		MenuIndex = display_menu(menu_entries, 6, Info);
		consoleClear();
		if(MenuIndex==-1)return MenuIndex;
		fixcolor(255, 125, 0);
		}
		
		if(kDown & secret_code[index] && MenuIndex==0)
		{
			if(index==8) 
			{
				MenuIndex = 99;
				printf("Cool, you discovered the 'secret' code :P\n");
			}
			else index++;
		}
		
		nfcGetTagState(&curstate);
		if(curstate!=prevstate)//See nfc.h for the TagState values.
		{
			printf("TagState changed from %d to %d\n", prevstate, curstate);
			prevstate = curstate;

			if(curstate==NFC_TagState_InRange)
			{
				printf("A NFC tag has been found\n");
				
				fixcolor(255, 0, 0);
				
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
				else 
				{
					Convert_Amiibo_Nickname(Name, amiibosettings.nickname);
					printf("Hello %s!!!!\n", Name);
				}
				
			
				ret = nfcGetAmiiboConfig(&amiiboconfig);
				if(R_FAILED(ret))
				{
					printf("Failed to get the Amiibo's configuration.\n");
					break;
				}

				printf("%s's data successfully loaded.\n", Name);

				printf("Opening appdata...\n");

				appdata_initialized = 1;

				if(MenuIndex != 1 && MenuIndex != 2  && MenuIndex != 5) ret = nfcOpenAppData(amiibo_appid);
				else
				{
					ret = nfcOpenAppData(amiibo_appid);
					if(ret==NFC_ERR_APPID_MISMATCH)
					{
						if(MenuIndex == 1) MenuIndex = -6; else if(MenuIndex == 2) MenuIndex = -5;
						clock_t t = clock();
						printf("AppID isn't Sm4sh, currently bruteforcing AppID\n");
						stfuled();
						rave();
						while(bruteforce != 0xFFFFFFFF)
						{
							ret = nfcOpenAppData(bruteforce);
							nfcGetTagState(&curstate);
							if(ret==NFC_ERR_APPID_MISMATCH) bruteforce++;
							else if(curstate == NFC_TagState_OutOfRange)
								{
									printf("Why did you remore your amiibo >:3\n");
									return 0xC0FFE1;
								}
							else
							{
								clock_t t2 = clock() - t;
								seconds = t2 / CLOCKS_PER_SEC;
								printf("AppID match found\nThe AppID of your amiibo is: 0x%X\nIt took %u seconds to find the AppID\n", bruteforce, seconds);
								break;
							}
						}
					}
					
				}
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
					memset(bruteforce_appdata, 0, sizeof(bruteforce_appdata));
					
					snprintf(path, sizeof(path)-1, "/SACT/%s_%s", uidstr, Name);
					snprintf(tmpstr, sizeof(tmpstr)-1, "%s/Modded.amiibo", path);
					snprintf(backupstr, sizeof(backupstr)-1, "%s/Backup.amiibo", path);
					snprintf(bruteforce_appdata, sizeof(bruteforce_appdata)-1, "%s/%X.amiibo", path, bruteforce);
					mkdir("/SACT", 0777);
					mkdir(path, 0777);
					
					if(MenuIndex!=1 && MenuIndex!=2 && MenuIndex!=3 && MenuIndex!=-6)
					{
						if(fopen(tmpstr, "r") != NULL)
							remove(tmpstr); 
						else
							if(bruteforce!=0x10000000 && fopen(bruteforce_appdata, "r") != NULL)
								remove(bruteforce_appdata);
					}
					if(MenuIndex!=1 && MenuIndex!=3 && MenuIndex!=-5 && MenuIndex!=-6) 
						if(fopen(backupstr, "r") != NULL)
							remove(backupstr);
					
					if(MenuIndex==0)
					{
						printf("Modifying the %s's data\n", Name);
						f = fopen(tmpstr, "w");
						backup = fopen(backupstr, "w");
						fwrite(appdata, 1, sizeof(appdata), f);
						fwrite(appdata, 1, sizeof(appdata), backup);
						fclose(f);
						fclose(backup);
						f = fopen(tmpstr, "r+b");
						u8 val = 0xC8;
						u8 nul = 0;
						WriteValTo(f, 16, nul);
						WriteValTo(f, 17, val);
						WriteValTo(f, 18, nul);
						WriteValTo(f, 19, val);
						WriteValTo(f, 20, nul);
						WriteValTo(f, 21, val);
						
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
						printf("Finished, appdata dump is located at '%s'\n", backupstr);
					}
					else if(MenuIndex==3)
					{
						printf("Writing '/SACT/Write.amiibo' to %s\n", Name);
						f = fopen("/SACT/Write.amiibo", "r");
						if(f==NULL) ERRF_ThrowResult(0xFFFFF);
					}
					else if(MenuIndex==4)
					{
						printf("hello");
						f = fopen(tmpstr, "w");
						backup = fopen(backupstr, "w");
						fwrite(appdata, 1, sizeof(appdata), f);
						fwrite(appdata, 1, sizeof(appdata), backup);
						fclose(f);
						fclose(backup);
						f = fopen(tmpstr, "r+b");
						void *buffer =  (char*) malloc (sizeof(char)*2);
						char Info2[255];
						const char *MoveEntries[] =
						{
							"1",
							"2",
							"3"
						};
						fseek(f, 9, SEEK_SET);
						snprintf(Info2, 254, "Neutral: %u", fread(buffer, 1, 1, f));
						int MoveIndex = display_menu(MoveEntries, 3, Info2);
						WriteValTo(f, 9, MoveIndex);
						fseek(f, 10, SEEK_SET);
						snprintf(Info2, 254, "Side: %u", fread(buffer, 1, 1, f));
						MoveIndex = display_menu(MoveEntries, 3, Info2);
						WriteValTo(f, 10, MoveIndex);
						fseek(f, 11, SEEK_SET);
						snprintf(Info2, 254, "Up: %u", fread(buffer, 1, 1, f));
						MoveIndex = display_menu(MoveEntries, 3, Info2);
						WriteValTo(f, 11, MoveIndex);
						fseek(f, 12, SEEK_SET);
						snprintf(Info2, 254, "Down: %u", fread(buffer, 1, 1, f));
						MoveIndex = display_menu(MoveEntries, 3, Info2);				
						WriteValTo(f, 12, MoveIndex);
						
						fclose(f);
						f = fopen(tmpstr, "r");
					}
					else if(MenuIndex==-5)
					{
						backup = fopen(bruteforce_appdata, "w");
						fwrite(appdata, 1, sizeof(appdata), backup);
						fclose(backup);
						printf("Finished, appdata dump is located at '%s'\n", bruteforce_appdata);							
					}
					else if(MenuIndex==-6)
					{
						printf("Writing %s to %s\n", bruteforce_appdata, Name);
						f = fopen(bruteforce_appdata, "r");
						if(f==NULL) ERRF_ThrowResult(0xFFFFF);
					}
					if(MenuIndex==5)
					{
						printf("Randomizing %s's data\n", Name);
						if(bruteforce  == 0x10000000) backup = fopen(backupstr, "w"); else backup = fopen(bruteforce_appdata, "w");
						f = fopen(tmpstr, "w");
						fwrite(appdata, 1, sizeof(appdata), f);
						fwrite(appdata, 1, sizeof(appdata), backup);
						fclose(f);
						fclose(backup);
						f = fopen(tmpstr, "r+b");
						for(int ind = 0; ind<=215; ind++)
							if(ind==0)
								WriteValTo(f, 0, ind);
							else
							{ 
								srand(time(NULL));
								WriteValTo(f, ind, (u8)rand());
							}
						fclose(f);
						f = fopen(tmpstr, "r");
					}
					else if(MenuIndex==99)
					{
						printf("Modifying the %s's data\n", Name);
						f = fopen(tmpstr, "w");
						backup = fopen(backupstr, "w");
						fwrite(appdata, 1, sizeof(appdata), f);
						fwrite(appdata, 1, sizeof(appdata), backup);
						fclose(f);
						fclose(backup);
						f = fopen(tmpstr, "r+b");
						u8 val = 0x7F;
						WriteValTo(f, 16, val);
						WriteValTo(f, 17, val);
						WriteValTo(f, 18, val);
						WriteValTo(f, 19, val);
						WriteValTo(f, 20, val);
						WriteValTo(f, 21, val);
					
						fclose(f);
						f = fopen(tmpstr, "r");
					}
						
					if(MenuIndex!=2 && MenuIndex!=-5)
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

							printf("Writing to %s...\n", Name);

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
					}	
					if(MenuIndex!=2 && MenuIndex!=-5)
					{
						printf("Writing finished.\n");
						fclose(f);
					}
					if((model == 2) || (model == 4)) printf("You can now safely remove %s from the touchscreen\n", Name);
					else printf("You can now safely remove %s from the NFC reader/writer\n", Name);
					fixcolor(0, 255, 0);				
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

	swkbdInit(&swkbd, SWKBD_TYPE_NUMPAD, 2, sizeof(input_str));
	swkbdSetPasswordMode(&swkbd, SWKBD_PASSWORD_NONE);
	swkbdSetValidation(&swkbd, SWKBD_ANYTHING, 0, 4);
	swkbdSetFeatures(&swkbd, SWKBD_DEFAULT_QWERTY);
	swkbdSetNumpadKeys(&swkbd, 0, 0);
	
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
	
	stfuled();
	nfcExit();
	romfsExit();
	cfguExit();
	gfxExit();
	return 0;
}