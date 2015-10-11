#ifndef _PNG_DEC_H_
#define _PNG_DEC_H_

#include <stdlib.h>
#include <string.h>
#include <cell/codec/pngdec.h>

#include "blitting.h"


// memory callback's
typedef struct{
	uint32_t mallocCallCounts;
	uint32_t freeCallCounts;
} Cb_Arg;

// decoder context
typedef struct{
	CellPngDecMainHandle main_h;             // decoder
	CellPngDecSubHandle sub_h;               // stream
	Cb_Arg cb_arg;                           // callback arg
} png_dec_info;



Buffer load_png(const char *file_path);


#endif // _PNG_DEC_H_
