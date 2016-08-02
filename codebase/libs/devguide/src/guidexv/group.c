/*
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

#ifndef lint
static char	sccsid[] = "@(#)group.c	2.27 91/10/15 Copyright 1991 Sun Microsystems";
#endif

/*
 * Routines for relative layout support through groups
 */

#include	<devguide/group_impl.h>

static	void	layout_none();
static	void	layout_row();
static	void	layout_col();
static	void	layout_rowcol();

static	void		group_set_xy();
static	void		group_set_members();
static	Rect		*get_rect_for_group();
static	Group_public	*get_parent_group();
static	void		get_compass_point();
static	void		get_rowcol_info();
static	int		get_value_x();
static	void		place_cell();

/*
 * Package initialization, called on xv_create
 */
/*ARGSUSED*/
Pkg_private int
#ifdef __STDC__
group_init(Xv_opaque owner, Group_public *group_public, Attr_avlist avlist)
#else
group_init(owner, group_public, avlist)
	Xv_opaque	owner;
	Group_public	*group_public;
	Attr_avlist	avlist;
#endif
{
	Group_private	*group_private = xv_alloc(Group_private);

	if (!group_private)
		return XV_ERROR;

	group_public->private_data = (Xv_opaque)group_private;
	group_private->public_self = (Xv_opaque)group_public;

	/*
	 * Initialize the defaults for certain values
	 */
	group_private->group_type = GROUP_NONE;
	group_private->hspacing = 10;
	group_private->vspacing = 10;
	group_private->row_alignment = GROUP_TOP_EDGES;
	group_private->col_alignment = GROUP_LEFT_EDGES;
	group_private->reference_point = GROUP_NORTHWEST;
	group_private->hoffset = 10;
	group_private->voffset = 10;
	group_private->flags |= (SHOWING|LAYOUT);

	return XV_OK;
}

/*
 * Package set function, called on xv_set
 */
Pkg_private Xv_opaque
#ifdef __STDC__
group_set(Group_public *group_public, Attr_avlist avlist)
#else
group_set(group_public, avlist)
	Group_public	*group_public;
	Attr_avlist	avlist;
