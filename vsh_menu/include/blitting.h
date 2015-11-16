#ifndef __BLITT_H__
#define __BLITT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "vsh_exports.h"


// font constants
#define FONT_W         20.f            // default font width
#define FONT_H         20.f            // default font height
#define FONT_WEIGHT    1.f             // default font weight
#define FONT_CACHE_MAX 256             // max glyph cache count

// canvas constants
#define BASE          0xC0000000UL     // local memory base ea
#define CANVAS_W      720              // canvas width in pixel
#define CANVAS_H      400              // canvas height in pixel

#define PNG_MAX       4                // additional png bitmaps

// get pixel offset into framebuffer by x/y coordinates
#define OFFSET(x, y) (uint32_t)((((uint32_t)offset) + ((((int16_t)x) + \
                     (((int16_t)y) * (((uint32_t)pitch) / \
                     ((int32_t)4)))) * ((int32_t)4))) + (BASE))

extern int32_t LINE_HEIGHT;

// graphic buffers
typedef struct _Buffer {
	uint32_t *addr;               // buffer address
	int32_t  w;                   // buffer width
	int32_t  h;                   // buffer height
} Buffer;

// font cache
typedef struct _Glyph {
	uint32_t code;                           // char unicode
	CellFontGlyphMetrics metrics;            // glyph metrics
	uint16_t w;                              // image width
	uint16_t h;                              // image height
	uint8_t *image;                          // addr -> image data
} Glyph;

typedef struct _Bitmap {
	CellFontHorizontalLayout horizontal_layout;   // struct -> horizontal text layout info
	float font_w, font_h;                         // char w/h
	float_t weight, slant;                        // line weight and char slant
	int32_t distance;                             // distance between chars
	int32_t count;                                // count of current cached glyphs
	int32_t max;                                  // max glyph into this cache
	Glyph glyph[FONT_CACHE_MAX];                  // glyph struct
} Bitmap;

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

DrawCtx ctx;                                 // drawing context

void font_finalize(void);
void init_graphic(void);
int32_t load_png_bitmap(int32_t idx, const char *path);
void flip_frame(void);
void set_background_color(uint32_t color);
void set_foreground_color(uint32_t color);
void set_font(float_t font_w, float_t font_h, float_t weight, int32_t distance);
void draw_background(void);
int32_t print_text(int32_t x, int32_t y, const char *str);
int32_t draw_png(int32_t idx, int32_t c_x, int32_t c_y, int32_t p_x, int32_t p_y, int32_t w, int32_t h);

void screenshot(uint8_t mode);

//void draw_pixel(int32_t x, int32_t y);
//void draw_line(int32_t x, int32_t y, int32_t x2, int32_t y2);
//void draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
//void draw_circle(int32_t x_c, int32_t y_c, int32_t r);


#endif // __BLITT_H__
