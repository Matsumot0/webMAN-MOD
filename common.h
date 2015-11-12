#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef DEBUG
#define DPRINTF		printf
#else
#define DPRINTF(...)
#endif

/*
#define HVSC_SYSCALL				811                  	// which syscall to overwrite with hvsc redirect
#define HVSC_SYSCALL_ADDR_341		0x80000000001BB414ULL	// where above syscall is in lv2 3.41
#define HVSC_SYSCALL_ADDR_355		0x8000000000195540ULL	// where above syscall is in lv2 3.55
#define HVSC_SYSCALL_ADDR_355D		0x800000000019B8FCULL	// where above syscall is in lv2 3.55 DEX
#define HVSC_SYSCALL_ADDR_421		0x80000000001B60A0ULL	// where above syscall is in lv2 4.21
#define HVSC_SYSCALL_ADDR_421D		0x80000000001BC490ULL	// where above syscall is in lv2 4.21 DEX
#define HVSC_SYSCALL_ADDR_430		0x80000000001B6428ULL	// where above syscall is in lv2 4.30
#define HVSC_SYSCALL_ADDR_430D		0x80000000001BC818ULL	// where above syscall is in lv2 4.30 DEX
#define HVSC_SYSCALL_ADDR_431		0x80000000001B6430ULL	// where above syscall is in lv2 4.31
#define HVSC_SYSCALL_ADDR_440		0x80000000001B540CULL	// where above syscall is in lv2 4.40
#define HVSC_SYSCALL_ADDR_441		0x80000000001B5414ULL	// where above syscall is in lv2 4.41
#define HVSC_SYSCALL_ADDR_441D		0x80000000001BB804ULL	// where above syscall is in lv2 4.41 DEX
#define HVSC_SYSCALL_ADDR_446		0x80000000001B5674ULL	// where above syscall is in lv2 4.46
#define HVSC_SYSCALL_ADDR_446D		0x80000000001BBA64ULL	// where above syscall is in lv2 4.46 DEX
#define HVSC_SYSCALL_ADDR_450		0x80000000001A6750ULL	// where above syscall is in lv2 4.50
#define HVSC_SYSCALL_ADDR_450D		0x80000000001ACC00ULL	// where above syscall is in lv2 4.50 DEX
#define HVSC_SYSCALL_ADDR_453		0x80000000001A68B8ULL	// where above syscall is in lv2 4.53
#define HVSC_SYSCALL_ADDR_453D		0x80000000001ACD68ULL	// where above syscall is in lv2 4.53 DEX
#define HVSC_SYSCALL_ADDR_455		0x80000000001A7DA4ULL	// where above syscall is in lv2 4.55
#define HVSC_SYSCALL_ADDR_455D		0x80000000001AE254ULL	// where above syscall is in lv2 4.55 DEX
#define HVSC_SYSCALL_ADDR_460		0x80000000001A6A14ULL	// where above syscall is in lv2 4.60     <- peek( SYSCALL_TABLE_460  + HVSC_SYSCALL*8)
#define HVSC_SYSCALL_ADDR_460D		0x80000000001ACEC4ULL	// where above syscall is in lv2 4.60 DEX <- peek( SYSCALL_TABLE_460D + HVSC_SYSCALL*8)
#define HVSC_SYSCALL_ADDR_465		0x80000000001A6A1CULL	// where above syscall is in lv2 4.65     <- peek( SYSCALL_TABLE_465  + HVSC_SYSCALL*8)
#define HVSC_SYSCALL_ADDR_465D		0x80000000001ACECCULL	// where above syscall is in lv2 4.65 DEX <- peek( SYSCALL_TABLE_465D + HVSC_SYSCALL*8)
#define HVSC_SYSCALL_ADDR_470		0x80000000002A0EE0ULL	// where above syscall is in lv2 4.70     <- peek( SYSCALL_TABLE_470  + HVSC_SYSCALL*8)
#define HVSC_SYSCALL_ADDR_470D		0x80000000002BC084ULL	// where above syscall is in lv2 4.70 DEX <- peek( SYSCALL_TABLE_470D + HVSC_SYSCALL*8)
#define HVSC_SYSCALL_ADDR_475		0x80000000002A0F58ULL	// where above syscall is in lv2 4.75     <- peek( SYSCALL_TABLE_475  + HVSC_SYSCALL*8)
#define HVSC_SYSCALL_ADDR_475D		0x80000000002BC0FCULL	// where above syscall is in lv2 4.75 DEX <- peek( SYSCALL_TABLE_475D + HVSC_SYSCALL*8)

#define NEW_POKE_SYSCALL			813                  	// which syscall to overwrite with new poke
#define NEW_POKE_SYSCALL_ADDR_341	0x80000000001BB93CULL	// where above syscall is in lv2 3.41
#define NEW_POKE_SYSCALL_ADDR_355	0x8000000000195A68ULL	// where above syscall is in lv2 3.55
#define NEW_POKE_SYSCALL_ADDR_355D	0x800000000019BE24ULL	// where above syscall is in lv2 3.55 DEX
#define NEW_POKE_SYSCALL_ADDR_421	0x80000000001B65C8ULL	// where above syscall is in lv2 4.21
#define NEW_POKE_SYSCALL_ADDR_421D	0x80000000001BC71CULL	// where above syscall is in lv2 4.21 DEX
#define NEW_POKE_SYSCALL_ADDR_430	0x80000000001B6950ULL	// where above syscall is in lv2 4.30
#define NEW_POKE_SYSCALL_ADDR_430D	0x80000000001BCD40ULL	// where above syscall is in lv2 4.30 DEX
#define NEW_POKE_SYSCALL_ADDR_431	0x80000000001B6958ULL	// where above syscall is in lv2 4.31
#define NEW_POKE_SYSCALL_ADDR_440	0x80000000001B5934ULL	// where above syscall is in lv2 4.40
#define NEW_POKE_SYSCALL_ADDR_441	0x80000000001B593CULL	// where above syscall is in lv2 4.41
#define NEW_POKE_SYSCALL_ADDR_441D	0x80000000001BBD2CULL	// where above syscall is in lv2 4.41 DEX
#define NEW_POKE_SYSCALL_ADDR_446	0x80000000001B5B9CULL	// where above syscall is in lv2 4.46
#define NEW_POKE_SYSCALL_ADDR_446D	0x80000000001BBF8CULL	// where above syscall is in lv2 4.46 DEX
#define NEW_POKE_SYSCALL_ADDR_450	0x80000000001A6C78ULL	// where above syscall is in lv2 4.50
#define NEW_POKE_SYSCALL_ADDR_450D	0x80000000001AD128ULL	// where above syscall is in lv2 4.50 DEX
#define NEW_POKE_SYSCALL_ADDR_453	0x80000000001A6DE0ULL	// where above syscall is in lv2 4.53
#define NEW_POKE_SYSCALL_ADDR_453D	0x80000000001AD290ULL	// where above syscall is in lv2 4.53 DEX
#define NEW_POKE_SYSCALL_ADDR_455	0x80000000001A82CCULL	// where above syscall is in lv2 4.55
#define NEW_POKE_SYSCALL_ADDR_455D	0x80000000001AE77CULL	// where above syscall is in lv2 4.55 DEX
#define NEW_POKE_SYSCALL_ADDR_460	0x80000000001A6F3CULL	// where above syscall is in lv2 4.60     <- peek( SYSCALL_TABLE_460  + NEW_POKE_SYSCALL*8)
#define NEW_POKE_SYSCALL_ADDR_460D	0x80000000001AD3ECULL	// where above syscall is in lv2 4.60 DEX <- peek( SYSCALL_TABLE_460D + NEW_POKE_SYSCALL*8)
#define NEW_POKE_SYSCALL_ADDR_465	0x80000000001A6F44ULL	// where above syscall is in lv2 4.65     <- peek( SYSCALL_TABLE_465  + NEW_POKE_SYSCALL*8)
#define NEW_POKE_SYSCALL_ADDR_465D	0x80000000001AD3F4ULL	// where above syscall is in lv2 4.65 DEX <- peek( SYSCALL_TABLE_465D + NEW_POKE_SYSCALL*8)
#define NEW_POKE_SYSCALL_ADDR_470	0x80000000002A1408ULL	// where above syscall is in lv2 4.70     <- peek( SYSCALL_TABLE_470  + NEW_POKE_SYSCALL*8)
#define NEW_POKE_SYSCALL_ADDR_470D	0x80000000002BC5ACULL	// where above syscall is in lv2 4.70 DEX <- peek( SYSCALL_TABLE_470D + NEW_POKE_SYSCALL*8)
#define NEW_POKE_SYSCALL_ADDR_475	0x80000000002A1480ULL	// where above syscall is in lv2 4.75     <- peek( SYSCALL_TABLE_475  + NEW_POKE_SYSCALL*8)
#define NEW_POKE_SYSCALL_ADDR_475D	0x80000000002BC624ULL	// where above syscall is in lv2 4.75 DEX <- peek( SYSCALL_TABLE_475D + NEW_POKE_SYSCALL*8)
*/

