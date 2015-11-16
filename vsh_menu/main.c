#include <arpa/inet.h>

#define ssend(socket, str) send(socket, str, strlen(str), 0)

#include <sys/prx.h>
#include <sys/ppu_thread.h>
#include <sys/process.h>
#include <sys/event.h>
#include <sys/syscall.h>
#include <sys/memory.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/sys_time.h>
#include <sys/timer.h>
#include <cell/pad.h>
#include <cell/cell_fs.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#include <netinet/in.h>

#include "include/vsh_exports.h"

#ifdef DEBUG
#include "include/network.h"
#endif

#include "include/misc.h"
#include "include/mem.h"
#include "include/blitting.h"

SYS_MODULE_INFO(VSH_MENU, 0, 1, 0);
SYS_MODULE_START(vsh_menu_start);
SYS_MODULE_STOP(vsh_menu_stop);

#define THREAD_NAME         "vsh_menu_thread"
#define STOP_THREAD_NAME    "vsh_menu_stop_thread"

#define VSH_MODULE_PATH 	"/dev_blind/vsh/module/"
#define VSH_ETC_PATH		"/dev_blind/vsh/etc/"

#define FAILED       -1

#define WHITE				0xFFFFFFFF
#define GREEN				0xFF00FF00
#define BLUE				0xFF008FFF
#define GRAY				0xFF999999
#define YELLOW				0xFFFFFF55

enum menus
{
    MAIN_MENU,
    REBUG_MENU,
    PLUGINS_MANAGER,
    FILE_MANAGER,
};

typedef struct
{
	uint8_t usb0;
	uint8_t usb1;
	uint8_t usb2;
	uint8_t usb3;
	uint8_t usb6;
	uint8_t usb7;
	uint8_t netd0;
	uint8_t lastp;
	uint8_t autob;
	uint8_t delay;
	uint8_t bootd;
	uint8_t boots;
	uint8_t blind;
	uint8_t nogrp;
	uint8_t noset;
	uint8_t cmask;
	uint32_t netp0;
	char neth0[16];
	uint8_t poll;
	uint8_t ftpd;
	uint8_t warn;
	uint8_t fanc;
	uint8_t temp1;
	uint8_t rxvid;
	uint8_t bind;
	uint8_t refr;
	uint8_t manu;
	uint8_t temp0;
	uint8_t netd1;
	uint32_t netp1;
	char neth1[16];
	uint8_t foot;
	uint8_t nopad;
	uint8_t nocov;
	uint8_t nospoof;
	uint8_t ps2temp;
	uint8_t pspl;
	uint8_t minfan;
	uint16_t combo;
	uint8_t sidps;
	uint8_t spsid;
	uint8_t spp;
	uint8_t lang;
	char vIDPS1[17];
	char vIDPS2[17];
	char vPSID1[17];
	char vPSID2[17];
	uint8_t tid;
	uint8_t wmdn;
	char autoboot_path[256];
	uint8_t ps2l;
	uint32_t combo2;
	uint8_t homeb;
	char home_url[256];
	uint8_t netd2;
	uint32_t netp2;
	char neth2[16];
	uint8_t profile;
	char uaccount[9];
	char allow_ip[16];
	uint8_t noss;
	uint8_t fixgame;
	uint8_t bus;
	uint8_t dev_sd;
	uint8_t dev_ms;
	uint8_t dev_cf;
	uint8_t ps1emu;
	uint8_t autoplay;
	uint8_t use_filename;
	uint32_t rec_video_format;
	uint32_t rec_audio_format;
	uint8_t keep_ccapi;
	char padding[67];
} __attribute__((packed)) WebmanCfg;

typedef struct
{
	uint16_t bgindex;
	uint8_t  dnotify;
	char filler[509];
} __attribute__((packed)) vsh_menu_Cfg;

static char FW[10];
static char payload_type[64];
static char kernel_type[64];
static void get_firmware_version(void);

struct platform_info {
	uint32_t firmware_version;
} info;

static uint8_t vsh_menu_config[sizeof(vsh_menu_Cfg)];
static vsh_menu_Cfg *config = (vsh_menu_Cfg*) vsh_menu_config;


static sys_ppu_thread_t vsh_menu_tid = -1;
static int32_t running = 1;
static uint8_t menu_running = 0;    // vsh menu off(0) or on(1)
static uint8_t clipboard_mode = 0;

int32_t vsh_menu_start(uint64_t arg);
int32_t vsh_menu_stop(void);

static void finalize_module(void);
static void vsh_menu_stop_thread(uint64_t arg);

static char tempstr[512];
static uint16_t t_icon_X;
static char netstr[64];
static char cfw_str[64];
static char drivestr[6][64];
static uint8_t drive_type[6];

static uint8_t has_icon0 = 0;
static uint8_t wm_unload = 0;

#define MAX_PATH_LEN 128
#define MAX_ITEMS    256

#define REFRESH_DIR  0

static char curdir[MAX_PATH_LEN] = "/";
static char items[MAX_ITEMS][MAX_PATH_LEN];
static uint8_t items_isdir[MAX_ITEMS];
static uint16_t nitems = 0, cur_item = 0, curdir_offset = 0, cdir = 0;

static char item_size[64];

char *current_file[512];

extern int32_t netctl_main_9A528B81(int32_t size, const char *ip);  // get ip addr of interface "eth0"

////////////////////////////////////////////////////////////////////////
//            SYS_PPU_THREAD_EXIT, DIRECT OVER SYSCALL                //
////////////////////////////////////////////////////////////////////////
static inline void _sys_ppu_thread_exit(uint64_t val)
{
  system_call_1(41, val);
}

////////////////////////////////////////////////////////////////////////
//                         GET MODULE BY ADDRESS                      //
////////////////////////////////////////////////////////////////////////
static inline sys_prx_id_t prx_get_module_id_by_address(void *addr)
{
  system_call_1(461, (uint64_t)(uint32_t)addr);
  return (int32_t)p1;
}

////////////////////////////////////////////////////////////////////////
//                      GET CPU & RSX TEMPERATURES                    //
////////////////////////////////////////////////////////////////////////
static void get_temperature(uint32_t _dev, uint32_t *_temp)
{
  system_call_2(383, (uint64_t)(uint32_t) _dev, (uint64_t)(uint32_t) _temp);
}

////////////////////////////////////////////////////////////////////////
//              DELETE TURNOFF FILE TO AVOID BAD SHUTDOWN             //
////////////////////////////////////////////////////////////////////////
static void soft_reboot(void)
{
  cellFsUnlink((char*)"/dev_hdd0/tmp/turnoff");
  {system_call_3(379, 0x8201, NULL, 0);}
    sys_ppu_thread_exit(0);
}

static void hard_reboot(void)
{
  cellFsUnlink((char*)"/dev_hdd0/tmp/turnoff");
  {system_call_3(379, 0x1200, NULL, 0);}
    sys_ppu_thread_exit(0);
}

static void shutdown_system(void)
{
  cellFsUnlink((char*)"/dev_hdd0/tmp/turnoff");
  {system_call_4(379, 0x1100, 0, 0, 0);}
    sys_ppu_thread_exit(0);
}

static int connect_to_webman(void)
{
	struct sockaddr_in sin;
	int s;

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = 0x7F000001; //127.0.0.1 (localhost)
	sin.sin_port = htons(80);         //http port (80)
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
	{
		return -1;
	}

	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		return -1;
	}

	return s;
}

static void send_wm_request(char *cmd)
{
	// send command
	int conn_s=-1;
	conn_s=connect_to_webman();
	if(conn_s>=0) ssend(conn_s, cmd);
/*
	uint64_t written; int fd=0;
	if(cellFsOpen("/dev_hdd0/tmp/wm_request", CELL_FS_O_CREAT|CELL_FS_O_WRONLY|CELL_FS_O_TRUNC, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
	{
		cellFsWrite(fd, (void *)cmd, strlen(cmd), &written);
		cellFsClose(fd);
        return CELL_FS_SUCCEEDED;
	}
	else

		return -1;

}

static void wait_for_request(void)
{
    struct CellFsStat s;
    int timeout = 20;
    while(timeout > 0 && cellFsStat((char*)"/dev_hdd0/tmp/wm_request", &s) == CELL_FS_SUCCEEDED) {sys_timer_usleep(250000); --timeout;}

    sys_timer_usleep(100000);
*/
}


////////////////////////////////////////////////////////////////////////
//                      EJECT/INSERT DISC CMD                         //
////////////////////////////////////////////////////////////////////////
static void eject_insert(uint8_t eject, uint8_t insert)
{
  uint8_t atapi_cmnd2[56];
  uint8_t* atapi_cmnd = atapi_cmnd2;
  int dev_id;

  memset(atapi_cmnd, 0, 56);
        atapi_cmnd[0x00]=0x1b;
        atapi_cmnd[0x01]=0x01;
  if(eject) atapi_cmnd[0x04]=0x02;
  if(insert)  atapi_cmnd[0x04]=0x03;
        atapi_cmnd[0x23]=0x0c;

  {system_call_4(600, 0x101000000000006ULL, 0, (uint64_t)(uint32_t) &dev_id, 0);}      //SC_STORAGE_OPEN
  {system_call_7(616, dev_id, 1, (uint64_t)(uint32_t) atapi_cmnd, 56, NULL, 0, NULL);} //SC_STORAGE_INSERT_EJECT
  {system_call_1(601, dev_id);}                                                        //SC_STORAGE_CLOSE
  sys_timer_sleep(2);
}

