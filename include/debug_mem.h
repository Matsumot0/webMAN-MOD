#ifdef DEBUG_MEM

#define LV1_UPPER_MEMORY	0x8000000010000000ULL
#define LV2_UPPER_MEMORY	0x8000000000800000ULL

static void dump_mem(char *file, uint64_t start, uint32_t size_mb);

static void peek_chunk(uint64_t start, uint64_t size, uint8_t* buf) // read from lv1
{
	uint64_t i = 0, t = 0;
	for(i = 0; i < size; i += 8)
	{
		t = peek_lv1(start + i); memcpy(buf + i, &t, 8);
	}
}

static void dump_mem(char *file, uint64_t start, uint32_t size_mb)
{
#ifdef COBRA_ONLY
	if(syscalls_removed) { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_REQUEST_ACCESS, ps3mapi_key); }
#endif

	int fp;
	uint64_t sw;
	uint32_t mem_size = (_128KB_), i;
	sys_addr_t sys_mem = 0;

    if(start < 0x0000028080000000ULL) start |= 0x8000000000000000ULL;

	if(sys_memory_allocate(mem_size, SYS_MEMORY_PAGE_SIZE_64K, &sys_mem)==0)
	{
		uint8_t *mem_buf	= (uint8_t*)sys_mem;

		if(cellFsOpen((char*)file, CELL_FS_O_CREAT | CELL_FS_O_TRUNC | CELL_FS_O_WRONLY, &fp, NULL, 0)==CELL_FS_SUCCEEDED)
		{
			for(i = 0; i < size_mb*8UL; i++)
			{
				peek_chunk(start + (i * mem_size), mem_size, mem_buf);
				cellFsWrite(fp, mem_buf, mem_size, &sw);
			}
			cellFsClose(fp);
		}
		sys_memory_free((sys_addr_t)sys_mem);
		show_msg((char*)"Memory dump completed!");
		{ BEEP2 }
	}

#ifdef COBRA_ONLY
	if(syscalls_removed && !is_mounting) { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_SET_ACCESS_KEY, ps3mapi_key); }
#endif
}

static void ps3mapi_mem_dump(char *buffer, char *templn, char *param)
{
	char dump_file[MAX_PATH_LEN]; uint64_t start=0; uint32_t size=8;
	strcat(buffer, "Dump: [<a href=\"/dump.ps3?mem\">Full Memory</a>] [<a href=\"/dump.ps3?lv1\">LV1</a>] [<a href=\"/dump.ps3?lv2\">LV2</a>]<hr>");

	if(strlen(param+10))
	{
		if(strstr(param,"?lv1")        ) {size=16;} else
		if(strstr(param,"?lv2")        ) {start=0x1000000;} else
		//if(strstr(param,"?v") /*vsh  */) {start=0x910000;}  else
		if(strstr(param,"?r") /*rsx  */) {start=0x0000028080000000ULL; size=256;}  else
		if(strstr(param,"?f") /*full */) {size=256;} else
		if(strstr(param,"?m") /*mem  */) {size=256;} else
		{
			start = convertH(param+10);
			if(start>=LV1_UPPER_MEMORY-((uint64_t)(size*_1MB_))) start=LV1_UPPER_MEMORY-((uint64_t)(size*_1MB_));
		}

		char *pos=strstr(param, "&size=");
		if(pos) size = convertH(pos+6);

		sprintf(dump_file, "/dev_hdd0/dump_%s.bin", param+10);
		dump_mem(dump_file, start, size);
		sprintf(templn, "<p>Dumped: <a href=\"%s\">%s</a> [<a href=\"/delete.ps3%s\">%s</a>]", dump_file, dump_file+10, dump_file, STR_DELETE); strcat(buffer, templn);
	}
}

