#include <sdk_version.h>
#include <cellstatus.h>
#include <cell/cell_fs.h>
#include <cell/rtc.h>
#include <cell/gcm.h>
#include <cell/pad.h>
#include <sys/vm.h>
#include <sysutil/sysutil_common.h>

#include <sys/prx.h>
#include <sys/ppu_thread.h>
#include <sys/event.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/memory.h>
#include <sys/timer.h>
#include <sys/process.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netex/net.h>
#include <netex/errno.h>
#include <netex/libnetctl.h>
#include <netex/sockinfo.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "flags.h"

#include "types.h"
#include "common.h"
#include "cobra/cobra.h"
#include "cobra/storage.h"
#include "vsh/game_plugin.h"
#include "vsh/netctl_main.h"
#include "vsh/xregistry.h"

#define _game_TitleID  _game_info+0x04
#define _game_Title    _game_info+0x14

static char _game_info[0x120];
static char search_url[50] = "http://google.com/search?q=";

#ifdef COBRA_ONLY
 #include "cobra/netiso.h"

 #ifdef LITE_EDITION
    #define EDITION " [Lite]"
 #else
  #ifdef PS3MAPI
	#ifdef REX_ONLY
		#define EDITION " [Rebug-PS3MAPI]"
	#else
		#define EDITION " [PS3MAPI]"
	#endif
  #else
   #ifdef REX_ONLY
    #define EDITION " [Rebug]"
   #else
    #define EDITION ""
   #endif
  #endif
 #endif
#else
 #ifdef CCAPI
    #define EDITION " [CCAPI]"
 #else
    #define EDITION " [nonCobra]"
 #endif
#endif

SYS_MODULE_INFO(WWWD, 0, 1, 0);
SYS_MODULE_START(wwwd_start);
SYS_MODULE_STOP(wwwd_stop);

#define VSH_MODULE_PATH 	"/dev_blind/vsh/module/"
#define VSH_ETC_PATH		"/dev_blind/vsh/etc/"
#define PS2_EMU_PATH		"/dev_blind/ps2emu/"
#define REBUG_COBRA_PATH	"/dev_blind/rebug/cobra/"
#define HABIB_COBRA_PATH	"/dev_blind/habib/cobra/"
#define SYS_COBRA_PATH		"/dev_blind/sys/"
#define PS2_CLASSIC_TOGGLER "/dev_hdd0/classic_ps2"
#define REBUG_TOOLBOX		"/dev_hdd0/game/RBGTLBOX2/USRDIR/"
#define COLDBOOT_PATH		"/dev_blind/vsh/resource/coldboot.raf"
#define ORG_LIBFS_PATH		"/dev_flash/sys/external/libfs.sprx"
#define NEW_LIBFS_PATH		"/dev_hdd0/tmp/libfs.sprx"

#define PS2_CLASSIC_PLACEHOLDER  "/dev_hdd0/game/PS2U10000/USRDIR"
#define PS2_CLASSIC_ISO_PATH     "/dev_hdd0/game/PS2U10000/USRDIR/ISO.BIN.ENC"
#define PS2_CLASSIC_ISO_ICON     "/dev_hdd0/game/PS2U10000/ICON0.PNG"

#define WM_VERSION			"1.43.07 MOD"						// webMAN version
#define MM_ROOT_STD			"/dev_hdd0/game/BLES80608/USRDIR"	// multiMAN root folder
#define MM_ROOT_SSTL		"/dev_hdd0/game/NPEA00374/USRDIR"	// multiman SingStar® Stealth root folder
#define MM_ROOT_STL			"/dev_hdd0/tmp/game_repo/main"		// stealthMAN root folder

#define WMCONFIG			"/dev_hdd0/tmp/wmconfig.bin"		// webMAN config file
#define WMTMP				"/dev_hdd0/tmp/wmtmp"				// webMAN work/temp folder
#define WM_ICONS_PATH		"/dev_hdd0/tmp/wm_icons/"			// webMAN icons path
#define WMNOSCAN			"/dev_hdd0/tmp/wm_noscan"			// webMAN config file

#ifdef WEB_CHAT
 #define DELETE_TURNOFF		{do_umount(false); cellFsUnlink((char*)"/dev_hdd0/tmp/turnoff"); cellFsUnlink((char*)WMCHATFILE);}
#else
 #define DELETE_TURNOFF		{do_umount(false); cellFsUnlink((char*)"/dev_hdd0/tmp/turnoff");}
#endif

#define THREAD_NAME 		"wwwdt"
#define THREAD_NAME_FTP 	"ftpdt"
#define THREAD_NAME_NET 	"netiso"
#define THREAD_NAME_NTFS 	"ntfsd"

#define STOP_THREAD_NAME 	"wwwds"

////////////

#define HTML_BASE_PATH			"/dev_hdd0/xmlhost/game_plugin"

#define FB_XML					HTML_BASE_PATH "/fb.xml"
#define MY_GAMES_XML			HTML_BASE_PATH "/mygames.xml"
#define MOBILE_HTML				HTML_BASE_PATH "/mobile.html"
#define GAMELIST_JS				HTML_BASE_PATH "/gamelist.js"

#define DELETE_CACHED_GAMES		{cellFsUnlink((char*)WMTMP "/games.html"); cellFsUnlink((char*)GAMELIST_JS);}

////////////

#define SC_COBRA_SYSCALL8				(8)

#define SC_GET_FREE_MEM 				(352)
#define SC_GET_PLATFORM_INFO			(387)
#define SC_RING_BUZZER  				(392)

#define SC_FS_MOUNT  					(837)
#define SC_FS_UMOUNT 					(838)
#define SC_GET_CONSOLE_TYPE				(985)

#define SC_GET_PRX_MODULE_BY_ADDRESS	(461)
#define SC_STOP_PRX_MODULE 				(482)
#define SC_UNLOAD_PRX_MODULE 			(483)
#define SC_PPU_THREAD_EXIT				(41)

#define SC_SYS_POWER 					(379)
#define SYS_SOFT_REBOOT 				0x0200
#define SYS_HARD_REBOOT					0x1200
#define SYS_REBOOT						0x8201 /*load LPAR id 1*/
#define SYS_SHUTDOWN					0x1100

#define SYS_NET_EURUS_POST_COMMAND		(726)
#define CMD_GET_MAC_ADDRESS				0x103f

#define SYSCALL8_OPCODE_GET_MAMBA		0x7FFFULL

#define BEEP1 { system_call_3(SC_RING_BUZZER, 0x1004, 0x4,   0x6); }
#define BEEP2 { system_call_3(SC_RING_BUZZER, 0x1004, 0x7,  0x36); }
#define BEEP3 { system_call_3(SC_RING_BUZZER, 0x1004, 0xa, 0x1b6); }

#define WWWPORT			(80)
#define FTPPORT			(21)

#define KB			   1024UL
#define   _4KB_		   4096UL
#define   _8KB_		   8192UL
#define  _32KB_		  32768UL
#define  _64KB_		  65536UL
#define _128KB_		 131072UL
#define _192KB_		 196608UL
#define _256KB_		 262144UL
#define  _1MB_		1048576UL
#define _32MB_		33554432UL

#define MIN_MEM		_192KB_

#define MODE		0777

#define LINELEN			512 // file listing
#define MAX_LINE_LEN	640 // html games
#define MAX_PATH_LEN	512 // do not change!

#define FAILED		-1

#define HTML_RECV_SIZE 2048

static u32 BUFFER_SIZE_FTP	= ( _128KB_);

static u32 BUFFER_SIZE		= ( 448*KB);
static u32 BUFFER_SIZE_PSX	= ( 160*KB);
static u32 BUFFER_SIZE_PSP	= (  _32KB_);
static u32 BUFFER_SIZE_PS2	= (  _64KB_);
static u32 BUFFER_SIZE_DVD	= ( _192KB_);
static u32 BUFFER_SIZE_ALL	= ( 896*KB);

#ifdef COBRA_ONLY
 #ifndef LITE_EDITION
 static sys_ppu_thread_t thread_id_net	=-1;
 #endif
 static sys_ppu_thread_t thread_id_ntfs	=-1;
#endif
static sys_ppu_thread_t thread_id_poll	=-1;
static sys_ppu_thread_t thread_id_ftp	=-1;
static sys_ppu_thread_t thread_id		=-1;

#define SUFIX(a)	((a==1)? "_1" :(a==2)? "_2" :(a==3)? "_3" :(a==4)?"_4":"")
#define SUFIX2(a)	((a==1)?" (1)":(a==2)?" (2)":(a==3)?" (3)":(a==4)?" (4)":"")
#define SUFIX3(a)	((a==1)?" (1).ntfs[":(a==2)?" (2).ntfs[":(a==3)?" (3).ntfs[":(a==4)?" (4).ntfs[":"")

#define IS_ISO_FOLDER (f1>1 && f1<10)
#define IS_PS3_FOLDER (f1<3 || f1>=10)
#define IS_BLU_FOLDER (f1==3)
#define IS_DVD_FOLDER (f1==4)
#define IS_PS2_FOLDER (f1==5)
#define IS_PSX_FOLDER (f1==6 || f1==7)
#define IS_PSP_FOLDER (f1==8 || f1==9)

#define PS3 (1<<0)
#define PS2 (1<<1)
#define PS1 (1<<2)
#define PSP (1<<3)
#define BLU (1<<4)
#define DVD (1<<5)