////////////////////////////////////////////////////////////////////////
//                      GET FIRMWARE VERSION                          //
////////////////////////////////////////////////////////////////////////

#define SYSCALL8_OPCODE_GET_MAMBA				0x7FFFULL
#define SYSCALL8_OPCODE_GET_VERSION				0x7000
#define SYSCALL8_OPCODE_GET_VERSION2			0x7001

static int sys_get_version(uint32_t *version)
{
	system_call_2(8, SYSCALL8_OPCODE_GET_VERSION, (uint64_t)(uint32_t)version);
	return (int)p1;
}

static int sys_get_version2(uint16_t *version)
{
	system_call_2(8, SYSCALL8_OPCODE_GET_VERSION2, (uint64_t)(uint32_t)version);
	return (int)p1;
}

static int is_cobra_based(void)
{
    uint32_t version = 0x99999999;

    if (sys_get_version(&version) < 0)
        return 0;

    if (version != 0x99999999) // If value changed, it is cobra
        return 1;

    return 0;
}

static int lv2_get_platform_info(struct platform_info *info)
{
	system_call_1(387, (uint32_t) info);
	return (int32_t)p1;
}

static void get_firmware_version(void)
{
	lv2_get_platform_info(&info);
	sprintf(FW, "%02X", info.firmware_version);
}

static void get_kernel_type(void)
{
	uint64_t type;
	memset(kernel_type, 0, 64);
	system_call_1(985, (uint32_t)&type);
	if(type == 1) sprintf(kernel_type, "CEX"); else // Retail
	if(type == 2) sprintf(kernel_type, "DEX"); else // Debug
	if(type == 3) sprintf(kernel_type, "Debugger"); // Debugger
}

static void get_payload_type(void)
{
	if(!is_cobra_based()) return;

	bool is_mamba; {system_call_1(8, SYSCALL8_OPCODE_GET_MAMBA); is_mamba = ((int)p1 ==0x666);}

	uint16_t cobra_version; sys_get_version2(&cobra_version);
    sprintf(payload_type, "%s %X.%X", is_mamba ? "Mamba" : "Cobra", cobra_version>>8, (cobra_version & 0xF) ? (cobra_version & 0xFF) : ((cobra_version>>4) & 0xF));
}

static void get_network_info(void)
{
  char netdevice[32];
  char ipaddr[32];

  net_info info1;
  memset(&info1, 0, sizeof(net_info));
  xsetting_F48C0548()->sub_44A47C(&info1);

  if (info1.device == 0)
  {
    strcpy(netdevice, "LAN");
  }
  else if (info1.device == 1)
  {
    strcpy(netdevice, "WLAN");
  }
  else
    strcpy(netdevice, "[N/A]");

  int32_t size = 0x10;
  char ip[size];
  netctl_main_9A528B81(size, ip);

  if (ip[0] == '\0')
    strcpy(ipaddr, "[N/A]");
  else
    sprintf(ipaddr, "%s", ip);

  sprintf(netstr, "Network connection :  %s\r\nIP address :  %s", netdevice, ipaddr);
}

#define SC_COBRA_SYSCALL8 8
#define SYSCALL8_OPCODE_LOAD_VSH_PLUGIN                 0x1EE7
#define SYSCALL8_OPCODE_UNLOAD_VSH_PLUGIN               0x364F
#define SYSCALL8_OPCODE_PS3MAPI                         0x7777
#define PS3MAPI_OPCODE_GET_VSH_PLUGIN_INFO              0x0047
#define PS3MAPI_OPCODE_UNLOAD_VSH_PLUGIN                0x0046

char *strcasestr(const char *s1, const char *s2);

static int cobra_load_vsh_plugin(unsigned int slot, char *path, void *arg, uint32_t arg_size)
{
	system_call_5(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_LOAD_VSH_PLUGIN, slot, (uint64_t)(uint32_t)path, (uint64_t)(uint32_t)arg, arg_size);
	return (int)p1;
}

static int cobra_unload_vsh_plugin(unsigned int slot)
{
	system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_UNLOAD_VSH_PLUGIN, slot);
	return (int)p1;
}

static char h2a(char hex)
{
	char c = hex;
	if(c>=0 && c<=9)
		c += '0';
	else if(c>=10 && c<=15)
		c += 0x57; //a-f
	return c;
}

static void urlenc(char *dst, char *src)
{
	size_t j=0;
    size_t n=strlen(src);
	for(size_t i=0; i<n; i++,j++)
	{
		     if(src[i]==' ') {dst[j++] = '%'; dst[j++] = '2'; dst[j] = '0';}
		else if(src[i]==':') {dst[j++] = '%'; dst[j++] = '3'; dst[j] = 'A';}
		else if(src[i] & 0x80)
		{
			dst[j++] = '%';
			dst[j++] = h2a((unsigned char)src[i]>>4);
			dst[j] = h2a(src[i] & 0xf);
		}
		else if(src[i]==34) {dst[j++] = '%'; dst[j++] = '2'; dst[j] = '2';}
		else if(src[i]==39) {dst[j++] = '%'; dst[j++] = '2'; dst[j] = '7';}
		else dst[j] = src[i];
	}
	dst[j] = '\0';
}

static int isDir(const char* path)
{
	struct CellFsStat s;
	if(cellFsStat(path, &s)==CELL_FS_SUCCEEDED)
		return ((s.st_mode & CELL_FS_S_IFDIR) != 0);
	else
		return 0;
}

static int file_exists(const char* path)
{
	struct CellFsStat s;
	return (cellFsStat(path, &s)==CELL_FS_SUCCEEDED);
}

static int del(char *path, bool recursive)
{
	if(!isDir(path)) {return cellFsUnlink(path);}
	if(strlen(path)<11 || !memcmp(path, "/dev_bdvd", 9) || !memcmp(path, "/dev_flash", 10) || !memcmp(path, "/dev_blind", 10)) return FAILED;

	int fd;
	uint64_t read;
	CellFsDirent dir;
	char entry[MAX_PATH_LEN];

	if(cellFsOpendir(path, &fd) == CELL_FS_SUCCEEDED)
	{
		read = sizeof(CellFsDirent);
		while(!cellFsReaddir(fd, &dir, &read))
		{
			if(!read) break;
			if(dir.d_name[0]=='.' && (dir.d_name[1]=='.' || dir.d_name[1]==0)) continue;

			sprintf(entry, "%s/%s", path, dir.d_name);

			if(isDir(entry))
				{if(recursive) del(entry, recursive);}
			else
				cellFsUnlink(entry);
		}
		cellFsClosedir(fd);
	}
	else
		return FAILED;

	if(recursive) cellFsRmdir(path);

	return CELL_FS_SUCCEEDED;
}

static unsigned int get_vsh_plugin_slot_by_name(char *name, bool unload)
{
	char tmp_name[30];
	char tmp_filename[256];
	unsigned int slot, unused_slot = 0;

	for (slot = 1; slot < 7; slot++)
	{
		memset(tmp_name, 0, sizeof(tmp_name));
		memset(tmp_filename, 0, sizeof(tmp_filename));
		{system_call_5(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_VSH_PLUGIN_INFO, (uint64_t)slot, (uint64_t)(uint32_t)tmp_name, (uint64_t)(uint32_t)tmp_filename); }

		if(strstr(tmp_filename, name) || !strcmp(tmp_name, name))
		{
			if(strstr(tmp_filename, "webftp_server"))
			{
				if(unload) {if(wm_unload) continue; send_wm_request((char *)"GET /quit.ps3"); return 0;}
				return wm_unload ? 0 : slot;
			}
			else
			if(unload)
			{
				cobra_unload_vsh_plugin(slot);
				return 0;
			}
			else
				return slot;
		}

		if(unused_slot == 0 && strlen(tmp_name)==0) unused_slot = slot;
	}
	return unload ? unused_slot : 0;
}

static void start_VSH_Menu(void)
{
  struct CellFsStat s;
  char bg_image[48], sufix[8];

  for(uint8_t i = 0; i < 2; i++)
  {
      if(config->bgindex == 0) sprintf(sufix, ""); else sprintf(sufix, "_%i", config->bgindex);
      sprintf(bg_image, "/dev_hdd0/wm_vsh_menu%s.png", sufix);
      if(cellFsStat(bg_image, &s) == CELL_FS_SUCCEEDED) break; else sprintf(bg_image, "/dev_hdd0/plugins/wm_vsh_menu%s.png", sufix);
      if(cellFsStat(bg_image, &s) == CELL_FS_SUCCEEDED) break; else sprintf(bg_image, "/dev_hdd0/littlebalup_vsh_menu%s.png", sufix);
      if(cellFsStat(bg_image, &s) == CELL_FS_SUCCEEDED) break; else sprintf(bg_image, "/dev_hdd0/plugins/images/wm_vsh_menu%s.png", sufix);
      if(cellFsStat(bg_image, &s) == CELL_FS_SUCCEEDED) break; else sprintf(bg_image, "/dev_hdd0/plugins/littlebalup_vsh_menu%s.png", sufix);
      if(cellFsStat(bg_image, &s) == CELL_FS_SUCCEEDED) break; else config->bgindex = 0;
  }

  rsx_fifo_pause(1);

  int32_t ret, mem_size;

  // create VSH Menu heap memory from memory container 1("app")
  mem_size = (((CANVAS_W * CANVAS_H * 4 * 2) + (FONT_CACHE_MAX * 32 * 32)) + (320 * 176 * 4) + MB(4)) / MB(1);
  ret = create_heap(mem_size);  // 6 MB

  if(ret) return;

  // initialize VSH Menu graphic
  init_graphic();

  // set_font(17, 17, 1, 1);  // set font(char w/h = 20 pxl, line-weight = 1 pxl, distance between chars = 1 pxl)

  // load png image
  load_png_bitmap(0, bg_image);

  get_payload_type();

  get_network_info();

  sprintf(cfw_str, "Firmware : %c.%c%c %s %s", FW[0], FW[2], FW[3], kernel_type, payload_type);

  // stop vsh pad
  start_stop_vsh_pad(0);

  // set menu_running on
  menu_running = 1;

  // reset clipboard mode
  clipboard_mode = 0;
}

