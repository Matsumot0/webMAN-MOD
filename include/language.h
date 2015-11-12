#ifndef ENGLISH_ONLY
static bool language(const char *file_str, char *default_str);
static void update_language(void);
uint32_t get_xreg_value(char *key, u32 default_value);
uint32_t get_system_language(uint8_t *lang);
#endif

int lang_pos, fh;

#ifdef ENGLISH_ONLY

static char STR_HOME[8] = "Home";

#define STR_TRADBY	"<br>"

#define STR_FILES		"Files"
#define STR_GAMES		"Games"
#define STR_SETUP		"Setup"

#define STR_EJECT		"Eject"
#define STR_INSERT		"Insert"
#define STR_UNMOUNT		"Unmount"
#define STR_COPY		"Copy Folder"
#define STR_REFRESH		"Refresh"
#define STR_SHUTDOWN	"Shutdown"
#define STR_RESTART		"Restart"

#define STR_BYTE		"b"
#define STR_KILOBYTE	"KB"
#define STR_MEGABYTE	"MB"
#define STR_GIGABYTE	"GB"

#define STR_COPYING		"Copying"
#define STR_CPYDEST		"Destination"
#define STR_CPYFINISH	"Copy Finished!"
#define STR_CPYABORT	"Copy aborted!"
#define STR_DELETE		"Delete"

#define STR_SCAN1		"Scan these devices"
#define STR_SCAN2		"Scan for content"
#define STR_PSPL		"Show PSP Launcher"
#define STR_PS2L		"Show PS2 Classic Launcher"
#define STR_RXVID		"Show Video sub-folder"
#define STR_VIDLG		"Video"
#define STR_LPG			"Load last-played game on startup"
#define STR_AUTOB		"Check for /dev_hdd0/PS3ISO/AUTOBOOT.ISO on startup"
#define STR_DELAYAB		"Delay loading of AUTOBOOT.ISO/last-game (Disc Auto-start)"
#define STR_DEVBL		"Enable /dev_blind (writable /dev_flash) on startup"
#define STR_CONTSCAN	"Disable content scan on startup"
#define STR_USBPOLL		"Disable USB polling"
#define STR_FTPSVC		"Disable FTP service"
#define STR_FIXGAME		"Disable auto-fix game"
#define STR_COMBOS		"Disable all PAD shortcuts"
#define STR_MMCOVERS	"Disable multiMAN covers"
#define STR_ACCESS		"Disable remote access to FTP/WWW services"
#define STR_NOSETUP		"Disable webMAN Setup entry in \"webMAN Games\""
#define STR_NOSPOOF		"Disable firmware version spoofing"
#define STR_NOGRP		"Disable grouping of content in \"webMAN Games\""
#define STR_NOWMDN		"Disable startup notification of WebMAN on the XMB"
#ifdef NOSINGSTAR
 #define STR_NOSINGSTAR	"Remove SingStar icon"
#endif
#define STR_RESET_USB	"Disable Reset USB Bus"
#define STR_AUTO_PLAY	"Auto-Play"
#define STR_TITLEID		"Include the ID as part of the title of the game"
#define STR_FANCTRL		"Enable dynamic fan control"
#define STR_NOWARN		"Disable temperature warnings"
#define STR_AUTOAT		"Auto at"
#define STR_LOWEST		"Lowest"
#define STR_FANSPEED	"fan speed"
#define STR_MANUAL		"Manual"
#define STR_PS2EMU		"PS2 Emulator"
#define STR_LANGAMES	"Scan for LAN games/videos"
#define STR_ANYUSB		"Wait for any USB device to be ready"
#define STR_ADDUSB		"Wait additionally for each selected USB device to be ready"
#define STR_SPOOFID		"Change idps and psid in lv2 memory at system startup"
#define STR_DELCFWSYS	"Disable CFW syscalls and delete history files at system startup"
#define STR_MEMUSAGE	"Plugin memory usage"
#define STR_PLANG		"Plugin language"
#define STR_PROFILE		"Profile"
#define STR_DEFAULT		"Default"
#define STR_COMBOS2		"XMB/In-Game PAD SHORTCUTS"
#define STR_FAILSAFE	"FAIL SAFE"
#define STR_SHOWTEMP	"SHOW TEMP"
#define STR_SHOWIDPS	"SHOW IDPS"
#define STR_PREVGAME	"PREV GAME"
#define STR_NEXTGAME	"NEXT GAME"
#define STR_SHUTDOWN2	"SHUTDOWN "
#define STR_RESTART2	"RESTART&nbsp; "
#define STR_DELCFWSYS2	"DEL CFW SYSCALLS"
#define STR_UNLOADWM	"UNLOAD WM"
#define STR_FANCTRL2	"CTRL FAN"
#define STR_FANCTRL4	"CTRL DYN FAN"
#define STR_FANCTRL5	"CTRL MIN FAN"
#define STR_UPDN		"&#8593;/&#8595;" //↑/↓
#define STR_LFRG		"&#8592;/&#8594;" //←/→