#define MIN(a, b)	((a) <= (b) ? (a) : (b))
#define ABS(a)		(((a) < 0) ? -(a) : (a))
#define RANGE(a, b, c)	((a) <= (b) ? (b) : (a) >= (c) ? (c) : (a))

#define START_DAEMON		(0xC0FEBABE)
#define REFRESH_CONTENT		(0xC0FEBAB0)
#define WM_FILE_REQUEST		(0xC0FEBEB0)

typedef struct {
	uint32_t total;
	uint32_t avail;
} _meminfo;

//static bool is_rebug = false;
static u8 profile = 0;

static u8 loading_html = 0;
static u8 loading_games = 0;
static u8 init_running = 0;

#define NTFS 		 	(10)

static bool show_info_popup = false;

#ifdef USE_DEBUG
 static int debug_s=-1;
 static char debug[256];
#endif
static volatile u8 wm_unload_combo = 0;
static volatile u8 working = 1;
static u8 cobra_mode=0;
static u8 max_mapped=0;

static float c_firmware=0.0f;
static u8 dex_mode=0;

static u64 SYSCALL_TABLE = 0;

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

#define AUTOBOOT_PATH            "/dev_hdd0/PS3ISO/AUTOBOOT.ISO"

#ifdef COBRA_ONLY
 #define DEFAULT_AUTOBOOT_PATH   "/dev_hdd0/PS3ISO/AUTOBOOT.ISO"
#else
 #define DEFAULT_AUTOBOOT_PATH   "/dev_hdd0/GAMES/AUTOBOOT"
#endif

#define ISO_EXTENSIONS           ".iso.cue.img.mdf.bin"

static CellRtcTick rTick, gTick;

static int set_gamedata_status(u8 status, bool do_mount);
static void set_buffer_sizes(int footprint);

#ifdef COBRA_ONLY
static void select_ps1emu(void);
#endif

int extcmp(const char *s1, const char *s2, size_t n);
int extcasecmp(const char *s1, const char *s2, size_t n);
char *strcasestr(const char *s1, const char *s2);

static void reset_settings(void);
static int save_settings(void);
static u64 backup[6];

static u8 wmconfig[sizeof(WebmanCfg)];
static WebmanCfg *webman_config = (WebmanCfg*) wmconfig;

static bool gmobile_mode = false;

static char ftp_password[20]="";
static char html_base_path[MAX_PATH_LEN]="";