////////////////////////////////////////////////////////////////////////
//                         STOP VSH MENU                              //
////////////////////////////////////////////////////////////////////////
static void stop_VSH_Menu(void)
{
  // menu off
  menu_running = 0;

  // unbind renderer and kill font-instance
  font_finalize();

  // free heap memory
  destroy_heap();

  // continue rsx rendering
  rsx_fifo_pause(0);

  // restart vsh pad
  start_stop_vsh_pad(1);

  sys_timer_usleep(100000);
}

////////////////////////////////////////////////////////////////////////
//                         MOUNT DEV_BLIND                            //
////////////////////////////////////////////////////////////////////////

static void mount_dev_blind(void)
{
	system_call_8(837, (uint64_t)(char*)"CELL_FS_IOS:BUILTIN_FLSH1", (uint64_t)(char*)"CELL_FS_FAT", (uint64_t)(char*)"/dev_blind", 0, 0, 0, 0, 0);
}

////////////////////////////////////////////////////////////////////////
//                        TOGGLE NORMAL/REBUG MODE                    //
////////////////////////////////////////////////////////////////////////

static void toggle_normal_rebug_mode(void)
{
	struct CellFsStat s;
	mount_dev_blind();

	if(cellFsStat((char*) VSH_MODULE_PATH "vsh.self.swp", &s)==CELL_FS_SUCCEEDED)
	{
		stop_VSH_Menu();
		vshtask_notify("Normal Mode detected!\r\nSwitch to REBUG Mode...");
		play_rco_sound("system_plugin", "snd_system_ok");
		sys_timer_sleep(1);

		if(cellFsStat((char*) VSH_ETC_PATH "index.dat.swp", &s)==CELL_FS_SUCCEEDED)
		{
			cellFsRename(VSH_ETC_PATH "index.dat", VSH_ETC_PATH "index.dat.nrm");
			cellFsRename(VSH_ETC_PATH "index.dat.swp", VSH_ETC_PATH "index.dat");
		}

		if(cellFsStat((char*) VSH_ETC_PATH "version.txt.swp", &s)==CELL_FS_SUCCEEDED)
		{
			cellFsRename(VSH_ETC_PATH "version.txt", VSH_ETC_PATH "version.txt.nrm");
			cellFsRename(VSH_ETC_PATH "version.txt.swp", VSH_ETC_PATH "version.txt");
		}

		cellFsRename(VSH_MODULE_PATH "vsh.self", VSH_MODULE_PATH "vsh.self.nrm");
		cellFsRename(VSH_MODULE_PATH "vsh.self.swp", VSH_MODULE_PATH "vsh.self");

		soft_reboot();
	}
	else
	if(cellFsStat((char*) VSH_MODULE_PATH "vsh.self.nrm", &s)==CELL_FS_SUCCEEDED)
	{
		stop_VSH_Menu();
		vshtask_notify("Rebug Mode detected!\r\nSwitch to Normal Mode...");
		play_rco_sound("system_plugin", "snd_system_ok");
		sys_timer_sleep(1);

		if(cellFsStat((char*) VSH_ETC_PATH "index.dat.nrm", &s)==CELL_FS_SUCCEEDED)
		{
			cellFsRename(VSH_ETC_PATH "index.dat", VSH_ETC_PATH "index.dat.swp");
			cellFsRename(VSH_ETC_PATH "index.dat.nrm", VSH_ETC_PATH "index.dat");
		}

		if(cellFsStat((char*) VSH_ETC_PATH "version.txt.nrm", &s)==CELL_FS_SUCCEEDED)
		{
			cellFsRename(VSH_ETC_PATH "version.txt", VSH_ETC_PATH "version.txt.swp");
			cellFsRename(VSH_ETC_PATH "version.txt.nrm", VSH_ETC_PATH "version.txt");
		}

		cellFsRename(VSH_MODULE_PATH "vsh.self", VSH_MODULE_PATH "vsh.self.swp");
		cellFsRename(VSH_MODULE_PATH "vsh.self.nrm", VSH_MODULE_PATH "vsh.self");

		soft_reboot();
	}
}

////////////////////////////////////////////////////////////////////////
//                       TOGGLE XMB MODE                              //
////////////////////////////////////////////////////////////////////////

static void toggle_xmb_mode(void)
{
    struct CellFsStat s;
	mount_dev_blind();

	if((cellFsStat((char*) VSH_MODULE_PATH "vsh.self.cexsp", &s)==CELL_FS_SUCCEEDED))
	{
		stop_VSH_Menu();
		vshtask_notify("Debug XMB detected!\r\nSwitch to Retail XMB...");
		play_rco_sound("system_plugin", "snd_system_ok");
		sys_timer_sleep(1);

		cellFsRename(VSH_MODULE_PATH "vsh.self", VSH_MODULE_PATH "vsh.self.dexsp");
		cellFsRename(VSH_MODULE_PATH "vsh.self.cexsp", VSH_MODULE_PATH "vsh.self");

		soft_reboot();
	}
	else
	if(cellFsStat((char*) VSH_MODULE_PATH "vsh.self.dexsp", &s)==CELL_FS_SUCCEEDED)
	{
		stop_VSH_Menu();
		vshtask_notify("Retail XMB detected!\r\nSwitch to Debug XMB...");
		play_rco_sound("system_plugin", "snd_system_ok");
		sys_timer_sleep(1);

		cellFsRename(VSH_MODULE_PATH "vsh.self", VSH_MODULE_PATH "vsh.self.cexsp");
		cellFsRename(VSH_MODULE_PATH "vsh.self.dexsp", VSH_MODULE_PATH "vsh.self");

		soft_reboot();
	}
}

////////////////////////////////////////////////////////////////////////
//                        TOGGLE DEBUG MENU                           //
////////////////////////////////////////////////////////////////////////

static void toggle_debug_menu(void)
{
    struct CellFsStat s;
	mount_dev_blind();

	if(cellFsStat((char*) VSH_MODULE_PATH "sysconf_plugin.sprx.dex", &s)==CELL_FS_SUCCEEDED)
	{

		stop_VSH_Menu();
		vshtask_notify("CEX QA Menu is active!\r\nSwitch to DEX Debug Menu...");
		play_rco_sound("system_plugin", "snd_system_ok");
		sys_timer_sleep(1);

		cellFsRename(VSH_MODULE_PATH "sysconf_plugin.sprx", VSH_MODULE_PATH "sysconf_plugin.sprx.cex");
		cellFsRename(VSH_MODULE_PATH "sysconf_plugin.sprx.dex", VSH_MODULE_PATH "sysconf_plugin.sprx");
	}
	else
	if(cellFsStat((char*) VSH_MODULE_PATH "sysconf_plugin.sprx.cex", &s)==CELL_FS_SUCCEEDED)
	{
		stop_VSH_Menu();
		vshtask_notify("DEX Debug Menu is active!\r\nSwitch to CEX QA Menu...");
		play_rco_sound("system_plugin", "snd_system_ok");
		sys_timer_sleep(1);

		cellFsRename(VSH_MODULE_PATH "sysconf_plugin.sprx", VSH_MODULE_PATH "sysconf_plugin.sprx.dex");
		cellFsRename(VSH_MODULE_PATH "sysconf_plugin.sprx.cex", VSH_MODULE_PATH "sysconf_plugin.sprx");
	}
	sys_timer_sleep(1);
	{system_call_3(838, (uint64_t)(char*)"/dev_blind", 0, 1);}
}

////////////////////////////////////////////////////////////////////////
//                        DISABLE COBRA STAGE2                        //
////////////////////////////////////////////////////////////////////////

static void disable_cobra_stage2(void)
{
	stop_VSH_Menu();

	if(is_cobra_based())
	{
		mount_dev_blind();

		vshtask_notify("Cobra Mode detected!\r\nDisabling Cobra stage2...");
		play_rco_sound("system_plugin", "snd_system_ok");
		sys_timer_sleep(1);

		cellFsRename("/dev_blind/rebug/cobra/stage2.cex", "/dev_blind/rebug/cobra/stage2.cex.bak");
		cellFsRename("/dev_blind/rebug/cobra/stage2.dex", "/dev_blind/rebug/cobra/stage2.dex.bak");
		cellFsRename("/dev_blind/sys/stage2.bin", "/dev_blind/sys/stage2_disabled.bin");

		soft_reboot();
	}
	else
	{
		vshtask_notify("Cobra Mode was NOT detected!");
		play_rco_sound("system_plugin", "snd_system_ok");
		sys_timer_sleep(1);
	}
}

////////////////////////////////////////////////////////////////////////
//                        DISABLE Webman                              //
////////////////////////////////////////////////////////////////////////

