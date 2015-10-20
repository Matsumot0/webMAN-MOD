#include <arpa/inet.h>

#define ssend(socket, str) send(socket, str, strlen(str), 0)

#include <stdio.h>
#include <string.h>
#include "ntfs.h"
#include "cobra.h"
#include "scsi.h"
#include <lv2/sysfs.h>
#include <sys/file.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/process.h>

#include <io/pad.h>

#define SUFIX(a)	((a==1)? "_1" :(a==2)? "_2" :(a==3)? "_3" :(a==4)? "_4" :(a==5)? " [auto]" :"")
#define SUFIX2(a)	((a==1)?" (1)":(a==2)?" (2)":(a==3)?" (3)":(a==4)?" (4)":(a==5)? " [auto]" :"")

#define USB_MASS_STORAGE_1(n)	(0x10300000000000AULL+(n)) /* For 0-5 */
#define USB_MASS_STORAGE_2(n)	(0x10300000000001FULL+((n)-6)) /* For 6-127 */
#define USB_MASS_STORAGE(n)	(((n) < 6) ? USB_MASS_STORAGE_1(n) : USB_MASS_STORAGE_2(n))

enum emu_modes
{
    PS3ISO = 0,
    BDISO  = 1,
    DVDISO = 2,
    PSXISO = 3,
};

typedef struct
{
	uint64_t device;
	uint32_t emu_mode;
	uint32_t num_sections;
	uint32_t num_tracks;
} __attribute__((packed)) rawseciso_args;

#define MAX_SECTIONS	((0x10000-sizeof(rawseciso_args))/8)

#define cue_buf  plugin_args
#define _4KB_    4096

uint8_t plugin_args[0x10000];
char path[0x420];
char wm_path[0x420];
char image_file[0x420];
//char extension[10];

uint32_t sections[MAX_SECTIONS], sections_size[MAX_SECTIONS];
ntfs_md *mounts;
int mountCount;

//----------------
#define USE_64BITS_LSEEK 1

#define SUCCESS 0
#define FAILED -1

#define SECTOR_FILL    2047
#define SECTOR_SIZE    2048
#define SECTOR_SIZELL  2048LL

#define FS_S_IFMT 0170000
#define FS_S_IFDIR 0040000

#define ISODCL(from, to) (to - from + 1)

static u16 wstring[1024];
static char temp_string[1024];

struct iso_primary_descriptor {
    unsigned char type                      [ISODCL (  1,   1)]; /* 711 */
    unsigned char id                        [ISODCL (  2,   6)];
    unsigned char version                   [ISODCL (  7,   7)]; /* 711 */
    unsigned char unused1                   [ISODCL (  8,   8)];
    unsigned char system_id                 [ISODCL (  9,  40)]; /* aunsigned chars */
    unsigned char volume_id                 [ISODCL ( 41,  72)]; /* dunsigned chars */
    unsigned char unused2                   [ISODCL ( 73,  80)];
    unsigned char volume_space_size         [ISODCL ( 81,  88)]; /* 733 */
    unsigned char unused3                   [ISODCL ( 89, 120)];
    unsigned char volume_set_size           [ISODCL (121, 124)]; /* 723 */
    unsigned char volume_sequence_number    [ISODCL (125, 128)]; /* 723 */
    unsigned char logical_block_size        [ISODCL (129, 132)]; /* 723 */
    unsigned char path_table_size           [ISODCL (133, 140)]; /* 733 */
    unsigned char type_l_path_table         [ISODCL (141, 144)]; /* 731 */
    unsigned char opt_type_l_path_table     [ISODCL (145, 148)]; /* 731 */
    unsigned char type_m_path_table         [ISODCL (149, 152)]; /* 732 */
    unsigned char opt_type_m_path_table     [ISODCL (153, 156)]; /* 732 */
    unsigned char root_directory_record     [ISODCL (157, 190)]; /* 9.1 */
    unsigned char volume_set_id             [ISODCL (191, 318)]; /* dunsigned chars */
    unsigned char publisher_id              [ISODCL (319, 446)]; /* achars */
    unsigned char preparer_id               [ISODCL (447, 574)]; /* achars */
    unsigned char application_id            [ISODCL (575, 702)]; /* achars */
    unsigned char copyright_file_id         [ISODCL (703, 739)]; /* 7.5 dchars */
    unsigned char abstract_file_id          [ISODCL (740, 776)]; /* 7.5 dchars */
    unsigned char bibliographic_file_id     [ISODCL (777, 813)]; /* 7.5 dchars */
    unsigned char creation_date             [ISODCL (814, 830)]; /* 8.4.26.1 */
    unsigned char modification_date         [ISODCL (831, 847)]; /* 8.4.26.1 */
    unsigned char expiration_date           [ISODCL (848, 864)]; /* 8.4.26.1 */
    unsigned char effective_date            [ISODCL (865, 881)]; /* 8.4.26.1 */
    unsigned char file_structure_version    [ISODCL (882, 882)]; /* 711 */
    unsigned char unused4                   [ISODCL (883, 883)];
    unsigned char application_data          [ISODCL (884, 1395)];
    unsigned char unused5                   [ISODCL (1396, 2048)];
};