#ifdef COBRA_ONLY
 #define STR_DISCOBRA	"COBRA TOGGLE"
#endif

#ifdef REX_ONLY
 #define STR_RBGMODE	"RBG MODE TOGGLE"
 #define STR_RBGNORM	"NORM MODE TOGGLE"
 #define STR_RBGMENU 	"MENU TOGGLE"
#endif

#define STR_SAVE		"Save"
#define STR_SETTINGSUPD	"Settings updated.<br><br>Click <a href=\"/restart.ps3\">here</a> to restart your PLAYSTATION®3 system."
#define STR_ERROR		"Error!"

#define STR_MYGAMES		"webMAN Games"
#define STR_LOADGAMES	"Load games with webMAN"
#define STR_FIXING		"Fixing"

#define STR_WMSETUP		"webMAN Setup"
#define STR_WMSETUP2	"Setup webMAN options"

#define STR_EJECTDISC	"Eject Disc"
#define STR_UNMOUNTGAME	"Unmount current game"

#define STR_WMSTART		"webMAN loaded!"
#define STR_WMUNL		"webMAN unloaded!"
#define STR_CFWSYSALRD	"CFW Syscalls already disabled"
#define STR_CFWSYSRIP	"Removal History files & CFW Syscalls in progress..."
#define STR_RMVCFWSYS	"History files & CFW Syscalls deleted OK!"
#define STR_RMVCFWSYSF	"Failed to remove CFW Syscalls"

#define STR_RMVWMCFG	"webMAN config reset in progress..."
#define STR_RMVWMCFGOK	"Done! Restart within 3 seconds"

#define STR_PS3FORMAT	"PS3 format games"
#define STR_PS2FORMAT	"PS2 format games"
#define STR_PS1FORMAT	"PSOne format games"
#define STR_PSPFORMAT	"PSP\xE2\x84\xA2 format games"

#define STR_VIDFORMAT	"Blu-ray\xE2\x84\xA2 and DVD"
#define STR_VIDEO		"Video content"

#define STR_LAUNCHPSP	"Launch PSP ISO mounted through webMAN or mmCM"
#define STR_LAUNCHPS2	"Launch PS2 Classic"

#define STR_GAMEUM		"Game unmounted."

#define STR_EJECTED		"Disc ejected."
#define STR_LOADED		"Disc inserted."

#define STR_GAMETOM		"Game to mount"
#define STR_GAMELOADED	"Game loaded successfully. Start the game from the disc icon<br>or from <b>/app_home</b>&nbsp;XMB entry.</a><hr>Click <a href=\"/mount.ps3/unmount\">here</a> to unmount the game."
#define STR_PSPLOADED	"Game loaded successfully. Start the game using <b>PSP Launcher</b>.<hr>"
#define STR_PS2LOADED	"Game loaded successfully. Start the game using <b>PS2 Classic Launcher</b>.<hr>"
#define STR_LOADED2		"loaded   "

#define STR_MOVIETOM	"Movie to mount"
#define STR_MOVIELOADED	"Movie loaded successfully. Start the movie from the disc icon<br>under the Video column.</a><hr>Click <a href=\"/mount.ps3/unmount\">here</a> to unmount the movie."

#define STR_XMLRF		"Game list refreshed (<a href=\"" MY_GAMES_XML "\">mygames.xml</a>).<br>Click <a href=\"/restart.ps3\">here</a> to restart your PLAYSTATION®3 system now."

