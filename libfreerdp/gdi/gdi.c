/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * GDI Library
 *
 * Copyright 2010-2011 Marc-Andre Moreau <marcandre.moreau@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <winpr/crt.h>
#include <winpr/image.h>

#include <freerdp/api.h>
#include <freerdp/log.h>
#include <freerdp/freerdp.h>
#include <freerdp/constants.h>
#include <freerdp/codec/color.h>
#include <freerdp/codec/bitmap.h>
#include <freerdp/codec/rfx.h>
#include <freerdp/codec/nsc.h>

#include <freerdp/gdi/dc.h>
#include <freerdp/gdi/pen.h>
#include <freerdp/gdi/line.h>
#include <freerdp/gdi/shape.h>
#include <freerdp/gdi/brush.h>
#include <freerdp/gdi/region.h>
#include <freerdp/gdi/bitmap.h>
#include <freerdp/gdi/palette.h>
#include <freerdp/gdi/drawing.h>
#include <freerdp/gdi/clipping.h>

#include <freerdp/gdi/gdi.h>

#include "gdi.h"

#define TAG FREERDP_TAG("gdi")

/* Ternary Raster Operation Table */
static const UINT32 rop3_code_table[] =
{
	0x00000042, /* 0 */
	0x00010289, /* DPSoon */
	0x00020C89, /* DPSona */
	0x000300AA, /* PSon */
	0x00040C88, /* SDPona */
	0x000500A9, /* DPon */
	0x00060865, /* PDSxnon */
	0x000702C5, /* PDSaon */
	0x00080F08, /* SDPnaa */
	0x00090245, /* PDSxon */
	0x000A0329, /* DPna */
	0x000B0B2A, /* PSDnaon */
	0x000C0324, /* SPna */
	0x000D0B25, /* PDSnaon */
	0x000E08A5, /* PDSonon */
	0x000F0001, /* Pn */
	0x00100C85, /* PDSona */
	0x001100A6, /* DSon */
	0x00120868, /* SDPxnon */
	0x001302C8, /* SDPaon */
	0x00140869, /* DPSxnon */
	0x001502C9, /* DPSaon */
	0x00165CCA, /* PSDPSanaxx */
	0x00171D54, /* SSPxDSxaxn */
	0x00180D59, /* SPxPDxa */
	0x00191CC8, /* SDPSanaxn */
	0x001A06C5, /* PDSPaox */
	0x001B0768, /* SDPSxaxn */
	0x001C06CA, /* PSDPaox */
	0x001D0766, /* DSPDxaxn */
	0x001E01A5, /* PDSox */
	0x001F0385, /* PDSoan */
	0x00200F09, /* DPSnaa */
	0x00210248, /* SDPxon */
	0x00220326, /* DSna */
	0x00230B24, /* SPDnaon */
	0x00240D55, /* SPxDSxa */
	0x00251CC5, /* PDSPanaxn */
	0x002606C8, /* SDPSaox */
	0x00271868, /* SDPSxnox */
	0x00280369, /* DPSxa */
	0x002916CA, /* PSDPSaoxxn */
	0x002A0CC9, /* DPSana */
	0x002B1D58, /* SSPxPDxaxn */
	0x002C0784, /* SPDSoax */
	0x002D060A, /* PSDnox */
	0x002E064A, /* PSDPxox */
	0x002F0E2A, /* PSDnoan */
	0x0030032A, /* PSna */
	0x00310B28, /* SDPnaon */
	0x00320688, /* SDPSoox */
	0x00330008, /* Sn */
	0x003406C4, /* SPDSaox */
	0x00351864, /* SPDSxnox */
	0x003601A8, /* SDPox */
	0x00370388, /* SDPoan */
	0x0038078A, /* PSDPoax */
	0x00390604, /* SPDnox */
	0x003A0644, /* SPDSxox */
	0x003B0E24, /* SPDnoan */
	0x003C004A, /* PSx */
	0x003D18A4, /* SPDSonox */
	0x003E1B24, /* SPDSnaox */
	0x003F00EA, /* PSan */
	0x00400F0A, /* PSDnaa */
	0x00410249, /* DPSxon */
	0x00420D5D, /* SDxPDxa */
	0x00431CC4, /* SPDSanaxn */
	0x00440328, /* SDna */
	0x00450B29, /* DPSnaon */
	0x004606C6, /* DSPDaox */
	0x0047076A, /* PSDPxaxn */
	0x00480368, /* SDPxa */
	0x004916C5, /* PDSPDaoxxn */
	0x004A0789, /* DPSDoax */
	0x004B0605, /* PDSnox */
	0x004C0CC8, /* SDPana */
	0x004D1954, /* SSPxDSxoxn */
	0x004E0645, /* PDSPxox */
	0x004F0E25, /* PDSnoan */
	0x00500325, /* PDna */
	0x00510B26, /* DSPnaon */
	0x005206C9, /* DPSDaox */
	0x00530764, /* SPDSxaxn */
	0x005408A9, /* DPSonon */
	0x00550009, /* Dn */
	0x005601A9, /* DPSox */
	0x00570389, /* DPSoan */
	0x00580785, /* PDSPoax */
	0x00590609, /* DPSnox */
	0x005A0049, /* DPx */
	0x005B18A9, /* DPSDonox */
	0x005C0649, /* DPSDxox */
	0x005D0E29, /* DPSnoan */
	0x005E1B29, /* DPSDnaox */
	0x005F00E9, /* DPan */
	0x00600365, /* PDSxa */
	0x006116C6, /* DSPDSaoxxn */
	0x00620786, /* DSPDoax */
	0x00630608, /* SDPnox */
	0x00640788, /* SDPSoax */
	0x00650606, /* DSPnox */
	0x00660046, /* DSx */
	0x006718A8, /* SDPSonox */
	0x006858A6, /* DSPDSonoxxn */
	0x00690145, /* PDSxxn */
	0x006A01E9, /* DPSax */
	0x006B178A, /* PSDPSoaxxn */
	0x006C01E8, /* SDPax */
	0x006D1785, /* PDSPDoaxxn */
	0x006E1E28, /* SDPSnoax */
	0x006F0C65, /* PDSxnan */
	0x00700CC5, /* PDSana */
	0x00711D5C, /* SSDxPDxaxn */
	0x00720648, /* SDPSxox */
	0x00730E28, /* SDPnoan */
	0x00740646, /* DSPDxox */
	0x00750E26, /* DSPnoan */
	0x00761B28, /* SDPSnaox */
	0x007700E6, /* DSan */
	0x007801E5, /* PDSax */
	0x00791786, /* DSPDSoaxxn */
	0x007A1E29, /* DPSDnoax */
	0x007B0C68, /* SDPxnan */
	0x007C1E24, /* SPDSnoax */
	0x007D0C69, /* DPSxnan */
	0x007E0955, /* SPxDSxo */
	0x007F03C9, /* DPSaan */
	0x008003E9, /* DPSaa */
	0x00810975, /* SPxDSxon */
	0x00820C49, /* DPSxna */
	0x00831E04, /* SPDSnoaxn */
	0x00840C48, /* SDPxna */
	0x00851E05, /* PDSPnoaxn */
	0x008617A6, /* DSPDSoaxx */
	0x008701C5, /* PDSaxn */
	0x008800C6, /* DSa */
	0x00891B08, /* SDPSnaoxn */
	0x008A0E06, /* DSPnoa */
	0x008B0666, /* DSPDxoxn */
	0x008C0E08, /* SDPnoa */
	0x008D0668, /* SDPSxoxn */
	0x008E1D7C, /* SSDxPDxax */
	0x008F0CE5, /* PDSanan */
	0x00900C45, /* PDSxna */
	0x00911E08, /* SDPSnoaxn */
	0x009217A9, /* DPSDPoaxx */
	0x009301C4, /* SPDaxn */
	0x009417AA, /* PSDPSoaxx */
	0x009501C9, /* DPSaxn */
	0x00960169, /* DPSxx */
	0x0097588A, /* PSDPSonoxx */
	0x00981888, /* SDPSonoxn */
	0x00990066, /* DSxn */
	0x009A0709, /* DPSnax */
	0x009B07A8, /* SDPSoaxn */
	0x009C0704, /* SPDnax */
	0x009D07A6, /* DSPDoaxn */
	0x009E16E6, /* DSPDSaoxx */
	0x009F0345, /* PDSxan */
	0x00A000C9, /* DPa */
	0x00A11B05, /* PDSPnaoxn */
	0x00A20E09, /* DPSnoa */
	0x00A30669, /* DPSDxoxn */
	0x00A41885, /* PDSPonoxn */
	0x00A50065, /* PDxn */
	0x00A60706, /* DSPnax */
	0x00A707A5, /* PDSPoaxn */
	0x00A803A9, /* DPSoa */
	0x00A90189, /* DPSoxn */
	0x00AA0029, /* D */
	0x00AB0889, /* DPSono */
	0x00AC0744, /* SPDSxax */
	0x00AD06E9, /* DPSDaoxn */
	0x00AE0B06, /* DSPnao */
	0x00AF0229, /* DPno */
	0x00B00E05, /* PDSnoa */
	0x00B10665, /* PDSPxoxn */
	0x00B21974, /* SSPxDSxox */
	0x00B30CE8, /* SDPanan */
	0x00B4070A, /* PSDnax */
	0x00B507A9, /* DPSDoaxn */
	0x00B616E9, /* DPSDPaoxx */
	0x00B70348, /* SDPxan */
	0x00B8074A, /* PSDPxax */
	0x00B906E6, /* DSPDaoxn */
	0x00BA0B09, /* DPSnao */
	0x00BB0226, /* DSno */
	0x00BC1CE4, /* SPDSanax */
	0x00BD0D7D, /* SDxPDxan */
	0x00BE0269, /* DPSxo */
	0x00BF08C9, /* DPSano */
	0x00C000CA, /* PSa */
	0x00C11B04, /* SPDSnaoxn */
	0x00C21884, /* SPDSonoxn */
	0x00C3006A, /* PSxn */
	0x00C40E04, /* SPDnoa */
	0x00C50664, /* SPDSxoxn */
	0x00C60708, /* SDPnax */
	0x00C707AA, /* PSDPoaxn */
	0x00C803A8, /* SDPoa */
	0x00C90184, /* SPDoxn */
	0x00CA0749, /* DPSDxax */
	0x00CB06E4, /* SPDSaoxn */
	0x00CC0020, /* S */
	0x00CD0888, /* SDPono */
	0x00CE0B08, /* SDPnao */
	0x00CF0224, /* SPno */
	0x00D00E0A, /* PSDnoa */
	0x00D1066A, /* PSDPxoxn */
	0x00D20705, /* PDSnax */
	0x00D307A4, /* SPDSoaxn */
	0x00D41D78, /* SSPxPDxax */
	0x00D50CE9, /* DPSanan */
	0x00D616EA, /* PSDPSaoxx */
	0x00D70349, /* DPSxan */
	0x00D80745, /* PDSPxax */
	0x00D906E8, /* SDPSaoxn */
	0x00DA1CE9, /* DPSDanax */
	0x00DB0D75, /* SPxDSxan */
	0x00DC0B04, /* SPDnao */
	0x00DD0228, /* SDno */
	0x00DE0268, /* SDPxo */
	0x00DF08C8, /* SDPano */
	0x00E003A5, /* PDSoa */
	0x00E10185, /* PDSoxn */
	0x00E20746, /* DSPDxax */
	0x00E306EA, /* PSDPaoxn */
	0x00E40748, /* SDPSxax */
	0x00E506E5, /* PDSPaoxn */
	0x00E61CE8, /* SDPSanax */
	0x00E70D79, /* SPxPDxan */
	0x00E81D74, /* SSPxDSxax */
	0x00E95CE6, /* DSPDSanaxxn */
	0x00EA02E9, /* DPSao */
	0x00EB0849, /* DPSxno */
	0x00EC02E8, /* SDPao */
	0x00ED0848, /* SDPxno */
	0x00EE0086, /* DSo */
	0x00EF0A08, /* SDPnoo */
	0x00F00021, /* P */
	0x00F10885, /* PDSono */
	0x00F20B05, /* PDSnao */
	0x00F3022A, /* PSno */
	0x00F40B0A, /* PSDnao */
	0x00F50225, /* PDno */
	0x00F60265, /* PDSxo */
	0x00F708C5, /* PDSano */
	0x00F802E5, /* PDSao */
	0x00F90845, /* PDSxno */
	0x00FA0089, /* DPo */
	0x00FB0A09, /* DPSnoo */
	0x00FC008A, /* PSo */
	0x00FD0A0A, /* PSDnoo */
	0x00FE02A9, /* DPSoo */
	0x00FF0062  /* 1 */
};