struct iso_directory_record {
    unsigned char length                    [ISODCL ( 1,  1)]; /* 711 */
    unsigned char ext_attr_length           [ISODCL ( 2,  2)]; /* 711 */
    unsigned char extent                    [ISODCL ( 3, 10)]; /* 733 */
    unsigned char size                      [ISODCL (11, 18)]; /* 733 */
    unsigned char date                      [ISODCL (19, 25)]; /* 7 by 711 */
    unsigned char flags                     [ISODCL (26, 26)];
    unsigned char file_unit_size            [ISODCL (27, 27)]; /* 711 */
    unsigned char interleave                [ISODCL (28, 28)]; /* 711 */
    unsigned char volume_sequence_number    [ISODCL (29, 32)]; /* 723 */
    unsigned char name_len                  [1]; /* 711 */
    unsigned char name                      [1];
};

struct iso_path_table{
    unsigned char  name_len[2]; /* 721 */
    char extent[4];             /* 731 */
    char parent[2];             /* 721 */
    char name[1];
};

static int isonum_731 (unsigned char * p)
{
    return ((p[0] & 0xff)
         | ((p[1] & 0xff) << 8)
         | ((p[2] & 0xff) << 16)
         | ((p[3] & 0xff) << 24));
}

/*
static int isonum_732 (unsigned char * p)
{
    return ((p[3] & 0xff)
         | ((p[2] & 0xff) << 8)
         | ((p[1] & 0xff) << 16)
         | ((p[0] & 0xff) << 24));
}
*/

static int isonum_733 (unsigned char * p)
{
    return (isonum_731 (p));
}


static int isonum_721 (unsigned char * p)
{
    return ((p[0] & 0xff) | ((p[1] & 0xff) << 8));
}

void UTF16_to_UTF8(u16 *stw, u8 *stb)
{
    while(stw[0])
    {
        if((stw[0] & 0xFF80) == 0)
        {
            *(stb++) = stw[0] & 0xFF;   // utf16 00000000 0xxxxxxx utf8 0xxxxxxx
        }
        else if((stw[0] & 0xF800) == 0)
        {
            // utf16 00000yyy yyxxxxxx utf8 110yyyyy 10xxxxxx
            *(stb++) = ((stw[0]>>6) & 0xFF) | 0xC0; *(stb++) = (stw[0] & 0x3F) | 0x80;
        }
        else if((stw[0] & 0xFC00) == 0xD800 && (stw[1] & 0xFC00) == 0xDC00)
        {
            // utf16 110110ww wwzzzzyy 110111yy yyxxxxxx (wwww = uuuuu - 1)
            // utf8  1111000uu 10uuzzzz 10yyyyyy 10xxxxxx
            *(stb++)= (((stw[0] + 64)>>8) & 0x3) | 0xF0; *(stb++)= (((stw[0]>>2) + 16) & 0x3F) | 0x80;
            *(stb++)= ((stw[0]>>4) & 0x30) | 0x80 | ((stw[1]<<2) & 0xF); *(stb++)= (stw[1] & 0x3F) | 0x80;
            stw++;
        }
        else
        {
            // utf16 zzzzyyyy yyxxxxxx utf8 1110zzzz 10yyyyyy 10xxxxxx
            *(stb++)= ((stw[0]>>12) & 0xF) | 0xE0; *(stb++)= ((stw[0]>>6) & 0x3F) | 0x80; *(stb++)= (stw[0] & 0x3F) | 0x80;
        }

        stw++;
    }

    *stb = 0;
}

