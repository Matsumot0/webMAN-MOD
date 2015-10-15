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
#endif

static void block_online_servers(void)
{
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
}