#define STR_STORAGE		"System storage"
#define STR_MEMORY		"Memory"
#define STR_MBFREE		"MB free"
#define STR_KBFREE		"KB free"

#define STR_FANCTRL3	"Fan control:"

#define STR_ENABLED 	"Enabled"
#define STR_DISABLED	"Disabled"

#define STR_FANCH0		"Fan setting changed:"
#define STR_FANCH1		"MAX TEMP: "
#define STR_FANCH2		"FAN SPEED: "
#define STR_FANCH3		"MIN FAN SPEED: "

#define STR_OVERHEAT	"System overheat warning!"
#define STR_OVERHEAT2	"  OVERHEAT DANGER!\r\nFAN SPEED INCREASED!"

#define STR_NOTFOUND	"Not found!"

#else
static char lang_code[3]			= "";

static char STR_TRADBY[150]			= "<br>";

static char STR_FILES[30]			= "Files";
static char STR_GAMES[30]			= "Games";
static char STR_SETUP[30]			= "Setup";
static char STR_HOME[30]			= "Home";
static char STR_EJECT[50]			= "Eject";
static char STR_INSERT[50]			= "Insert";
static char STR_UNMOUNT[50]			= "Unmount";
static char STR_COPY[50]			= "Copy Folder";
static char STR_REFRESH[50]			= "Refresh";
static char STR_SHUTDOWN[50]		= "Shutdown";
static char STR_RESTART[50]			= "Restart";

static char STR_BYTE[10]			= "b";
static char STR_KILOBYTE[10]		= "KB";
static char STR_MEGABYTE[10]		= "MB";
static char STR_GIGABYTE[10]		= "GB";

static char STR_COPYING[30]			= "Copying";
static char STR_CPYDEST[30]			= "Destination";
static char STR_CPYFINISH[30]		= "Copy Finished!";
static char STR_CPYABORT[50]		= "Copy aborted!";
static char STR_DELETE[50]			= "Delete";