/* Hatch Patterns as monochrome data */
static BYTE GDI_BS_HATCHED_PATTERNS[] =
{
	0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, /* HS_HORIZONTAL */
	0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, /* HS_VERTICAL */
	0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F, /* HS_FDIAGONAL */
	0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE, /* HS_BDIAGONAL */
	0xF7, 0xF7, 0xF7, 0x00, 0xF7, 0xF7, 0xF7, 0xF7, /* HS_CROSS */
	0x7E, 0xBD, 0xDB, 0xE7, 0xE7, 0xDB, 0xBD, 0x7E /* HS_DIACROSS */
};

/* GDI Helper Functions */

INLINE UINT32 gdi_rop3_code(BYTE code)
{
	return rop3_code_table[code];
}

UINT32 gdi_get_pixel_format(UINT32 bitsPerPixel, BOOL vFlip)
{
	UINT32 format = PIXEL_FORMAT_XRGB32_VF;

	switch (bitsPerPixel)
	{
		case 32:
			format = vFlip ? PIXEL_FORMAT_XRGB32_VF : PIXEL_FORMAT_XRGB32;
			break;

		case 24:
			format = vFlip ? PIXEL_FORMAT_RGB24_VF : PIXEL_FORMAT_RGB24;
			break;

		case 16:
			format = vFlip ? PIXEL_FORMAT_RGB16_VF : PIXEL_FORMAT_RGB16;
			break;

		case 15:
			format = vFlip ? PIXEL_FORMAT_RGB15_VF : PIXEL_FORMAT_RGB15;
			break;

		case 8:
			format = vFlip ? PIXEL_FORMAT_RGB8_VF : PIXEL_FORMAT_RGB8;
			break;
	}

	return format;
}

