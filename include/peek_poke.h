#define SC_PEEK_LV2 					(6)
#define SC_POKE_LV2 					(7)
#define SC_PEEK_LV1 					(8)
#define SC_POKE_LV1 					(9)

#define SYSCALL8_OPCODE_PS3MAPI			0x7777
#define PS3MAPI_OPCODE_LV1_POKE			0x1009

static inline uint64_t peekq(uint64_t addr);
static void pokeq( uint64_t addr, uint64_t val);
static inline uint64_t peek_lv1(uint64_t addr);
static void poke_lv1( uint64_t addr, uint64_t val);

static void lv2poke32(u64 addr, u32 value);

static uint64_t convertH(char *val);

/*
static u32 lv2peek32(u64 addr);
static u32 lv2peek32(u64 addr)
{
    u32 ret = (u32) (peekq(addr) >> 32ULL);
    return ret;
}
*/

static void lv2poke32(u64 addr, u32 value)
{
	pokeq(addr, (((u64) value) <<32) | (peekq(addr) & 0xffffffffULL));
}

static uint64_t peek_lv1(uint64_t addr)
{
	system_call_1(SC_PEEK_LV1, (uint64_t) addr);
	return (uint64_t) p1;
}

static void poke_lv1( uint64_t addr, uint64_t value)
{
	if(!syscalls_removed)
		{system_call_2(SC_POKE_LV1, addr, value);}
	else
		{system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_LV1_POKE, addr, value);} // use ps3mapi extension to support poke lv1
}

static inline uint64_t peekq(uint64_t addr) //lv2
{
	system_call_1(SC_PEEK_LV1, addr + 0x1000000ULL); //old: system_call_1(SC_PEEK_LV2, addr);
	return (uint64_t) p1;
}

static void pokeq( uint64_t addr, uint64_t value) //lv2
{
	if(!syscalls_removed)
		{system_call_2(SC_POKE_LV1, addr + 0x1000000ULL, value);} //old: system_call_2(SC_POKE_LV2, addr, val);
	else
		{system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_LV1_POKE, addr + 0x1000000ULL, value);} // use ps3mapi extension to support poke lv2
}

static uint64_t convertH(char *val)
{
	uint64_t ret=0;
	uint8_t buff, i, n=0;

	for(i = 0; i < 16+n; i++)
	{
		if(val[i]==' ') {n++; continue;}

		if(val[i]>='0' && val[i]<='9') buff=(   val[i]-'0')*0x10; else
		if(val[i]>='A' && val[i]<='F') buff=(10+val[i]-'A')*0x10; else
		if(val[i]>='a' && val[i]<='f') buff=(10+val[i]-'a')*0x10; else
		return ret;

		i++;
		if(val[i]>='0' && val[i]<='9') buff+=(   val[i]-'0'); else
		if(val[i]>='A' && val[i]<='F') buff+=(10+val[i]-'A'); else
		if(val[i]>='a' && val[i]<='f') buff+=(10+val[i]-'a'); else
		{ret=(ret<<4)+(buff>>4); return ret;}

		ret = (ret << 8) | buff;
	}

	return ret;
}

#ifdef GET_KLICENSEE
static char *hex_dump(char *buffer, int offset, int size)
{
	for (int k = 0; k < size ; k++)
	{
		sprintf(&buffer[2 * k],"%02X", (unsigned int)(((unsigned char*)offset)[k]));
	}
	return buffer;
}
#endif

/*
s32 lv2_get_platform_info(struct platform_info *info)
{
	system_call_1(SC_GET_PLATFORM_INFO, (uint64_t) info);
	return_to_user_prog(s32);
}

s32 lv2_get_target_type(uint64_t *type)
{
	lv2syscall1(985, (uint64_t) type);
	return_to_user_prog(s32);
}

uint64_t find_syscall_table()
{
	uint64_t targettype;
	lv2_get_target_type(&targettype);

	for(uint64_t i = 0x8000000000340000ULL; i<0x8000000000400000ULL; i+=4)
    {
		if(peekq(i) == 0x3235352E3235352EULL) return (i + (targettype == 2) ? 0x1228 : 0x1220);
	}
	return 0;
}
*/