#define SYSCALL_TABLE_341			0x80000000002EB128ULL	// 3.41
#define SYSCALL_TABLE_355			0x8000000000346570ULL	// 3.55
#define SYSCALL_TABLE_355D			0x8000000000361578ULL	// 3.55 DEX
#define SYSCALL_TABLE_421			0x800000000035BCA8ULL	// 4.21
#define SYSCALL_TABLE_421D			0x800000000037A1B0ULL	// 4.21 DEX
#define SYSCALL_TABLE_430			0x800000000035DBE0ULL	// 4.30
#define SYSCALL_TABLE_430D			0x800000000037C068ULL	// 4.30 DEX
#define SYSCALL_TABLE_431			0x800000000035DBE0ULL	// 4.31
#define SYSCALL_TABLE_440			0x800000000035E260ULL	// 4.40
#define SYSCALL_TABLE_441			0x800000000035E260ULL	// 4.41
#define SYSCALL_TABLE_441D			0x800000000037C9E8ULL	// 4.41 DEX
#define SYSCALL_TABLE_446			0x800000000035E860ULL	// 4.46
#define SYSCALL_TABLE_446D			0x800000000037CFE8ULL	// 4.46 DEX
#define SYSCALL_TABLE_450			0x800000000035F0D0ULL	// 4.50
#define SYSCALL_TABLE_450D			0x8000000000383658ULL	// 4.50 DEX
#define SYSCALL_TABLE_453			0x800000000035F300ULL	// 4.53
#define SYSCALL_TABLE_453D			0x8000000000385108ULL	// 4.53 DEX
#define SYSCALL_TABLE_455			0x8000000000362680ULL	// 4.55
#define SYSCALL_TABLE_455D			0x8000000000388488ULL	// 4.55 DEX
#define SYSCALL_TABLE_460			0x8000000000363A18ULL	// 4.60
#define SYSCALL_TABLE_460D			0x800000000038A120ULL	// 4.60 DEX
#define SYSCALL_TABLE_465			0x8000000000363A18ULL	// 4.65
#define SYSCALL_TABLE_465D			0x800000000038A120ULL	// 4.65 DEX
#define SYSCALL_TABLE_470			0x8000000000363B60ULL	// 4.70
#define SYSCALL_TABLE_470D			0x800000000038A368ULL	// 4.70 DEX
#define SYSCALL_TABLE_475			0x8000000000363BE0ULL	// 4.75
#define SYSCALL_TABLE_475D			0x800000000038A3E8ULL	// 4.75 DEX