INLINE BYTE* gdi_get_bitmap_pointer(HGDI_DC hdcBmp, int x, int y)
{
	BYTE* p;
	HGDI_BITMAP hBmp = (HGDI_BITMAP) hdcBmp->selectedObject;
	
	if (x >= 0 && x < hBmp->width && y >= 0 && y < hBmp->height)
	{
		p = hBmp->data + (y * hBmp->width * hdcBmp->bytesPerPixel) + (x * hdcBmp->bytesPerPixel);
		return p;
	}
	else
	{
		WLog_ERR(TAG,  "gdi_get_bitmap_pointer: requesting invalid pointer: (%d,%d) in %dx%d",
			x, y, hBmp->width, hBmp->height);
		return 0;
	}
}

INLINE BYTE* gdi_get_brush_pointer(HGDI_DC hdcBrush, int x, int y)
{
	BYTE * p;

	if (hdcBrush->brush != NULL)
	{
		if ((hdcBrush->brush->style == GDI_BS_PATTERN) || (hdcBrush->brush->style == GDI_BS_HATCHED))
		{
			HGDI_BITMAP hBmpBrush = hdcBrush->brush->pattern;
	
			if (x >= 0 && y >= 0)
			{
				x = x % hBmpBrush->width;
				y = y % hBmpBrush->height;
				p = hBmpBrush->data + (y * hBmpBrush->scanline) + (x * hBmpBrush->bytesPerPixel);
				return p;
			}
		}
	}

	p = (BYTE*) &(hdcBrush->textColor);
	return p;
}

gdiBitmap* gdi_bitmap_new_ex(rdpGdi* gdi, int width, int height, int bpp, BYTE* data)
{
	gdiBitmap* bitmap;

	bitmap = (gdiBitmap*) malloc(sizeof(gdiBitmap));

	if (!bitmap)
		return NULL;

	bitmap->hdc = gdi_CreateCompatibleDC(gdi->hdc);

	DEBUG_GDI("gdi_bitmap_new: width:%d height:%d bpp:%d", width, height, bpp);

	if (!data)
		bitmap->bitmap = gdi_CreateCompatibleBitmap(gdi->hdc, width, height);
	else
		bitmap->bitmap = gdi_create_bitmap(gdi, width, height, bpp, data);

	gdi_SelectObject(bitmap->hdc, (HGDIOBJECT) bitmap->bitmap);
	bitmap->org_bitmap = NULL;

	return bitmap;
}

void gdi_bitmap_free_ex(gdiBitmap* bitmap)
{
	if (bitmap)
	{
		gdi_SelectObject(bitmap->hdc, (HGDIOBJECT) bitmap->org_bitmap);
		gdi_DeleteObject((HGDIOBJECT) bitmap->bitmap);
		gdi_DeleteDC(bitmap->hdc);
		free(bitmap);
	}
}