static char STR_SCAN1[100]			= "Scan these devices";
static char STR_SCAN2[100]			= "Scan for content";
static char STR_PSPL[100]			= "Show PSP Launcher";
static char STR_PS2L[100]			= "Show PS2 Classic Launcher";
static char STR_RXVID[100]			= "Show Video sub-folder";
static char STR_VIDLG[30]			= "Video";
static char STR_LPG[100]			= "Load last-played game on startup";
static char STR_AUTOB[150]			= "Check for /dev_hdd0/PS3ISO/AUTOBOOT.ISO on startup";
static char STR_DELAYAB[200]		= "Delay loading of AUTOBOOT.ISO/last-game (Disc Auto-start)";
static char STR_DEVBL[150]			= "Enable /dev_blind (writable /dev_flash) on startup";
static char STR_CONTSCAN[150]		= "Disable content scan on startup";
static char STR_USBPOLL[100]		= "Disable USB polling";
static char STR_FTPSVC[100]			= "Disable FTP service";
static char STR_FIXGAME[100]		= "Disable auto-fix game";
static char STR_COMBOS[100]			= "Disable all PAD shortcuts";
static char STR_MMCOVERS[100]		= "Disable multiMAN covers";
static char STR_ACCESS[100]			= "Disable remote access to FTP/WWW services";
static char STR_NOSETUP[150]		= "Disable webMAN Setup entry in \"webMAN Games\"";
static char STR_NOSPOOF[100]		= "Disable firmware version spoofing";
static char STR_NOGRP[100]			= "Disable grouping of content in \"webMAN Games\"";
static char STR_NOWMDN[200]			= "Disable startup notification of WebMAN on the XMB";
#ifdef NOSINGSTAR
static char STR_NOSINGSTAR[100]		= "Remove SingStar icon";
#endif
static char STR_RESET_USB[100]		= "Disable Reset USB Bus";
static char STR_AUTO_PLAY[100]		= "Auto-Play";
static char STR_TITLEID[200]		= "Include the ID as part of the title of the game";
static char STR_FANCTRL[120]		= "Enable dynamic fan control";
static char STR_NOWARN[120]			= "Disable temperature warnings";
static char STR_AUTOAT[100]			= "Auto at";
static char STR_LOWEST[30]			= "Lowest";
static char STR_FANSPEED[80]		= "fan speed";
static char STR_MANUAL[30]			= "Manual";
static char STR_PS2EMU[100]			= "PS2 Emulator";
static char STR_LANGAMES[100]		= "Scan for LAN games/videos";
static char STR_ANYUSB[100]			= "Wait for any USB device to be ready";
static char STR_ADDUSB[150]			= "Wait additionally for each selected USB device to be ready";
static char STR_SPOOFID[150]		= "Change idps and psid in lv2 memory at system startup";
static char STR_DELCFWSYS[200]		= "Disable CFW syscalls and delete history files at system startup";
static char STR_MEMUSAGE[100]		= "Plugin memory usage";
static char STR_PLANG[100]			= "Plugin language";
static char STR_PROFILE[30]			= "Profile";
static char STR_DEFAULT[30]			= "Default";
static char STR_COMBOS2[100]		= "XMB/In-Game PAD SHORTCUTS";
static char STR_FAILSAFE[100]		= "FAIL SAFE";
static char STR_SHOWTEMP[100]		= "SHOW TEMP";
static char STR_SHOWIDPS[100]		= "SHOW IDPS";
static char STR_PREVGAME[100]		= "PREV GAME";
static char STR_NEXTGAME[100]		= "NEXT GAME";
static char STR_SHUTDOWN2[100]		= "SHUTDOWN ";
static char STR_RESTART2[100]		= "RESTART&nbsp; ";
#ifdef REMOVE_SYSCALLS
static char STR_DELCFWSYS2[100] 	= "DEL CFW SYSCALLS";
#endif
static char STR_UNLOADWM[100]		= "UNLOAD WM";
static char STR_FANCTRL2[100]		= "CTRL FAN";
static char STR_FANCTRL4[100]		= "CTRL DYN FAN";
static char STR_FANCTRL5[100]		= "CTRL MIN FAN";
static char STR_UPDN[20]			= "&#8593;/&#8595;"; //↑/↓
static char STR_LFRG[20]			= "&#8592;/&#8594;"; //←/→
#ifdef COBRA_ONLY
static char STR_DISCOBRA[100]		= "COBRA TOGGLE";
#endif
#ifdef REX_ONLY
static char STR_RBGMODE[100]		= "RBG MODE TOGGLE";
static char STR_RBGNORM[100]		= "NORM MODE TOGGLE";
static char STR_RBGMENU[100] 		= "MENU TOGGLE";
#endif
static char STR_SAVE[30]			= "Save";
static char STR_SETTINGSUPD[250]	= "Settings updated.<br><br>Click <a href=\"/restart.ps3\">here</a> to restart your PLAYSTATION®3 system.";
static char STR_ERROR[30]			= "Error!";

static char STR_MYGAMES[50]			= "webMAN Games";
static char STR_LOADGAMES[80]		= "Load games with webMAN";
static char STR_FIXING[50]			= "Fixing";

static char STR_WMSETUP[50]			= "webMAN Setup";
static char STR_WMSETUP2[50]		= "Setup webMAN options";

static char STR_EJECTDISC[50]		= "Eject Disc";
static char STR_UNMOUNTGAME[100]	= "Unmount current game";

static char STR_WMSTART[50]			= "webMAN loaded!";
static char STR_WMUNL[80]			= "webMAN unloaded!";
static char STR_CFWSYSALRD[130]		= "CFW Syscalls already disabled";
static char STR_CFWSYSRIP[130]		= "Removal History files & CFW Syscalls in progress...";
static char STR_RMVCFWSYS[130]		= "History files & CFW Syscalls deleted OK!";
static char STR_RMVCFWSYSF[130]		= "Failed to remove CFW Syscalls";

static char STR_RMVWMCFG[130]		= "webMAN config reset in progress...";
static char STR_RMVWMCFGOK[130]		= "Done! Restart within 3 seconds";

static char STR_PS3FORMAT[50]		= "PS3 format games";
static char STR_PS2FORMAT[50]		= "PS2 format games";
static char STR_PS1FORMAT[50]		= "PSOne format games";
static char STR_PSPFORMAT[50]		= "PSP\xE2\x84\xA2 format games";