static void disable_webman(void)
{
	struct CellFsStat s;

	stop_VSH_Menu();

	if(cellFsStat("/dev_flash/vsh/module/webftp_server.sprx", &s)==CELL_FS_SUCCEEDED)
	{
		mount_dev_blind();
		vshtask_notify("webMAN MOD is Enabled!\r\nNow will be Disabled...");
		play_rco_sound("system_plugin", "snd_system_ok");
		sys_timer_sleep(1);

		cellFsRename("/dev_blind/vsh/module/webftp_server.sprx", "/dev_blind/vsh/module/webftp_server.sprx.vsh");
		soft_reboot();
	}
	else if(cellFsStat("/dev_blind/vsh/module/webftp_server.sprx.vsh", &s)==CELL_FS_SUCCEEDED)
	{
		mount_dev_blind();
		vshtask_notify("webMAN MOD Disabled!\r\nNow will be Enabled...");
		play_rco_sound("system_plugin", "snd_system_ok");
		sys_timer_sleep(1);

		cellFsRename("/dev_blind/vsh/module/webftp_server.sprx.vsh", "/dev_blind/vsh/module/webftp_server.sprx");
		soft_reboot();
	}
	else
	{
		vshtask_notify("webMAN MOD was not found on /dev_flash");
		play_rco_sound("system_plugin", "snd_system_ok");
		sys_timer_sleep(1);
	}
}

static void recovery_mode(void)
{
	#define SC_UPDATE_MANAGER_IF				863
	#define UPDATE_MGR_PACKET_ID_READ_EPROM		0x600B
	#define UPDATE_MGR_PACKET_ID_WRITE_EPROM	0x600C
	#define RECOVER_MODE_FLAG_OFFSET			0x48C61

	stop_VSH_Menu();
	vshtask_notify("Now PS3 will be restarted in Recovery Mode");
	play_rco_sound("system_plugin", "snd_system_ok");
	sys_timer_sleep(1);

   {system_call_7(SC_UPDATE_MANAGER_IF, UPDATE_MGR_PACKET_ID_WRITE_EPROM, RECOVER_MODE_FLAG_OFFSET, 0x00, 0, 0, 0, 0);} // set recovery mode
	hard_reboot();
}

////////////////////////////////////////////////////////////////////////
//                            BLITTING                                //
////////////////////////////////////////////////////////////////////////
static uint16_t line = 0;           // current line into menu, init 0 (Menu Entry 1)
#define MAX_MENU     11
#define MAX_MENU2    8

static uint8_t view = MAIN_MENU;

static uint8_t entry_mode[MAX_MENU] = {0, 0, 0, 0, 0, 0, 0, 0};

static char entry_str[2][MAX_MENU][32] = {
                                          {
                                           "0: Unmount Game",
                                           "1: Mount /net0",
                                           "2: Fan (+)",
                                           "3: Refresh XML",
                                           "4: Toggle gameDATA",
                                           "5: Backup Disc to HDD",
                                           "6: Screenshot (XMB)",
                                           "7: File Manager",
                                           "8: webMAN Setup",
                                           "9: Shutdown PS3",
                                           "A: Reboot PS3 (soft)",
                                          },
                                          {
                                           "0: Unload VSH Menu",
                                           "1: Toggle Rebug Mode",
                                           "2: Toggle XMB Mode",
                                           "3: Toggle Debug Menu",
                                           "4: Disable Cobra",
                                           "5: Disable webMAN MOD",
                                           "6: Recovery Mode",
                                           "7: Startup Message : ON",
                                          }
                                        };

////////////////////////////////////////////////////////////////////////
//               EXECUTE ACTION DEPENDING LINE SELECTED               //
////////////////////////////////////////////////////////////////////////
static uint8_t fan_mode = 0;

static void return_to_xmb(void)
{
    sys_timer_sleep(1);
    stop_VSH_Menu();
    view = MAIN_MENU; if(view) line = 0;
}

