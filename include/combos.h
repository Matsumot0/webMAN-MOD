/*
 FAIL SAFE    : SELECT+L3+L2+R2
 RESET SAFE   : SELECT+R3+L2+R2

 REFRESH XML  : SELECT+L3 (+R2=profile1, +L2=profile2)
 UNLOAD WM    : L3+R2+R3

 PREV GAME    : SELECT+L1
 NEXT GAME    : SELECT+R1
 SHUTDOWN     : L3+R2+X
 RESTART      : L3+R2+O

 FAN CNTRL    : L3+R2+START
 SHOW TEMP    : SELECT+R3 / SELECT+START
 DYNAMIC TEMP : SELECT+LEFT/RIGHT
 MANUAL TEMP  : SELECT+UP/DOWN

 SYSCALLS     : R2+TRIANGLE
 SHOW IDPS    : R2+O
 OFFLINE MODE : R2+口

 EXT GAME DATA: SELECT+口
 MOUNT net0/  : SELECT+R2+口
 MOUNT net1/  : SELECT+L2+口

 TOGGLE PS2CLASSIC    : SELECT+L2+TRIANGLE
 SWITCH PS2EMU        : SELECT+L2+R2

 COBRA TOGGLE         : L3+L2+TRIANGLE
 REBUG  Mode Switcher : L3+L2+口
 Normal Mode Switcher : L3+L2+O
 DEBUG  Menu Switcher : L3+L2+X
*/
		bool reboot = false;

#ifdef COBRA_ONLY
		struct CellFsStat s;
#endif

		CellPadData data;

		for(u8 n=0;n<10;n++)
		{
			if(show_info_popup) {show_info_popup = false; goto show_popup;}

			if(!webman_config->nopad)
			{
#ifdef VIRTUAL_PAD
				if(vcombo)
				{
					data.len=16; data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] = (vcombo & 0xFF); data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] = (vcombo & 0xFF00) >> 8; vcombo = 0;
				}
				else