static char STR_VIDFORMAT[50]		= "Blu-ray\xE2\x84\xA2 and DVD";
static char STR_VIDEO[50]			= "Video content";

static char STR_LAUNCHPSP[100]		= "Launch PSP ISO mounted through webMAN or mmCM";
static char STR_LAUNCHPS2[100]		= "Launch PS2 Classic";

static char STR_GAMEUM[50]			= "Game unmounted.";

static char STR_EJECTED[50]			= "Disc ejected.";
static char STR_LOADED[50]			= "Disc inserted.";

static char STR_GAMETOM[50]			= "Game to mount";
static char STR_GAMELOADED[250]		= "Game loaded successfully. Start the game from the disc icon<br>or from <b>/app_home</b>&nbsp;XMB entry.</a><hr>Click <a href=\"/mount.ps3/unmount\">here</a> to unmount the game.";
static char STR_PSPLOADED[230]		= "Game loaded successfully. Start the game using <b>PSP Launcher</b>.<hr>";
static char STR_PS2LOADED[230]		= "Game loaded successfully. Start the game using <b>PS2 Classic Launcher</b>.<hr>";
static char STR_LOADED2[50]			= "loaded   ";

static char STR_MOVIETOM[50]		= "Movie to mount";
static char STR_MOVIELOADED[250]	= "Movie loaded successfully. Start the movie from the disc icon<br>under the Video column.</a><hr>Click <a href=\"/mount.ps3/unmount\">here</a> to unmount the movie.";

static char STR_XMLRF[200]			= "Game list refreshed (<a href=\"" MY_GAMES_XML "\">mygames.xml</a>).<br>Click <a href=\"/restart.ps3\">here</a> to restart your PLAYSTATION®3 system now.";

static char STR_STORAGE[50]			= "System storage";
static char STR_MEMORY[50]			= "Memory available";
static char STR_MBFREE[50]			= "MB free";
static char STR_KBFREE[50]			= "KB free";

static char STR_FANCTRL3[50]		= "Fan control:";
static char STR_ENABLED[50]			= "Enabled";
static char STR_DISABLED[50]		= "Disabled";

static char STR_FANCH0[50]			= "Fan setting changed:";
static char STR_FANCH1[50]			= "MAX TEMP: ";
static char STR_FANCH2[50]			= "FAN SPEED: ";
static char STR_FANCH3[50]			= "MIN FAN SPEED: ";

static char STR_OVERHEAT[100]		= "System overheat warning!";
static char STR_OVERHEAT2[100]		= "  OVERHEAT DANGER!\r\nFAN SPEED INCREASED!";

static char STR_NOTFOUND[50]		= "Not found!";

static char COVERS_PATH[100]		= "";
#endif

#ifndef ENGLISH_ONLY

uint32_t get_xreg_value(char *key, u32 default_value)
{
	int reg = -1;
	u32 reg_value = default_value;
	u16 off_string, len_data, len_string;
	u64 read, pos, off_val_data;
	CellFsStat stat;
	char string[256];

	if(cellFsOpen("/dev_flash2/etc/xRegistry.sys", CELL_FS_O_RDONLY, &reg, NULL, 0) != CELL_FS_SUCCEEDED || reg == -1)
	{
		return reg_value;
	}

	cellFsStat("/dev_flash2/etc/xRegistry.sys", &stat);

	u64 entry_name = 0x10000;

	while(true)
	{
	//// Data entries ////
		//unk
		entry_name += 2;

		//relative entry offset
		cellFsLseek(reg, entry_name, 0, &pos);
		cellFsRead(reg, &off_string, 2, &read);
		entry_name += 4;

		//data lenght
		cellFsLseek(reg, entry_name, 0, &pos);
		cellFsRead(reg, &len_data, 2, &read);
		entry_name += 3;

		//data
		off_val_data = entry_name;
		entry_name += len_data + 1;

	//// String Entries ////
		off_string += 0x12;

		//string length
		cellFsLseek(reg, off_string, 0, &pos);
		cellFsRead(reg, &len_string, 2, &read);
		off_string += 3;

		//string
		memset(string, 0, sizeof(string));
		cellFsLseek(reg, off_string, 0, &pos);
		cellFsRead(reg, string, len_string, &read);

		//Find language
		if(strcmp(string, key) == 0)
		{
			cellFsLseek(reg, off_val_data, 0, &pos);
			cellFsRead(reg, &reg_value, 4, &read);
			break;
		}

		if(off_string == 0xCCDD || entry_name >= stat.st_size) break;
	}

	cellFsClose(reg);

	return reg_value;
}