#ifdef USE_64BITS_LSEEK
int get_iso_file_pos(int fd, char *path, u32 *flba, u64 *size)
#else
int get_iso_file_pos(FILE *fp, unsigned char *path, u32 *flba, u64 *size)
#endif
{
    static struct iso_primary_descriptor sect_descriptor;
    struct iso_directory_record * idr;
    static int folder_split[64][3];
    int nfolder_split = 0;

    u32 file_lba = 0xffffffff;

    u8 *sectors = NULL;

    #ifdef USE_64BITS_LSEEK
    if(fd <= 0 || !size || !flba) return FAILED;
    #else
    if(!fp || !size || !flba) return FAILED;
    #endif

    *size = 0;

    folder_split[nfolder_split][0] = 0;
    folder_split[nfolder_split][1] = 0;
    folder_split[nfolder_split][2] = 0;
    int i = 0;

    while(path[i] != 0 && i < strlen(path))
    {
        if(path[i] == '/')
        {
            folder_split[nfolder_split][2] = i - folder_split[nfolder_split][1];
            while(path[i] =='/' && i < strlen(path)) i++;
            if(folder_split[nfolder_split][2] == 0) {folder_split[nfolder_split][1] = i; continue;}
            folder_split[nfolder_split][0] = 1;
            nfolder_split++;
            folder_split[nfolder_split][0] = 0;
            folder_split[nfolder_split][1] = i;
            folder_split[nfolder_split][2] = 0;

        } else i++;
    }

    folder_split[nfolder_split][0] = 0;
    folder_split[nfolder_split][2] = i - folder_split[nfolder_split][1];
    nfolder_split++;

    #ifdef USE_64BITS_LSEEK
    if(ps3ntfs_seek64(fd, 0x8800LL, SEEK_SET) != 0x8800LL) goto err;
    if(ps3ntfs_read(fd, (void *) &sect_descriptor, sizeof(struct iso_primary_descriptor)) != sizeof(struct iso_primary_descriptor)) goto err;
    #else
    if(fseek(fp, 0x8800, SEEK_SET) != 0) goto err;
    if(fread((void *) &sect_descriptor, 1, sizeof(struct iso_primary_descriptor), fp) != sizeof(struct iso_primary_descriptor)) goto err;
    #endif

    if((sect_descriptor.type[0] != 2 && sect_descriptor.type[0] != 0xFF) || strncmp((void *) sect_descriptor.id, "CD001", 5)) goto err;

    // PSP ISO
    if(sect_descriptor.type[0] == 0xFF)
    {
        unsigned char dir_entry[0x38];
        char entry_name[0x20];

        #ifdef USE_64BITS_LSEEK
        if(ps3ntfs_seek64(fd, 0xB860LL, SEEK_SET) != 0xB860LL) goto err;
        #else
        if(fseek(fp, 0xB860, SEEK_SET) != 0) goto err;
        #endif

        int c;

        for(int i = 0; i < 8; i++)
        {
            #ifdef USE_64BITS_LSEEK
            if(ps3ntfs_read(fd, (void *) &dir_entry, 0x38) != 0x38) goto err;
            #else
            if(fread((void *) &dir_entry, 1, 0x38, fp) != 0x38) goto err;
            #endif

            for(c = 0; c < dir_entry[0x20]; c++) entry_name[c] = dir_entry[0x21 + c];
            for( ; c < 0x20; c++) entry_name[c] = 0;

            if(strstr(path, entry_name))
            {
                file_lba = isonum_731(&dir_entry[2]);
                *flba = file_lba;

                *size = isonum_731(&dir_entry[10]);

                #ifdef USE_64BITS_LSEEK
                if(ps3ntfs_seek64(fd, ((s64) file_lba) * SECTOR_SIZELL, SEEK_SET) != ((s64) file_lba) * SECTOR_SIZELL) goto err;
                #else
                if(fseek(fp, file_lba * SECTOR_SIZE, SEEK_SET) != 0) goto err;
                #endif

                return SUCCESS;
            }
        }

        return -4;
    }

    u32 lba0 = isonum_731(&sect_descriptor.type_l_path_table[0]); // lba
    if(!lba0) return -3;

    u32 size0 = isonum_733(&sect_descriptor.path_table_size[0]); // size
    if(!size0) return -3;

    int size1 = ((size0 + SECTOR_FILL) / SECTOR_SIZE) * SECTOR_SIZE;
    if(size1 < 0 || size1 > 0x400000) return -3; // size larger than 4MB

    sectors = malloc(size1 + SECTOR_SIZE);
    if(!sectors) return -3;

    memset(sectors, 0, size1 + SECTOR_SIZE);

    #ifdef USE_64BITS_LSEEK
    if(ps3ntfs_seek64(fd, ((s64) lba0) * SECTOR_SIZELL, SEEK_SET) != ((s64) lba0) * SECTOR_SIZELL) goto err;
    if(ps3ntfs_read(fd, (void *) sectors, size1) != size1) goto err;
    #else
    if(fseek(fp, lba0 * SECTOR_SIZE, SEEK_SET) != 0) goto err;
    if(fread((void *) sectors, 1, size1, fp) != size1) goto err;
    #endif

    u32 p = 0;

    u32 lba_folder = 0xffffffff;
    u32 lba  = 0xffffffff;

    int nsplit = 0;
    int last_parent = 1;
    int cur_parent = 1;

    while(p < size0)
    {
        if(nsplit >= nfolder_split) break;
        if(folder_split[nsplit][0] == 0 && nsplit != 0) {lba_folder = lba; break;}
        if(folder_split[nsplit][2] == 0) continue;

        u32 snamelen = isonum_721(&sectors[p]);
        if(snamelen == 0) p = ((p / SECTOR_SIZE) * SECTOR_SIZE) + SECTOR_SIZE; //break;
        p += 2;
        lba = isonum_731(&sectors[p]);
        p += 4;
        u32 parent_name = isonum_721(&sectors[p]);
        p += 2;

        memset(wstring, 0, 512 * 2);
        memcpy(wstring, &sectors[p], snamelen);

        UTF16_to_UTF8(wstring, (u8 *) temp_string);

        if(cur_parent == 1 && folder_split[nsplit][0] == 0 && nsplit == 0) {lba_folder = lba; break;}

        if(last_parent == parent_name && strlen(temp_string) == folder_split[nsplit][2] &&
           !strncmp((void *) temp_string, &path[folder_split[nsplit][1]], folder_split[nsplit][2]))
        {

            //DPrintf("p: %s %u %u\n", &path[folder_split[nsplit][1]], folder_split[nsplit][2], snamelen);
            last_parent = cur_parent;

            nsplit++;
            if(folder_split[nsplit][0] == 0) {lba_folder = lba; break;}
        }

        p += snamelen;
        cur_parent++;
        if(snamelen & 1) {p++;}
    }

    if(lba_folder == 0xffffffff) goto err;

    memset(sectors, 0, _4KB_);

    #ifdef USE_64BITS_LSEEK
    if(ps3ntfs_seek64(fd, ((s64) lba_folder) * SECTOR_SIZELL, SEEK_SET) != ((s64) lba_folder) * SECTOR_SIZELL) goto err;
    if(ps3ntfs_read(fd, (void *) sectors, SECTOR_SIZE) != SECTOR_SIZE) goto err;
    #else
    if(fseek(fp, lba_folder * SECTOR_SIZE, SEEK_SET) != 0) goto err;
    if(fread((void *) sectors, 1, SECTOR_SIZE, fp) != SECTOR_SIZE) goto err;
    #endif

    int size_directory = -1;
    int p2 = 0;

    p = 0;
    while(true)
    {
        if(nsplit >= nfolder_split) break;
        idr = (struct iso_directory_record *) &sectors[p];

        if(size_directory == -1)
        {
            if((int) idr->name_len[0] == 1 && idr->name[0] == 0 && lba == isonum_731((void *) idr->extent) && idr->flags[0] == 0x2)
            {
                size_directory = isonum_733((void *) idr->size);
            }
        }

        if(idr->length[0] == 0 && sizeof(struct iso_directory_record) + p > SECTOR_SIZE)
        {
            lba_folder++;

            #ifdef USE_64BITS_LSEEK
            if(ps3ntfs_seek64(fd, ((s64) lba_folder) * SECTOR_SIZELL, SEEK_SET) != ((s64) lba_folder) * SECTOR_SIZELL) goto err;
            if(ps3ntfs_read(fd, (void *) sectors, SECTOR_SIZE) != SECTOR_SIZE) goto err;
            #else
            if(fseek(fp, lba_folder * SECTOR_SIZE, SEEK_SET) != 0) goto err;
            if(fread((void *) sectors, 1, SECTOR_SIZE, fp) != SECTOR_SIZE) goto err;
            #endif

            p = 0; p2 = (p2 & ~SECTOR_FILL) + SECTOR_SIZE;

            idr = (struct iso_directory_record *) &sectors[p];
            if((int) idr->length[0] == 0) break;
            if((size_directory == -1 && idr->length[0] == 0) || (size_directory != -1 && p2 >= size_directory)) break;
            continue;
        }

        if((size_directory == -1 && idr->length[0] == 0) || (size_directory != -1 && p2 >= size_directory)) break;

        if((int) idr->length[0] == 0) break;

        memset(wstring, 0, 512 * 2);
        memcpy(wstring, (char *) idr->name, idr->name_len[0]);

        UTF16_to_UTF8(wstring, (u8 *) temp_string);

        if(strlen(temp_string) == folder_split[nsplit][2] &&
          !strncmp((char *) temp_string, &path[folder_split[nsplit][1]], (int) folder_split[nsplit][2]))
        {
            if(file_lba == 0xffffffff) file_lba = isonum_733(&idr->extent[0]);

            *size+= (u64) (u32) isonum_733(&idr->size[0]);

        } else if(file_lba != 0xffffffff) break;

        p += idr->length[0]; p2 += idr->length[0];
    }

    *flba = file_lba;

    if(file_lba == 0xffffffff) goto err;

    #ifdef USE_64BITS_LSEEK
    if(ps3ntfs_seek64(fd, ((s64) file_lba) * SECTOR_SIZELL, SEEK_SET) != ((s64) file_lba) * SECTOR_SIZELL) goto err;
    #else
    if(fseek(fp, file_lba * SECTOR_SIZE, SEEK_SET) != 0) goto err;
    #endif

    if(sectors) free(sectors);

    return SUCCESS;

err:
    if(sectors) free(sectors);

    return -4;
}