static void gdi_bitmap_update(rdpContext* context, BITMAP_UPDATE* bitmapUpdate)
{
	int status;
	int nXDst;
	int nYDst;
	int nXSrc;
	int nYSrc;
	int nWidth;
	int nHeight;
	int nSrcStep;
	int nDstStep;
	UINT32 index;
	BYTE* pSrcData;
	BYTE* pDstData;
	UINT32 SrcSize;
	BOOL compressed;
	UINT32 SrcFormat;
	UINT32 bitsPerPixel;
	BITMAP_DATA* bitmap;
	rdpGdi* gdi = context->gdi;
	rdpCodecs* codecs = context->codecs;

	for (index = 0; index < bitmapUpdate->number; index++)
	{
		bitmap = &(bitmapUpdate->rectangles[index]);

		nXSrc = 0;
		nYSrc = 0;

		nXDst = bitmap->destLeft;
		nYDst = bitmap->destTop;

		nWidth = bitmap->width;
		nHeight = bitmap->height;

		pSrcData = bitmap->bitmapDataStream;
		SrcSize = bitmap->bitmapLength;

		compressed = bitmap->compressed;
		bitsPerPixel = bitmap->bitsPerPixel;

		if (gdi->bitmap_size < (nWidth * nHeight * 4))
		{
			gdi->bitmap_size = nWidth * nHeight * 4;
			gdi->bitmap_buffer = (BYTE*) _aligned_realloc(gdi->bitmap_buffer, gdi->bitmap_size, 16);

			if (!gdi->bitmap_buffer)
				return;
		}

		if (compressed)
		{
			pDstData = gdi->bitmap_buffer;

			if (bitsPerPixel < 32)
			{
				freerdp_client_codecs_prepare(codecs, FREERDP_CODEC_INTERLEAVED);

				status = interleaved_decompress(codecs->interleaved, pSrcData, SrcSize, bitsPerPixel,
						&pDstData, gdi->format, -1, 0, 0, nWidth, nHeight, gdi->palette);
			}
			else
			{
				freerdp_client_codecs_prepare(codecs, FREERDP_CODEC_PLANAR);

				status = planar_decompress(codecs->planar, pSrcData, SrcSize, &pDstData,
						gdi->format, -1, 0, 0, nWidth, nHeight, TRUE);
			}

			if (status < 0)
			{
				WLog_ERR(TAG, "bitmap decompression failure");
				return;
			}

			pSrcData = gdi->bitmap_buffer;
		}
		else
		{
			pDstData = gdi->bitmap_buffer;
			SrcFormat = gdi_get_pixel_format(bitsPerPixel, TRUE);

			status = freerdp_image_copy(pDstData, gdi->format, -1, 0, 0,
						nWidth, nHeight, pSrcData, SrcFormat, -1, 0, 0, gdi->palette);

			pSrcData = gdi->bitmap_buffer;
		}

		nSrcStep = nWidth * 4;

		pDstData = gdi->primary_buffer;
		nDstStep = gdi->width * gdi->bytesPerPixel;

		nWidth = bitmap->destRight - bitmap->destLeft + 1; /* clip width */
		nHeight = bitmap->destBottom - bitmap->destTop + 1; /* clip height */

		status = freerdp_image_copy(pDstData, gdi->format, nDstStep, nXDst, nYDst,
				nWidth, nHeight, pSrcData, gdi->format, nSrcStep, nXSrc, nYSrc, gdi->palette);

		gdi_InvalidateRegion(gdi->primary->hdc, nXDst, nYDst, nWidth, nHeight);
	}
}

static void gdi_palette_update(rdpContext* context, PALETTE_UPDATE* palette)
{
	int index;
	PALETTE_ENTRY* pe;
	UINT32* palette32;
	rdpGdi* gdi = context->gdi;

	palette32 = (UINT32*) gdi->palette;

	for (index = 0; index < palette->number; index++)
	{
		pe = &(palette->entries[index]);
		palette32[index] = RGB32(pe->red, pe->green, pe->blue);
	}
}

static void gdi_set_bounds(rdpContext* context, rdpBounds* bounds)
{
	rdpGdi* gdi = context->gdi;

	if (bounds)
	{
		gdi_SetClipRgn(gdi->drawing->hdc, bounds->left, bounds->top,
				bounds->right - bounds->left + 1, bounds->bottom - bounds->top + 1);
	}
	else
	{
		gdi_SetNullClipRgn(gdi->drawing->hdc);
	}
}

static void gdi_dstblt(rdpContext* context, DSTBLT_ORDER* dstblt)
{
	rdpGdi* gdi = context->gdi;

	gdi_BitBlt(gdi->drawing->hdc, dstblt->nLeftRect, dstblt->nTopRect,
			dstblt->nWidth, dstblt->nHeight, NULL, 0, 0, gdi_rop3_code(dstblt->bRop));
}

static void gdi_patblt(rdpContext* context, PATBLT_ORDER* patblt)
{
	BYTE* data;
	rdpBrush* brush;
	UINT32 foreColor;
	UINT32 backColor;
	GDI_COLOR originalColor;
	HGDI_BRUSH originalBrush;
	rdpGdi* gdi = context->gdi;

	brush = &patblt->brush;

	foreColor = freerdp_convert_gdi_order_color(patblt->foreColor, gdi->srcBpp, gdi->format, gdi->palette);
	backColor = freerdp_convert_gdi_order_color(patblt->backColor, gdi->srcBpp, gdi->format, gdi->palette);

	originalColor = gdi_SetTextColor(gdi->drawing->hdc, foreColor);

	if (brush->style == GDI_BS_SOLID)
	{
		originalBrush = gdi->drawing->hdc->brush;

		gdi->drawing->hdc->brush = gdi_CreateSolidBrush(foreColor);

		gdi_PatBlt(gdi->drawing->hdc, patblt->nLeftRect, patblt->nTopRect,
				patblt->nWidth, patblt->nHeight, gdi_rop3_code(patblt->bRop));

		gdi_DeleteObject((HGDIOBJECT) gdi->drawing->hdc->brush);
		gdi->drawing->hdc->brush = originalBrush;
	}
	else if (brush->style == GDI_BS_HATCHED)
	{
		BYTE* hatched;
		HGDI_BITMAP hBmp;

		data = (BYTE*) _aligned_malloc(8 * 8 * gdi->bytesPerPixel, 16);

		hatched = GDI_BS_HATCHED_PATTERNS + (8 * brush->hatch);

		freerdp_image_copy_from_monochrome(data, gdi->format, -1, 0, 0, 8, 8,
				hatched, backColor, foreColor, gdi->palette);

		hBmp = gdi_CreateBitmap(8, 8, gdi->drawing->hdc->bitsPerPixel, data);

		originalBrush = gdi->drawing->hdc->brush;
		gdi->drawing->hdc->brush = gdi_CreateHatchBrush(hBmp);

		gdi_PatBlt(gdi->drawing->hdc, patblt->nLeftRect, patblt->nTopRect,
		patblt->nWidth, patblt->nHeight, gdi_rop3_code(patblt->bRop));

		gdi_DeleteObject((HGDIOBJECT) gdi->drawing->hdc->brush);
		gdi->drawing->hdc->brush = originalBrush;
	}
	else if (brush->style == GDI_BS_PATTERN)
	{
		HGDI_BITMAP hBmp;
		UINT32 brushFormat;

		if (brush->bpp > 1)
		{
			brushFormat = gdi_get_pixel_format(brush->bpp, FALSE);

			data = (BYTE*) _aligned_malloc(8 * 8 * gdi->bytesPerPixel, 16);

			freerdp_image_copy(data, gdi->format, -1, 0, 0,
					8, 8, brush->data, brushFormat, -1, 0, 0, gdi->palette);
		}
		else
		{
			data = (BYTE*) _aligned_malloc(8 * 8 * gdi->bytesPerPixel, 16);

			freerdp_image_copy_from_monochrome(data, gdi->format, -1, 0, 0, 8, 8,
					brush->data, backColor, foreColor, gdi->palette);
		}

		hBmp = gdi_CreateBitmap(8, 8, gdi->drawing->hdc->bitsPerPixel, data);

		originalBrush = gdi->drawing->hdc->brush;
		gdi->drawing->hdc->brush = gdi_CreatePatternBrush(hBmp);

		gdi_PatBlt(gdi->drawing->hdc, patblt->nLeftRect, patblt->nTopRect,
				patblt->nWidth, patblt->nHeight, gdi_rop3_code(patblt->bRop));

		gdi_DeleteObject((HGDIOBJECT) gdi->drawing->hdc->brush);
		gdi->drawing->hdc->brush = originalBrush;
	}
	else
	{
		WLog_ERR(TAG,  "unimplemented brush style:%d", brush->style);
	}

	gdi_SetTextColor(gdi->drawing->hdc, originalColor);
}