uint32_t get_system_language(uint8_t *lang)
{
	u32 val_lang = get_xreg_value((char*)"/setting/system/language", 1);

	switch(val_lang)
	{
		case 0x0:
			*lang = 4;		//ger;
			break;
		//case 0x1:
		//	lang = 0;		//eng-us
		//	break;
		case 0x2:
			*lang = 3;		//spa
			break;
		case 0x3:
			*lang = 1;		//fre
			break;
		case 0x4:
			*lang = 2;		//ita
			break;
		case 0x5:
			*lang = 5;		//dut //Olandese
			break;
		case 0x6:
			*lang = 6;		//por-por
			break;
		case 0x7:
			*lang = 7;		//rus
			break;
		case 0x8:
			*lang = 18;		//jap
			break;
		case 0x9:
			*lang = 17;		//kor
			break;
		case 0xA:
		case 0xB:
			*lang = 19;		//chi-tra / chi-sim
			break;
		//case 0xC:
		//	*lang = 0;		//fin /** missing **/
		//	break;
		//case 0xD:
		//	*lang = 0;		//swe /** missing**/
		//	break;
		case 0xE:
			*lang = 20;		//dan
			break;
		//case 0xF:
		//	*lang = 0;		//nor /** missing**/
		//	break;
		case 0x10:
			*lang = 9;		//pol
			break;
		case 0x11:
			*lang = 12;		//por-bra
			break;
		//case 0x12:
		//	*lang = 0;		//eng-uk
		//	break;
		default:
			*lang = 0;
			break;
	}

	return val_lang;
}

static bool language(const char *file_str, char *default_str)
{
	uint64_t siz = 0;
	uint8_t i;
	int f=0;
	char temp[1];

	bool do_retry=true;

	if(fh) f=fh; //file is already open
    else
    {
		if(webman_config->lang>22 && webman_config->lang!=99) return false;

		const char lang_codes[24][3]={"EN", "FR", "IT", "ES", "DE", "NL", "PT", "RU", "HU", "PL", "GR", "HR", "BG", "IN", "TR", "AR", "CN", "KR", "JP", "ZH", "DK", "CZ", "SK", "XX"};
		char lang_path[34];

		i=webman_config->lang; if(i>23) i=23;

		sprintf(lang_code, "_%s", lang_codes[i]);
		sprintf(lang_path, "/dev_hdd0/tmp/wm_lang/LANG%s.TXT", lang_code);

		if(cellFsOpen(lang_path, CELL_FS_O_RDONLY, &f, NULL, 0) != CELL_FS_SUCCEEDED) return false;

		fh = f;

 retry:
		cellFsLseek(f, lang_pos, CELL_FS_SEEK_SET, &siz);
	}

	do {
		cellFsRead(f, (void *)&temp, 0x01, &siz);
		lang_pos++;
		for(i=0; i < strlen(file_str); i++)
		{
			if(temp[0] != file_str[i]) break;
			else if(i==strlen(file_str)-1)
			{
				while(siz && temp[0] != '[')
				{
					cellFsRead(f, (void *)&temp, 0x01, &siz);
					lang_pos++;
				}
				int str_len = 0;
				while(siz)
				{
					cellFsRead(f, (void *)&temp, 0x01, &siz);
					lang_pos++;
					if(temp[0] == ']')
					{
						default_str[str_len] = NULL;
						return true;
					}
					default_str[str_len] = temp[0];
					str_len++;
				}
			}
			else
			{
				cellFsRead(f, (void *)&temp, 0x01, &siz);
				lang_pos++;
			}
		}
	} while(siz != 0);

	if(do_retry) {do_retry=false; lang_pos=0; goto retry;}

	return true;
}

