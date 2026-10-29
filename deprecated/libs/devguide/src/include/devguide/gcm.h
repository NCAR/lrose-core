/*
 * @(#)gcm.h	2.5 91/10/15 Copyright 1989 Sun Microsystems
 *
 * This file is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify this file without charge, but are not authorized to
 * license or distribute it to anyone else except as part of a product
 * or program developed by the user.
 * 
 * THIS FILE IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 * 
 * This file is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 * 
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY THIS FILE
 * OR ANY PART THEREOF.
 * 
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even
 * if Sun has been advised of the possibility of such damages.
 * 
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

/*
 * GUIDE colormap segment support functions.
 */

#ifndef guide_gcm_DEFINED
#define guide_gcm_DEFINED

#include	<devguide/c_varieties.h>

EXTERN_FUNCTION( char	*gcm_colormap_name,	(_VOID_) );
EXTERN_FUNCTION( void	gcm_initialize_colors,	(Xv_opaque, const char *, const char *) );
EXTERN_FUNCTION( int	gcm_color_index,	(const char *) );
EXTERN_FUNCTION( Xv_opaque gcm_color_palette_menu_create, (_VOID_) );
extern char *Gcm_colornames[];  /* how do I extern this properly */

#define GUIDE_COLOR_LIST	\
	"Black", "Dark Slate Gray", "Dim Gray", "Gray", "Light Gray", "White", \
	"Yellow", "Gold", "Khaki", "Wheat", "Tan", "Goldenrod", "Orange", \
	"Coral", "Salmon", "Red", "Orange Red", "Indian Red", "Firebrick", \
	"Brown", "Sienna", "Maroon", "Medium Violet Red", "Violet Red", \
	"Pink", \
	"Green", "Spring Green", "Medium Spring Green", "Green Yellow", \
	"Pale Green", "Yellow Green", "Lime Green", \
	"Medium Sea Green", "Sea Green", "Forest Green", "Olive Drab", \
	"Dark Olive Green", "Dark Green", \
	"Cadet Blue", "Medium Aquamarine", "Dark Turquoise", \
	"Medium Turquoise","Turquoise", "Aquamarine", "Cyan", \
	"Blue", "Medium Blue", "Medium Slate Blue", "Cornflower Blue", \
	"Sky Blue", "Light Blue", "Light Steel Blue", "Steel Blue", \
	"Dark Slate Blue", "Navy", "Navy Blue", "Midnight Blue", \
	"Dark Orchid", "Medium Orchid", "Orchid", "Magenta", "Violet", "Plum", \
	"Thistle", "Blue Violet", "Slate Blue"

#endif /* ~guide_gcm_DEFINED */
