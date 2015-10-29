#ifdef PS2_DISC

static void do_umount_ps2disc(bool mount);
static bool mount_ps2disc(char *path);

static void do_umount_ps2disc(bool mount)
{
	system_call_3(SC_FS_UMOUNT, (u64)(char*)"/dev_ps2disc", 0, 1);

    if(!mount) return;

	{system_call_8(SC_FS_MOUNT, (u64)(char*)"CELL_FS_IOS:BDVD_DRIVE", (u64)(char*)"CELL_FS_ISO9660", (u64)(char*)"/dev_ps2disc", 0, 1, 0, 0, 0);}
}

static bool mount_ps2disc(char *path)
{
	do_umount_ps2disc(true);

	if(!isDir(path)) return false;

 #ifdef COBRA_ONLY
	if(syscalls_removed) { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_REQUEST_ACCESS, ps3mapi_key); }

	sys_map_path((char*)"/dev_ps2disc", path);

	if(syscalls_removed && !is_mounting) { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_SET_ACCESS_KEY, ps3mapi_key); }
 #else

	max_mapped=0;
	add_to_map((char*)"/dev_ps2disc", path);
	add_to_map((char*)"//dev_ps2disc", path);

	u64 map_data  = (MAP_BASE);
	u64 map_paths = (MAP_BASE) + (max_mapped+1) * 0x20;

	for(u16 n=0; n<0x400; n+=8) pokeq(map_data + n, 0);

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
 #endif //#ifdef COBRA_ONLY

	if(!isDir("/dev_ps2disc")) sys_timer_sleep(2);
	if(isDir("/dev_ps2disc")) return true;
	return false;
}

#endif //#ifdef PS2_DISC