#endif
				{
					data.len=0;
					if(cellPadGetData(0, &data) != CELL_PAD_OK || data.len == 0)
						if(cellPadGetData(1, &data) != CELL_PAD_OK || data.len == 0)
							if(cellPadGetData(2, &data) != CELL_PAD_OK) {sys_timer_usleep(300000); continue;}
				}

				if(data.len > 0)
				{
					if((data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_SELECT))
					{
						if( !(webman_config->combo2 & (EXTGAMDAT | MOUNTNET0 | MOUNTNET1))         // Toggle External Game Data
                            && (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_SQUARE)) // SELECT+SQUARE
						{
#ifndef LITE_EDITION
							if(!(webman_config->combo2 & MOUNTNET0) &&
								(data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R2))
							{if(webman_config->netp0 && webman_config->neth0[0]) mount_with_mm((char*)"/net0", 1);}
							else
							if(!(webman_config->combo2 & MOUNTNET1) &&
								(data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_L2))
							{if(webman_config->netp1 && webman_config->neth1[0]) mount_with_mm((char*)"/net1", 1);}
							else
#endif

#ifdef EXT_GDATA
							set_gamedata_status(extgd^1, true);
#endif
							sys_timer_sleep(2);
							break;
						}
						else
						if( !(webman_config->combo & FAIL_SAFE)
							&& (data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_L3)
							&& (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_L2) // fail-safe mode
							&& (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R2) // SELECT+L3+L2+R2
							)
						{
							cellFsUnlink((char*)"/dev_hdd0/boot_plugins.txt");
							goto reboot; // vsh reboot
						}
						else
						if( (data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_R3)
							&& (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_L2) // reset-safe mode
							&& (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R2) // SELECT+R3+L2+R2
							)
						{
							cellFsUnlink(WMCONFIG);
							{ BEEP1 }
							show_msg((char*)STR_RMVWMCFG);
							sys_timer_sleep(2);
							show_msg((char*)STR_RMVWMCFGOK);
							sys_timer_sleep(3);
							goto reboot; // vsh reboot
						}
#ifdef COBRA_ONLY
						else
						if( !(webman_config->combo2 & PS2TOGGLE)
							&& (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_L2)
							&& (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_TRIANGLE) // SELECT+L2+TRIANGLE
							&& (c_firmware>=4.65f) )
						{
							bool classic_ps2_enabled = (cellFsStat((char*)PS2_CLASSIC_TOGGLER, &s)==CELL_FS_SUCCEEDED);

							if(classic_ps2_enabled)
							{
								disable_classic_ps2_mode();
							}
							else
							{
								enable_classic_ps2_mode();
							}

							sprintf((char*) msg, (char*)"PS2 Classic %s", classic_ps2_enabled ? STR_DISABLED : STR_ENABLED);
							show_msg((char*) msg);
							sys_timer_sleep(3);
						}
						else
						if( !(webman_config->combo2 & PS2SWITCH)
							&& (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_L2) // Clone ps2emu habib's switcher
							&& (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R2) // SELECT+L2+R2
							&& (c_firmware>=4.53f) )
						{
								enable_dev_blind((char*)"Swapping ps2emu activated!");
 #ifdef REX_ONLY
								if(cellFsStat((char*)REBUG_TOOLBOX "ps2_netemu.self", &s)==CELL_FS_SUCCEEDED)
								{
									uint64_t size1, size2;

									// ---- Backup PS2Emus to Rebug Toolbox folder ----
									if( cellFsStat( (char*)REBUG_TOOLBOX "ps2_netemu.self.cobra", &s)!=CELL_FS_SUCCEEDED )
										  filecopy( (char*)PS2_EMU_PATH  "ps2_netemu.self",
													(char*)REBUG_TOOLBOX "ps2_netemu.self.cobra", COPY_WHOLE_FILE);

									if( cellFsStat( (char*)REBUG_TOOLBOX "ps2_gxemu.self.cobra", &s)!=CELL_FS_SUCCEEDED )
										  filecopy( (char*)PS2_EMU_PATH  "ps2_gxemu.self",
													(char*)REBUG_TOOLBOX "ps2_gxemu.self.cobra", COPY_WHOLE_FILE);

									if( cellFsStat( (char*)REBUG_TOOLBOX "ps2_emu.self.cobra", &s)!=CELL_FS_SUCCEEDED )
										  filecopy( (char*)PS2_EMU_PATH  "ps2_emu.self",
													(char*)REBUG_TOOLBOX "ps2_emu.self.cobra", COPY_WHOLE_FILE);

									// ---- Swap ps2_netemu.self ----
									size1 = size2 = 0;
									if( cellFsStat((char*)PS2_EMU_PATH  "ps2_netemu.self", &s)==CELL_FS_SUCCEEDED) size1 = s.st_size;
									if( cellFsStat((char*)REBUG_TOOLBOX "ps2_netemu.self", &s)==CELL_FS_SUCCEEDED) size2 = s.st_size;

									show_msg((size1==size2) ?   (char*)"Restoring original Cobra ps2emu...":
																(char*)"Switching to custom ps2emu...");

									if(size1>0 && size2>0)
										filecopy((size1==size2) ?   (char*)REBUG_TOOLBOX "ps2_netemu.self.cobra":
																	(char*)REBUG_TOOLBOX "ps2_netemu.self",
																	(char*)PS2_EMU_PATH  "ps2_netemu.self", COPY_WHOLE_FILE);

									// ---- Swap ps2_gxemu.self ----
									size1 = size2 = 0;
									if( cellFsStat((char*)PS2_EMU_PATH  "ps2_gxemu.self", &s)==CELL_FS_SUCCEEDED) size1 = s.st_size;
									if( cellFsStat((char*)REBUG_TOOLBOX "ps2_gxemu.self", &s)==CELL_FS_SUCCEEDED) size2 = s.st_size;

									if(size1>0 && size2>0)
										filecopy((size1==size2) ?   (char*)REBUG_TOOLBOX "ps2_gxemu.self.cobra":
																	(char*)REBUG_TOOLBOX "ps2_gxemu.self",
																	(char*)PS2_EMU_PATH  "ps2_gxemu.self", COPY_WHOLE_FILE);

									// ---- Swap ps2_emu.self ----
									size1 = size2 = 0;
									if( cellFsStat((char*)PS2_EMU_PATH  "ps2_emu.self", &s)==CELL_FS_SUCCEEDED) size1 = s.st_size;
									if( cellFsStat((char*)REBUG_TOOLBOX "ps2_emu.self", &s)==CELL_FS_SUCCEEDED) size2 = s.st_size;

									if(size1>0 && size2>0)
										filecopy((size1==size2) ?   (char*)REBUG_TOOLBOX "ps2_emu.self.cobra":
																	(char*)REBUG_TOOLBOX "ps2_emu.self",
																	(char*)PS2_EMU_PATH  "ps2_emu.self", COPY_WHOLE_FILE);
								}
								else
 #endif //#ifdef REX_ONLY
								if(cellFsStat((char*)PS2_EMU_PATH "ps2_netemu.self.swap", &s)==CELL_FS_SUCCEEDED)
								{
									show_msg((char*)"Switch to custom ps2emu...");

									cellFsRename(PS2_EMU_PATH "ps2_netemu.self"     , PS2_EMU_PATH "ps2_netemu.tmp");
									cellFsRename(PS2_EMU_PATH "ps2_netemu.self.swap", PS2_EMU_PATH "ps2_netemu.self");

									cellFsRename(PS2_EMU_PATH "ps2_gxemu.self"      , PS2_EMU_PATH "ps2_gxemu.tmp");
									cellFsRename(PS2_EMU_PATH "ps2_gxemu.self.swap" , PS2_EMU_PATH "ps2_gxemu.self");

									cellFsRename(PS2_EMU_PATH "ps2_emu.self"        , PS2_EMU_PATH "ps2_emu.tmp");
									cellFsRename(PS2_EMU_PATH "ps2_emu.self.swap"   , PS2_EMU_PATH "ps2_emu.self");
								}
								else if(cellFsStat((char*)PS2_EMU_PATH "ps2_netemu.self.sp", &s)==CELL_FS_SUCCEEDED)
								{
									show_msg((char*)"Switching to custom ps2emu...");

									cellFsRename(PS2_EMU_PATH "ps2_netemu.self"   , PS2_EMU_PATH "ps2_netemu.tmp");
									cellFsRename(PS2_EMU_PATH "ps2_netemu.self.sp", PS2_EMU_PATH "ps2_netemu.self");

									cellFsRename(PS2_EMU_PATH "ps2_gxemu.self"    , PS2_EMU_PATH "ps2_gxemu.tmp");
									cellFsRename(PS2_EMU_PATH "ps2_gxemu.self.sp" , PS2_EMU_PATH "ps2_gxemu.self");

									cellFsRename(PS2_EMU_PATH "ps2_emu.self"      , PS2_EMU_PATH "ps2_emu.tmp");
									cellFsRename(PS2_EMU_PATH "ps2_emu.self.sp"   , PS2_EMU_PATH "ps2_emu.self");
								}
								else if(cellFsStat(PS2_EMU_PATH "ps2_netemu.tmp", &s)==CELL_FS_SUCCEEDED)
								{
									show_msg((char*)"Restoring original ps2emu...");

									if(c_firmware>=4.65f)
									{
										cellFsRename(PS2_EMU_PATH "ps2_netemu.self", PS2_EMU_PATH "ps2_netemu.self.swap");
										cellFsRename(PS2_EMU_PATH "ps2_netemu.tmp" , PS2_EMU_PATH "ps2_netemu.self");

										cellFsRename(PS2_EMU_PATH "ps2_gxemu.self" , PS2_EMU_PATH "ps2_gxemu.self.swap");
										cellFsRename(PS2_EMU_PATH "ps2_gxemu.tmp"  , PS2_EMU_PATH "ps2_gxemu.self");

										cellFsRename(PS2_EMU_PATH "ps2_emu.self"   , PS2_EMU_PATH "ps2_emu.self.swap");
										cellFsRename(PS2_EMU_PATH "ps2_emu.tmp"    , PS2_EMU_PATH "ps2_emu.self");
									}
									else
									{
										cellFsRename(PS2_EMU_PATH "ps2_netemu.self", PS2_EMU_PATH "ps2_netemu.self.sp");
										cellFsRename(PS2_EMU_PATH "ps2_netemu.tmp" , PS2_EMU_PATH "ps2_netemu.self");

										cellFsRename(PS2_EMU_PATH "ps2_gxemu.self" , PS2_EMU_PATH "ps2_gxemu.self.sp");
										cellFsRename(PS2_EMU_PATH "ps2_gxemu.tmp"  , PS2_EMU_PATH "ps2_gxemu.self");

										cellFsRename(PS2_EMU_PATH "ps2_emu.self"   , PS2_EMU_PATH "ps2_emu.self.sp");
										cellFsRename(PS2_EMU_PATH "ps2_emu.tmp"    , PS2_EMU_PATH "ps2_emu.self");
									}
								}
						}
#endif //#ifdef COBRA_ONLY

						else
						if(!(webman_config->combo2 & XMLREFRSH) && (data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_L3) ) // SELECT+L3 refresh XML
						{
							if(data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] == CELL_PAD_CTRL_R2) profile=1; else
							if(data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] == CELL_PAD_CTRL_L2) profile=2; else
							if(data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] == CELL_PAD_CTRL_R1) profile=3; else
							if(data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] == CELL_PAD_CTRL_L1) profile=4; else profile=0;

							refresh_xml((char*)msg);
						}
                        else
						if( (!(webman_config->combo & SHOW_TEMP) && (data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & (CELL_PAD_CTRL_R3 | CELL_PAD_CTRL_START)))) // SELECT+START show temperatures / hdd space
						{
#ifdef VIDEO_REC
							if(!(webman_config->combo2 & VIDRECORD) && data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_R3) // SELECT + R3
							{
								toggle_video_rec();
								sys_timer_sleep(2);
								break;
							}
#endif

#ifdef EXTRA_FEAT
							if(data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] == (CELL_PAD_CTRL_R2 | CELL_PAD_CTRL_L2) )
								saveBMP();
							else
