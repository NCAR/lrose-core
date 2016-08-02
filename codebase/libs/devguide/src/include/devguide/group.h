/*
 * @(#)group.h	2.13 91/10/15 Copyright 1991 Sun Microsystems
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

#ifndef guide_group_DEFINED

#include	<xview/xview.h>
#include	<xview/panel.h>

extern Xv_pkg	group_pkg;

#define	GROUP	&group_pkg

typedef Xv_opaque	Group;

#define	ATTR_PKG_GROUP			ATTR_PKG_UNUSED_LAST - 2
#define	GROUP_ATTR(type, ordinal)	ATTR(ATTR_PKG_GROUP, type, ordinal)

/*
 * Public attributes
 */
typedef enum {
	GROUP_TYPE		= GROUP_ATTR(ATTR_ENUM,     1),	/* CSG */
	GROUP_ROWS		= GROUP_ATTR(ATTR_INT,      2),	/* CSG */
	GROUP_COLUMNS		= GROUP_ATTR(ATTR_INT,      3),	/* CSG */
	GROUP_HORIZONTAL_SPACING = GROUP_ATTR(ATTR_INT,     4),	/* CSG */
	GROUP_VERTICAL_SPACING	= GROUP_ATTR(ATTR_INT,      5),	/* CSG */
	GROUP_ROW_ALIGNMENT	= GROUP_ATTR(ATTR_ENUM,     6),	/* CSG */
	GROUP_COLUMN_ALIGNMENT	= GROUP_ATTR(ATTR_ENUM,     7),	/* CSG */
	GROUP_MEMBERS		=				/* CSG */
		GROUP_ATTR(ATTR_LIST_INLINE(ATTR_NULL, ATTR_OPAQUE), 8),
	GROUP_MEMBERS_PTR	= GROUP_ATTR(ATTR_OPAQUE,   9),	/* CSG */
	GROUP_ANCHOR_OBJ	= GROUP_ATTR(ATTR_OPAQUE,  10),	/* CSG */
	GROUP_ANCHOR_POINT	= GROUP_ATTR(ATTR_ENUM,    11),	/* CSG */
	GROUP_REFERENCE_POINT	= GROUP_ATTR(ATTR_ENUM,    12),	/* CSG */
	GROUP_HORIZONTAL_OFFSET	= GROUP_ATTR(ATTR_INT,     13),	/* CSG */
	GROUP_VERTICAL_OFFSET	= GROUP_ATTR(ATTR_INT,     14),	/* CSG */
	GROUP_PARENT		= GROUP_ATTR(ATTR_OPAQUE,  15),	/* --G */
	GROUP_LAYOUT		= GROUP_ATTR(ATTR_BOOLEAN, 16),	/* CSG */
	GROUP_REPLACE_MEMBER	= GROUP_ATTR(ATTR_OPAQUE_PAIR, 17), /* -S- */
} Group_attr;

typedef enum {
	GROUP_NONE,
	GROUP_ROW,
	GROUP_COLUMN,
	GROUP_ROWCOLUMN
} GROUP_TYPES;

typedef enum {
	GROUP_LEFT_EDGES,
	GROUP_LABELS,
	GROUP_VERTICAL_CENTERS,
	GROUP_RIGHT_EDGES
} GROUP_COLUMN_ALIGNMENTS;

typedef enum {
	GROUP_TOP_EDGES,
	GROUP_HORIZONTAL_CENTERS,
	GROUP_BOTTOM_EDGES
} GROUP_ROW_ALIGNMENTS;

typedef enum {
	GROUP_NORTHWEST,
	GROUP_NORTH,
	GROUP_NORTHEAST,
	GROUP_WEST,
	GROUP_CENTER,
	GROUP_EAST,
	GROUP_SOUTHWEST,
	GROUP_SOUTH,
	GROUP_SOUTHEAST,
} GROUP_COMPASS_POINTS;

typedef struct {
	Xv_generic_struct	parent_data;
	Xv_opaque		private_data;
} Group_public;

/*
 * Public functions
 */
EXTERN_FUNCTION( void	group_layout,	(Group) );
EXTERN_FUNCTION( void	group_anchor,	(Group) );

#endif /* guide_group_DEFINED */
