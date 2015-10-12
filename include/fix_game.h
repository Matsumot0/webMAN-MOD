static bool fix_in_progress = false;
static bool fix_aborted = false;

enum FIX_GAME_MODES
{
	FIX_GAME_AUTO,
	FIX_GAME_QUICK,
	FIX_GAME_FORCED,
	FIX_GAME_DISABLED
};

#define READ_SFO_HEADER() \
	if(!(mem[1]=='P' && mem[2]=='S' && mem[3]=='F')) return false; \
	u16 pos, str, dat, indx=0; \
	str=(mem[0x8]+(mem[0x9]<<8)); \
	dat=pos=(mem[0xc]+(mem[0xd]<<8));

#define READ_SFO_HEADER2() \
	if(!(mem[1]=='P' && mem[2]=='S' && mem[3]=='F')) return; \
	u16 pos, str, dat, indx=0; \
	str=(mem[0x8]+(mem[0x9]<<8)); \
	dat=pos=(mem[0xc]+(mem[0xd]<<8));

#define FOR_EACH_SFO_FIELD() \
	while(str<4090) \
	{ \
		if((mem[str]==0) || (str>=dat)) break;

#define READ_NEXT_SFO_FIELD() \
		while(mem[str]) str++;str++; \
		pos+=(mem[0x1c+indx]+(mem[0x1d+indx]<<8)); \
		indx+=0x10; \
	}

static void parse_param_sfo(unsigned char *mem, char *titleID, char *title)
{
	READ_SFO_HEADER2()

	memset(titleID, 0, 10);
	memset(title, 0, 64);

	u8 fcount=0;

	FOR_EACH_SFO_FIELD()
	{
		if(!memcmp((char *) &mem[str], "TITLE_ID", 8))
		{
			strncpy(titleID, (char *) &mem[pos], 9);
			fcount++; if(fcount>=2) break;
		}
		else
		if(!memcmp((char *) &mem[str], "TITLE", 6))
		{
			strncpy(title, (char *) &mem[pos], 63);
			fcount++; if(fcount>=2) break;
		}

		READ_NEXT_SFO_FIELD()
	}

	if(webman_config->tid && strlen(titleID)==9 && (titleID[0]=='B' || titleID[0]=='N'))
	{
		strcat(title, " ["); strcat(title, titleID); strcat(title, "]");
	}
}

static bool fix_param_sfo(unsigned char *mem, char *titleID, u8 msg)
{
	READ_SFO_HEADER()

	memset(titleID, 0, 10);

#ifdef FIX_GAME
	u8 fcount=0;
#endif

	bool ret=false;

	FOR_EACH_SFO_FIELD()
	{
		if(!memcmp((char *) &mem[str], "TITLE_ID", 8))
		{
			strncpy(titleID, (char *) &mem[pos], 9);
#ifdef FIX_GAME
			fcount++; if(fcount>=2) break;
#else
			break;
#endif
		}
#ifdef FIX_GAME
		else
		if(!memcmp((char *) &mem[str], "PS3_SYSTEM_VER", 14))
		{
			char version[8];
			strncpy(version, (char *) &mem[pos], 7);
			int fw_ver=10000*((version[1] & 0xFF)-'0') + 1000*((version[3] & 0xFF)-'0') + 100*((version[4] & 0xFF)-'0');
			if((fw_ver>(int)(c_firmware*10000.0f)) && c_firmware>=4.20f && c_firmware<4.76f)
			{
				if(msg) {char text[64]; sprintf(text, "WARNING: Game requires firmware version %i.%i", (fw_ver/10000), (fw_ver-10000*(fw_ver/10000))/100); show_msg((char*)text);}

				mem[pos+1]='4'; mem[pos+3]='2'; mem[pos+4]='0'; ret=true;
			}
			fcount++; if(fcount>=2) break;
		}
#endif

		READ_NEXT_SFO_FIELD()
	}

	return ret;
}

#ifdef FIX_GAME
static bool fix_ps3_extra(unsigned char *mem)
{
	READ_SFO_HEADER()

	FOR_EACH_SFO_FIELD()
	{
		if(!memcmp((char *) &mem[str], "ATTRIBUTE", 9))
		{
			if(!(mem[pos+2] & 2)) {mem[pos+2]|=0x2; return true;}
			break;
		}

		READ_NEXT_SFO_FIELD()
	}

	return false;
}

char fix_game_path[7][256]; int plevel=-1;

