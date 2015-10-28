u64 blocked_url[64][2]; u8 url_count = 0;

#ifdef REMOVE_SYSCALLS

static void remove_cfw_syscalls(void)
{
	detect_firmware();

	if(!SYSCALL_TABLE) return;

	u64 sc_null = peekq(SYSCALL_TABLE);
	u64 sc_not_impl_pt = peekq(sc_null);

	get_idps_psid();

	// restore blocked servers
	if(View_Find("game_plugin")==0) {for(u8 u = 0; u<url_count; u++) poke_lv1(blocked_url[u][0], blocked_url[u][1]); url_count = 0;}

	#ifdef COBRA_ONLY
  //{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 8); }
	{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 9); }
	{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 10); }
	{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 11); }
	{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 35); }
	{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 36); }
	{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 38); }
	{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 6); }
	{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 7); }
	{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_PDISABLE_SYSCALL8, 1); }//Partial disable syscall8 (Keep cobra/mamba+ps3mapi features only)
	#endif

	pokeq(SYSCALL_PTR( 6), sc_null); // lv2 peek
	pokeq(SYSCALL_PTR( 7), sc_null); // lv2 poke
	pokeq(SYSCALL_PTR(10), sc_null); // lv2_lv1_call rebug
	pokeq(SYSCALL_PTR(11), sc_null); // lv1 peek Hermes
	pokeq(SYSCALL_PTR(35), sc_null); // Remapper PL3
	pokeq(SYSCALL_PTR(36), sc_null); // Remapper
	pokeq(SYSCALL_PTR(38), sc_null); // new sk1e syscall

	u64 sc6  = peekq(SYSCALL_PTR( 6));
	u64 sc7  = peekq(SYSCALL_PTR( 7));
	u64 sc10 = peekq(SYSCALL_PTR(10));
	u64 sc11 = peekq(SYSCALL_PTR(11));
	u64 sc35 = peekq(SYSCALL_PTR(35));
	u64 sc36 = peekq(SYSCALL_PTR(36));
	u64 sc38 = peekq(SYSCALL_PTR(38));

	if(sc6 !=sc_null) pokeq(sc6,  sc_not_impl_pt);
	if(sc7 !=sc_null) pokeq(sc7,  sc_not_impl_pt);
	if(sc10!=sc_null) pokeq(sc10, sc_not_impl_pt);
	if(sc11!=sc_null) pokeq(sc11, sc_not_impl_pt);
	if(sc35!=sc_null) pokeq(sc35, sc_not_impl_pt);
	if(sc36!=sc_null) pokeq(sc36, sc_not_impl_pt);
	if(sc38!=sc_null) pokeq(sc38, sc_not_impl_pt);

	#ifndef COBRA_ONLY
	pokeq(SYSCALL_PTR( 8), sc_null); // lv1 peek / cobra syscall
	#endif
	pokeq(SYSCALL_PTR( 9), sc_null); // lv1 poke

	#ifndef COBRA_ONLY
	u64 sc8  = peekq(SYSCALL_PTR( 8));
	#endif
	u64 sc9  = peekq(SYSCALL_PTR( 9));

	#ifndef COBRA_ONLY
	if(sc8 !=sc_null) pokeq(sc8,  sc_not_impl_pt);
	#endif
	if(sc9 !=sc_null) pokeq(sc9,  sc_not_impl_pt);

	sc9  = peekq(SYSCALL_PTR( 9));

	bool status = (peekq(0x8000000000003000ULL) == SYSCALLS_UNAVAILABLE || sc9 == sc_null || sc9 == sc_not_impl_pt);

	#ifdef COBRA_ONLY
	if(status && !syscalls_removed)
	{
		CellRtcTick mTick; cellRtcGetCurrentTick(&mTick);
		ps3mapi_key = mTick.tick; for(u16 r = 0; r < (ps3mapi_key & 0xFFF) + 0xF; r++) {ps3mapi_key = ((ps3mapi_key<<15) ^ (ps3mapi_key>>49)) ^ (ps3mapi_key + 1);}
	}
	#endif

	syscalls_removed = status;
}

