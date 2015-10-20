#define MAX_XMB_ITEMS	1350

#define QUERY_XMB(key, src) 	"<Query class=\"type:x-xmb/folder-pixmap\" key=\"" key "\" attr=\"" key "\" src=\"" src "\"/>"
#define ADD_XMB_ITEM(key)		"<Item class=\"type:x-xmb/module-action\" key=\"" key "\" attr=\"" key "\"/>"

#define XML_HEADER				"<?xml version=\"1.0\" encoding=\"UTF-8\"?><XMBML version=\"1.0\">"
#define XML_PAIR(key, value) 	"<Pair key=\"" key "\"><String>" value "</String></Pair>"

#define WEB_LINK_PAIR			XML_PAIR("module_name", "webbrowser_plugin")
#define STR_NOITEM_PAIR			XML_PAIR("str_noitem", "msg_error_no_content") "</Table>"

static void add_xmb_entry(char *param, char *tempstr, char *templn, char *skey, u32 key, char *myxml_ps3, char *myxml_ps2, char *myxml_psx, char *myxml_psp, char *myxml_dvd, char *entry_name, u16 *item_count);

static void refresh_xml(char *msg);

static void add_xmb_entry(char *param, char *tempstr, char *templn, char *skey, u32 key, char *myxml_ps3, char *myxml_ps2, char *myxml_psx, char *myxml_psp, char *myxml_dvd, char *entry_name, u16 *item_count)
{
	if(strlen(templn)<6) strcat(templn, "      ");
	sprintf(skey, "3%c%c%c%c%c%c%04i", templn[0], templn[1], templn[2], templn[3], templn[4], templn[5], key);

	char *p = strstr(templn, "CD");
	if(p) {if(p[2]>='0' && p[2]<='9') skey[6]=p[2]; if(p[3]>='0' && p[3]<='9') skey[6]=p[3];} // sort by CD#
	else
	{
		u16 tlen=strlen(templn), i=5; if(tlen>64) tlen=64;
		while(i<tlen) {if(templn[i]>='0' && templn[i]<='9') {skey[6]=templn[i]; break;} else i++;} // sort by game number (if possible)
	}

	if( !(webman_config->nogrp) )
	{
		if(strstr(param, "/PS2ISO") && strlen(myxml_ps2)<(BUFFER_SIZE_PS2-1024))
		{strcat(myxml_ps2, tempstr); skey[0]=PS2; item_count[2]++;}
#ifdef COBRA_ONLY
		else
		if((strstr(param, "/PSPISO") || strstr(param, "/ISO/")) && strlen(myxml_psp)<(BUFFER_SIZE_PSP-1024))
		{strcat(myxml_psp, tempstr); skey[0]=PSP; item_count[4]++;}
		else
		if((strstr(param, "/PSX") || !extcmp(entry_name, ".ntfs[PSXISO]", 13)) && strlen(myxml_psx)<(BUFFER_SIZE_PSX-1024))
		{strcat(myxml_psx, tempstr); skey[0]=PS1; item_count[1]++;}
		else
		if((strstr(param, "/BDISO") || strstr(param, "/DVDISO") || !extcmp(entry_name, ".ntfs[DVDISO]", 13) || !extcmp(entry_name, ".ntfs[BDISO]", 12) || !extcmp(entry_name, ".ntfs[BDFILE]", 13)) && strlen(myxml_dvd)<(BUFFER_SIZE_DVD-1024))
		{strcat(myxml_dvd, tempstr); skey[0]=BLU; item_count[0]++;}
#endif
		else
		if(strlen(myxml_ps3)<(BUFFER_SIZE-5000))
        {strcat(myxml_ps3, tempstr); item_count[3]++;}
	}
	else
	{
		if(strlen(myxml_ps3)<(BUFFER_SIZE-5000))
			strcat(myxml_ps3, tempstr);
	}
}

static void make_fb_xml(char *myxml)
{
	cellFsUnlink(FB_XML);
	sprintf(myxml,  "%s"
					"<View id=\"seg_fb\">"
					"<Attributes><Table key=\"mgames\">"
					XML_PAIR("icon_rsc","item_tex_ps3util")
					XML_PAIR("icon_notation","WNT_XmbItemSavePS3")
					XML_PAIR("title","%s%s")
					XML_PAIR("info","%s")
					"</Table>"
					"</Attributes>"
					"<Items>"
					QUERY_XMB("mgames", "xmb://localhost%s#seg_mygames")
					"%s</XMBML>", XML_HEADER, STR_MYGAMES, SUFIX2(profile), STR_LOADGAMES, MY_GAMES_XML, "</Items></View>");
	savefile((char*)FB_XML, (char*)myxml, strlen(myxml));
}