bool file_exists( char* path )
{
    struct stat st;
    return ps3ntfs_stat(path, &st) >= SUCCESS;
}

int SaveFile(char *path, char *mem, int file_size)
{
    sysLv2FsUnlink((char*)path);

    FILE *fp;

    fp = fopen(path, "wb");

    if (fp)
    {
        if(file_size != fwrite((void *) mem, 1, file_size, fp))
        {
            fclose(fp);
            return FAILED;
        }
        fclose(fp);
    }
    else
        return FAILED;

    sysLv2FsChmod(path, FS_S_IFMT | 0777);

    return SUCCESS;
}

static void get_titleid(char *filename, char *titleID)
{
	u16 pos, str, dat, indx=0;

	char paramsfo[_4KB_];
	unsigned char *mem = (u8*)paramsfo;

	int fd = ps3ntfs_open(filename, O_RDONLY, 0);
	if(fd >= 0)
	{
        uint64_t size = _4KB_;
        ps3ntfs_read(fd, (void *) paramsfo, size);
        ps3ntfs_close(fd);
	}
    else return;

	str=(mem[0x8]+(mem[0x9]<<8));
	dat=pos=(mem[0xc]+(mem[0xd]<<8));
	memset(titleID, 0, 16);

	while(str<4090)
	{
		if((mem[str]==0) || (str>=dat)) break;

		if(!memcmp((char *) &mem[str], "TITLE_ID", 8))
		{
			strncpy(titleID, (char *) &mem[pos], 9); break;
		}

		while(mem[str]) str++;str++;
		pos+=(mem[0x1c+indx]+(mem[0x1d+indx]<<8));
		indx+=0x10;
	}
}

