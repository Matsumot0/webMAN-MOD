#ifndef __PS3ISOGEN_H__
#define __PS3ISOGEN_H__

#include "Iso9660Gen.h"

class Ps3IsoGen : public Iso9660Gen
{
private:

	char *gameCode;

public:
	
	Ps3IsoGen(const char *gameCode);
	virtual ~Ps3IsoGen();
	
	virtual int generate(const char *inDir, const char *outFile, const char *volumeName);
};

#endif /* __PS3ISOGEN_H__ */