#endif
{
	int		i;
	int		x;
	int		x_changed = FALSE;
	int		y;
	int		y_changed = FALSE;
	int		replaced;
	int		need_layout = FALSE;
	int		initial_position = FALSE;
	Xv_opaque	old, new;
	Attr_attribute	*attrs;
	Group_private	*group_private = GROUP_PRIVATE(group_public);

	for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
		switch ((int) attrs[0]) {
		case XV_X:
			x = (int)attrs[1];
			x_changed = TRUE;
			if (!(group_private->flags & CREATED)) {
				group_private->initial_x = x;
				group_private->group_rect.r_left = x;
			}
			ATTR_CONSUME(attrs[0]);
			break;

		case XV_Y:
			y = (int)attrs[1];
			y_changed = TRUE;
			if (!(group_private->flags & CREATED)) {
				group_private->initial_y = y;
				group_private->group_rect.r_top = y;
			}
			ATTR_CONSUME(attrs[0]);
			break;

		case PANEL_VALUE_X:
			x = group_private->group_rect.r_left + 
			    ((int)attrs[1] - group_private->value_rect.r_left);
			x_changed = TRUE;
			ATTR_CONSUME(attrs[0]);
			break;
			
		case XV_SHOW:
			for (i = 0; group_private->members[i]; i++) {
				xv_set(group_private->members[i],
					XV_SHOW, attrs[1], NULL);
			}
			if ((int)attrs[1])
				group_private->flags |= SHOWING;
			else
				group_private->flags &= ~SHOWING;
			break;

		case PANEL_INACTIVE:
			for (i = 0; group_private->members[i]; i++) {
				xv_set(group_private->members[i],
					PANEL_INACTIVE, (int)attrs[1], NULL);
			}
			if ((int)attrs[1])
				group_private->flags |= INACTIVE;
			else
				group_private->flags &= ~INACTIVE;
			break;

		case GROUP_TYPE:
			need_layout = TRUE;
			group_private->group_type = (GROUP_TYPES)attrs[1];
			break;

		case GROUP_ROWS:
			need_layout = TRUE;
			if ((group_private->rows = (int)attrs[1]) > 0) {
				group_private->flags |= ROWFIRST;
				group_private->flags &= ~COLFIRST;
			}
			break;

		case GROUP_COLUMNS:
			need_layout = TRUE;
			if ((group_private->cols = (int)attrs[1]) > 0) {
				group_private->flags |= COLFIRST;
				group_private->flags &= ~ROWFIRST;
			}
			break;

		case GROUP_HORIZONTAL_SPACING:
			need_layout = TRUE;
			group_private->hspacing = (int)attrs[1];
			break;

		case GROUP_VERTICAL_SPACING:
			need_layout = TRUE;
			group_private->vspacing = (int)attrs[1];
			break;

		case GROUP_ROW_ALIGNMENT:
			need_layout = TRUE;
			group_private->row_alignment = (GROUP_ROW_ALIGNMENTS)attrs[1];
			break;

		case GROUP_COLUMN_ALIGNMENT:
			need_layout = TRUE;
			group_private->col_alignment = (GROUP_COLUMN_ALIGNMENTS)attrs[1];
			break;

		case GROUP_REPLACE_MEMBER:
			old = (Xv_opaque)attrs[1];
			new = (Xv_opaque)attrs[2];

			replaced = FALSE;

			for (i = 0; group_private->members[i]; i++) {
				if (group_private->members[i] == old) {
					replaced = TRUE;
					need_layout = TRUE;
					group_private->members[i] = new;
				}
			}
			
			if (!replaced)
				xv_error((Group)group_public,
				    ERROR_STRING, "member not found, not replaced",
				    ERROR_PKG,    GROUP,
				    NULL);

			break;

		case GROUP_MEMBERS:
			group_set_members(group_public, &attrs[1]);
			need_layout = TRUE;
			break;

		case GROUP_MEMBERS_PTR:
			group_set_members(group_public, (Xv_opaque *)attrs[1]);
			need_layout = TRUE;
			break;

		case GROUP_LAYOUT:
			if ((int)attrs[1]) {
				need_layout = TRUE;
				group_private->flags |= LAYOUT;
			} else
				group_private->flags &= ~LAYOUT;
			break;

		case GROUP_ANCHOR_OBJ:
			need_layout = TRUE;
			group_private->anchor_obj = (Xv_opaque)attrs[1];
			break;

		case GROUP_ANCHOR_POINT:
			need_layout = TRUE;
			group_private->anchor_point =
				(GROUP_COMPASS_POINTS)attrs[1];
			break;

		case GROUP_REFERENCE_POINT:
			need_layout = TRUE;
			group_private->reference_point =
				(GROUP_COMPASS_POINTS)attrs[1];
			break;

		case GROUP_HORIZONTAL_OFFSET:
			need_layout = TRUE;
			group_private->hoffset = (int)attrs[1];
			break;

		case GROUP_VERTICAL_OFFSET:
			need_layout = TRUE;
			group_private->voffset = (int)attrs[1];
			break;

		case XV_END_CREATE:
			need_layout = TRUE;
			initial_position = TRUE;
			group_private->flags |= CREATED;
			break;

		default:
			xv_check_bad_attr(GROUP, attrs[0]);
			break;
		}
	}

	/*
	 * Don't go any further until object has been created
	 */
	if (!(group_private->flags & CREATED))
		return (Xv_opaque)XV_OK;

	/*
	 * If something changed that would change the layout of
	 * the group, percolate!
	 */
	if (need_layout && (group_private->flags & LAYOUT))
		group_layout((Group)group_public);

	/*
	 * Check to see if x or y were set, if so we need to move the group.
	 */
	if (x_changed || y_changed) {
		if (!x_changed)
			x = group_private->group_rect.r_left;
		if (!y_changed)
			y = group_private->group_rect.r_top;

		group_set_xy(group_public, x, y);
	}
	
	/*
	 * If this is the first time this group has been positioned,
	 * make sure it is placed correctly if it is not anchored.  If
	 * it is anchored it was placed correctly earlier.
	 */
	if (initial_position && !group_private->anchor_obj) {
		group_set_xy(group_public,
			     group_private->initial_x,
			     group_private->initial_y);
	}

	return (Xv_opaque)XV_OK;
}