static void ps3mapi_find_peek_poke(char *buffer, char *templn, char *param)
{
	uint64_t address, addr, fvalue, value=0, upper_memory=LV2_UPPER_MEMORY, found_address=0;
	u8 byte=0, p=0, lv1=0;
	bool bits8=false, bits16=false, bits32=false, found=false;
	u8 flen=0;
	char *v;

	v=strstr(param+10,"&");
	if(v) v=NULL;

	address = convertH(param+10);

	v=strstr(param+10, "=");
	if(v)
	{
		flen=strlen(v+1);
		for(p=1; p<=flen;p++) if(!memcmp(v+p," ",1)) byte++; //ignore spaces
		flen-=byte; byte=p=0;
	}

	bits32=(flen>4) && (flen<=8);
	bits16=(flen>2) && (flen<=4);
	bits8 =(flen<=2);

	strcat(buffer, "<pre>");

	address|=0x8000000000000000ULL;

	lv1=strstr(param,".lv1?")?1:0;
	upper_memory=(lv1?LV1_UPPER_MEMORY:LV2_UPPER_MEMORY)-8;

	if(v!=NULL && strstr(param, "find.lv") && (address<upper_memory))
	{
		uint64_t j;
		fvalue = convertH(v+1);

		if(bits8)  fvalue=(fvalue<<56);
		if(bits16) fvalue=(fvalue<<48);
		if(bits32) fvalue=(fvalue<<32);

		if(lv1)
			for(j = address; j < upper_memory; j++)
			{
				value = peek_lv1(j);

				if(bits32) value&=0xffffffff00000000ULL; else
				if(bits16) value&=0xffff000000000000ULL; else
				if(bits8 ) value&=0xff00000000000000ULL;

				if(value==fvalue) {found=true; break;}
			}
		else
			for(j = address; j < upper_memory; j++)
			{
				value = peekq(j);

				if(bits32) value&=0xffffffff00000000ULL; else
				if(bits16) value&=0xffff000000000000ULL; else
				if(bits8 ) value&=0xff00000000000000ULL;

				if(value==fvalue) {found=true; break;}
			}

		if(!found)
		{
			sprintf(templn, "<b><font color=red>%s</font></b><br>", STR_NOTFOUND); strcat(buffer, templn);
		}
		else
		{
			found_address=address=j;
			sprintf(templn, "Offset: 0x%X<br><br>", (u32)address); strcat(buffer, templn);
		}
	}
	else
	if(v!=NULL && strstr(param, "poke.lv2") && (address<upper_memory))
    {
		value  = convertH(v+1);
		fvalue = peekq(address);

		if(bits32) value = ((uint64_t)(value<<32) | (uint64_t)(fvalue & 0xffffffffULL)); else
		if(bits16) value = ((uint64_t)(value<<48) | (uint64_t)(fvalue & 0xffffffffffffULL)); else
		if(bits8)  value = ((uint64_t)(value<<56) | (uint64_t)(fvalue & 0xffffffffffffffULL));

		pokeq(address, value);
		found_address=address; found=true;
	}
	else
	if(v!=NULL && strstr(param, "poke.lv1") && (address<upper_memory))
    {
		value = convertH(v+1);
		fvalue = peek_lv1(address);

		if(bits32) value = ((uint64_t)(value<<32) | (uint64_t)(fvalue & 0xffffffffULL)); else
		if(bits16) value = ((uint64_t)(value<<48) | (uint64_t)(fvalue & 0xffffffffffffULL)); else
		if(bits8)  value = ((uint64_t)(value<<56) | (uint64_t)(fvalue & 0xffffffffffffffULL));

		poke_lv1(address, value);
		found_address=address; found=true;
	}

	if(address+0x200>(upper_memory+8)) address=0;

	flen=(bits8)?1:(bits16)?2:(bits32)?4:8;
	address&=0xFFFFFFFFFFFFFFF0ULL;
	addr=address;

	for(u16 i=0; i<0x200; i++)
	{
		if(!p)
		{
			sprintf(templn, "%X  ", (int)((address & 0xFFFFFFFFULL) +i));
			for(u8 c=10-strlen(templn);c>0;c--) strcat(buffer, "0");
			strcat(buffer, templn);
		}

		byte=(u8)((lv1?peek_lv1(address+i):peekq(address+i))>>56);

		if(found && address+i>=found_address && address+i<found_address+flen) strcat(buffer, "<font color=yellow><b>");
		sprintf(templn, byte<16?"0%X ":"%X ", byte); strcat(buffer, templn);
		if(found && address+i>=found_address && address+i<found_address+flen) strcat(buffer, "</b></font>");

		if(p==0x7) strcat(buffer, " ");

		if(p==0xF)
		{
			strcat(buffer, " ");
			for(u8 c=0;c<0x10;c++, addr++)
			{
				byte=(u8)((lv1?peek_lv1(addr):peekq(addr))>>56);
				if(byte<32 || byte>=127) byte='.';

				if(found && addr>=found_address && addr<found_address+flen) strcat(buffer, "<font color=yellow><b>");
				if(byte==0x3C)
					strcat(buffer, "&lt;");
				else if(byte==0x3E)
					strcat(buffer, "&gt;");
				else
					{sprintf(templn,"%c", byte); strcat(buffer, templn);}
				if(found && addr>=found_address && addr<found_address+flen) strcat(buffer, "</b></font>");
			}
			strcat(buffer, "<br>");
		}

		p++; if(p>=0x10) p=0;
	}
	strcat(buffer, "<hr>Dump: [<a href=\"/dump.ps3?mem\">Full Memory</a>] [<a href=\"/dump.ps3?lv1\">LV1</a>] [<a href=\"/dump.ps3?lv2\">LV2</a>]");
	sprintf(templn, " [<a href=\"/dump.ps3?%llx\">Dump 0x%llx</a>]", address, address); strcat(buffer, templn);
	sprintf(templn, " <a href=\"/peek.lv%i?%llx\">&lt;&lt;</a> <a href=\"/peek.lv%i?%llx\">&lt;Back</a>", lv1?1:2, ((int)(address-0x1000)>=0)?(address-0x1000):0, lv1?1:2, ((int)(address-0x200)>=0)?(address-0x200):0); strcat(buffer, templn);
	sprintf(templn, " <a href=\"/peek.lv%i?%llx\">Next&gt;</a> <a href=\"/peek.lv%i?%llx\">&gt;&gt;</a></pre>", lv1?1:2, ((int)(address+0x400)<(int)upper_memory)?(address+0x200):(upper_memory-0x200), lv1?1:2, ((int)(lv1+0x1200)<(int)upper_memory)?(address+0x1000):(upper_memory-0x200)); strcat(buffer, templn);
}

#endif