static void gdi_scrblt(rdpContext* context, SCRBLT_ORDER* scrblt)
{
	rdpGdi* gdi = context->gdi;

	gdi_BitBlt(gdi->drawing->hdc, scrblt->nLeftRect, scrblt->nTopRect,
			scrblt->nWidth, scrblt->nHeight, gdi->primary->hdc,
			scrblt->nXSrc, scrblt->nYSrc, gdi_rop3_code(scrblt->bRop));
}

static void gdi_opaque_rect(rdpContext* context, OPAQUE_RECT_ORDER* opaque_rect)
{
	GDI_RECT rect;
	HGDI_BRUSH hBrush;
	UINT32 brush_color;
	rdpGdi* gdi = context->gdi;

	gdi_CRgnToRect(opaque_rect->nLeftRect, opaque_rect->nTopRect,
			opaque_rect->nWidth, opaque_rect->nHeight, &rect);

	brush_color = freerdp_convert_gdi_order_color(opaque_rect->color, gdi->srcBpp, gdi->format, gdi->palette);

	hBrush = gdi_CreateSolidBrush(brush_color);
	gdi_FillRect(gdi->drawing->hdc, &rect, hBrush);

	gdi_DeleteObject((HGDIOBJECT) hBrush);
}

static void gdi_multi_opaque_rect(rdpContext* context, MULTI_OPAQUE_RECT_ORDER* multi_opaque_rect)
{
	int i;
	GDI_RECT rect;
	HGDI_BRUSH hBrush;
	UINT32 brush_color;
	DELTA_RECT* rectangle;
	rdpGdi* gdi = context->gdi;

	for (i = 1; i < (int) multi_opaque_rect->numRectangles + 1; i++)
	{
		rectangle = &multi_opaque_rect->rectangles[i];

		gdi_CRgnToRect(rectangle->left, rectangle->top,
				rectangle->width, rectangle->height, &rect);

		brush_color = freerdp_convert_gdi_order_color(multi_opaque_rect->color, gdi->srcBpp, gdi->format, gdi->palette);

		hBrush = gdi_CreateSolidBrush(brush_color);
		gdi_FillRect(gdi->drawing->hdc, &rect, hBrush);

		gdi_DeleteObject((HGDIOBJECT) hBrush);
	}
}

static void gdi_line_to(rdpContext* context, LINE_TO_ORDER* lineTo)
{
	UINT32 color;
	HGDI_PEN hPen;
	rdpGdi* gdi = context->gdi;

	color = freerdp_convert_gdi_order_color(lineTo->penColor, gdi->srcBpp, gdi->format, gdi->palette);
	hPen = gdi_CreatePen(lineTo->penStyle, lineTo->penWidth, (GDI_COLOR) color);
	gdi_SelectObject(gdi->drawing->hdc, (HGDIOBJECT) hPen);
	gdi_SetROP2(gdi->drawing->hdc, lineTo->bRop2);

	gdi_MoveToEx(gdi->drawing->hdc, lineTo->nXStart, lineTo->nYStart, NULL);
	gdi_LineTo(gdi->drawing->hdc, lineTo->nXEnd, lineTo->nYEnd);

	gdi_DeleteObject((HGDIOBJECT) hPen);
}

static void gdi_polyline(rdpContext* context, POLYLINE_ORDER* polyline)
{
	int i;
	INT32 x;
	INT32 y;
	UINT32 color;
	HGDI_PEN hPen;
	DELTA_POINT* points;
	rdpGdi* gdi = context->gdi;

	color = freerdp_convert_gdi_order_color(polyline->penColor, gdi->srcBpp, gdi->format, gdi->palette);
	hPen = gdi_CreatePen(GDI_PS_SOLID, 1, (GDI_COLOR) color);
	gdi_SelectObject(gdi->drawing->hdc, (HGDIOBJECT) hPen);
	gdi_SetROP2(gdi->drawing->hdc, polyline->bRop2);

	x = polyline->xStart;
	y = polyline->yStart;
	gdi_MoveToEx(gdi->drawing->hdc, x, y, NULL);

	points = polyline->points;
	for (i = 0; i < (int) polyline->numDeltaEntries; i++)
	{
		x += points[i].x;
		y += points[i].y;
		gdi_LineTo(gdi->drawing->hdc, x, y);
		gdi_MoveToEx(gdi->drawing->hdc, x, y, NULL);
	}

	gdi_DeleteObject((HGDIOBJECT) hPen);
}

