#ifdef REMOVE_SYSCALLS
static void remove_cfw_syscalls(void)
{
	detect_firmware();

	if(!SYSCALL_TABLE) return;

    u64 sc_null = peekq(SYSCALL_TABLE);
	u64 sc_not_impl_pt = peekq(sc_null);

	get_idps_psid();

	#ifndef COBRA_ONLY
	pokeq(SYSCALL_PTR( 8), sc_null); // lv1 peek / cobra syscall
	pokeq(SYSCALL_PTR(35), sc_null); // Remapper PL3
	#endif
	pokeq(SYSCALL_PTR( 9), sc_null); // lv1 poke
	pokeq(SYSCALL_PTR(10), sc_null); // lv2_lv1_call rebug
	pokeq(SYSCALL_PTR(11), sc_null); // lv1 peek Hermes
	pokeq(SYSCALL_PTR(36), sc_null); // Remapper
	pokeq(SYSCALL_PTR(38), sc_null); // new sk1e syscall

	#ifndef COBRA_ONLY
	u64 sc8  = peekq(SYSCALL_PTR( 8));
	u64 sc35 = peekq(SYSCALL_PTR(35));
	#endif
	u64 sc9  = peekq(SYSCALL_PTR( 9));
	u64 sc10 = peekq(SYSCALL_PTR(10));
	u64 sc11 = peekq(SYSCALL_PTR(11));
	u64 sc36 = peekq(SYSCALL_PTR(36));
	u64 sc38 = peekq(SYSCALL_PTR(38));

	#ifndef COBRA_ONLY
	if(sc8 !=sc_null) pokeq(sc8,  sc_not_impl_pt);
	if(sc35!=sc_null) pokeq(sc35, sc_not_impl_pt);
	#endif
	if(sc9 !=sc_null) pokeq(sc9,  sc_not_impl_pt);
	if(sc10!=sc_null) pokeq(sc10, sc_not_impl_pt);
	if(sc11!=sc_null) pokeq(sc11, sc_not_impl_pt);
	if(sc36!=sc_null) pokeq(sc36, sc_not_impl_pt);
	if(sc38!=sc_null) pokeq(sc38, sc_not_impl_pt);

	#ifdef PS3MAPI
  //{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 8); }
	{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 9); }
	{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 10); }
	{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 11); }
  //{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 35); }
	{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 36); }
	{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 38); }
	{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 6); }
	{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 7); }
	{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_PDISABLE_SYSCALL8, 1); }//Partial disable syscall8 (Keep cobra/mamba+ps3mapi features only)
	#endif

	pokeq(SYSCALL_PTR( 6), sc_null); // lv2 peek
	pokeq(SYSCALL_PTR( 7), sc_null); // lv2 poke

	u64 sc6  = peekq(SYSCALL_PTR( 6));
	u64 sc7  = peekq(SYSCALL_PTR( 7));

	if(sc6 !=sc_null) pokeq(sc6,  sc_not_impl_pt);
	if(sc7 !=sc_null) pokeq(sc7,  sc_not_impl_pt);
}

static void disable_cfw_syscalls(void)
{
#ifdef COBRA_ONLY
	get_vsh_plugin_slot_by_name((char *)"VSH_MENU", true); // unload vsh menu
#endif

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

static void block_online_servers(void)
{
	led(YELLOW, BLINK_FAST);

	u64 mem=0;
	for(u64 addr=0x860000; addr<0xFFFFF8ULL; addr+=4)//16MB
	{
	 mem=peek_lv1(addr);
	 if(mem         == 0x733A2F2F61757468ULL)  // s://auth
	  poke_lv1(addr,   0x733A2F2F00000000ULL);
	 else if(mem    == 0x2E7073332E757064ULL)  // .ps3.upd
	  poke_lv1(addr-8, 0x3A2F2F0000000000ULL);
	 else if(mem    == 0x656E612E6E65742EULL)  // ena.net.
	  poke_lv1(addr,   0x0000000000000000ULL);
	}
	for(u64 addr=0x300000; addr<0x7FFFF8ULL; addr+=4)//8MB
	{
	 mem=peekq(addr);
	 if(mem      == 0x733A2F2F6E73782EULL)  // s://nsx.
	  pokeq(addr,   0x733A2F2F00000000ULL);
	 else if(mem == 0x733A2F2F6E73782DULL)  // s://nsx-
	  pokeq(addr,   0x733A2F2F00000000ULL);
	 else if(mem == 0x3A2F2F786D622D65ULL)  // ://xmb-e
	  pokeq(addr,   0x3A2F2F0000000000ULL);
	 else if(mem == 0x2E7073332E757064ULL)  // .ps3.upd
	  pokeq(addr-8, 0x3A2F2F0000000000ULL);
	 else if(mem == 0x702E616470726F78ULL)  // p.adprox
	  pokeq(addr-8, 0x733A2F2F00000000ULL);
	 else if(mem == 0x656E612E6E65742EULL)  // ena.net.
	  pokeq(addr,   0x0000000000000000ULL);
	 else if(mem == 0x702E7374756E2E70ULL)  // p.stun.p
	  pokeq(addr-4, 0x0000000000000000ULL);
	 else if(mem == 0x2E7374756E2E706CULL)  // .stun.pl
	  pokeq(addr-4, 0x0000000000000000ULL);
	 else if(mem == 0x63726565706F2E77ULL)  // creepo.w
	  pokeq(addr  , 0x0000000000000000ULL);
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