#define SYSCALL_PTR(n)				( (SYSCALL_TABLE) + ( 8 * (n) ) )

#define HV_BASE						0x8000000014000000ULL	// where in lv2 to map lv1
#define HV_SIZE						0x001000				// 0x1000 (we need 4k from lv1 only)
#define HV_PAGE_SIZE				0x0C					// 4k = 0x1000 (1 << 0x0c)
#define	HV_START_OFFSET				0x363000				// remove lv2 protection
#define HV_OFFSET					0x000A78				// at address 0x363a78
#define HV_START_OFFSET_421			0x370A28				// 4.21 lv2 protection
#define HV_START_OFFSET_430			0x370AA8				// 4.30 lv2 protection
#define HV_START_OFFSET_440			0x370AA8				// 4.40 lv2 protection
#define HV_START_OFFSET_450			0x370AA8				// 4.50 lv2 protection
#define HV_START_OFFSET_455			0x370F28				// 4.55 lv2 protection
#define HV_START_OFFSET_460			0x370F28				// 4.60 lv2 protection
#define HV_START_OFFSET_465			0x370F28				// 4.65 lv2 protection
#define HV_START_OFFSET_470			0x370F28				// 4.70 lv2 protection
#define HV_START_OFFSET_475			0x370F28				// 4.75 lv2 protection