static void gdi_memblt(rdpContext* context, MEMBLT_ORDER* memblt)
{
	gdiBitmap* bitmap;
	rdpGdi* gdi = context->gdi;

	bitmap = (gdiBitmap*) memblt->bitmap;

	gdi_BitBlt(gdi->drawing->hdc, memblt->nLeftRect, memblt->nTopRect,
			memblt->nWidth, memblt->nHeight, bitmap->hdc,
			memblt->nXSrc, memblt->nYSrc, gdi_rop3_code(memblt->bRop));
}

static void gdi_mem3blt(rdpContext* context, MEM3BLT_ORDER* mem3blt)
{
	BYTE* data;
	rdpBrush* brush;
	UINT32 foreColor;
	UINT32 backColor;
	gdiBitmap* bitmap;
	GDI_COLOR originalColor;
	HGDI_BRUSH originalBrush;
	rdpGdi* gdi = context->gdi;

	brush = &mem3blt->brush;
	bitmap = (gdiBitmap*) mem3blt->bitmap;

	foreColor = freerdp_convert_gdi_order_color(mem3blt->foreColor, gdi->srcBpp, gdi->format, gdi->palette);
	backColor = freerdp_convert_gdi_order_color(mem3blt->backColor, gdi->srcBpp, gdi->format, gdi->palette);

	originalColor = gdi_SetTextColor(gdi->drawing->hdc, foreColor);

	if (brush->style == GDI_BS_SOLID)
	{
		originalBrush = gdi->drawing->hdc->brush;
		gdi->drawing->hdc->brush = gdi_CreateSolidBrush(foreColor);

		gdi_BitBlt(gdi->drawing->hdc, mem3blt->nLeftRect, mem3blt->nTopRect,
				mem3blt->nWidth, mem3blt->nHeight, bitmap->hdc,
				mem3blt->nXSrc, mem3blt->nYSrc, gdi_rop3_code(mem3blt->bRop));

		gdi_DeleteObject((HGDIOBJECT) gdi->drawing->hdc->brush);
		gdi->drawing->hdc->brush = originalBrush;
	}
	else if (brush->style == GDI_BS_PATTERN)
	{
		HGDI_BITMAP hBmp;
		UINT32 brushFormat;

		if (brush->bpp > 1)
		{
			brushFormat = gdi_get_pixel_format(brush->bpp, FALSE);

			data = (BYTE*) _aligned_malloc(8 * 8 * gdi->bytesPerPixel, 16);

			freerdp_image_copy(data, gdi->format, -1, 0, 0,
					8, 8, brush->data, brushFormat, -1, 0, 0, gdi->palette);
		}
		else
		{
			data = (BYTE*) _aligned_malloc(8 * 8 * gdi->bytesPerPixel, 16);

			freerdp_image_copy_from_monochrome(data, gdi->format, -1, 0, 0, 8, 8,
					brush->data, backColor, foreColor, gdi->palette);
		}

		hBmp = gdi_CreateBitmap(8, 8, gdi->drawing->hdc->bitsPerPixel, data);

		originalBrush = gdi->drawing->hdc->brush;
		gdi->drawing->hdc->brush = gdi_CreatePatternBrush(hBmp);

		gdi_BitBlt(gdi->drawing->hdc, mem3blt->nLeftRect, mem3blt->nTopRect,
				mem3blt->nWidth, mem3blt->nHeight, bitmap->hdc,
				mem3blt->nXSrc, mem3blt->nYSrc, gdi_rop3_code(mem3blt->bRop));

		gdi_DeleteObject((HGDIOBJECT) gdi->drawing->hdc->brush);
		gdi->drawing->hdc->brush = originalBrush;
	}
	else
	{
		WLog_ERR(TAG,  "Mem3Blt unimplemented brush style:%d", brush->style);
	}

	gdi_SetTextColor(gdi->drawing->hdc, originalColor);
}

static void gdi_polygon_sc(rdpContext* context, POLYGON_SC_ORDER* polygon_sc)
{
	WLog_VRB(TAG,  "not implemented");
}

static void gdi_polygon_cb(rdpContext* context, POLYGON_CB_ORDER* polygon_cb)
{
	WLog_VRB(TAG,  "not implemented");
}

static void gdi_ellipse_sc(rdpContext* context, ELLIPSE_SC_ORDER* ellipse_sc)
{
	WLog_VRB(TAG,  "not implemented");
}

static void gdi_ellipse_cb(rdpContext* context, ELLIPSE_CB_ORDER* ellipse_cb)
{
	WLog_VRB(TAG,  "not implemented");
}

static void gdi_frame_marker(rdpContext* context, FRAME_MARKER_ORDER* frameMarker)
{


}

void gdi_surface_frame_marker(rdpContext* context, SURFACE_FRAME_MARKER* surfaceFrameMarker)
{
	DEBUG_GDI("frameId %d frameAction %d",
		surfaceFrameMarker->frameId,
		surfaceFrameMarker->frameAction);

	switch (surfaceFrameMarker->frameAction)
	{
		case SURFACECMD_FRAMEACTION_BEGIN:
			break;

		case SURFACECMD_FRAMEACTION_END:
			if (context->settings->FrameAcknowledge > 0)
			{
				IFCALL(context->update->SurfaceFrameAcknowledge, context, surfaceFrameMarker->frameId);
			}
			break;
	}
}

