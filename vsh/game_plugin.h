// Mysis game_plugin.h v0.1
// 3141card (C style version)
#ifndef __GAME_PLUGIN_H__
#define __GAME_PLUGIN_H__

typedef struct game_plugin_interface_t
{
	int32_t (*DoUnk0)(void); // set Widget "page_game_main" and activate
	int32_t (*DoUnk1)(void *); // uint8_t [0x5B8]
	int32_t (*DoUnk2)(void *); // uint8_t [0x230]
	int32_t (*DoUnk3)(int); // 3 = "CB" Category
	int32_t (*DoUnk4)(int, void *); // uint8_t [0x1A0]
	int32_t (*DoUnk5)(void);
	int32_t (*DoUnk6)(void);
	int32_t (*DoUnk7)(void);
	int32_t (*gameInfo)(void *); // uint8_t [0x114] // 0x4=TitleID, 0x14=Title/Game Name
	int32_t (*DoUnk9)(void *); // uint8_t [0x80]
	int32_t (*DoUnk10)(char *); // char [8]
	int32_t (*DoUnk11)(unsigned long, int);
	int32_t (*DoUnk12)(void);
	int32_t (*DoUnk13)(void);
	int32_t (*DoUnk14)(void);
	int32_t (*GetExecAppType)(int *, int *); // apptype, extended type?
	int32_t (*DoUnk16)(int *);
	int32_t (*DoUnk17)(void);
	int32_t (*DoUnk18)(int *, char *); // char [0x20]
	int32_t (*DoUnk19)(int *, char *, char *); // char [0x20]
	int32_t (*DoUnk20)(void *); // uint8_t [0x5B8]
	int32_t (*DoUnk21)(void);
	int32_t (*commerce2ExecuteStoreBrowse)(int, char *, int, int); //targetType,targetId,flags,userdata
	int32_t (*DoUnk23)(void *); // uint8_t [0xA4]
	int32_t (*DoUnk24)(void *); // uint8_t [0xA4]
	int32_t (*wakeupWithGameExit)(char *, int); // char [0x800], userdata
	int32_t (*commerce2Reboot4PrgDl)(int); // taskId
	int32_t (*DoUnk27)(char *); // [0x800]
	int32_t (*DoUnk28)(void);
	int32_t (*DoUnk29)(void *); // [0xxA4]
	int32_t (*commerce2GetOptions)(int *); // userdata
	int32_t (*GetUsrdataOnGameExit)(int *);
	int32_t (*GetManualPath)(char *); // [0x80]
	int32_t (*DoUnk33)(void);
	int32_t (*DoUnk34)(char *); // [0x20]
	int32_t (*DoUnk35)(char *); // [0x20]
	int32_t (*DoUnk36)(int, char *); // no size check
	int32_t (*DoUnk37)(void);
	int32_t (*DoUnk38)(unsigned long);
	int32_t (*DoUnk39)(char *); // titleId[0x20]
	int32_t (*DoUnk40)(char *, int *, int); // titleId[0x20]
	int32_t (*DoUnk41)(char *, char *, int); // titleId[0x20], char [4]
	int32_t (*DoUnk42)(char *, int, char *, int); //titleid, flags
	int32_t (*DoUnk43)(void);
	int32_t (*DoUnk44)(void);
	int32_t (*initGameData)(int, int); // memContainer, NoCalcFlag
	int32_t (*EndGameData)(void);
	int32_t (*getGameDataStat)(char *, char *, void *); // [0x1450]
	int32_t (*updateGameData)(char *, char *, void *, void *);//callback, sysparam[0x1450]
	int32_t (*DoUnk49)(int, int, int, void *, char *);
	int32_t (*DoUnk50)(void);
	int32_t (*DoUnk51)(void);
	int32_t (*cacheInit)(void *, void *); // callback, SysCacheParam[0x444]
	int32_t (*cacheClear)(void);
	int32_t (*GetBootInfo)(void *);// [0x20]
	int32_t (*GetTitleId)(void *);
	int32_t (*kbGetSize)(int *, int);
	int32_t (*SetSysVersion)(char *);
	int32_t (*GetDiscInfo)(void *); //[0x20]
	int32_t (*DoUnk59)(int, int, void *, int);
	int32_t (*SetEjectMode)(int); //int mode
	int32_t (*gameExec)(char *,char *,int,int,int,int,int);
	int32_t (*getList)(int *, int *, int *, int *, int);
	int32_t (*DoUnk63_GetBootInfo)(int *, char *, int *); //[0x20]
	int32_t (*SetExecDataParam)(int *);
	int32_t (*GetExitInfo)(int *, int *, char *, int *, int *);
	int32_t (*HomePath)(char *);
	int32_t (*DeleteGame)(char *, int);
	int32_t (*OptionPath)(char *, char *);
	int32_t (*ExportPath)(char *);
	int32_t (*ImportPath)(char *);
	int32_t (*Open)(int,char *, int, int *);
	int32_t (*BootCheck)(int *, int *, int, int *, char *);
	int32_t (*PatchCheck)(int,int *);
	int32_t (*Create)(void *, char *, char *, int);
	int32_t (*getInt)(int, int*, int);
	int32_t (*getStr)(int, char *, int, int);
	int32_t (*setInt)(int, int, int);
	int32_t (*setStr)(int, char *, int);
	int32_t (*Close)(char *, char *);
	int32_t (*DoUnk80)(int, int, char *);
	int32_t (*getSizeKB)(int *);
	int32_t (*tInstall)(char *, char *, int, int);
	int32_t (*mtInstall)(int);
	int32_t (*mtWrite)(int, void *);
	int32_t (*mtClose)(int, int);
	int32_t (*getUpPath)(char *);
	int32_t (*getWConPath)(char *);
	int32_t (*delGameData)(char *);
	int32_t (*getDevIdList)(int *, void *);
	int32_t (*getDevInfo)(unsigned long, void *);
	int32_t (*getUsbDevInfo)(unsigned long,void *);
	int32_t (*storageMode)(void);
	int32_t (*notifyCtrl)(int);
	int32_t (*allreadyDisp)(void);
	int32_t (*pspLoad)(void *, int);
	int32_t (*pspSave)(void *, int);
	int32_t (*vmcAssign)(int,char *, void *, int);
	int32_t (*ps1End)(int);
	int32_t (*SetPadRumble)(int);
	int32_t (*DoUnk100)(void *, int);
	int32_t (*DoUnk101)(void);
	int32_t (*DoUnk102)(char *);
	int32_t (*DoUnk103_DeleteGame)(char *);
	int32_t (*DoUnk104)(void);
} game_plugin_interface;

game_plugin_interface * game_interface;

//int (*View_Find)(const char *) = 0;
//int (*plugin_GetInterface)(int,int) = 0;
//int (*vsh_sprintf)( char*, const char*,...) = 0;

#endif // __GAME_PLUGIN_H__