static void disable_cfw_syscalls(void)
{
#ifdef COBRA_ONLY
	get_vsh_plugin_slot_by_name((char *)"VSH_MENU", true); // unload vsh menu
#endif

	if(syscalls_removed)
	{
		{ BEEP2 }
		show_msg((char*)STR_CFWSYSALRD);
		sys_timer_sleep(2);
	}
	else
	{
		show_msg((char*)STR_CFWSYSRIP);
		remove_cfw_syscalls();
		delete_history(true);

		if(syscalls_removed)
		{
			{ BEEP1 }
			show_msg((char*)STR_RMVCFWSYS);
			sys_timer_sleep(2);
		}
        else
		{
			{ BEEP2 }
			show_msg((char*)STR_RMVCFWSYSF);
			sys_timer_sleep(2);
		}
	}

	if(syscalls_removed) { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_SET_ACCESS_KEY, ps3mapi_key); }
}
#endif

static bool block_url(u64 addr, u64 cur_value, u64 value)
{
	blocked_url[url_count][0]=addr;
	blocked_url[url_count][1]=cur_value;
	url_count++;

	poke_lv1(addr, value);
	return (url_count<64);
}

static void block_online_servers(void)
{
	if(View_Find("game_plugin")) return;

	led(YELLOW, BLINK_FAST);

	u64 mem=0; url_count = 0;

	for(u64 addr=0x860000; addr<0xFFFFF8ULL; addr+=4)//16MB
	{
	 mem=peek_lv1(addr);
	 if(mem         == 0x733A2F2F61757468ULL)  // s://auth
	 	 {if(!block_url(addr, mem,   0x733A2F2F00000000ULL)) break;}
	 else if(mem    == 0x2E7073332E757064ULL)  // .ps3.upd
	 	 {if(!block_url(addr-8, mem, 0x3A2F2F0000000000ULL)) break;}
	 else if(mem    == 0x656E612E6E65742EULL)  // ena.net.
	 	 {if(!block_url(addr, mem,   0x0000000000000000ULL)) break;}
	}
	for(u64 addr=0x300000; addr<0x7FFFF8ULL; addr+=4)//8MB
	{
	 mem=peekq(addr);
	 if(mem      == 0x733A2F2F6E73782EULL)   // s://nsx.
	 	 {if(!block_url(addr+0x1000000ULL,   mem, 0x733A2F2F00000000ULL)) break;}
	 else if(mem == 0x733A2F2F6E73782DULL)   // s://nsx-
	 	 {if(!block_url(addr+0x1000000ULL,   mem, 0x733A2F2F00000000ULL)) break;}
	 else if(mem == 0x3A2F2F786D622D65ULL)   // ://xmb-e
	 	 {if(!block_url(addr+0x1000000ULL,   mem, 0x3A2F2F0000000000ULL)) break;}
	 else if(mem == 0x2E7073332E757064ULL)   // .ps3.upd
	 	 {if(!block_url(addr+0x1000000ULL-8, mem, 0x3A2F2F0000000000ULL)) break;}
	 else if(mem == 0x702E616470726F78ULL)   // p.adprox
	 	 {if(!block_url(addr+0x1000000ULL-8, mem, 0x733A2F2F00000000ULL)) break;}
	 else if(mem == 0x656E612E6E65742EULL)   // ena.net.
	 	 {if(!block_url(addr+0x1000000ULL,   mem, 0x0000000000000000ULL)) break;}
	 else if(mem == 0x702E7374756E2E70ULL)   // p.stun.p
	 	 {if(!block_url(addr+0x1000000ULL-4, mem, 0x0000000000000000ULL)) break;}
	 else if(mem == 0x2E7374756E2E706CULL)   // .stun.pl
	 	 {if(!block_url(addr+0x1000000ULL-4, mem, 0x0000000000000000ULL)) break;}
	 else if(mem == 0x63726565706F2E77ULL)   // creepo.w
	 	 {if(!block_url(addr+0x1000000ULL,   mem, 0x0000000000000000ULL)) break;}
	}

	led(YELLOW, OFF);

	led(GREEN, ON);
}

static void show_idps(char *msg)
{
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