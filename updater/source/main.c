#include <fcntl.h>
#include <ppu-lv2.h>
#include <sys/file.h>
#include <stdio.h>
#include <string.h>

#include <io/pad.h>

#define SUCCESS 0
#define FAILED -1

#define SC_SYS_POWER 					(379)
#define SYS_REBOOT				 		0x8201

bool lite=false;

int sys_fs_mount(char const* deviceName, char const* deviceFileSystem, char const* devicePath, int writeProt)
{
	lv2syscall8(837, (u64) deviceName, (u64) deviceFileSystem, (u64) devicePath, 0, (u64) writeProt, 0, 0, 0 );
	return_to_user_prog(int);
}

int CopyFile(char* path, char* path2)
{
	int ret = 0;
	s32 fd = -1;
	s32 fd2 = -1;
	u64 lenght = 0LL;

	u64 pos = 0ULL;
	u64 readed = 0, writed = 0;

	char *mem = NULL;

	sysFSStat stat;

	ret= sysLv2FsStat(path, &stat);
	lenght = stat.st_size;

	if(ret) goto skip;

	if(strstr(path, "/dev_hdd0/") != NULL && strstr(path2, "/dev_hdd0/") != NULL)
	{
		if(strcmp(path, path2)==0) return ret;

		sysLv2FsUnlink(path2);
		sysLv2FsLink(path, path2);

		if (sysLv2FsStat(path2, &stat) == 0) return 0;
	}

	ret = sysLv2FsOpen(path, 0, &fd, S_IRWXU | S_IRWXG | S_IRWXO, NULL, 0);
	if(ret) goto skip;

	ret = sysLv2FsOpen(path2, SYS_O_WRONLY | SYS_O_CREAT | SYS_O_TRUNC, &fd2, 0777, NULL, 0);
	if(ret) {sysLv2FsClose(fd);goto skip;}

	mem = malloc(0x100000);
	if (mem == NULL) return FAILED;

	while(pos < lenght)
	{
		readed = lenght - pos; if(readed > 0x100000ULL) readed = 0x100000ULL;
		ret=sysLv2FsRead(fd, mem, readed, &writed);
		if(ret<0) goto skip;
		if(readed != writed) {ret = 0x8001000C; goto skip;}

		ret=sysLv2FsWrite(fd2, mem, readed, &writed);
		if(ret<0) goto skip;
		if(readed != writed) {ret = 0x8001000C; goto skip;}

		pos += readed;
	}

skip:

	if(mem) free(mem);
	if(fd >=0) sysLv2FsClose(fd);
	if(fd2>=0) sysLv2FsClose(fd2);
	if(ret) return ret;

	ret = sysLv2FsStat(path2, &stat);
	if((ret == SUCCESS) && (stat.st_size == lenght)) ret = SUCCESS; else ret = FAILED;

	return ret;
}

int sys_get_version(u32 *version)
{
	lv2syscall2(8, 0x7000, (u64)version);
	return_to_user_prog(int);
}

int sys_get_mamba(void)
{
	lv2syscall1(8, 0x7FFF);
    return_to_user_prog(int);
}

bool is_mamba(void)
{
	sysFSStat stat;
	if(sysLv2FsStat("/dev_hdd0/mamba_plugins.txt", &stat) == SUCCESS) return true;

    u32 version = 0x99999999;
	if (sys_get_version(&version) < 0) return false;
	if (sys_get_mamba() == 0x666) return true;
	return false;
}

bool is_disabled(char *filename, char *filename2)
{
	sysFSStat stat;

    if(sysLv2FsStat("/dev_blind", &stat) != SUCCESS)
       sys_fs_mount("CELL_FS_IOS:BUILTIN_FLSH1", "CELL_FS_FAT", "/dev_blind", 0);

    if(sysLv2FsStat(filename, &stat) == SUCCESS)
    {
        sysLv2FsRename(filename, filename2); // re-enable stage2.bin

        return true;
    }

    return false;
}