/*
#define	HV_START_OFFSET2			0x16f000				// set lv2 access rights for sys_storage
															// at address 0x16f000 (3.55)

#define	HV_START_OFFSET2_421		0x16F758				// set lv2 access rights for sys_storage
															// at address 0x16f758 (4.21)

#define	HV_START_OFFSET2_430		0x16FA60				// set lv2 access rights for sys_storage
															// at address 0x16FA60 (4.30)

#define	HV_START_OFFSET2_440		0x16FA60				// set lv2 access rights for sys_storage
															// at address 0x16FA60 (4.40)

#define	HV_START_OFFSET2_450		0x16FA60				// set lv2 access rights for sys_storage
															// at address 0x16FA60 (4.50)

#define	HV_START_OFFSET2_455		0x16FA60				// set lv2 access rights for sys_storage
															// at address 0x16FA60 (4.55)

#define	HV_START_OFFSET2_460		0x16FA60				// set lv2 access rights for sys_storage
															// at address 0x16FA60 (4.60)

#define	HV_START_OFFSET2_465		0x16FA60				// set lv2 access rights for sys_storage
															// at address 0x16FA60 (4.65)

#define	HV_START_OFFSET2_470		0x16FA60				// set lv2 access rights for sys_storage
															// at address 0x16FA60 (4.70)

#define	HV_START_OFFSET2_475		0x16FA60				// set lv2 access rights for sys_storage
															// at address 0x16FA60 (4.75)

#define HTAB_BASE					0x800000000f000000ULL

#define HTAB_LV2_START_355			(0x01000000ULL)
#define HTAB_LV2_START_355D			(0x08000000ULL)
#define HTAB_LV2_START_421			(0x01000000ULL)
#define HTAB_LV2_START_421D			(0x08000000ULL)
#define HTAB_LV2_START_430			(0x01000000ULL)
#define HTAB_LV2_START_430D			(0x08000000ULL)
#define HTAB_LV2_START_431			(0x01000000ULL)
#define HTAB_LV2_START_440			(0x01000000ULL)
#define HTAB_LV2_START_441			(0x01000000ULL)
#define HTAB_LV2_START_441D			(0x08000000ULL)
#define HTAB_LV2_START_446			(0x01000000ULL)
#define HTAB_LV2_START_446D			(0x08000000ULL)
#define HTAB_LV2_START_450			(0x01000000ULL)
#define HTAB_LV2_START_450D			(0x08000000ULL)
#define HTAB_LV2_START_453			(0x01000000ULL)
#define HTAB_LV2_START_453D			(0x08000000ULL)
#define HTAB_LV2_START_455			(0x01000000ULL)
#define HTAB_LV2_START_455D			(0x08000000ULL)
#define HTAB_LV2_START_460			(0x01000000ULL)
#define HTAB_LV2_START_460D			(0x08000000ULL)
#define HTAB_LV2_START_465			(0x01000000ULL)
#define HTAB_LV2_START_465D			(0x08000000ULL)
#define HTAB_LV2_START_470			(0x01000000ULL)
#define HTAB_LV2_START_470D			(0x08000000ULL)
#define HTAB_LV2_START_475			(0x01000000ULL)
#define HTAB_LV2_START_475D			(0x08000000ULL)
*/
#define MAP_BASE (0x80000000007FAE00ULL)		// lv2 memory area to store path redirections table
#define	MAP_ADDR (0xE8)							// address in lv2 to store MAP_BASE address (0x0000 ---> 0xFFFF)

#define SYSCALLS_UNAVAILABLE    0xFFFFFFFF80010003ULL
#define TOC                     0x8000000000003000ULL

#endif /* __COMMON_H__ */

