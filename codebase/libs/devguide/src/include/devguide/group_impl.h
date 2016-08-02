/*
 * @(#)group_impl.h	2.11 91/10/15 Copyright 1991 Sun Microsystems
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

#ifndef guide_group_impl_DEFINED

#include	"group.h"

typedef struct {
	Xv_object		public_self;
	GROUP_TYPES		group_type;
	Xv_opaque		*members;
	int			cols;
	GROUP_COLUMN_ALIGNMENTS	col_alignment;
	int			rows;
	GROUP_ROW_ALIGNMENTS	row_alignment;
	int			hspacing;
	int			vspacing;
	Xv_opaque		anchor_obj;	 /* Object anchored to */
	GROUP_COMPASS_POINTS	anchor_point;	 /* Point on anchor obj */
	GROUP_COMPASS_POINTS	reference_point; /* Point on group */
	int			hoffset;
	int			voffset;
	Rect			group_rect;
	Rect			value_rect;
	int			initial_x;
	int			initial_y;
	unsigned int		flags;
} Group_private;

Pkg_private int		group_init();
Pkg_private Xv_opaque	group_set();
Pkg_private Xv_opaque	group_get();
Pkg_private int		group_destroy();

#define	GROUP_PUBLIC(item)	XV_PUBLIC(item)
#define	GROUP_PRIVATE(item)	XV_PRIVATE(Group_private, Group_public, item)

typedef enum {
        CREATED		= (1L << 0),
        LAYOUT		= (1L << 1),
        INACTIVE	= (1L << 2),
        SHOWING		= (1L << 3),
        ROWFIRST	= (1L << 4),
        COLFIRST	= (1L << 5)
} GROUP_FLAGS;

#endif /* guide_group_impl_DEFINED */
