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

#include "../vsh/vsh_exports.h"

#include "include/network.h"
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
} __attribute__((packed)) WebmanCfg;

typedef struct
{
	uint16_t bgindex;
	uint8_t  dnotify;
	char filler[509];
} __attribute__((packed)) vsh_menu_Cfg;

static uint8_t vsh_menu_config[sizeof(vsh_menu_Cfg)];
static vsh_menu_Cfg *config = (vsh_menu_Cfg*) vsh_menu_config;


static sys_ppu_thread_t vsh_menu_tid = -1;
static int32_t running = 1;
static uint8_t menu_running = 0;    // vsh menu off(0) or on(1)
static char fw[40];
int32_t vsh_menu_start(uint64_t arg);
int32_t vsh_menu_stop(void);

static void finalize_module(void);
static void vsh_menu_stop_thread(uint64_t arg);

char *current_file[512];

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

static int send_wm_request(char *cmd)
{
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

static void get_firmware(void)
{
  uint64_t n, data;

  strcpy(fw, "[unknown]");

  for(n=0x8000000000330000ULL; n<0x8000000000800000ULL; n+=8)
  {
    data = lv2peek(n);
    if( 8000 < data && data < 80000 )
    {
      char param[16] = "";

      if(is_cobra_based())
      {
        uint16_t cobra_version = 0; sys_get_version2(&cobra_version);

        char cobra_ver[8];
        if((cobra_version & 0x0F) == 0)
            sprintf(cobra_ver, "%X.%X", cobra_version>>8, (cobra_version & 0xFF) >> 4);
        else
            sprintf(cobra_ver, "%X.%02X", cobra_version>>8, (cobra_version & 0xFF));

        bool is_mamba; {system_call_1(8, SYSCALL8_OPCODE_GET_MAMBA); is_mamba = ((int)p1 ==0x666);}

        sprintf(param, " %s %s", is_mamba ? "Mamba" : "Cobra", cobra_ver);
      }

      if( n > 0x8000000000360000ULL )
        sprintf(fw, "%1.2f DEX %s", (float) data/10000, param);
      else
        sprintf(fw, "%1.2f CEX %s", (float) data/10000, param);
      break;
    }
  }
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

  // create VSH Menu heap memory from memory container 1("app")
  create_heap(16);  // 16 MB

  // initialize VSH Menu graphic
  init_graphic();

  // set_font(17, 17, 1, 1);  // set font(char w/h = 20 pxl, line-weight = 1 pxl, distance between chars = 1 pxl)

  // load png image
  load_png_bitmap(0, bg_image);

  get_firmware();


  // stop vsh pad
  start_stop_vsh_pad(0);

  // set menu_running on
  menu_running = 1;
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

		cellFsRename(VSH_ETC_PATH "index.dat", VSH_ETC_PATH "index.dat.nrm");
		cellFsRename(VSH_ETC_PATH "index.dat.swp", VSH_ETC_PATH "index.dat");

		cellFsRename(VSH_ETC_PATH "version.txt", VSH_ETC_PATH "version.txt.nrm");
		cellFsRename(VSH_ETC_PATH "version.txt.swp", VSH_ETC_PATH "version.txt");

		cellFsRename(VSH_MODULE_PATH "vsh.self", VSH_MODULE_PATH "vsh.self.nrm");
	    cellFsRename(VSH_MODULE_PATH "vsh.self.swp", VSH_MODULE_PATH "vsh.self");

		soft_reboot();
	}
	else
	if((cellFsStat((char*) VSH_MODULE_PATH "vsh.self.nrm", &s)==CELL_FS_SUCCEEDED))
	{
		stop_VSH_Menu();
		vshtask_notify("Rebug Mode detected!\r\nSwitch to Normal Mode...");
		play_rco_sound("system_plugin", "snd_system_ok");
		sys_timer_sleep(1);

		cellFsRename(VSH_ETC_PATH "index.dat", VSH_ETC_PATH "index.dat.swp");
		cellFsRename(VSH_ETC_PATH "index.dat.nrm", VSH_ETC_PATH "index.dat");

		cellFsRename(VSH_ETC_PATH "version.txt", VSH_ETC_PATH "version.txt.swp");
		cellFsRename(VSH_ETC_PATH "version.txt.nrm", VSH_ETC_PATH "version.txt");

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
		vshtask_notify("webMAN MOD was not detected on dev_flash");
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
#define MAX_MENU     9
#define MAX_MENU2    7

static uint8_t view = 0;

uint8_t entry_mode[MAX_MENU] = {0, 0, 0, 0, 0, 0, 0, 0};

static char entry_str[2][MAX_MENU][32] = {
                                          {
                                           "0: Unmount Game",
                                           "1: Mount /net0",
                                           "2: Fan (+)",
                                           "3: Refresh XML",
                                           "4: Toggle gameDATA",
                                           "5: Backup Disc to HDD",
                                           "6: Screenshot (XMB)",
                                           "7: Shutdown PS3",
                                           "8: Reboot PS3 (soft)",
                                          },
                                          {
                                           "0: Toggle Rebug Mode",
                                           "1: Toggle XMB Mode",
                                           "2: Toggle Debug Menu",
                                           "3: Disable Cobra",
                                           "4: Disable webMAN MOD",
                                           "5: Recovery Mode",
                                           "6: Startup Message : ON",
                                          }
                                        };

////////////////////////////////////////////////////////////////////////
//               EXECUTE ACTION DEPENDING LINE SELECTED               //
////////////////////////////////////////////////////////////////////////
static uint8_t fan_mode = 0;

static void do_menu_action(void)
{
  switch(line)
  {
    case 0:
      buzzer(1);
      send_wm_request((char*)"GET /mount_ps3/unmount");

      if(entry_mode[line]) wait_for_request(); else sys_timer_sleep(1);

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
      {
        int fd=0;
        if(cellFsOpen((char*)"/dev_hdd0/tmp/wmconfig.bin", CELL_FS_O_RDONLY, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
        {
           uint8_t wmconfig[sizeof(WebmanCfg)];
           WebmanCfg *webman_config = (WebmanCfg*) wmconfig;

           cellFsRead(fd, (void *)wmconfig, sizeof(WebmanCfg), 0);
           cellFsClose(fd);

           fan_mode = (webman_config->temp0>0); // manual
        }
      }

      if(entry_mode[line]==(fan_mode ? 1 : 0)) {send_wm_request((char*)"GET /cpursx.ps3?dn"); buzzer(1);}
      if(entry_mode[line]==(fan_mode ? 0 : 1)) {send_wm_request((char*)"GET /cpursx.ps3?up"); buzzer(1);}

      if(entry_mode[line]==2) {send_wm_request((char*)"GET /cpursx.ps3?mode"); buzzer(3); entry_mode[line]=3; strcpy(entry_str[view][line], "2: System Info"); fan_mode = fan_mode ? 0 : 1;} else
      if(entry_mode[line]==3) {send_wm_request((char*)"GET /popup.ps3"); sys_timer_sleep(1); stop_VSH_Menu();}

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

      break;
    case 7:
      stop_VSH_Menu();

      buzzer(2);
      shutdown_system();
      return;
    case 8:
      stop_VSH_Menu();

      buzzer(1);
      if(entry_mode[line]) hard_reboot(); else soft_reboot();
      return;
  }

  // return to XMB
  sys_timer_sleep(1);
  stop_VSH_Menu();

  play_rco_sound("system_plugin", "snd_system_ok");
}

static void do_menu_action2(void)
{
  buzzer(1);

  switch(line)
  {
    case 0:
      toggle_normal_rebug_mode();
      return;

    case 1:
      toggle_xmb_mode();
      return;

    case 2:
      toggle_debug_menu();
      return;

    case 3:
      disable_cobra_stage2();
      return;

    case 4:
      disable_webman();
      return;

    case 5:
      recovery_mode();
      return;

    case 6:
      config->dnotify = config->dnotify ? 0 : 1;
      strcpy(entry_str[view][line], (config->dnotify) ? "6: Startup Message : OFF\0" : "6: Startup Message : ON\0");

      // save config
      int fd=0;
      if(cellFsOpen((char*)"/dev_hdd0/tmp/wm_vsh_menu.cfg", CELL_FS_O_CREAT|CELL_FS_O_WRONLY, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
      {
         cellFsWrite(fd, (void *)vsh_menu_config, sizeof(vsh_menu_Cfg), 0);
         cellFsClose(fd);
      }
      return;
  }
}

////////////////////////////////////////////////////////////////////////
//                             DRAW A FRAME                           //
////////////////////////////////////////////////////////////////////////
static uint32_t frame = 0;

static void draw_frame(void)
{
  int32_t i, selected;
  int32_t count = 4;                      // list count  .. tip: if less than entries nbr, will scroll :)
  uint32_t color = 0, selcolor = 0;

  // all 32bit colors are ARGB, the framebuffer format
  set_background_color(0xFF000000);     // red, semitransparent
  set_foreground_color(0xFFFFFFFF);     // white, opac

  // fill background with background color
  //draw_background();

  // draw background from png
  draw_png(0, 0, 0, 0, 0, 720, 400);

  // draw logo from png
  draw_png(0, 648, 336, 576, 400, 64, 64);


  // print headline string, center(x = -1)
  set_font(22.f, 23.f, 1.f, 1); print_text(-1, 8, (view ? "VSH Menu for Rebug" : "VSH Menu for webMAN"));
  set_font(14.f, 14.f, 1.f, 1); print_text(650, 8, "v0.7");


  set_font(20.f, 20.f, 1.f, 1);

  frame++; if(frame & 0x8) frame = 0; selcolor = (frame & 0x4) ? 0xFFFFFFFF : 0xFF00FF00;

  // draw menu entry list
  for(i = 0; i < count; i++)
  {
    if(line < count)
    {
      selected = 0;
      i == line ? (color = selcolor) : (color = 0xFFFFFFFF);
    }
    else
    {
      selected = line - (count - 1);
      i == (count - 1) ? (color = selcolor) : (color = 0xFFFFFFFF);
    }

    set_foreground_color(color);

    print_text(20, 40 + (LINE_HEIGHT * (i + 1)), entry_str[view][selected + i]);

    selected++;
  }
  if (count < MAX_MENU)
    if (line > count - 1)    draw_png(0, 20, 56, 688, 400, 16, 8);   // UP arrow
    if (line < MAX_MENU - 1) draw_png(0, 20, 177, 688, 408, 16, 8);  // DOWN arrow


  set_font(20.f, 17.f, 1.f, 1);


  //draw command buttons
  set_foreground_color(0xFF999999);
  draw_png(0, 522, 230, 128 + ((!view && (line<3||line==6||line==8)) ? 64 : 0), 432, 32, 32); print_text(560, 234, ": Choose");  // draw up-down button
  draw_png(0, 522, 262, 0, 400, 32, 32); print_text(560, 266, ": Select");  // draw X button
  draw_png(0, 522, 294, 416, 400, 32, 32); print_text(560, 298, ": Exit");  // draw select button
  draw_png(0, 522, 326, 64, 400, 32, 32); print_text(560, 330, ": Menu");  // draw Triangle button
  set_foreground_color(0xFF008FFF);


  //draw firmware version info
  char firmware[48];

  sprintf(firmware, "Firmware: %s", fw);
  print_text(352, 30 + (LINE_HEIGHT * 1), firmware);


  //draw Network info
  char netdevice[32];
  char ipaddr[32];
  char netstr[64];

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

  print_text(352, 30 + (LINE_HEIGHT * 2.5), netstr);


  // draw temperatures
  char tempstr[128];
  int t_icon_X;
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

  set_font(24.f, 17.f, 1.f, 1);

  draw_png(0, 355, 38 + (LINE_HEIGHT * 5), t_icon_X, 464, 32, 32);
  print_text(395, 30 + (LINE_HEIGHT * 5), tempstr);

  set_font(20.f, 17.f, 1.f, 1);


  //draw drives info
  set_foreground_color(0xFFFFFF55);
  print_text(20, 208, "Available free space on device(s):");

  int fd;

  if(cellFsOpendir("/", &fd) == CELL_FS_SUCCEEDED)
  {
    int32_t j = 0;
    char drivestr[64], drivepath[32], freeSizeStr[32], devSizeStr[32];
    uint64_t read, freeSize, devSize;
    CellFsDirent dir;

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

      sprintf(drivestr, "%s :  %s / %s", drivepath, freeSizeStr, devSizeStr);

      if (strncmp("dev_hdd", dir.d_name, 7) == 0)
        draw_png(0, 25, 230 + (26 * j), 64, 464, 32, 32);
      else if (strncmp("dev_usb", dir.d_name, 7) == 0)
        draw_png(0, 25, 230 + (26 * j), 96, 464, 32, 32);
      else if ((strncmp("dev_sd", dir.d_name, 6) == 0 ) || (strncmp("dev_ms", dir.d_name, 6) == 0 ) ||  (strncmp("dev_cf", dir.d_name, 6) == 0 ))
        draw_png(0, 25, 230 + (26 * j), 160, 464, 32, 32);
      else if (strncmp("dev_blind", dir.d_name, 9) == 0 )
        draw_png(0, 25, 230 + (26 * j), 128, 464, 32, 32);

      print_text(60, 235 + (26 * j), drivestr);

      j++; if(j > 5) break;

    }
    cellFsClosedir(fd);
  }

  //...

}


////////////////////////////////////////////////////////////////////////
//                        PLUGIN MAIN PPU THREAD                      //
////////////////////////////////////////////////////////////////////////
static void vsh_menu_thread(uint64_t arg)
{
  dbg_init();
  dbg_printf("programstart:\n");

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

     strcpy(entry_str[1][6], (config->dnotify) ? "6: Startup Message : OFF\0" : "6: Startup Message : ON\0");
  }

  sys_timer_sleep(13);

  if(!config->dnotify)
  {
    vshtask_notify("VSH Menu loaded.\nPress [Select] to open it.");
  }

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
          if(line % 2) line = 0;   // menu on first entry in list

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
      draw_frame();

      flip_frame();

      for(int32_t port=0; port<4; port++)
        {MyPadGetData(port, &pdata); if(pdata.len > 0) break;}          // use MyPadGetData() during VSH menu

      if(pdata.len > 0)                                                 // if pad data
      {
        curpad = (pdata.button[2] | (pdata.button[3] << 8));            // merge pad data

        if((curpad & (PAD_SELECT | PAD_CIRCLE)) && (curpad != oldpad))  // if select was pressed stop VSH menu
        {
          stop_VSH_Menu();
          if(view) {view = line = 0;}
        }
        else
        if((curpad & (PAD_LEFT | PAD_RIGHT)) && (curpad != oldpad))
        {
          if(!view)
          {
            if(curpad & PAD_RIGHT) ++entry_mode[line]; else if(entry_mode[line]==0) entry_mode[line] = ((line==2) ? 3 : (line<=1) ? 2 : 1); else --entry_mode[line];

            if(entry_mode[line]>((line==2) ? 3 : (line<=1) ? 2 : 1)) entry_mode[line]=0;

            switch (line)
            {
             case 0: strcpy(entry_str[view][line], ((entry_mode[line] == 1) ? "0: Eject Disc\0"   : (entry_mode[line] == 2) ? "0: Insert Disc\0" : "0: Unmount Game\0")); break;
             case 1: strcpy(entry_str[view][line], ((entry_mode[line] == 1) ? "1: Mount /net1"    : (entry_mode[line] == 2) ? "1: Mount /net2"   : "1: Mount /net0"));  break;
             case 2: strcpy(entry_str[view][line], ((entry_mode[line] == 1) ? "2: Fan (-)\0"      : (entry_mode[line] == 2) ? "2: Fan Mode\0"    : (entry_mode[line] == 3) ? "2: System Info\0" : "2: Fan (+)\0")); break;
             case 6: strcpy(entry_str[view][line], ((entry_mode[line]) ? "6: Screenshot (XMB + Menu)\0"  : "6: Screenshot (XMB)\0"));  break;
             case 8: strcpy(entry_str[view][line], ((entry_mode[line]) ? "8: Reboot PS3 (hard)\0"        : "8: Reboot PS3 (soft)\0")); break;
            }
          }
          else
          {
            switch (line)
            {
             case 8: do_menu_action2(); break;
            }
          }
        }
        else
        if((curpad & PAD_UP) && (curpad != oldpad))
        {
          frame = 0;
          if(line > 0)
          {
            line--;
            play_rco_sound("system_plugin", "snd_cursor");
          }
          else
            line = (view ? MAX_MENU2 : MAX_MENU)-1;
        }
        else
        if((curpad & PAD_DOWN) && (curpad != oldpad))
        {
          frame = 0;
          if(line < ((view ? MAX_MENU2 : MAX_MENU)-1))
          {
            line++;
            play_rco_sound("system_plugin", "snd_cursor");
          }
          else
            line = 0;
        }
        else
        if((curpad & PAD_CROSS) && (curpad != oldpad))
        {
          if(view) do_menu_action2(); else do_menu_action();
        }
        else
        if((curpad & PAD_TRIANGLE) && (curpad != oldpad))
        {
            view = (view ? 0 : 1); line = 0;
        }
        else
        if((curpad & PAD_SQUARE) && (curpad != oldpad))
        {
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

        oldpad = curpad;
      }
      else
        oldpad = 0;

      sys_timer_usleep(75000); // short menu frame delay
    }
  }

  dbg_fini();

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