static void do_main_menu_action(void)
{
  switch(line)
  {
    case 0:
      buzzer(1);
      send_wm_request((char*)"GET /mount_ps3/unmount");

      //if(entry_mode[line]) wait_for_request(); else
      sys_timer_sleep(1);

      if(entry_mode[line]==2) eject_insert(0, 1); else if(entry_mode[line]==1) eject_insert(1, 0);

      stop_VSH_Menu();

      if(entry_mode[line])
      {
          entry_mode[line]=(entry_mode[line]==2) ? 1 : 2;
          strcpy(entry_str[view][line], ((entry_mode[line] == 2) ? "0: Insert Disc\0" : (entry_mode[line] == 1) ? "0: Eject Disc\0" : "0: Unmount Game"));
      }
      return;
    case 1:
      if(entry_mode[line]==0) {send_wm_request((char*)"GET /mount_ps3/net0");}
      if(entry_mode[line]==1) {send_wm_request((char*)"GET /mount_ps3/net1");}
      if(entry_mode[line]==2) {send_wm_request((char*)"GET /mount_ps3/net2");}

      break;
    case 2:
      // get fan_mode (0 = dynamic / 1 = manual)
      if(line<3)
      {
        uint8_t wmconfig[sizeof(WebmanCfg)];
        WebmanCfg *webman_config = (WebmanCfg*) wmconfig;

        int fd=0;
        if(cellFsOpen((char*)"/dev_hdd0/tmp/wmconfig.bin", CELL_FS_O_RDONLY, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
        {

           cellFsRead(fd, (void *)wmconfig, sizeof(WebmanCfg), 0);
           cellFsClose(fd);

           fan_mode = (webman_config->temp0>0); // manual
        }
      }

      if(entry_mode[line]==(fan_mode ? 1 : 0)) {send_wm_request((char*)"GET /cpursx.ps3?dn"); buzzer(1);}
      if(entry_mode[line]==(fan_mode ? 0 : 1)) {send_wm_request((char*)"GET /cpursx.ps3?up"); buzzer(1);}

      if(entry_mode[line]==2) {send_wm_request((char*)"GET /cpursx.ps3?mode"); buzzer(3); entry_mode[line]=3; strcpy(entry_str[view][line], "2: System Info"); fan_mode = fan_mode ? 0 : 1;} else
      if(entry_mode[line]==3) {send_wm_request((char*)"GET /popup.ps3"); return_to_xmb();}

      play_rco_sound("system_plugin", "snd_system_ok");
      return;
    case 3:
      send_wm_request((char*)"GET /refresh.ps3");

      break;
    case 4:
      send_wm_request((char*)"GET /extgd.ps3");

      break;
    case 5:
      send_wm_request((char*)"GET /copy.ps3/dev_bdvd");

      break;
    case 6:
      buzzer(1);
      screenshot(entry_mode[line]); // mode = 0 (XMB only), 1 (XMB + menu)
      stop_VSH_Menu();

      play_rco_sound("system_plugin", "snd_system_ok");

      return;
    case 7:
      send_wm_request((char*)"GET /browser.ps3/");

      break;
    case 8:
      send_wm_request((char*)"GET /browser.ps3/setup.ps3");

      break;
    case 9:
      return_to_xmb();

      buzzer(2);
      shutdown_system();
      return;
    case 0xA:
      return_to_xmb();

      buzzer(1);
      if(entry_mode[line]) hard_reboot(); else soft_reboot();
      return;
  }

  // return to XMB
  return_to_xmb();

  play_rco_sound("system_plugin", "snd_system_ok");
}

static void do_rebug_menu_action(void)
{
  struct CellFsStat s;

  buzzer(1);

  switch(line)
  {
    case 0:

      if(cellFsStat("/dev_hdd0/plugins/wm_vsh_menu.sprx", &s) == CELL_FS_SUCCEEDED)
          send_wm_request((char*)"GET /unloadprx.ps3/dev_hdd0/plugins/wm_vsh_menu.sprx");
      else
          return;

      break;
    case 1:
      toggle_normal_rebug_mode();
      return;

    case 2:
      toggle_xmb_mode();
      return;

    case 3:
      toggle_debug_menu();
      return;

    case 4:
      disable_cobra_stage2();
      return;

    case 5:
      disable_webman();
      return;

    case 6:
      recovery_mode();
      return;

    case 7:
      config->dnotify = config->dnotify ? 0 : 1;
      strcpy(entry_str[view][line], (config->dnotify) ? "7: Startup Message : OFF\0" : "7: Startup Message : ON\0");

      // save config
      int fd=0;
      if(cellFsOpen((char*)"/dev_hdd0/tmp/wm_vsh_menu.cfg", CELL_FS_O_CREAT|CELL_FS_O_WRONLY, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
      {
         cellFsWrite(fd, (void *)vsh_menu_config, sizeof(vsh_menu_Cfg), 0);
         cellFsClose(fd);
      }
      return;
  }

  // return to XMB
  return_to_xmb();

  play_rco_sound("system_plugin", "snd_system_ok");
}

static void set_initial_file(void);
static void sort_files(void);

static void do_file_manager_action(uint32_t curpad)
{
  uint16_t old_nitems = nitems, old_line = line;

  int fd;

  // delete file
  if((curpad & (PAD_L2 | PAD_SQUARE))==(PAD_L2 | PAD_SQUARE))
  {
      sprintf(tempstr, "%s/%s", curdir, items[cur_item]);
      del(tempstr, true);
  }

  // cut / copy /paste
  else if(curpad & PAD_SQUARE)
  {
      if(clipboard_mode == 0 && strcmp(items[cur_item], "..") == 0) return;

      char url[MAX_PATH_LEN];
      if(clipboard_mode)
          sprintf(tempstr, "%s", curdir);
      else
          sprintf(tempstr, "%s/%s", curdir, items[cur_item]);

      urlenc(url, tempstr);

      if(clipboard_mode)
          {sprintf(item_size, "<Paste>"); sprintf(tempstr, "GET /paste.ps3%s", curdir); clipboard_mode = 0;}
      else if(curpad & PAD_R2)
          {sprintf(item_size, "<Cut>"); sprintf(tempstr, "GET /cut.ps3%s", url); clipboard_mode = 2;}
      else
          {sprintf(item_size, "<Copy>"); sprintf(tempstr, "GET /cpy.ps3%s", url); clipboard_mode = 1;}

      send_wm_request(tempstr);
      sys_timer_sleep(1);

      tempstr[0]=0;

      if(clipboard_mode) return;
  }

  // do file action
  else if(curpad & (PAD_CROSS | PAD_START | PAD_TRIANGLE))
  {
      int ext_offset = strlen(items[cur_item]); if(ext_offset<0) ext_offset = 0;

      // go folder up
      if(curpad & PAD_TRIANGLE)
      {
          if(!strcmp(items[cur_item], "..")) sprintf(curdir, "/");

          cur_item = 0;
          char *p = strrchr(curdir, '/'); p[0]=0;
          if(strlen(curdir)==0) sprintf(curdir, "/");
      }
      else

      // open file in browser
      if(strcasestr(items[cur_item] + ext_offset, ".png") || strcasestr(items[cur_item] + ext_offset, ".bmp") || strcasestr(items[cur_item] + ext_offset, ".jpg") || strcasestr(items[cur_item] + ext_offset, ".txt") || strcasestr(items[cur_item] + ext_offset, ".log") || strcasestr(items[cur_item] + ext_offset, ".htm"))
      {
          char url[MAX_PATH_LEN];
          sprintf(tempstr, "%s/%s", curdir, items[cur_item]);
          urlenc(url, tempstr);
          if(curpad & PAD_CROSS)
              sprintf(tempstr, "GET /browser.ps3%s", url);
          else
              sprintf(tempstr, "GET /copy.ps3%s", url);
          send_wm_request(tempstr);
          return_to_xmb();
          return;
      }
      else

      // copy file from hdd0->usb000 / usb000->hdd0
      if(!items_isdir[cur_item] && (strcasestr(items[cur_item] + ext_offset, ".pkg") || strstr(items[cur_item] + ext_offset, ".p3t") || strcasestr(items[cur_item] + ext_offset, ".mp3") || strcasestr(items[cur_item] + ext_offset, ".mp4") || strcasestr(items[cur_item] + ext_offset, ".mkv") || strcasestr(items[cur_item] + ext_offset, ".avi") || strstr(items[cur_item] + ext_offset, "edat") || strstr(items[cur_item] + ext_offset, ".rco") || strstr(items[cur_item] + ext_offset, ".qrc") || strstr(items[cur_item] + ext_offset, "sprx")  || strstr(items[cur_item], "coldboot")))
      {
          char url[MAX_PATH_LEN];
          sprintf(tempstr, "%s/%s", curdir, items[cur_item]);
          urlenc(url, tempstr);
          sprintf(tempstr, "GET /copy.ps3%s", url);
          send_wm_request(tempstr);
          return_to_xmb();
          return;
      }
      else

      // mount game
      if((!items_isdir[cur_item] && (strcasestr(items[cur_item] + ext_offset, ".iso") || strcasestr(items[cur_item] + ext_offset, ".img") || strcasestr(items[cur_item] + ext_offset, ".mdf") || strcasestr(items[cur_item] + ext_offset, ".cue") || strstr(items[cur_item] + ext_offset, ".bin"))) || (items_isdir[cur_item] && (curpad & PAD_START)) || (!items_isdir[cur_item] && (strstr(curdir, "/PS3_GAME")!=NULL || strcmp(items[cur_item], "PS3_DISC.SFB")==0)))
      {
          if(strcmp(items[cur_item], "PS3_DISC.SFB")==0)
              sprintf(tempstr, "%s", curdir);
          else
              sprintf(tempstr, "%s/%s", curdir, items[cur_item]);

          char url[MAX_PATH_LEN];
          urlenc(url, tempstr);

          if(strstr(curdir, "/dev_hdd0/game"))
              sprintf(tempstr, "GET /fixgame.ps3%s", url);
          else if(strstr(url, "/dev_bdvd") || strstr(url, "/app_home"))
              sprintf(tempstr, "GET /play.ps3");
          else if(strstr(url, "/dev_hdd0/home"))
              sprintf(tempstr, "GET /copy.ps3%s", url);
          else
              sprintf(tempstr, "GET /mount_ps3%s", url);
          send_wm_request(tempstr);
          return_to_xmb();
          return;
      }
      else

      // change folder
      if(items_isdir[cur_item])
      {
          if(!strcmp(items[cur_item], ".."))
          {
              char *p = strrchr(curdir, '/'); p[0]=0;
              if(strlen(curdir)==0) sprintf(curdir, "/");
          }
          else
          {
              if(strlen(curdir)>1) strcat(curdir, "/");
              strcat(curdir, items[cur_item]);
          }
      }

  }

  // set title offset
  if(strlen(curdir)<38) curdir_offset = 0; else curdir_offset = strlen(curdir) - 38;

  // clear list
  for(int i=0;i<MAX_ITEMS;i++) {items[i][0] = 0; items_isdir[i] = 0;}

  nitems = line = 0;

  // list files
  if(cellFsOpendir(curdir, &fd) == CELL_FS_SUCCEEDED)
  {
      CellFsDirent dir; uint64_t read = sizeof(CellFsDirent);

      while(!cellFsReaddir(fd, &dir, &read))
      {
          if(!read || nitems>=MAX_ITEMS) break;
          if(dir.d_name[0]=='.' && dir.d_name[1]==0) continue;

          sprintf(items[nitems], "/%s", dir.d_name);

          sprintf(tempstr, "%s/%s", curdir, dir.d_name);
          items_isdir[nitems] = isDir(tempstr);

          if(items_isdir[nitems]) items[nitems][0]=' ';

          nitems++;
      }
      cellFsClosedir(fd);
  }

  // set initial file after delete or read folder
  if((curpad & (PAD_L2 | PAD_SQUARE))==(PAD_L2 | PAD_SQUARE))
  {
      line = old_line; old_nitems--;
      if(line>=old_nitems) {if(line>0) line--; else line = 0;}
  }
  else
      line = (nitems>1) ? 1 : 0;

  // sort files
  sort_files();

  set_initial_file();

  tempstr[0]=0;
}

static void do_plugins_manager_action(uint32_t curpad)
{
  nitems = line = 0;

  int fd; char paths[10][48] = {"/dev_hdd0", "/dev_hdd0/plugins", "/dev_hdd0/plugins/ps3xpad", "/dev_hdd0/plugins/ps3_menu", "/dev_usb000", "/dev_usb001", "/dev_hdd0/game/UPDWEBMOD/USRDIR", "/dev_hdd0/game/UPDWEBMOD/USRDIR/official", "/dev_hdd0/tmp"};

  // clear list
  for(int i=0;i<MAX_ITEMS;i++) {items[i][0] = 0; items_isdir[i] = 0;}

  nitems = line = 0;

  // list plugins
  for(uint8_t i = 0; i < 9; i++)
  if(cellFsOpendir(paths[i], &fd) == CELL_FS_SUCCEEDED)
  {
      CellFsDirent dir; uint64_t read = sizeof(CellFsDirent);

      while(!cellFsReaddir(fd, &dir, &read))
      {
          if(!read || nitems>=MAX_ITEMS) break;
          if(strstr(dir.d_name, ".sprx"))
          {
              dir.d_name[MAX_PATH_LEN-1]=0;
              sprintf(items[nitems], "%s/%s", paths[i], dir.d_name);

              items_isdir[nitems] = get_vsh_plugin_slot_by_name(items[nitems], false);

              sprintf(items[nitems], "/%s/%s", paths[i], dir.d_name);

              if(items_isdir[nitems]) items[nitems][0]=' ';

              nitems++;
          }
      }
      cellFsClosedir(fd);
  }

  // sort files
  sort_files();

  set_initial_file();

  // save boot_plugins.txt
  if(curpad & PAD_START)
  {
      int fd;
      if(cellFsOpen("/dev_hdd0/boot_plugins.txt", CELL_FS_O_CREAT|CELL_FS_O_WRONLY|CELL_FS_O_TRUNC, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
      {
          for (int i = 0; i < nitems; i++)
          {
              if(items_isdir[i])
              {
                  sprintf(tempstr, "%s\n", items[i]);
                  cellFsWrite(fd, (void *)tempstr, strlen(tempstr), 0);
              }
          }
          cellFsClose(fd);
      }

      return_to_xmb();
      vshtask_notify("Saved /dev_hdd0/boot_plugins.txt");
  }

  tempstr[0]=0;
}

static void sort_files(void)
{
  // sort file entries
  uint16_t n, m; char swap[MAX_PATH_LEN]; uint8_t s;
  for(n=0; n<(nitems-1); n++)
      for(m=(n+1); m<nitems; m++)
          if(strcasecmp(items[n], items[m])>0)
          {
              strcpy(swap, items[n]);
              strcpy(items[n], items[m]);
              strcpy(items[m], swap);

              s=items_isdir[n];
              items_isdir[n]=items_isdir[m];
              items_isdir[m]=s;
          }

  // remove sort prefix for directories
  for(n = 0; n < nitems; n++) memcpy(items[n], items[n]+1, MAX_PATH_LEN-1);
}

static void set_initial_file(void)
{
  // set first option
  if(strcmp(items[0], "..") == 0 && nitems>1) cur_item = 1; else cur_item = 0;

  if(!curdir[1]) for(int i=1; i<nitems; i++) if(!strcmp(items[i], "dev_hdd0")) {cur_item = line=i; break;}

  // show initial icon0
  has_icon0 = 0;
  sprintf(tempstr, "%s/ICON0.PNG", curdir);
  if(file_exists(tempstr)) {has_icon0 = 2; load_png_bitmap(1, tempstr);} else
  {
      sprintf(tempstr, "%s/ICON2.PNG", curdir);
      if(file_exists(tempstr)) {has_icon0 = 2; load_png_bitmap(1, tempstr);}
  }

  if(!has_icon0)
  {
      sprintf(tempstr, "%s/PS3_GAME/ICON0.PNG", curdir);
      if(file_exists(tempstr)) {has_icon0 = 2; load_png_bitmap(1, tempstr);}
  }

  if(!has_icon0 && items_isdir[cur_item])
  {

      sprintf(tempstr, "%s/%s/ICON0.PNG", curdir, items[cur_item]);
      if(file_exists(tempstr)) {has_icon0 = 1; load_png_bitmap(1, tempstr);} else
      {
          sprintf(tempstr, "%s/%s/PS3_GAME/ICON0.PNG", curdir, items[cur_item]);
          if(file_exists(tempstr)) {has_icon0 = 1; load_png_bitmap(1, tempstr);} else
          {
              sprintf(tempstr, "%s/%s/../ICON0.PNG", curdir, items[cur_item]);
              if(file_exists(tempstr)) {has_icon0 = 2; load_png_bitmap(1, tempstr);} else
              {
                  sprintf(tempstr, "%s/%s/../../ICON0.PNG", curdir, items[cur_item]);
                  if(file_exists(tempstr)) {has_icon0 = 2; load_png_bitmap(1, tempstr);}
              }
          }
      }
  }
}


////////////////////////////////////////////////////////////////////////
//                             DRAW A FRAME                           //
////////////////////////////////////////////////////////////////////////
static uint32_t frame = 0;

static void draw_background_and_title(void)
{
  // all 32bit colors are ARGB, the framebuffer format
  set_background_color(0xEE333333);  // dark gray, semitransparent
  set_foreground_color(WHITE);       // white, opac

  if(ctx.png[0].w == 0 || ctx.png[0].h == 0)
  {
      // fill background with background color
      draw_background();
  }
  else
  {
      // draw background from png
      draw_png(0, 0, 0, 0, 0, 720, 400);
  }

  // draw logo from png
  draw_png(0, 648, 336, 576, 400, 64, 64);

  if(has_icon0 && (view == FILE_MANAGER && clipboard_mode == 0))
     draw_png(1, 18, 208, 0, 0, ctx.png[1].w, ctx.png[1].h);

  // print headline string, center(x = -1)
  set_font(22.f, 23.f, 1.f, 1); print_text(-1, 8, ( (view == REBUG_MENU)      ? "VSH Menu for Rebug"   :
                                                    (view == FILE_MANAGER)    ? curdir + curdir_offset :
                                                    (view == PLUGINS_MANAGER) ? "Plugins Manager"      :
                                                                               "VSH Menu for webMAN") );
  set_font(14.f, 14.f, 1.f, 1); print_text(650, 8, "v1.06");
}

static void draw_menu_options(void)
{
  int32_t i, selected;
  const int32_t count = 4;                      // list count  .. tip: if less than entries nbr, will scroll :)
  uint32_t color = 0, selcolor = 0;

  set_font(20.f, 20.f, 1.f, 1);

  selcolor = (frame & 0x4) ? WHITE : GREEN;

  // draw menu entry list
  for(i = 0; i < count; i++)
  {
    if(line < count)
    {
        selected = 0;
        color = (i == line) ? selcolor : WHITE;
    }
    else
    {
        selected = line - (count - 1);
        color = (i == (count - 1)) ? selcolor : WHITE;
    }

    if(view == FILE_MANAGER || view == PLUGINS_MANAGER)
    {
        if( (selected + i) >= nitems ) break;

        if(color == WHITE && items_isdir[selected + i]) color = YELLOW;
        if(color == GREEN) cur_item = selected + i;
    }

    set_foreground_color(color);

    if(view == FILE_MANAGER)
        print_text(20, 40 + (LINE_HEIGHT * (i + 1)), items[selected + i]);
    else if(view == PLUGINS_MANAGER)
        print_text(20, 40 + (LINE_HEIGHT * (i + 1)), strrchr(items[selected + i], '/') + 1 );
    else
        print_text(20, 40 + (LINE_HEIGHT * (i + 1)), entry_str[view][selected + i]);

    selected++;
  }

  if (i < MAX_MENU)
    if (line > count - 1)    draw_png(0, 20, 56, 688, 400, 16, 8);   // UP arrow
    if (line < MAX_MENU - 1) draw_png(0, 20, 177, 688, 408, 16, 8);  // DOWN arrow
}

static void draw_legend(void)
{
  bool no_button = (ctx.png[0].w == 0 || ctx.png[0].h == 0);

  // draw command buttons
  set_foreground_color(GRAY);

  set_font(20.f, 17.f, 1.f, 1);

  // draw 1st button
  if(no_button) ;

  else if(view == FILE_MANAGER)
  {
      // draw start button
      draw_png(0, 522, 230, 320, 400, 32, 32);
      print_text(560, 234, items_isdir[cur_item] ? " Mount" : " Copy");
  }
  else if(view == PLUGINS_MANAGER)
  {
      // draw start button
      draw_png(0, 522, 230, 320, 400, 32, 32);
      print_text(560, 234, " Save");
  }
  else if(view == MAIN_MENU)
  {
      // draw up-down button
      draw_png(0, 522, 230, 128 + ((line<3||line==6||line==10) ? 64 : 0), 432, 32, 32);
      print_text(560, 234, " Choose");
  }
  else if(view == REBUG_MENU)
  {
      // draw up-down button
      draw_png(0, 522, 230, 128 + ((line==7) ? 64 : 0), 432, 32, 32);
      print_text(560, 234, " Choose");
  }

  // draw X button
  if(no_button) print_text(530, 266, "X"); else draw_png(0, 522, 262, 0, 400, 32, 32);

  // print X legend
  if(view == FILE_MANAGER)
  {
      int ext_offset = strlen(items[cur_item]) - 4; if(ext_offset < 0) ext_offset = 0;

      if(strcasestr(items[cur_item] + ext_offset, ".png") ||
         strcasestr(items[cur_item] + ext_offset, ".bmp") ||
         strcasestr(items[cur_item] + ext_offset, ".jpg") ||
         strcasestr(items[cur_item] + ext_offset, ".sht") ||
         strcasestr(items[cur_item] + ext_offset, ".txt") ||
         strcasestr(items[cur_item] + ext_offset, ".log") ||
         strcasestr(items[cur_item] + ext_offset, ".htm"))
                                           print_text(570, 266, "View");
      else
      if((!items_isdir[cur_item] && (
          strcasestr(items[cur_item] + ext_offset, ".pkg") ||
         strstr(items[cur_item] + ext_offset, ".p3t")     ||
         strcasestr(items[cur_item] + ext_offset, ".mp3") ||
         strcasestr(items[cur_item] + ext_offset, ".mp4") ||
         strcasestr(items[cur_item] + ext_offset, ".mkv") ||
         strcasestr(items[cur_item] + ext_offset, ".avi") ||
         strstr(items[cur_item] + ext_offset, ".rco")     ||
         strstr(items[cur_item] + ext_offset, ".qrc")     ||
         strstr(items[cur_item] + ext_offset, "sprx")     ||
         strstr(items[cur_item] + ext_offset, "edat")     ||
         strstr(items[cur_item], "coldboot") ) )          ||
         strstr(curdir, "/dev_hdd0/home"))
                                           print_text(570, 266, "Copy");
      else
      if((!items_isdir[cur_item] && (
          strcasestr(items[cur_item], ".iso") ||
          strcasestr(items[cur_item], ".img") ||
          strcasestr(items[cur_item], ".mdf") ||
          strcasestr(items[cur_item], ".cue") ||
          strstr(items[cur_item], ".bin") ) ) ||
         (!items_isdir[cur_item] && strstr(curdir, "/PS3_GAME")!=NULL) ||
         strcmp(items[cur_item], "PS3_DISC.SFB")==0)
                                           print_text(570, 266, "Mount");
      else
                                           print_text(570, 266, "Select");  // draw X button

      // draw file size / clipboard operation
      if(!items_isdir[cur_item] || clipboard_mode)
                                           print_text(410, 208, item_size);
  }
  else if(view == PLUGINS_MANAGER)
  {
                                           print_text(570, 266, items_isdir[cur_item] ? " Unload" : " Load");  // draw X button
  }
  else
  {
                                           print_text(570, 266, "Select");  // draw X button
  }

  if(no_button) return;

  draw_png(0, 522, 294, 416, 400, 32, 32); print_text(570, 298, "Exit");    // draw select button

  draw_png(0, 522, 326, 128, 400, 32, 32); print_text(570, 330, "Mode");    // draw L1 button
}

static void draw_system_info(void)
{
  set_foreground_color(BLUE);

  // draw firmware version info
  print_text(352, 30 + (LINE_HEIGHT * 1), cfw_str);

  // draw network info
  print_text(352, 30 + (LINE_HEIGHT * 2.5), netstr);

  // draw temperatures
  if(frame == 1 || frame == 33 || !tempstr[0])
  {
      uint32_t cpu_temp_c = 0, rsx_temp_c = 0, cpu_temp_f = 0, rsx_temp_f = 0, higher_temp;

      get_temperature(0, &cpu_temp_c);
      get_temperature(1, &rsx_temp_c);
      cpu_temp_c = cpu_temp_c >> 24;
      rsx_temp_c = rsx_temp_c >> 24;
      cpu_temp_f = (1.8f * (float)cpu_temp_c + 32.f);
      rsx_temp_f = (1.8f * (float)rsx_temp_c + 32.f);

      sprintf(tempstr, "CPU :  %i°C  •  %i°F\r\nRSX :  %i°C  •  %i°F", cpu_temp_c, cpu_temp_f, rsx_temp_c, rsx_temp_f);

      if (cpu_temp_c > rsx_temp_c) higher_temp = cpu_temp_c;
      else higher_temp = rsx_temp_c;

           if (higher_temp < 50)                       t_icon_X = 224;  // blue icon
      else if (higher_temp >= 50 && higher_temp <= 65) t_icon_X = 256;  // green icon
      else if (higher_temp >  65 && higher_temp <  75) t_icon_X = 288;  // yellow icon
      else                                             t_icon_X = 320;  // red icon
  }

  set_font(24.f, 17.f, 1.f, 1);

  draw_png(0, 355, 38 + (LINE_HEIGHT * 5), t_icon_X, 464, 32, 32);
  print_text(395, 30 + (LINE_HEIGHT * 5), tempstr);
}

static void draw_drives_info(void)
{
  set_font(20.f, 17.f, 1.f, 1);
  set_foreground_color(YELLOW);
  print_text(20, 208, "Available free space on device(s):");

  int fd;

  //draw drives info
  if((frame == 1) && cellFsOpendir("/", &fd) == CELL_FS_SUCCEEDED)
  {
    char drivepath[32], freeSizeStr[32], devSizeStr[32];
    uint64_t read, freeSize, devSize;
    CellFsDirent dir;

    int8_t j; for(j = 0; j < 6; j++) {memset(drivestr[j], 0, 64); drive_type[j] = 0;} j = 0;

    read = sizeof(CellFsDirent);
    while(!cellFsReaddir(fd, &dir, &read))
    {
      if(!read) break;

      if((strncmp("dev_hdd",   dir.d_name, 7) != 0) &&
         (strncmp("dev_usb",   dir.d_name, 7) != 0) &&
         (strncmp("dev_sd",    dir.d_name, 6) != 0) &&
         (strncmp("dev_ms",    dir.d_name, 6) != 0) &&
         (strncmp("dev_cf",    dir.d_name, 6) != 0) &&
         (strncmp("dev_blind", dir.d_name, 9) != 0))
        continue;

      sprintf(drivepath, "/%s", dir.d_name);

      system_call_3(840, (uint64_t)(uint32_t)drivepath, (uint64_t)(uint32_t)&devSize, (uint64_t)(uint32_t)&freeSize);

      if (freeSize < 1073741824) sprintf(freeSizeStr, "%.2f MB", (double) (freeSize / 1048576.00f));
      else  sprintf(freeSizeStr, "%.2f GB", (double) (freeSize / 1073741824.00f));
      if (devSize < 1073741824) sprintf(devSizeStr, "%.2f MB", (double) (devSize / 1048576.00f));
      else  sprintf(devSizeStr, "%.2f GB", (double) (devSize / 1073741824.00f));

      sprintf(drivestr[j], "%s :  %s / %s", drivepath, freeSizeStr, devSizeStr);

      if (strncmp("dev_hdd", dir.d_name, 7) == 0)
        drive_type[j] = 1;
      else if (strncmp("dev_usb", dir.d_name, 7) == 0)
        drive_type[j] = 2;
      else if (strncmp("dev_blind", dir.d_name, 9) == 0 )
        drive_type[j] = 3;
      else if ((strncmp("dev_sd", dir.d_name, 6) == 0 ) || (strncmp("dev_ms", dir.d_name, 6) == 0 ) ||  (strncmp("dev_cf", dir.d_name, 6) == 0 ))
        drive_type[j] = 4;

      j++; if(j > 5) break;
    }

    cellFsClosedir(fd);
  }

  for(int8_t j = 0; j < 6; j++)
  {
        if(drive_type[j]) {draw_png(0, 25, 230 + (26 * j), 32 + (32 * drive_type[j]), 464, 32, 32); print_text(60, 235 + (26 * j), drivestr[j]);}
  }

  //...
}
static void draw_frame(void)
{
  draw_background_and_title();

  draw_menu_options();

  draw_legend();

  if(view != FILE_MANAGER)
  {
      draw_system_info();
  }
  else if(has_icon0 && clipboard_mode == 0) return;

  draw_drives_info();
}

static void change_current_folder(uint32_t curpad)
{
  if(curpad & PAD_LEFT)  if(cdir>0) cdir--;
  if(curpad & PAD_RIGHT) cdir++;

  while(true)
  {
      if(cdir== 0) sprintf(curdir, "/");
      if(cdir== 1) sprintf(curdir, "/dev_hdd0");
      if(cdir== 2) sprintf(curdir, "/dev_hdd0/GAMES");
      if(cdir== 3) sprintf(curdir, "/dev_hdd0/GAMEZ");
      if(cdir== 4) sprintf(curdir, "/dev_hdd0/PS3ISO");
      if(cdir== 5) sprintf(curdir, "/dev_hdd0/PS2ISO");
      if(cdir== 6) sprintf(curdir, "/dev_hdd0/PSXISO");
      if(cdir== 7) sprintf(curdir, "/dev_hdd0/PSPISO");
      if(cdir== 8) sprintf(curdir, "/dev_hdd0/packages");
      if(cdir== 9) sprintf(curdir, "/dev_hdd0/plugins");
      if(cdir==10) sprintf(curdir, "/dev_hdd0/BDISO");
      if(cdir==11) sprintf(curdir, "/dev_hdd0/DVDISO");
      if(cdir==12) sprintf(curdir, "/dev_hdd0/game/BLES80608/USRDIR");
      if(cdir==13) sprintf(curdir, "/dev_usb001");
      if(cdir==14) sprintf(curdir, "/dev_usb001/GAMES");
      if(cdir==15) sprintf(curdir, "/dev_usb001/PS3ISO");
      if(cdir==16) sprintf(curdir, "/dev_usb000/PS3ISO");
      if(cdir==17) sprintf(curdir, "/dev_usb000/GAMES");
      if(cdir==18) sprintf(curdir, "/dev_usb000");

      if(isDir(curdir)) {curpad = REFRESH_DIR; break;}
      if(curpad & PAD_LEFT) {if(cdir>0) cdir--;} else cdir++;
      if(cdir > 18) cdir=0;
  }

  do_file_manager_action(REFRESH_DIR);
}

static void change_main_menu_options(uint32_t curpad)
{
  if(curpad & PAD_RIGHT)  ++entry_mode[line];
  else
  if(entry_mode[line]==0) entry_mode[line] = ((line==2) ? 3 : (line<=1) ? 2 : 1);
  else
                          --entry_mode[line];

  if(entry_mode[line]>((line==2) ? 3 : (line<=1) ? 2 : 1)) entry_mode[line]=0;

  switch (line)
  {
   case 0x0: strcpy(entry_str[view][line], ((entry_mode[line] == 1) ? "0: Eject Disc\0"   : (entry_mode[line] == 2) ? "0: Insert Disc\0" : "0: Unmount Game\0")); break;
   case 0x1: strcpy(entry_str[view][line], ((entry_mode[line] == 1) ? "1: Mount /net1"    : (entry_mode[line] == 2) ? "1: Mount /net2"   : "1: Mount /net0"));  break;
   case 0x2: strcpy(entry_str[view][line], ((entry_mode[line] == 1) ? "2: Fan (-)\0"      : (entry_mode[line] == 2) ? "2: Fan Mode\0"    : (entry_mode[line] == 3) ? "2: System Info\0" : "2: Fan (+)\0")); break;
   case 0x6: strcpy(entry_str[view][line], ((entry_mode[line]) ? "6: Screenshot (XMB + Menu)\0"  : "6: Screenshot (XMB)\0"));  break;
   case 0xA: strcpy(entry_str[view][line], ((entry_mode[line]) ? "A: Reboot PS3 (hard)\0"        : "A: Reboot PS3 (soft)\0")); break;
  }
}

static void show_icon0(uint32_t curpad)
{
  if(view == FILE_MANAGER)
  {
    if(nitems == 0) return;

    if(curpad & PAD_DOWN) {cur_item++; if(cur_item >= nitems) cur_item = 0;}
    if(curpad & PAD_UP)   {if(cur_item > 0) cur_item--; else cur_item = nitems - 1;}


    if(has_icon0 == 1) has_icon0 = 0;

    if(!items_isdir[cur_item])
    {
        struct CellFsStat s;
        sprintf(tempstr, "%s/%s", curdir, items[cur_item]);
        cellFsStat(tempstr, &s);

        if(s.st_size >= 0x40000000ULL)
            sprintf(item_size, "%1.2f GB", ((double)(s.st_size)) / 1073741824.0f);
        if(s.st_size >= 0x100000ULL)
            sprintf(item_size, "%1.2f MB", ((double)(s.st_size)) / 1048576.0f);
        else
            sprintf(item_size, "%1.2f KB", ((double)(s.st_size)) / 1024.0f);
    }
    else if(has_icon0 == 0)
    {

        sprintf(tempstr, "%s/%s/ICON0.PNG", curdir, items[cur_item]);
        if(file_exists(tempstr)) {has_icon0 = 1; load_png_bitmap(1, tempstr);} else
        {
            sprintf(tempstr, "%s/%s/PS3_GAME/ICON0.PNG", curdir, items[cur_item]);
            if(file_exists(tempstr)) {has_icon0 = 1; load_png_bitmap(1, tempstr);} else
            {
                sprintf(tempstr, "%s/%s/../ICON0.PNG", curdir, items[cur_item]);
                if(file_exists(tempstr)) {has_icon0 = 1; load_png_bitmap(1, tempstr);} else
                {
                    sprintf(tempstr, "%s/%s/../../ICON0.PNG", curdir, items[cur_item]);
                    if(file_exists(tempstr)) {has_icon0 = 1; load_png_bitmap(1, tempstr);}
                }
            }
        }
    }

    tempstr[0]=0;
  }
}

////////////////////////////////////////////////////////////////////////
//                        PLUGIN MAIN PPU THREAD                      //
////////////////////////////////////////////////////////////////////////
static void vsh_menu_thread(uint64_t arg)
{
#ifdef DEBUG
  dbg_init();
  dbg_printf("programstart:\n");
#endif

  uint32_t oldpad = 0, curpad = 0;
  CellPadData pdata;

  uint32_t show_menu = 0;

  // init config
  config->bgindex = 0;
  config->dnotify = 0;

  // read config file
  int fd=0;
  if(cellFsOpen((char*)"/dev_hdd0/tmp/wm_vsh_menu.cfg", CELL_FS_O_RDONLY, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
  {
     cellFsRead(fd, (void *)vsh_menu_config, sizeof(vsh_menu_Cfg), 0);
     cellFsClose(fd);

     strcpy(entry_str[1][7], (config->dnotify) ? "7: Startup Message : OFF\0" : "7: Startup Message : ON\0");
  }

  sys_timer_sleep(13);

  if(!config->dnotify)
  {
    vshtask_notify("VSH Menu loaded.\nPress [Select] to open it.");
  }

  get_firmware_version();
  get_kernel_type();
  memset(payload_type, 0, 64);

  while(running)
  {
    if(!menu_running)                                                   // VSH menu is not running, normal XMB execution
    {
      VSHPadGetData(&pdata);                                            // use VSHPadGetData() to check pad

      if((pdata.len > 0) && (vshmain_EB757101() == 0))                  // if pad data and we are in XMB
      {
        curpad = (pdata.button[2] | (pdata.button[3] << 8));            // merge pad data

        if(curpad != oldpad) show_menu = 0;
        if(curpad == PAD_SELECT) ++show_menu;

        if(show_menu>4) // if select was pressed start VSH menu
        {
          show_menu = 0;
          if(line & 1) line = 0;   // menu on first entry in list

          start_VSH_Menu();
        }

        oldpad = curpad;
      }
      else
        oldpad = 0;

      sys_timer_usleep(200000);                                          // vsh sync
    }
    else // menu is running, draw frame, flip frame, check for pad events, sleep, ...
    {
      frame++; if(frame & 0x40) frame = 0; if(frame & 1) {draw_frame(); flip_frame();}

      for(int32_t port=0; port<8; port++)
        {MyPadGetData(port, &pdata); curpad = (pdata.button[2] | (pdata.button[3] << 8)); if(curpad) break;}  // use MyPadGetData() during VSH menu

      if(curpad)
      {
        if(curpad == oldpad) continue;

        oldpad = curpad;

        if(curpad & (PAD_SELECT | PAD_CIRCLE))  // if select was pressed stop VSH menu
        {
          return_to_xmb();
        }
        else
        if((curpad & (PAD_LEFT | PAD_RIGHT)))
        {
          if(view == MAIN_MENU)
          {
              change_main_menu_options(curpad);
          }
          if(view == REBUG_MENU)
          {
            switch (line)
            {
             case 7: do_rebug_menu_action(); break;
            }
          }
          if(view == FILE_MANAGER)
          {
              change_current_folder(curpad);
          }
        }
        else
        if(curpad & PAD_UP)
        {
          frame = 0;
          if(line > 0)
          {
            line--;
            play_rco_sound("system_plugin", "snd_cursor");
          }
          else
            line = (view > REBUG_MENU ? nitems : view == REBUG_MENU ? MAX_MENU2 : MAX_MENU)-1;

          show_icon0(curpad);
          oldpad = 0;
        }
        else
        if(curpad & PAD_DOWN)
        {
          frame = 0;
          if(line < ((view > REBUG_MENU ? nitems : view == REBUG_MENU ? MAX_MENU2 : MAX_MENU)-1))
          {
            line++;
            play_rco_sound("system_plugin", "snd_cursor");
          }
          else
            line = 0;

          show_icon0(curpad);
          oldpad = 0;
        }

        if(view == FILE_MANAGER && ((curpad & (PAD_CROSS | PAD_SQUARE)) || curpad == REFRESH_DIR))
        {
            do_file_manager_action(curpad);
        }
        else
        if(curpad & (PAD_R1 | PAD_TRIANGLE | PAD_START | PAD_L1))
        {
          if((curpad & PAD_R1) == PAD_R1 || (view != FILE_MANAGER && (curpad & PAD_TRIANGLE))) {clipboard_mode = 0; if(++view > 3) view = MAIN_MENU;}
          if((curpad & PAD_L1) == PAD_L1) {clipboard_mode = 0; if(view > 0) --view; else view = 3;}

          if(view == FILE_MANAGER)    do_file_manager_action(curpad);
          if(view == PLUGINS_MANAGER) do_plugins_manager_action(curpad);
        }
        else
        if(curpad & PAD_CROSS)
        {
          if(view == MAIN_MENU) do_main_menu_action(); else
          if(view == REBUG_MENU) do_rebug_menu_action();
          if(view == PLUGINS_MANAGER)
          {
              unsigned int slot = get_vsh_plugin_slot_by_name(items[cur_item], true);

              return_to_xmb();

              if(!slot) sprintf(tempstr, "%s unloaded", items[cur_item]); else {cobra_load_vsh_plugin(slot, items[cur_item], NULL, 0); sprintf(tempstr, "%s loaded in slot %i", items[cur_item], slot);}

              if(strstr(items[cur_item], "webftp_server")) wm_unload ^= 1;

              vshtask_notify(tempstr);
          }
        }
        else
        if(curpad & PAD_SQUARE)
        {
          // switch them file
          stop_VSH_Menu();
          config->bgindex++;
          start_VSH_Menu();

          // save config
          int fd=0;
          if(cellFsOpen((char*)"/dev_hdd0/tmp/wm_vsh_menu.cfg", CELL_FS_O_CREAT|CELL_FS_O_WRONLY, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
          {
             cellFsWrite(fd, (void *)vsh_menu_config, sizeof(vsh_menu_Cfg), 0);
             cellFsClose(fd);
          }
        }

        // ...
      }
      else
        oldpad = 0;

      sys_timer_usleep(75000); // short menu frame delay
    }
  }

#ifdef DEBUG
  dbg_fini();
#endif

  finalize_module();

  uint64_t exit_code;
  sys_ppu_thread_join(vsh_menu_tid, &exit_code);

  sys_ppu_thread_exit(0);
}

/***********************************************************************
* start thread
***********************************************************************/
int32_t vsh_menu_start(uint64_t arg)
{
  sys_ppu_thread_create(&vsh_menu_tid, vsh_menu_thread, 0, 3000, 0x4000, 1, THREAD_NAME);

  _sys_ppu_thread_exit(0);
  return SYS_PRX_RESIDENT;
}

/***********************************************************************
* stop thread
***********************************************************************/
static void vsh_menu_stop_thread(uint64_t arg)
{
  if(menu_running) stop_VSH_Menu();

  vshtask_notify("VSH Menu unloaded.");

  running = 0;
  sys_timer_usleep(500000); //Prevent unload too fast

  uint64_t exit_code;

  if(vsh_menu_tid != (sys_ppu_thread_t)-1)
      sys_ppu_thread_join(vsh_menu_tid, &exit_code);

  sys_ppu_thread_exit(0);
}

/***********************************************************************
*
***********************************************************************/
static void finalize_module(void)
{
  uint64_t meminfo[5];

  sys_prx_id_t prx = prx_get_module_id_by_address(finalize_module);

  meminfo[0] = 0x28;
  meminfo[1] = 2;
  meminfo[3] = 0;

  system_call_3(482, prx, 0, (uint64_t)(uint32_t)meminfo);
}

/***********************************************************************
*
***********************************************************************/
int vsh_menu_stop(void)
{
  sys_ppu_thread_t t;
  uint64_t exit_code;

  int ret = sys_ppu_thread_create(&t, vsh_menu_stop_thread, 0, 0, 0x2000, 1, STOP_THREAD_NAME);
  if (ret == 0) sys_ppu_thread_join(t, &exit_code);

  sys_timer_usleep(500000); // 0.5s

  finalize_module();

  _sys_ppu_thread_exit(0);
  return SYS_PRX_STOP_OK;
}