/*
 * Package get function, called on xv_get
 */
/*ARGSUSED*/
Pkg_private Xv_opaque
#ifdef __STDC__
group_get(Group_public *group_public, int *status, Attr_attribute attr,
	  Attr_avlist args)
#else
group_get(group_public, status, attr, args)
	Group_public	*group_public;
	int		*status;
	Attr_attribute	attr;
	Attr_avlist	args;
#endif
{
	Group_private	*group_private = GROUP_PRIVATE(group_public);

	switch ((int) attr) {
		case XV_X:
			return (Xv_opaque)group_private->group_rect.r_left;

		case PANEL_VALUE_X:
			return (Xv_opaque)group_private->value_rect.r_left;

		case XV_Y:
			return (Xv_opaque)group_private->group_rect.r_top;

		case PANEL_VALUE_Y:
			return (Xv_opaque)group_private->value_rect.r_top;

		case XV_WIDTH:
			return (Xv_opaque)group_private->group_rect.r_width;

		case XV_HEIGHT:
			return (Xv_opaque)group_private->group_rect.r_height;

		case XV_RECT:
			return (Xv_opaque)&(group_private->group_rect);

		case XV_SHOW:
			return (Xv_opaque)(group_private->flags & SHOWING);

		case PANEL_INACTIVE:
			return (Xv_opaque)(group_private->flags & INACTIVE);

		case GROUP_TYPE:
			return (Xv_opaque)group_private->group_type;

		case GROUP_ROWS:
			return (Xv_opaque)group_private->rows;

		case GROUP_COLUMNS:
			return (Xv_opaque)group_private->cols;

		case GROUP_HORIZONTAL_SPACING:
			return (Xv_opaque)group_private->hspacing;

		case GROUP_VERTICAL_SPACING:
			return (Xv_opaque)group_private->vspacing;

		case GROUP_ROW_ALIGNMENT:
			return (Xv_opaque)group_private->row_alignment;

		case GROUP_COLUMN_ALIGNMENT:
			return (Xv_opaque)group_private->col_alignment;

		case GROUP_MEMBERS:
			return (Xv_opaque)group_private->members;

		case GROUP_LAYOUT:
			return (Xv_opaque)(group_private->flags & LAYOUT);

		case GROUP_ANCHOR_OBJ:
			return (Xv_opaque)group_private->anchor_obj;

		case GROUP_ANCHOR_POINT:
			return (Xv_opaque)group_private->anchor_point;

		case GROUP_REFERENCE_POINT:
			return (Xv_opaque)group_private->reference_point;

		case GROUP_HORIZONTAL_OFFSET:
			return (Xv_opaque)group_private->hoffset;

		case GROUP_VERTICAL_OFFSET:
			return (Xv_opaque)group_private->voffset;

		case GROUP_PARENT:
			return (Xv_opaque)get_parent_group(group_public);

		default:
			if (xv_check_bad_attr(GROUP, attr) == XV_ERROR)
				*status = XV_ERROR;
			break;
	}

	return (Xv_opaque)XV_OK;
}

/*
 * Package destroy function, called on xv_destroy
 */
Pkg_private int
#ifdef __STDC__
group_destroy(Group_public *group_public, Destroy_status status)
#else
group_destroy(group_public, status)
	Group_public	*group_public;
	Destroy_status	status;
#endif
{
	Group_private	*group_private = GROUP_PRIVATE(group_public);

	if (status == DESTROY_CLEANUP)
	{
		/*
		 * Mark all members free of this group, then free up
		 * space allocated for members list.
		 */
		if (group_private->members)
		{
			/*
			 * MOOSE - we could only do this if we could tell
			 * whether the XView handle is still valid...
			 *
			for (i = 0; group_private->members[i]; i++)
				xv_set(group_private->members[i],
					XV_KEY_DATA, GROUP_PARENT, NULL, NULL);
			 */

			xv_free(group_private->members);
		}

		xv_free(group_private);
	}

	return XV_OK;
}