#endif
							{
								/////////////////////////////
#ifdef COPY_PS3
								if(copy_in_progress)
								{
									sprintf((char*) msg, "%s %s (%i %s)", STR_COPYING, current_file, copied_count, STR_FILES);
									show_msg((char*) msg);
									sys_timer_sleep(2);
									if(data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & (CELL_PAD_CTRL_R2 | CELL_PAD_CTRL_L2) ) break;
								}
								else
								if(fix_in_progress)
								{
									sprintf((char*) msg, "%s %s", STR_FIXING, current_file);
									show_msg((char*) msg);
									sys_timer_sleep(2);
									if(data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & (CELL_PAD_CTRL_R2 | CELL_PAD_CTRL_L2) ) break;
								}
#endif
								/////////////////////////////

								CellRtcTick pTick; u32 dd, hh, mm, ss; char tmp[200];
show_popup:
								cellRtcGetCurrentTick(&pTick);
								get_temperature(0, &t1);
								get_temperature(1, &t2);

								uint32_t blockSize;
								uint64_t freeSize;
								cellFsGetFreeSize((char*)"/dev_hdd0", &blockSize, &freeSize);

								u8 st, mode, speed, unknown;
								if(get_fan_policy_offset)
								{
									if(!webman_config->fanc)
									{
										backup[5]=peekq(get_fan_policy_offset);
										lv2poke32(get_fan_policy_offset, 0x38600001); // sys 409 get_fan_policy  4.55/4.60/4.65/4.70/4.75/4.76
									}

									sys_sm_get_fan_policy(0, &st, &mode, &speed, &unknown);

									if(!webman_config->fanc)
									{
										pokeq(get_fan_policy_offset, backup[5]); // sys 409 get_fan_policy  4.55/4.60/4.65/4.70/4.75/4.76
									}
								}
								_meminfo meminfo;
								{system_call_1(SC_GET_FREE_MEM, (uint64_t)(u32) &meminfo);}

								// detect aprox. time when a game is launched
								if(View_Find("game_plugin")==0) gTick=rTick; else if(gTick.tick==rTick.tick) cellRtcGetCurrentTick(&gTick);

								bool R2 = (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] == CELL_PAD_CTRL_R2), bb;

								///// startup/play time /////
								bb = (!R2 && gTick.tick>rTick.tick); // show play time
								ss = (u32)((pTick.tick-(bb?gTick.tick:rTick.tick))/1000000); dd = (u32)(ss / 86400);
								if(dd>100) {bb=false; ss = (u32)((pTick.tick-rTick.tick)/1000000); dd = (u32)(ss / 86400);}
								ss = ss % 86400; hh = (u32)(ss / 3600); ss = ss % 3600; mm = (u32)(ss / 60); ss = ss % 60;
								/////////////////////////////

								net_info info;
								memset(&info, 0, sizeof(net_info));
								xsetting_F48C0548()->sub_44A47C(&info);

								char net_type[8] = "";
								if (info.device == 0) strcpy(net_type, "LAN"); else
								if (info.device == 1) strcpy(net_type, "WLAN");

								int32_t ip_size = 0x10;
								char ip[ip_size];
								netctl_main_9A528B81(ip_size, ip);

								char cfw_info[20];
