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

/*
 * @(#)Group.h	1.9 91/10/15 Copyright 1991 Sun Microsystems
 */

#ifndef _OlGroup_h
#define _OlGroup_h

#include <X11/Constraint.h>
					/* Class record constants */

extern WidgetClass			groupWidgetClass;
typedef struct _GroupClassRec	*	GroupWidgetClass;
typedef struct _GroupRec	*	GroupWidget;

#define	XtNgroupType		"groupType"
#define	XtCGroupType		"GroupType"

#define XtNuseConstraints	"useConstraints"
#define XtCUseConstraints	"UseConstraints"

#define XtNcolumnAlignment	"columnAlignment"
#define XtCColumnAlignment	"ColumnAlignment"

#define XtNrowAlignment		"rowAlignment"
#define XtCRowAlignment		"RowAlignment"

#define	XtNvSpace		"vSpace"
#define	XtCVSpace		"VSpace"

#define	XtNhSpace		"hSpace"
#define	XtCHSpace		"HSpace"

#define	XtNminX			"minX"
#define	XtCMinX			"MinX"

#define	XtNminY			"minY"
#define	XtCMinY			"MinY"

#define	XtNdenominator		"denominator"
#define	XtCDenominator		"Denominator"

#define	XtNvOffset		"vOffset"
#define	XtCVOffset		"VOffset"

#define	XtNhOffset		"hOffset"
#define	XtCHOffset		"hOffset"

#define	XtNcolumns		"columns"
#define	XtCColumns		"Columns"

#define	XtNrows			"rows"
#define	XtCRows			"Rows"

#define	XtNcolumn		"column"
#define	XtNrow			"row"
#define	XtCRowColLocation	"RowColLocation"

#define	XtNanchor		"anchor"
#define	XtCAnchor		"Anchor"

#define	XtNanchorPoint		"anchorPoint"
#define	XtCAnchorPoint		"AnchorPoint"

#define	XtNreferencePoint	"referencePoint"
#define	XtCReferencePoint	"ReferencePoint"

#define	XtNanchorName		"anchorName"
#define	XtCAnchorName		"AnchorName"

#define	XtNminimiseRowCol	"minimiseRowCol"
#define	XtCMinimiseRowCol	"MinimiseRowCol"

#define	XtNtrackAnchorGeom	"trackAnchorGeom"
#define	XtCTrackAnchorGeom	"TrackAnchorGeom"

/* group type ..... ROW & COLUMN def'd already in OpenLook.h */

#define	OL_ROWCOLUMN		147

/* column alignments */

#define	OL_COLUMN_ALIGN_V_CENTER	150
#define	OL_COLUMN_ALIGN_LEFT		151
#define	OL_COLUMN_ALIGN_RIGHT		152
#define	OL_COLUMN_ALIGN_LABELS		153

/* row alignments */

#define	OL_ROW_ALIGN_TOP_EDGES		160
#define	OL_ROW_ALIGN_BOTTOM_EDGES	161
#define	OL_ROW_ALIGN_H_CENTER		162

/* compass points */

#define	OL_NORTH			170
#define	OL_NORTHEAST			171
#define	OL_EAST				172
#define	OL_SOUTHEAST			173
#define	OL_SOUTH			174
#define	OL_SOUTHWEST			175
#define	OL_WEST				176
#define	OL_NORTHWEST			177

#endif	/* _OlGroup_h */