/*
 * Layout a group according to it's constraints
 */
void
#ifdef __STDC__
group_layout(Group group_public)
#else
group_layout(group_public)
	Group	group_public;
#endif
{
	Rect		*r;
	Group_public	*parent;
	Group_private	*group_private;

	if (!group_public)
		return;

	group_private = GROUP_PRIVATE(group_public);

	if (!group_private || !group_private->members)
		return;

	switch (group_private->group_type) {
	case GROUP_NONE:
		layout_none(group_public);
		break;
	case GROUP_ROW:
		layout_row(group_public);
		break;
	case GROUP_COLUMN:
		layout_col(group_public);
		break;
	case GROUP_ROWCOLUMN:
		layout_rowcol(group_public);
		break;
	default:
		break;
	}

	/*
	 * Update x/y/width/height for this group, values may have changed
	 */
	r = get_rect_for_group(group_public);
	group_private->group_rect.r_left = r->r_left;
	group_private->group_rect.r_top = r->r_top;
	group_private->group_rect.r_width = r->r_width;
	group_private->group_rect.r_height = r->r_height;

	if (group_private->members) {
		group_private->value_rect.r_left = 
			(int)xv_get(group_private->members[0], PANEL_VALUE_X);
		group_private->value_rect.r_top = 
			(int)xv_get(group_private->members[0], PANEL_VALUE_Y);
	} else {
		group_private->value_rect.r_left = group_private->group_rect.r_left;
		group_private->value_rect.r_top = group_private->group_rect.r_top;
	}

	/*
	 * If this group belongs to a parent group, we need to recurse
	 * upwards and layout the parent group now.
	 *
	 * Otherwise, if this group is anchored, place it relative to anchor
	 */
	if (parent = get_parent_group(group_public))
		group_layout((Group)parent);
	else if (group_private->anchor_obj)
		group_anchor((Group)group_public);
}

/*
 * Allocate space for an array of Xv_opaques to store new members.
 */
static void
#ifdef __STDC__
group_set_members(Group_public *group_public, Xv_opaque *members)
#else
group_set_members(group_public, members)
	Group_public	*group_public;
	Xv_opaque	*members;
#endif
{
	int		i;
	int		num;
	Xv_opaque	*xvptr = members;
	Group_private	*group_private = GROUP_PRIVATE(group_public);

	/*
	 * Free up space from old members list
	 */
	if (group_private->members)
		xv_free(group_private->members);

	if (!members)
	{
		group_private->members = NULL;
		return;
	}

	/*
	 * Count number of new members
	 */
	for (num = 0; *xvptr++; num++)
		;

	/*
	 * Allocate space for new members list.  Should use xv_alloc_n
	 * here, but it was incorrect in V2 and we want this package
	 * to stay portable back to V2.
	 */
	group_private->members = (Xv_opaque *)calloc(num+1, sizeof(Xv_opaque));

	/*
	 * Walk through list and store members, mark each
	 * new member in this group.
	 */
	for (i = 0; i < num; i++)
	{
		group_private->members[i] = members[i];
		xv_set(group_private->members[i],
			XV_KEY_DATA, GROUP_PARENT, group_public, NULL);
	}
}

/*
 * Move a group to a new (absolute) x/y position
 */
static void
#ifdef __STDC__
group_set_xy(Group_public *group_public, int x, int y)
#else
group_set_xy(group_public, x, y)
	Group_public	*group_public;
	int		x;
	int		y;
#endif
{
	int		i;
	Xv_opaque	cur;
	Group_private	*group_private = GROUP_PRIVATE(group_public);

	if (!group_private->members)
		return;

	for (i = 0; group_private->members[i]; i++)
	{
		cur = group_private->members[i];
		xv_set(cur, XV_X, x + (xv_get(cur, XV_X) - group_private->group_rect.r_left),
			    XV_Y, y + (xv_get(cur, XV_Y) - group_private->group_rect.r_top),
			    NULL);
	}

	group_private->group_rect.r_left = x;
	group_private->group_rect.r_top = y;

	if (group_private->members) {
		group_private->value_rect.r_left =
			xv_get(group_private->members[0], PANEL_VALUE_X);
		group_private->value_rect.r_top =
			xv_get(group_private->members[0], PANEL_VALUE_X);
	}
	else
	{
		group_private->value_rect.r_left = x;
		group_private->value_rect.r_top = y;
	}
}

