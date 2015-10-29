#define SC_GET_IDPS 					(870)
#define SC_GET_PSID 					(872)

uint64_t idps_offset1=0;
uint64_t idps_offset2=0;
uint64_t psid_offset=0;

uint64_t IDPS[2] = {0, 0};
uint64_t PSID[2] = {0, 0};

static void get_idps_psid(void);
static void get_idps_psid(void)
{
#ifdef COBRA_ONLY
	if(syscalls_removed) { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_REQUEST_ACCESS, ps3mapi_key); }
#endif

	if(c_firmware<=4.53f)
	{
		{system_call_1(SC_GET_IDPS, (uint64_t) IDPS);}
		{system_call_1(SC_GET_PSID, (uint64_t) PSID);}
	}
	else if(peekq(0x8000000000003000ULL)==SYSCALLS_UNAVAILABLE)
		return; // do not update IDPS/PSID if syscalls are removed
	else if(idps_offset2 | psid_offset)
	{
			IDPS[0] = peekq(idps_offset2  );
			IDPS[1] = peekq(idps_offset2+8);
			PSID[0] = peekq(psid_offset   );
			PSID[1] = peekq(psid_offset +8);
	}

#ifdef COBRA_ONLY
	if(syscalls_removed && !is_mounting) { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_SET_ACCESS_KEY, ps3mapi_key); }
#endif
}

#ifdef SPOOF_CONSOLEID
static void spoof_idps_psid(void);
static void spoof_idps_psid(void)
{
#ifdef COBRA_ONLY
	if(syscalls_removed) { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_REQUEST_ACCESS, ps3mapi_key); }
#endif

	if(webman_config->spsid)
	{
		uint64_t j, newPSID[2] = {0, 0};

		newPSID[0] = convertH(webman_config->vPSID1);
		newPSID[1] = convertH(webman_config->vPSID2);

		//if(newPSID[0] != 0 && newPSID[1] != 0)
		{
			if(c_firmware<=4.53f)
			{
				{system_call_1(SC_GET_PSID, (uint64_t) PSID);}
				for(j = 0x8000000000300000ULL; j < 0x8000000000600000ULL; j+=4) {
					if((peekq(j) == PSID[0]) && (peekq(j+8) == PSID[1])) {
						pokeq(j, newPSID[0]); j+=8;
						pokeq(j, newPSID[1]); j+=8;
					}
				}
			}
			else if(psid_offset)
			{
				pokeq(psid_offset  , newPSID[0]);
				pokeq(psid_offset+8, newPSID[1]);
			}
		}
	}

	if(webman_config->sidps)
	{
		uint64_t j, newIDPS[2] = {0, 0};

		newIDPS[0] = convertH(webman_config->vIDPS1);
		newIDPS[1] = convertH(webman_config->vIDPS2);

		if(newIDPS[0] != 0 && newIDPS[1] != 0)
		{
			if(c_firmware<=4.53f)
			{
				{system_call_1(SC_GET_IDPS, (uint64_t) IDPS);}
				for(j = 0x8000000000300000ULL; j < 0x8000000000600000ULL; j+=4)
				{
					if((peekq(j) == IDPS[0]) && (peekq(j+8) == IDPS[1]))
					{
						pokeq(j, newIDPS[0]); j+=8;
						pokeq(j, newIDPS[1]); j+=8;
					}
				}
			}
			else if(idps_offset1 | idps_offset2)
			{
				pokeq(idps_offset1  , newIDPS[0]);
				pokeq(idps_offset1+8, newIDPS[1]);
				pokeq(idps_offset2  , newIDPS[0]);
				pokeq(idps_offset2+8, newIDPS[1]);
			}
		}
	}

	get_idps_psid();
}
#endif
