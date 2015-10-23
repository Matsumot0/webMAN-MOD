#define MAX_LAST_GAMES (5)

// File name TAGS:
// [auto]   Auto-play
// [online] Auto-disable syscalls
// [gd]     Auto-enable external gameDATA
// [raw]    Use raw_iso.sprx to mount the ISO (ntfs)
// [PS2]    PS2 extracted folders in /PS2DISC (needs PS2_DISC compilation flag)

typedef struct
{
	uint8_t last;
	char game[MAX_LAST_GAMES][MAX_PATH_LEN];
} __attribute__((packed)) _lastgames;

#ifndef COBRA_ONLY
 static u64 base_addr=0;
 static u64 open_hook=0;

 typedef struct
 {
	char src[384];
	char dst[384];
 } redir_files_struct;
 static redir_files_struct file_to_map[10];
#endif

static void detect_firmware(void)
{
	if(c_firmware>3.40f) return;

	u64 CEX=0x4345580000000000ULL;
	u64 DEX=0x4445580000000000ULL;

	dex_mode=0;

	if(peekq(0x80000000002ED818ULL)==CEX) {SYSCALL_TABLE = SYSCALL_TABLE_475;  c_firmware=(peekq(0x80000000002FCB68ULL)==0x323031352F30382FULL)?4.76f:4.75f;} else
	if(peekq(0x800000000030F2D0ULL)==DEX) {SYSCALL_TABLE = SYSCALL_TABLE_475D; c_firmware=(peekq(0x800000000031EF48ULL)==0x323031352F30382FULL)?4.76f:4.75f; dex_mode=2;}	else
	if(peekq(0x80000000002ED778ULL)==CEX) {SYSCALL_TABLE = SYSCALL_TABLE_470;  c_firmware=4.70f;}				else
	if(peekq(0x800000000030F240ULL)==DEX) {SYSCALL_TABLE = SYSCALL_TABLE_470D; c_firmware=4.70f; dex_mode=2;}	else
	if(peekq(0x80000000002ED860ULL)==CEX) {SYSCALL_TABLE = SYSCALL_TABLE_465;  c_firmware=(peekq(0x80000000002FC938ULL)==0x323031342F31312FULL)?4.66f:4.65f;} else
	if(peekq(0x800000000030F1A8ULL)==DEX) {SYSCALL_TABLE = SYSCALL_TABLE_465D; c_firmware=(peekq(0x800000000031EBA8ULL)==0x323031342F31312FULL)?4.66f:4.65f; dex_mode=2;} else
	if(peekq(0x80000000002ED850ULL)==CEX) {SYSCALL_TABLE = SYSCALL_TABLE_460;  c_firmware=4.60f;}				else
	if(peekq(0x800000000030F198ULL)==DEX) {SYSCALL_TABLE = SYSCALL_TABLE_460D; c_firmware=4.60f; dex_mode=2;}	else
	if(peekq(0x80000000002EC5E0ULL)==CEX) {SYSCALL_TABLE = SYSCALL_TABLE_455;  c_firmware=4.55f;}				else
	if(peekq(0x80000000002E9D70ULL)==CEX) {SYSCALL_TABLE = SYSCALL_TABLE_453;  c_firmware=4.53f;}				else
	if(peekq(0x800000000030AEA8ULL)==DEX) {SYSCALL_TABLE = SYSCALL_TABLE_453D; c_firmware=4.53f; dex_mode=2;}	else
	if(peekq(0x80000000002E9BE0ULL)==CEX) {SYSCALL_TABLE = SYSCALL_TABLE_450;  c_firmware=4.50f;}				else
	if(peekq(0x80000000002EA9B8ULL)==CEX) {SYSCALL_TABLE = SYSCALL_TABLE_446;  c_firmware=4.46f;}				else
	if(peekq(0x8000000000305410ULL)==DEX) {SYSCALL_TABLE = SYSCALL_TABLE_446D; c_firmware=4.46f; dex_mode=2;}	else
	if(peekq(0x80000000002E8610ULL)==CEX) {SYSCALL_TABLE = SYSCALL_TABLE_421;  c_firmware=4.21f;}				else
	if(peekq(0x8000000000302D88ULL)==DEX) {SYSCALL_TABLE = SYSCALL_TABLE_421D; c_firmware=4.21f; dex_mode=2;}	else
	if(peekq(0x80000000002D83D0ULL)==CEX) {SYSCALL_TABLE = SYSCALL_TABLE_355;  c_firmware=3.55f;}				else
	if(peekq(0x80000000002EFE20ULL)==DEX) {SYSCALL_TABLE = SYSCALL_TABLE_355D; c_firmware=3.55f; dex_mode=2;}	else
 //No cobra cfw but as mamba compatibility
	if(peekq(0x800000000030D6A8ULL)==DEX) {SYSCALL_TABLE = SYSCALL_TABLE_455D; c_firmware=4.55f; dex_mode=2;}	else
	if(peekq(0x8000000000309698ULL)==DEX) {SYSCALL_TABLE = SYSCALL_TABLE_450D; c_firmware=4.50f; dex_mode=2;}	else
	if(peekq(0x8000000000304EF0ULL)==DEX) {SYSCALL_TABLE = SYSCALL_TABLE_441D; c_firmware=4.41f; dex_mode=2;}	else
	if(peekq(0x80000000002EA498ULL)==CEX) {SYSCALL_TABLE = SYSCALL_TABLE_441;  c_firmware=4.41f;}				else
	if(peekq(0x80000000002EA488ULL)==CEX) {SYSCALL_TABLE = SYSCALL_TABLE_440;  c_firmware=4.40f;}				else
	if(peekq(0x80000000002E9F18ULL)==CEX) {SYSCALL_TABLE = SYSCALL_TABLE_431;  c_firmware=4.31f;}				else
	if(peekq(0x8000000000304630ULL)==DEX) {SYSCALL_TABLE = SYSCALL_TABLE_430D; c_firmware=4.30f; dex_mode=2;}	else
	if(peekq(0x80000000002E9F08ULL)==CEX) {SYSCALL_TABLE = SYSCALL_TABLE_430;  c_firmware=4.30f;}				else
#ifndef COBRA_ONLY
	if(peekq(0x80000000002CFF98ULL)==CEX) {SYSCALL_TABLE = SYSCALL_TABLE_341;  c_firmware=3.41f;}				else
	//if(peekq(0x80000000002E79C8ULL)==DEX) {c_firmware=3.41f; dex_mode=2;}	else
#endif
	{c_firmware=0.00f; return;}

	if(!SYSCALL_TABLE) {c_firmware=0.00f; return;}

#ifndef COBRA_ONLY
	if(!dex_mode)
	{   // CEX
		if(c_firmware==3.41f) {base_addr=0x2B5D30; open_hook=0x2AAFC8;} else
		if(c_firmware==3.55f) {base_addr=0x2BE0D0; open_hook=0x2B3274;} else
		if(c_firmware==4.21f) {base_addr=0x2D0C98; open_hook=0x2C2558;} else
		if(c_firmware==4.30f) {base_addr=0x2D2418; open_hook=0x2C3CD4;} else
		if(c_firmware==4.31f) {base_addr=0x2D2428; open_hook=0x2C3CE0;} else
		if(c_firmware==4.40f) {base_addr=0x2D29A8; open_hook=0x2C4284;} else
		if(c_firmware==4.41f) {base_addr=0x2D29B8; open_hook=0x2C4290;} else
		if(c_firmware==4.46f) {base_addr=0x2D2ED8; open_hook=0x2C47B0;} else
		if(c_firmware==4.50f) {base_addr=0x2D4CB8; open_hook=0x29DD20;} else
		if(c_firmware==4.53f) {base_addr=0x2D4E48; open_hook=0x29DEF8;} else
		if(c_firmware==4.55f) {base_addr=0x2D7660; open_hook=0x29F748;} else
		if(c_firmware==4.60f) {base_addr=0x2D88D0; open_hook=0x2A02BC;} else
		if(c_firmware==4.65f) {base_addr=0x2D88E0; open_hook=0x2A02C8;} else
		if(c_firmware==4.66f) {base_addr=0x2D88E0; open_hook=0x2A02C8;} else
		if(c_firmware==4.70f) {base_addr=0x2D8A70; open_hook=0x2975C0;} else
		if(c_firmware==4.75f) {base_addr=0x2D8AF0; open_hook=0x297638;} else
		if(c_firmware==4.76f) {base_addr=0x2D8AF0; open_hook=0x297638;}
	}
	else
	{   // DEX
		if(c_firmware==3.55f) {base_addr=0x2D5B20; open_hook=0x2C8A94;} else
		if(c_firmware==4.21f) {base_addr=0x2EB418; open_hook=0x2D9718;} else
		if(c_firmware==4.30f) {base_addr=0x2ECB48; open_hook=0x2DAE4C;} else
	  //if(c_firmware==4.31f) {base_addr=0x??????; open_hook=0x??????;} else
	  //if(c_firmware==4.40f) {base_addr=0x??????; open_hook=0x??????;} else
		if(c_firmware==4.41f) {base_addr=0x2ED418; open_hook=0x2DB73C;} else
		if(c_firmware==4.46f) {base_addr=0x2ED938; open_hook=0x2DBC5C;} else
		if(c_firmware==4.50f) {base_addr=0x2F4778; open_hook=0x2B81E8;} else
		if(c_firmware==4.53f) {base_addr=0x2F5F88; open_hook=0x2B83C0;} else
		if(c_firmware==4.55f) {base_addr=0x2F8730; open_hook=0x2B9C14;} else
		if(c_firmware==4.60f) {base_addr=0x2FA220; open_hook=0x2BB004;} else
		if(c_firmware==4.65f) {base_addr=0x2FA230; open_hook=0x2BB010;} else
		if(c_firmware==4.66f) {base_addr=0x2FA230; open_hook=0x2BB010;} else
		if(c_firmware==4.70f) {base_addr=0x2FA540; open_hook=0x2B2480;} else
		if(c_firmware==4.75f) {base_addr=0x2FA5B0; open_hook=0x2B24F8;} else
		if(c_firmware==4.76f) {base_addr=0x2FA5B0; open_hook=0x2B24F8;}
	}

	base_addr |=0x8000000000000000ULL;
	open_hook |=0x8000000000000000ULL;
#endif

	if(!dex_mode)
	{
		if(c_firmware>=4.55f && c_firmware<=4.76f)
		{
			get_fan_policy_offset=0x8000000000009E38ULL; // sys 409 get_fan_policy  4.55/4.60/4.65/4.70/4.75/4.76
			set_fan_policy_offset=0x800000000000A334ULL; // sys 389 set_fan_policy

			// idps / psid cex
			if(c_firmware==4.55f)
			{
				idps_offset1 = 0x80000000003E17B0ULL;
				idps_offset2 = 0x8000000000474F1CULL;
				psid_offset  = 0x8000000000474F34ULL;
			}
			else if(c_firmware>=4.60f && c_firmware<=4.66f)
			{
				idps_offset1 = 0x80000000003E2BB0ULL;
				idps_offset2 = 0x8000000000474F1CULL;
				psid_offset  = 0x8000000000474F34ULL;
			}
			else if(c_firmware==4.70f)
			{
				idps_offset1 = 0x80000000003E2DB0ULL;
				idps_offset2 = 0x8000000000474AF4ULL;
				psid_offset  = 0x8000000000474B0CULL;
			}
			else if(c_firmware==4.75f || c_firmware==4.76f)
			{
				idps_offset1 = 0x80000000003E2E30ULL;
				idps_offset2 = 0x8000000000474AF4ULL;
				psid_offset  = 0x8000000000474B0CULL;
			}
		}
		else if(c_firmware>=4.21f && c_firmware<=4.53f)
		{
			get_fan_policy_offset=0x8000000000009E28ULL; // sys 409 get_fan_policy  4.21/4.30/4.31/4.40/4.41/4.46/4.50/4.53
			set_fan_policy_offset=0x800000000000A324ULL; // sys 389 set_fan_policy
		}
		else if(c_firmware == 3.55f)
		{
			get_fan_policy_offset=0x8000000000008CBCULL; // sys 409 get_fan_policy
			set_fan_policy_offset=0x80000000000091B8ULL; // sys 389 set_fan_policy
		}
#ifndef COBRA_ONLY
		else if(c_firmware == 3.41f)
		{
			get_fan_policy_offset=0x8000000000008644ULL; // sys 409 get_fan_policy
			set_fan_policy_offset=0x8000000000008B40ULL; // sys 389 set_fan_policy
		}
#endif
	}
	else // DEX
	if(c_firmware>=4.55f && c_firmware<=4.76f)
	{
			get_fan_policy_offset=0x8000000000009EB8ULL; // sys 409 get_fan_policy  4.55/4.60/4.65/4.70/4.75/4.76
			set_fan_policy_offset=0x800000000000A3B4ULL; // sys 389 set_fan_policy

			// idps / psid dex
			if(c_firmware==4.55f)
			{
				idps_offset1 = 0x8000000000407930ULL;
				idps_offset2 = 0x8000000000494F1CULL;
				psid_offset  = 0x8000000000494F34ULL;
			}
			else if(c_firmware>=4.60f && c_firmware<=4.66f)
			{
				idps_offset1 = 0x80000000004095B0ULL;
				idps_offset2 = 0x800000000049CF1CULL;
				psid_offset  = 0x800000000049CF34ULL;
			}
			else if(c_firmware==4.70f)
			{
				idps_offset1 = 0x80000000004098B0ULL;
				idps_offset2 = 0x800000000049CAF4ULL;
				psid_offset  = 0x800000000049CB0CULL;
			}
			else if(c_firmware==4.75f || c_firmware==4.76f)
			{
				idps_offset1 = 0x8000000000409930ULL;
				idps_offset2 = 0x800000000049CAF4ULL;
				psid_offset  = 0x800000000049CB0CULL;
			}
	}
	else if(c_firmware>=4.21f && c_firmware<=4.53f)
	{
			get_fan_policy_offset=0x8000000000009EA8ULL; // sys 409 get_fan_policy  4.21/4.30/4.31/4.40/4.41/4.46/4.50/4.53
			set_fan_policy_offset=0x800000000000A3A4ULL; // sys 389 set_fan_policy
	}
	else if(c_firmware == 3.55f)
	{
			get_fan_policy_offset=0x8000000000008D3CULL; // sys 409 get_fan_policy
			set_fan_policy_offset=0x8000000000009238ULL; // sys 389 set_fan_policy
	}
}