int ExtractFileFromISO(char *iso_file, char *file, char *outfile)
{
    int fd = ps3ntfs_open(iso_file, O_RDONLY, 0);
    if(fd >= 0)
    {
        u32 flba;
        u64 size;
        char *mem = NULL;

        int re = get_iso_file_pos(fd, file, &flba, &size);

        if(!re && (mem = malloc(size)) != NULL)
        {
            re = ps3ntfs_read(fd, (void *) mem, size);
            ps3ntfs_close(fd);
            if(re == size)
              SaveFile(outfile, mem, size);

            free(mem);

            return (re == size) ? SUCCESS : FAILED;
        }
        else
            ps3ntfs_close(fd);
    }

    return FAILED;
}

u64 get_filesize(char *path)
{
    struct stat st;
    if (ps3ntfs_stat(path, &st) < 0) return 0ULL;
    return st.st_size;
}

int copy_file(char *src_file, char *out_file)
{
    u64 size=get_filesize(src_file);
    if(size==0) return FAILED;

    int fd = ps3ntfs_open(src_file, O_RDONLY, 0);
    if(fd >= 0)
    {
        char *mem = NULL;

        if((mem = malloc(size)) != NULL)
        {
            ps3ntfs_seek64(fd, 0, SEEK_SET);
            int re = ps3ntfs_read(fd, (void *) mem, size);
            ps3ntfs_close(fd);

            SaveFile(out_file, mem, re);

            free(mem);

            return (re == size) ? SUCCESS : FAILED;
        }
        else
            ps3ntfs_close(fd);
    }
    return FAILED;
}

static int connect_to_webman()
{
	struct sockaddr_in sin;
	int s;

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = 0x7F000001; //127.0.0.1 (localhost)
	sin.sin_port = htons(80);         //http port (80)
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
	{
		return FAILED;
	}

	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		return FAILED;
	}

	return s;
}