/*
 * Anchor a group
 */
void
#ifdef __STDC__
group_anchor(Group group_public)
#else
group_anchor(group_public)
	Group	group_public;
#endif
{
	int		new_x;
	int		new_y;
	int		anchor_x;
	int		anchor_y;
	int		ref_x;
	int		ref_y;
	Group_private	*group_private;

	if (!group_public)
		return;

	group_private = GROUP_PRIVATE(group_public);

	if (!group_private || !group_private->anchor_obj)
		return;

	get_compass_point(group_private->anchor_obj, group_private->anchor_point,
			  &anchor_x, &anchor_y);
	get_compass_point(group_public, group_private->reference_point,
			  &ref_x, &ref_y);

	if (xv_get(group_private->anchor_obj, XV_OWNER) !=
	    xv_get(group_public, XV_OWNER)) {
		anchor_x -= (int)xv_get(group_private->anchor_obj, XV_X);
		anchor_y -= (int)xv_get(group_private->anchor_obj, XV_Y);
	}

	new_x = anchor_x + (group_private->group_rect.r_left - ref_x) +
		group_private->hoffset;
	new_y = anchor_y + (group_private->group_rect.r_top - ref_y) +
		group_private->voffset;

	group_set_xy((Group_public *) group_public, new_x, new_y);
}

/*
 * Layout an as-is group.  Usually means do nothing, check to see
 * if members are anchored first though.
 */
static void
#ifdef __STDC__
layout_none(Group_public *group_public)
#else
layout_none(group_public)
	Group_public	*group_public;
#endif
{
	int		i;
	Xv_opaque	cur;
	Xv_opaque	obj;
	Group_private	*group_private = GROUP_PRIVATE(group_public);

	for (i = 0; group_private->members[i]; i++)
	{
		cur = group_private->members[i];

		/*
		 * Check to see if this member is a group, if so
		 * lay it out so it will be anchored correctly.
		 */
		XV_OBJECT_TO_STANDARD(cur, "GROUP", obj);

		if (obj && (((Xv_base *) obj)->pkg == GROUP))
		{
			group_anchor((Group)cur);
		}
	}

}

/*
 * Layout a row group
 */
static void
#ifdef __STDC__
layout_row(Group_public *group_public)
#else
layout_row(group_public)
	Group_public	*group_public;
#endif
{
	int		i;
	int		base_y;
	int		new_y;
	Xv_opaque	cur;
	Xv_opaque	prev;
	Group_private	*group_private = GROUP_PRIVATE(group_public);

	switch (group_private->row_alignment)
	{
	case GROUP_TOP_EDGES:
		base_y = xv_get(group_private->members[0], XV_Y);
		break;

	case GROUP_HORIZONTAL_CENTERS:
		base_y = xv_get(group_private->members[0], XV_Y) + 
			xv_get(group_private->members[0], XV_HEIGHT)/2;
		break;

	case GROUP_BOTTOM_EDGES:
		base_y = xv_get(group_private->members[0], XV_Y) + 
			xv_get(group_private->members[0], XV_HEIGHT);
		break;
	}

	for (i = 1; group_private->members[i]; i++)
	{
		cur = group_private->members[i];
		prev = group_private->members[i-1];

		switch (group_private->row_alignment)
		{
		case GROUP_TOP_EDGES:
			new_y = base_y;
			break;

		case GROUP_HORIZONTAL_CENTERS:
			new_y = base_y - xv_get(cur, XV_HEIGHT)/2;
			break;

		case GROUP_BOTTOM_EDGES:
			new_y = base_y - xv_get(cur, XV_HEIGHT);
			break;
		}

		xv_set(cur, XV_X, xv_get(prev, XV_X) +
				  xv_get(prev, XV_WIDTH) +
				  group_private->hspacing,
			    XV_Y, new_y,
			    NULL);
	}
}

