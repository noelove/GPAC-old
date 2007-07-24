/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005
 *					All rights reserved
 *
 *  This file is part of GPAC / 2D+3D rendering module
 *
 *  GPAC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  GPAC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *   
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */

#ifndef _TEXTURING_H_
#define _TEXTURING_H_

#include "render.h"

/*allocates the HW specific texture handle(s) (potentially both raster and OpenGL handlers)*/
GF_Err render_texture_allocate(GF_TextureHandler *txh);
/*releases the hardware handle(s) and all associated system resources*/
void render_texture_release(GF_TextureHandler *txh);
/*signals new data is available in the texture interface*/
GF_Err render_texture_set_data(GF_TextureHandler *hdl);
/*reset the hardware handle(s) only, but keep associated resources for later hardware bind*/
void render_texture_reset(GF_TextureHandler *hdl);

/*push data to hardware if needed, creating the hardware handle(s)*/
Bool render_texture_push_image(GF_TextureHandler *txh, Bool generate_mipmaps, Bool for2d);

/*gets texture transform matrix - returns 1 if not identity
@tx_transform: texture transform node from appearance*/
Bool render_texture_get_transform(GF_TextureHandler *txh, GF_Node *tx_transform, GF_Matrix *mx);

/*gets the associated raster2D stencil handler*/
GF_STENCIL render_texture_get_stencil(GF_TextureHandler *hdl);
/*sets the associated raster2D stencil handler (used by gradients)*/
void render_texture_set_stencil(GF_TextureHandler *hdl, GF_STENCIL stencil);

Bool render_texture_is_transparent(GF_TextureHandler *txh);

/*ALL THE FOLLOWING ARE ONLY AVAILABLE IN 3D AND DEAL WITH OPENGL TEXTURE MANAGEMENT*/
#ifndef GPAC_DISABLE_3D

/*enable the texture and pushes the given texture transform on the graphics card*/
Bool tx_enable(GF_TextureHandler *txh, GF_Node *tx_transform);
/*disables the texture (unbinds it)*/
void tx_disable(GF_TextureHandler *txh);
/*retrieves the internal (potentially converted for YUV) data buffer*/
char *tx_get_data(GF_TextureHandler *txh, u32 *pix_format);
/*checks if the data buffer shall be pushed to graphics card*/
Bool tx_needs_reload(GF_TextureHandler *hdl);

#ifndef GPAC_USE_TINYGL
/*copy current GL window to the texture - the viewport used is the texture one (0,0,W,H) */
void tx_copy_to_texture(GF_TextureHandler *txh);
#endif


/*copy current GL window to the associated data buffer - the viewport used is the texture one (0,0,W,H) */
void tx_copy_to_stencil(GF_TextureHandler *txh);

/*set blending mode*/
enum
{
	TX_DECAL = 0,
	TX_MODULATE,
	TX_REPLACE,
	TX_BLEND,
};
/*set texturing blend mode*/
void tx_set_blend_mode(GF_TextureHandler *txh, u32 mode);

#endif	/*GPAC_DISABLE_3D*/


#endif	/*_TEXTURING_H_*/