int main(int argc, const char* argv[])
{
	int i, parts;
	unsigned int num_tracks;
	u8 cue=0; int ext_len = 4;

	int emu_mode;
	TrackDef tracks[100];
	ScsiTrackDescriptor *scsi_tracks;

	rawseciso_args *p_args;

	sysFSDirent dir;
    DIR_ITER *pdir = NULL, *psubdir= NULL;
    struct stat st;
	char c_path[4][8]={"PS3ISO", "BDISO", "DVDISO", "PSXISO"};
	char cover_ext[4][8]={".jpg", ".png", ".PNG", ".JPG"};

	snprintf(path, sizeof(path), "/dev_hdd0/tmp/wmtmp");
	mkdir(path, S_IRWXO | S_IRWXU | S_IRWXG | S_IFDIR);
	sysFsChmod(path, S_IFDIR | 0777);

	sysLv2FsUnlink((char*)"/dev_hdd0/tmp/wmtmp/games.html");

	int fd=-1;
	u64 read = 0;
	char path0[512], subpath[512];
	char direntry[1024];
	char filename[512];
	bool is_iso = false;

	bool mmCM_found = false; char mmCM_cache[512], mmCM_path[512], titleID[16];

	sprintf(mmCM_cache, "%s", "/dev_hdd0/game/BLES80608/USRDIR/cache");
	if(file_exists(mmCM_cache)) mmCM_found=true;
	else
	{sprintf(mmCM_cache, "%s", "/dev_hdd0/game/NPEA00374/USRDIR/cache");
	if(file_exists(mmCM_cache)) mmCM_found=true;
	else
	{sprintf(mmCM_cache, "%s", "/dev_hdd0/tmp/game_repo/main/cache");
	if(file_exists(mmCM_cache)) mmCM_found=true;
	}
	}


//--- hold CROSS to keep previous cached files
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
                if (paddata.len > 0) break;
            }
        }
        if(button) break; else usleep(20000);
    }
    ioPadEnd();

    if(button) ; // skip removal of .ntfs[ files
    else
