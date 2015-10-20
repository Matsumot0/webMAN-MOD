#ifdef XMB_SCREENSHOT

#include "../vsh/vsh.h"

#include <cell/font.h>

// canvas constants
#define BASE          0xC0000000UL     // local memory base ea
#define CANVAS_W      720              // canvas width in pixel
#define CANVAS_H      400              // canvas height in pixel

// font constants
#define FONT_CACHE_MAX 256             // max glyph cache count

#define PNG_MAX       4                // additional png bitmaps

// get pixel offset into framebuffer by x/y coordinates
#define OFFSET(x, y) (uint32_t)((((uint32_t)offset) + ((((int16_t)x) + \
                     (((int16_t)y) * (((uint32_t)pitch) / \
                     ((int32_t)4)))) * ((int32_t)4))) + (BASE))

#define _ES32(v)((uint32_t)(((((uint32_t)v) & 0xFF000000) >> 24) | \
							              ((((uint32_t)v) & 0x00FF0000) >> 8 ) | \
							              ((((uint32_t)v) & 0x0000FF00) << 8 ) | \
							              ((((uint32_t)v) & 0x000000FF) << 24)))

// graphic buffers
typedef struct _Buffer {
	uint32_t *addr;               // buffer address
	int32_t  w;                   // buffer width
	int32_t  h;                   // buffer height
} Buffer;

// drawing context
typedef struct _DrawCtx {
	uint32_t *canvas;             // addr of canvas
	uint32_t *bg;                 // addr of background backup
	uint32_t *font_cache;         // addr of glyph bitmap cache buffer
	CellFont font;
	CellFontRenderer renderer;
	Buffer   png[PNG_MAX];        // bitmaps
	uint32_t bg_color;            // background color
	uint32_t fg_color;            // foreground color
} DrawCtx;

// display values
static uint32_t unk1 = 0, offset = 0, pitch = 0;
static int32_t h = 0, w = 0, canvas_x = 0, canvas_y = 0;

static DrawCtx ctx;                                 // drawing context