static void update_language(void)
{
	lang_pos=fh=0;

	if(language("STR_TRADBY", STR_TRADBY))
	{
		language("STR_FILES", STR_FILES);
		language("STR_GAMES", STR_GAMES);
		language("STR_SETUP", STR_SETUP);
		language("STR_HOME", STR_HOME);
		language("STR_EJECT", STR_EJECT);
		language("STR_INSERT", STR_INSERT);
		language("STR_UNMOUNT", STR_UNMOUNT);
		language("STR_COPY", STR_COPY);
		language("STR_REFRESH", STR_REFRESH);
		language("STR_SHUTDOWN", STR_SHUTDOWN);
		language("STR_RESTART", STR_RESTART);

		language("STR_BYTE", STR_BYTE);
		language("STR_KILOBYTE", STR_KILOBYTE);
		language("STR_MEGABYTE", STR_MEGABYTE);
		language("STR_GIGABYTE", STR_GIGABYTE);

		language("STR_COPYING", STR_COPYING);
		language("STR_CPYDEST", STR_CPYDEST);
		language("STR_CPYFINISH", STR_CPYFINISH);
		language("STR_CPYABORT", STR_CPYABORT);
		language("STR_DELETE", STR_DELETE);

		language("STR_SCAN1", STR_SCAN1);
		language("STR_SCAN2", STR_SCAN2);
		language("STR_PSPL", STR_PSPL);
		language("STR_PS2L", STR_PS2L);
		language("STR_RXVID", STR_RXVID);
		language("STR_VIDLG", STR_VIDLG	);
		language("STR_LPG", STR_LPG);
		language("STR_AUTOB", STR_AUTOB);
		language("STR_DELAYAB", STR_DELAYAB);
		language("STR_DEVBL", STR_DEVBL);
		language("STR_CONTSCAN", STR_CONTSCAN);
		language("STR_USBPOLL", STR_USBPOLL);
		language("STR_FTPSVC", STR_FTPSVC);
		language("STR_FIXGAME", STR_FIXGAME);
		language("STR_COMBOS", STR_COMBOS);
		language("STR_MMCOVERS", STR_MMCOVERS);
		language("STR_ACCESS", STR_ACCESS);
		language("STR_NOSETUP", STR_NOSETUP);
		language("STR_NOSPOOF", STR_NOSPOOF);
		language("STR_NOGRP", STR_NOGRP);
		language("STR_NOWMDN", STR_NOWMDN);
#ifdef NOSINGSTAR
		language("STR_NOSINGSTAR", STR_NOSINGSTAR);
#endif
		language("STR_RESET_USB", STR_RESET_USB);
		language("STR_AUTO_PLAY", STR_AUTO_PLAY);
		language("STR_TITLEID", STR_TITLEID);
		language("STR_FANCTRL", STR_FANCTRL);
		language("STR_NOWARN", STR_NOWARN);
		language("STR_AUTOAT", STR_AUTOAT);
		language("STR_LOWEST", STR_LOWEST);
		language("STR_FANSPEED", STR_FANSPEED);
		language("STR_MANUAL", STR_MANUAL);
		language("STR_PS2EMU", STR_PS2EMU);
		language("STR_LANGAMES", STR_LANGAMES);
		language("STR_ANYUSB", STR_ANYUSB);
		language("STR_ADDUSB", STR_ADDUSB);
		language("STR_SPOOFID", STR_SPOOFID);
		language("STR_DELCFWSYS", STR_DELCFWSYS);
		language("STR_MEMUSAGE", STR_MEMUSAGE);
		language("STR_PLANG", STR_PLANG);
		language("STR_PROFILE", STR_PROFILE);
		language("STR_DEFAULT", STR_DEFAULT);
		language("STR_COMBOS2", STR_COMBOS2);
		language("STR_FAILSAFE", STR_FAILSAFE);
		language("STR_SHOWTEMP", STR_SHOWTEMP);
		language("STR_SHOWIDPS", STR_SHOWIDPS);
		language("STR_PREVGAME", STR_PREVGAME);
		language("STR_NEXTGAME", STR_NEXTGAME);
		language("STR_SHUTDOWN2", STR_SHUTDOWN2);
		language("STR_RESTART2", STR_RESTART2);
#ifdef REMOVE_SYSCALLS
		language("STR_DELCFWSYS2", STR_DELCFWSYS2);
#endif
		language("STR_UNLOADWM", STR_UNLOADWM);
		language("STR_FANCTRL2", STR_FANCTRL2);
		language("STR_FANCTRL4", STR_FANCTRL4);
		language("STR_FANCTRL5", STR_FANCTRL5);
		language("STR_UPDN", STR_UPDN);
		language("STR_LFRG", STR_LFRG);
#ifdef COBRA_ONLY
		language("STR_DISCOBRA", STR_DISCOBRA);
#endif
#ifdef REX_ONLY
		language("STR_RBGMODE", STR_RBGMODE);
		language("STR_RBGNORM", STR_RBGNORM);
		language("STR_RBGMENU", STR_RBGMENU);
#endif
		language("STR_SAVE", STR_SAVE);
		language("STR_SETTINGSUPD", STR_SETTINGSUPD);
		language("STR_ERROR", STR_ERROR);

		language("STR_MYGAMES", STR_MYGAMES);
		language("STR_LOADGAMES", STR_LOADGAMES);
		language("STR_FIXING", STR_FIXING);

		language("STR_WMSETUP", STR_WMSETUP);
		language("STR_WMSETUP2", STR_WMSETUP2);

		language("STR_EJECTDISC", STR_EJECTDISC);
		language("STR_UNMOUNTGAME", STR_UNMOUNTGAME);

        language("STR_WMSTART", STR_WMSTART);
		language("STR_WMUNL", STR_WMUNL);
		language("STR_CFWSYSALRD", STR_CFWSYSALRD);
		language("STR_CFWSYSRIP", STR_CFWSYSRIP);
		language("STR_RMVCFWSYS", STR_RMVCFWSYS);
		language("STR_RMVCFWSYSF", STR_RMVCFWSYSF);

		language("STR_RMVWMCFG", STR_RMVWMCFG);
		language("STR_RMVWMCFGOK", STR_RMVWMCFGOK);

		language("STR_PS3FORMAT", STR_PS3FORMAT);
		language("STR_PS2FORMAT", STR_PS2FORMAT);
		language("STR_PS1FORMAT", STR_PS1FORMAT);
		language("STR_PSPFORMAT", STR_PSPFORMAT);

		language("STR_VIDFORMAT", STR_VIDFORMAT);
		language("STR_VIDEO", STR_VIDEO);

		language("STR_LAUNCHPSP", STR_LAUNCHPSP);
		language("STR_LAUNCHPS2", STR_LAUNCHPS2);

		language("STR_GAMEUM", STR_GAMEUM);

		language("STR_EJECTED", STR_EJECTED);
		language("STR_LOADED", STR_LOADED);

		language("STR_GAMETOM", STR_GAMETOM);
		language("STR_GAMELOADED", STR_GAMELOADED);
		language("STR_PSPLOADED", STR_PSPLOADED);
		language("STR_PS2LOADED", STR_PS2LOADED);
		language("STR_LOADED2", STR_LOADED2);

		language("STR_MOVIETOM", STR_MOVIETOM);
		language("STR_MOVIELOADED", STR_MOVIELOADED);

		language("STR_XMLRF", STR_XMLRF);

		language("STR_STORAGE", STR_STORAGE);
		language("STR_MEMORY", STR_MEMORY);
		language("STR_MBFREE", STR_MBFREE);
		language("STR_KBFREE", STR_KBFREE);

		language("STR_FANCTRL3", STR_FANCTRL3);
		language("STR_ENABLED", STR_ENABLED);
		language("STR_DISABLED", STR_DISABLED);

		language("STR_FANCH0", STR_FANCH0);
		language("STR_FANCH1", STR_FANCH1);
		language("STR_FANCH2", STR_FANCH2);
		language("STR_FANCH3", STR_FANCH3);

		language("STR_OVERHEAT", STR_OVERHEAT);
		language("STR_OVERHEAT2", STR_OVERHEAT2);

		language("STR_NOTFOUND", STR_NOTFOUND);

		language("COVERS_PATH", COVERS_PATH);
		language("IP_ADDRESS", local_ip);
		language("SEARCH_URL", search_url);
	}

	if(fh) {cellFsClose(fh); lang_pos=fh=0;}
}
#endif //#ifndef ENGLISH_ONLY
