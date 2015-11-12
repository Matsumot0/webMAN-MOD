#ifndef __ISO9660GEN_H__
#define __ISO9660GEN_H__

#include <stdio.h>
#include <stdint.h>
#include "iso9660.h"

enum
{
	ISO9660_GEN_ERROR_OUTPUT = -0x8000,
	ISO9660_GEN_ERROR_INPUT,
	IS09660_GEN_ERROR_SETUP
};

typedef struct _FileList
{
	char *path;
	uint32_t rlba;
	off64_t size;
	bool multipart;
	struct _FileList *next;
} FileList;

typedef struct _DirList
{
	char *path;
	uint8_t *content;
	uint8_t *contentJoliet;
	size_t contentSize;
	size_t contentJolietSize;
	int idx;
	FileList *fileList;
	struct _DirList *next;
} DirList;

#ifdef __BIG_ENDIAN__

static inline uint16_t LE16(uint16_t val)
{
	return ((val&0xff)<<8) | (val >> 8);
}

static inline uint16_t BE16(uint16_t val)
{
	return val;
}

static inline uint32_t LE32(uint32_t val)
{
	return ((((val) & 0xff) << 24) | (((val) & 0xff00) << 8) | (((val) & 0xff0000) >> 8) | (((val) >> 24) & 0xff));
}

static inline uint32_t BE32(uint32_t val)
{
	return val;
}

#else

static inline uint16_t LE16(uint16_t val)
{
	return val;
}

static inline uint16_t BE16(uint16_t val)
{
	return ((val&0xff)<<8) | (val >> 8);
}

static inline uint32_t LE32(uint32_t val)
{
	return val;
}

static inline uint32_t BE32(uint32_t val)
{
	return ((((val) & 0xff) << 24) | (((val) & 0xff00) << 8) | (((val) & 0xff0000) >> 8) | (((val) >> 24) & 0xff));
}

#endif

static inline uint32_t bytesToSectors(off64_t size)
{
	return ((size+0x7ff)&~0x7ff) / 0x800;	
}

static inline char *dupString(const char *str)
{
	char *ret = new char[strlen(str)+1];
	strcpy(ret, str);
	return ret;
}

class Iso9660Gen
{

private:
	
	void *fd;
	int currentPart;
	off64_t bytesWrittenInPart;
	uint32_t padSectors;
	bool canceled;
	
	DirList *rootList;
	
	uint32_t filesSizeSectors;
	uint32_t dirsSizeSectors;
	uint32_t dirsSizeSectorsJoliet;
	
	uint8_t *pathTableL;
	uint8_t *pathTableM;	
	uint8_t *pathTableJolietL;
	uint8_t *pathTableJolietM;
	
	size_t pathTableSize;
	size_t pathTableSizeJoliet;	
	
	void (* progress)(off64_t current, off64_t total, bool *cancelCheck);
	
	void reset(void);
	
	bool openOutputFile(void);
	void closeOutputFile(void);
	bool writeOutputFile(void *buf, off64_t size, off64_t *written);
	
	DirList *getParent(DirList *dirList);	
	bool isDirectChild(DirList *dir, DirList *parentCheck);
	Iso9660DirectoryRecord *findDirRecord(char *dirName, Iso9660DirectoryRecord *parentRecord, size_t size, bool joliet);
	
	uint8_t *buildPathTable(bool msb, bool joliet, size_t *retSize);
	bool buildContent(DirList *dirList, bool joliet);
	void fixDirLba(Iso9660DirectoryRecord *record, size_t size, uint32_t dirLba, uint32_t filesLba);
	void fixPathTableLba(uint8_t *pathTable, size_t size, uint32_t dirLba, bool msb);
	void fixLba(uint32_t isoLba, uint32_t jolietLba, uint32_t filesLba);
	bool build(const char *inDir);
	
	bool write(const char *volumeName);
	
protected:	
	
	char *outFilePath;
	int numOfParts;
	
	uint8_t *tempBuf;
	uint8_t *ioBuf;
	size_t tempBufSize;
	size_t ioBufSize;
		
	off64_t partitionSize;
	uint32_t volumeSize;
	off64_t totalSize;
		
	
public:
	Iso9660Gen();
	virtual ~Iso9660Gen();
	
	void setPartitionSize(off64_t size);
	void setProgressFunction(void (* progress)(off64_t current, off64_t total, bool *cancelCheck));
	bool setBuffers(void *tempBuf, size_t tempBufSize, void *ioBuf, size_t ioBufSize);
	virtual int generate(const char *inDir, const char *outFile, const char *volumeName);		
};


#endif /* __ISO9660GEN_H__ */

 