// screenshot
uint8_t bmp_header[] = {
  0x42, 0x4D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x0B, 0x00, 0x00, 0x12, 0x0B, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


#ifndef __PAF_H__

extern uint32_t paf_F476E8AA(void);  //  u32 get_display_width
#define getDisplayWidth paf_F476E8AA

extern uint32_t paf_AC984A12(void);  // u32 get_display_height
#define getDisplayHeight paf_AC984A12

extern int32_t paf_FFE0FBC9(uint32_t *pitch, uint32_t *unk1);  // unk1 = 0x12 color bit depth? ret 0
#define getDisplayPitch paf_FFE0FBC9

#endif // __PAF_H__


#define MB(x)      ((x)*1024*1024)

static sys_memory_container_t mc_app = (sys_memory_container_t)-1;
static sys_addr_t heap_mem = 0;
static uint32_t prx_heap = 0;

/***********************************************************************
* create prx heap from memory container 1("app")
***********************************************************************/
static void create_heap(int32_t size)
{
  mc_app = vsh_memory_container_by_id(1);
  sys_memory_allocate_from_container(MB(size), mc_app, SYS_MEMORY_PAGE_SIZE_1M, &heap_mem);
  prx_heap = (uint32_t)heap_mem;
}

/***********************************************************************
*
***********************************************************************/
static void destroy_heap(void)
{
  sys_memory_free((sys_addr_t)heap_mem);
  prx_heap = 0;
  mc_app = (sys_memory_container_t)-1;
}

/***********************************************************************
*
***********************************************************************/
static void *mem_alloc(uint32_t size)
{
	uint32_t add = prx_heap;
	prx_heap += size;

	return (void*)add;
}

/***********************************************************************
* pause/continue rsx fifo
*
* uint8_t pause    = pause fifo (1), continue fifo (0)
***********************************************************************/
static int32_t rsx_fifo_pause(uint8_t pause)
{
	// lv2 sys_rsx_context_attribute()
	system_call_6(0x2A2, 0x55555555ULL, (uint64_t)(pause ? 2 : 3), 0, 0, 0, 0);

	return (int32_t)p1;
}

/***********************************************************************
*
***********************************************************************/
static void init_graphic(void)
{
	memset(&ctx, 0, sizeof(DrawCtx));

	// set drawing context
	ctx.canvas     = mem_alloc(CANVAS_W * CANVAS_H * 4);  // canvas buffer
	ctx.bg         = mem_alloc(CANVAS_W * CANVAS_H * 4);  // background buffer
	ctx.font_cache = mem_alloc(FONT_CACHE_MAX * 32 * 32); // glyph bitmap cache
	ctx.bg_color   = 0xFF000000;                          // black, opaque
	ctx.fg_color   = 0xFFFFFFFF;                          // white, opaque

	//font_init();

	// get current display values
	offset = *(uint32_t*)0x60201104;      // start offset of current framebuffer
	getDisplayPitch(&pitch, &unk1);       // framebuffer pitch size
	h = getDisplayHeight();               // display height
	w = getDisplayWidth();                // display width

	// get x/y start coordinates for our canvas, here always center
	canvas_x = (w - CANVAS_W) / 2;
	canvas_y = (h - CANVAS_H) / 2;

	// init first frame with background dump
	memcpy((uint8_t *)ctx.canvas, (uint8_t *)ctx.bg, CANVAS_W * CANVAS_H * 4);
}

static void saveBMP(void)
{
	char path[64];

	// build file path
	CellRtcDateTime t;
	cellRtcGetCurrentClockLocalTime(&t);
	sprintf(path, "/dev_hdd0/screenshot_%02d_%02d_%02d_%02d_%02d_%02d.bmp", t.year, t.month, t.day, t.hour, t.minute, t.second);

	// create bmp file
	int fd;
	if(cellFsOpen(path, CELL_FS_O_WRONLY|CELL_FS_O_CREAT|CELL_FS_O_TRUNC, &fd, NULL, 0)!=CELL_FS_SUCCEEDED) return;

	rsx_fifo_pause(1);

	// create heap memory from memory container 1("app")
	create_heap(16);  // 16 MB

	// initialize graphic
	init_graphic();

	// alloc buffers
	sys_memory_container_t mc_app = (sys_memory_container_t)-1;
	mc_app = vsh_memory_container_by_id(1);

	const int32_t mem_size = (14 * 1024 * 1024);  // 8MB(frame dump) 6MB(bmp data)

    sys_addr_t sys_mem = NULL;
	sys_memory_allocate_from_container(mem_size, mc_app, SYS_MEMORY_PAGE_SIZE_1M, &sys_mem);

	uint64_t *dump_buf = (uint64_t*)sys_mem;
	uint8_t *bmp_buf = (uint8_t*)sys_mem + (4 * 1920 * 1080); // ABGR * HD

	int32_t i, k, idx = 0;

	// dump frame
	for(i = 0; i < h; i++)
		for(k = 0; k < w/2; k++)
			dump_buf[k + i * w/2] = *(uint64_t*)(OFFSET(k*2, i));

	// convert dump color data from ABGR to RGB
	uint8_t *tmp_buf = (uint8_t*)sys_mem;

	for(i = 0; i < h; i++)
	{
		idx = (h-1-i)*w*3;

		for(k = 0; k < w; k++)
		{
			bmp_buf[idx]   = tmp_buf[(i*w+k)*4+3];  // R
			bmp_buf[idx+1] = tmp_buf[(i*w+k)*4+2];  // G
			bmp_buf[idx+2] = tmp_buf[(i*w+k)*4+1];  // B

			idx+=3;
		}
	}

	// set bmp header
	uint32_t tmp = 0;
	tmp = _ES32(w*h*3+0x36);
	memcpy(bmp_header + 2 , &tmp, 4);     // file size
	tmp = _ES32(w);
	memcpy(bmp_header + 18, &tmp, 4);     // bmp width
	tmp = _ES32(h);
	memcpy(bmp_header + 22, &tmp, 4);     // bmp height
	tmp = _ES32(w*h*3);
	memcpy(bmp_header + 34, &tmp, 4);     // bmp data size

	// write bmp header
	cellFsWrite(fd, (void *)bmp_header, sizeof(bmp_header), 0);

	// write bmp data
	cellFsWrite(fd, (void *)bmp_buf, (w*h*3), 0);

	// padding
	int32_t rest = (w*3) % 4, pad = 0;
	if(rest)
		pad = 4 - rest;
	cellFsLseek(fd, pad, CELL_FS_SEEK_SET, 0);

	cellFsClose(fd);
	sys_memory_free((sys_addr_t)sys_mem);

	// free heap memory
	destroy_heap();

	// continue rsx rendering
	rsx_fifo_pause(0);

	show_msg(path);
}

/*
#include "../vsh/system_plugin.h"

static void saveBMP()
{
	if(View_Find("game_plugin")==0) //XMB
	{
		system_interface = (system_plugin_interface *)plugin_GetInterface(View_Find("system_plugin"),1); // 1=regular xmb, 3=ingame xmb (doesnt work)

		CellRtcDateTime t;
		cellRtcGetCurrentClockLocalTime(&t);

		char bmp[0x50];
		sprintf(bmp, "/dev_hdd0/screenshot_%02d_%02d_%02d_%02d_%02d_%02d.bmp", t.year, t.month, t.day, t.hour, t.minute, t.second);

		rsx_fifo_pause(1);

		system_interface->saveBMP(bmp);

		rsx_fifo_pause(0);

		show_msg(bmp);
	}
}
*/

#endif