bool is_cobra(void)
{
	sysFSStat stat; bool ret = false;

	if(is_disabled("/dev_blind/habib/cobra/stage2_disabled.cex", "/dev_blind/habib/cobra/stage2.cex")) return true;
	if(is_disabled("/dev_blind/sys/stage2_disabled.bin", "/dev_blind/sys/stage2.bin")) return true;
	if(is_disabled("/dev_blind/sys/stage2.bin.bak", "/dev_blind/sys/stage2.bin")) return true;

	if(is_disabled("/dev_blind/rebug/cobra/stage2.cex.bak", "/dev_blind/rebug/cobra/stage2.cex")) ret = true;
	if(is_disabled("/dev_blind/rebug/cobra/stage2.dex.bak", "/dev_blind/rebug/cobra/stage2.dex")) ret = true;

	if(sysLv2FsStat("/dev_flash/sys/stage2.bin", &stat) == SUCCESS) return true;
	if(sysLv2FsStat("/dev_hdd0/boot_plugins.txt", &stat) == SUCCESS) return true;
	if(sysLv2FsStat("/dev_flash/rebug/cobra", &stat) == SUCCESS) return true;

	if (is_mamba())         return false;

	u32 version = 0x99999999;
	if (sys_get_version(&version) < 0) return false;
	if (version != 0x99999999)         return true;

	return ret;
}

#define SYSCALL8_OPCODE_PS3MAPI			 		0x7777
#define PS3MAPI_OPCODE_GET_CORE_MINVERSION		0x0012

int sys_get_minversion_ps3mapi(void)
{
	lv2syscall2(8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_CORE_MINVERSION);
	return_to_user_prog(int);
}


bool is_ps3mapi(void)
{
	if (0x0111 <= sys_get_minversion_ps3mapi()) return true;
	else return false;
}