/*
 * Layout a column group
 */
static void
#ifdef __STDC__
layout_col(Group_public *group_public)
#else
layout_col(group_public)
	Group_public	*group_public;
#endif
{
	int		i;
	int		new_x;
	int		base_x;
	Xv_opaque	cur;
	Xv_opaque	prev;
	Group_private	*group_private = GROUP_PRIVATE(group_public);

	switch (group_private->col_alignment)
	{
	case GROUP_LEFT_EDGES:
		base_x = xv_get(group_private->members[0], XV_X);
		break;

	case GROUP_LABELS:
		base_x = xv_get(group_private->members[0], PANEL_VALUE_X);
		break;

	case GROUP_VERTICAL_CENTERS:
		base_x = xv_get(group_private->members[0], XV_X) + 
			xv_get(group_private->members[0], XV_WIDTH)/2;
		break;

	case GROUP_RIGHT_EDGES:
		base_x = xv_get(group_private->members[0], XV_X) + 
			xv_get(group_private->members[0], XV_WIDTH);
		break;
	}

	for (i = 1; group_private->members[i]; i++)
	{
		cur = group_private->members[i];
		prev = group_private->members[i-1];

		switch (group_private->col_alignment)
		{
		case GROUP_LEFT_EDGES:
		case GROUP_LABELS:
			new_x = base_x;
			break;

		case GROUP_VERTICAL_CENTERS:
			new_x = base_x - xv_get(cur, XV_WIDTH)/2;
			break;

		case GROUP_RIGHT_EDGES:
			new_x = base_x - xv_get(cur, XV_WIDTH);
			break;
		}

		if (group_private->col_alignment == GROUP_LABELS)
		{
			xv_set(cur, PANEL_VALUE_X, new_x,
			       XV_Y, xv_get(prev, XV_Y) +
			       xv_get(prev, XV_HEIGHT) +
			       group_private->vspacing,
			       NULL);
		}
		else
		{
			xv_set(cur, XV_X, new_x,
			       XV_Y, xv_get(prev, XV_Y) +
			       xv_get(prev, XV_HEIGHT) +
			       group_private->vspacing,
			       NULL);
		}
	}
}

/*
 * Layout a row/column group
 */
static void
#ifdef __STDC__
layout_rowcol(Group_public *group_public)
#else
layout_rowcol(group_public)
	Group_public	*group_public;
#endif
{
	int		i;
	int		vx = 0;
	int		current_row = 0;
	int		current_col = 0;
	int		cell_width;
	int		cell_height;
	Group_private	*group_private = GROUP_PRIVATE(group_public);
	Xv_opaque	*members = group_private->members;

	get_rowcol_info(group_public, &cell_width, &cell_height);

	if (group_private->col_alignment == GROUP_LABELS)
		vx = get_value_x(group_public, 0);

	/*
	 * Walk through the list, place each object in it's "cell". 
	 */
	for (i = 0; members[i]; i++) {
		place_cell(group_public, i, cell_width, cell_height,
			   vx, current_row, current_col);

		if (group_private->flags & ROWFIRST) {
			if (++current_col >= group_private->cols) {
				current_row++;
				current_col = 0;
			}
			if ((group_private->col_alignment == GROUP_LABELS) &&
			    members[i+1])
				vx = get_value_x(group_public, current_col);
		} else {
			if (++current_row >= group_private->rows) {
				current_col++;
				current_row = 0;
				if ((group_private->col_alignment == GROUP_LABELS) &&
				    members[i+1])
					vx = get_value_x(group_public, current_col);
			}
		}
	}
}

static void
#ifdef __STDC__
get_rowcol_info(Group_public *group_public, int *cell_width, int *cell_height)
#else
get_rowcol_info(group_public, cell_width, cell_height)
	Group_public	*group_public;
	int		*cell_width;
	int		*cell_height;