static void fix_game(char *path)
{
	int fd; if(plevel>=6) return; // limit recursion up to 6 levels

	if(cellFsOpendir(path, &fd) == CELL_FS_SUCCEEDED)
	{
		plevel++;
		CellFsDirent dir; u64 read = sizeof(CellFsDirent); struct CellFsStat s;

#ifdef COPY_PS3
		sprintf(current_file, "%s", path);
#endif

		while(cellFsReaddir(fd, &dir, &read)==CELL_FS_SUCCEEDED)
		{
			if(!read || fix_aborted) break;
			if(dir.d_name[0]=='.') continue;

			sprintf(fix_game_path[plevel], "%s/%s", path, dir.d_name);

			if(!extcasecmp(dir.d_name, ".sprx", 5) || !extcasecmp(dir.d_name, ".self", 5) || !strcmp(dir.d_name, "EBOOT.BIN"))
			{

				if(cellFsStat(fix_game_path[plevel], &s)!=CELL_FS_SUCCEEDED || s.st_size < 0x500) continue;

				int fdw, offset; uint64_t msiz = 0; char ps3_sys_version[8];

				cellFsChmod(fix_game_path[plevel], MODE); //fix file read-write permission

				if(cellFsOpen(fix_game_path[plevel], CELL_FS_O_RDWR, &fdw, NULL, 0)==CELL_FS_SUCCEEDED)
				{
					cellFsLseek(fdw, 0xC, CELL_FS_SEEK_SET, &msiz);
					cellFsRead(fdw, (void *)&ps3_sys_version, 4, &msiz);

					offset=(ps3_sys_version[0]<<24) + (ps3_sys_version[1]<<16) + (ps3_sys_version[2]<<8) + ps3_sys_version[3]; offset-=0x78;

					retry_offset:
					if(offset < 0x90 || offset > 0x800) offset=!extcasecmp(dir.d_name, ".sprx", 5)?0x258:0x428;
					cellFsLseek(fdw, offset, CELL_FS_SEEK_SET, &msiz);
					cellFsRead(fdw, (void *)&ps3_sys_version, 8, &msiz);

					if(offset!=0x278 && offset!=0x428 && (ps3_sys_version[0] | ps3_sys_version[1] | ps3_sys_version[2] | ps3_sys_version[3] | ps3_sys_version[4] | ps3_sys_version[5])!=0)
					{
						offset=(offset==0x258) ? 0x278 : 0; goto retry_offset;
					}

					if((ps3_sys_version[0]+ps3_sys_version[1]+ps3_sys_version[2]+ps3_sys_version[3]+ps3_sys_version[4]+ps3_sys_version[5])==0 && (ps3_sys_version[6] & 0xFF)>0xA4)
					{
						ps3_sys_version[6]=0XA4; ps3_sys_version[7]=0X10;

						cellFsLseek(fdw, offset, CELL_FS_SEEK_SET, &msiz);
						cellFsWrite(fdw, (char*)ps3_sys_version, 8, NULL);
					}
					cellFsClose(fdw);
				}
				cellFsClose(fdw);
			}
			else if(isDir(fix_game_path[plevel]) && (webman_config->fixgame!=FIX_GAME_QUICK)) fix_game(fix_game_path[plevel]);

			sys_timer_usleep(1);
		}

		cellFsClosedir(fd);
		plevel--;
	}
}

#ifdef COBRA_ONLY
uint64_t getlba(const char *s1, u16 n1, const char *s2, u16 n2, u16 start);
void fix_iso(char *iso_file, uint64_t maxbytes, bool patch_update);

uint64_t getlba(const char *s1, u16 n1, const char *s2, u16 n2, u16 start)
{
    u16 c=0; u32 lba=0;
    for(u16 n=start+0x1F; n<n1-n2; n++)
    {
        c=0; while(s1[n+c]==s2[c] && c<n2) c++;
        if(c==n2)
        {
            while(n>0x1D && s1[n--]!=0x01); n-=0x1C;
            lba=(s1[n+0]&0xFF)+(s1[n+1]&0xFF)*0x100UL+(s1[n+2]&0xFF)*0x10000UL+(s1[n+3]&0xFF)*0x1000000UL;
            start=n+0x1C+n2; return lba;
        }
    }
    return 0;
}

