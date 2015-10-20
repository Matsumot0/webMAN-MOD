#ifdef REX_ONLY
static bool toggle_rebug_mode(void)
{
	struct CellFsStat s;
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

		return true; // vsh reboot
	}
	else
	if((cellFsStat((char*) VSH_MODULE_PATH "vsh.self.nrm", &s)==CELL_FS_SUCCEEDED)
	&& (cellFsStat((char*) VSH_MODULE_PATH "vsh.self.cexsp", &s)==CELL_FS_SUCCEEDED))
	{
		show_msg((char*)"REBUG Mode Debug XMB detected!\r\nSwitch to Retail XMB...");
		sys_timer_sleep(3);

		cellFsRename(VSH_MODULE_PATH "vsh.self", VSH_MODULE_PATH "vsh.self.dexsp");
		cellFsRename(VSH_MODULE_PATH "vsh.self.cexsp", VSH_MODULE_PATH "vsh.self");

		return true; // vsh reboot
	}
	else
	if(cellFsStat((char*) VSH_MODULE_PATH "vsh.self.dexsp", &s)==CELL_FS_SUCCEEDED)
	{
		show_msg((char*)"REBUG Mode Retail XMB detected!\r\nSwitch to Debug XMB...");
		sys_timer_sleep(3);

		cellFsRename(VSH_MODULE_PATH "vsh.self", VSH_MODULE_PATH "vsh.self.cexsp");
		cellFsRename(VSH_MODULE_PATH "vsh.self.dexsp", VSH_MODULE_PATH "vsh.self");

		return true; // vsh reboot
	}
	return false;
}

static bool toggle_normal_mode(void)
{
	struct CellFsStat s;
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

		return true; // vsh reboot
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

		return true; // vsh reboot
	}
	else
	if(cellFsStat((char*) VSH_MODULE_PATH "vsh.self.swp", &s)==CELL_FS_SUCCEEDED)
	{
		show_msg((char*)"Normal Mode detected!\r\nNo need to switch!");
		sys_timer_sleep(3);
		{system_call_3(SC_FS_UMOUNT, (u64)(char*)"/dev_blind", 0, 1);}
	}
	return false;
}

static void toggle_debug_menu(void)
{
	struct CellFsStat s;
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
#endif

#ifdef COBRA_ONLY
static bool toggle_cobra(void)
{
	struct CellFsStat s;
	enable_dev_blind((char*)"COBRA Toggle activated!");
 #ifdef REX_ONLY
	if( (cellFsStat((char*) REBUG_COBRA_PATH "stage2.cex", &s)==CELL_FS_SUCCEEDED) /* &&
		(cellFsStat((char*) REBUG_COBRA_PATH "stage2.dex", &s)==CELL_FS_SUCCEEDED) */)
	{
		show_msg((char*)"REBUG COBRA is active!\r\nDeactivating COBRA...");

		cellFsRename(REBUG_COBRA_PATH "stage2.cex", REBUG_COBRA_PATH "stage2.cex.bak");
		cellFsRename(REBUG_COBRA_PATH "stage2.dex", REBUG_COBRA_PATH "stage2.dex.bak");
		return true; // vsh reboot
	}
	else if((cellFsStat((char*) REBUG_COBRA_PATH "stage2.cex.bak", &s)==CELL_FS_SUCCEEDED) /* &&
			(cellFsStat((char*) REBUG_COBRA_PATH "stage2.dex.bak", &s)==CELL_FS_SUCCEEDED) */)
	{
		show_msg((char*)"REBUG COBRA is inactive!\r\nActivating COBRA...");

		cellFsRename(REBUG_COBRA_PATH "stage2.cex.bak", REBUG_COBRA_PATH "stage2.cex");
		cellFsRename(REBUG_COBRA_PATH "stage2.dex.bak", REBUG_COBRA_PATH "stage2.dex");
		return true; // vsh reboot
	}
 #else
	if(cellFsStat((char*)HABIB_COBRA_PATH "stage2.cex", &s)==CELL_FS_SUCCEEDED)
	{
		show_msg((char*)"COBRA is active!\r\nDeactivating COBRA...");

		cellFsRename(HABIB_COBRA_PATH "stage2.cex", HABIB_COBRA_PATH "stage2_disabled.cex");

		return true; // vsh reboot
	}
	else if(cellFsStat((char*)HABIB_COBRA_PATH "stage2_disabled.cex", &s)==CELL_FS_SUCCEEDED)
	{
		show_msg((char*)"COBRA is inactive!\r\nActivating COBRA...");

		cellFsRename(HABIB_COBRA_PATH "stage2_disabled.cex", HABIB_COBRA_PATH "stage2.cex");

		return true; // vsh reboot
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

		return true; // vsh reboot
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

		return true; // vsh reboot
	}
 #endif //#ifdef REX_ONLY
	return false;
}
#endif //#ifdef COBRA_ONLY

#ifdef COBRA_ONLY
static void toggle_ps2emu(void)
{
	struct CellFsStat s;
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