static void gdi_surface_bits(rdpContext* context, SURFACE_BITS_COMMAND* cmd)
{
	int i, j;
	int tx, ty;
	BYTE* pSrcData;
	BYTE* pDstData;
	RFX_MESSAGE* message;
	rdpGdi* gdi = context->gdi;

	DEBUG_GDI("destLeft %d destTop %d destRight %d destBottom %d "
		"bpp %d codecID %d width %d height %d length %d",
		cmd->destLeft, cmd->destTop, cmd->destRight, cmd->destBottom,
		cmd->bpp, cmd->codecID, cmd->width, cmd->height, cmd->bitmapDataLength);

	if (cmd->codecID == RDP_CODEC_ID_REMOTEFX)
	{
		freerdp_client_codecs_prepare(gdi->codecs, FREERDP_CODEC_REMOTEFX);

		message = rfx_process_message(gdi->codecs->rfx, cmd->bitmapData, cmd->bitmapDataLength);

		/* blit each tile */
		for (i = 0; i < message->numTiles; i++)
		{
			tx = message->tiles[i]->x + cmd->destLeft;
			ty = message->tiles[i]->y + cmd->destTop;

			pSrcData = message->tiles[i]->data;
			pDstData = gdi->tile->bitmap->data;

			if (!gdi->invert && (gdi->dstBpp == 32))
			{
				gdi->tile->bitmap->data = pSrcData;
			}
			else
			{
				freerdp_image_copy(pDstData, gdi->format, -1, 0, 0,
						64, 64, pSrcData, PIXEL_FORMAT_XRGB32, -1, 0, 0, gdi->palette);
			}

			for (j = 0; j < message->numRects; j++)
			{
				gdi_SetClipRgn(gdi->primary->hdc,
					cmd->destLeft + message->rects[j].x,
					cmd->destTop + message->rects[j].y,
					message->rects[j].width, message->rects[j].height);

				gdi_BitBlt(gdi->primary->hdc, tx, ty, 64, 64, gdi->tile->hdc, 0, 0, GDI_SRCCOPY);
			}

			gdi->tile->bitmap->data = pDstData;
		}

		gdi_SetNullClipRgn(gdi->primary->hdc);
		rfx_message_free(gdi->codecs->rfx, message);
	}
	else if (cmd->codecID == RDP_CODEC_ID_NSCODEC)
	{
		freerdp_client_codecs_prepare(gdi->codecs, FREERDP_CODEC_NSCODEC);

		nsc_process_message(gdi->codecs->nsc, cmd->bpp, cmd->width, cmd->height, cmd->bitmapData, cmd->bitmapDataLength);

		if (gdi->bitmap_size < (cmd->width * cmd->height * 4))
		{
			gdi->bitmap_size = cmd->width * cmd->height * 4;
			gdi->bitmap_buffer = (BYTE*) _aligned_realloc(gdi->bitmap_buffer, gdi->bitmap_size, 16);

			if (!gdi->bitmap_buffer)
				return;
		}

		pDstData = gdi->bitmap_buffer;
		pSrcData = gdi->codecs->nsc->BitmapData;

		freerdp_image_copy(pDstData, gdi->format, -1, 0, 0,
				cmd->width, cmd->height, pSrcData, PIXEL_FORMAT_XRGB32_VF, -1, 0, 0, gdi->palette);

		gdi->image->bitmap->width = cmd->width;
		gdi->image->bitmap->height = cmd->height;
		gdi->image->bitmap->bitsPerPixel = cmd->bpp;
		gdi->image->bitmap->bytesPerPixel = cmd->bpp / 8;
		gdi->image->bitmap->data = gdi->bitmap_buffer;

		gdi_BitBlt(gdi->primary->hdc, cmd->destLeft, cmd->destTop, cmd->width, cmd->height, gdi->image->hdc, 0, 0, GDI_SRCCOPY);
	} 
	else if (cmd->codecID == RDP_CODEC_ID_NONE)
	{
		if (gdi->bitmap_size < (cmd->width * cmd->height * 4))
		{
			gdi->bitmap_size = cmd->width * cmd->height * 4;
			gdi->bitmap_buffer = (BYTE*) _aligned_realloc(gdi->bitmap_buffer, gdi->bitmap_size, 16);

			if (!gdi->bitmap_buffer)
				return;
		}

		pDstData = gdi->bitmap_buffer;
		pSrcData = cmd->bitmapData;

		freerdp_image_copy(pDstData, gdi->format, -1, 0, 0,
				cmd->width, cmd->height, pSrcData, PIXEL_FORMAT_XRGB32_VF, -1, 0, 0, gdi->palette);

		gdi->image->bitmap->width = cmd->width;
		gdi->image->bitmap->height = cmd->height;
		gdi->image->bitmap->bitsPerPixel = cmd->bpp;
		gdi->image->bitmap->bytesPerPixel = cmd->bpp / 8;
		gdi->image->bitmap->data = gdi->bitmap_buffer;

		gdi_BitBlt(gdi->primary->hdc, cmd->destLeft, cmd->destTop, cmd->width, cmd->height, gdi->image->hdc, 0, 0, GDI_SRCCOPY);
	}
	else
	{
		WLog_ERR(TAG, "Unsupported codecID %d", cmd->codecID);
	}
}

/**
 * Register GDI callbacks with libfreerdp-core.
 * @param inst current instance
 * @return
 */

void gdi_register_update_callbacks(rdpUpdate* update)
{
	rdpPrimaryUpdate* primary = update->primary;

	update->Palette = gdi_palette_update;
	update->SetBounds = gdi_set_bounds;

	primary->DstBlt = gdi_dstblt;
	primary->PatBlt = gdi_patblt;
	primary->ScrBlt = gdi_scrblt;
	primary->OpaqueRect = gdi_opaque_rect;
	primary->DrawNineGrid = NULL;
	primary->MultiDstBlt = NULL;
	primary->MultiPatBlt = NULL;
	primary->MultiScrBlt = NULL;
	primary->MultiOpaqueRect = gdi_multi_opaque_rect;
	primary->MultiDrawNineGrid = NULL;
	primary->LineTo = gdi_line_to;
	primary->Polyline = gdi_polyline;
	primary->MemBlt = gdi_memblt;
	primary->Mem3Blt = gdi_mem3blt;
	primary->SaveBitmap = NULL;
	primary->GlyphIndex = NULL;
	primary->FastIndex = NULL;
	primary->FastGlyph = NULL;
	primary->PolygonSC = gdi_polygon_sc;
	primary->PolygonCB = gdi_polygon_cb;
	primary->EllipseSC = gdi_ellipse_sc;
	primary->EllipseCB = gdi_ellipse_cb;

	update->SurfaceBits = gdi_surface_bits;
	update->SurfaceFrameMarker = gdi_surface_frame_marker;

	update->altsec->FrameMarker = gdi_frame_marker;
}