void fix_iso(char *iso_file, uint64_t maxbytes, bool patch_update)
{
	struct CellFsStat buf;

	if(fix_aborted || cellFsStat(iso_file, &buf)!=CELL_FS_SUCCEEDED) return;

	int fd;

#ifdef COPY_PS3
	sprintf(current_file, "%s", iso_file);
#endif

	cellFsChmod(iso_file, MODE); //fix file read-write permission

	if(cellFsOpen((char*)iso_file, CELL_FS_O_RDWR, &fd, NULL, 0)==CELL_FS_SUCCEEDED)
	{
		uint64_t chunk_size=_4KB_; char chunk[_4KB_], ps3_sys_version[8];
		uint64_t msiz1 = 0, msiz2 = 0, lba = 0, pos=0xA000ULL;

		bool fix_sfo=true, fix_eboot=true, fix_ver=false, fix_update=false; char update_path[MAX_PATH_LEN];

		uint64_t size = buf.st_size; char titleID[10];
		if(maxbytes>0 && size>maxbytes) size=maxbytes;
		if(size>pos) size-=pos; else size=0;

		while(size>0ULL)
    	{
			if(fix_aborted) break;

			if(fix_sfo)
			{
				cellFsLseek(fd, pos, CELL_FS_SEEK_SET, &msiz2);
				cellFsRead(fd, chunk, chunk_size, &msiz1); if(msiz1<=0) break;

				lba=getlba(chunk, msiz1, "PARAM.SFO;1", 11, 0);

				if(lba)
				{
					lba*=0x800ULL; fix_sfo=false;
					cellFsLseek(fd, lba, CELL_FS_SEEK_SET, &msiz2);
					cellFsRead(fd, (void *)&chunk, chunk_size, &msiz1); if(msiz1<=0) break;

					fix_ver = fix_param_sfo((unsigned char *)chunk, titleID, 0);

					if(patch_update)
					{
						sprintf(update_path, "/dev_hdd0/game/%s/USRDIR/EBOOT.BIN", titleID); // has update on hdd0?
						if(cellFsStat(update_path, &buf)==CELL_FS_SUCCEEDED) {fix_update=true; fix_ver=false;}
					}

					if(fix_ver)
					{
						cellFsLseek(fd, lba, CELL_FS_SEEK_SET, &msiz2);
						cellFsWrite(fd, chunk, msiz1, &msiz2);
					}
					else goto exit_fix; //do not fix if sfo version is ok

					if(size>lba) size=lba;

					sprintf(chunk, "%s %s", STR_FIXING, iso_file);
					show_msg(chunk);

					lba=getlba(chunk, msiz1, "PS3_DISC.SFB;1", 14, 0); lba*=0x800ULL; chunk_size=0x800; //1 sector
					if(lba>0 && size>lba) size=lba;
				}
			}

			u16 start, offset;

			for(u8 t=(fix_eboot?0:1);t<5;t++)
			{
				cellFsLseek(fd, pos, CELL_FS_SEEK_SET, &msiz2);
				cellFsRead(fd, chunk, chunk_size, &msiz1); if(msiz1<=0) break;

				start=0;

				while(true)
				{
					sys_timer_usleep(1);
					if(fix_aborted) goto exit_fix;

					if(t==0) lba=getlba(chunk, msiz1, "EBOOT.BIN;1", 11, start);
					if(t==1) lba=getlba(chunk, msiz1, ".SELF;1", 7, start);
					if(t==2) lba=getlba(chunk, msiz1, ".self;1", 7, start);
					if(t==3) lba=getlba(chunk, msiz1, ".SPRX;1", 7, start);
					if(t==4) lba=getlba(chunk, msiz1, ".sprx;1", 7, start);

					if(lba)
					{
						if(t==0) fix_eboot=false;

						lba*=0x800ULL;
						cellFsLseek(fd, lba, CELL_FS_SEEK_SET, &msiz2);
						cellFsRead(fd, (void *)&chunk, chunk_size, &msiz1); if(msiz1<=0) break;

						offset=(chunk[0xC]<<24) + (chunk[0xD]<<16) + (chunk[0xE]<<8) + chunk[0xF]; offset-=0x78;
						if(offset < 0x90 || offset > 0x800 || (chunk[offset] | chunk[offset+1] | chunk[offset+2] | chunk[offset+3] | chunk[offset+4] | chunk[offset+5])) offset=(t>2)?0x258:0x428;

						if((t>2) && (offset == 0x258) && (chunk[offset] | chunk[offset+1] | chunk[offset+2] | chunk[offset+3] | chunk[offset+4] | chunk[offset+5])) offset=0x278;

						for(u8 i=0;i<8;i++) ps3_sys_version[i]=chunk[offset+i];

						if((ps3_sys_version[0]+ps3_sys_version[1]+ps3_sys_version[2]+ps3_sys_version[3]+ps3_sys_version[4]+ps3_sys_version[5])==0 && (ps3_sys_version[6] & 0xFF)>0xA4)
						{
							ps3_sys_version[6]=0XA4; ps3_sys_version[7]=0X10;
							cellFsLseek(fd, lba+offset, CELL_FS_SEEK_SET, &msiz2);
							cellFsWrite(fd, ps3_sys_version, 8, &msiz2);
						}
						else goto exit_fix;

						if(t==0) break;

					} else break;
				}
			}

			if(msiz1<=0) break;

			pos+=msiz1;
			size-=msiz1;
			if(chunk_size>size) chunk_size=(int) size;

			sys_timer_usleep(1);
		}
exit_fix:
		cellFsClose(fd);

		// fix update folder
		sprintf(update_path, "/dev_hdd0/game/%s/PARAM.SFO", titleID); int fs;
		if(fix_update && cellFsOpen(update_path, CELL_FS_O_RDONLY, &fs, NULL, 0)==CELL_FS_SUCCEEDED)
		{
			char paramsfo[_4KB_]; unsigned char *mem = (u8*)paramsfo;
			uint64_t msiz = 0;

			cellFsLseek(fs, 0, CELL_FS_SEEK_SET, &msiz);
			cellFsRead(fs, (void *)&paramsfo, _4KB_, &msiz);
			cellFsClose(fs);

			if(fix_param_sfo(mem, titleID, 0) || webman_config->fixgame==FIX_GAME_FORCED) {savefile(update_path, paramsfo, msiz); sprintf(update_path, "/dev_hdd0/game/%s/USRDIR", titleID); fix_game(update_path);}
		}
	}
}
#endif //#ifdef COBRA_ONLY

#endif //#ifdef FIX_GAME
