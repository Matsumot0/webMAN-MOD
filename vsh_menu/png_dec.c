#include "include/png_dec.h"
#include "include/mem.h"
#include "include/vsh_exports.h"

//#include "include/network.h"	// debug

static int32_t png_w = 0, png_h = 0;


static int32_t create_decoder(png_dec_info *dec_ctx);
static int32_t open_png(png_dec_info *dec_ctx, const char *file_path);
static int32_t set_dec_param(png_dec_info	*dec_ctx);
static int32_t decode_png_stream(png_dec_info	*dec_ctx, void *buf);
static void* cb_malloc(uint32_t size, void *cb_malloc_arg);
static int32_t cb_free(void *ptr, void *cb_free_arg);


/***********************************************************************
* create png decoder
***********************************************************************/
static int32_t create_decoder(png_dec_info *dec_ctx)
{
	uint32_t ret = 0;
	CellPngDecThreadInParam   in;
	CellPngDecThreadOutParam  out;

	// set params
	dec_ctx->cb_arg.mallocCallCounts	= 0;
	dec_ctx->cb_arg.freeCallCounts	  = 0;

	in.spuThreadEnable	  = CELL_PNGDEC_SPU_THREAD_DISABLE;   // ppu only
	in.ppuThreadPriority	= 512;
	in.spuThreadPriority	= 200;
	in.cbCtrlMallocFunc	  = cb_malloc;
	in.cbCtrlMallocArg	  = &dec_ctx->cb_arg;
	in.cbCtrlFreeFunc	    = cb_free;
	in.cbCtrlFreeArg		  = &dec_ctx->cb_arg;

	// create png decoder
	ret = PngDecCreate(&dec_ctx->main_h, &in, &out);

	return ret;
}

/***********************************************************************
* open png stream
***********************************************************************/
static int32_t open_png(png_dec_info *dec_ctx, const char *file_path)
{
	uint32_t ret = 0;
	CellPngDecSrc      src;
	CellPngDecOpnInfo  info;

	// set stream source
	src.srcSelect  = CELL_PNGDEC_FILE;        // source is file
	src.fileName   = file_path;               // path to source file
	src.fileOffset = 0;
	src.fileSize   = 0;
	src.streamPtr  = NULL;
	src.streamSize = 0;

	// spu thread disable
	src.spuThreadEnable = CELL_PNGDEC_SPU_THREAD_DISABLE;

	// open stream
	ret = PngDecOpen(dec_ctx->main_h, &dec_ctx->sub_h, &src, &info);

	return ret;
}

/***********************************************************************
* set decode parameter
***********************************************************************/
static int32_t set_dec_param(png_dec_info	*dec_ctx)
{
	uint32_t ret = 0;
	CellPngDecInfo      info;
	CellPngDecInParam   in;
	CellPngDecOutParam  out;

	// read png header
	PngDecReadHeader(dec_ctx->main_h, dec_ctx->sub_h, &info);

	png_w = info.imageWidth;
	png_h = info.imageHeight;

	// set decoder parameter
	in.commandPtr		    = NULL;
	in.outputMode		    = CELL_PNGDEC_TOP_TO_BOTTOM;
	in.outputColorSpace	= CELL_PNGDEC_ARGB;        // ps3 framebuffer is ARGB
	in.outputBitDepth	  = 8;
	in.outputPackFlag	  = CELL_PNGDEC_1BYTE_PER_1PIXEL;

	if((info.colorSpace == CELL_PNGDEC_GRAYSCALE_ALPHA) ||
	   (info.colorSpace == CELL_PNGDEC_RGBA) ||
	   (info.chunkInformation & 0x10))
	{
		in.outputAlphaSelect = CELL_PNGDEC_STREAM_ALPHA;
	}
	else
	{
		in.outputAlphaSelect = CELL_PNGDEC_FIX_ALPHA;
	}
	in.outputColorAlpha = 0xFF;

	ret = PngDecSetParameter(dec_ctx->main_h, dec_ctx->sub_h, &in, &out);

	return ret;
}

/***********************************************************************
* decode png stream
***********************************************************************/
static int32_t decode_png_stream(png_dec_info	*dec_ctx, void *buf)
{
	uint32_t ret = 0;
	uint8_t *out;
	CellPngDecDataCtrlParam  param;
	CellPngDecDataOutInfo    info;


	param.outputBytesPerLine = png_w * 4;
	out = (void*)buf;

	memset(out, 0, (png_w * png_h * 4));

	// decode png...
	ret = PngDecDecodeData(dec_ctx->main_h, dec_ctx->sub_h, out, &param, &info);

	return ret;
}

/***********************************************************************
* malloc callback
***********************************************************************/
static void *cb_malloc(uint32_t size, void *cb_malloc_arg)
{
	Cb_Arg *arg;
	arg = (Cb_Arg *)cb_malloc_arg;
	arg->mallocCallCounts++;
	return malloc(size);
}

/***********************************************************************
* free callback
***********************************************************************/
static int32_t cb_free(void *ptr, void *cb_free_arg)
{
	Cb_Arg *arg;
	arg = (Cb_Arg *)cb_free_arg;
	arg->freeCallCounts++;
	free(ptr);
	return 0;
}


/***********************************************************************
* decode png file
* const char *file_path  =  path to png file e.g. "/dev_hdd0/test.png"
***********************************************************************/
Buffer load_png(const char *file_path)
{
	Buffer tmp;
	png_dec_info dec_ctx;          // decryption handles
	void *buf_addr = NULL;         // buffer for decoded png data

	// create png decoder
	create_decoder(&dec_ctx);

	// open png stream
	open_png(&dec_ctx, file_path);

	// set decode parameter
	set_dec_param(&dec_ctx);

	// alloc target buffer
	buf_addr = mem_alloc(png_w * png_h * 4);

	// decode png stream, into target buffer
	decode_png_stream(&dec_ctx, buf_addr);

	// close png stream
	PngDecClose(dec_ctx.main_h, dec_ctx.sub_h);

	// destroy png decoder
	PngDecDestroy(dec_ctx.main_h);

	// store png values
	tmp.addr = (uint32_t*)buf_addr;
	tmp.w = png_w;
	tmp.h = png_h;

	return tmp;
}