void gdi_init_primary(rdpGdi* gdi)
{
	gdi->primary = (gdiBitmap*) malloc(sizeof(gdiBitmap));

	if (!gdi->primary)
		return;

	gdi->primary->hdc = gdi_CreateCompatibleDC(gdi->hdc);

	if (!gdi->primary_buffer)
		gdi->primary->bitmap = gdi_CreateCompatibleBitmap(gdi->hdc, gdi->width, gdi->height);
	else
		gdi->primary->bitmap = gdi_CreateBitmap(gdi->width, gdi->height, gdi->dstBpp, gdi->primary_buffer);

	gdi_SelectObject(gdi->primary->hdc, (HGDIOBJECT) gdi->primary->bitmap);
	gdi->primary->org_bitmap = NULL;

	gdi->primary_buffer = gdi->primary->bitmap->data;

	if (!gdi->drawing)
		gdi->drawing = gdi->primary;

	gdi->primary->hdc->hwnd = (HGDI_WND) malloc(sizeof(GDI_WND));
	gdi->primary->hdc->hwnd->invalid = gdi_CreateRectRgn(0, 0, 0, 0);
	gdi->primary->hdc->hwnd->invalid->null = 1;

	gdi->primary->hdc->hwnd->count = 32;
	gdi->primary->hdc->hwnd->cinvalid = (HGDI_RGN) malloc(sizeof(GDI_RGN) * gdi->primary->hdc->hwnd->count);
	gdi->primary->hdc->hwnd->ninvalid = 0;
}

void gdi_resize(rdpGdi* gdi, int width, int height)
{
	if (gdi && gdi->primary)
	{
		if (gdi->width != width || gdi->height != height)
		{
			if (gdi->drawing == gdi->primary)
				gdi->drawing = NULL;

			gdi->width = width;
			gdi->height = height;
			gdi_bitmap_free_ex(gdi->primary);
			gdi->primary_buffer = NULL;
			gdi_init_primary(gdi);
		}
	}
}

/**
 * Initialize GDI
 * @param inst current instance
 * @return
 */

int gdi_init(freerdp* instance, UINT32 flags, BYTE* buffer)
{
	BOOL rgb555;
	rdpGdi* gdi;
	rdpCache* cache;

	gdi = (rdpGdi*) calloc(1, sizeof(rdpGdi));

	if (!gdi)
		return -1;

	instance->context->gdi = gdi;
	gdi->context = instance->context;
	cache = instance->context->cache;

	gdi->codecs = instance->context->codecs;
	gdi->width = instance->settings->DesktopWidth;
	gdi->height = instance->settings->DesktopHeight;
	gdi->srcBpp = instance->settings->ColorDepth;
	gdi->primary_buffer = buffer;

	/* default internal buffer format */
	gdi->dstBpp = 32;
	gdi->bytesPerPixel = 4;
	gdi->format = PIXEL_FORMAT_XRGB32;

	if (flags & CLRCONV_INVERT)
		gdi->invert = TRUE;

	rgb555 = (flags & CLRCONV_RGB555) ? TRUE : FALSE;

	if (gdi->srcBpp > 16)
	{
		if (flags & CLRBUF_32BPP)
		{
			gdi->dstBpp = 32;
			gdi->bytesPerPixel = 4;
		}
		else if (flags & CLRBUF_16BPP)
		{
			gdi->dstBpp = rgb555 ? 15 : 16;
			gdi->bytesPerPixel = 2;
		}
	}
	else
	{
		if (flags & CLRBUF_16BPP)
		{
			gdi->dstBpp = rgb555 ? 15 : 16;
			gdi->bytesPerPixel = 2;
		}
		else if (flags & CLRBUF_32BPP)
		{
			gdi->dstBpp = 32;
			gdi->bytesPerPixel = 4;
		}
	}

	if (!gdi->invert)
	{
		if (gdi->bytesPerPixel == 4)
			gdi->format = PIXEL_FORMAT_XRGB32;
		else if ((gdi->bytesPerPixel == 2) && (gdi->dstBpp == 16))
			gdi->format = PIXEL_FORMAT_RGB565;
		else if ((gdi->bytesPerPixel == 2) && (gdi->dstBpp == 15))
			gdi->format = PIXEL_FORMAT_RGB555;
	}
	else
	{
		if (gdi->bytesPerPixel == 4)
			gdi->format = PIXEL_FORMAT_XBGR32;
		else if ((gdi->bytesPerPixel == 2) && (gdi->dstBpp == 16))
			gdi->format = PIXEL_FORMAT_BGR565;
		else if ((gdi->bytesPerPixel == 2) && (gdi->dstBpp == 15))
			gdi->format = PIXEL_FORMAT_BGR555;
	}

	gdi->hdc = gdi_GetDC();
	gdi->hdc->bitsPerPixel = gdi->dstBpp;
	gdi->hdc->bytesPerPixel = gdi->bytesPerPixel;

	gdi->hdc->alpha = (flags & CLRCONV_ALPHA) ? TRUE : FALSE;
	gdi->hdc->invert = (flags & CLRCONV_INVERT) ? TRUE : FALSE;
	gdi->hdc->rgb555 = (flags & CLRCONV_RGB555) ? TRUE : FALSE;

	gdi_init_primary(gdi);

	gdi->tile = gdi_bitmap_new_ex(gdi, 64, 64, 32, NULL);
	gdi->image = gdi_bitmap_new_ex(gdi, 64, 64, 32, NULL);

	if (!cache)
	{
		cache = cache_new(instance->settings);
		instance->context->cache = cache;
	}

	gdi_register_update_callbacks(instance->update);

	brush_cache_register_callbacks(instance->update);
	glyph_cache_register_callbacks(instance->update);
	bitmap_cache_register_callbacks(instance->update);
	offscreen_cache_register_callbacks(instance->update);
	palette_cache_register_callbacks(instance->update);

	gdi_register_graphics(instance->context->graphics);

	instance->update->BitmapUpdate = gdi_bitmap_update;

	return 0;
}

void gdi_free(freerdp* instance)
{
	rdpGdi* gdi = instance->context->gdi;

	if (gdi)
	{
		gdi_bitmap_free_ex(gdi->primary);
		gdi_bitmap_free_ex(gdi->tile);
		gdi_bitmap_free_ex(gdi->image);
		gdi_DeleteDC(gdi->hdc);
		_aligned_free(gdi->bitmap_buffer);
		free(gdi);
	}
	
	instance->context->gdi = (rdpGdi*) NULL;
}