//---

    {
		sysLv2FsOpenDir(path, &fd);
		if(fd>=0)
		{
			while(!sysLv2FsReadDir(fd, &dir, &read) && read)
				if(strstr(dir.d_name, ".ntfs[")) {sprintf(path0, "%s/%s", path, dir.d_name); sysLv2FsUnlink(path0);}
			sysLv2FsCloseDir(fd);
		}
    }

	mountCount = ntfsMountAll(&mounts, NTFS_DEFAULT | NTFS_RECOVER | NTFS_READ_ONLY);
	if (mountCount <= 0) goto exit;

	for (int profile = 0; profile < 6; profile++)
	{
		for (i = 0; i < mountCount; i++)
		{
			if (strncmp(mounts[i].name, "ntfs", 4) == 0)
			{
				for(u8 m=0;m<4;m++) //0="PS3ISO", 1="BDISO", 2="DVDISO", 3="PSXISO"
				{
					bool has_dirs; u16 flen; has_dirs = false;

					snprintf(path, sizeof(path), "%s:/%s%s", mounts[i].name, c_path[m], SUFIX(profile));

					pdir = ps3ntfs_diropen(path);
					if(pdir!=NULL)
					{
						while(ps3ntfs_dirnext(pdir, dir.d_name, &st) == 0)
						{
							sprintf(direntry, "%s", dir.d_name);
							sprintf(filename, "%s", dir.d_name);
							flen = strlen(filename)-1; if(flen<3) continue;

							is_iso =	( (strcasestr(filename + flen - 3, ".iso")) /*&& (filename[flen]=='o' || filename[flen]=='O')*/ ) ||
							  (m>0 && ( ( (strcasestr(filename + flen - 3, ".bin")) /*&& (filename[flen]=='n' || filename[flen]=='N')*/ ) ||
										( (strcasestr(filename + flen - 3, ".img")) /*&& (filename[flen]=='g' || filename[flen]=='G')*/ ) ||
										( (strcasestr(filename + flen - 3, ".mdf")) /*&& (filename[flen]=='f' || filename[flen]=='F')*/ ) ));

							if(!is_iso) {ext_len = 6; is_iso = (flen>5 && strcasestr(filename + flen - 5, ".iso.0"));} else ext_len = 4;

////////////////////////////////////////////////////////
							if(!is_iso)
							{
								sprintf(subpath, "%s:/%s%s/%s", mounts[i].name, c_path[m], SUFIX(profile), dir.d_name);
								psubdir = ps3ntfs_diropen(subpath);
								if(psubdir==NULL) continue;
								sprintf(subpath, "%s", filename); has_dirs=true;
next_ntfs_entry:
								if(ps3ntfs_dirnext(psubdir, dir.d_name, &st) < 0) {has_dirs=false; continue;}
								if(dir.d_name[0]=='.') goto next_ntfs_entry;

								sprintf(direntry, "%s/%s", subpath, dir.d_name);
								sprintf(filename, "[%s] %s", subpath, dir.d_name);
								flen = strlen(filename)-1; if(flen<3) goto next_ntfs_entry;

								is_iso =	( (strcasestr(filename + flen - 3, ".iso")) /*&& (filename[flen]=='o' || filename[flen]=='O')*/ ) ||
								  (m>0 && ( ( (strcasestr(filename + flen - 3, ".bin")) /*&& (filename[flen]=='n' || filename[flen]=='N')*/ ) ||
											( (strcasestr(filename + flen - 3, ".img")) /*&& (filename[flen]=='g' || filename[flen]=='G')*/ ) ||
											( (strcasestr(filename + flen - 3, ".mdf")) /*&& (filename[flen]=='f' || filename[flen]=='F')*/ ) ));

								if(!is_iso) {ext_len = 6; is_iso = (flen>5 && strcasestr(filename + flen - 5, ".iso.0"));} else ext_len = 4;
							}
////////////////////////////////////////////////////////

							if( is_iso )
							{
								filename[strlen(filename)-ext_len]=0;
								snprintf(path, sizeof(path), "%s:/%s%s/%s", mounts[i].name, c_path[m], SUFIX(profile), direntry);

								if(m==PS3ISO)
								{
									titleID[0]=0;

									sprintf(wm_path, "/dev_hdd0/tmp/wmtmp/%s.SFO", filename);
									if(file_exists(wm_path)==false)
										ExtractFileFromISO(path, "/PS3_GAME/PARAM.SFO;1", wm_path);

									if(mmCM_found && titleID[0]>' ')
									{
										get_titleid(wm_path, titleID);
										sprintf(mmCM_path, "%s/%s.SFO", mmCM_cache, titleID);
										if(file_exists(mmCM_path)==false)
											sysLv2FsLink(wm_path, mmCM_path);
									}

									sprintf(wm_path, "/dev_hdd0/tmp/wmtmp/%s.PNG", filename);
									if(file_exists(wm_path)==false)
										ExtractFileFromISO(path, "/PS3_GAME/ICON0.PNG;1", wm_path);

									if(mmCM_found && titleID[0]>' ' && file_exists(wm_path))
									{
										sprintf(mmCM_path, "%s/%s_320.PNG", mmCM_cache, titleID);
										if(file_exists(mmCM_path)==false)
											sysLv2FsLink(wm_path, mmCM_path);

										sprintf(mmCM_path, "%s/%s_1920.PNG", mmCM_cache, titleID);
										if(file_exists(mmCM_path)==false)
											ExtractFileFromISO(path, "/PS3_GAME/PIC1.PNG;1", mmCM_path);
									}
								}
								else
								{
									int plen=0;

									for(u8 e=0;e<4;e++)
									{
										sprintf(wm_path, "/dev_hdd0/tmp/wmtmp/%s%s", filename, cover_ext[e]);
										if(file_exists(wm_path)) break;

										if(plen==0)
										{
											sprintf(image_file, "%s", path);
											plen=strlen(image_file)-4;
										}

										image_file[plen]=0; strcat(image_file, cover_ext[e]);

										copy_file(image_file, wm_path);
										if(file_exists(wm_path)) break;
									}
								}

								parts = ps3ntfs_file_to_sectors(path, sections, sections_size, MAX_SECTIONS, 1);

								if(ext_len==6)
								{
									char iso_name[0x420], iso_path[0x420];

									sprintf(iso_name, "%s", path);
									iso_name[strlen(iso_name) - 1] = 0;

									for (int o = 1; o < 64; o++)
									{
										if(parts >= MAX_SECTIONS) break;

										sprintf(iso_path, "%s%i", iso_name, o);
										if(file_exists(iso_path) == false) break;

										parts += ps3ntfs_file_to_sectors(iso_path, sections + parts, sections_size + parts, MAX_SECTIONS - parts, 1);
									}
								}

								if (parts == MAX_SECTIONS)
								{
									continue;
								}
								else if (parts > 0)
								{
									num_tracks = 1;
									if(m==PS3ISO) emu_mode = EMU_PS3; else
									if(m==BDISO ) emu_mode = EMU_BD;  else
									if(m==DVDISO) emu_mode = EMU_DVD; else
									if(m==PSXISO)
									{
										emu_mode = EMU_PSX;
										cue=0;
										int fd, cd_sector_size; cd_sector_size = 2352;

										// detect sector size
										fd = ps3ntfs_open(path, O_RDONLY, 0);
										if(fd >= 0)
										{
											char buffer[0x10]; memset(buffer, 0xF, 0);
											ps3ntfs_seek64(fd, 0x9320LL, SEEK_SET); ps3ntfs_read(fd, (char *)buffer, 0xD); if(memcmp(buffer, "PLAYSTATION ", 0xC)==0) cd_sector_size = 2352; else {
											ps3ntfs_seek64(fd, 0x8020LL, SEEK_SET); ps3ntfs_read(fd, (char *)buffer, 0xD); if(memcmp(buffer, "PLAYSTATION ", 0xC)==0) cd_sector_size = 2048; else {
											ps3ntfs_seek64(fd, 0x9920LL, SEEK_SET); ps3ntfs_read(fd, (char *)buffer, 0xD); if(memcmp(buffer, "PLAYSTATION ", 0xC)==0) cd_sector_size = 2448; else {
											ps3ntfs_seek64(fd, 0x9220LL, SEEK_SET); ps3ntfs_read(fd, (char *)buffer, 0xD); if(memcmp(buffer, "PLAYSTATION ", 0xC)==0) cd_sector_size = 2336; }}}
											ps3ntfs_close(fd);
										}

										path[strlen(path)-3]='C'; path[strlen(path)-2]='U'; path[strlen(path)-1]='E';
										fd = ps3ntfs_open(path, O_RDONLY, 0);
										if(fd<0)
										{
											path[strlen(path)-3]='c'; path[strlen(path)-2]='u'; path[strlen(path)-1]='e';
											fd = ps3ntfs_open(path, O_RDONLY, 0);
										}

										if (fd >= 0)
										{
											int r = ps3ntfs_read(fd, (char *)cue_buf, sizeof(cue_buf));
											ps3ntfs_close(fd);

											if (r > 0)
											{
												char dummy[64];

												if (cobra_parse_cue(cue_buf, r, tracks, 100, &num_tracks, dummy, sizeof(dummy)-1) != 0)
												{
													num_tracks=1;
													cue=0;
												}
												else
													cue=1;
											}
										}

										if(cd_sector_size != 2352) num_tracks |= cd_sector_size<<4;
									}

									p_args = (rawseciso_args *)plugin_args; memset(p_args, 0, 0x10000);
									p_args->device = USB_MASS_STORAGE((mounts[i].interface->ioType & 0xff) - '0');
									p_args->emu_mode = emu_mode;
									p_args->num_sections = parts;

									memcpy(plugin_args+sizeof(rawseciso_args), sections, parts*sizeof(uint32_t));
									memcpy(plugin_args+sizeof(rawseciso_args)+(parts*sizeof(uint32_t)), sections_size, parts*sizeof(uint32_t));

									if (emu_mode == EMU_PSX)
									{
										int max = MAX_SECTIONS - ((num_tracks*sizeof(ScsiTrackDescriptor)) / 8);

										if (parts == max)
										{
											continue;
										}

										p_args->num_tracks = num_tracks;
										scsi_tracks = (ScsiTrackDescriptor *)(plugin_args+sizeof(rawseciso_args)+(2*parts*sizeof(uint32_t)));

										if (!cue)
										{
											scsi_tracks[0].adr_control = 0x14;
											scsi_tracks[0].track_number = 1;
											scsi_tracks[0].track_start_addr = 0;
										}
										else
										{
											for (u8 j = 0; j < num_tracks; j++)
											{
												scsi_tracks[j].adr_control = (tracks[j].is_audio) ? 0x10 : 0x14;
												scsi_tracks[j].track_number = j+1;
												scsi_tracks[j].track_start_addr = tracks[j].lba;
											}
										}
									}

									FILE *flistW;
									snprintf(path, sizeof(path), "/dev_hdd0/tmp/wmtmp/%s%s.ntfs[%s]", filename, SUFIX2(profile), c_path[m]);
									flistW = fopen(path, "wb");
									if(flistW!=NULL)
									{
										fwrite(plugin_args, sizeof(plugin_args), 1, flistW);
										fclose(flistW);
										sysFsChmod(path, 0666);
									}
								}
							}
//////////////////////////////////////////////////////////////
							if(has_dirs) goto next_ntfs_entry;
//////////////////////////////////////////////////////////////
						}
						ps3ntfs_dirclose(pdir);
					}
				}
			}
		}
	}

exit:
	for (u8 u = 0; u < mountCount; u++) ntfsUnmount(mounts[u].name, 1);


	// force refresh xml
	int refresh_xml=-1;
	refresh_xml=connect_to_webman();
	if(refresh_xml>=0) ssend(refresh_xml, "GET /refresh.ps3 HTTP/1.0\r\n");

	// launch RELOAD.SELF
	char self_path[1024] = "";
	if(argc > 0 && argv)
	{
		if(!strncmp(argv[0], "/dev_hdd0/game/", 15))
		{
			int n;

			strcpy(self_path, argv[0]);

			n = 15; while(self_path[n] != '/' && self_path[n] != 0) n++;

			if(self_path[n] == '/') self_path[n] = 0;

			strcat(self_path, "/USRDIR/RELOAD.SELF");
		}
	}

	if(file_exists(self_path))
		sysProcessExitSpawn2(self_path, NULL, NULL, NULL, 0, 1001, SYS_PROCESS_SPAWN_STACK_SIZE_1M);

	return 0;
}
