#include <3ds.h>
#include <string.h>

Handle ptmsysmHandle = 0;

Result ptmsysmInit()
{
    return srvGetServiceHandle(&ptmsysmHandle, "ptm:sysm");
}

Result ptmsysmExit()
{
    return svcCloseHandle(ptmsysmHandle);
}

typedef struct
{
    u32 ani;
    u8 r[32];
    u8 g[32];
    u8 b[32];
} RGBLedPattern;

Result ptmsysmSetInfoLedPattern(RGBLedPattern pattern)
{
    u32* ipc = getThreadCommandBuffer();
    ipc[0] = 0x8010640;
    memcpy(&ipc[1], &pattern, 0x64);
    Result ret = svcSendSyncRequest(ptmsysmHandle);
    if(ret < 0) return ret;
    return ipc[1];
}

void fixcolor(u8 r, u8 g, u8 b)
{
    RGBLedPattern pat;
    memset(&pat.r, r, 32);
    memset(&pat.g, g, 32);
    memset(&pat.b, b, 32);
    pat.ani = 0xFF0000;
    if(ptmsysmInit() < 0) return;
    ptmsysmSetInfoLedPattern(pat);
    ptmsysmExit();
}

void rave()
{
	RGBLedPattern pat2;
    
    //marcus@Werkstaetiun:/media/marcus/WESTERNDIGI/dev_threedee/MCU_examples/RGB_rave$ lua assets/colorgen.lua
    pat2.r[0] = 128;
    pat2.r[1] = 103;
    pat2.r[2] = 79;
    pat2.r[3] = 57;
    pat2.r[4] = 38;
    pat2.r[5] = 22;
    pat2.r[6] = 11;
    pat2.r[7] = 3;
    pat2.r[8] = 1;
    pat2.r[9] = 3;
    pat2.r[10] = 11;
    pat2.r[11] = 22;
    pat2.r[12] = 38;
    pat2.r[13] = 57;
    pat2.r[14] = 79;
    pat2.r[15] = 103;
    pat2.r[16] = 128;
    pat2.r[17] = 153;
    pat2.r[18] = 177;
    pat2.r[19] = 199;
    pat2.r[20] = 218;
    pat2.r[21] = 234;
    pat2.r[22] = 245;
    pat2.r[23] = 253;
    pat2.r[24] = 255;
    pat2.r[25] = 253;
    pat2.r[26] = 245;
    pat2.r[27] = 234;
    pat2.r[28] = 218;
    pat2.r[29] = 199;
    pat2.r[30] = 177;
    pat2.r[31] = 153;
    pat2.g[0] = 238;
    pat2.g[1] = 248;
    pat2.g[2] = 254;
    pat2.g[3] = 255;
    pat2.g[4] = 251;
    pat2.g[5] = 242;
    pat2.g[6] = 229;
    pat2.g[7] = 212;
    pat2.g[8] = 192;
    pat2.g[9] = 169;
    pat2.g[10] = 145;
    pat2.g[11] = 120;
    pat2.g[12] = 95;
    pat2.g[13] = 72;
    pat2.g[14] = 51;
    pat2.g[15] = 33;
    pat2.g[16] = 18;
    pat2.g[17] = 8;
    pat2.g[18] = 2;
    pat2.g[19] = 1;
    pat2.g[20] = 5;
    pat2.g[21] = 14;
    pat2.g[22] = 27;
    pat2.g[23] = 44;
    pat2.g[24] = 65;
    pat2.g[25] = 87;
    pat2.g[26] = 111;
    pat2.g[27] = 136;
    pat2.g[28] = 161;
    pat2.g[29] = 184;
    pat2.g[30] = 205;
    pat2.g[31] = 223;
    pat2.b[0] = 18;
    pat2.b[1] = 33;
    pat2.b[2] = 51;
    pat2.b[3] = 72;
    pat2.b[4] = 95;
    pat2.b[5] = 120;
    pat2.b[6] = 145;
    pat2.b[7] = 169;
    pat2.b[8] = 192;
    pat2.b[9] = 212;
    pat2.b[10] = 229;
    pat2.b[11] = 242;
    pat2.b[12] = 251;
    pat2.b[13] = 255;
    pat2.b[14] = 254;
    pat2.b[15] = 248;
    pat2.b[16] = 238;
    pat2.b[17] = 223;
    pat2.b[18] = 205;
    pat2.b[19] = 184;
    pat2.b[20] = 161;
    pat2.b[21] = 136;
    pat2.b[22] = 111;
    pat2.b[23] = 87;
    pat2.b[24] = 64;
    pat2.b[25] = 44;
    pat2.b[26] = 27;
    pat2.b[27] = 14;
    pat2.b[28] = 5;
    pat2.b[29] = 1;
    pat2.b[30] = 2;
    pat2.b[31] = 8;
    
    pat2.ani = 0x20;

	if(ptmsysmInit() < 0) return;
    ptmsysmSetInfoLedPattern(pat2);
	ptmsysmExit();
}

void stfuled()
{
    RGBLedPattern pat;
    memset(&pat, 0, sizeof(pat));
    pat.ani = 0xFF0000;
    if(ptmsysmInit() < 0) return;
    ptmsysmSetInfoLedPattern(pat);
    ptmsysmExit();
}