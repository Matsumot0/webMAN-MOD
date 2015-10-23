/***********************************************************************
* pause/continue rsx fifo
*
* uint8_t pause    = pause fifo (1), continue fifo (0)
***********************************************************************/
#if defined(PS3_BROWSER) || defined(XMB_SCREENSHOT)
static int32_t rsx_fifo_pause(uint8_t pause)
{
	// lv2 sys_rsx_context_attribute()
	system_call_6(0x2A2, 0x55555555ULL, (uint64_t)(pause ? 2 : 3), 0, 0, 0, 0);

	return (int32_t)p1;
}
#endif

#ifdef XMB_SCREENSHOT

#include "../vsh/vsh.h"

#include <cell/font.h>

// canvas constants
#define BASE          0xC0000000UL     // local memory base ea

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

// display values
static uint32_t unk1 = 0, offset = 0, pitch = 0;
static uint32_t h = 0, w = 0;

//static DrawCtx ctx;                                 // drawing context

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


/***********************************************************************
*
***********************************************************************/
static void init_graphic(void)
{
	// get current display values
	offset = *(uint32_t*)0x60201104;      // start offset of current framebuffer
	getDisplayPitch(&pitch, &unk1);       // framebuffer pitch size
	h = getDisplayHeight();               // display height
	w = getDisplayWidth();                // display width
}

static void saveBMP(char *path)
{
	if(!strstr(path, ".bmp"))
	{
		// build file path
		CellRtcDateTime t;
		cellRtcGetCurrentClockLocalTime(&t);

		sprintf(path, "/dev_hdd0/screenshot_%02d_%02d_%02d_%02d_%02d_%02d.bmp", t.year, t.month, t.day, t.hour, t.minute, t.second);
	}

	// create bmp file
	int fd;
	if(cellFsOpen(path, CELL_FS_O_WRONLY|CELL_FS_O_CREAT|CELL_FS_O_TRUNC, &fd, NULL, 0)!=CELL_FS_SUCCEEDED) { BEEP3 ; return;}

	{ BEEP2 }

	rsx_fifo_pause(1);

	// initialize graphic
	init_graphic();

	// alloc buffers
	sys_memory_container_t mc_app = (sys_memory_container_t)-1;
	mc_app = vsh_memory_container_by_id(1);

	const int32_t mem_size = 64 * 1024; // 64 KB (bmp data and frame buffer)

	// max frame line size = 1920 pixel * 4(byte per pixel) = 7680 byte = 8 KB
	// max bmp buffer size = 1920 pixel * 3(byte per pixel) = 5760 byte = 6 KB

	sys_addr_t sys_mem = NULL;
	sys_memory_allocate_from_container(mem_size, mc_app, SYS_MEMORY_PAGE_SIZE_64K, &sys_mem);

	// calc buffer sizes
	uint32_t line_frame_size = w * 4;

	// alloc buffers
	uint64_t *line_frame = (uint64_t*)sys_mem;
	uint8_t *bmp_buf = (uint8_t*)sys_mem + line_frame_size; // start offset: 8 KB


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

	uint32_t i, k, idx, ww=w/2;

	// dump...
	for(i = h; i > 0; i--)
	{
		for(k = 0; k < ww; k++)
			line_frame[k] = *(uint64_t*)(OFFSET(k*2, i));

		// convert line from ARGB to RGB
		uint8_t *tmp_buf = (uint8_t*)line_frame;

		idx = 0;

		for(k = 0; k < w; k++)
		{
			bmp_buf[idx]   = tmp_buf[(k)*4+3];  // R
			bmp_buf[idx+1] = tmp_buf[(k)*4+2];  // G
			bmp_buf[idx+2] = tmp_buf[(k)*4+1];  // B

			idx+=3;
		}

		// write bmp data
		cellFsWrite(fd, (void *)bmp_buf, idx, 0);
	}

	// padding
	int32_t rest = (w*3) % 4, pad = 0;
	if(rest)
		pad = 4 - rest;
	cellFsLseek(fd, pad, CELL_FS_SEEK_SET, 0);

	cellFsClose(fd);
	sys_memory_free((sys_addr_t)sys_mem);

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