#ifndef COBRA_ONLY
static void add_to_map(char *path1, char *path2)
{
	if(max_mapped==0) pokeq(MAP_BASE + 0x00, 0x0000000000000000ULL);

	if(max_mapped<10)
	{
		for(u8 n=0; n<max_mapped; n++)
		{
			if(!strcmp(file_to_map[n].src, path1)) goto leave;
		}
		sprintf(file_to_map[max_mapped].src, "%s", path1);
		sprintf(file_to_map[max_mapped].dst, "%s", path2);
		max_mapped++;
	}
leave:
	return;
}

static void string_to_lv2(char* path, u64 addr)
{
	u16 len=(strlen(path)+8)&0x7f8;
	len=RANGE(len, 8, 384);
	u16 len2=strlen(path); if(len2>len) len2=len;

	u8 data2[384];
	u8* data = data2;
	memset(data, 0, 384);
	memcpy(data, path, len2);

	u64 val=0x0000000000000000ULL;
	for(u64 n = 0; n < len; n += 8)
	{
		memcpy(&val, &data[n], 8);
		pokeq(addr+n, val);
	}
}
#endif

static void game_mount(char *buffer, char *templn, char *param, char *tempstr, u8 is_binary, bool mount_ps3, bool forced_mount)
{
	//unmount game
	if(strstr(param, "ps3/unmount"))
	{
		do_umount(true);

		strcat(buffer, STR_GAMEUM);
	}
#ifdef PS2_DISC
	else if(strstr(param, "ps2/unmount"))
	{
		do_umount_ps2disc(false);

		strcat(buffer, STR_GAMEUM);
	}
#endif
	else
	{
		if(strstr(param, "?random="))
			param[strrchr(param, '?')-param]=0;

		int plen=10;
#ifdef COPY_PS3
		if(strstr(param, "copy.ps3")) plen=IS_COPY;
		char target[MAX_PATH_LEN];
#endif
		char enc_dir_name[1024];
		bool mounted=false; max_mapped=0;
		is_binary=1;

		if(!(plen==IS_COPY && !copy_in_progress))
		{
			for(int n=0; n<(int)(strlen(param)-9); n++)
				if(memcmp(param + n, "/PS3_GAME", 9)==0) {param[n]=0; break;}
#ifdef PS2_DISC
			if(!memcmp(param, "/mount.ps2", 10))
			{
				mounted=mount_ps2disc(param+plen);
			}
			else
			if(!memcmp(param, "/mount_ps2", 10))
			{
				do_umount(true);
				mounted=mount_ps2disc(param+plen);
			}
			else
#endif
			if(!forced_mount && get_game_info())
			{
				sprintf(templn, "<H3>%s : <a href=\"/mount.ps3/unmount\">%s %s</a></H3><hr><a href=\"/mount_ps3%s\">", STR_UNMOUNTGAME, _game_TitleID, _game_Title, param+plen); strcat(buffer, templn);
			}
			else
				mounted=mount_with_mm(param+plen, 1);
		}

		if(mount_ps3)
		{
			if(mounted && View_Find("game_plugin")==0 && strstr(param, "/PSPISO")==NULL && extcmp(param, ".BIN.ENC", 8)!=0)
			{
				CellPadData pad_data = pad_read();
				bool atag = (strcasestr(param, AUTOPLAY_TAG)!=NULL) || (webman_config->autoplay);
#ifdef REMOVE_SYSCALLS
				bool l2 = (pad_data.len>0 && (pad_data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_L2));
#else
				bool l2 = (pad_data.len>0 && (pad_data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & (CELL_PAD_CTRL_L2 | CELL_PAD_CTRL_R2)));
#endif
				char category[16] = "game", seg_name[80] = "seg_device";
				if((atag && !l2) || (!atag && l2)) {sys_timer_sleep(1); launch_disc(category, seg_name);} // L2 + X
			}

			is_busy=false;
			return;
		}
		else
		{
			htmlenc(templn, param+plen, 0);
			sprintf(tempstr, "%s/PS3_GAME/ICON0.PNG", param+plen);

			if(FileExists(tempstr)==false)
			{
				char fpath[MAX_PATH_LEN], fname[MAX_PATH_LEN], tempID[10]; tempstr[0]=0;

				// get iso name
				strcpy(fname, strrchr(param+plen, '/')+1);
				strcpy(fpath, param+plen); fpath[strlen(fpath)-strlen(fname)-1]=0;

				get_default_icon(tempstr, fpath, fname, 0, tempID, -1, 0);
			}

			urlenc(enc_dir_name, tempstr);

#ifdef COPY_PS3
			if(plen==IS_COPY)
			{
				bool is_copying_from_hdd = (strstr(param+plen, "/dev_hdd0")!=NULL);
#ifdef SWAP_KERNEL
				if(strstr(param+plen, "/lv2_kernel"))
				{
					struct CellFsStat buf;
					if(cellFsStat(param+plen, &buf)!=CELL_FS_SUCCEEDED)
						sprintf(target, STR_ERROR);
					else
					{
						uint64_t size = buf.st_size;

						enable_dev_blind(param+plen);

						// for	cobra req: /dev_flash/sys/stage2.bin & /dev_flash/sys/lv2_self
						sprintf(tempstr, "%s", param+plen);
						strcpy(strrchr(tempstr, '/'), "/stage2.bin");

						sprintf(target, "/dev_blind/sys/stage2.bin");
						if(FileExists(target)==false)
							filecopy(tempstr, target, COPY_WHOLE_FILE);

						// copy: /dev_flash/sys/lv2_self
						sprintf(target, "/dev_blind/sys/lv2_self");
						if(cellFsStat(target, &buf)!=CELL_FS_SUCCEEDED || buf.st_size != size)
							filecopy(param+plen, target, COPY_WHOLE_FILE);

						if(FileExists(target))
						{
							u64 lv2_offset=0x15DE78; // 4.xx CFW LV1 memory location for: /flh/os/lv2_kernel.self
							/*
							if(peek_lv1(lv2_offset)!=0x2F666C682F6F732FULL)
								for(u64	addr=0;	addr<0xFFFFF8ULL; addr+=4)         // Find in 16MB
									if(peek_lv1(addr)	== 0x2F6F732F6C76325FULL)  // /os/lv2_
									{
										lv2_offset=addr-4; break; // 0x12A2C0 on 3.55
									}
							*/
							if(peek_lv1(lv2_offset)==0x2F666C682F6F732FULL)  // Original: /flh/os/lv2_kernel.self
							{
								poke_lv1(lv2_offset + 0x00, 0x2F6C6F63616C5F73ULL); // replace:	/flh/os/lv2_kernel.self -> /local_sys0/sys/lv2_self
								poke_lv1(lv2_offset + 0x08, 0x7973302F7379732FULL);
								poke_lv1(lv2_offset + 0x10, 0x6C76325F73656C66ULL);

								working = 0;
								{ DELETE_TURNOFF }
								savefile((char*)WMNOSCAN, NULL, 0);
								{system_call_3(SC_SYS_POWER, SYS_REBOOT, NULL, 0);}
								sys_ppu_thread_exit(0);
							}
						}
					}
					plen=0; //do not copy
				}
				else
#endif // #ifdef SWAP_KERNEL
				if(!extcmp(param+plen, ".p3t", 4))
				{
					if(is_copying_from_hdd)
						sprintf(target, "/dev_usb000/PS3/THEME");
					else
						sprintf(target, "/dev_hdd0/theme");

					strcat(target, strrchr(param, '/'));
				}
				else if(!extcmp(param+plen, ".pkg", 4))
				{
					if(is_copying_from_hdd)
						sprintf(target, "/dev_usb000/Packages");
					else
						sprintf(target, "/dev_hdd0/packages");

					strcat(target, strrchr(param, '/'));
				}
				else if(!extcmp(param+plen, ".edat", 5))
				{
					if(is_copying_from_hdd)
						sprintf(target, "/dev_usb000/exdata");
					else
						sprintf(target, "%s/%s/exdata", "/dev_hdd0/home", webman_config->uaccount);

					strcat(target, strrchr(param, '/'));
				}
				else if(strstr(param+plen, "/exdata"))
				{
					if(is_copying_from_hdd)
						sprintf(target, "/dev_usb000/exdata");
					else
						sprintf(target, "%s/%s/exdata", "/dev_hdd0/home", webman_config->uaccount);
				}
				else if(strstr(param+plen, "/PS3/THEME"))
					sprintf(target, "/dev_hdd0/theme");
				else if(strcasestr(param+plen, "/savedata/"))
				{
					if(is_copying_from_hdd)
						sprintf(target, "/dev_usb000/PS3/SAVEDATA");
					else
						sprintf(target, "%s/%s/savedata", "/dev_hdd0/home", webman_config->uaccount);

					strcat(target, strrchr(param, '/'));
				}
				else if(strcasestr(param+plen, "/trophy/"))
				{
					if(is_copying_from_hdd)
						sprintf(target, "/dev_usb000/PS3/TROPHY");
					else
						sprintf(target, "%s/%s/trophy", "/dev_hdd0/home", webman_config->uaccount);

					strcat(target, strrchr(param, '/'));
				}
				else if(strstr(param+plen, "/webftp_server"))
				{
					sprintf(target, "/dev_hdd0/plugins/webftp_server.sprx");
					if(FileExists(target)==false) sprintf(target, "/dev_hdd0/webftp_server.sprx");
				}
				else if(strstr(param+plen, "/boot_plugins_"))
					sprintf(target, "/dev_hdd0/boot_plugins.txt");
				else if(strstr(param+plen, "/dev_usb"))
					sprintf(target, "/dev_hdd0%s", param+plen+11);
				else if(is_copying_from_hdd)
					sprintf(target, "/dev_usb000%s", param+plen+9);
				else
				{
					sprintf(target, "%s/%s", "/dev_hdd0/GAMES", "My Disc Backup");
					if(strstr(param, "/dev_bdvd"))
					{
						char titleid[10], title[60];
						unsigned char *mem = (u8*)tempstr;

						int fdw;
						if(cellFsOpen((char*)"/dev_bdvd/PS3_GAME/PARAM.SFO", CELL_FS_O_RDONLY, &fdw, NULL, 0)==CELL_FS_SUCCEEDED)
						{   memset(mem, 0, _4KB_); uint64_t msiz = 0;
							cellFsLseek(fdw, 0, CELL_FS_SEEK_SET, &msiz);
							cellFsRead(fdw, (void *)mem, _4KB_, &msiz);
							cellFsClose(fdw);

							parse_param_sfo(mem, titleid, title);
							if(titleid[0] && titleid[8]>='0')
							{
								if(strstr(title, " ["))
									sprintf(target, "%s/%s", "/dev_hdd0/GAMES", title);
								else
									sprintf(target, "%s/%s [%s]", "/dev_hdd0/GAMES", title, titleid);
							}
						}
					}
				}

				sprintf(tempstr, "%s <a href=\"%s\">%s</a><hr>"
								 "<a href=\"%s\"><img src=\"%s\" border=0></a><hr>"
								 "%s: <a href=\"%s\">%s</a>",
								 STR_COPYING, param+plen, templn,
								 target, enc_dir_name,
								 STR_CPYDEST, target, target);

				if(strstr(target, "/webftp_server")) {strcat(buffer, tempstr); sprintf(tempstr, "<HR>%s", STR_SETTINGSUPD);}
			}
			else
#endif // #ifdef COPY_PS3
			if(!extcmp(param, ".BIN.ENC", 8))
				sprintf(tempstr, "%s: %s<hr><img src=\"%s\"><hr>%s", STR_GAMETOM, templn, enc_dir_name, mounted?STR_PS2LOADED:STR_ERROR);
			else if(strstr(param, "/PSPISO") || strstr(param, "/ISO/"))
				sprintf(tempstr, "%s: %s<hr><img src=\"%s\" height=%i><hr>%s", STR_GAMETOM, templn, enc_dir_name, strcasestr(enc_dir_name,".png")?200:300, mounted?STR_PSPLOADED:STR_ERROR);
			else if(strstr(param, "/BDISO") || strstr(param, "/DVDISO") || !extcmp(param, ".ntfs[BDISO]", 12) || !extcmp(param, ".ntfs[DVDISO]", 13))
				sprintf(tempstr, "%s: <a href=\"%s\">%s</a><hr><a href=\"/play.ps3\"><img src=\"%s\" border=0></a><hr><a href=\"/dev_bdvd\">%s</a>", STR_MOVIETOM, templn, templn, enc_dir_name, mounted?STR_MOVIELOADED:STR_ERROR);
			else
				sprintf(tempstr, "%s: <a href=\"%s\">%s</a><hr><a href=\"/play.ps3\"><img src=\"%s\" border=0></a><hr><a href=\"/dev_bdvd\">%s</a>", STR_GAMETOM, templn, templn, enc_dir_name, mounted?STR_GAMELOADED:STR_ERROR);

			strcat(buffer, tempstr);

#ifdef PS2_DISC
			if(mounted && (strstr(param+plen, "/GAME") || strstr(param+plen, "/PS3ISO") || strstr(param+plen, ".ntfs[PS3ISO]")))
			{
				CellFsDirent entry; u64 read_e; int fd2; u16 pcount=0; u32 tlen=strlen(buffer)+8;

				sprintf(target, "%s", param+plen); u8 is_iso=0;
				if(strstr(target, "Sing"))
				{
					if(strstr(target, "/PS3ISO")) {strcpy(strstr(target, "/PS3ISO"), "/PS2DISC\0"); is_iso=1;}
					if(strstr(target, ".ntfs[PS3ISO]")) {strcpy(target, "/dev_hdd0/PS2DISC\0"); is_iso=1;}
				}

				// check for [PS2] extracted folders
				if(cellFsOpendir(target, &fd2) == CELL_FS_SUCCEEDED)
				{
					while((cellFsReaddir(fd2, &entry, &read_e) == 0 && read_e > 0))
					{
						if((entry.d_name[0]=='.')) continue;

						if(is_iso || strstr(entry.d_name, "[PS2")!=NULL)
						{
							if(pcount==0) strcat(buffer, "<br><HR>");
							urlenc(enc_dir_name, entry.d_name);
							sprintf(templn, "<a href=\"/mount.ps2%s/%s\">%s</a><br>", target, enc_dir_name, entry.d_name);

							tlen+=strlen(tempstr);
							if(tlen>(BUFFER_SIZE-1024)) break;
							strcat(buffer, templn); pcount++;
						}
					}
				}
			}
#endif // #ifdef PS2_DISC
		}
#ifdef COPY_PS3
		if(plen==IS_COPY && !copy_in_progress)
		{
			if(FileExists(param+plen)==false)
				sprintf(templn, "%s", STR_ERROR);
			else
			{
				// show msg begin
				sprintf(templn, "%s %s\n%s %s", STR_COPYING, param+plen, STR_CPYDEST, target);
				show_msg((char*)templn);
				copy_in_progress=true; copied_count = 0;

				// make target dir tree
				for(u16 p=12; p<strlen(target); p++)
					if(target[p]=='/') {target[p]=0; cellFsMkdir((char*)target, MODE); target[p]='/';}

				// copy folder to target
				if(strstr(param+plen,"/exdata"))
					import_edats(param+plen, target);
				else if(isDir(param+plen))
					folder_copy(param+plen, target);
				else
					filecopy(param+plen, target, COPY_WHOLE_FILE);

				copy_in_progress=false;

				// show msg end
				if(copy_aborted)
					show_msg((char*)STR_CPYABORT);
				else
					show_msg((char*)STR_CPYFINISH);
			}
		}
#endif //#ifdef COPY_PS3
	}
}

