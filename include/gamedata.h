#ifdef EXT_GDATA

#define MOUNT_EXT_GDATA		2

static u8 extgd = 0;       //external gameDATA

static int set_gamedata_status(u8 status, bool do_mount)
{
	char msg[100];
	char gamei_path[MAX_PATH_LEN]; u8 n;

	{ PS3MAPI_ENABLE_ACCESS_SYSCALL8 }

#ifndef COBRA_ONLY
	sprintf(gamei_path, "//dev_hdd0/game");
	if(do_mount) max_mapped=0;
#endif

	if(status)
	{
		for(n=0; n<8; n++) {sprintf(gamei_path, "/dev_usb00%i/GAMEI", n); if(isDir(gamei_path)) break;}
		if(n>7)
		{
			for(n=0; n<8; n++) {sprintf(gamei_path, "/dev_usb00%i", n); if(isDir(gamei_path)) break;}
			if(n<8) {sprintf(gamei_path, "/dev_usb00%i/GAMEI", n); if(cellFsMkdir((char*)gamei_path, MODE)==CELL_FS_SUCCEEDED) n=99;}
		}

		if(n<8)
		{
#ifdef COBRA_ONLY

			sys_map_path((char*)"/dev_hdd0/game", gamei_path);
			if(isDir(MM_ROOT_STD)) sys_map_path((char*)MM_ROOT_STD, (char*)"/" MM_ROOT_STD);
#else
			if(isDir(MM_ROOT_STD)) add_to_map((char*)MM_ROOT_STD, (char*)MM_ROOT_STD);
			add_to_map((char*)"/dev_hdd0/game", gamei_path);
#endif
			sprintf(msg, "gameDATA %s (usb00%i)", STR_ENABLED, n);
		}
		else
		{
			extgd = 0;
#ifdef COBRA_ONLY
			sys_map_path((char*)"/dev_hdd0/game", NULL);
#endif
			sprintf(msg, (char*)"gameDATA %s (no usb)", STR_ERROR);
			show_msg((char*) msg);
			return FAILED;
		}
	}
	else
    {
		sprintf(msg, (char*)"gameDATA %s", STR_DISABLED);

#ifdef COBRA_ONLY
		sys_map_path((char*)"/dev_hdd0/game", NULL);
#else
		add_to_map((char*)"/dev_hdd0/game", gamei_path);
#endif
	}

	extgd = status;

	{ PS3MAPI_DISABLE_ACCESS_SYSCALL8 }

	if(do_mount)
    {
		show_msg((char*) msg);
#ifndef COBRA_ONLY
		mount_with_mm(gamei_path, MOUNT_EXT_GDATA);
#endif
	}
	return 0;
}

#endif //#ifdef EXT_GDATA