int add_mygame()
{
// -2 failed and cannot rename the backup
// -1 failed
//  0 already
//  1 done

	char gameexit[] = {0x0d, 0x0a, 0x09, 0x09, 0x09, 0x3c, 0x51, 0x75, 0x65, 0x72, 0x79, 0x0d, 0x0a, 0x09, 0x09, 0x09, 0x09, 0x63, 0x6c, 0x61, 0x73, 0x73, 0x3d, 0x22, 0x74, 0x79, 0x70, 0x65, 0x3a, 0x78, 0x2d, 0x78, 0x6d, 0x62, 0x2f, 0x66, 0x6f, 0x6c, 0x64, 0x65, 0x72, 0x2d, 0x70, 0x69, 0x78, 0x6d, 0x61, 0x70, 0x22, 0x0d, 0x0a, 0x09, 0x09, 0x09, 0x09, 0x6b, 0x65, 0x79, 0x3d, 0x22, 0x73, 0x65, 0x67, 0x5f, 0x67, 0x61, 0x6d, 0x65, 0x65, 0x78, 0x69, 0x74, 0x22, 0x0d, 0x0a, 0x09, 0x09, 0x09, 0x09, 0x73, 0x72, 0x63, 0x3d, 0x22, 0x73, 0x65, 0x6c, 0x3a, 0x2f, 0x2f, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x68, 0x6f, 0x73, 0x74, 0x2f, 0x69, 0x6e, 0x67, 0x61, 0x6d, 0x65, 0x3f, 0x70, 0x61, 0x74, 0x68, 0x3d, 0x63, 0x61, 0x74, 0x65, 0x67, 0x6f, 0x72, 0x79, 0x5f, 0x67, 0x61, 0x6d, 0x65, 0x2e, 0x78, 0x6d, 0x6c, 0x23, 0x73, 0x65, 0x67, 0x5f, 0x67, 0x61, 0x6d, 0x65, 0x65, 0x78, 0x69, 0x74, 0x26, 0x74, 0x79, 0x70, 0x65, 0x3d, 0x67, 0x61, 0x6d, 0x65, 0x22, 0x0d, 0x0a, 0x09, 0x09, 0x09, 0x09, 0x2f, 0x3e};
	char fb[] = {0x0d, 0x0a, 0x09, 0x09, 0x09, 0x3c, 0x51, 0x75, 0x65, 0x72, 0x79, 0x20, 0x63, 0x6c, 0x61, 0x73, 0x73, 0x3d, 0x22, 0x74, 0x79, 0x70, 0x65, 0x3a, 0x78, 0x2d, 0x78, 0x6d, 0x62, 0x2f, 0x66, 0x6f, 0x6c, 0x64, 0x65, 0x72, 0x2d, 0x70, 0x69, 0x78, 0x6d, 0x61, 0x70, 0x22, 0x0d, 0x0a, 0x09, 0x09, 0x09, 0x09, 0x6b, 0x65, 0x79, 0x3d, 0x22, 0x78, 0x6d, 0x62, 0x5f, 0x61, 0x70, 0x70, 0x33, 0x22, 0x20, 0x0d, 0x0a, 0x09, 0x09, 0x09, 0x09, 0x61, 0x74, 0x74, 0x72, 0x3d, 0x22, 0x78, 0x6d, 0x62, 0x5f, 0x61, 0x70, 0x70, 0x33, 0x22, 0x20, 0x0d, 0x0a, 0x09, 0x09, 0x09, 0x09, 0x73, 0x72, 0x63, 0x3d, 0x22, 0x78, 0x6d, 0x62, 0x3a, 0x2f, 0x2f, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x68, 0x6f, 0x73, 0x74, 0x2f, 0x64, 0x65, 0x76, 0x5f, 0x68, 0x64, 0x64, 0x30, 0x2f, 0x78, 0x6d, 0x6c, 0x68, 0x6f, 0x73, 0x74, 0x2f, 0x67, 0x61, 0x6d, 0x65, 0x5f, 0x70, 0x6c, 0x75, 0x67, 0x69, 0x6e, 0x2f, 0x66, 0x62, 0x2e, 0x78, 0x6d, 0x6c, 0x23, 0x73, 0x65, 0x67, 0x5f, 0x66, 0x62, 0x22, 0x0d, 0x0a, 0x09, 0x09, 0x09, 0x09, 0x2f, 0x3e};
	FILE* f;
	long size;
	char *cat;
	char cat_path[255];
	size_t result;
	int i, j, pos=0;
	sysFSStat stat;

	//read original cat
	f=fopen("/dev_flash/vsh/resource/explore/xmb/category_game.xml", "r");
	if(f==NULL) return FAILED;
	fseek (f , 0 , SEEK_END);
	size = ftell (f);
	fseek(f, 0, SEEK_SET);

	cat = (char*) malloc (sizeof(char)*size);
	if (cat == NULL) {free(cat); return FAILED;}

	result = fread(cat,1,size, f);
	if (result != size) {free (cat); fclose (f); return FAILED;}
	fclose (f);

	// is fb.xml entry in cat file ?
	if(strstr(cat, "fb.xml")!=NULL) {free(cat); return 0;}

	// search position
	for(i=0; i < size - sizeof(gameexit); i++) {
		pos=i;
		for(j=0; j<sizeof(gameexit); j++) {
			if( cat[i+j] != gameexit[j]) break;
			if(j==(sizeof(gameexit)-1)) goto out_s;
		}
	}
	out_s:

	//write patched cat
	f=fopen("/dev_hdd0/game/UPDWEBMOD/USRDIR/category_game.xml", "w");
	if(f==NULL) {free(cat); return FAILED;}
	fwrite(cat, 1, pos, f);
	fwrite(fb, 1, sizeof(fb), f);
	fwrite(&cat[pos], 1, size-pos, f);
	fclose(f);


	// set target path for category_game.xml
	strcpy(cat_path, "/dev_blind/vsh/resource/explore/xmb/category_game.xml");
	if(sysLv2FsStat(cat_path, &stat) != SUCCESS) {
		strcpy(cat_path, "/dev_habib/vsh/resource/explore/xmb/category_game.xml");
		if(sysLv2FsStat(cat_path, &stat) != SUCCESS) {
			strcpy(cat_path, "/dev_rewrite/vsh/resource/explore/xmb/category_game.xml");

			// mount /dev_blind if category_game.xml is not found
			if(sysLv2FsStat(cat_path, &stat) != SUCCESS) {
				if(sys_fs_mount("CELL_FS_IOS:BUILTIN_FLSH1", "CELL_FS_FAT", "/dev_blind", 0) == SUCCESS) {
					strcpy(cat_path, "/dev_blind/vsh/resource/explore/xmb/category_game.xml");
					if(sysLv2FsStat(cat_path, &stat) != SUCCESS) { free(cat); return FAILED;}
				} else { free(cat); return FAILED;}
			}
		}
	}

	// rename category_game.xml as category_game.xml.bak
	char cat_path_bak[255];
	strcpy(cat_path_bak, cat_path);
	strcat(cat_path_bak, ".bak");
	sysLv2FsUnlink(cat_path_bak);
	if(sysLv2FsRename(cat_path, cat_path_bak) != SUCCESS) {free(cat); return FAILED;} ;

	// update category_game.xml
	if(CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/category_game.xml", cat_path) != SUCCESS)
	{
		sysLv2FsUnlink(cat_path);
		if(sysLv2FsRename(cat_path_bak, cat_path)) { //restore category_game.xml from category_game.xml.bak
			{lv2syscall3(392, 0x1004, 0xa, 0x1b6);} ///TRIPLE BIP
			free(cat);
			return -2;
		}
	}
	free(cat);
	return 1;
}

int main()
{
//--- hold CROSS

    unsigned button = 0;

    padInfo padinfo;
    padData paddata;

    ioPadInit(7);

    int n, r;
    for(r=0; r<10; r++)
    {
        ioPadGetInfo(&padinfo);
        for(n = 0; n < 7; n++)
        {
            if(padinfo.status[n])
            {
                ioPadGetData(n, &paddata);
                button = (paddata.button[2] << 8) | (paddata.button[3] & 0xff);
                break;
            }
        }
        if(button) break; else usleep(20000);
    }
    ioPadEnd();

	if(button) lite=true;
//---

	sysFSStat stat;

	sysLv2FsMkdir("/dev_hdd0/tmp", 0777);
	sysLv2FsMkdir("/dev_hdd0/tmp/wm_lang", 0777);

	// remove language files (old location)
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_EN.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_AR.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_CN.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_DE.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_ES.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_FR.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_GR.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_DK.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_HU.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_HR.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_BG.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_CZ.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_SK.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_IN.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_JP.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_KR.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_IT.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_NL.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_PL.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_PT.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_RU.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_TR.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_ZH.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/LANG_XX.TXT");

	// remove language files
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_EN.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_AR.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_CN.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_DE.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_ES.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_FR.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_GR.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_DK.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_HU.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_HR.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_BG.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_CZ.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_SK.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_IN.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_JP.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_KR.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_IT.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_NL.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_PL.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_PT.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_RU.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_TR.TXT");
	sysLv2FsUnlink("/dev_hdd0/tmp/wm_lang/LANG_ZH.TXT");

	// remove old files
	sysLv2FsUnlink("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_rebug_cobra_multi19.sprx");
	sysLv2FsUnlink("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_rebug_cobra_multi20.sprx");
	sysLv2FsUnlink("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_rebug_cobra_multi21.sprx");
	sysLv2FsUnlink("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_webchat.sprx");

	// update languages
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_EN.TXT","/dev_hdd0/tmp/wm_lang/LANG_EN.TXT");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_AR.TXT","/dev_hdd0/tmp/wm_lang/LANG_AR.TXT");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_CN.TXT","/dev_hdd0/tmp/wm_lang/LANG_CN.TXT");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_DE.TXT","/dev_hdd0/tmp/wm_lang/LANG_DE.TXT");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_ES.TXT","/dev_hdd0/tmp/wm_lang/LANG_ES.TXT");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_FR.TXT","/dev_hdd0/tmp/wm_lang/LANG_FR.TXT");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_GR.TXT","/dev_hdd0/tmp/wm_lang/LANG_GR.TXT");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_DK.TXT","/dev_hdd0/tmp/wm_lang/LANG_DK.TXT");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_HU.TXT","/dev_hdd0/tmp/wm_lang/LANG_HU.TXT");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_HR.TXT","/dev_hdd0/tmp/wm_lang/LANG_HR.TXT");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_BG.TXT","/dev_hdd0/tmp/wm_lang/LANG_BG.TXT");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_CZ.TXT","/dev_hdd0/tmp/wm_lang/LANG_CZ.TXT");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_SK.TXT","/dev_hdd0/tmp/wm_lang/LANG_SK.TXT");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_IN.TXT","/dev_hdd0/tmp/wm_lang/LANG_IN.TXT");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_IT.TXT","/dev_hdd0/tmp/wm_lang/LANG_IT.TXT");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_JP.TXT","/dev_hdd0/tmp/wm_lang/LANG_JP.TXT");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_KR.TXT","/dev_hdd0/tmp/wm_lang/LANG_KR.TXT");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_NL.TXT","/dev_hdd0/tmp/wm_lang/LANG_NL.TXT");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_PL.TXT","/dev_hdd0/tmp/wm_lang/LANG_PL.TXT");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_PT.TXT","/dev_hdd0/tmp/wm_lang/LANG_PT.TXT");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_RU.TXT","/dev_hdd0/tmp/wm_lang/LANG_RU.TXT");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_TR.TXT","/dev_hdd0/tmp/wm_lang/LANG_TR.TXT");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_ZH.TXT","/dev_hdd0/tmp/wm_lang/LANG_ZH.TXT");

	sysLv2FsMkdir("/dev_hdd0/xmlhost/game_plugin", 0777);
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/mobile.html","/dev_hdd0/xmlhost/game_plugin/mobile.html");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/background.gif","/dev_hdd0/xmlhost/game_plugin/background.gif");

	sysLv2FsMkdir("/dev_hdd0/tmp/wm_icons", 0777);

	// copy new icons
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/icon_wm_album_ps3.png", "/dev_hdd0/tmp/wm_icons/icon_wm_album_ps3.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/icon_wm_album_psx.png", "/dev_hdd0/tmp/wm_icons/icon_wm_album_psx.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/icon_wm_album_ps2.png", "/dev_hdd0/tmp/wm_icons/icon_wm_album_ps2.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/icon_wm_album_psp.png", "/dev_hdd0/tmp/wm_icons/icon_wm_album_psp.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/icon_wm_album_dvd.png", "/dev_hdd0/tmp/wm_icons/icon_wm_album_dvd.png");

	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/icon_wm_ps3.png"      , "/dev_hdd0/tmp/wm_icons/icon_wm_ps3.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/icon_wm_psx.png"      , "/dev_hdd0/tmp/wm_icons/icon_wm_psx.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/icon_wm_ps2.png"      , "/dev_hdd0/tmp/wm_icons/icon_wm_ps2.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/icon_wm_psp.png"      , "/dev_hdd0/tmp/wm_icons/icon_wm_psp.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/icon_wm_dvd.png"      , "/dev_hdd0/tmp/wm_icons/icon_wm_dvd.png");

	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/icon_wm_settings.png" , "/dev_hdd0/tmp/wm_icons/icon_wm_settings.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/icon_wm_eject.png"    , "/dev_hdd0/tmp/wm_icons/icon_wm_eject.png"   );

	// XMBM+ webMAN
	sysLv2FsMkdir("/dev_hdd0/game/XMBMANPLS", 0777);
	sysLv2FsMkdir("/dev_hdd0/game/XMBMANPLS/USRDIR", 0777);
	sysLv2FsMkdir("/dev_hdd0/game/XMBMANPLS/USRDIR/IMAGES", 0777);
	sysLv2FsMkdir("/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES", 0777);

	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN.xml"    ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN.xml");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN_AR.xml" ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN_AR.xml");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN_CN.xml" ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN_CN.xml");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN_DE.xml" ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN_DE.xml");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN_ES.xml" ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN_ES.xml");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN_FR.xml" ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN_FR.xml");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN_GR.xml" ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN_GR.xml");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN_DK.xml" ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN_DK.xml");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN_HU.xml" ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN_HU.xml");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN_HR.xml" ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN_HR.xml");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN_BG.xml" ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN_BG.xml");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN_CZ.xml" ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN_CZ.xml");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN_SK.xml" ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN_SK.xml");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN_IN.xml" ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN_IN.xml");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN_JP.xml" ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN_JP.xml");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN_KR.xml" ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN_KR.xml");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN_IT.xml" ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN_IT.xml");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN_NL.xml" ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN_NL.xml");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN_PL.xml" ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN_PL.xml");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN_PT.xml" ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN_PT.xml");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN_RU.xml" ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN_RU.xml");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN_TR.xml" ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN_TR.xml");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webMAN_ZH.xml" ,"/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN_ZH.xml");

	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/bd.png"        	,"/dev_hdd0/game/XMBMANPLS/USRDIR/IMAGES/bd.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/cachefiles.png"	,"/dev_hdd0/game/XMBMANPLS/USRDIR/IMAGES/cachefiles.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/filemanager.png"	,"/dev_hdd0/game/XMBMANPLS/USRDIR/IMAGES/filemanager.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/devflash.png"  	,"/dev_hdd0/game/XMBMANPLS/USRDIR/IMAGES/devflash.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/gamesbrowser.png"	,"/dev_hdd0/game/XMBMANPLS/USRDIR/IMAGES/gamesbrowser.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/gamedata.png"  	,"/dev_hdd0/game/XMBMANPLS/USRDIR/IMAGES/gamedata.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/homebrew.png"  	,"/dev_hdd0/game/XMBMANPLS/USRDIR/IMAGES/homebrew.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/multiman.png"  	,"/dev_hdd0/game/XMBMANPLS/USRDIR/IMAGES/multiman.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/network.png"   	,"/dev_hdd0/game/XMBMANPLS/USRDIR/IMAGES/network.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/pkgmanager.png"	,"/dev_hdd0/game/XMBMANPLS/USRDIR/IMAGES/pkgmanager.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/refreshhtml.png"	,"/dev_hdd0/game/XMBMANPLS/USRDIR/IMAGES/refreshhtml.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/refreshxml.png"	,"/dev_hdd0/game/XMBMANPLS/USRDIR/IMAGES/refreshxml.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/restartps3.png"	,"/dev_hdd0/game/XMBMANPLS/USRDIR/IMAGES/restartps3.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/settings.png"  	,"/dev_hdd0/game/XMBMANPLS/USRDIR/IMAGES/settings.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/sysinfo.png"  	,"/dev_hdd0/game/XMBMANPLS/USRDIR/IMAGES/sysinfo.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/tools.png"  		,"/dev_hdd0/game/XMBMANPLS/USRDIR/IMAGES/tools.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/unload.png"  		,"/dev_hdd0/game/XMBMANPLS/USRDIR/IMAGES/unload.png");
	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webman.png"  		,"/dev_hdd0/game/XMBMANPLS/USRDIR/IMAGES/webman.png");

	// skip update custom language
	if(sysLv2FsStat("/dev_hdd0/tmp/wm_lang/LANG_XX.TXT", &stat))
		CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/LANG_XX.TXT","/dev_hdd0/tmp/wm_lang/LANG_XX.TXT");

	CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/libfs.sprx","/dev_hdd0/tmp/libfs.sprx");

	// copy raw_iso.sprx to dev_flash
	if(sysLv2FsStat("/dev_flash/vsh/module/raw_iso.sprx", &stat) != SUCCESS)
	{
		if(sysLv2FsStat("/dev_blind", &stat) != SUCCESS)
			sys_fs_mount("CELL_FS_IOS:BUILTIN_FLSH1", "CELL_FS_FAT", "/dev_blind", 0);

		if(sysLv2FsStat("/dev_blind", &stat) == SUCCESS)
			CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/raw_iso.sprx","/dev_blind/vsh/module/raw_iso.sprx");
	}

	// copy raw_iso.sprx to dev_hdd (if failed to copy it to dev_flash)
	if(sysLv2FsStat("/dev_flash/vsh/module/raw_iso.sprx", &stat) != SUCCESS)
	{
		if(sysLv2FsStat("/dev_hdd0/plugins", &stat) == SUCCESS)
			CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/raw_iso.sprx","/dev_hdd0/plugins/raw_iso.sprx");
		else
			CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/raw_iso.sprx","/dev_hdd0/raw_iso.sprx");
	}

	// copy standalone video recorder plugin (video_rec.sprx) to /plugins folder
	if((sysLv2FsStat("/dev_hdd0/plugins", &stat) == SUCCESS))
		CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/video_rec.sprx","/dev_hdd0/plugins/video_rec.sprx");

	// update PRX+Mamba Loader
	if((sysLv2FsStat("/dev_hdd0/game/IRISMAN01/USRDIR/webftp_server.sprx", &stat) == SUCCESS))
	{
		sysLv2FsChmod("/dev_hdd0/game/IRISMAN01/USRDIR/webftp_server.sprx", 0777);
		sysLv2FsUnlink("/dev_hdd0/game/IRISMAN01/USRDIR/webftp_server.sprx");

		if((sysLv2FsStat("/dev_flash/rebug", &stat) == SUCCESS) && is_ps3mapi())
			CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_rebug_cobra_ps3mapi.sprx", "/dev_hdd0/game/IRISMAN01/USRDIR/webftp_server.sprx");
		else if((sysLv2FsStat("/dev_flash/rebug", &stat) == SUCCESS))
			CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_rebug_cobra_multi23.sprx", "/dev_hdd0/game/IRISMAN01/USRDIR/webftp_server.sprx");
		else if(lite)
			CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_lite.sprx", "/dev_hdd0/game/IRISMAN01/USRDIR/webftp_server.sprx");
		else if(is_ps3mapi())
			CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_ps3mapi.sprx", "/dev_hdd0/game/IRISMAN01/USRDIR/webftp_server.sprx");
		else
			CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server.sprx", "/dev_hdd0/game/IRISMAN01/USRDIR/webftp_server.sprx");
	}
	else if((sysLv2FsStat("/dev_hdd0/game/IRISMAN01/USRDIR/webftp_server_ps3mapi.sprx", &stat) == SUCCESS))
	{
		sysLv2FsChmod("/dev_hdd0/game/IRISMAN01/USRDIR/webftp_server_ps3mapi.sprx", 0777);
		sysLv2FsUnlink("/dev_hdd0/game/IRISMAN01/USRDIR/webftp_server_ps3mapi.sprx");

		if((sysLv2FsStat("/dev_flash/rebug", &stat) == SUCCESS))
			CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_rebug_cobra_ps3mapi.sprx", "/dev_hdd0/game/IRISMAN01/USRDIR/webftp_server_ps3mapi.sprx");
		else
			CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_ps3mapi.sprx", "/dev_hdd0/game/IRISMAN01/USRDIR/webftp_server_ps3mapi.sprx");
	}

	char ligne[255];
	FILE* f=NULL;

	// update PRX Loader
	if(sysLv2FsStat("/dev_hdd0/game/PRXLOADER/USRDIR/plugins.txt", &stat) == SUCCESS)
	{
		f=fopen("/dev_hdd0/game/PRXLOADER/USRDIR/plugins.txt", "r");
		while(fgets(ligne, 255, f) != NULL)
		{
			if(strstr(ligne,"webftp_server") != NULL)
			{
				fclose(f);
				strtok(ligne, "\r\n");
				sysLv2FsUnlink(ligne);
				CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_noncobra.sprx",ligne);
				goto cont;
			}
		}
		fclose(f);
		f=fopen("/dev_hdd0/game/PRXLOADER/USRDIR/plugins.txt", "a");
		fputs("\r\n/dev_hdd0/game/PRXLOADER/USRDIR/webftp_server_noncobra.sprx", f);
		fclose(f);

		sysLv2FsChmod("/dev_hdd0/game/PRXLOADER/USRDIR/webftp_server_noncobra.sprx", 0777);
		sysLv2FsUnlink("/dev_hdd0/game/PRXLOADER/USRDIR/webftp_server_noncobra.sprx");

		CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_noncobra.sprx","/dev_hdd0/game/PRXLOADER/USRDIR/webftp_server_noncobra.sprx");
	}