#endif
{
	int		i;
	int		lw;
	int		vw;
	int		max_lw = -1;
	int		max_vw = -1;
	int		count;
	Group_private	*group_private = GROUP_PRIVATE(group_public);
	Xv_opaque	cur;

	/*
	 * Calculate rows and column based on number of members
	 * and current fill order.
	 */
	if ((group_private->rows == 0) && (group_private->cols == 0))
		group_private->rows = 1;

	for (count = 0; group_private->members[count]; count++)
		;

	if (group_private->flags & ROWFIRST) {
		group_private->cols = count / group_private->rows;

		if (count % group_private->rows)
			group_private->cols++;
	} else {
		group_private->rows = count  / group_private->cols;

		if (count % group_private->cols)
			group_private->rows++;
	}

	/*
	 * Walk through the list, find maximum cell size.  Row/Col
	 * groups aligned on labels are special.  The widest item may
	 * not determine the cell size.  It is determined by the
	 * widest label plus the widest value field, phew.
	 */
	*cell_width = -1;
	*cell_height = -1;

	if ((group_private->group_type == GROUP_ROWCOLUMN) &&
	    (group_private->col_alignment == GROUP_LABELS)) {
		for (i = 0; group_private->members[i]; i++) {
			cur = group_private->members[i];

			lw = xv_get(cur, PANEL_VALUE_X) - xv_get(cur, XV_X);
			vw = xv_get(cur, XV_WIDTH) - lw;

			if (lw > max_lw)
				max_lw = lw;
			if (vw > max_vw)
				max_vw = vw;
			if ((int)xv_get(cur, XV_HEIGHT) > *cell_height)
				*cell_height = xv_get(cur, XV_HEIGHT);
		}

		*cell_width = max_lw + max_vw;
	}
	else
	{
		for (i = 0; group_private->members[i]; i++)
		{
			cur = group_private->members[i];

			if ((int)xv_get(cur, XV_WIDTH) > *cell_width)
				*cell_width = xv_get(cur, XV_WIDTH);
			if ((int)xv_get(cur, XV_HEIGHT) > *cell_height)
				*cell_height = xv_get(cur, XV_HEIGHT);
		}
	}
}

static int
#ifdef __STDC__
get_value_x(Group_public *group_public, int col)
#else
get_value_x(group_public, col)
	Group_public	*group_public;
	int		col;
#endif
{
	int		i;
	int		j;
	int		tmp;
	int		start;
	int		incr;
	int		vx = -1;
	Group_private	*group_private = GROUP_PRIVATE(group_public);

	/*
	 * Walk through the list, find maximum cell size
	 */
	if (group_private->flags & ROWFIRST)
	{
		start = col;
		incr = group_private->cols;
	}
	else
	{
		start = col * group_private->rows;
		incr = 1;
	}

	for (j = 0, i = start;
	     group_private->members[i] && j < group_private->rows;
	     i += incr, j++) {
		tmp = xv_get(group_private->members[i], PANEL_VALUE_X) -
		      xv_get(group_private->members[i], XV_X);

		if (tmp > vx)
			vx = tmp;
	}

	return vx;
}

/*
 * Place an object correctly inside a cell in a row/col group
 */
static void
#ifdef __STDC__
place_cell(Group_public *group_public, int i, int cell_width, int cell_height,
	   int vx, int row, int col)
#else
place_cell(group_public, i, cell_width, cell_height, vx, row, col)
	Group_public	*group_public;
	int		i;
	int		cell_width;
	int		cell_height;
	int		vx;
	int		row;
	int		col;