#ifdef COBRA_ONLY
static void do_umount_iso(void)
{
	unsigned int real_disctype, effective_disctype, iso_disctype;

	cobra_get_disc_type(&real_disctype, &effective_disctype, &iso_disctype);

	// If there is an effective disc in the system, it must be ejected
	if(effective_disctype != DISC_TYPE_NONE)
	{
		cobra_send_fake_disc_eject_event();
		for(u8 m=0; m<25; m++)
		{
			sys_timer_usleep(4000);

			if(!isDir("/dev_bdvd")) break;
		}
	}

	if(iso_disctype != DISC_TYPE_NONE) cobra_umount_disc_image();

	// If there is a real disc in the system, issue an insert event
	if(real_disctype != DISC_TYPE_NONE)
	{
		cobra_send_fake_disc_insert_event();
		for(u8 m=0; m<25; m++)
		{
			sys_timer_usleep(4000);

			if(isDir("/dev_bdvd")) break;
		}
		cobra_disc_auth();
	}
}
#endif

static void do_umount(bool clean)
{
	if(clean) cellFsUnlink((char*)WMTMP "/last_game.txt");

	if(fan_ps2_mode) reset_fan_mode();

#ifdef COBRA_ONLY
	//if(cobra_mode)
	{
		do_umount_iso();
 #ifdef PS2_DISC
		do_umount_ps2disc(false);
 #endif
		sys_timer_usleep(20000);

		cobra_unset_psp_umd();

		{sys_map_path((char*)"/dev_bdvd", NULL);}
		{sys_map_path((char*)"/app_home", !isDir("/dev_hdd0/packages")?NULL:(char*)"/dev_hdd0/packages");}

		{sys_map_path((char*)"//dev_bdvd", NULL);}
		//{sys_map_path((char*)"//app_home", NULL);}

		{
			sys_ppu_thread_t t;
			uint64_t exit_code;
 #ifndef LITE_EDITION
			sys_ppu_thread_create(&t, netiso_stop_thread, 0, 0, 0x2000, SYS_PPU_THREAD_CREATE_JOINABLE, STOP_THREAD_NAME);
			sys_ppu_thread_join(t, &exit_code);
 #endif
			sys_ppu_thread_create(&t, rawseciso_stop_thread, 0, 0, 0x2000, SYS_PPU_THREAD_CREATE_JOINABLE, STOP_THREAD_NAME);
			sys_ppu_thread_join(t, &exit_code);
		}

#ifndef LITE_EDITION
		while(netiso_loaded || rawseciso_loaded) {sys_timer_usleep(100000);}
#else
		while(rawseciso_loaded) {sys_timer_usleep(100000);}
#endif

	}
#else
	{
		pokeq(0x8000000000000000ULL+MAP_ADDR, 0x0000000000000000ULL);
		pokeq(0x8000000000000008ULL+MAP_ADDR, 0x0000000000000000ULL);
		//eject_insert(1, 1);

		if(isDir("/dev_flash/pkg"))
			mount_with_mm((char*)"/dev_flash/pkg", 0);
	}
#endif //#ifdef COBRA_ONLY
}