static bool update_mygames_xml(u64 conn_s_p)
{
	struct CellFsStat buf;

	char xml[48]; sprintf(xml, MY_GAMES_XML);

	if(conn_s_p==START_DAEMON && ((webman_config->refr==1) || from_reboot))
	{
		cellFsUnlink((char*)WMNOSCAN);

		mount_autoboot();

		if(cellFsStat((char*)xml, &buf)==CELL_FS_SUCCEEDED)
		{
			if(cellFsStat(FB_XML, &buf)==CELL_FS_SUCCEEDED) return false;
		}
	}

	set_buffer_sizes(webman_config->foot);

	_meminfo meminfo;
	{system_call_1(SC_GET_FREE_MEM, (uint64_t)(u32) &meminfo);}
	if((meminfo.avail)<( (BUFFER_SIZE_ALL) + MIN_MEM)) set_buffer_sizes(3); //MIN+
	if((meminfo.avail)<( (BUFFER_SIZE_ALL) + MIN_MEM)) set_buffer_sizes(1); //MIN
	if((meminfo.avail)<( (BUFFER_SIZE_ALL) + MIN_MEM))
	{
		return false;  //leave if less than min memory
	}

	sys_addr_t sysmem=0;

#ifdef USE_VM
	if(sys_vm_memory_map(_32MB_, _1MB_, SYS_MEMORY_CONTAINER_ID_INVALID, SYS_MEMORY_PAGE_SIZE_64K, SYS_VM_POLICY_AUTO_RECOMMENDED, &sysmem)!=CELL_OK)
	{
		return false;
	}
#else
	if(sys_memory_allocate((BUFFER_SIZE_ALL), SYS_MEMORY_PAGE_SIZE_64K, &sysmem)!=0)
	{
		return false;
	}
#endif

	sys_addr_t sysmem1=sysmem +(BUFFER_SIZE);
	sys_addr_t sysmem2=sysmem1+(BUFFER_SIZE_PSX)+(BUFFER_SIZE_PSP);
	sys_addr_t sysmem3=sysmem2+(BUFFER_SIZE_PS2);

	char *myxml_ps3   = (char*)sysmem;
	char *myxml_psx   = NULL;
	char *myxml_psp   = NULL;
	char *myxml_ps2   = NULL;
	char *myxml_dvd   = NULL;
	char *myxml       = NULL;
	char *myxml_items = NULL;

	myxml_psx = (char*)sysmem1;
	myxml_psp = (char*)sysmem1+(BUFFER_SIZE_PSX);
	myxml_ps2 = (char*)sysmem2;

	myxml_dvd	= (char*)sysmem3;
	myxml		= (char*)sysmem+(BUFFER_SIZE)-4300;
	myxml_items = (char*)sysmem3;

	cellFsMkdir((char*)"/dev_hdd0/xmlhost", MODE);
	cellFsMkdir((char*)"/dev_hdd0/xmlhost/game_plugin", MODE);
	u32 key=0;

	make_fb_xml(myxml);

	if(conn_s_p==START_DAEMON)
	{
#ifdef USE_VM
		sys_vm_unmap(sysmem);
#else
		sys_memory_free(sysmem);
#endif
		// start a new thread for refresh content at start up
		if(!webman_config->refr || cellFsStat((char*)xml, &buf)!=CELL_FS_SUCCEEDED)
		{
			sys_ppu_thread_t id3;
			sys_ppu_thread_create(&id3, handleclient, (u64)REFRESH_CONTENT, -0x1d8, 0x20000, 0, "wwwd2");
		}
		return false;
	}

	// --- build group headers ---
	char templn[1024]; char *tempstr, *folder_name; tempstr=myxml; memset(tempstr, 0, _4KB_); folder_name=myxml+(3*KB);

	u16 item_count[5];
	for(u8 i=0;i<5;i++) item_count[i]=0;

	cellFsUnlink(xml);

	key=0;

	if(!(webman_config->nogrp))
	{
		if(!(webman_config->cmask & PS3)) strcpy(myxml_ps3, "<View id=\"seg_wm_ps3_items\"><Attributes>");
		if(!(webman_config->cmask & PS2))
		{
			strcpy(myxml_ps2, "<View id=\"seg_wm_ps2_items\"><Attributes>");
			if(webman_config->ps2l && cellFsStat((char*)"/dev_hdd0/game/PS2U10000", &buf)==CELL_FS_SUCCEEDED)
			{
				sprintf(templn, "<Table key=\"ps2_classic_launcher\">"
								XML_PAIR("icon","/dev_hdd0/game/PS2U10000/ICON0.PNG")
								XML_PAIR("title","PS2 Classic Launcher")
								XML_PAIR("info","%s") "%s",
								STR_LAUNCHPS2, "</Table>"); strcat(myxml_ps2, templn);
			}
		}
#ifdef COBRA_ONLY
		if(!(webman_config->cmask & PS1)) strcpy(myxml_psx, "<View id=\"seg_wm_psx_items\"><Attributes>");
		if(!(webman_config->cmask & PSP))
		{
			strcpy(myxml_psp, "<View id=\"seg_wm_psp_items\"><Attributes>");
			if(webman_config->pspl && cellFsStat((char*)"/dev_hdd0/game/PSPC66820", &buf)==CELL_FS_SUCCEEDED)
			{
				sprintf(templn, "<Table key=\"cobra_psp_launcher\">"
								XML_PAIR("icon","/dev_hdd0/game/PSPC66820/ICON0.PNG")
								XML_PAIR("title","PSP Launcher")
								XML_PAIR("info","%s") "%s",
								STR_LAUNCHPSP, "</Table>"); strcat(myxml_psp, templn);
			}
		}
		if(!(webman_config->cmask & DVD) || !(webman_config->cmask & BLU))
		{
			strcpy(myxml_dvd, "<View id=\"seg_wm_dvd_items\"><Attributes>");
			if(webman_config->rxvid)
			{
				sprintf(templn, "<Table key=\"rx_video\">"
								XML_PAIR("icon","%s")
								XML_PAIR("title","%s")
								XML_PAIR("child","segment") "%s",
								wm_icons[4], STR_VIDLG, STR_NOITEM_PAIR); strcat(myxml_dvd, templn);
			}
		}
#endif
	}

	CellRtcTick pTick;
	cellRtcGetCurrentTick(&pTick);

	int fd;
	char skey[MAX_XMB_ITEMS][12];
	u8 is_net=0;

	// --- scan xml content ---

	led(YELLOW, BLINK_FAST);

	for(u8 f0=0; f0<14; f0++)  // drives: 0="/dev_hdd0", 1="/dev_usb000", 2="/dev_usb001", 3="/dev_usb002", 4="/dev_usb003", 5="/dev_usb006", 6="/dev_usb007", 7="/net0", 8="/net1", 9="/net2", 10="/ext", 11="/dev_sd", 12="/dev_ms", 13="/dev_cf"
	{
		if(!webman_config->usb0 && (f0==1)) continue;
		if(!webman_config->usb1 && (f0==2)) continue;
		if(!webman_config->usb2 && (f0==3)) continue;
		if(!webman_config->usb3 && (f0==4)) continue;
		if(!webman_config->usb6 && (f0==5)) continue;
		if(!webman_config->usb7 && (f0==6)) continue;

		if(!webman_config->dev_sd && (f0==11)) continue;
		if(!webman_config->dev_ms && (f0==12)) continue;
		if(!webman_config->dev_cf && (f0==13)) continue;

		if( f0==NTFS && (!webman_config->usb0 && !webman_config->usb1 && !webman_config->usb2 &&
						 !webman_config->usb3 && !webman_config->usb6 && !webman_config->usb7)) continue;

		int ns=-2; u8 uprofile=profile;
		for(u8 f1=0; f1<11; f1++) // paths: 0="GAMES", 1="GAMEZ", 2="PS3ISO", 3="BDISO", 4="DVDISO", 5="PS2ISO", 6="PSXISO", 7="PSXGAMES", 8="PSPISO", 9="ISO", 10="video"
		{
#ifndef COBRA_ONLY
			if(IS_ISO_FOLDER && !(IS_PS2_FOLDER)) continue; // 0="GAMES", 1="GAMEZ", 5="PS2ISO", 10="video"
#endif
			if(key>MAX_XMB_ITEMS) break;

			cellRtcGetCurrentTick(&pTick);

			if(IS_PS2_FOLDER && f0>0)  continue; // PS2ISO is supported only from /dev_hdd0
            if(f1>=10) {if(f0<7 || f0>NTFS) strcpy(paths[10], f0==0 ? "video" : "GAMES_DUP"); else continue;}
			if(f0==NTFS) {if(f1>6 || !cobra_mode) break; else if(f1<2 || f1==5) continue;}
			if(f0==7 && (!webman_config->netd0 || f1>6 || !cobra_mode)) break; //net0
			if(f0==8 && (!webman_config->netd1 || f1>6 || !cobra_mode)) break; //net1
			if(f0==9 && (!webman_config->netd2 || f1>6 || !cobra_mode)) break; //net2

			if( (webman_config->cmask & PS3) && (IS_PS3_FOLDER)) continue; // 0="GAMES", 1="GAMEZ", 2="PS3ISO", 10="video"
			if( (webman_config->cmask & BLU) && (IS_BLU_FOLDER)) continue;
			if( (webman_config->cmask & DVD) && (IS_DVD_FOLDER)) continue;
			if( (webman_config->cmask & PS2) && (IS_PS2_FOLDER)) continue;
			if( (webman_config->cmask & PS1) && (IS_PSX_FOLDER)) continue;
			if( (webman_config->cmask & PSP) && (IS_PSP_FOLDER)) continue;

			is_net=(f0>=7 && f0<=9);

#ifdef COBRA_ONLY
 #ifndef LITE_EDITION
			if(ns==-2 && is_net) ns=connect_to_remote_server(f0-7);
 #endif
#endif

			if(is_net && (ns<0)) break;

//
			char param[MAX_PATH_LEN];

			bool ls; u8 li, subfolder; li=subfolder=0; ls=false;

		subfolder_letter_xml:
			subfolder = 0; uprofile = profile;
read_folder_xml:
//
#ifndef LITE_EDITION
			if(is_net)
			{
				char ll[4]; if(li) sprintf(ll, "/%c", '@'+li); else ll[0]=0;
				sprintf(param, "/%s%s%s",    paths[f1], SUFIX(uprofile), ll);
				if(li==99) sprintf(param, "/%s %s", paths[f1], AUTOPLAY_TAG);
			}
			else
#endif
			{
				if(f0==NTFS) //ntfs
					sprintf(param, "%s", WMTMP);
				else
					sprintf(param, "%s/%s%s", drives[f0], paths[f1], SUFIX(uprofile));

				if(li==99) sprintf(param, "%s/%s %s", drives[f0], paths[f1], AUTOPLAY_TAG);
			}

			if(conn_s_p==START_DAEMON && f1==0)
			{
				if(webman_config->bootd && (f0==1))
				{
					waitfor((char*)"/dev_usb", webman_config->bootd); // wait for any usb
				}

				if(webman_config->boots && (f0>=1 && f0<=6)) // usb000->007
				{
					if( (webman_config->usb0 && (f0==1)) ||
						(webman_config->usb1 && (f0==2)) ||
						(webman_config->usb2 && (f0==3)) ||
						(webman_config->usb3 && (f0==4)) ||
						(webman_config->usb6 && (f0==5)) ||
						(webman_config->usb7 && (f0==6)) )
					{
						waitfor((char*)drives[f0], webman_config->boots);
					}
				}
			}

			if(!is_net && cellFsOpendir( param, &fd) != CELL_FS_SUCCEEDED) goto continue_reading_folder_xml; //continue;

			int abort_connection=0;

#ifndef LITE_EDITION
 #ifdef COBRA_ONLY
			if(is_net && open_remote_dir(ns, param, &abort_connection) < 0) goto continue_reading_folder_xml; //continue;
 #endif
#endif

			//led(YELLOW, ON);
			{
				CellFsDirent entry;
				u64 read_e;
				int fs;
				uint64_t msiz = 0;
				u8 is_iso=0;
				char icon[MAX_PATH_LEN], enc_dir_name[1024], subpath[MAX_PATH_LEN]; int fd2;
				char tempID[12];

#ifdef COBRA_ONLY
 #ifndef LITE_EDITION
				sys_addr_t data2=0;
				int v3_entries, v3_entry; v3_entries=v3_entry=0;
				netiso_read_dir_result_data *data=NULL; char neth[8];

				if(is_net)
				{
					v3_entries = read_remote_dir(ns, &data2, &abort_connection);
					if(data2==NULL) goto continue_reading_folder_xml; //continue;
					data=(netiso_read_dir_result_data*)data2; sprintf(neth, "/net%i", (f0-7));
				}
 #endif
#endif

				while((!is_net && cellFsReaddir(fd, &entry, &read_e) == 0 && read_e > 0)
#ifdef COBRA_ONLY
 #ifndef LITE_EDITION
					|| (is_net && v3_entry<v3_entries)
 #endif
#endif
					)
				{
					if(key>MAX_XMB_ITEMS) break;
					cellRtcGetCurrentTick(&pTick);
					icon[0]=tempID[0]=0;

#ifdef COBRA_ONLY
 #ifndef LITE_EDITION
					if(is_net)
					{
						if(!ls && li==0 && f1>1 && data[v3_entry].is_directory && strlen(data[v3_entry].name)==1) ls=true;

						if(add_net_game(ns, data, v3_entry, neth, param, templn, tempstr, enc_dir_name, icon, tempID, f1, 0)==FAILED) {v3_entry++; continue;}

						sprintf(tempstr, "<Table key=\"%04i\">"
										 XML_PAIR("icon","%s")
										 XML_PAIR("title","%s") "%s"
										 XML_PAIR("module_action","http://%s/mount_ps3%s%s/%s?random=%x")
										 XML_PAIR("info","%s%s%s") "</Table>",
								key, icon,
								templn, WEB_LINK_PAIR, local_ip, neth, param, enc_dir_name, (u16)pTick.tick, neth, param, "");

						add_xmb_entry(param, tempstr, templn, skey[key], key, myxml_ps3, myxml_ps2, myxml_psx, myxml_psp, myxml_dvd, data[v3_entry].name, item_count); key++;

						v3_entry++;
					}
					else
 #endif
#endif

					{
						if(entry.d_name[0]=='.') continue;

/////////////////////////////////////////
						subfolder=0;
						if(IS_ISO_FOLDER && (f0<7 || f0>NTFS))
						{
							sprintf(subpath, "%s/%s", param, entry.d_name);
							if(isDir(subpath) && cellFsOpendir(subpath, &fd2) == CELL_FS_SUCCEEDED)
							{
								strcpy(subpath, entry.d_name); subfolder = 1;
next_xml_entry:
								cellFsReaddir(fd2, &entry, &read_e);
								if(read_e<1) continue;
								if(entry.d_name[0]=='.') goto next_xml_entry;
								sprintf(templn, "%s/%s", subpath, entry.d_name); strcpy(entry.d_name, templn);
							}
						}
						int flen = strlen(entry.d_name);
/////////////////////////////////////////

#ifdef COBRA_ONLY
						is_iso = (f0==NTFS && flen>13 && strstr(entry.d_name + flen - 13, ".ntfs[")!=NULL) ||
								 (IS_ISO_FOLDER && flen > 4 && (
								 (            !strncasecmp(entry.d_name + flen - 4, ".iso", 4)) ||
								 (flen > 6 && !strncasecmp(entry.d_name + flen - 6, ".iso.0", 6)) ||
								 ((IS_PS2_FOLDER) && strcasestr(".bin.img.mdf.enc", entry.d_name + flen - 4)) ||
								 ((IS_PSX_FOLDER) && strcasestr(".cue.img.mdf", entry.d_name + flen - 4))
								 ));
#else
						is_iso = (IS_PS2_FOLDER && flen > 8 && !strncmp(entry.d_name + flen - 8, ".BIN.ENC", 8));
#endif //#ifdef COBRA_ONLY
						if(!is_iso)
						{
							sprintf(templn, "%s/%s/PS3_GAME/PARAM.SFO", param, entry.d_name);
						}

						if(is_iso || (f1<2 && cellFsStat(templn, &buf)==CELL_FS_SUCCEEDED))
						{
							msiz=0;
							if(!is_iso)
							{
								get_title_and_id_from_sfo(templn, tempID, entry.d_name, icon, tempstr, 0);
							}
							else
							{
#ifdef COBRA_ONLY
								if(f0==NTFS)
								{   // ntfs
									if(f1< 2 || f1>6) continue; //2="PS3ISO", 3="BDISO", 4="DVDISO", 5="PS2ISO", 6="PSXISO"
									if((uprofile >0) && !strstr(entry.d_name, SUFIX3(uprofile))) continue;
									if((uprofile==0 && flen>17)) {for(u8 u=1;u<5;u++) if(strstr(entry.d_name + flen - 17, SUFIX3(u))) continue;}
								}

								if((strstr(param, "/PS3ISO") && f0!=NTFS) || (f0==NTFS && f1==2 && !extcmp(entry.d_name, ".ntfs[PS3ISO]", 13)))
								{
									get_name(templn, entry.d_name, 1); strcat(templn, ".SFO\0");
									if(f0!=NTFS && cellFsStat(templn, &buf)!=CELL_FS_SUCCEEDED)
									{
										get_name(tempstr, entry.d_name, 0);
										sprintf(templn, "%s/%s.SFO", param, tempstr);
									}

									get_title_and_id_from_sfo(templn, tempID, entry.d_name, icon, tempstr, f0);
								}
								else
#endif
								{
#ifdef COBRA_ONLY
									if(f0==NTFS)
									{   // ntfs
										int flen=strlen(entry.d_name)-13; if(flen<0) continue;

										if(f1==2 && !strstr(entry.d_name+flen, ".ntfs[PS3ISO]")) continue;
										if(f1==3 && !strstr(entry.d_name+flen, ".ntfs[BD"))      continue;
										if(f1==4 && !strstr(entry.d_name+flen, ".ntfs[DVDISO]")) continue;
										if(f1==6 && !strstr(entry.d_name+flen, ".ntfs[PSXISO]")) continue;
									}
#endif
									get_name(templn, entry.d_name, 0);
								}

								if(f0!=NTFS && tempID[0]==0 && strstr(param, "/PS3ISO"))
								{   // get title ID from ISO
									sprintf(icon, "%s/%s", param, entry.d_name);
									if(cellFsOpen(icon, CELL_FS_O_RDONLY, &fs, NULL, 0) == CELL_FS_SUCCEEDED)
									{
										if(cellFsLseek(fs, 0x810, CELL_FS_SEEK_SET, &msiz) == CELL_FS_SUCCEEDED)
										{
											if(cellFsRead(fs, (void *)&tempID, 11, &msiz) == CELL_FS_SUCCEEDED)
											{
												strncpy(&tempID[4], &tempID[5], 5); tempID[9]='\0';
											}
										}
										cellFsClose(fs);
									}
									icon[0]=0;
								}
							}
		//title_foundx:
							if(!is_iso && f1<2 && (icon[0]==0 || webman_config->nocov)) sprintf(icon, "%s/%s/PS3_GAME/ICON0.PNG", param, entry.d_name);

							get_cover_from_name(icon, entry.d_name, tempID);

							if(is_iso)
							{
								if(icon[0]==0 || cellFsStat(icon, &buf)!=CELL_FS_SUCCEEDED)
								{
									sprintf(icon, "%s/%s", param, entry.d_name);

									flen = strlen(icon);
#ifdef COBRA_ONLY
									if(flen > 13 && (!extcmp(icon, ".ntfs[PS3ISO]", 13) || !extcmp(icon, ".ntfs[DVDISO]", 13) || !extcmp(icon, ".ntfs[PSXISO]", 13) || !extcmp(icon, ".ntfs[BDFILE]", 13))) {flen -= 13; icon[flen]=0;} else
									if(flen > 12 &&  !extcmp(icon, ".ntfs[BDISO]" , 12)) {flen -= 12; icon[flen]=0;}
#endif
									if(flen > 4 && icon[flen-4]=='.')
									{
										icon[flen-3]='p'; icon[flen-2]='n'; icon[flen-1]='g';
										if(cellFsStat(icon, &buf)!=CELL_FS_SUCCEEDED)
										{
											icon[flen-3]='P'; icon[flen-2]='N'; icon[flen-1]='G';
										}
									}
									else
									if(flen > 5 && icon[flen-2]=='.')
									{
										icon[flen-5]='p'; icon[flen-4]='n'; icon[flen-3]='g'; flen -= 2; icon[flen]=0;
									}

									if(cellFsStat(icon, &buf)!=CELL_FS_SUCCEEDED)
										{icon[flen-3]='j'; icon[flen-2]='p'; icon[flen-1]='g';}
								}
							}
							else if(icon[0]==0 || cellFsStat(icon, &buf)!=CELL_FS_SUCCEEDED)
								sprintf(icon, "%s/%s/PS3_GAME/ICON0.PNG", param, entry.d_name);

							get_default_icon(icon, param, entry.d_name, 0, tempID, ns, abort_connection);

							if(webman_config->tid && tempID[0]>'@' && strlen(templn) < 50 && strstr(templn, " [")==NULL) {strcat(templn, " ["); strcat(templn, tempID); strcat(templn, "]");}

							urlenc(enc_dir_name, entry.d_name);

							// subfolder name
							if(f0==NTFS && entry.d_name[0]=='[')
							{
								strcpy(folder_name, entry.d_name); folder_name[0]='/'; char *p=strchr(folder_name, ']'); if(p) p[0]=0;
							}
							else
							{
								char *p=strchr(entry.d_name, '/');
								folder_name[0]=0; if(p) {p[0]=0; sprintf(folder_name, "/%s", entry.d_name); p[0]='/';}
							}

							sprintf(tempstr, "<Table key=\"%04i\">"
											 XML_PAIR("icon","%s")
											 XML_PAIR("title","%s") "%s"
											 XML_PAIR("module_action","http://%s/mount_ps3%s%s/%s?random=%x")
											 XML_PAIR("info","%s%s%s") "</Table>",
								key, icon,
								templn, WEB_LINK_PAIR, local_ip, "", param, enc_dir_name, (u16)pTick.tick, (f0==NTFS?(char*)"/ntfs/":param), (f0==NTFS?paths[f1]:""), folder_name);

							add_xmb_entry(param, tempstr, templn, skey[key], key, myxml_ps3, myxml_ps2, myxml_psx, myxml_psp, myxml_dvd, entry.d_name, item_count); key++;
						}
//////////////////////////////
						if(subfolder) goto next_xml_entry;
//////////////////////////////
					}
				}
				if(!is_net) cellFsClosedir(fd);

#ifdef COBRA_ONLY
 #ifndef LITE_EDITION
				if(data2) sys_memory_free(data2);
 #endif
#endif
			}

//
continue_reading_folder_xml:

			if((uprofile>0) && (f1<9)) {subfolder=uprofile=0; goto read_folder_xml;}
			if(is_net && ls && li<27) {li++; goto subfolder_letter_xml;} else if(li<99 && !(IS_PSP_FOLDER)) {li=99; goto subfolder_letter_xml;}
//
		}
		if(is_net && ns>=0) {shutdown(ns, SHUT_RDWR); socketclose(ns); ns=-2;}
	}

	if( !(webman_config->nogrp))
	{
		if(!(webman_config->cmask & PS3)) {strcat(myxml_ps3, "</Attributes><Items>");}
		if(!(webman_config->cmask & PS2)) {strcat(myxml_ps2, "</Attributes><Items>"); if(webman_config->ps2l && cellFsStat((char*)PS2_CLASSIC_PLACEHOLDER, &buf)==CELL_FS_SUCCEEDED) strcat(myxml_ps2, QUERY_XMB("ps2_classic_launcher", "xcb://127.0.0.1/query?limit=1&cond=Ae+Game:Game.titleId PS2U10000"));}

#ifdef COBRA_ONLY
		if(!(webman_config->cmask & PS1)) {strcat(myxml_psx, "</Attributes><Items>");}
		if(!(webman_config->cmask & PSP)) {strcat(myxml_psp, "</Attributes><Items>"); if(webman_config->pspl && cellFsStat((char*)"/dev_hdd0/game/PSPC66820", &buf)==CELL_FS_SUCCEEDED) strcat(myxml_psp, QUERY_XMB("cobra_psp_launcher", "xcb://127.0.0.1/query?limit=1&cond=Ae+Game:Game.titleId PSPC66820"));}
		if(!(webman_config->cmask & DVD) || !(webman_config->cmask & BLU)) {strcat(myxml_dvd, "</Attributes><Items>"); if(webman_config->rxvid) strcat(myxml_dvd, QUERY_XMB("rx_video", "#seg_wm_bdvd"));}
#endif
	}
	else
		myxml_dvd[0]=0;

	// --- sort scanned content

	led(YELLOW, OFF);
	led(GREEN, ON);

	if(key)
	{   // sort xmb items
		char swap[16]; u16 m, n;

		if((webman_config->nogrp))
		{
			for(n=0; n<(key-1); n++)
				for(m=(n+1); m<key; m++)
					if(strcasecmp(skey[n]+1, skey[m]+1)>0)
					{
						strcpy(swap, skey[n]);
						strcpy(skey[n], skey[m]);
						strcpy(skey[m], swap);
					}
		}
		else
			for(n=0; n<(key-1); n++)
				for(m=(n+1); m<key; m++)
					if(strcasecmp(skey[n], skey[m])>0)
					{
						strcpy(swap, skey[n]);
						strcpy(skey[n], skey[m]);
						strcpy(skey[m], swap);
					}
	}

	// --- add eject & setup/xmbm+ menu
#ifdef ENGLISH_ONLY
	bool add_xmbm_plus = cellFsStat("/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN.xml", &buf)==CELL_FS_SUCCEEDED;
#else
	bool add_xmbm_plus = false;

	while(true)
	{
		sprintf(templn, "/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN%s.xml", lang_code);
		add_xmbm_plus = cellFsStat(templn, &buf)==CELL_FS_SUCCEEDED;
		if(add_xmbm_plus || lang_code[0]==0) break; lang_code[0]=0;
	}
#endif

	if( (webman_config->nogrp))
	{
		strcat(myxml_items, ADD_XMB_ITEM("eject"));
		if(!webman_config->noset)
        {
			if(add_xmbm_plus)
#ifdef ENGLISH_ONLY
				strcat(myxml_items, QUERY_XMB("setup", "xmb://localhost/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN.xml#seg_webman_links_items"));
#else
			{
				sprintf(tempstr, QUERY_XMB("setup", "xmb://localhost%s#seg_webman_links_items"), templn);
				strcat(myxml_items, tempstr);
			}
#endif
			else
				strcat(myxml_items, ADD_XMB_ITEM("setup"));
		}
	}

	// --- add sorted items to xml

	for(u16 a=0; a<key; a++)
	{
		sprintf(templn, ADD_XMB_ITEM("%s"), skey[(a)]+7, skey[(a)]+7);
		if( !(webman_config->nogrp))
		{
#ifdef COBRA_ONLY
			if(skey[(a)][0]==PSP && strlen(myxml_psp)<(BUFFER_SIZE_PSP-128))
				strcat(myxml_psp, templn);
			else
			if(skey[(a)][0]==PS1 && strlen(myxml_psx)<(BUFFER_SIZE_PSX-128))
				strcat(myxml_psx, templn);
			else
			if(skey[(a)][0]==BLU && strlen(myxml_dvd)<(BUFFER_SIZE_DVD-1200))
				strcat(myxml_dvd, templn);
			else
#endif
			if(skey[(a)][0]==PS2 && strlen(myxml_ps2)<(BUFFER_SIZE_PS2-128))
				strcat(myxml_ps2, templn);
			else
			if(strlen(myxml_ps3)<(BUFFER_SIZE-5000))
				strcat(myxml_ps3, templn);

		}
		else
			if(strlen(myxml_dvd)<(BUFFER_SIZE-1000))
				strcat(myxml_items, templn);
	}

	// --- close xml items

	if( !(webman_config->nogrp))
	{
		if(!(webman_config->cmask & PS3)) strcat(myxml_ps3, "</Items></View>");
		if(!(webman_config->cmask & PS2)) strcat(myxml_ps2, "</Items></View>");
#ifdef COBRA_ONLY
		if(!(webman_config->cmask & PS1)) strcat(myxml_psx, "</Items></View>");
		if(!(webman_config->cmask & PSP)) strcat(myxml_psp, "</Items></View>");
		if(!(webman_config->cmask & DVD) || !(webman_config->cmask & BLU))
		{
			strcat(myxml_dvd, "</Items></View>");
			if(webman_config->rxvid)
			{
				strcat(myxml_dvd,	"<View id=\"seg_wm_bdvd\">"
									"<Items>"
									QUERY_XMB("rx_video1", "xcb://localhost/query?table=MMS_MEDIA_TYPE_SYSTEM&genre=Video&sort=+StorageMedia:StorageMedia.sortOrder+StorageMedia:StorageMedia.timeInserted&cond=Ae+StorageMedia:StorageMedia.stat.mediaStatus %xCB_MEDIA_INSERTED+Ae+StorageMedia:StorageMedia.mediaFormat %xCB_MEDIA_FORMAT_DATA+AGL+StorageMedia:StorageMedia.type %xCB_MEDIA_TYPE_BDROM %xCB_MEDIA_TYPE_WM")
									QUERY_XMB("rx_video2", "xcb://localhost/query?sort=+Game:Common.titleForSort&cond=AGL+Game:Game.titleId RXMOV0000 RXMOVZZZZ+An+Game:Game.category 2D+An+Game:Game.category BV+An+Game:Game.category HG")
									"</Items>"
									"</View>");
			}
		}
#endif
	}

	// --- build xml headers
	memset(myxml, 0, 4300);
	sprintf(templn, "%s"
					"<View id=\"seg_mygames\">"
					"<Attributes><Table key=\"eject\">"
					XML_PAIR("icon","%s")
					XML_PAIR("title","%s")
					XML_PAIR("info","%s") "%s"
					XML_PAIR("module_action","http://%s/mount_ps3/unmount") "%s",
					XML_HEADER, wm_icons[11], STR_EJECTDISC, STR_UNMOUNTGAME, WEB_LINK_PAIR, local_ip, "</Table>"); strcpy(myxml, templn);

	if( !(webman_config->nogrp))
	{
		if( !(webman_config->cmask & PS3)) {sprintf(templn, "<Table key=\"wm_ps3\">"
															XML_PAIR("icon","%s")
															XML_PAIR("title","PLAYSTATION\xC2\xAE\x33")
															XML_PAIR("info","%'i %s") "%s",
															wm_icons[0], item_count[3], STR_PS3FORMAT, STR_NOITEM_PAIR); strcat(myxml, templn);}
		if( !(webman_config->cmask & PS2)) {sprintf(templn, "<Table key=\"wm_ps2\">"
															XML_PAIR("icon","%s")
															XML_PAIR("title","PLAYSTATION\xC2\xAE\x32")
															XML_PAIR("info","%'i %s") "%s",
															wm_icons[2], item_count[2], STR_PS2FORMAT, STR_NOITEM_PAIR); strcat(myxml, templn);}
#ifdef COBRA_ONLY
		if( !(webman_config->cmask & PS1)) {sprintf(templn, "<Table key=\"wm_psx\">"
															XML_PAIR("icon","%s")
															XML_PAIR("title","PLAYSTATION\xC2\xAE")
															XML_PAIR("info","%'i %s") "%s",
															wm_icons[1], item_count[1], STR_PS1FORMAT, STR_NOITEM_PAIR);strcat(myxml, templn);}
		if( !(webman_config->cmask & PSP)) {sprintf(templn, "<Table key=\"wm_psp\">"
															XML_PAIR("icon","%s")
															XML_PAIR("title","PLAYSTATION\xC2\xAEPORTABLE")
															XML_PAIR("info","%'i %s") "%s",
															wm_icons[3], item_count[4], STR_PSPFORMAT, STR_NOITEM_PAIR);strcat(myxml, templn);}
		if( !(webman_config->cmask & DVD) ||
            !(webman_config->cmask & BLU)) {sprintf(templn, "<Table key=\"wm_dvd\">"
															XML_PAIR("icon","%s")
															XML_PAIR("title","%s")
															XML_PAIR("info","%'i %s") "%s",
															wm_icons[4], STR_VIDFORMAT, item_count[0], STR_VIDEO, STR_NOITEM_PAIR); strcat(myxml, templn);}
#endif
	}

	if(!webman_config->noset)
	{
		sprintf(templn, "<Table key=\"setup\">"
						 XML_PAIR("icon","%s")
						 XML_PAIR("title","%s")
						 XML_PAIR("info","%s") "%s",
						 add_xmbm_plus ? "/dev_hdd0/game/XMBMANPLS/USRDIR/IMAGES/multiman.png" : wm_icons[10],
						 STR_WMSETUP, STR_WMSETUP2, WEB_LINK_PAIR); strcat(myxml, templn);

		if(add_xmbm_plus)
			strcat(myxml, XML_PAIR("child","segment"));
		else
			{sprintf(templn, XML_PAIR("module_action","http://%s/setup.ps3"), local_ip); strcat(myxml, templn);}

		strcat(myxml, "</Table>");
	}

	if(!(webman_config->nogrp))
	{
        strcat(myxml, "</Attributes><Items>");
		if( !(webman_config->noset) )
		{
			if(add_xmbm_plus)
#ifdef ENGLISH_ONLY
				strcat(myxml, QUERY_XMB("setup", "xmb://localhost/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN.xml#seg_webman_links_items"));
#else
			{
				sprintf(templn, QUERY_XMB("setup", "xmb://localhost/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN%s.xml#seg_webman_links_items"), lang_code);
				strcat(myxml, templn);
			}
#endif
			else
				strcat(myxml, ADD_XMB_ITEM("setup"));
		}

		if(!add_xmbm_plus) strcat(myxml, ADD_XMB_ITEM("eject"));

		if(!(webman_config->cmask & PS3)) strcat(myxml, QUERY_XMB("wm_ps3", "#seg_wm_ps3_items"));
		if(!(webman_config->cmask & PS2)) strcat(myxml, QUERY_XMB("wm_ps2", "#seg_wm_ps2_items"));
#ifdef COBRA_ONLY
		if(!(webman_config->cmask & PS1)) strcat(myxml, QUERY_XMB("wm_psx", "#seg_wm_psx_items"));
		if(!(webman_config->cmask & PSP)) strcat(myxml, QUERY_XMB("wm_psp", "#seg_wm_psp_items"));
		if(!(webman_config->cmask & DVD) ||
		   !(webman_config->cmask & BLU)) strcat(myxml, QUERY_XMB("wm_dvd", "#seg_wm_dvd_items"));
#endif

		strcat(myxml, "</Items></View>");
	}

	// --- save xml file
	int fdxml=0;
	cellFsOpen(xml, CELL_FS_O_CREAT | CELL_FS_O_TRUNC | CELL_FS_O_WRONLY, &fdxml, NULL, 0);
	cellFsWrite(fdxml, (char*)myxml, strlen(myxml), NULL);

	if( (webman_config->nogrp))
	{
		cellFsWrite(fdxml, (char*)myxml_ps3, strlen(myxml_ps3), NULL);
		cellFsWrite(fdxml, (char*)"</Attributes><Items>", 20, NULL);
		cellFsWrite(fdxml, (char*)myxml_items, strlen(myxml_items), NULL);
		sprintf(myxml, "</Items></View></XMBML>\r\n");
	}
	else
	{
		if(!(webman_config->cmask & PS3)) cellFsWrite(fdxml, (char*)myxml_ps3, strlen(myxml_ps3), NULL);
		if(!(webman_config->cmask & PS2)) cellFsWrite(fdxml, (char*)myxml_ps2, strlen(myxml_ps2), NULL);
#ifdef COBRA_ONLY
		if(!(webman_config->cmask & PS1)) cellFsWrite(fdxml, (char*)myxml_psx, strlen(myxml_psx), NULL);
		if(!(webman_config->cmask & PSP)) cellFsWrite(fdxml, (char*)myxml_psp, strlen(myxml_psp), NULL);
		if(!(webman_config->cmask & DVD) || !(webman_config->cmask & BLU)) cellFsWrite(fdxml, (char*)myxml_dvd, strlen(myxml_dvd), NULL);
#endif
		sprintf(myxml, "</XMBML>\r\n");
	}

	cellFsWrite(fdxml, (char*)myxml, strlen(myxml), NULL);
	cellFsClose(fdxml);
	cellFsChmod(xml, MODE);

	// --- replace & with ^ for droidMAN

	if(cellFsOpen(xml, CELL_FS_O_RDONLY, &fdxml, NULL, 0) == CELL_FS_SUCCEEDED)
	{
		u64 read_e = 0;
		u32 xmlsize=BUFFER_SIZE;
		cellFsRead(fdxml, (void *)myxml_ps3, xmlsize, &read_e);
		cellFsClose(fdxml);
		for(u32 n=0;n<read_e;n++) if(myxml_ps3[n]=='&') myxml_ps3[n]='^';

		strcpy(xml+37, ".droid\0"); // .xml -> .droid
		cellFsOpen((char*)xml, CELL_FS_O_CREAT | CELL_FS_O_TRUNC | CELL_FS_O_WRONLY, &fdxml, NULL, 0);
		cellFsWrite(fdxml, (char*)myxml_ps3, strlen(myxml_ps3), NULL);
		cellFsClose(fdxml);
	}

	// --- release allocated memory

	led(GREEN, ON);

#ifdef USE_VM
	sys_vm_unmap(sysmem);
#else
	sys_memory_free(sysmem);
#endif

	return true;
}

static void refresh_xml(char *msg)
{
	webman_config->profile=profile; save_settings();

	sprintf(msg, "%s XML%s: %s", STR_REFRESH, SUFIX2(profile), STR_SCAN2);
	show_msg((char*) msg);
	init_running = 1;
	sys_ppu_thread_t id3;
	sys_ppu_thread_create(&id3, handleclient, (u64)REFRESH_CONTENT, -0x1d8, 0x20000, 0, "wwwd2");
	while(init_running && working) sys_timer_usleep(300000);
	sprintf(msg, "%s XML%s: OK", STR_REFRESH, SUFIX2(profile));
	show_msg((char*) msg);
}