static char smonth[12][4]={"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

static char drives[14][12]={"/dev_hdd0", "/dev_usb000", "/dev_usb001", "/dev_usb002", "/dev_usb003", "/dev_usb006", "/dev_usb007", "/net0", "/net1", "/net2", "/ext", "/dev_sd", "/dev_ms", "/dev_cf"};
static char paths [11][12]={"GAMES", "GAMEZ", "PS3ISO", "BDISO", "DVDISO", "PS2ISO", "PSXISO", "PSXGAMES", "PSPISO", "ISO", "video"};

static char wm_icons[12][60]={WM_ICONS_PATH "icon_wm_album_ps3.png", //024.png  [0]
                              WM_ICONS_PATH "icon_wm_album_psx.png", //026.png  [1]
                              WM_ICONS_PATH "icon_wm_album_ps2.png", //025.png  [2]
                              WM_ICONS_PATH "icon_wm_album_psp.png", //022.png  [3]
                              WM_ICONS_PATH "icon_wm_album_dvd.png", //023.png  [4]

                              WM_ICONS_PATH "icon_wm_ps3.png",       //024.png  [5]
                              WM_ICONS_PATH "icon_wm_psx.png",       //026.png  [6]
                              WM_ICONS_PATH "icon_wm_ps2.png",       //025.png  [7]
                              WM_ICONS_PATH "icon_wm_psp.png",       //022.png  [8]
                              WM_ICONS_PATH "icon_wm_dvd.png",       //023.png  [9]

                              WM_ICONS_PATH "icon_wm_settings.png",  //icon/icon_home.png  [10]
                              WM_ICONS_PATH "icon_wm_eject.png"      //icon/icon_home.png  [11]
                             };

static bool covers_exist[7];
static char local_ip[16] = "127.0.0.1";

//uint64_t find_syscall();
//uint64_t search64(uint64_t val);
//uint64_t find_syscall_table();

#define SYSCALLS_UNAVAILABLE    0xFFFFFFFF80010003ULL

#include "include/peek_poke.h"
#include "include/led.h"
#include "include/socket.h"
#include "include/html.h"
#include "include/language.h"

#include "include/ps2_classic.h"
#include "include/vpad.h"
#include "include/idps.h"
#include "include/singstar.h"
#include "include/xmb_savebmp.h"

int wwwd_start(uint64_t arg);
int wwwd_stop(void);
static void stop_prx_module(void);
static void unload_prx_module(void);

static void detect_firmware(void);

static void handleclient(u64 conn_s_p);

static void do_umount(bool clean);
static void mount_autoboot(void);
static bool mount_with_mm(const char *_path, u8 do_eject);

#ifdef COBRA_ONLY
static void do_umount_iso(void);
#endif

static bool from_reboot = false;
static bool is_busy = false;
static bool is_mounting = false;

#ifdef COPY_PS3
static char current_file[MAX_PATH_LEN];
#endif

#include "include/vsh.h"
#include "include/webchat.h"
#include "include/file.h"
#include "include/fix_game.h"
#include "include/ps2_disc.h"

#ifndef COBRA_ONLY
static void string_to_lv2(char* path, u64 addr);
static void add_to_map(char *path1, char *path2);
#endif

static inline void _sys_ppu_thread_exit(uint64_t val)
{
	system_call_1(SC_PPU_THREAD_EXIT, val); // prxloader = mandatory; cobra = optional; ccapi = don't use !!!
}

static inline sys_prx_id_t prx_get_module_id_by_address(void *addr)
{
	system_call_1(SC_GET_PRX_MODULE_BY_ADDRESS, (uint64_t)(uint32_t)addr);
	return (int)p1;
}

#include "include/eject_insert.h"

#ifdef COBRA_ONLY

#include "include/rawseciso.h"
#include "include/netsvr.h"

#endif //#ifdef COBRA_ONLY

#include "include/gamedata.h"
#include "include/psxemu.h"

#include "include/debug_mem.h"
#include "include/ftp.h"
#include "include/fancontrol.h"
#include "include/ps3mapi.h"
#include "include/video_rec.h"
#include "include/stealth.h"

#include "include/games_html.h"
#include "include/games_xml.h"

#include "include/cpursx.h"
#include "include/setup.h"
#include "include/fancontrol2.h"

#include "include/file_manager.h"
#include "include/_mount.h"

static void http_response(int conn_s, char *header, char *param, int code, char *msg)
{
	char text[MAX_LINE_LEN]; if(msg[0]=='/') sprintf(text, "%s : OK", msg+1); else sprintf(text, "%s", msg);

	sprintf(header, "HTTP/1.1 %i OK\r\n"
					"X-PS3-Info: [%s]\r\n"
					"Content-Type: text/html;charset=UTF-8\r\n"
					"Cache-Control: no-cache\r\n"
					"Content-Length: %i\r\n\r\n"
					"<body bgcolor=\"#101010\" text=\"#c0c0c0\">"
					"<font face=\"Courier New\">"
					"webMAN MOD " WM_VERSION "<hr><h2>%s</h2>"
					"</font></body>",
					 code, param, 113+strlen(text), text);

	ssend(conn_s, header);
	sclose(&conn_s);

	if(msg[0]=='/') {show_msg((char*)text); sys_timer_sleep(1); }
}

static void prepare_html(char *buffer, char *templn, char *param, u8 is_ps3_http, u8 is_cpursx, bool mount_ps3)
{
	strcpy(buffer,  "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"
					"<html xmlns=\"http://www.w3.org/1999/xhtml\">"
					"<meta http-equiv=\"Content-type\" content=\"text/html;charset=UTF-8\">"
					"<meta http-equiv=\"Cache-Control\" content=\"no-cache\">"
					"<meta name=\"viewport\" content=\"width=device-width,initial-scale=0.6,maximum-scale=1.0\">");

	if(is_cpursx) strcat(buffer, "<meta http-equiv=\"refresh\" content=\"6;URL=/cpursx.ps3\">");
	if(mount_ps3) {	strcat(buffer, "<body bgcolor=\"#101010\">"); return;}

	strcat(buffer,	"<head><title>webMAN MOD</title>"
					"<style type=\"text/css\"><!--\r\n"

					"a.s:active{color:#F0F0F0;}"
					"a:link{color:#909090;text-decoration:none;}"

					"a.f:active{color:#F8F8F8;}"
					"a,a.f:link,a:visited{color:#D0D0D0;}");

	if(!is_cpursx)
	strcat(buffer,	"a.d:link{color:#D0D0D0;background-position:0px 2px;background-image:url('data:image/gif;base64,R0lGODlhEAAMAIMAAOenIumzLbmOWOuxN++9Me+1Pe+9QvDAUtWxaffKXvPOcfTWc/fWe/fWhPfckgAAACH5BAMAAA8ALAAAAAAQAAwAAARQMI1Agzk4n5Sa+84CVNUwHAz4KWzLMo3SzDStOkrHMO8O2zmXsAXD5DjIJEdxyRie0KfzYChYr1jpYVAweb/cwrMbAJjP54AXwRa433A2IgIAOw==');padding:0 0 0 20px;background-repeat:no-repeat;margin-left:auto;margin-right: auto;}"
					"a.w:link{color:#D0D0D0;background-image:url('data:image/gif;base64,R0lGODlhDgAQAIMAAAAAAOfn5+/v7/f39////////////////////////////////////////////wAAACH5BAMAAA8ALAAAAAAOABAAAAQx8D0xqh0iSHl70FxnfaDohWYloOk6papEwa5g37gt5/zO475fJvgDCW8gknIpWToDEQA7');padding:0 0 0 20px;background-repeat:no-repeat; margin-left:auto; margin-right:auto;}");

	strcat(buffer,	"a:active,a:active:hover,a:visited:hover,a:link:hover{color:#FFFFFF;}"
					".list{display:inline;}"
					"input:focus{border:2px solid #0099FF;}"
					".gc{float:left;overflow:hidden;position:relative;text-align:center;width:280px;height:260px;margin:3px;border:1px dashed grey;}"
					".ic{position:absolute;top:5px;right:5px;left:5px;bottom:40px;}"
					".propfont{font-family:\"Courier New\",Courier,monospace;}"
					"body,a.s,td,th{color:#F0F0F0;white-space:nowrap;");

	if(is_ps3_http==2)
		strcat(buffer, "width:800px;}");
	else
		strcat(buffer, "}");

	if(!strstr(param, "/setup.ps3")) strcat(buffer, "td+td{text-align:right;white-space:nowrap}");

	if(is_ps3_http==1)
		strcat(buffer, ".gi{height:210px;width:267px;");
	else
		strcat(buffer, ".gi{position:absolute;max-height:210px;max-width:260px;");

	strcat(buffer, "position:absolute;bottom:0px;top:0px;left:0px;right:0px;margin:auto;}"
				   ".gn{position:absolute;height:38px;bottom:0px;right:7px;left:7px;text-align:center;}--></style>");

	if(param[1]!=NULL) {sprintf(templn, "<base href=\"%s/\">", param); strcat(buffer, templn);}

	strcat(buffer,	"</head>"
					"<body bgcolor=\"#101010\">"
					"<font face=\"Courier New\"><b>");

#ifndef ENGLISH_ONLY
    if(strlen(STR_TRADBY)==0) language("STR_TRADBY", STR_TRADBY); //strcpy(STR_TRADBY, "<br>");
    if(strlen(STR_HOME  )==0) language("STR_HOME"  , STR_HOME  ); //strcpy(STR_HOME  , "<br>");
#else
    strcpy(STR_HOME, "Home");
#endif

#ifdef PS3MAPI
	#ifdef WEB_CHAT
		sprintf(templn, "webMAN " WM_VERSION " %s <font style=\"font-size:18px\">[<a href=\"/\">%s</a>] [<a href=\"/index.ps3\">%s</a>] [<a href=\"/games.ps3\">Slider</a>] [<a href=\"/chat.ps3\">Chat</a>] [<a href=\"/home.ps3mapi\">PS3MAPI</a>] [<a href=\"/setup.ps3\">%s</a>]</b>", STR_TRADBY, STR_FILES, STR_GAMES, STR_SETUP); strcat(buffer, templn);
	#else
		sprintf(templn, "webMAN " WM_VERSION " %s <font style=\"font-size:18px\">[<a href=\"/\">%s</a>] [<a href=\"/index.ps3\">%s</a>] [<a href=\"/games.ps3\">Slider</a>] [<a href=\"/home.ps3mapi\">PS3MAPI</a>] [<a href=\"/setup.ps3\">%s</a>]</b>", STR_TRADBY, STR_FILES, STR_GAMES, STR_SETUP ); strcat(buffer, templn);
	#endif
#else
	#ifdef WEB_CHAT
		sprintf(templn, "webMAN " WM_VERSION " %s <font style=\"font-size:18px\">[<a href=\"/\">%s</a>] [<a href=\"/index.ps3\">%s</a>] [<a href=\"/games.ps3\">Slider</a>] [<a href=\"/chat.ps3\">Chat</a>] [<a href=\"/setup.ps3\">%s</a>]</b>", STR_TRADBY, STR_FILES, STR_GAMES, STR_SETUP); strcat(buffer, templn);
	#else
		sprintf(templn, "webMAN " WM_VERSION " %s <font style=\"font-size:18px\">[<a href=\"/\">%s</a>] [<a href=\"/index.ps3\">%s</a>] [<a href=\"/games.ps3\">Slider</a>] [<a href=\"/setup.ps3\">%s</a>]</b>", STR_TRADBY, STR_FILES, STR_GAMES, STR_SETUP ); strcat(buffer, templn);
	#endif
#endif

}

static void handleclient(u64 conn_s_p)
{
	int conn_s = (int)conn_s_p; // main communications socket


	char param[HTML_RECV_SIZE];
	struct CellFsStat buf;
	int fd;

	if(conn_s_p==START_DAEMON || conn_s_p==REFRESH_CONTENT)
	{

#ifndef ENGLISH_ONLY
		update_language();
#endif

		if(conn_s_p==START_DAEMON)
		{
			if(profile || (!(webman_config->wmdn) && strlen(STR_WMSTART)>0))
			{
				sys_timer_sleep(10);
				sprintf(param, "%s%s", STR_WMSTART, SUFIX2(profile));
				show_msg((char*)param);
			}
		}
		else //if(conn_s_p==REFRESH_CONTENT)
		{
			{DELETE_CACHED_GAMES} // refresh XML will force "refresh HTML" to rebuild the cache file
		}

		init_running = 1;

		cellFsMkdir((char*)WMTMP, MODE);

		//identify covers folders to be scanned
#ifndef ENGLISH_ONLY
													covers_exist[0]=isDir(COVERS_PATH);
#endif
		sprintf(param, "%s/covers", MM_ROOT_STD) ;	covers_exist[1]=isDir(param);
		sprintf(param, "%s/covers", MM_ROOT_STL) ;	covers_exist[2]=isDir(param);
		sprintf(param, "%s/covers", MM_ROOT_SSTL);	covers_exist[3]=isDir(param);
													covers_exist[4]=isDir("/dev_hdd0/GAMES/covers");
													covers_exist[5]=isDir("/dev_hdd0/GAMEZ/covers");
													covers_exist[6]=isDir(WMTMP);

		for(u8 i=0; i<12; i++)
		{
			if(cellFsStat(wm_icons[i], &buf)!=CELL_FS_SUCCEEDED)
			{
				sprintf(param, "/dev_flash/vsh/resource/explore/icon/%s", wm_icons[i] + 23); strcpy(wm_icons[i], param);
				if(cellFsStat(param, &buf)==CELL_FS_SUCCEEDED) continue;
				else
				if(i==0 || i==5) strcpy(wm_icons[i] + 32, "user/024.png\0"); else //ps3
				if(i==1 || i==6) strcpy(wm_icons[i] + 32, "user/026.png\0"); else //psx
				if(i==2 || i==7) strcpy(wm_icons[i] + 32, "user/025.png\0"); else //ps2
				if(i==3 || i==8) strcpy(wm_icons[i] + 32, "user/022.png\0"); else //psp
				if(i==4 || i==9) strcpy(wm_icons[i] + 32, "user/023.png\0"); else //dvd
								 strcpy(wm_icons[i] + 37, "icon_home.png\0"); //setup / eject
			}
		}

#ifdef NOSINGSTAR
		if(webman_config->noss) no_singstar_icon();
#endif

#ifdef COBRA_ONLY
		//if(cobra_mode)
		{
			u8 cconfig[15];
			CobraConfig *cobra_config = (CobraConfig*) cconfig;
			memset(cobra_config, 0, 15);
			cobra_read_config(cobra_config);

			if(webman_config->nospoof)
			{
				cobra_config->spoof_version=0;
				cobra_config->spoof_revision=0;
			}
			else
			{   // cobra spoofer not working on 4.53
    			if(c_firmware!=4.53f)
				{
					cobra_config->spoof_version=0x0476;
					cobra_config->spoof_revision=65514;
				}
			}

			if( cobra_config->ps2softemu == 0 && cobra_get_ps2_emu_type()==PS2_EMU_SW )
				cobra_config->ps2softemu =  1;

			cobra_write_config(cobra_config);

		}
#endif

#ifdef SPOOF_CONSOLEID
		spoof_idps_psid();
#endif

#ifdef COBRA_ONLY
	#ifdef REMOVE_SYSCALLS
		if(webman_config->spp & 1) //remove syscalls & history
        {
			sys_timer_sleep(5);

			remove_cfw_syscalls();
			delete_history(true);
		}
		else
	#endif
		if(webman_config->spp & 2) //remove history only
		{
			delete_history(false);
			block_online_servers();
		}
#endif

		if(update_mygames_xml(conn_s_p)) mount_autoboot();

		init_running = 0;
		sys_ppu_thread_exit(0);
	}

#ifdef USE_DEBUG
	ssend(debug_s, "waiting...");
#endif
	if(loading_html>10) loading_html=0;
	//while((init_running/* || loading_html>3*/) && working) sys_timer_usleep(10000);

	sys_net_sockinfo_t conn_info_main;


#ifdef WM_REQUEST
	u8 wm_request=(cellFsStat((char*)"/dev_hdd0/tmp/wm_request", &buf) == CELL_FS_SUCCEEDED);

	if(wm_request)
#endif
	{
		sys_net_get_sockinfo(conn_s, &conn_info_main, 1);

		char ip_address[16];
		sprintf(ip_address, "%s", inet_ntoa(conn_info_main.remote_adr));
		if( webman_config->bind && ((conn_info_main.local_adr.s_addr!=conn_info_main.remote_adr.s_addr) && strncmp(ip_address, webman_config->allow_ip, strlen(webman_config->allow_ip))!=0))
		{
			sclose(&conn_s);
			loading_html--;
			sys_ppu_thread_exit(0);
		}

		if(!webman_config->netd0 && !webman_config->neth0[0]) strcpy(webman_config->neth0, ip_address);
		if(!webman_config->bind) strcpy(webman_config->allow_ip, ip_address);
	}

	set_buffer_sizes(webman_config->foot);

	_meminfo meminfo;
	u8 retries=0;
again3:
	{system_call_1(SC_GET_FREE_MEM, (uint64_t)(u32) &meminfo);}
	if((meminfo.avail)<( (_64KB_) + MIN_MEM)) //leave if less than min memory
	{
#ifdef USE_DEBUG
	ssend(debug_s, "!!! NOT ENOUGH MEMORY!\r\n");
#endif
		retries++;
		sys_timer_sleep(1);
		if(retries<5) goto again3;
		init_running = 0;
		sclose(&conn_s);
		loading_html--;
		sys_ppu_thread_exit(0);
	}

	sys_addr_t sysmem=0;

	u8 is_binary = 0, served=0;	// served http requests
	u64 c_len = 0;
	char cmd[16], header[HTML_RECV_SIZE];

	u8 is_ps3_http=0;
	u8 is_cpursx=0;
	u8 is_popup=0;

#ifdef WM_REQUEST
	if(!wm_request)
#endif
	{
		struct timeval tv;
		tv.tv_usec = 0;
		tv.tv_sec = 3;
		setsockopt(conn_s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	}

	while(!served && working)
	{
		served++;
		header[0]=0;

#ifdef USE_DEBUG
	ssend(debug_s, "ACC - ");
#endif

#ifdef WM_REQUEST
		if(wm_request)
		{
			if(buf.st_size > 5 && buf.st_size < HTML_RECV_SIZE && cellFsOpen((char*)"/dev_hdd0/tmp/wm_request", CELL_FS_O_RDONLY, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
			{
				cellFsRead(fd, (void *)header, buf.st_size, NULL);
				cellFsClose(fd); for(size_t n = buf.st_size; n > 4; n--) if(header[n]==' ') header[n]=9;
			}
			cellFsUnlink((char*)"/dev_hdd0/tmp/wm_request");
		}
#endif

		if(((header[0]=='G') || recv(conn_s, header, HTML_RECV_SIZE, 0) > 0) && header[0]=='G' && header[4]=='/') // serve only GET /xxx requests
		{
			if(strstr(header, "Mozilla/5.0 (PLAYSTATION 3;")) is_ps3_http=1; else
			if(strstr(header, "Gecko/36")) is_ps3_http=2; else is_ps3_http=0;

			header[strcspn(header, "\n")] = '\0';
			header[strcspn(header, "\r")] = '\0';

			ssplit(header, cmd, 15, header, (HTML_RECV_SIZE-1));
			ssplit(header, param, (HTML_RECV_SIZE-1), cmd, 15);

#ifdef WM_REQUEST
			if(wm_request) { for(size_t n = strlen(param); n > 0; n--) {if(param[n]==9) param[n]=' ';} } wm_request = 0;
#endif

			bool allow_retry_response=true, small_alloc = true, mobile_mode = false;

#ifdef USE_DEBUG
	ssend(debug_s, param);
	ssend(debug_s, "\r\n");
#endif
			//url decode (unescape)
			if(strstr(param, "%"))
			{
				strcpy(header, param);

				u16 pos=0, len=strlen(param);
				for(u16 i=0;i<len;i++, pos++)
				{
					if(header[i]!='%')
						param[pos]=header[i];
					else
					{
						i++;
						if(header[i]>='0' && header[i]<='9') param[pos]=(header[i]-0x30)*0x10; else
						if(header[i]>='A' && header[i]<='F') param[pos]=(header[i]-0x37)*0x10; else
						if(header[i]>='a' && header[i]<='f') param[pos]=(header[i]-0x57)*0x10;

						i++;
						if(header[i]>='0' && header[i]<='9') param[pos]+=header[i]-0x30; else
						if(header[i]>='A' && header[i]<='F') param[pos]+=header[i]-0x37; else
						if(header[i]>='a' && header[i]<='f') param[pos]+=header[i]-0x57;
					}
				}
				param[pos]=0;
			}

 #ifdef VIRTUAL_PAD
			if(strstr(param, "/pad.ps3"))
			{
				if(!webman_config->nopad) parse_pad_command(param);

				is_binary=0;
				http_response(conn_s, header, param, 200, param+9);
				loading_html--;
				sys_ppu_thread_exit(0);
				return;
			}
#endif

#ifndef LITE_EDITION
 #ifdef WEB_CHAT
			if(strstr(param, "/chat.ps3"))
			{
				is_popup=1; is_binary=0;
				goto html_response;
			}
 #endif
			if(strstr(param, "/popup.ps3"))
			{
				if(param[10]==0) show_info_popup = true; else is_popup=1;
				is_binary=0;
				goto html_response;
			}
			if(strstr(param, "/dev_blind"))
			{
				is_binary=2;
				goto html_response;
			}
			if(strstr(param, "/mkdir.ps3") || strstr(param, "/rmdir.ps3"))
			{
				if(strstr(param, "/rmdir.ps3"))
				{
					if(param[10]=='/') {sprintf(param, "%s", param+10); cellFsRmdir((char*)param); for(u16 i = strlen(param); i > 0; i--) if(param[i]=='/') {param[i]=0; break;}}
					else {delete_history(true); sprintf(param, "/dev_hdd0");}
				}
				else if(param[10]=='/') {sprintf(param, "%s", param+10); cellFsMkdir((char*)param, MODE);}
				else
				{
					sprintf(param, "/dev_hdd0");

					cellFsMkdir((char*)"/dev_hdd0/packages", MODE);
					cellFsMkdir((char*)"/dev_hdd0/GAMES", MODE);
					cellFsMkdir((char*)"/dev_hdd0/PS3ISO", MODE);
					cellFsMkdir((char*)"/dev_hdd0/PSXISO", MODE);
					cellFsMkdir((char*)"/dev_hdd0/PS2ISO", MODE);
					cellFsMkdir((char*)"/dev_hdd0/PSPISO", MODE);
					cellFsMkdir((char*)"/dev_hdd0/DVDISO", MODE);
					cellFsMkdir((char*)"/dev_hdd0/BDISO", MODE);
				}
			}
#endif
			if(strstr(param, "quit.ps3"))
			{
				http_response(conn_s, header, param, 200, param);
#ifdef LOAD_PRX
quit:
#endif
				if(!webman_config->fanc || strstr(param, "?0") || webman_config->ps2temp<33)
					restore_fan(0); //restore syscon fan control mode
				else
					restore_fan(1); //set ps2 fan control mode

				working = 0;
				sclose(&conn_s);
				if(sysmem) sys_memory_free(sysmem);
				loading_html=0;

				stop_prx_module();
				sys_ppu_thread_exit(0);
				break;
			}

			if(strstr(param, "shutdown.ps3"))
			{
				http_response(conn_s, header, param, 200, param);
				working = 0;
				{ DELETE_TURNOFF } { BEEP1 }

				if(strstr(param, "?"))
					vshmain_87BB0001(1); // shutdown using VSH
				else
					{system_call_4(SC_SYS_POWER, SYS_SHUTDOWN, 0, 0, 0);}

				sys_ppu_thread_exit(0);
				break;
			}
			if(strstr(param, "rebuild.ps3"))
			{
				http_response(conn_s, header, param, 200, param);

				cmd[0] = cmd[1] = 0; cmd[2] = 0x03; cmd[3] = 0xE9; // 00 00 03 E9
				savefile((char*)"/dev_hdd0/mms/db.err", cmd, 4);
				goto restart;
			}
			if(strstr(param, "recovery.ps3"))
			{
				#define SC_UPDATE_MANAGER_IF				863
				#define UPDATE_MGR_PACKET_ID_READ_EPROM		0x600B
				#define UPDATE_MGR_PACKET_ID_WRITE_EPROM	0x600C
				#define RECOVER_MODE_FLAG_OFFSET			0x48C61

				http_response(conn_s, header, param, 200, param);

				{system_call_7(SC_UPDATE_MANAGER_IF, UPDATE_MGR_PACKET_ID_WRITE_EPROM, RECOVER_MODE_FLAG_OFFSET, 0x00, 0, 0, 0, 0);} // set recovery mode
				goto reboot; // hard reboot
			}
			if(strstr(param, "restart.ps3"))
			{
				http_response(conn_s, header, param, 200, param);
restart:
				working = 0;
				{ DELETE_TURNOFF } { BEEP2 }
				if(strstr(param,"?0")==NULL) savefile((char*)WMNOSCAN, NULL, 0);

				vshmain_87BB0001(2); // VSH reboot

				sys_ppu_thread_exit(0);
				break;
			}
			if(strstr(param, "reboot.ps3"))
			{
				http_response(conn_s, header, param, 200, param);
reboot:
				working = 0;
				{ DELETE_TURNOFF } { BEEP2 }

				if(strstr(param, "?v")) vshmain_87BB0001(2); // VSH reboot
				else
				if(strstr(param, "?q"))
					{system_call_3(SC_SYS_POWER, SYS_REBOOT, NULL, 0);} // (quick reboot) load LPAR id 1
				else
				if(strstr(param, "?s"))
					{system_call_3(SC_SYS_POWER, SYS_SOFT_REBOOT, NULL, 0);} // soft reboot
				else
					{system_call_3(SC_SYS_POWER, SYS_HARD_REBOOT, NULL, 0);} // hard reboot

				sys_ppu_thread_exit(0);
				break;
			}

#ifdef FIX_GAME
			if(strstr(param, "/fixgame.ps3"))
			{
				// fix game folder
                char *game_path = param + 12;
				if(cellFsStat((char*)game_path, &buf)==CELL_FS_SUCCEEDED)
				{
					fix_in_progress=true; fix_aborted=false;
#ifdef COBRA_ONLY
					if(strcasestr(game_path, ".iso"))
						fix_iso(game_path, 0x100000UL, false);
					else
#endif //#ifdef COBRA_ONLY
						fix_game(game_path);

					fix_in_progress=false;

					is_popup=1; is_binary=0;
					goto html_response;
				}
			}
#endif

			if(strstr(param, "/games.ps3"))
			{
mobile_response:
				mobile_mode = false;

				if(cellFsStat((char*)MOBILE_HTML, &buf)!=CELL_FS_SUCCEEDED)
					sprintf(param, "/index.ps3%s", param+10);
				else if(strstr(param, "?g="))
					sprintf(param, "%s", MOBILE_HTML);
				else if(strstr(param, "?"))
					{sprintf(param, "/index.ps3%s", param+10); mobile_mode = true;}
				else if(cellFsStat((char*)GAMELIST_JS, &buf)!=CELL_FS_SUCCEEDED)
					sprintf(param, "%s", "index.ps3?mobile");
				else
					sprintf(param, "%s", MOBILE_HTML);
			}
			else mobile_mode = false;

			if(strstr(param, "index.ps3")!=NULL) small_alloc=false;

			if(!is_busy && (strstr(param, "index.ps3?")  ||
#ifdef DEBUG_MEM
							strstr(param, "peek.lv2?")   ||
							strstr(param, "poke.lv2?")   ||
							strstr(param, "find.lv2?")   ||
							strstr(param, "peek.lv1?")   ||
							strstr(param, "poke.lv1?")   ||
							strstr(param, "find.lv1?")   ||
							strstr(param, "dump.ps3")    ||
#endif

#ifndef LITE_EDITION
							strstr(param, "delete.ps3")  ||
							strstr(param, "delete_ps3")  ||
#endif

#ifdef PS3MAPI
							strstr(param, "home.ps3mapi")     ||
							strstr(param, "setmem.ps3mapi")   ||
							strstr(param, "getmem.ps3mapi")   ||
							strstr(param, "led.ps3mapi")      ||
							strstr(param, "buzzer.ps3mapi")   ||
							strstr(param, "notify.ps3mapi")   ||
							strstr(param, "syscall.ps3mapi")  ||
							strstr(param, "syscall8.ps3mapi") ||
							strstr(param, "setidps.ps3mapi")  ||
							strstr(param, "vshplugin.ps3mapi")  ||
							strstr(param, "gameplugin.ps3mapi") ||
#endif

#ifdef COPY_PS3
							strstr(param, "copy.ps3/") ||
#endif
							strstr(param, "refresh.ps3")
			))
				is_binary=0;
			else if(strstr(param, "cpursx.ps3")  ||
					strstr(param, "index.ps3")   ||
					strstr(param, "setup.ps3")   ||
					strstr(param, "mount_ps3/")  ||
					strstr(param, "mount.ps3/")  ||
#ifdef PS2_DISC
					strstr(param, "mount.ps2/")  ||
					strstr(param, "mount_ps2/")  ||
#endif

#ifdef VIDEO_REC
					strstr(param, "videorec.ps3") ||
#endif

#ifdef EXT_GDATA
					strstr(param, "extgd.ps3")   ||
#endif

#ifdef SYS_BGM
					strstr(param, "sysbgm.ps3")  ||
#endif

#ifdef LOAD_PRX
					strstr(param, "loadprx.ps3") ||
#endif

					strstr(param, "eject.ps3")   ||
					strstr(param, "insert.ps3"))
				is_binary=0;
			else if(param[1]=='n' && param[2]=='e' && param[3]=='t' && (param[4]>='0' && param[4]<='2')) //net0/net1/net2
			{
				is_binary=2; small_alloc = false;
			}
			else
			{
				is_binary=(cellFsStat(param, &buf)==CELL_FS_SUCCEEDED);

				if(!is_binary) {strcpy(header, param);  sprintf(param, "%s/%s", html_base_path, header); is_binary=(cellFsStat(param, &buf)==CELL_FS_SUCCEEDED);} // use html path (if path is omitted)

				if(is_binary)
				{
					c_len=buf.st_size;
					if((buf.st_mode & S_IFDIR) != 0) {is_binary=2; small_alloc = false;} // folder listing
				}
				else
				{
					c_len=0;
					is_binary=0;
					http_response(conn_s, header, param, is_busy ? 503:400, is_busy ? (char*)"503 Server is Busy":(char*)"400 Bad Request");
					loading_html--;
					sys_ppu_thread_exit(0);
				}
			}

html_response:
			prepare_header(header, param, is_binary);
			char templn[1024];
			{u16 ulen=strlen(param); if(ulen>1 && param[ulen-1]=='/') param[ulen-1]=0;}
			//sprintf(templn, "X-PS3-Info: %llu [%s]\r\n", (unsigned long long)c_len, param); strcat(header, templn);

			//-- select content profile
			if(strstr(param, ".ps3?"))
			{
				u8 uprofile=profile; char url[10]; bool is_index_ps3 = (strstr(param, "index.ps3?")!=NULL);

				for(u8 i=0;i<5;i++)
				{
					sprintf(url, "?%i", i); if(strstr(param, url)) {profile=i; break;}
					sprintf(url, "usr=%i", i); if(strstr(param, url)) {profile=i; break;}
					if(is_index_ps3) {sprintf(url, "_%i", i); if(strstr(param, url)) {profile=i; break;}}
				}

				if(uprofile!=profile) {webman_config->profile=profile; save_settings();}
				if((uprofile!=profile) || is_index_ps3) {DELETE_CACHED_GAMES}
			}
			//--

			if(is_binary==1) //file
			{
				sprintf(templn, "Content-Length: %llu\r\n\r\n", (unsigned long long)c_len); strcat(header, templn);
				ssend(conn_s, header);
				if(!sysmem && sys_memory_allocate(_64KB_, SYS_MEMORY_PAGE_SIZE_64K, &sysmem)!=0)
				{
					sclose(&conn_s);
					loading_html--;
					sys_ppu_thread_exit(0);
				}
				char *buffer= (char*)sysmem;
				if(cellFsOpen(param, CELL_FS_O_RDONLY, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
				{
					u64 read_e = 0, pos;
					cellFsLseek(fd, 0, CELL_FS_SEEK_SET, &pos);

					while(working)
					{
						//sys_timer_usleep(500);
						if(cellFsRead(fd, (void *)buffer, _64KB_, &read_e)==CELL_FS_SUCCEEDED)
						{
							if(read_e>0)
							{
								if(send(conn_s, buffer, (size_t)read_e, 0)<0) break;
							}
							else
								break;
						}
						else
							break;
					}
					cellFsClose(fd);
				}
				sys_memory_free(sysmem);
				sclose(&conn_s);
				loading_html--;
				sys_ppu_thread_exit(0);

			}
			if(strstr(param, "cpursx.ps3") || show_info_popup)
			{
				if(!sysmem && sys_memory_allocate(_64KB_, SYS_MEMORY_PAGE_SIZE_64K, &sysmem)!=0)
				{
					sclose(&conn_s);
					loading_html--;
					sys_ppu_thread_exit(0);
				}
				is_cpursx=1;
			}
			else
			{
				if(!small_alloc)
				{
					set_buffer_sizes(webman_config->foot);

					_meminfo meminfo;
					{system_call_1(SC_GET_FREE_MEM, (uint64_t)(u32) &meminfo);}

					if((meminfo.avail)<( (BUFFER_SIZE_ALL) + MIN_MEM)) set_buffer_sizes(3); //MIN+
					if((meminfo.avail)<( (BUFFER_SIZE_ALL) + MIN_MEM)) set_buffer_sizes(1); //MIN
				}

				if(!sysmem && sys_memory_allocate(small_alloc?_64KB_:BUFFER_SIZE_ALL, SYS_MEMORY_PAGE_SIZE_64K, &sysmem)!=0)
				{
					sclose(&conn_s);
					loading_html--;
					sys_ppu_thread_exit(0);
				}
			}

			char *buffer= (char*)sysmem;
			//else	// text page
			{
				if(is_binary!=2 && strstr(param, "setup.ps3?"))
				{
					setup_parse_settings(param);
				}

				bool mount_ps3 = !is_popup && (strstr(param, "mount_ps3")!=NULL), forced_mount = false;

				if(mount_ps3 && View_Find("game_plugin")) {mount_ps3=false; forced_mount=true;}

				prepare_html(buffer, templn, param, is_ps3_http, is_cpursx, mount_ps3);

				if(is_cpursx)
				{
					cpu_rsx_stats(buffer, templn, param);

					is_cpursx = 0; goto send_response;

					//CellGcmConfig config; cellGcmGetConfiguration(&config);
					//sprintf(templn, "localAddr: %x", (u32) config.localAddress); strcat(buffer, templn);
				}
				else if(!mount_ps3)
				{
					u32 t1=0, t2=0;
					get_temperature(0, &t1); // 3E030000 -> 3E.03°C -> 62.(03/256)°C
					get_temperature(1, &t2);
					t1=t1>>24;
					t2=t2>>24;

					sprintf(templn, " [<a href=\"/cpursx.ps3\">CPU: %i°C | RSX: %i°C</a>]<hr>"
									"<form action=\"\">", t1, t2); strcat(buffer, templn);

					if((webman_config->homeb) && (strlen(webman_config->home_url)>0))
					{sprintf(templn, HTML_BUTTON_FMT, HTML_BUTTON, STR_HOME, HTML_ONCLICK, webman_config->home_url); strcat(buffer, templn);}

					sprintf(templn, HTML_BUTTON_FMT
									HTML_BUTTON_FMT
									HTML_BUTTON_FMT
#ifdef EXT_GDATA
									HTML_BUTTON_FMT
#endif
									, HTML_BUTTON, STR_EJECT, HTML_ONCLICK, "/eject.ps3"
									, HTML_BUTTON, STR_INSERT, HTML_ONCLICK, "/insert.ps3"
									, HTML_BUTTON, STR_UNMOUNT, HTML_ONCLICK, "/mount.ps3/unmount"
#ifdef EXT_GDATA
									, HTML_BUTTON, "gameDATA", HTML_ONCLICK, "/extgd.ps3"
#endif
                    ); strcat(buffer, templn);
#ifdef COPY_PS3
					if(((strstr(param, "/dev_") && strlen(param)>12 && !strstr(param,"?")) || strstr(param, "/dev_bdvd")) && !strstr(param,".ps3/") && !strstr(param,".ps3?"))
					{sprintf(templn, "%s%s\" onclick='window.location.href=\"/copy.ps3%s\";'\">", HTML_BUTTON, STR_COPY, param); strcat(buffer, templn);}
#ifndef LITE_EDITION
					if((strstr(param, "/dev_") && !strstr(param,"?")) && !strstr(param,"/dev_flash") && !strstr(param,".ps3/") && !strstr(param,".ps3?"))
					{sprintf(templn,"<script type=\"text/javascript\">"
									"function toggle(){"
									"btnDel.value=(btnDel.value=='%s')?'%s %s':'%s';"
									"var i,p,l=document.querySelectorAll('.d,.w');"
									"for(i=1;i<l.length;i++)"
									"{p=l[i].href.indexOf('/delete.ps3');"
									"if(p>0){l[i].href=l[i].href.substring(p+11,l[i].href.length);l[i].style.color='#ccc';}"
									"else{p=l[i].href.indexOf('/', 8);l[i].href='/delete.ps3'+l[i].href.substring(p,l[i].href.length);l[i].style.color='red';}}}"
									"</script>", STR_DELETE, STR_DELETE, STR_ENABLED, STR_DELETE); strcat(buffer, templn);
					 sprintf(templn, "%s%s\" id='btnDel' onclick='toggle();'>", HTML_BUTTON, STR_DELETE); strcat(buffer, templn);
					}
#endif
#endif
					sprintf(templn,  "%s%s XML%s\" %s'%s';\"> "
									 "%s%s HTML%s\" %s'%s';\">"
									 HTML_BUTTON_FMT
									 HTML_BUTTON_FMT
									 "</form><hr>",
									 HTML_BUTTON, STR_REFRESH, SUFIX2(profile), HTML_ONCLICK, "/refresh.ps3",
									 HTML_BUTTON, STR_REFRESH, SUFIX2(profile), HTML_ONCLICK, "/index.ps3?html",
									 HTML_BUTTON, STR_SHUTDOWN, HTML_ONCLICK, "/shutdown.ps3",
									 HTML_BUTTON, STR_RESTART, HTML_ONCLICK, "/restart.ps3"); strcat(buffer, templn);
				}

				char tempstr[_4KB_];

#ifndef LITE_EDITION
				if(is_popup)
				{
#ifdef WEB_CHAT
					if(strstr(param, "/chat.ps3"))
					{
						webchat(buffer, templn, param, tempstr, conn_info_main);
					}
					else
#endif
#ifdef FIX_GAME
					if(strstr(param, "/fixgame.ps3"))
					{
						char *game_path = param + 12;
						sprintf(templn, "Fixed: %s", game_path);
						templn[211]=0; //limit message to 200 characters
						show_msg((char*)templn);
						sprintf(templn, "Fixed: <a href=\"%s\">%s</a>", game_path, game_path); strcat(buffer, templn);
					}
					else
#endif
					{
						param[211]=0; //limit message to 200 characters
						show_msg((char*)(param+11));
						sprintf(templn, "Message sent: %s", param+11); strcat(buffer, templn);
					}

					is_popup=0; goto send_response;
				}
#endif
				if(is_binary==2) // folder listing
				{
					if(folder_listing(buffer, templn, param, conn_s, tempstr, header) == false)
					{
						if(sysmem) sys_memory_free(sysmem);
						loading_html--;
						sys_ppu_thread_exit(0);
					}
				}
				else
				{
					is_busy=true;

					if(strstr(param, "refresh.ps3") && init_running==0)
					{
						init_running = 1;
						refresh_xml(templn);
						sprintf(templn,  "<br>%s", STR_XMLRF); strcat(buffer, templn);
					}
					else
					if(strstr(param, "eject.ps3"))
					{
						eject_insert(1, 0);
						strcat(buffer, STR_EJECTED);
					}
					else
					if(strstr(param, "insert.ps3"))
					{
						eject_insert(0, 1);
						strcat(buffer, STR_LOADED);
					}
#ifdef LOAD_PRX
					else
					if(strstr(param, "loadprx.ps3"))
					{
						char *pos; unsigned int slot=6; bool prx_found;

						pos=strstr(param, "slot=");
						if(pos)
						{
							get_value(templn, pos + 5, 2);
							slot=RANGE((unsigned int)val(templn), 1, 6);
						}

						if(param[12]=='/') sprintf(templn, "%s", param+12);
						else
						{
							sprintf(templn, "/dev_hdd0/plugins/webftp_server.sprx");
							if(cellFsStat(templn, &buf)!=CELL_FS_SUCCEEDED) sprintf(templn, "/dev_hdd0/webftp_server.sprx");

							pos=strstr(param, "prx=");
							if(pos) get_value(templn, pos + 4, MAX_PATH_LEN);
						}

						prx_found = (cellFsStat(templn, &buf)==CELL_FS_SUCCEEDED);

						if(prx_found)
							sprintf(param, "slot: %i<br>load prx: %s", slot, templn);
						else
							sprintf(param, "unload slot: %i", slot);

						strcat(buffer, param); strcat(buffer, "</font></body></html>");

						cobra_unload_vsh_plugin(slot);

						if(prx_found)
							{cobra_load_vsh_plugin(slot, templn, NULL, 0); if(strstr(templn, "/webftp_server")) goto quit;}
					}
#endif

#ifdef VIDEO_REC
					else
					if(strstr(param, "videorec.ps3"))
					{
						toggle_video_rec();
						strcat(buffer,	"<a class=\"f\" href=\"/dev_hdd0\">/dev_hdd0/</a><a href=\"/dev_hdd0/VIDEO\">VIDEO</a>:<p>"
										"Video recording: <a href=\"/videorec.ps3\">");
						strcat(buffer, recording?STR_ENABLED:STR_DISABLED);
						strcat(buffer, "</a>");
					}
#endif

#ifdef EXT_GDATA
					else
					if(strstr(param, "extgd.ps3"))
					{
						if(strstr(param,"?s" /*status */ )); else
						if(strstr(param,"?e" /*enable */ ) || strstr(param, "?1"))  extgd=1; else
						if(strstr(param,"?d" /*disable*/ ) || strstr(param, "?0"))  extgd=0; else
																					extgd=extgd^1;

						strcat(buffer, "External Game DATA: ");
						if(set_gamedata_status(extgd, true))
							strcat(buffer, STR_ERROR);
						else
							strcat(buffer, extgd?STR_ENABLED:STR_DISABLED);
					}
#endif

#ifdef SYS_BGM
					else
					if(strstr(param, "sysbgm.ps3"))
					{
						static int system_bgm=0;

						if(system_bgm==0)
						{
							system_bgm=-1;
							BgmPlaybackEnable  = (void*)((int)getNIDfunc("vshmain", 0xEDAB5E5E, 16*2));
							BgmPlaybackDisable = (void*)((int)getNIDfunc("vshmain", 0xEDAB5E5E, 17*2));
						}

						if(strstr(param, "?1") || strstr(param, "?e")) system_bgm=-1; //enable
						if(strstr(param, "?0") || strstr(param, "?d")) system_bgm=1;  //disable

						if(strstr(param, "?s")) goto bgm_status;

						int * arg2;
						if(system_bgm<0)
						{
							BgmPlaybackEnable(0, &arg2); system_bgm=1;
						}
						else
						{
							BgmPlaybackDisable(0, &arg2); system_bgm=-1;
						}

bgm_status:
						sprintf(templn, "System BGM: %s", (system_bgm>0)?STR_ENABLED:STR_DISABLED);
						strcat(buffer, templn);
						show_msg(templn);
					}
#endif

#ifdef DEBUG_MEM
					else
					if(strstr(param, "dump.ps3"))
					{
						ps3mapi_mem_dump(buffer, templn, param);
					}
					else
					if(strstr(param, "peek.lv") || strstr(param, "poke.lv") || strstr(param, "find.lv"))
					{
						ps3mapi_find_peek_poke(buffer, templn, param);
					}
#endif
					else
					if(strstr(param, "setup.ps3?"))
					{
						if(strstr(param, "&")==NULL)
						{
							cellFsUnlink(WMCONFIG);
							reset_settings();
						}
						if(save_settings() == CELL_FS_SUCCEEDED)
						{
							sprintf(templn, "<br> %s", STR_SETTINGSUPD); strcat(buffer, templn);
						}
						else
							strcat(buffer, STR_ERROR);
					}
					else
					if(strstr(param, "setup.ps3"))
					{
                        setup_form(buffer, templn);
					}
#ifndef LITE_EDITION
					else
					if(strstr(param, "/delete.ps3") || strstr(param, "/delete_ps3"))
					{
						bool is_reset = false;
						if(strstr(param, "?wmreset")) is_reset=true;
						if(is_reset || strstr(param, "?wmconfig")) {cellFsUnlink(WMCONFIG); reset_settings(); sprintf(param, "/delete_ps3%s", WMCONFIG);}
						if(is_reset || strstr(param, "?wmtmp")) strcpy(param, "/delete_ps3/dev_hdd0/tmp/wmtmp\0");

						if(strstr(param, "?history"))
						{
							delete_history(true);
							sprintf(templn, "%s : history", STR_DELETE);
						}
						else if(strstr(param, "?uninstall"))
						{
							if(cellFsStat((char*)"/dev_hdd0/boot_plugins.txt", &buf)==CELL_FS_SUCCEEDED && buf.st_size<40) cellFsUnlink((char*)"/dev_hdd0/boot_plugins.txt");
							cellFsUnlink((char*)"/dev_hdd0/webftp_server.sprx");
							cellFsUnlink((char*)"/dev_hdd0/plugins/webftp_server.sprx");
							cellFsUnlink((char*)WMCONFIG);
							del((char*)WMTMP, true);
							del((char*)"/dev_hdd0/xmlhost", true);
							del((char*)"/dev_hdd0/tmp/wm_lang", true);
							del((char*)"/dev_hdd0/tmp/wm_icons", true);
							http_response(conn_s, header, param, 200, param);
							goto restart;
						}
						else if(del(param+11, (strstr(param, "/delete.ps3")!=NULL)))
						{
							sprintf(tempstr, "%s", param+11); if(strchr(tempstr, '/')) tempstr[strrchr(tempstr, '/')-tempstr]=0;
							sprintf(templn, "%s %s : <a href=\"%s\">%s</a><a href=\"%s\">%s</a><br>", STR_DELETE, STR_ERROR, tempstr, tempstr, param+11+strlen(tempstr), param+11+strlen(tempstr));
						}
						else
						{
							sprintf(tempstr, "%s", param+11); if(strchr(tempstr, '/')) tempstr[strrchr(tempstr, '/')-tempstr]=0;
							sprintf(templn, "%s : <a href=\"%s\">%s</a>%s<br>", STR_DELETE, tempstr, tempstr, param+11+strlen(tempstr));
						}
						strcat(buffer, templn);
					}
#endif

#ifdef PS3MAPI
					else
					if(strstr(param, "home.ps3mapi"))
					{
						ps3mapi_home(buffer, templn);
					}
					else
					if(strstr(param, "buzzer.ps3mapi"))
					{
						ps3mapi_buzzer(buffer, templn, param);
					}
					else
					if(strstr(param, "led.ps3mapi"))
					{
						ps3mapi_led(buffer, templn, param);
					}
					else
					if(strstr(param, "notify.ps3mapi"))
					{
						ps3mapi_notify(buffer, templn, param);
					}
					else
					if(strstr(param, "syscall.ps3mapi"))
					{
						ps3mapi_syscall(buffer, templn, param);
					}
					else
					if(strstr(param, "syscall8.ps3mapi"))
					{
						ps3mapi_syscall8(buffer, templn, param);
					}
					else
					if(strstr(param, "getmem.ps3mapi"))
					{
						ps3mapi_getmem(buffer, templn, param);
					}
					else
					if(strstr(param, "setmem.ps3mapi"))
					{
						ps3mapi_setmem(buffer, templn, param);
					}
					else
					if(strstr(param, "setidps.ps3mapi"))
					{
						ps3mapi_setidps(buffer, templn, param);
					}
					else
					if(strstr(param, "vshplugin.ps3mapi"))
					{
						ps3mapi_vshplugin(buffer, templn, param);
					}
					else
					if(strstr(param,"gameplugin.ps3mapi"))
					{
						ps3mapi_gameplugin(buffer, templn, param);
					}
#endif

					else
#ifdef PS2_DISC
					if(mount_ps3 || forced_mount || !memcmp(param, "/mount.ps3", 10) || !memcmp(param, "/mount.ps2", 10) || !memcmp(param, "/mount_ps2", 10) || !memcmp(param, "/copy.ps3", 9))
#else
					if(mount_ps3 || forced_mount || !memcmp(param, "/mount.ps3", 10) || !memcmp(param, "/copy.ps3", 9))
#endif
					{
                        game_mount(buffer, templn, param, tempstr, is_binary, mount_ps3, forced_mount);
					}
					else
					{
						mobile_mode|=(strstr(param, "?mob")!=NULL || strstr(param, "&mob")!=NULL);

						if(game_listing(buffer, templn, param, conn_s, tempstr, mobile_mode) == false)
						{
							is_busy=false;
							if(sysmem) sys_memory_free(sysmem);
							loading_html--;
							sys_ppu_thread_exit(0);
							break;
                        }
					}

					is_busy=false;
				}

send_response:
				if(mobile_mode && allow_retry_response) {allow_retry_response=false; goto mobile_response;}

				if(mount_ps3)
					strcat(buffer, "<script type=\"text/javascript\">window.close(this);</script>"); //auto-close
				else
					strcat(buffer, "</font></body></html>"); //end-html

				sprintf(templn, "Content-Length: %llu\r\n\r\n", (unsigned long long)strlen(buffer)); strcat(header, templn);
				ssend(conn_s, header);
				ssend(conn_s, buffer);
				buffer[0]=0;
			}
		}

		break;
	}

#ifdef USE_DEBUG
	ssend(debug_s, "Request served.\r\n");
#endif

	sclose(&conn_s);
	if(sysmem) sys_memory_free(sysmem);
	loading_html--;
	sys_ppu_thread_exit(0);
}

/*
	u64 sa=0x03300000ULL;
	u64 offset=0;
	for(u64 i=0;i<0x7700000;i+=8)
	{
		if( peek(0x8000000000000000ULL+i+sa	 )==0xFFFFFF00FFFFFF00ULL &&
			peek(0x8000000000000000ULL+i+sa+8)==0xFFFFFF00FFFFFF00ULL )
		{
			show_msg((char*)"Found match!");
			offset=i+sa+16;
			for(u64 i2=offset;i2<(offset+320*4*90);i2+=8)
				poke(0x8000000000000000ULL+i2, 0x30405060708090A0ULL);
		}
	}
*/

static void set_buffer_sizes(int footprint)
{
	if(footprint==1) //MIN
	{
#ifndef LITE_EDITION
		BUFFER_SIZE_ALL = ( 320*KB);
#else
		BUFFER_SIZE_ALL = ( _256KB_);
#endif
		BUFFER_SIZE_FTP	= ( _128KB_);
		//BUFFER_SIZE	= ( _128KB_);
		BUFFER_SIZE_PSX	= (  _32KB_);
		BUFFER_SIZE_PSP	= (  _32KB_);
		BUFFER_SIZE_PS2	= (  _64KB_);
		BUFFER_SIZE_DVD	= (  _64KB_);
	}
	else
	if(footprint==2) //MAX
	{
		BUFFER_SIZE_ALL = ( 1280*KB);
		BUFFER_SIZE_FTP	= ( _256KB_);
		//BUFFER_SIZE	= ( 512*KB);
		BUFFER_SIZE_PSX	= ( _192KB_);
		BUFFER_SIZE_PSP	= (  _64KB_);
		BUFFER_SIZE_PS2	= ( _128KB_);
		BUFFER_SIZE_DVD	= ( _256KB_);

		if((webman_config->cmask & PS1)) BUFFER_SIZE_PSX	= (_64KB_);
		if((webman_config->cmask & PS2)) BUFFER_SIZE_PS2	= (_64KB_);
		if((webman_config->cmask & (BLU | DVD)) == (BLU | DVD)) BUFFER_SIZE_DVD = (_64KB_);
	}
	else
	if(footprint==3) //MIN+
	{
		BUFFER_SIZE_ALL = ( 512*KB);
		BUFFER_SIZE_FTP	= ( _128KB_);
		//BUFFER_SIZE	= ( 320*KB);
		BUFFER_SIZE_PSX	= (  _32KB_);
		BUFFER_SIZE_PSP	= (  _32KB_);
		BUFFER_SIZE_PS2	= (  _64KB_);
		BUFFER_SIZE_DVD	= (  _64KB_);
	}
	else
	if(footprint==4) //MAX+
	{
		BUFFER_SIZE_ALL = ( 1280*KB);
		BUFFER_SIZE_FTP	= ( _128KB_);
		//BUFFER_SIZE	= ( 1088*KB);
		BUFFER_SIZE_PSX	= (  _32KB_);
		BUFFER_SIZE_PSP	= (  _32KB_);
		BUFFER_SIZE_PS2	= (  _64KB_);
		BUFFER_SIZE_DVD	= (  _64KB_);
	}
	else	//STANDARD
	{
		BUFFER_SIZE_ALL = ( 896*KB);
		BUFFER_SIZE_FTP	= ( _128KB_);
		//BUFFER_SIZE	= ( 448*KB);
		BUFFER_SIZE_PSX	= ( 160*KB);
		BUFFER_SIZE_PSP	= (  _32KB_);
		BUFFER_SIZE_PS2	= (  _64KB_);
		BUFFER_SIZE_DVD	= ( _192KB_);

		if((webman_config->cmask & PS1)) BUFFER_SIZE_PSX	= (_32KB_);
		if((webman_config->cmask & (BLU | DVD)) == (BLU | DVD)) BUFFER_SIZE_DVD = (_64KB_);
	}

	BUFFER_SIZE = BUFFER_SIZE_ALL - (BUFFER_SIZE_PSX + BUFFER_SIZE_PSP + BUFFER_SIZE_PS2 + BUFFER_SIZE_DVD);
}

static void wwwd_thread(uint64_t arg)
{

//	sys_timer_sleep(8);
//	u32 mode=0;
//	if(in_cobra(&mode)==0) cobra_mode=1;

	backup[0]=0;

	detect_firmware();

#ifdef COBRA_ONLY
	//cobra_lib_init();
	cobra_mode=1;
#else
	cobra_mode=0;
#endif

	View_Find = (void*)((int)getNIDfunc("paf", 0xF21655F3, 0));
	plugin_GetInterface = (void*)((int)getNIDfunc("paf", 0x23AFB290, 0));
#ifdef EXTRA_FEAT
	vsh_sprintf = (void*)((int)getNIDfunc("stdc", 0x273B9711, 0)); // sprintf
#endif

	//pokeq(0x8000000000003560ULL, 0x386000014E800020ULL); // li r3, 0 / blr
	//pokeq(0x8000000000003D90ULL, 0x386000014E800020ULL); // li r3, 0 / blr

	led(YELLOW, BLINK_FAST);

	WebmanCfg *webman_config = (WebmanCfg*) wmconfig;
	reset_settings();


	{struct CellFsStat buf; from_reboot = (cellFsStat((char*)WMNOSCAN, &buf)==CELL_FS_SUCCEEDED);} //is_rebug=isDir("/dev_flash/rebug");

	if(webman_config->blind) enable_dev_blind(NULL);

#ifdef COBRA_ONLY
	{sys_map_path((char*)"/app_home", NULL);}
	{sys_map_path((char*)"/dev_bdvd/PS3_UPDATE", (char*)"/dev_bdvd");} //redirect firmware update to root of bdvd
#endif

	set_buffer_sizes(webman_config->foot);

	init_running = 1;

	sys_ppu_thread_t id2;
	sys_ppu_thread_create(&id2, handleclient, (u64)START_DAEMON, -0x1d8, 0x20000, 0, "wwwd2");

	if(!webman_config->ftpd)
		sys_ppu_thread_create(&thread_id_ftp, ftpd_thread, NULL, -0x1d8,  0x2000, SYS_PPU_THREAD_CREATE_JOINABLE, THREAD_NAME_FTP);

#ifdef PS3MAPI
	///////////// PS3MAPI BEGIN //////////////
		sys_ppu_thread_create(&thread_id_ps3mapi, ps3mapi_thread, NULL, -0x1d8, 0x2000, SYS_PPU_THREAD_CREATE_JOINABLE, THREAD_NAME_PS3MAPI);
	///////////// PS3MAPI END //////////////
#endif

	led(YELLOW, OFF);
	sys_timer_sleep(5);

#ifdef USE_DEBUG
	u8 d_retries=0;
again_debug:
	debug_s=connect_to_server((char*)"192.168.100.209", 38009);
	if(debug_s<0) {d_retries++; sys_timer_sleep(2); if(d_retries<10) goto again_debug;}
	if(debug_s>=0) ssend(debug_s, "Connected...\r\n");
	sprintf(debug, "FC=%i T0=%i T1=%i\r\n", webman_config->fanc, webman_config->temp0, webman_config->temp1);
	ssend(debug_s, debug);
#endif

	max_temp=0;

	if(webman_config->fanc)
	{
		if(webman_config->temp0==0) max_temp=webman_config->temp1; else max_temp=0;
		fan_control(webman_config->temp0, 0);
	}

    sys_ppu_thread_create(&thread_id_poll, poll_thread, (u64)webman_config->poll, -0x1d8, 0x20000, SYS_PPU_THREAD_CREATE_JOINABLE, "poll_thread");

	while(init_running && working) sys_timer_usleep(100000);

	led(GREEN, ON);

//	{ DELETE_TURNOFF }

	int list_s = FAILED;

relisten:
#ifdef USE_DEBUG
	ssend(debug_s, "Listening on port 80...");
#endif
	if(working) list_s = slisten(WWWPORT, 4);
	else goto end;

	if(list_s<0)
	{
		if(working) {sys_timer_sleep(2); goto relisten;}
		else goto end;
	}

	if((list_s >= 0) && working)
	{
#ifdef USE_DEBUG
		ssend(debug_s, " OK!\r\n");
#endif

		while(working)
		{
			sys_timer_usleep(10000);
			while(loading_html>2 && working)
			{
#ifdef USE_DEBUG
	sprintf(debug, "THREADS: %i\r\n", loading_html);
	ssend(debug_s, debug);
#endif
				sys_timer_usleep(300000);
			}
			int conn_s;
			if(!working) goto end;
			else
			if((conn_s = accept(list_s, NULL, NULL)) > 0)
			{
				loading_html++;
				#ifdef USE_DEBUG
				ssend(debug_s, "*** Incoming connection... ");
				#endif
				sys_ppu_thread_t id;
				if(working) sys_ppu_thread_create(&id, handleclient, (u64)conn_s, -0x1d8, 0x20000, 0, "wwwd");
				else {sclose(&conn_s); break;}
			}
			else
			if((sys_net_errno==SYS_NET_EBADF) || (sys_net_errno==SYS_NET_ENETDOWN))
			{
				sclose(&list_s);
				list_s=FAILED;
				if(working) goto relisten;
				else break;
			}
		}

	}
end:
	sclose(&list_s);
	sys_ppu_thread_exit(0);
}


int wwwd_start(uint64_t arg)
{
	cellRtcGetCurrentTick(&rTick); gTick=rTick;

	sys_ppu_thread_create(&thread_id, wwwd_thread, NULL, -0x1d8, 0x4000, SYS_PPU_THREAD_CREATE_JOINABLE, THREAD_NAME);
#ifndef CCAPI
	_sys_ppu_thread_exit(0); // remove for ccapi compatibility
#endif
	return SYS_PRX_RESIDENT;
}

static void wwwd_stop_thread(uint64_t arg)
{
	while(init_running) sys_timer_usleep(500000); //Prevent unload too fast

	restore_fan(1); //restore & set static fan speed for ps2

	working = 0;
	sys_timer_usleep(500000);

	uint64_t exit_code;

/*
	sys_ppu_thread_t t;
 #ifndef LITE_EDITION
	sys_ppu_thread_create(&t, netiso_stop_thread, 0, 0, 0x2000, SYS_PPU_THREAD_CREATE_JOINABLE, STOP_THREAD_NAME);
	sys_ppu_thread_join(t, &exit_code);
 #endif
	sys_ppu_thread_create(&t, rawseciso_stop_thread, 0, 0, 0x2000, SYS_PPU_THREAD_CREATE_JOINABLE, STOP_THREAD_NAME);
	sys_ppu_thread_join(t, &exit_code);

	while(netiso_loaded || rawseciso_loaded) {sys_timer_usleep(100000);}
*/
	//if(thread_id != (sys_ppu_thread_t)-1)
		sys_ppu_thread_join(thread_id, &exit_code);

	if(thread_id_ftp != (sys_ppu_thread_t)-1)
		sys_ppu_thread_join(thread_id_ftp, &exit_code);

#ifdef PS3MAPI
	///////////// PS3MAPI BEGIN //////////////
	if(thread_id_ps3mapi != (sys_ppu_thread_t)-1)
		sys_ppu_thread_join(thread_id_ps3mapi, &exit_code);
	///////////// PS3MAPI END //////////////
#endif

	if(wm_unload_combo !=1)
	{
		if(thread_id_poll != (sys_ppu_thread_t)-1)
			sys_ppu_thread_join(thread_id_poll, &exit_code);
	}

	sys_ppu_thread_exit(0);
}

static void stop_prx_module(void)
{
	show_msg((char*)STR_WMUNL);

	sys_prx_id_t prx = prx_get_module_id_by_address(stop_prx_module);
	int *result = NULL;

	{system_call_6(SC_STOP_PRX_MODULE, (u64)(u32)prx, 0, NULL, (u64)(u32)result, 0, NULL);}
}

static void unload_prx_module(void)
{

	sys_prx_id_t prx = prx_get_module_id_by_address(unload_prx_module);

	{system_call_3(SC_UNLOAD_PRX_MODULE, (u64)prx, 0, NULL);}

}

int wwwd_stop(void)
{
	sys_ppu_thread_t t;
	uint64_t exit_code;

	int ret = sys_ppu_thread_create(&t, wwwd_stop_thread, 0, 0, 0x2000, SYS_PPU_THREAD_CREATE_JOINABLE, STOP_THREAD_NAME);
	if (ret == 0) sys_ppu_thread_join(t, &exit_code);

	sys_timer_usleep(500000);

	unload_prx_module();

//#ifndef CCAPI
	_sys_ppu_thread_exit(0); // remove for ccapi compatibility ???
//#endif

	return SYS_PRX_STOP_OK;
}