static void mount_autoboot(void)
{
	char path[MAX_PATH_LEN];

	// get autoboot path
	if(webman_config->autob &&
      ((cobra_mode && strstr(webman_config->autoboot_path, "/net")!=NULL) || FileExists(webman_config->autoboot_path))) // autoboot
		strcpy(path, (char *) webman_config->autoboot_path);
	else
	{   // get last game path
		sprintf(path, WMTMP "/last_game.txt");
		if(webman_config->lastp && FileExists(path))
		{
			int fd=0;
			if(cellFsOpen(path, CELL_FS_O_RDONLY, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
			{
				u64 read_e = 0;
				if(cellFsRead(fd, (void *)path, MAX_PATH_LEN, &read_e) == CELL_FS_SUCCEEDED) path[read_e]=0;
				cellFsClose(fd);
			}
			else
				path[0]=0;
		}
		else
			path[0]=0;
	}

	bool do_mount=false;

	if(from_reboot && !path[0] && strstr(path, "/PS2")) return; //avoid re-launch PS2 returning to XMB

    // wait few seconds until path becomes ready
	if(strlen(path)>10 || (cobra_mode && strstr(path, "/net")!=NULL))
	{
		waitfor((char*)path, 2*(webman_config->boots+webman_config->bootd));
		do_mount=((cobra_mode && strstr(path, "/net")!=NULL) || FileExists(path));
	}

	if(do_mount)
	{   // add some delay
		if(webman_config->delay)      {sys_timer_sleep(10); waitfor((char*)path, 2*(webman_config->boots+webman_config->bootd));}
		else if(strstr(path, "/net"))  sys_timer_sleep(10);
#ifndef COBRA_ONLY
		if(strstr(path, "/net")==NULL && strstr(path, ".ntfs[")==NULL)
#endif
		mount_with_mm(path, 1); // mount path
	}
}

static bool mount_with_mm(const char *_path0, u8 do_eject)
{
	if(is_mounting) return false;

	bool ret=true;

	if(fan_ps2_mode) reset_fan_mode();

	is_mounting=true;

#ifndef COBRA_ONLY
	u64 sc_600=0;
	u64 sc_604=0;
	u64 sc_142=0;
#endif

	if(!dex_mode)
	{
		if(c_firmware==4.21f)
		{
			pokeq(0x8000000000296264ULL, 0x4E80002038600000ULL );
			pokeq(0x800000000029626CULL, 0x7C6307B44E800020ULL ); // fix 8001003C error
			pokeq(0x8000000000057020ULL, 0x63FF003D60000000ULL ); // fix 8001003D error
			pokeq(0x80000000000570E4ULL, 0x3FE080013BE00000ULL ); // fix 8001003E error

			pokeq(0x8000000000057090ULL, 0x419E00D860000000ULL );
			pokeq(0x8000000000057098ULL, 0x2F84000448000098ULL );
			pokeq(0x800000000005AA54ULL, 0x2F83000060000000ULL ); // fix 80010009 error
			pokeq(0x800000000005AA68ULL, 0x2F83000060000000ULL ); // fix 80010019 error

#ifndef COBRA_ONLY
			sc_600=0x33B2E0;
			sc_604=0x33B448;
			sc_142=0x2FD810;
#endif
		}
        else
		if(c_firmware==4.30f)
		{
			pokeq(0x80000000002979D8ULL, 0x4E80002038600000ULL );
			pokeq(0x80000000002979E0ULL, 0x7C6307B44E800020ULL ); // fix 8001003C error
			pokeq(0x8000000000057170ULL, 0x63FF003D60000000ULL ); // fix 8001003D error
			pokeq(0x8000000000057234ULL, 0x3FE080013BE00000ULL ); // fix 8001003E error
			pokeq(0x8000000000057238ULL, 0x3BE000004BFFFF0CULL ); // introduced by me bug

			pokeq(0x80000000000571E0ULL, 0x419E00D860000000ULL );
			pokeq(0x80000000000571E8ULL, 0x2F84000448000098ULL );
			pokeq(0x800000000005ABA4ULL, 0x2F83000060000000ULL );
			pokeq(0x800000000005ABB8ULL, 0x2F83000060000000ULL );

#ifndef COBRA_ONLY
			sc_600=0x33D158; //35EEA0
			sc_604=0x33D2C0; //35EEC0
			sc_142=0x2FF460; //35E050
#endif
		}
        else
		if(c_firmware==4.31f)
		{
			pokeq(0x80000000002979E0ULL, 0x4E80002038600000ULL );
			pokeq(0x80000000002979E8ULL, 0x7C6307B44E800020ULL ); // fix 8001003C error
			pokeq(0x8000000000057174ULL, 0x63FF003D60000000ULL ); // fix 8001003D error
			pokeq(0x800000000005723CULL, 0x3FE080013BE00000ULL ); // fix 8001003E error

			pokeq(0x80000000000571E8ULL, 0x600000002F840004ULL );
			pokeq(0x80000000000571F0ULL, 0x48000098E8629870ULL );
			pokeq(0x800000000005ABACULL, 0x60000000E8610188ULL );
			pokeq(0x800000000005ABA0ULL, 0x600000005463063EULL );

#ifndef COBRA_ONLY
			sc_600=0x33D168;
			sc_604=0x33D2D0;
			sc_142=0x2FF470;
#endif
		}
        else
		if(c_firmware==4.40f)
		{
			pokeq(0x8000000000296DE8ULL, 0x4E80002038600000ULL );
			pokeq(0x8000000000296DF0ULL, 0x7C6307B44E800020ULL ); // fix 8001003C error
			pokeq(0x80000000000560BCULL, 0x63FF003D60000000ULL ); // fix 8001003D error
			pokeq(0x8000000000056180ULL, 0x3FE080013BE00000ULL ); // fix 8001003E error +C4

			pokeq(0x800000000005612CULL, 0x419E00D860000000ULL );
			pokeq(0x8000000000056134ULL, 0x2F84000448000098ULL );
			pokeq(0x8000000000059AF0ULL, 0x2F83000060000000ULL );
			pokeq(0x8000000000059B04ULL, 0x2F83000060000000ULL );

#ifndef COBRA_ONLY
			sc_600=0x33D720;
			sc_604=0x33D888;
			sc_142=0x2FF9E0;
#endif
		}
        else
		if(c_firmware==4.41f)
		{
			pokeq(0x8000000000296DF0ULL, 0x4E80002038600000ULL ); // fix 8001003C error
			pokeq(0x8000000000296DF8ULL, 0x7C6307B44E800020ULL ); // fix 8001003C error
			pokeq(0x80000000000560C0ULL, 0x63FF003D60000000ULL ); // fix 8001003D error
			pokeq(0x8000000000056184ULL, 0x3FE080013BE00000ULL ); // fix 8001003E error

			pokeq(0x8000000000056130ULL, 0x419E00D860000000ULL );
			pokeq(0x8000000000056138ULL, 0x2F84000448000098ULL );
			pokeq(0x8000000000059AF4ULL, 0x2F83000060000000ULL );
			pokeq(0x8000000000059B08ULL, 0x2F83000060000000ULL );

#ifndef COBRA_ONLY
			sc_600=0x33D730;
			sc_604=0x33D898;
			sc_142=0x2FF9F0;
#endif
		}
        else
		if(c_firmware==4.46f)
		{
			pokeq(0x8000000000297310ULL, 0x4E80002038600000ULL ); // fix 8001003C error
			pokeq(0x8000000000297318ULL, 0x7C6307B44E800020ULL ); // fix 8001003C error
			pokeq(0x80000000000560C0ULL, 0x63FF003D60000000ULL ); // fix 8001003D error
			pokeq(0x8000000000056184ULL, 0x3FE080013BE00000ULL ); // fix 8001003E error

			pokeq(0x8000000000056130ULL, 0x419E00D860000000ULL );
			pokeq(0x8000000000056138ULL, 0x2F84000448000098ULL );
			pokeq(0x8000000000059AF4ULL, 0x2F83000060000000ULL );
			pokeq(0x8000000000059B08ULL, 0x2F83000060000000ULL );

#ifndef COBRA_ONLY
			sc_600=0x33DD40;
			sc_604=0x33DEA8;
			sc_142=0x2FFF58;
#endif
		}
        else
		if(c_firmware==4.50f)
		{
			pokeq(0x800000000026F61CULL, 0x4E80002038600000ULL ); // fix 8001003C error
			pokeq(0x800000000026F624ULL, 0x7C6307B44E800020ULL ); // fix 8001003C error
			pokeq(0x80000000000560BCULL, 0x63FF003D60000000ULL ); // fix 8001003D error
			pokeq(0x8000000000056180ULL, 0x3FE080013BE00000ULL ); // fix 8001003E error

			pokeq(0x800000000005612CULL, 0x419E00D860000000ULL );
			pokeq(0x8000000000056134ULL, 0x2F84000448000098ULL );
			pokeq(0x8000000000059AF0ULL, 0x2F83000060000000ULL );
			pokeq(0x8000000000059B04ULL, 0x2F83000060000000ULL );

#ifndef COBRA_ONLY
			sc_600=0x33C180;
			sc_604=0x33C2E8;
			sc_142=0x302100;
#endif
		}
        else
		if(c_firmware==4.53f)
		{
			pokeq(0x800000000026F7F0ULL, 0x4E80002038600000ULL ); // fix 8001003C error
			pokeq(0x800000000026F7F8ULL, 0x7C6307B44E800020ULL ); // fix 8001003C error
			pokeq(0x80000000000560C0ULL, 0x63FF003D60000000ULL ); // fix 8001003D error
			pokeq(0x8000000000056184ULL, 0x3FE080013BE00000ULL ); // fix 8001003E error

			pokeq(0x8000000000056130ULL, 0x419E00D860000000ULL );
			pokeq(0x8000000000056138ULL, 0x2F84000448000098ULL );
			pokeq(0x8000000000059AF4ULL, 0x2F83000060000000ULL );
			pokeq(0x8000000000059B08ULL, 0x2F83000060000000ULL );

#ifndef COBRA_ONLY
			sc_600=0x33C308;
			sc_604=0x33C470;
			sc_142=0x302108;
#endif
		}
        else
		if(c_firmware==4.55f)
		{
			pokeq(0x800000000027103CULL, 0x4E80002038600000ULL ); // fix 8001003C error
			pokeq(0x8000000000271044ULL, 0x7C6307B44E800020ULL ); // fix 8001003C error
			pokeq(0x8000000000056380ULL, 0x63FF003D60000000ULL ); // fix 8001003D error
			pokeq(0x8000000000056444ULL, 0x3FE080013BE00000ULL ); // fix 8001003E error

			pokeq(0x80000000000563F0ULL, 0x419E00D860000000ULL );
			pokeq(0x80000000000563F8ULL, 0x2F84000448000098ULL );
			pokeq(0x800000000005A2ECULL, 0x2F83000060000000ULL );
			pokeq(0x800000000005A300ULL, 0x2F83000060000000ULL );

#ifndef COBRA_ONLY
			sc_600=0x33F5C8;
			sc_604=0x33F730;
			sc_142=0x3051D0;
#endif
		}
        else
		if(c_firmware==4.60f)
		{
			pokeq(0x80000000002925D8ULL, 0x4E80002038600000ULL ); // fix 8001003C error
			pokeq(0x80000000002925E0ULL, 0x7C6307B44E800020ULL ); // fix 8001003C error
			pokeq(0x8000000000056588ULL, 0x63FF003D60000000ULL ); // fix 8001003D error
			pokeq(0x800000000005664CULL, 0x3FE080013BE00000ULL ); // fix 8001003E error

			pokeq(0x80000000000565F8ULL, 0x419E00D860000000ULL );
			pokeq(0x8000000000056600ULL, 0x2F84000448000098ULL );
			pokeq(0x800000000005A654ULL, 0x2F83000060000000ULL );
			pokeq(0x800000000005A668ULL, 0x2F83000060000000ULL );

			pokeq(0x80000000002A1054ULL, 0x386000014E800020ULL); // fix 0x80010017 error   Original: 0xFBC1FFF0EBC225B0ULL
			pokeq(0x8000000000055C58ULL, 0x386000004E800020ULL); // fix 0x8001002B error   Original: 0xF821FE917C0802A6ULL

			//lv2poke32(0x8000000000058DACULL, 0x60000000);      // fix 0x80010017 error (found by @smhabib)

			// Booting of game discs and backups speed increased
			//lv2poke32(0x8000000000058DA0ULL, 0x38600001);
			//lv2poke32(0x800000000005A96CULL, 0x38600000);

			// enable new habib patches
			pokeq(0x8000000000058DACULL + 0x00, 0x60000000E8610098ULL);
			pokeq(0x8000000000058DACULL + 0x08, 0x2FA30000419E000CULL);
			pokeq(0x8000000000058DACULL + 0x10, 0x388000334800BE15ULL);
			pokeq(0x8000000000058DACULL + 0x18, 0xE80100F07FE307B4ULL);

			pokeq(0x8000000000055C5CULL + 0x00, 0x386000004E800020ULL);
			pokeq(0x8000000000055C5CULL + 0x08, 0xFBC10160FBE10168ULL);
			pokeq(0x8000000000055C5CULL + 0x10, 0xFB610148FB810150ULL);
			pokeq(0x8000000000055C5CULL + 0x18, 0xFBA10158F8010180ULL);

#ifndef COBRA_ONLY
			sc_600=0x340630; //0x363A18 + 600*8 = 00364CD8 -> 80 00 00 00 00 34 06 30
			sc_604=0x340798; //0x363A18 + 604*8 = 00364CF8 -> 80 00 00 00 00 34 07 98
			sc_142=0x306478; //0x363A18 + 142*8 = 00363E88 -> 80 00 00 00 00 30 64 78
#endif
		}
        else
		if(c_firmware==4.65f || c_firmware==4.66f)
		{
			//patches by deank
			pokeq(0x800000000026FDDCULL, 0x4E80002038600000ULL ); // fix 8001003C error  Original: 0x4E8000208003026CULL
			pokeq(0x800000000026FDE4ULL, 0x7C6307B44E800020ULL ); // fix 8001003C error  Original: 0x3D6000463D201B43ULL
			pokeq(0x800000000005658CULL, 0x63FF003D60000000ULL ); // fix 8001003D error  Original: 0x63FF003D419EFFD4ULL
			pokeq(0x8000000000056650ULL, 0x3FE080013BE00000ULL ); // fix 8001003E error  Original: 0x3FE0800163FF003EULL

			pokeq(0x80000000000565FCULL, 0x419E00D860000000ULL ); // Original: 0x419E00D8419D00C0ULL
			pokeq(0x8000000000056604ULL, 0x2F84000448000098ULL ); // Original: 0x2F840004409C0048ULL //PATCH_JUMP
			pokeq(0x800000000005A658ULL, 0x2F83000060000000ULL ); // fix 80010009 error  Original: 0x2F830000419E00ACULL
			pokeq(0x800000000005A66CULL, 0x2F83000060000000ULL ); // fix 80010009 error  Original: 0x2F830000419E00ACULL

			pokeq(0x8000000000056230ULL, 0x386000012F830000ULL ); // ignore LIC.DAT check
			pokeq(0x80000000002302F0ULL, 0x38600000F8690000ULL ); // fix 0x8001002B / 80010017 errors (2015-01-03)

			pokeq(0x8000000000055C5CULL, 0xF821FE917C0802A6ULL ); // just restore the original
			pokeq(0x8000000000058DB0ULL, 0x419E0038E8610098ULL ); // just restore the original

		 /*
			//anti-ode patches by deank
			if(!is_rebug)
            {
				//pokeq(0x8000000000055C5CULL, 0xF821FE917C0802A6ULL ); //replaced by deank's patch (2015-01-03)
				pokeq(0x8000000000055C84ULL, 0x6000000060000000ULL );
				pokeq(0x8000000000055C8CULL, 0x600000003BA00000ULL );
            }

			//pokeq(0x80000000002A1060ULL, 0x386000014E800020ULL); // fix 0x80010017 error   Original: 0xFBC1FFF0EBC225B0ULL
			//pokeq(0x8000000000055C5CULL, 0x386000004E800020ULL); // fix 0x8001002B error   Original: 0xF821FE917C0802A6ULL

			// Booting of game discs and backups speed increased
			//lv2poke32(0x8000000000058DA4ULL, 0x38600001);
			//lv2poke32(0x800000000005A970ULL, 0x38600000);

			// habib patches (newest ones)
			pokeq(0x8000000000055C98ULL, 0x38600000EB610148ULL); //Original: 0x7FA307B4EB610148EB8101507C0803A6
			pokeq(0x8000000000058DCCULL, 0x38600000EBA100C8ULL); //Original: 0x7FE307B4EBA100C8EBC100D07C0803A6

			// enable new habib patches (now obsolete) //replaced by deank's patch (2015-01-03)
			pokeq(0x8000000000058DB0ULL + 0x00, 0x60000000E8610098ULL);
			pokeq(0x8000000000058DB0ULL + 0x08, 0x2FA30000419E000CULL);
			pokeq(0x8000000000058DB0ULL + 0x10, 0x388000334800BE15ULL);
			pokeq(0x8000000000058DB0ULL + 0x18, 0xE80100F07FE307B4ULL);

			pokeq(0x8000000000055C5CULL + 0x00, 0x386000004E800020ULL);
			pokeq(0x8000000000055C5CULL + 0x08, 0xFBC10160FBE10168ULL);
			pokeq(0x8000000000055C5CULL + 0x10, 0xFB610148FB810150ULL);
			pokeq(0x8000000000055C5CULL + 0x18, 0xFBA10158F8010180ULL);

			//patch to prevent blackscreen on usb games in jb format
			pokeq(0x8000000000055C84ULL, 0x386000002F830001ULL); //Original: 0x481DA6692F830001ULL
			pokeq(0x8000000000055C8CULL, 0x419E00303BA00000ULL); //Original: 0x419E00303BA00000ULL
		 */

#ifndef COBRA_ONLY
			sc_600=0x340640; //0x363A18 + 600*8 = 00364CD8 -> 80 00 00 00 00 34 06 40
			sc_604=0x3407A8; //0x363A18 + 604*8 = 00364CF8 -> 80 00 00 00 00 34 07 A8
			sc_142=0x306488; //0x363A18 + 142*8 = 00363E88 -> 80 00 00 00 00 30 64 88
#endif
		}
		else
		if(c_firmware==4.70f)
		{
			//patches by deank
			pokeq(0x80000000002670D8ULL, 0x4E80002038600000ULL ); // fix 8001003C error  Original: 0x4E8000208003026CULL //0x80000000002898DCULL??
			pokeq(0x80000000002670E0ULL, 0x7C6307B44E800020ULL ); // fix 8001003C error  Original: 0x3D6000463D201B43ULL //0x80000000002898E4ULL??
			pokeq(0x8000000000056588ULL, 0x63FF003D60000000ULL ); // fix 8001003D error  Original: 0x63FF003D419EFFD4ULL
			pokeq(0x800000000005664CULL, 0x3FE080013BE00000ULL ); // fix 8001003E error  Original: 0x3FE0800163FF003EULL

			pokeq(0x80000000000565F8ULL, 0x419E00D860000000ULL ); // Original: 0x419E00D8419D00C0ULL
			pokeq(0x8000000000056600ULL, 0x2F84000448000098ULL ); // Original: 0x2F840004409C0048ULL //PATCH_JUMP
			pokeq(0x800000000005A6DCULL, 0x2F83000060000000ULL ); // fix 80010009 error  Original: 0x2F830000419E00ACULL
			pokeq(0x800000000005A6F0ULL, 0x2F83000060000000ULL ); // fix 80010009 error  Original: 0x2F830000419E00ACULL

			pokeq(0x800000000005622CULL, 0x386000012F830000ULL ); // ignore LIC.DAT check
			pokeq(0x80000000002275ECULL, 0x38600000F8690000ULL ); // fix 0x8001002B / 80010017 errors (2015-01-03)

			pokeq(0x8000000000055C58ULL, 0xF821FE917C0802A6ULL ); // just restore the original
			pokeq(0x8000000000058E18ULL, 0x419E0038E8610098ULL ); // just restore the original

#ifndef COBRA_ONLY
			sc_600=0x33FE88;
			sc_604=0x33FFF0;
			sc_142=0x306618;
#endif
		}
		else
		if(c_firmware==4.75f || c_firmware==4.76f)
		{
			//patches by deank
			pokeq(0x800000000026714CULL, 0x4E80002038600000ULL ); // fix 8001003C error  Original: 0x4E8000208003026CULL
			pokeq(0x8000000000267154ULL, 0x7C6307B44E800020ULL ); // fix 8001003C error  Original: 0x3D6000473D201B43ULL
			pokeq(0x800000000005658CULL, 0x63FF003D60000000ULL ); // fix 8001003D error  Original: 0x63FF003D419EFFD4ULL
			pokeq(0x8000000000056650ULL, 0x3FE080013BE00000ULL ); // fix 8001003E error  Original: 0x3FE0800163FF003EULL

			pokeq(0x80000000000565FCULL, 0x419E00D860000000ULL ); // Original: 0x419E00D8419D00C0ULL
			pokeq(0x8000000000056604ULL, 0x2F84000448000098ULL ); // Original: 0x2F840004409C0048ULL //PATCH_JUMP
			pokeq(0x800000000005A6E0ULL, 0x2F83000060000000ULL ); // fix 80010009 error  Original: 0x2F830000419E00ACULL
			pokeq(0x800000000005A6F4ULL, 0x2F83000060000000ULL ); // fix 80010009 error  Original: 0x2F830000419E00ACULL

			pokeq(0x8000000000056230ULL, 0x386000012F830000ULL ); // ignore LIC.DAT check
			pokeq(0x80000000002275F4ULL, 0x38600000F8690000ULL ); // fix 0x8001002B / 80010017 errors (2015-01-03)

			pokeq(0x8000000000055C5CULL, 0xF821FE917C0802A6ULL ); // just restore the original
			pokeq(0x8000000000058E1CULL, 0x419E0038E8610098ULL ); // just restore the original

#ifndef COBRA_ONLY
			sc_600=0x33FF28;
			sc_604=0x340090;
			sc_142=0x3066B8;
#endif
		}
	}
	else
	{ //DEX

		if(c_firmware==4.21f)
		{
			pokeq(0x800000000029C8C0ULL, 0x4E80002038600000ULL );
			pokeq(0x800000000029C8C8ULL, 0x7C6307B44E800020ULL ); // fix 8001003C error
			pokeq(0x800000000005A938ULL, 0x63FF003D60000000ULL ); // fix 8001003D error
			pokeq(0x800000000005A9FCULL, 0x3FE080013BE00000ULL ); // fix 8001003E error

			pokeq(0x800000000005A9A8ULL, 0x419E00D860000000ULL );
			pokeq(0x800000000005A9B0ULL, 0x2F84000448000098ULL );
			pokeq(0x800000000005E36CULL, 0x2F83000060000000ULL );
			pokeq(0x800000000005E380ULL, 0x2F83000060000000ULL );

#ifndef COBRA_ONLY
			sc_600=0x3583F8;
			sc_604=0x3584D0;
			sc_142=0x318BA0;
#endif
		}
		else
		if(c_firmware==4.30f)
		{
			pokeq(0x800000000029E034ULL, 0x4E80002038600000ULL );
			pokeq(0x800000000029E03CULL, 0x7C6307B44E800020ULL ); // fix 8001003C error
			pokeq(0x800000000005AA88ULL, 0x63FF003D60000000ULL ); // fix 8001003D error
			pokeq(0x800000000005AB4CULL, 0x3FE080013BE00000ULL ); // fix 8001003E error

			pokeq(0x800000000005AAF8ULL, 0x419E00D860000000ULL );
			pokeq(0x800000000005AB00ULL, 0x2F84000448000098ULL );
			pokeq(0x800000000005E4BCULL, 0x2F83000060000000ULL );
			pokeq(0x800000000005E4D0ULL, 0x2F83000060000000ULL );

#ifndef COBRA_ONLY
			sc_600=0x35A220;
			sc_604=0x35A2F8;
			sc_142=0x31A7A0;
#endif
		}
		else
		if(c_firmware==4.41f)
		{
			pokeq(0x800000000029D44CULL, 0x4E80002038600000ULL );
			pokeq(0x800000000029D454ULL, 0x7C6307B44E800020ULL ); // fix 8001003C error
			pokeq(0x80000000000599D8ULL, 0x63FF003D60000000ULL ); // fix 8001003D error
			pokeq(0x8000000000059A9CULL, 0x3FE080013BE00000ULL ); // fix 8001003E error

			pokeq(0x8000000000059A48ULL, 0x419E00D860000000ULL );
			pokeq(0x8000000000059A50ULL, 0x2F84000448000098ULL );
			pokeq(0x800000000005D40CULL, 0x2F83000060000000ULL );
			pokeq(0x800000000005D420ULL, 0x2F83000060000000ULL );

#ifndef COBRA_ONLY
			sc_600=0x35AB40;
			sc_604=0x35AC18;
			sc_142=0x31B060;
#endif
		}
		else
		if(c_firmware==4.46f)
		{
			pokeq(0x800000000029D96CULL, 0x4E80002038600000ULL ); // fix 8001003C error
			pokeq(0x800000000029D974ULL, 0x7C6307B44E800020ULL ); // fix 8001003C error
			pokeq(0x80000000000599D8ULL, 0x63FF003D60000000ULL ); // fix 8001003D error
			pokeq(0x8000000000059A9CULL, 0x3FE080013BE00000ULL ); // fix 8001003E error

			pokeq(0x8000000000059A48ULL, 0x419E00D860000000ULL );
			pokeq(0x8000000000059A50ULL, 0x2F84000448000098ULL );
			pokeq(0x800000000005D40CULL, 0x2F83000060000000ULL );
			pokeq(0x800000000005D420ULL, 0x2F83000060000000ULL );

#ifndef COBRA_ONLY
			sc_600=0x35B150;
			sc_604=0x35B228;
			sc_142=0x31B5C8;
#endif
		}
		else
		if(c_firmware==4.50f)
		{
			pokeq(0x8000000000275D38ULL, 0x4E80002038600000ULL ); // fix 8001003C error
			pokeq(0x8000000000275D40ULL, 0x7C6307B44E800020ULL ); // fix 8001003C error
			pokeq(0x8000000000059A8CULL, 0x63FF003D60000000ULL ); // fix 8001003D error
			pokeq(0x8000000000059B50ULL, 0x3FE080013BE00000ULL ); // fix 8001003E error

			pokeq(0x8000000000059AFCULL, 0x419E00D860000000ULL );
			pokeq(0x8000000000059B04ULL, 0x2F84000448000098ULL );
			pokeq(0x800000000005D4C0ULL, 0x2F83000060000000ULL );
			pokeq(0x800000000005D4D4ULL, 0x2F83000060000000ULL );

#ifndef COBRA_ONLY
			sc_600=0x35EA90;
			sc_604=0x35EB68;
			sc_142=0x322B38;
#endif
		}
		else
		if(c_firmware==4.53f)
		{
			pokeq(0x8000000000275F0CULL, 0x4E80002038600000ULL ); // fix 8001003C error
			pokeq(0x8000000000275F14ULL, 0x7C6307B44E800020ULL ); // fix 8001003C error
			pokeq(0x8000000000059A90ULL, 0x63FF003D60000000ULL ); // fix 8001003D error
			pokeq(0x8000000000059B54ULL, 0x3FE080013BE00000ULL ); // fix 8001003E error

			pokeq(0x8000000000059B00ULL, 0x419E00D860000000ULL );
			pokeq(0x8000000000059B08ULL, 0x2F84000448000098ULL );
			pokeq(0x800000000005D4C4ULL, 0x2F83000060000000ULL );
			pokeq(0x800000000005D4D8ULL, 0x2F83000060000000ULL );

#ifndef COBRA_ONLY
			sc_600=0x3602A8; //0x385108 + 600*8 = 003863C8 -> 80 00 00 00 00 36 02 A8
			sc_604=0x360380; //0x385108 + 604*8 = 003863E8 -> 80 00 00 00 00 36 03 80
			sc_142=0x3242F0; //0x385108 + 142*8 = 00385578 -> 80 00 00 00 00 32 42 F0
#endif
		}
		else
		if(c_firmware==4.55f)
		{
			pokeq(0x8000000000277758ULL, 0x4E80002038600000ULL ); // fix 8001003C error
			pokeq(0x8000000000277760ULL, 0x7C6307B44E800020ULL ); // fix 8001003C error
			pokeq(0x8000000000059D50ULL, 0x63FF003D60000000ULL ); // fix 8001003D error
			pokeq(0x8000000000059E14ULL, 0x3FE080013BE00000ULL ); // fix 8001003E error

			pokeq(0x8000000000059DC0ULL, 0x419E00D860000000ULL );
			pokeq(0x8000000000059DC8ULL, 0x2F84000448000098ULL );
			pokeq(0x800000000005DCB8ULL, 0x2F83000060000000ULL );
			pokeq(0x800000000005DCD0ULL, 0x2F83000060000000ULL );

#ifndef COBRA_ONLY
			sc_600=0x3634F8; //0x388488 + 600*8 = 00389748 -> 80 00 00 00 00 36 34 F8
			sc_604=0x3635D0; //0x388488 + 604*8 = 00389768 -> 80 00 00 00 00 36 35 D0
			sc_142=0x327348; //0x388488 + 142*8 = 003888F8 -> 80 00 00 00 00 32 73 48
#endif
		}
		else
		if(c_firmware==4.60f)
		{
			pokeq(0x80000000002764F0ULL, 0x4E80002038600000ULL ); // fix 8001003C error
			pokeq(0x80000000002764F8ULL, 0x7C6307B44E800020ULL ); // fix 8001003C error
			pokeq(0x8000000000059F58ULL, 0x63FF003D60000000ULL ); // fix 8001003D error
			pokeq(0x800000000005A01CULL, 0x3FE080013BE00000ULL ); // fix 8001003E error

			pokeq(0x8000000000059FC8ULL, 0x419E00D860000000ULL );
			pokeq(0x8000000000059FD0ULL, 0x2F84000448000098ULL );
			pokeq(0x800000000005E020ULL, 0x2F83000060000000ULL );
			pokeq(0x800000000005E038ULL, 0x2F83000060000000ULL );

			pokeq(0x8000000000059BFCULL, 0x386000012F830000ULL ); // ignore LIC.DAT check
			pokeq(0x80000000002367C4ULL, 0x38600000F8690000ULL ); // fix 0x8001002B / 80010017 errors (ported for DEX 2015-01-03)

			pokeq(0x8000000000059628ULL, 0xF821FE917C0802A6ULL ); // just restore the original
			pokeq(0x800000000005C77CULL, 0x419E0038E8610098ULL ); // just restore the original

#ifndef COBRA_ONLY
			sc_600=0x364DE0; //0x38A120 + 600*8 = 0038B3E0 -> 80 00 00 00 00 36 4D E0
			sc_604=0x364EB8; //0x38A120 + 604*8 = 0038B400 -> 80 00 00 00 00 36 4E B8
			sc_142=0x328E70; //0x38A120 + 142*8 = 0038A590 -> 80 00 00 00 00 32 8E 70
#endif
		}
		else
		if(c_firmware==4.65f || c_firmware==4.66f)
		{
			//patches by deank
			pokeq(0x80000000002764F8ULL, 0x4E80002038600000ULL ); // fix 8001003C error  Original: 0x4E8000208003026CULL
			pokeq(0x8000000000276500ULL, 0x7C6307B44E800020ULL ); // fix 8001003C error  Original: 0x3D6000463D201B43ULL
			pokeq(0x8000000000059F5CULL, 0x63FF003D60000000ULL ); // fix 8001003D error  Original: 0x63FF003D419EFFD4ULL
			pokeq(0x800000000005A020ULL, 0x3FE080013BE00000ULL ); // fix 8001003E error  Original: 0x3FE0800163FF003EULL

			pokeq(0x8000000000059FCCULL, 0x419E00D860000000ULL ); // Original: 0x419E00D8419D00C0ULL
			pokeq(0x8000000000059FD4ULL, 0x2F84000448000098ULL ); // Original: 0x2F840004409C0048ULL //PATCH_JUMP
			pokeq(0x800000000005E028ULL, 0x2F83000060000000ULL ); // fix 80010009 error  Original: 0x2F830000419E00ACULL
			pokeq(0x800000000005E03CULL, 0x2F83000060000000ULL ); // fix 80010009 error  Original: 0x2F830000419E00ACULL

			pokeq(0x8000000000059C00ULL, 0x386000012F830000ULL ); // ignore LIC.DAT check
			pokeq(0x80000000002367CCULL, 0x38600000F8690000ULL ); // fix 0x8001002B / 80010017 errors (ported for DEX 2015-01-03)

			pokeq(0x800000000005962CULL, 0xF821FE917C0802A6ULL ); // just restore the original
			pokeq(0x800000000005C780ULL, 0x419E0038E8610098ULL ); // just restore the original

		 /*
			//anti-ode patches by deank
			if(!is_rebug)
			{
				//pokeq(0x800000000005962CULL, 0xF821FE917C0802A6ULL );
				pokeq(0x8000000000059654ULL, 0x6000000060000000ULL );
				pokeq(0x800000000005965CULL, 0x600000003BA00000ULL );
			}

			//pokeq(0x800000000005C780ULL, 0x60000000E8610098ULL );

			// habib patches (newest ones)
			pokeq(0x8000000000059668ULL, 0x38600000EB610148ULL); //Original: 0x7FA307B4EB610148EB8101507C0803A6
			pokeq(0x800000000005C79CULL, 0x38600000EBA100C8ULL); //Original: 0x7FE307B4EBA100C8EBC100D07C0803A6

			// enable new habib patches (now obsolete) //replaced by deank's patch (2015-01-03)
			pokeq(0x800000000005C780ULL + 0x00, 0x60000000E8610098ULL);
			pokeq(0x800000000005C780ULL + 0x08, 0x2FA30000419E000CULL);
			pokeq(0x800000000005C780ULL + 0x10, 0x388000334800BE15ULL);
			pokeq(0x800000000005C780ULL + 0x18, 0xE80100F07FE307B4ULL);

			pokeq(0x800000000005962CULL + 0x00, 0x386000004E800020ULL);
			pokeq(0x800000000005962CULL + 0x08, 0xFBC10160FBE10168ULL);
			pokeq(0x800000000005962CULL + 0x10, 0xFB610148FB810150ULL);
			pokeq(0x800000000005962CULL + 0x18, 0xFBA10158F8010180ULL);

			//patch to prevent blackscreen on usb games in jb format
			pokeq(0x8000000000059654ULL, 0x386000002F830141ULL);
			pokeq(0x800000000005965CULL, 0x9E00303BA0000000ULL);
		 */

#ifndef COBRA_ONLY
			sc_600=0x364DF0; //0x38A120 + 600*8 = 0038B3E0 -> 80 00 00 00 00 36 4D F0
			sc_604=0x364EC8; //0x38A120 + 604*8 = 0038B400 -> 80 00 00 00 00 36 4E C8
			sc_142=0x328E80; //0x38A120 + 142*8 = 0038A590 -> 80 00 00 00 00 32 8E 80
#endif
		}
		else
		if(c_firmware==4.70f)
		{
			//patches by deank
			pokeq(0x800000000026D7F4ULL, 0x4E80002038600000ULL ); // fix 8001003C error  Original: 0x4E8000208003026CULL
			pokeq(0x800000000026D7FCULL, 0x7C6307B44E800020ULL ); // fix 8001003C error  Original: 0x3D201B433C608001ULL
            pokeq(0x8000000000059F58ULL, 0x63FF003D60000000ULL ); // fix 8001003D error  Original: 0x63FF003D419EFFD4ULL
			pokeq(0x800000000005A01CULL, 0x3FE080013BE00000ULL ); // fix 8001003E error  Original: 0x3FE0800163FF003EULL

			pokeq(0x8000000000059FC8ULL, 0x419E00D860000000ULL ); // Original: 0x419E00D8419D00C0ULL
			pokeq(0x8000000000059FD0ULL, 0x2F84000448000098ULL ); // Original: 0x2F840004409C0048ULL //PATCH_JUMP
			pokeq(0x800000000005E0ACULL, 0x2F83000060000000ULL ); // fix 80010009 error  Original: 0x2F830000419E00ACULL
			pokeq(0x800000000005E0C0ULL, 0x2F83000060000000ULL ); // fix 80010009 error  Original: 0x2F830000419E00ACULL

			pokeq(0x8000000000059BFCULL, 0x386000012F830000ULL ); // ignore LIC.DAT check
			pokeq(0x800000000022DAC8ULL, 0x38600000F8690000ULL ); // fix 0x8001002B / 80010017 errors (ported for DEX 2015-04)

			pokeq(0x8000000000059628ULL, 0xF821FE917C0802A6ULL ); // just restore the original
			pokeq(0x800000000005C7E8ULL, 0x419E0038E8610098ULL ); // just restore the original

#ifndef COBRA_ONLY
			sc_600=0x3647B8; //0x38A368 + 600*8 = 0038B628 -> 80 00 00 00 00 36 47 B8
			sc_604=0x364890; //0x38A368 + 604*8 = 0038B648 -> 80 00 00 00 00 36 48 90
			sc_142=0x329190; //0x38A368 + 142*8 = 0038A7D8 -> 80 00 00 00 00 32 91 90
#endif
		}
		else
		if(c_firmware==4.75f || c_firmware==4.76f)
		{
			//patches by deank
			pokeq(0x800000000026D868ULL, 0x4E80002038600000ULL ); // fix 8001003C error  Original: 0x4E8000208003026CULL
			pokeq(0x800000000026D870ULL, 0x7C6307B44E800020ULL ); // fix 8001003C error  Original: 0x3D6000473D201B43ULL
            pokeq(0x8000000000059F5CULL, 0x63FF003D60000000ULL ); // fix 8001003D error  Original: 0x63FF003D419EFFD4ULL
			pokeq(0x800000000005A020ULL, 0x3FE080013BE00000ULL ); // fix 8001003E error  Original: 0x3FE0800163FF003EULL

			pokeq(0x8000000000059FCCULL, 0x419E00D860000000ULL ); // Original: 0x419E00D8419D00C0ULL
			pokeq(0x8000000000059FD4ULL, 0x2F84000448000098ULL ); // Original: 0x2F840004409C0048ULL //PATCH_JUMP
			pokeq(0x800000000005E0B0ULL, 0x2F83000060000000ULL ); // fix 80010009 error  Original: 0x2F830000419E00ACULL
			pokeq(0x800000000005E0C4ULL, 0x2F83000060000000ULL ); // fix 80010009 error  Original: 0x2F830000419E00ACULL

			pokeq(0x8000000000059C00ULL, 0x386000012F830000ULL ); // ignore LIC.DAT check
			pokeq(0x800000000022DAD0ULL, 0x38600000F8690000ULL ); // fix 0x8001002B / 80010017 errors (ported for DEX 2015-08-14)

			pokeq(0x800000000005962CULL, 0xF821FE917C0802A6ULL ); // just restore the original
			pokeq(0x800000000005C7ECULL, 0x419E0038E8610098ULL ); // just restore the original

#ifndef COBRA_ONLY
			sc_600=0x364848; //0x38A3E8 + 600*8 = 0038B6A8 -> 80 00 00 00 00 36 48 48
			sc_604=0x364920; //0x38A3E8 + 604*8 = 0038B6C8 -> 80 00 00 00 00 36 49 20
			sc_142=0x329220; //0x38A3E8 + 142*8 = 0038A858 -> 80 00 00 00 00 32 92 20
#endif
		}
	}

	char titleID[10];

	char _path[MAX_PATH_LEN];

	strcpy(_path, _path0);

	int plen=strlen(_path)-9;

	if(_path[0]=='/' && plen>0)
	{
		for(int n=0; n<plen; n++)
			if(memcmp(_path + n, "/PS3_GAME", 9)==0) {_path[n]=0; break;}
	}

#ifndef LITE_EDITION
	if(!strcmp(_path, "/net0")) strcpy((char*)_path, "/net0/."); else
	if(!strcmp(_path, "/net1")) strcpy((char*)_path, "/net1/."); else
	if(!strcmp(_path, "/net2")) strcpy((char*)_path, "/net2/.");
#endif

	if(!strcmp(_path, "/dev_bdvd")) {do_umount(false); goto exit_mount;}

#ifndef COBRA_ONLY
 #ifdef EXT_GDATA
	if(do_eject==MOUNT_EXT_GDATA) goto patch;
 #endif
#endif

	// save lastgame.bin / process _next & _prev commands
	bool _prev=false, _next=false;
	if(do_eject)
	{
		//cobra_lib_init();

		//if(!strstr(_path0, "/PSPISO") && !strstr(_path0, "/ISO/"))
		{
			int fd=0;
			_lastgames lastgames; memset(&lastgames, 0, sizeof(_lastgames)); lastgames.last=250;

			if(cellFsOpen((char*)WMTMP "/last_games.bin", CELL_FS_O_RDONLY, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
			{
				cellFsRead(fd, (void *)&lastgames, sizeof(_lastgames), NULL);
				cellFsClose(fd);
			}

			_next=(bool)(strstr(_path0, "_next")!=NULL);
			_prev=(bool)(strstr(_path0, "_prev")!=NULL);

			if(_next || _prev)
			{
				if(lastgames.last>(MAX_LAST_GAMES-1)) goto exit_mount;
				if(_prev)
				{
					if(lastgames.last==0) lastgames.last=(MAX_LAST_GAMES-1); else lastgames.last--;
				}
				if(_next)
				{
					if(lastgames.last==(MAX_LAST_GAMES-1)) lastgames.last=0; else lastgames.last++;
				}
				if(lastgames.game[lastgames.last][0]!='/') lastgames.last=0;
				if(lastgames.game[lastgames.last][0]!='/' || strlen(lastgames.game[lastgames.last])<7) goto exit_mount;
				strcpy(_path, lastgames.game[lastgames.last]);
			}
			else

			{
				if(lastgames.last==250)
				{
					lastgames.last=0;
					strcpy(lastgames.game[lastgames.last], _path);
				}
				else
				{
					bool found=false;
					for(u8 n=0;n<MAX_LAST_GAMES;n++)
					{
						if(!strcmp(lastgames.game[n], _path)) {found=true; break;}
					}
					if(!found)
					{
						lastgames.last++;
						if(lastgames.last>(MAX_LAST_GAMES-1)) lastgames.last=0;
						strcpy(lastgames.game[lastgames.last], _path);
					}
				}
			}

			if(cellFsOpen((char*)WMTMP "/last_games.bin", CELL_FS_O_CREAT | CELL_FS_O_TRUNC | CELL_FS_O_WRONLY, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
			{
				cellFsWrite(fd, (void *)&lastgames, sizeof(_lastgames), NULL);
				cellFsClose(fd);
				cellFsChmod((char*)WMTMP "/last_games.bin", MODE);
			}
		}
	}

	// last mounted game
	if(_path[0]=='_' || strrchr(_path, '/')==NULL) goto exit_mount;
	else
	{
		char path2[MAX_PATH_LEN]; int fd;
		sprintf(path2, WMTMP "/last_game.txt");

		if(cellFsOpen(path2, CELL_FS_O_CREAT | CELL_FS_O_TRUNC | CELL_FS_O_WRONLY, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
		{
			u64 written = 0;
			cellFsWrite(fd, (void *)_path, strlen(_path), &written);
			cellFsClose(fd);
			cellFsChmod(path2, MODE);
		}
	}

#ifdef COBRA_ONLY
 #ifdef FAKEISO
	// launch ntfs fake iso
	{
		if(strstr(_path, ".ntfs[BDFILE]") || (strstr(_path, ".ntfs[") && strstr(_path, "[raw]")))
		{

			bool found=false; u8 n;
			const char raw_iso_sprx[4][40] = {  "/dev_flash/vsh/module/raw_iso.sprx",
												"/dev_hdd0/raw_iso.sprx",
												"/dev_hdd0/plugins/raw_iso.sprx",
												"/dev_hdd0/game/IRISMAN00/sprx_iso" };

			for(n = 0; n < 4; n++)
				if(FileExists(raw_iso_sprx[n])) {found = true; break;}

			if(found)
			{
				cellFsChmod(_path, MODE);

				int fdw;
				if(cellFsOpen((char*)_path, CELL_FS_O_RDONLY, &fdw, NULL, 0)==CELL_FS_SUCCEEDED)
				{
					uint64_t msiz = 0; uint8_t sprx_data[_64KB_];
					cellFsLseek(fdw, 0, CELL_FS_SEEK_SET, &msiz);
					cellFsRead(fdw, sprx_data, _64KB_, &msiz);
					cellFsClose(fdw);

					cobra_unload_vsh_plugin(0);

					do_umount(false);
					cobra_load_vsh_plugin(0, (char*)raw_iso_sprx[n], sprx_data, msiz);

					_path[strlen(_path)-13]=0;
					sprintf((char*)sprx_data, "\"%s\" %s", _path+20, STR_LOADED2);
					show_msg((char*)sprx_data);
					goto patch;
				}
			}
		}
	}
 #endif
#endif

	// Launch PS2 Classic
	if(!extcmp(_path, ".BIN.ENC", 8))
	{
		char temp[MAX_PATH_LEN];

		if(FileExists(PS2_CLASSIC_PLACEHOLDER))
		{
			sprintf(temp, "PS2 Classic\n%s", strrchr(_path, '/') + 1);
			copy_in_progress=true; copied_count = 0;
			show_msg(temp);

			if(c_firmware>=4.65f)
			{   // Auto create "classic_ps2 flag" for PS2 Classic (.BIN.ENC) on rebug 4.65.2
				do_umount(false);
				enable_classic_ps2_mode();
			}

			cellFsUnlink(PS2_CLASSIC_ISO_PATH);
			if(filecopy(_path, (char*)PS2_CLASSIC_ISO_PATH, COPY_WHOLE_FILE) == 0)
			{
				if(FileExists(PS2_CLASSIC_ISO_ICON ".bak")==false)
					filecopy((char*)PS2_CLASSIC_ISO_ICON, (char*)(PS2_CLASSIC_ISO_ICON ".bak"), COPY_WHOLE_FILE);

				sprintf(temp, "%s.png", _path);
				if(FileExists(temp)==false) sprintf(temp, "%s.PNG", _path);

				cellFsUnlink(PS2_CLASSIC_ISO_ICON);
				if(FileExists(temp))
					filecopy(temp, (char*)PS2_CLASSIC_ISO_ICON, COPY_WHOLE_FILE);
				else
					filecopy((char*)(PS2_CLASSIC_ISO_ICON ".bak"), (char*)PS2_CLASSIC_ISO_ICON, COPY_WHOLE_FILE);

				if(webman_config->fanc) restore_fan(1); //fan_control( ((webman_config->ps2temp*255)/100), 0);

				// create "wm_noscan" to avoid re-scan of XML returning to XMB from PS2
				savefile((char*)WMNOSCAN, NULL, 0);

				sprintf(temp, "\"%s\" %s", strrchr(_path, '/') + 1, STR_LOADED2);
			}
			else
				{sprintf(temp, "PS2 Classic\n%s", STR_ERROR); ret=false;}

			show_msg(temp);
			copy_in_progress=false;
		}
		else
		{
			sprintf(temp, "PS2 Classic Placeholder %s", STR_NOTFOUND);
			show_msg(temp);
			ret=false;
		}

		goto patch;
	}

	if((c_firmware>=4.65f) && strstr(_path, "/PS2ISO")!=NULL)
	{   // Auto remove "classic_ps2" flag for PS2 ISOs on rebug 4.65.2
		disable_classic_ps2_mode();
	}

#ifdef COBRA_ONLY
	//if(cobra_mode)
	{
 #ifdef EXT_GDATA
		{
			// auto-enable external GD
			if(do_eject!=1) ;
			else if(strstr(_path, "/GAME"))
			{
				int fdd=0; char extgdfile[540];
				sprintf(extgdfile, "%s/PS3_GAME/PS3GAME.INI", _path);
				if(cellFsOpen(extgdfile, CELL_FS_O_RDONLY, &fdd, NULL, 0) == CELL_FS_SUCCEEDED)
				{
					u64 read_e = 0;
					if(cellFsRead(fdd, (void *)&extgdfile, 12, &read_e) == CELL_FS_SUCCEEDED) extgdfile[read_e]=0;
					cellFsClose(fdd);
					if((extgd==0) &&  (extgdfile[10] & (1<<1))) set_gamedata_status(1, false); else
					if((extgd==1) && !(extgdfile[10] & (1<<1))) set_gamedata_status(0, false);
				}
				else if(extgd) set_gamedata_status(0, false);
			}
			else if((extgd==0) && (strstr(_path, "/PS3ISO")!=NULL) && (strstr(_path, "[gd]")!=NULL))
				set_gamedata_status(1, false);
			else if(extgd) set_gamedata_status(0, false);
		}
 #endif //#ifdef EXT_GDATA

		{
			// show loaded path
			char path2[MAX_PATH_LEN];
			char temp[MAX_PATH_LEN];
			sprintf(path2, "\"%s", (strrchr(_path, '/')+1));
			if(strstr(path2, ".ntfs[")) path2[strrchr(path2, '.')-path2]=0;
			if(strrchr(path2, '.')!=NULL) path2[strrchr(path2, '.')-path2]=0;
			if(path2[1]==NULL) sprintf(path2, "\"%s", _path);
			sprintf(temp, "\" %s", STR_LOADED2); strcat(path2, temp);
			show_msg(path2);
		}

		do_umount(false);

		sys_timer_usleep(4000);
		cobra_send_fake_disc_eject_event();
		sys_timer_usleep(4000);

		if( strstr(_path, "/PS3ISO") || strstr(_path, "/BDISO")    || strstr(_path, "/DVDISO") || strstr(_path, "/PS2ISO") ||
			strstr(_path, "/PSXISO") || strstr(_path, "/PSXGAMES") || strstr(_path, "/PSPISO") || strstr(_path, "/ISO/")    ||
			strstr(_path,"/net0/")    || strstr(_path,"/net1/")      || strstr(_path, "/net2/")   || strstr(_path, ".ntfs[") )
		{
			if( strstr(_path, "/PSXISO") || strstr(_path, "/PSXGAMES") ) select_ps1emu();

			if(_next || _prev)
				sys_timer_sleep(1);
			else
				sys_timer_usleep(50000);

			u8 iso_num=1;
			char templn[MAX_LINE_LEN];
			char iso_list[16][MAX_PATH_LEN];
			char *cobra_iso_list[16];
			char path2[MAX_PATH_LEN];
			strcpy(iso_list[0], _path);
			cobra_iso_list[0] = (char*)iso_list[0];

			if(!extcasecmp(_path, ".iso.0", 6))
			{
				sprintf(path2, "%s", _path);
				path2[strlen(path2)-2]=0;
				for(u8 n=1;n<16;n++)
				{
					sprintf(templn, "%s.%i", path2, n);
					if(FileExists(templn))
					{
						iso_num++;
						strcpy(iso_list[n], templn);
						cobra_iso_list[n] = (char*)iso_list[n];
					}
					else
						break;
				}
			}

			if(strstr(_path, ".ntfs["))
			{
				int fdw;
				if(cellFsOpen(_path, CELL_FS_O_RDONLY, &fdw, NULL, 0)==CELL_FS_SUCCEEDED)
				{
					sys_addr_t addr=0;
					if(sys_memory_allocate(_64KB_, SYS_MEMORY_PAGE_SIZE_64K, &addr)==0)
					{
						u8* sprx_data=(u8*)addr; uint64_t msiz = 0;
						cellFsLseek(fdw, 0, CELL_FS_SEEK_SET, &msiz);
						cellFsRead(fdw, sprx_data, (_64KB_), &msiz);
						cellFsClose(fdw);

						sys_ppu_thread_create(&thread_id_ntfs, rawseciso_thread, (uint64_t)addr, -0x1d8, 0x2000, SYS_PPU_THREAD_CREATE_JOINABLE, THREAD_NAME_NTFS);

						if(strstr(_path, ".ntfs[PS3ISO]"))
						{
							get_name(iso_list[15], _path, 0); strcat(iso_list[15], ".SFO\0");

							// cache PARAM.SFO
							if(FileExists(iso_list[15])==false)
							{
								for(u8 n=0;n<10;n++)
								{
									if(filecopy((char*)"/dev_bdvd/PS3_GAME/PARAM.SFO", iso_list[15], _4KB_)==CELL_FS_SUCCEEDED) break;
									sys_timer_usleep(500000);
								}
							}

							// cache ICON0.PNG
							iso_list[15][strlen(iso_list[15])-4]=0; strcat(iso_list[15], ".PNG");
							if(FileExists(iso_list[15])==false)
							{
								for(u8 n=0;n<10;n++)
								{
									if(filecopy((char*)"/dev_bdvd/PS3_GAME/ICON0.PNG", iso_list[15], COPY_WHOLE_FILE)==CELL_FS_SUCCEEDED) break;
									sys_timer_usleep(500000);
								}
							}
						}

					}
				}
				goto patch;
				//return;
			}

 #ifndef LITE_EDITION
			if(strstr(_path, "/net0") || strstr(_path, "/net1") || strstr(_path, "/net2"))
			{
				sys_addr_t addr=0;
				if(sys_memory_allocate(_64KB_, SYS_MEMORY_PAGE_SIZE_64K, &addr)==0)
				{
					netiso_args *mynet_iso	= (netiso_args*)addr;
					memset(mynet_iso, 0, _64KB_);

					if( (_path[4]=='0' && webman_config->netd0 && webman_config->neth0[0] && webman_config->netp0>0) ||
						(_path[4]=='1' && webman_config->netd1 && webman_config->neth1[0] && webman_config->netp1>0) ||
						(_path[4]=='2' && webman_config->netd2 && webman_config->neth2[0] && webman_config->netp2>0) )
					{
						if(_path[4]=='1')
						{
							sprintf(mynet_iso->server, "%s", webman_config->neth1);
							mynet_iso->port=webman_config->netp1;
						}
						else
						if(_path[4]=='2')
						{
							sprintf(mynet_iso->server, "%s", webman_config->neth2);
							mynet_iso->port=webman_config->netp2;
						}
						else
						{
							sprintf(mynet_iso->server, "%s", webman_config->neth0);
							mynet_iso->port=webman_config->netp0;
						}
					}
					else
					{
						sys_memory_free(addr);
						goto patch;
					}

					strcpy(mynet_iso->path, _path+5);
					if(strstr(_path, "/PS3ISO")) mynet_iso->emu_mode=EMU_PS3; else
					if(strstr(_path, "/BDISO" )) mynet_iso->emu_mode=EMU_BD;  else
					if(strstr(_path, "/DVDISO")) mynet_iso->emu_mode=EMU_DVD; else
					if(strstr(_path, "/PSX"))
					{
						TrackDef tracks[1];
						tracks[0].lba = 0;
						tracks[0].is_audio = 0;
						mynet_iso->emu_mode=EMU_PSX;
						mynet_iso->num_tracks=1;

						memcpy(mynet_iso->tracks, tracks, sizeof(TrackDef));
					}
					else if(strstr(_path, "/GAMES") || strstr(_path, "/GAMEZ"))
					{
						mynet_iso->emu_mode=EMU_PS3;
						sprintf(mynet_iso->path, "/***PS3***%s", _path+5);
					}
					else
					{
						mynet_iso->emu_mode=EMU_DVD;
						sprintf(mynet_iso->path, "/***DVD***%s", _path+5);
					}

					sys_ppu_thread_create(&thread_id_net, netiso_thread, (uint64_t)addr, -0x1d8, 0x2000, SYS_PPU_THREAD_CREATE_JOINABLE, THREAD_NAME_NET);

					if(mynet_iso->emu_mode==EMU_PS3)
					{
						sprintf(iso_list[15], WMTMP "/%s", (strrchr(_path, '/')+1));
						if(!strstr(mynet_iso->path, "/***PS3***")) iso_list[15][strlen(iso_list[15])-4]=0;
						strcat(iso_list[15], ".SFO\0");

						// cache PARAM.SFO
						if(FileExists(iso_list[15])==false)
						{
							for(u8 n=0;n<30;n++)
							{
								if(filecopy((char*)"/dev_bdvd/PS3_GAME/PARAM.SFO", iso_list[15], _4KB_)==CELL_FS_SUCCEEDED) break;
								sys_timer_usleep(500000);
							}
						}

						// cache ICON0.PNG
						iso_list[15][strlen(iso_list[15])-4]=0; strcat(iso_list[15], ".PNG");
						if(FileExists(iso_list[15])==false)
						{
							for(u8 n=0;n<30;n++)
							{
								if(filecopy((char*)"/dev_bdvd/PS3_GAME/ICON0.PNG", iso_list[15], COPY_WHOLE_FILE)==CELL_FS_SUCCEEDED) break;
								sys_timer_usleep(500000);
							}
						}
					}
				}
				goto patch;
				//return;
			}
			else
 #endif //#ifndef LITE_EDITION
			{
				cellFsUnlink((char*)WMNOSCAN); // remove wm_noscan if PS2ISO was already mounted

				if(strstr(_path, "/PS3ISO"))
				{
 #ifdef FIX_GAME
					if(webman_config->fixgame!=FIX_GAME_DISABLED)
					{
						fix_in_progress=true; fix_aborted = false;
						fix_iso(_path, 0x100000UL, true);
						fix_in_progress=false;
					}
 #endif //#ifdef FIX_GAME
					cobra_mount_ps3_disc_image(cobra_iso_list, iso_num);
					sys_timer_usleep(2500);
					cobra_send_fake_disc_insert_event();

					{
						get_name(iso_list[15], (strrchr(_path, '/')+1), 1); strcat(iso_list[15], ".SFO\0");

						// cache PARAM.SFO
						if(FileExists(iso_list[15])==false)
						{
							for(u8 n=0;n<5;n++)
							{
								if(filecopy((char*)"/dev_bdvd/PS3_GAME/PARAM.SFO", iso_list[15], _4KB_)==CELL_FS_SUCCEEDED) break;
								sys_timer_usleep(500000);
							}
						}

						// cache ICON0.PNG
						iso_list[15][strlen(iso_list[15])-4]=0; strcat(iso_list[15], ".PNG");
						if(FileExists(iso_list[15])==false)
						{
							for(u8 n=0;n<5;n++)
							{
								if(filecopy((char*)"/dev_bdvd/PS3_GAME/ICON0.PNG", iso_list[15], COPY_WHOLE_FILE)==CELL_FS_SUCCEEDED) break;
								sys_timer_usleep(500000);
							}
						}
					}
					goto patch;
					//return;
				}
				else if(strstr(_path, "/PSPISO") || strstr(_path, "/ISO/"))
				{
					delete_history(false);

					cellFsUnlink((char*)"/dev_hdd0/game/PSPC66820/PIC1.PNG");
					cobra_unset_psp_umd();
					int result=cobra_set_psp_umd2(_path, NULL, (char*)"/dev_hdd0/tmp/psp_icon.png", 2);
					is_mounting=false;
					if(result==ENOTSUP || result==EABORT)
						return false;
					else if(!result)
					{
						cobra_send_fake_disc_insert_event();
						return true;
					}
				}
				else if(strstr(_path, "/BDISO"))
					cobra_mount_bd_disc_image(cobra_iso_list, iso_num);
				else if(strstr(_path, "/DVDISO"))
					cobra_mount_dvd_disc_image(cobra_iso_list, iso_num);
				else if(strstr(_path, "/PS2ISO"))
				{
					TrackDef tracks[1];
					tracks[0].lba = 0;
					tracks[0].is_audio = 0;
					cobra_mount_ps2_disc_image(cobra_iso_list, 1, tracks, 1);
					if(webman_config->fanc) restore_fan(1); //fan_control( ((webman_config->ps2temp*255)/100), 0);

					// create "wm_noscan" to avoid re-scan of XML returning to XMB from PS2
					savefile((char*)WMNOSCAN, NULL, 0);
				}
				else if(strstr(_path, "/PSXISO") || strstr(_path, "/PSXGAMES"))
				{
					if(!extcasecmp(_path, ".cue", 4))
					{
						int flen=strlen(cobra_iso_list[0]);

						char extensions[8][8]={".BIN", ".bin", ".iso", ".ISO", ".img", ".IMG", ".mdf", ".MDF"};
						for(u8 e=0; e<8; e++)
						{
							cobra_iso_list[0][flen-4]=0; strcat(cobra_iso_list[0], extensions[e]);
							if(FileExists(cobra_iso_list[0])) break;
						}

						unsigned int num_tracks=0;
						int fdw;

						sys_addr_t buf1=0;
						if(sys_memory_allocate(_64KB_, SYS_MEMORY_PAGE_SIZE_64K, &buf1)==0)
						{
							if(cellFsOpen(_path, CELL_FS_O_RDONLY, &fdw, NULL, 0)==CELL_FS_SUCCEEDED)
							{
								char *buf=(char*)buf1; uint64_t msiz = 0;
								cellFsLseek(fdw, 0, CELL_FS_SEEK_SET, &msiz);
								cellFsRead(fdw, (void *)buf, 65500, &msiz);
								cellFsClose(fdw);

								if(msiz>10)
								{

									TrackDef tracks[32];
									tracks[0].lba = 0;
									tracks[0].is_audio = 0;

									char tcode[MAX_LINE_LEN];
									u8 tmin=0, tsec=0, tfrm=0;
									u8 use_pregap=0;
									u32 lp=0;

									while(lp<msiz)// get_line ( templn, 512, buf1 ) != NULL )
									{
										u8 line_found=0;
										templn[0]=0;
										for(u32 l=0; l<511; l++)
										{
											if(l>=msiz) break;
											if(lp<msiz && buf[lp] && buf[lp]!='\n' && buf[lp]!='\r')
											{
												templn[l]=buf[lp];
												templn[l+1]=0;
											}
											else
											{
												templn[l]=0;
											}
											if(buf[lp]=='\n' || buf[lp]=='\r') line_found=1;
											lp++;
											if(buf[lp]=='\n' || buf[lp]=='\r') lp++;

											if(templn[l]==0) break;
										}

										if(!line_found) break;

										if(strstr(templn, "PREGAP")) {use_pregap=1; continue;}
										if(!strstr(templn, "INDEX 01") && !strstr(templn, "INDEX 1 ")) continue;

										sprintf(tcode, "%s", strrchr(templn, ' ')+1); tcode[8]=0;
										if(strlen(tcode)!=8 || tcode[2]!=':' || tcode[5]!=':') continue;
										tmin=(tcode[0]-'0')*10 + (tcode[1]-'0');
										tsec=(tcode[3]-'0')*10 + (tcode[4]-'0');
										tfrm=(tcode[6]-'0')*10 + (tcode[7]-'0');
										if(use_pregap && num_tracks) tsec+=2;

										if(num_tracks) tracks[num_tracks].is_audio = 1;
										tracks[num_tracks].lba=(tmin*60 + tsec)*75 + tfrm;

										num_tracks++; if(num_tracks>=32) break;
									}

									if(!num_tracks) num_tracks++;
									cobra_mount_psx_disc_image(cobra_iso_list[0], tracks, num_tracks);
								}
							}
							else
							{
								TrackDef tracks[1];
								tracks[0].lba = 0;
								tracks[0].is_audio = 0;
								cobra_mount_psx_disc_image_iso(cobra_iso_list[0], tracks, 1);
							}

							sys_memory_free(buf1);
						}
					}
					else
					{
						TrackDef tracks[1];
						tracks[0].lba = 0;
						tracks[0].is_audio = 0;
						cobra_mount_psx_disc_image_iso(cobra_iso_list[0], tracks, 1);
					}
				}

				sys_timer_usleep(2500);
				cobra_send_fake_disc_insert_event();
			}
		}
		else
		{
			int special_mode=0;

 #ifdef EXTRA_FEAT
			CellPadData pad_data = pad_read();

			if(pad_data.len > 0 && (pad_data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_SELECT)) special_mode=true; //mount also app_home / eject disc
			sys_timer_usleep(10000);

			if(special_mode) eject_insert(1, 0);
 #endif //#ifdef EXTRA_FEAT

			// -- get TitleID from PARAM.SFO
 #ifndef FIX_GAME
			memset(titleID, 0, 10);

			char filename[MAX_PATH_LEN]; int fs;
			sprintf(filename, "%s/PS3_GAME/PARAM.SFO", _path);
			if(cellFsOpen(filename, CELL_FS_O_RDONLY, &fs, NULL, 0)==CELL_FS_SUCCEEDED)
			{
				char paramsfo[_4KB_]; unsigned char *mem = (u8*)paramsfo;
				uint64_t msiz = 0;

				cellFsLseek(fs, 0, CELL_FS_SEEK_SET, &msiz);
				cellFsRead(fs, (void *)&paramsfo, _4KB_, &msiz);
				cellFsClose(fs);

				// get titleid
				fix_param_sfo(mem, titleID, 0);
			}
 #else
			fix_game(_path, titleID, webman_config->fixgame);
 #endif //#ifndef FIX_GAME
			// ----

			// -- reset USB bus
			if(!webman_config->bus)
			{
				if(strstr(_path, "/dev_usb") && isDir(_path))
				{
					reset_usb_ports(_path);
				}
			}

			// -- mount game folder
			if(titleID[0] && titleID[8]>='0')
				cobra_map_game(_path, titleID, &special_mode);
			else
				cobra_map_game(_path, (char*)"TEST00000", &special_mode);
		}
		//return;
	}
#endif //#ifdef COBRA_ONLY

patch:

#ifndef COBRA_ONLY

	if(c_firmware==0.0f) {ret=false; goto exit_mount;}

	//if(!cobra_mode)
	{
        //Remove Lv2 memory protection
		if(c_firmware==4.21f)
		{
			poke_lv1(HV_START_OFFSET_421 + 0x00, 0x0000000000000001ULL);
			poke_lv1(HV_START_OFFSET_421 + 0x08, 0xe0d251b556c59f05ULL);
			poke_lv1(HV_START_OFFSET_421 + 0x10, 0xc232fcad552c80d7ULL);
			poke_lv1(HV_START_OFFSET_421 + 0x18, 0x65140cd200000000ULL);
		}
		else
		if(c_firmware>=4.30f && c_firmware<=4.53f)
		{
			poke_lv1(HV_START_OFFSET_430 + 0x00, 0x0000000000000001ULL);
			poke_lv1(HV_START_OFFSET_430 + 0x08, 0xe0d251b556c59f05ULL);
			poke_lv1(HV_START_OFFSET_430 + 0x10, 0xc232fcad552c80d7ULL);
			poke_lv1(HV_START_OFFSET_430 + 0x18, 0x65140cd200000000ULL);
		}
        else
		if(c_firmware>=4.55f && c_firmware<=4.76f)
		{
			poke_lv1(HV_START_OFFSET_460 + 0x00, 0x0000000000000001ULL);
			poke_lv1(HV_START_OFFSET_460 + 0x08, 0xe0d251b556c59f05ULL);
			poke_lv1(HV_START_OFFSET_460 + 0x10, 0xc232fcad552c80d7ULL);
			poke_lv1(HV_START_OFFSET_460 + 0x18, 0x65140cd200000000ULL);
		}

		if(do_eject) eject_insert(1, 1);

		if(c_firmware>=4.30f && c_firmware<=4.76f)
		{	// add and enable lv2 peek/poke + lv1 peek/poke
			pokeq(0x800000000000171CULL + 0x00, 0x7C0802A6F8010010ULL);
			pokeq(0x800000000000171CULL + 0x08, 0x396000B644000022ULL);
			pokeq(0x800000000000171CULL + 0x10, 0x7C832378E8010010ULL);
			pokeq(0x800000000000171CULL + 0x18, 0x7C0803A64E800020ULL);
			pokeq(0x800000000000171CULL + 0x20, 0x7C0802A6F8010010ULL);
			pokeq(0x800000000000171CULL + 0x28, 0x396000B744000022ULL);
			pokeq(0x800000000000171CULL + 0x30, 0x38600000E8010010ULL);
			pokeq(0x800000000000171CULL + 0x38, 0x7C0803A64E800020ULL);
			pokeq(0x800000000000171CULL + 0x40, 0x7C0802A6F8010010ULL);
			pokeq(0x800000000000171CULL + 0x48, 0x7D4B537844000022ULL);
			pokeq(0x800000000000171CULL + 0x50, 0xE80100107C0803A6ULL);
			pokeq(0x800000000000171CULL + 0x58, 0x4E80002080000000ULL);
			pokeq(0x800000000000171CULL + 0x60, 0x0000170C80000000ULL);
			pokeq(0x800000000000171CULL + 0x68, 0x0000171480000000ULL);
			pokeq(0x800000000000171CULL + 0x70, 0x0000171C80000000ULL);
			pokeq(0x800000000000171CULL + 0x78, 0x0000173C80000000ULL);
			pokeq(0x800000000000171CULL + 0x80, 0x0000175C00000000ULL);

			pokeq(SYSCALL_PTR( 6), 0x8000000000001778ULL); //sc6
			pokeq(SYSCALL_PTR( 7), 0x8000000000001780ULL); //sc7
			pokeq(SYSCALL_PTR( 8), 0x8000000000001788ULL); //sc8
			pokeq(SYSCALL_PTR( 9), 0x8000000000001790ULL); //sc9
			pokeq(SYSCALL_PTR(10), 0x8000000000001798ULL); //sc10
		}
	}

	pokeq(0x8000000000000000ULL+MAP_ADDR, 0x0000000000000000ULL);
	pokeq(0x8000000000000008ULL+MAP_ADDR, 0x0000000000000000ULL);

	if(cobra_mode) goto exit_mount;

	if(base_addr==0) {ret=false; goto exit_mount;}

	// restore syscall table
	u64 sc_null = peekq(SYSCALL_TABLE);

	if(sc_null == peekq(SYSCALL_PTR(79)))
	{
		pokeq(SYSCALL_PTR(  35), sc_null);
		pokeq(SYSCALL_PTR(  36), sc_null);
		pokeq(SYSCALL_PTR(  37), sc_null);
		//pokeq(SYSCALL_PTR(1023), sc_null);

		if(sc_600)
		{   // restore original values
			sc_600|=0x8000000000000000ULL;
			sc_604|=0x8000000000000000ULL;
			sc_142|=0x8000000000000000ULL;

			pokeq(SYSCALL_PTR(600), sc_600); // sys_storage_open 600
			pokeq(SYSCALL_PTR(604), sc_604); // sys_storage_send_device_cmd 604
			pokeq(SYSCALL_PTR(142), sc_142); // sys_timer_sleep 142
		}
	}

	pokeq(0x80000000007FD000ULL, 0);

	// disable mM path table
	pokeq(0x8000000000000000ULL+MAP_ADDR, 0x0000000000000000ULL);
	pokeq(0x8000000000000008ULL+MAP_ADDR, 0x0000000000000000ULL);

	// disable Iris path table
	pokeq(0x80000000007FD000ULL,		  0x0000000000000000ULL);

	// restore hook used by all payloads)
	pokeq(open_hook + 0x00, 0xF821FF617C0802A6ULL);
	pokeq(open_hook + 0x08, 0xFB810080FBA10088ULL);
	pokeq(open_hook + 0x10, 0xFBE10098FB410070ULL);
	pokeq(open_hook + 0x18, 0xFB610078F80100B0ULL);
	pokeq(open_hook + 0x20, 0x7C9C23787C7D1B78ULL);

	// poke mM payload
	pokeq(base_addr + 0x00, 0x7C7D1B783B600001ULL);
	pokeq(base_addr + 0x08, 0x7B7BF806637B0000ULL | MAP_ADDR);
	pokeq(base_addr + 0x10, 0xEB5B00002C1A0000ULL);
	pokeq(base_addr + 0x18, 0x4D820020EBFB0008ULL);
	pokeq(base_addr + 0x20, 0xE8BA00002C050000ULL);
	pokeq(base_addr + 0x28, 0x418200CC7FA3EB78ULL);
	pokeq(base_addr + 0x30, 0xE89A001089640000ULL);
	pokeq(base_addr + 0x38, 0x892300005560063EULL);
	pokeq(base_addr + 0x40, 0x7F895800409E0040ULL);
	pokeq(base_addr + 0x48, 0x2F8000007CA903A6ULL);
	pokeq(base_addr + 0x50, 0x409E002448000030ULL);
	pokeq(base_addr + 0x58, 0x8964000089230000ULL);
	pokeq(base_addr + 0x60, 0x5560063E7F895800ULL);
	pokeq(base_addr + 0x68, 0x2F000000409E0018ULL);
	pokeq(base_addr + 0x70, 0x419A001438630001ULL);
	pokeq(base_addr + 0x78, 0x388400014200FFDCULL);
	pokeq(base_addr + 0x80, 0x4800000C3B5A0020ULL);
	pokeq(base_addr + 0x88, 0x4BFFFF98E89A0018ULL);
	pokeq(base_addr + 0x90, 0x7FE3FB7888040000ULL);
	pokeq(base_addr + 0x98, 0x2F80000098030000ULL);
	pokeq(base_addr + 0xA0, 0x419E00187C691B78ULL);
	pokeq(base_addr + 0xA8, 0x8C0400012F800000ULL);
	pokeq(base_addr + 0xB0, 0x9C090001409EFFF4ULL);
	pokeq(base_addr + 0xB8, 0xE8BA00087C632A14ULL);
	pokeq(base_addr + 0xC0, 0x7FA4EB78E8BA0000ULL);
	pokeq(base_addr + 0xC8, 0x7C842A1488040000ULL);
	pokeq(base_addr + 0xD0, 0x2F80000098030000ULL);
	pokeq(base_addr + 0xD8, 0x419E00187C691B78ULL);
	pokeq(base_addr + 0xE0, 0x8C0400012F800000ULL);
	pokeq(base_addr + 0xE8, 0x9C090001409EFFF4ULL);
	pokeq(base_addr + 0xF0, 0x7FFDFB787FA3EB78ULL);
	pokeq(base_addr + 0xF8, 0x4E8000204D4D504CULL); //blr + "MMPL"

	pokeq(MAP_BASE  + 0x00, 0x0000000000000000ULL);
	pokeq(MAP_BASE  + 0x08, 0x0000000000000000ULL);
	pokeq(MAP_BASE  + 0x10, 0x8000000000000000ULL);
	pokeq(MAP_BASE  + 0x18, 0x8000000000000000ULL);

	pokeq(0x8000000000000000ULL+MAP_ADDR, MAP_BASE);
	pokeq(0x8000000000000008ULL+MAP_ADDR, 0x80000000007FDBE0ULL);

	pokeq(open_hook + 0x20, (0x7C9C237848000001ULL | (base_addr-open_hook-0x24)));

	char expplg[128];
	char app_sys[128];

	char path[MAX_PATH_LEN];

 #ifdef EXT_GDATA

	//------------------
	// re-load last game
	//------------------

	if(do_eject==MOUNT_EXT_GDATA) // extgd
	{
		sprintf(_path, WMTMP "/last_game.txt"); int fd=0;
		if(cellFsOpen(_path, CELL_FS_O_RDONLY, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
		{
			u64 read_e = 0;
			if(cellFsRead(fd, (void *)_path, MAX_PATH_LEN, &read_e) == CELL_FS_SUCCEEDED) _path[read_e]=0;
			cellFsClose(fd);
		}
		else
			_path[0]=0;
	}

 #endif //#ifdef EXT_GDATA

	if(_path[0] && strstr(_path, "/PS3_GAME/USRDIR/EBOOT.BIN")) _path[strlen(_path)-26]=0;

	sprintf(path, "%s", _path);

	if(!isDir(path)) _path[0]=path[0]=0;
	else if(do_eject)
	{   // show loaded path
		char path2[MAX_PATH_LEN];
		char temp[MAX_PATH_LEN];
		sprintf(path2, "\"%s", (strrchr(_path, '/')+1));
		if(path2[1]==NULL) sprintf(path2, "\"%s", _path);

		sprintf(temp, "\" %s", STR_LOADED2); strcat(path2, temp);
		show_msg(path2);
	}

	// -- get TitleID from PARAM.SFO
 #ifndef FIX_GAME
	char filename[MAX_PATH_LEN];

	sprintf(filename, "%s/PS3_GAME/PARAM.SFO", _path);
	getTitleID(filename, titleID, 0);
 #else
	fix_game(_path, titleID, webman_config->fixgame);
 #endif //#ifndef FIX_GAME
	// ----

	//----------------------------------
	// map game to /dev_bdvd & /app_home
	//----------------------------------

	if(path[0])
	{
		if(do_eject)
		{
			add_to_map((char*)"/dev_bdvd", path);
			add_to_map((char*)"//dev_bdvd", path);

			char path2[strlen(_path)+24];

			sprintf(path2, "%s/PS3_GAME", _path);
			add_to_map((char*)"/app_home/PS3_GAME", path2);

			sprintf(path2, "%s/PS3_GAME/USRDIR", _path);
			add_to_map((char*)"/app_home/USRDIR", path2);

			sprintf(path2, "%s/PS3_GAME/USRDIR/", _path);
			add_to_map((char*)"/app_home/", path2);
		}

		add_to_map((char*)"/app_home", path);
	}

 #ifdef EXT_GDATA

	//--------------------------------------------
	// auto-map /dev_hdd0/game to dev_usbxxx/GAMEI
	//---------------------------------------------

	if(do_eject!=1) ;
	else if(strstr(_path, "/GAME"))
	{
		int fdd=0; char extgdfile[540];
		sprintf(extgdfile, "%s/PS3_GAME/PS3GAME.INI", _path);
		if(cellFsOpen(extgdfile, CELL_FS_O_RDONLY, &fdd, NULL, 0) == CELL_FS_SUCCEEDED)
		{
			u64 read_e = 0;
			if(cellFsRead(fdd, (void *)&extgdfile, 12, &read_e) == CELL_FS_SUCCEEDED) extgdfile[read_e]=0;
			cellFsClose(fdd);
			if((extgd==0) &&  (extgdfile[10] & (1<<1))) set_gamedata_status(1, false); else
			if((extgd==1) && !(extgdfile[10] & (1<<1))) set_gamedata_status(0, false);
		}
		else if(extgd) set_gamedata_status(0, false);
	}

 #endif

	sprintf(app_sys, MM_ROOT_STD "/sys");
	if(!isDir(app_sys))
		sprintf(app_sys, MM_ROOT_STL "/sys");
	if(!isDir(app_sys))
		sprintf(app_sys, MM_ROOT_SSTL "/sys");


	//----------------------------
	// Patched explore_plugin.sprx
	//----------------------------

	if(c_firmware==4.21f)
		sprintf(expplg, "%s/IEXP0_420.BIN", app_sys);
	else if(c_firmware==4.30f || c_firmware==4.31f)
		sprintf(expplg, "%s/IEXP0_430.BIN", app_sys);
	else if(c_firmware==4.40f || c_firmware==4.41f)
		sprintf(expplg, "%s/IEXP0_440.BIN", app_sys);
	else if(c_firmware==4.46f)
		sprintf(expplg, "%s/IEXP0_446.BIN", app_sys);
	else if(c_firmware==4.50f || c_firmware==4.53f || c_firmware==4.55f)
		sprintf(expplg, "%s/IEXP0_450.BIN", app_sys);
	else if(c_firmware==4.60f || c_firmware==4.65f || c_firmware==4.66f)
		sprintf(expplg, "%s/IEXP0_460.BIN", app_sys);
	else if(c_firmware==4.70f || c_firmware==4.75f || c_firmware==4.76f)
		sprintf(expplg, "%s/IEXP0_470.BIN", app_sys);
	else
        sprintf(expplg, "%s/none", app_sys);

	if(do_eject && FileExists(expplg))
		add_to_map( (char*)"/dev_flash/vsh/module/explore_plugin.sprx", expplg);


	//---------------
	// New libfs.sprx
	//---------------
	if((do_eject>0) && (c_firmware>=4.20f) && FileExists(NEW_LIBFS_PATH))
		add_to_map((char*) ORG_LIBFS_PATH, (char*)NEW_LIBFS_PATH);

	//-----------------------------------------------//
	u64 map_data  = (MAP_BASE);
	u64 map_paths = (MAP_BASE) + (max_mapped+1) * 0x20;

	for(u16 n=0; n<0x400; n+=8) pokeq(map_data + n, 0);

	if(!max_mapped) {ret=false; goto exit_mount;}

	for(u8 n=0; n<max_mapped; n++)
	{
		if(map_paths>0x80000000007FE800ULL) break;
		pokeq(map_data + (n * 0x20) + 0x10, map_paths);
		string_to_lv2(file_to_map[n].src, map_paths);
		map_paths+= (strlen(file_to_map[n].src)+8)&0x7f8;

		pokeq(map_data + (n * 0x20) + 0x18, map_paths);
		string_to_lv2(file_to_map[n].dst, map_paths);
		map_paths+= (strlen(file_to_map[n].dst)+8)&0x7f8;

		pokeq(map_data + (n * 0x20) + 0x08, strlen(file_to_map[n].dst));
		pokeq(map_data + (n * 0x20) + 0x00, strlen(file_to_map[n].src));
	}

	if(isDir("/dev_bdvd")) sys_timer_sleep(2);

	//if(do_eject) eject_insert(0, 1);
#endif //#ifndef COBRA_ONLY

exit_mount:
	// wait few seconds until the game is mounted
	if(ret && extcmp(_path, ".BIN.ENC", 8))
	{
		waitfor((char*)"/dev_bdvd", ((strstr(_path0, "/dev_hdd0")) ? 6 : 15));
		if(!isDir("/dev_bdvd")) ret = false;
	}

#ifdef FIX_GAME
	// re-check PARAM.SFO to notify if game needs to be fixed
	if(ret && (c_firmware<4.76f))
	{
		char filename[64];
		sprintf(filename, "/dev_bdvd/PS3_GAME/PARAM.SFO");
		getTitleID(filename, titleID, 0);

		// check update folder
		sprintf(filename, "/dev_hdd0/game/%s", titleID);

		if(isDir(filename))
			sprintf(filename, "/dev_hdd0/game/%s/PARAM.SFO", titleID);
		else
			sprintf(filename, "/dev_bdvd/PS3_GAME/PARAM.SFO");

		getTitleID(filename, titleID, 1);
	}
#endif

	delete_history(false);

	if(!ret && !isDir("/dev_bdvd")) {char msg[MAX_PATH_LEN]; sprintf(msg, "%s %s", STR_ERROR, _path); show_msg(msg);}
#ifdef REMOVE_SYSCALLS
	else
	{
		CellPadData pad_data = pad_read();
		bool otag = (strcasestr(_path, "[online]")!=NULL);
		bool r2 = (pad_data.len>0 && (pad_data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R2));
		if((!r2 && otag) || (r2 && !otag)) remove_cfw_syscalls();
	}
#endif

#ifdef COBRA_ONLY
	{
		if(ret && (strstr(_path, ".PUP.ntfs[BD") || FileExists("/dev_bdvd/PS3UPDAT.PUP")))
			sys_map_path((char*)"/dev_bdvd/PS3/UPDATE", (char*)"/dev_bdvd"); //redirect root of bdvd to /dev_bdvd/PS3/UPDATE

		if(ret && (strstr(_path, "/net") && FileExists("/dev_bdvd/PKG")))
			sys_map_path((char*)"/app_home", (char*)"/dev_bdvd/PKG"); //redirect net_host/PKG to app_home

		sys_map_path((char*)"/dev_bdvd/PS3_UPDATE", (char*)"/dev_bdvd"); //redirect firmware update to root of bdvd
	}
#endif

	is_mounting=false;
	max_mapped=0;
    return ret;
}