#ifdef COBRA_ONLY
								bool is_mamba; {system_call_1(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_GET_MAMBA); is_mamba = ((int)p1 ==0x666);}

								uint16_t cobra_version; sys_get_version2(&cobra_version);

								char cobra_ver[8];
								if((cobra_version & 0x0F) == 0)
									sprintf(cobra_ver, "%X.%X", cobra_version>>8, (cobra_version & 0xFF) >> 4);
								else
									sprintf(cobra_ver, "%X.%02X", cobra_version>>8, (cobra_version & 0xFF));

								sprintf(cfw_info, "%s %s: %s", dex_mode ? "DEX" : "CEX", is_mamba ? "Mamba" : "Cobra", cobra_ver);
#else
								sprintf(cfw_info, "%s", dex_mode ? "DEX" : "CEX");
#endif
								char smax[16]; if(max_temp) sprintf(smax, "   MAX: %i°C", max_temp); else if(webman_config->fanc==0) sprintf(smax, "   SYSCON"); else memset(smax, 0, 16);

								sprintf((char*)tmp, "CPU: %i°C  RSX: %i°C  FAN: %i%%   \r\n"
													"%s: %id %02d:%02d:%02d%s\r\n"
													"Firmware : %i.%02i %s\r\n"
													"IP: %s  %s",
													t1>>24, t2>>24, (int)(((int)speed*100)/255),
													bb?"Play":"Startup", dd, hh, mm, ss, smax,
													(int)c_firmware, ((u32)(c_firmware * 1000.0f) % 1000) / 10, cfw_info, ip, net_type);

								sprintf((char*)msg, "%s\r\n%s: %'i %s\r\n"
													"%s: %'i %s\r\n", tmp,
													STR_STORAGE, (int)((blockSize*freeSize)>>20), STR_MBFREE,
													STR_MEMORY, meminfo.avail>>10, STR_KBFREE);

								if(R2 && gTick.tick>rTick.tick)
								{
									////// play time //////
									ss = (u32)((pTick.tick-gTick.tick)/1000000);
									dd = (u32)(ss / 86400); ss = ss % 86400; hh = (u32)(ss / 3600); ss = ss % 3600; mm = (u32)(ss / 60); ss = ss % 60;

									if(dd<100) {char gname[200]; get_game_info(); sprintf(gname, "%s %s\r\n\r\n", _game_TitleID, _game_Title); sprintf((char*) msg, "%sPlay: %id %02d:%02d:%02d\r\n%s", gname, dd, hh, mm, ss, tmp); }
								}

								show_msg((char*) msg);
								sys_timer_sleep(2);
							}
						}
						else
						if(webman_config->fanc && !(webman_config->combo & MANUALFAN) && (data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_UP) ) // SELECT+UP increase TEMP/FAN
						{
							if(max_temp) //auto mode
							{
								if(data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R2) max_temp+=5; else max_temp+=1;
								if(max_temp>85) max_temp=85;
								webman_config->temp1=max_temp;
								sprintf((char*) msg, "%s\r\n%s %i°C", STR_FANCH0, STR_FANCH1, max_temp);
							}
							else
							{
								if(data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R2) webman_config->manu+=5; else webman_config->manu+=1;
								webman_config->manu=RANGE(webman_config->manu, 20, 95); //%
								webman_config->temp0= (u8)(((float)(webman_config->manu+1) * 255.f)/100.f);
								webman_config->temp0=RANGE(webman_config->temp0, 0x33, MAX_FANSPEED);
								fan_control(webman_config->temp0, 0);
								sprintf((char*) msg, "%s\r\n%s %i%%", STR_FANCH0, STR_FANCH2, webman_config->manu);
							}
							save_settings();
							show_msg((char*) msg);
							sys_timer_sleep(2);
						}
						else
						if(webman_config->fanc && !(webman_config->combo & MANUALFAN) && (data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_DOWN) ) // SELECT+DOWN increase TEMP/FAN
						{
							if(max_temp) //auto mode
							{
								if(max_temp>30) {if(data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R2) max_temp-=5; else max_temp-=1;}
								webman_config->temp1=max_temp;
								sprintf((char*) msg, "%s\r\n%s %i°C", STR_FANCH0, STR_FANCH1, max_temp);
							}
							else
							{
								if(webman_config->manu>20) {if(data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R2) webman_config->manu-=5; else webman_config->manu-=1;}
								webman_config->temp0= (u8)(((float)(webman_config->manu+1) * 255.f)/100.f);
								if(webman_config->temp0<0x33) webman_config->temp0=0x33;
								if(webman_config->temp0>MAX_FANSPEED) webman_config->temp0=MAX_FANSPEED;
								fan_control(webman_config->temp0, 0);
								sprintf((char*) msg, "%s\r\n%s %i%%", STR_FANCH0, STR_FANCH2, webman_config->manu);
							}
							save_settings();
							show_msg((char*) msg);
							sys_timer_sleep(2);
						}
						else
						if(webman_config->minfan && !(webman_config->combo & MINDYNFAN) && (data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_LEFT) ) // SELECT+LEFT decrease Minfan
						{
							if(webman_config->minfan-5>=MIN_FANSPEED) webman_config->minfan-=5;
							sprintf((char*) msg, "%s\r\n%s %i%%", STR_FANCH0, STR_FANCH3, webman_config->minfan);

							save_settings();
							show_msg((char*) msg);
							sys_timer_sleep(2);
						}
						else
						if(webman_config->minfan && !(webman_config->combo & MINDYNFAN) && (data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_RIGHT) ) // SELECT+RIGHT increase Minfan
						{
							if(webman_config->minfan+5<100) webman_config->minfan+=5;
							sprintf((char*) msg, "%s\r\n%s %i%%", STR_FANCH0, STR_FANCH3, webman_config->minfan);

							save_settings();
							show_msg((char*) msg);
							sys_timer_sleep(2);
						}
						else
						if(!(webman_config->combo & PREV_GAME) && (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] == CELL_PAD_CTRL_L1) ) // SELECT+L1 (previous title)
						{
							led(GREEN, BLINK_FAST);
							mount_with_mm((char*)"_prev", 1);
							sys_timer_sleep(3);
							led(GREEN, ON);
						}
						else
						if(!(webman_config->combo & NEXT_GAME) && (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] == CELL_PAD_CTRL_R1) ) // SELECT+R1 (next title)
						{
							led(GREEN, BLINK_FAST);
							mount_with_mm((char*)"_next", 1);
							sys_timer_sleep(3);
							led(GREEN, ON);
						}
						else
						if(!(webman_config->combo & UMNT_GAME) && (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] == CELL_PAD_CTRL_CIRCLE) ) // SELECT+O (unmount)
						{
							do_umount(true);
						}
					}
					else
					if((data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_L3) && (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R2))
					{
						if(!(webman_config->combo & SHUT_DOWN) && (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_CROSS) ) // L3+R2+X (shutdown)
						{
							// power off
							working = 0;
							{ DELETE_TURNOFF } { BEEP1 }

							{system_call_4(SC_SYS_POWER, SYS_SHUTDOWN, 0, 0, 0);}

							sys_ppu_thread_exit(0);
						}
						else if(!(webman_config->combo & RESTARTPS) && (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_CIRCLE) ) // L3+R2+O (lpar restart)
						{
							// lpar restart
							working = 0;
							{ DELETE_TURNOFF }{ BEEP2 }

							{system_call_3(SC_SYS_POWER, SYS_REBOOT, NULL, 0);}

							sys_ppu_thread_exit(0);
						}
						else if(!(webman_config->combo & UNLOAD_WM) && (data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_R3) ) // L3+R3+R2 (quit webMAN)
						{
#ifdef COBRA_ONLY
							char tmp_name[30];
							char tmp_filename[256];

#ifndef SYSCALL8_OPCODE_PS3MAPI
							#define SYSCALL8_OPCODE_PS3MAPI			 			0x7777
							#define PS3MAPI_OPCODE_GET_VSH_PLUGIN_INFO			0x0047
#endif
							for (unsigned int slot = 1; slot < 7; slot++)
							{
								memset(tmp_name, 0, sizeof(tmp_name));
								memset(tmp_filename, 0, sizeof(tmp_filename));
								{system_call_5(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_VSH_PLUGIN_INFO, (u64)slot, (u64)(u32)tmp_name, (u64)(u32)tmp_filename); }
								if(!strcmp(tmp_filename, "/dev_hdd0/plugins/wm_vsh_menu.sprx")) {cobra_unload_vsh_plugin(slot); break;}
							}
#endif
							if(!webman_config->fanc || webman_config->ps2temp<33)
								restore_fan(0); //restore syscon fan control mode
							else
								restore_fan(1); //set ps2 fan control mode

							working = 0;
							wm_unload_combo = 1;

							stop_prx_module();
							sys_ppu_thread_exit(0);
							break;
						}
						else if(!(webman_config->combo & DISABLEFC) && (data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_START) ) // L3+R2+START (enable/disable fancontrol)
						{
							enable_fan_control(2, msg);
							sys_timer_sleep(2);
							break;
						}
					}
					else
					if(!(webman_config->combo & RESTARTPS) && (data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_L3) && (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R1))
					{
						if(data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_CROSS) // L3+R1+X (vsh shutdown)
						{
							// vsh shutdown
							working = 0;
							{ DELETE_TURNOFF }{ BEEP1 }

							vshmain_87BB0001(1); // VSH shutdown

							sys_ppu_thread_exit(0);
						}
						else
						if(data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_CIRCLE) // L3+R1+O (vsh restart)
						{
							// vsh reboot
							working = 0;
							{ DELETE_TURNOFF }{ BEEP2 }

							vshmain_87BB0001(2); // VSH reboot

							sys_ppu_thread_exit(0);
						}
					}
					else
					if(data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R2)
					{
						if((copy_in_progress || fix_in_progress) && data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_CIRCLE) // R2+O Abort copy process
						{
							fix_aborted=copy_aborted=true;
						}
#ifdef REMOVE_SYSCALLS
						else
						if(!(webman_config->combo & DISABLESH) && (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_TRIANGLE) ) // R2+TRIANGLE Disable CFW Sycalls
						{
							if(peekq(0x8000000000003000ULL)==SYSCALLS_UNAVAILABLE) {
								{ BEEP2 }
								show_msg((char*)STR_CFWSYSALRD);
								sys_timer_sleep(2);
							} else {
								show_msg((char*)STR_CFWSYSRIP);
								remove_cfw_syscalls();
								delete_history(true);
								if(peekq(0x8000000000003000ULL)==SYSCALLS_UNAVAILABLE) {
									{ BEEP1 }
									show_msg((char*)STR_RMVCFWSYS);
									sys_timer_sleep(2);
								} else {
									{ BEEP2 }
									show_msg((char*)STR_RMVCFWSYSF);
									sys_timer_sleep(2);
								}
							}
						}
#endif
						else
						if(!(webman_config->combo2 & BLOCKSVRS) && (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_SQUARE) ) // R2+SQUARE
						{
							show_msg((char*)"Blocking servers");
							block_online_servers();
							show_msg((char*)"Servers blocked");
						}
						else
						if(!(webman_config->combo & SHOW_IDPS) && (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_CIRCLE) ) // R2+O Show IDPS EID0+LV2
						{
							vshmain_is_ss_enabled = (void*)((int)getNIDfunc("vshmain", 0x981D7E9F, 0)); //is screenshot enabled?


							if(vshmain_is_ss_enabled()==0)
							{
								set_SSHT_ = (uint32_t*)&opd;
								memcpy(set_SSHT_, vshmain_is_ss_enabled, 8);
								opd[0] -= 0x2C; // Sub before vshmain_981D7E9F sets Screenshot Flag
								set_SSHT_(1);	// enable screenshot

								show_msg((char*)"Screenshot enabled");
								sys_timer_sleep(2);
							}

							uint64_t eid0_idps[2], buffer[0x40], start_sector;
							uint32_t read;
							sys_device_handle_t source;
							if(sys_storage_open(0x100000000000004ULL, 0, &source, 0)!=0)
							{
								start_sector = 0x204;
								sys_storage_close(source);
								sys_storage_open(0x100000000000001ULL, 0, &source, 0);
							}
							else start_sector = 0x178;
							sys_storage_read(source, 0, start_sector, 1, buffer, &read, 0);
							sys_storage_close(source);

							eid0_idps[0]=buffer[0x0E];
							eid0_idps[1]=buffer[0x0F];

							get_idps_psid();

							#define SEP "\n                  "
							sprintf((char*) msg, "IDPS EID0 : %016llX" SEP
															 "%016llX\n"
												 "IDPS LV2  : %016llX" SEP
															 "%016llX\r\n"
												 "PSID LV2 : %016llX" SEP
															"%016llX", eid0_idps[0], eid0_idps[1], IDPS[0], IDPS[1], PSID[0], PSID[1]);
							show_msg((char*) msg);
							sys_timer_sleep(2);
						}
					}
					else
					if((data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_L3) && (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_L2))
					{
#ifdef COBRA_ONLY
						if(!(webman_config->combo & DISACOBRA)
							&& (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_TRIANGLE))
						{ // L3+L2+TRIANGLE COBRA Toggle
							enable_dev_blind((char*)"COBRA Toggle activated!");
 #ifdef REX_ONLY
							if( (cellFsStat((char*) REBUG_COBRA_PATH "stage2.cex", &s)==CELL_FS_SUCCEEDED) /* &&
								(cellFsStat((char*) REBUG_COBRA_PATH "stage2.dex", &s)==CELL_FS_SUCCEEDED) */)
							{
								show_msg((char*)"REBUG COBRA is active!\r\nDeactivating COBRA...");

								cellFsRename(REBUG_COBRA_PATH "stage2.cex", REBUG_COBRA_PATH "stage2.cex.bak");
								cellFsRename(REBUG_COBRA_PATH "stage2.dex", REBUG_COBRA_PATH "stage2.dex.bak");
								reboot=true; // vsh reboot
							}
							else if((cellFsStat((char*) REBUG_COBRA_PATH "stage2.cex.bak", &s)==CELL_FS_SUCCEEDED) /* &&
									(cellFsStat((char*) REBUG_COBRA_PATH "stage2.dex.bak", &s)==CELL_FS_SUCCEEDED) */)
							{
								show_msg((char*)"REBUG COBRA is inactive!\r\nActivating COBRA...");

								cellFsRename(REBUG_COBRA_PATH "stage2.cex.bak", REBUG_COBRA_PATH "stage2.cex");
								cellFsRename(REBUG_COBRA_PATH "stage2.dex.bak", REBUG_COBRA_PATH "stage2.dex");
								reboot=true; // vsh reboot
							}
 #else

							if(cellFsStat((char*)HABIB_COBRA_PATH "stage2.cex", &s)==CELL_FS_SUCCEEDED)
							{
								show_msg((char*)"COBRA is active!\r\nDeactivating COBRA...");

								cellFsRename(HABIB_COBRA_PATH "stage2.cex", HABIB_COBRA_PATH "stage2_disabled.cex");

								reboot=true; // vsh reboot
							}
							else if(cellFsStat((char*)HABIB_COBRA_PATH "stage2_disabled.cex", &s)==CELL_FS_SUCCEEDED)
							{
								show_msg((char*)"COBRA is inactive!\r\nActivating COBRA...");

								cellFsRename(HABIB_COBRA_PATH "stage2_disabled.cex", HABIB_COBRA_PATH "stage2.cex");

								reboot=true; // vsh reboot
							}

							if(cellFsStat((char*)SYS_COBRA_PATH "stage2.bin", &s)==CELL_FS_SUCCEEDED)
							{
								show_msg((char*)"COBRA is active!\r\nDeactivating COBRA...");

								cellFsRename(SYS_COBRA_PATH "stage2.bin", SYS_COBRA_PATH "stage2_disabled.bin");

								if(cellFsStat((char*)COLDBOOT_PATH ".normal", &s)==CELL_FS_SUCCEEDED)
								{
									cellFsRename(COLDBOOT_PATH          , COLDBOOT_PATH ".cobra");
									cellFsRename(COLDBOOT_PATH ".normal", COLDBOOT_PATH);
								}

								reboot=true; // vsh reboot
							}
							else if(cellFsStat((char*)SYS_COBRA_PATH "stage2_disabled.bin", &s)==CELL_FS_SUCCEEDED)
							{
								show_msg((char*)"COBRA is inactive!\r\nActivating COBRA...");

								cellFsRename(SYS_COBRA_PATH "stage2_disabled.bin", SYS_COBRA_PATH "stage2.bin");

								if(cellFsStat((char*)COLDBOOT_PATH ".cobra", &s)==CELL_FS_SUCCEEDED)
								{
									cellFsRename(COLDBOOT_PATH         , COLDBOOT_PATH ".normal");
									cellFsRename(COLDBOOT_PATH ".cobra", COLDBOOT_PATH);
								}

								reboot=true; // vsh reboot
							}
 #endif //#ifdef REX_ONLY
						}
#endif //#ifdef COBRA_ONLY

#ifdef REX_ONLY
						if(!(webman_config->combo2 & REBUGMODE)
							&& (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_SQUARE))
						{ // L3+L2+口 REBUG Mode Switcher
							enable_dev_blind((char*)"REBUG Mode Switcher activated!");

							if(cellFsStat((char*) VSH_MODULE_PATH "vsh.self.swp", &s)==CELL_FS_SUCCEEDED)
							{
								show_msg((char*)"Normal Mode detected!\r\nSwitch to REBUG Mode Debug XMB...");
								sys_timer_sleep(3);

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

								reboot=true; // vsh reboot
							}
							else
							if((cellFsStat((char*) VSH_MODULE_PATH "vsh.self.nrm", &s)==CELL_FS_SUCCEEDED)
							&& (cellFsStat((char*) VSH_MODULE_PATH "vsh.self.cexsp", &s)==CELL_FS_SUCCEEDED))
							{
								show_msg((char*)"REBUG Mode Debug XMB detected!\r\nSwitch to Retail XMB...");
								sys_timer_sleep(3);

								cellFsRename(VSH_MODULE_PATH "vsh.self", VSH_MODULE_PATH "vsh.self.dexsp");
								cellFsRename(VSH_MODULE_PATH "vsh.self.cexsp", VSH_MODULE_PATH "vsh.self");

								reboot=true; // vsh reboot
							}
							else
							if(cellFsStat((char*) VSH_MODULE_PATH "vsh.self.dexsp", &s)==CELL_FS_SUCCEEDED)
							{
								show_msg((char*)"REBUG Mode Retail XMB detected!\r\nSwitch to Debug XMB...");
								sys_timer_sleep(3);

								cellFsRename(VSH_MODULE_PATH "vsh.self", VSH_MODULE_PATH "vsh.self.cexsp");
								cellFsRename(VSH_MODULE_PATH "vsh.self.dexsp", VSH_MODULE_PATH "vsh.self");

								reboot=true; // vsh reboot
							}
						}
						else
						if(!(webman_config->combo2 & NORMAMODE)
							&& (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_CIRCLE))
						{ // L3+L2+O Normal Mode Switcher
							enable_dev_blind((char*)"Normal Mode Switcher activated!");

							if((cellFsStat((char*) VSH_MODULE_PATH "vsh.self.nrm", &s)==CELL_FS_SUCCEEDED)
							&& (cellFsStat(VSH_MODULE_PATH "vsh.self.cexsp", &s)==CELL_FS_SUCCEEDED))
							{
								show_msg((char*)"REBUG Mode Debug XMB detected!\r\nSwitch to Normal Mode...");

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

								reboot=true; // vsh reboot
							}
							else
							if(cellFsStat((char*) VSH_MODULE_PATH "vsh.self.dexsp", &s)==CELL_FS_SUCCEEDED)
							{
								show_msg((char*)"REBUG Mode Retail XMB detected!\r\nSwitch to Normal Mode...");

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

								cellFsRename(VSH_MODULE_PATH "vsh.self.dexsp", VSH_MODULE_PATH "vsh.self.swp");
								cellFsRename(VSH_MODULE_PATH "vsh.self", VSH_MODULE_PATH "vsh.self.cexsp");
								cellFsRename(VSH_MODULE_PATH "vsh.self.nrm", VSH_MODULE_PATH "vsh.self");

								reboot=true; // vsh reboot
							}
							else
							if(cellFsStat((char*) VSH_MODULE_PATH "vsh.self.swp", &s)==CELL_FS_SUCCEEDED)
							{
								show_msg((char*)"Normal Mode detected!\r\nNo need to switch!");
								sys_timer_sleep(3);
								{system_call_3(SC_FS_UMOUNT, (u64)(char*)"/dev_blind", 0, 1);}
								break;
							}
						}
						else
						if(!(webman_config->combo2 & DEBUGMENU)
							&& (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_CROSS))
						{ // L3+L2+X DEBUG Menu Switcher
							enable_dev_blind((char*)"Debug Menu Switcher activated!");

							if(cellFsStat((char*) VSH_MODULE_PATH "sysconf_plugin.sprx.dex", &s)==CELL_FS_SUCCEEDED)
							{
								show_msg((char*)"CEX QA Menu is active!\r\nSwitch to DEX Debug Menu...");

								cellFsRename(VSH_MODULE_PATH "sysconf_plugin.sprx", VSH_MODULE_PATH "sysconf_plugin.sprx.cex");
								cellFsRename(VSH_MODULE_PATH "sysconf_plugin.sprx.dex", VSH_MODULE_PATH "sysconf_plugin.sprx");
							}
							else
							if(cellFsStat((char*) VSH_MODULE_PATH "sysconf_plugin.sprx.cex", &s)==CELL_FS_SUCCEEDED)
							{
								show_msg((char*)"DEX Debug Menu is active!\r\nSwitch to CEX QA Menu...");

								cellFsRename(VSH_MODULE_PATH "sysconf_plugin.sprx", VSH_MODULE_PATH "sysconf_plugin.sprx.dex");
								cellFsRename(VSH_MODULE_PATH "sysconf_plugin.sprx.cex", VSH_MODULE_PATH "sysconf_plugin.sprx");
							}
							sys_timer_sleep(1);
							{system_call_3(SC_FS_UMOUNT, (u64)(char*)"/dev_blind", 0, 1);}
						}
#endif //#ifdef REX_ONLY
					}
				}

				if(reboot)
				{
					sys_timer_sleep(1);
					// reboot
					show_msg((char*)"Switching successful! Reboot now...");
					sys_timer_sleep(3);
					{system_call_3(SC_FS_UMOUNT, (u64)(char*)"/dev_blind", 0, 1);}
reboot:
					// vsh reboot
					working = 0;
					{ DELETE_TURNOFF }
					savefile((char*)WMNOSCAN, NULL, 0);

					vshmain_87BB0001(2); // VSH reboot

					sys_ppu_thread_exit(0);
				}

			}
			//sys_timer_sleep(step);
			sys_timer_usleep(300000);
		}