#endif
{
	int		x;
	int		y;
	int		cell_x;
	int		cell_y;
	Group_private	*group_private = GROUP_PRIVATE(group_public);
	Xv_opaque	cur = group_private->members[i];

	/*
	 * Calculate the upper left corner for this cell
	 */
	cell_x = group_private->group_rect.r_left +
		col * (cell_width + group_private->hspacing);
	cell_y = group_private->group_rect.r_top +
		row * (cell_height + group_private->vspacing);

	switch (group_private->col_alignment)
	{
	case GROUP_LEFT_EDGES:
		x = cell_x;
		break;
	case GROUP_LABELS:
		x = xv_get(cur, XV_X) +
			((cell_x + vx) - xv_get(cur, PANEL_VALUE_X));
		break;
	case GROUP_VERTICAL_CENTERS:
		x = (cell_x + cell_width/2) - xv_get(cur, XV_WIDTH)/2;
		break;
	case GROUP_RIGHT_EDGES:
		x = (cell_x + cell_width) - xv_get(cur, XV_WIDTH);
		break;
	}

	switch (group_private->row_alignment)
	{
	case GROUP_TOP_EDGES:
		y = cell_y;
		break;
	case GROUP_HORIZONTAL_CENTERS:
		y = (cell_y + cell_height/2) - xv_get(cur, XV_HEIGHT)/2;
		break;
	case GROUP_BOTTOM_EDGES:
		y = (cell_y + cell_height) - xv_get(cur, XV_HEIGHT);
		break;
	}

	xv_set(cur, XV_X, x, XV_Y, y, NULL);
}

/*
 * Return the bounding rectangle for a group
 */
static Rect *
#ifdef __STDC__
get_rect_for_group(Group_public *group_public)
#else
get_rect_for_group(group_public)
	Group_public	*group_public;
#endif
{
	int		i;
	int		cell_width;
	int		cell_height;
	static Rect	bbox;
	Rect		r;
	Rect		*r1;
	Group_private	*group_private = GROUP_PRIVATE(group_public);

	r = rect_null;

	if (!group_private->members)
	{
		bbox = r;
		return &bbox;
	}

	if (group_private->group_type == GROUP_ROWCOLUMN)
	{
		get_rowcol_info(group_public, &cell_width, &cell_height);
		r.r_left = group_private->group_rect.r_left;
		r.r_top = group_private->group_rect.r_top;
		r.r_width = (group_private->cols * cell_width) +
			((group_private->cols-1) * group_private->hspacing);
		r.r_height = (group_private->rows * cell_height) +
			((group_private->rows-1) * group_private->vspacing);
	}
	else
	{
		for (i = 0; group_private->members[i]; i++)
		{
			if (r1 = (Rect *)xv_get(group_private->members[i], XV_RECT))
				r = rect_bounding(&r, r1);
		}
	}

	bbox = r;
	return &bbox;
}

/*
 * Return a pointer to a parent group, NULL if none
 */
static Group_public *
#ifdef __STDC__
get_parent_group(Group_public *group_public)
#else
get_parent_group(group_public)
	Group_public	*group_public;
#endif
{
	return (Group_public *)xv_get((Group)group_public,
				      XV_KEY_DATA, GROUP_PARENT);
}

/*
 * Calculate the x/y locatiions for a compass point on an object
 */
static void
#ifdef __STDC__
get_compass_point(Xv_opaque handle, GROUP_COMPASS_POINTS point, int *x, int *y)
#else
get_compass_point(handle, point, x, y)
	Xv_opaque		handle;
	GROUP_COMPASS_POINTS	point;
	int			*x;
	int			*y;
#endif
{
	switch (point)
	{
	case GROUP_NORTHWEST:
	case GROUP_WEST:
	case GROUP_SOUTHWEST:
		*x = xv_get(handle, XV_X);
		break;
	case GROUP_NORTH:
	case GROUP_CENTER:
	case GROUP_SOUTH:
		*x = xv_get(handle, XV_X) + xv_get(handle, XV_WIDTH)/2;
		break;
	case GROUP_NORTHEAST:
	case GROUP_EAST:
	case GROUP_SOUTHEAST:
		*x = xv_get(handle, XV_X) + xv_get(handle, XV_WIDTH);
		break;
	}

	switch (point)
	{
	case GROUP_NORTHWEST:
	case GROUP_NORTH:
	case GROUP_NORTHEAST:
		*y = xv_get(handle, XV_Y);
		break;
	case GROUP_WEST:
	case GROUP_CENTER:
	case GROUP_EAST:
		*y = xv_get(handle, XV_Y) + xv_get(handle, XV_HEIGHT)/2;
		break;
	case GROUP_SOUTHWEST:
	case GROUP_SOUTH:
	case GROUP_SOUTHEAST:
		*y = xv_get(handle, XV_Y) + xv_get(handle, XV_HEIGHT);
		break;
	}
}