cont:

	// update dev_flash (rebug)
	if((sysLv2FsStat("/dev_flash/vsh/module/webftp_server.sprx", &stat) == SUCCESS) || (sysLv2FsStat("/dev_flash/vsh/module/webftp_server.sprx.bak", &stat) == SUCCESS))
	{
		if(sysLv2FsStat("/dev_blind", &stat) != SUCCESS)
			sys_fs_mount("CELL_FS_IOS:BUILTIN_FLSH1", "CELL_FS_FAT", "/dev_blind", 0);

		sysLv2FsChmod("/dev_blind/vsh/module/webftp_server.sprx", 0777);
		sysLv2FsUnlink("/dev_blind/vsh/module/webftp_server.sprx");

		sysLv2FsChmod("/dev_blind/vsh/module/webftp_server.sprx.bak", 0777);
		sysLv2FsUnlink("/dev_blind/vsh/module/webftp_server.sprx.bak");

		if(is_ps3mapi())
			CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_rebug_cobra_ps3mapi.sprx", "/dev_blind/vsh/module/webftp_server.sprx");
		else
			CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_rebug_cobra_multi23.sprx", "/dev_blind/vsh/module/webftp_server.sprx");


		// delete webMAN from hdd0
		if((sysLv2FsStat("/dev_blind/vsh/module/webftp_server.sprx", &stat) == SUCCESS))
		{
			sysLv2FsChmod("/dev_hdd0/webftp_server.sprx", 0777);
			sysLv2FsUnlink("/dev_hdd0/webftp_server.sprx");

			sysLv2FsChmod("/dev_hdd0/plugins/webftp_server.sprx", 0777);
			sysLv2FsUnlink("/dev_hdd0/plugins/webftp_server.sprx");

			if(sysLv2FsStat("/dev_hdd0/boot_plugins.txt", &stat) == SUCCESS)
			{
				f=fopen("/dev_hdd0/boot_plugins.txt", "r");
				while(fgets(ligne, 255, f) != NULL)
				{
					if(strstr(ligne,"webftp_server") != NULL && strstr(ligne,"/dev_blind") == NULL)
					{
						strtok(ligne, "\r\n");
						sysLv2FsChmod(ligne, 0777);
						sysLv2FsUnlink(ligne);
						break;
					}
				}
				fclose(f);
			}
		}

		// reboot
		sysLv2FsUnlink("/dev_hdd0/tmp/turnoff");
		//{lv2syscall4(379,0x200,0,0,0); return_to_user_prog(int);}
		//{lv2syscall4(379,0x1200,0,0,0); return_to_user_prog(int);}
		{lv2syscall3(SC_SYS_POWER, SYS_REBOOT, 0, 0); return_to_user_prog(int);}
        //{lv2syscall3(SC_SYS_POWER, SYS_REBOOT, 0, 0);}

		return 0;
	}

	// update boot_plugins.txt
	if(lite || is_cobra())
	   {
		if(sysLv2FsStat("/dev_hdd0/boot_plugins.txt", &stat) == SUCCESS)
		{
			f=fopen("/dev_hdd0/boot_plugins.txt", "r");
			while(fgets(ligne, 255, f) != NULL)
			{
				if(strstr(ligne,"webftp_server") != NULL)
				{
					fclose(f);
					strtok(ligne, "\r\n");
					sysLv2FsChmod(ligne, 0777);
					sysLv2FsUnlink(ligne);
					if(lite)
						CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_lite.sprx",ligne);
					else
					{
						if(is_ps3mapi())
							CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_ps3mapi.sprx", ligne);
						else
							CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server.sprx", ligne);
					}
					goto exit;
				}
			}
			fclose(f);
		}

		f=fopen("/dev_hdd0/boot_plugins.txt", "a");
		if(is_ps3mapi())
				fputs("\r\n/dev_hdd0/webftp_server_ps3mapi.sprx", f);
			else
				fputs("\r\n/dev_hdd0/webftp_server.sprx", f);
		fclose(f);

		sysLv2FsChmod("/dev_hdd0/webftp_server.sprx", 0777);
		sysLv2FsUnlink("/dev_hdd0/webftp_server.sprx");

		if(lite)
			CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_lite.sprx", "/dev_hdd0/webftp_server.sprx");
		else
		{
			if(is_ps3mapi())
			{
				if((sysLv2FsStat("/dev_flash/rebug", &stat) == SUCCESS))
					CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_rebug_cobra_ps3mapi.sprx", "/dev_hdd0/webftp_server_ps3mapi.sprx");
				else
					CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_ps3mapi.sprx", "/dev_hdd0/webftp_server_ps3mapi.sprx");
			}
			else
				CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server.sprx", "/dev_hdd0/webftp_server.sprx");
		}
	}
	// update mamba_plugins.txt
	if(is_mamba())
	{
		if(sysLv2FsStat("/dev_hdd0/mamba_plugins.txt", &stat) == SUCCESS)
		{
			f=fopen("/dev_hdd0/mamba_plugins.txt", "r");
			while(fgets(ligne, 255, f) != NULL)
			{
				if(strstr(ligne,"webftp_server") != NULL)
				{
					fclose(f);
					strtok(ligne, "\r\n");
					sysLv2FsChmod(ligne, 0777);
					sysLv2FsUnlink(ligne);
					CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_ps3mapi.sprx", ligne);
					goto exit;
				}
			}
			fclose(f);
		}

		f=fopen("/dev_hdd0/mamba_plugins.txt", "a");
		fputs("\r\n/dev_hdd0/webftp_server_ps3mapi.sprx", f);
		fclose(f);

		sysLv2FsChmod("/dev_hdd0/webftp_server_ps3mapi.sprx", 0777);
		sysLv2FsUnlink("/dev_hdd0/webftp_server_ps3mapi.sprx");

			if((sysLv2FsStat("/dev_flash/rebug", &stat) == SUCCESS))
					CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_rebug_cobra_ps3mapi.sprx", "/dev_hdd0/webftp_server_ps3mapi.sprx");
				else
					CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_ps3mapi.sprx", "/dev_hdd0/webftp_server_ps3mapi.sprx");
	}
	// update prx_plugins.txt
	if(sysLv2FsStat("/dev_hdd0/prx_plugins.txt", &stat) == SUCCESS)
	{
		f=fopen("/dev_hdd0/prx_plugins.txt", "r");
		while(fgets(ligne, 255, f) != NULL)
		{
			if(strstr(ligne,"webftp_server") != NULL)
			{
				fclose(f);
				strtok(ligne, "\r\n");
				sysLv2FsUnlink(ligne);
				CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_noncobra.sprx",ligne);
				goto cont;
			}
		}
		fclose(f);
		f=fopen("/dev_hdd0/prx_plugins.txt", "a");
		fputs("\r\n/dev_hdd0/webftp_server_noncobra.sprx", f);
		fclose(f);

		sysLv2FsChmod("/dev_hdd0/webftp_server_noncobra.sprx", 0777);
		sysLv2FsUnlink("/dev_hdd0/webftp_server_noncobra.sprx");

		CopyFile("/dev_hdd0/game/UPDWEBMOD/USRDIR/webftp_server_noncobra.sprx","/dev_hdd0/webftp_server_noncobra.sprx");
	}
	// exit
exit:

	// update category_game.xml (add fb.xml)
	if(add_mygame() != -2);


	// reboot
	sysLv2FsUnlink("/dev_hdd0/tmp/turnoff");
	//{lv2syscall4(379,0x200,0,0,0); return_to_user_prog(int);}
	//{lv2syscall4(379,0x1200,0,0,0); return_to_user_prog(int);}
	{lv2syscall3(SC_SYS_POWER, SYS_REBOOT, 0, 0); return_to_user_prog(int);}

	return 0;
}
