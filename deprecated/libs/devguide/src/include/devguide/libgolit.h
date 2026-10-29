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
 * @(#)libgolit.h	2.10 91/10/30 Copyright 1991 Sun Microsystems
 */
#ifndef	libgolit_h
#define	libgolit_h

#define	ExternCallbackProc(n) 					\
	extern	void n OL_ARGS(( Widget, XtPointer, XtPointer ))

#define	ExternWidgetDescPtr(n)	extern	GolitWidgetDescPtr	n

#define	ExternCreateProc(n)				       \
	extern	Widget n OL_ARGS(( String, GolitWidgetDescPtr, \
				   Widget, ArgList, Cardinal,  \
				   XtPointer ))

#define	ExternShellCreateProc(n)			\
	extern	Widget n OL_ARGS(( String, String, 	\
				   GolitWidgetDescPtr, 	\
				   Display *, ArgList,	\
				   Cardinal, XtPointer ))

typedef	struct _golit_widget_desc *WidgetDescPtr, *GolitWidgetDescPtr;

OLBeginFunctionPrototypeBlock

extern Widget
GolitNameToWidget OL_ARGS (( Widget, String ));
	/* Widget,		root		*/
	/* String,		name		*/

extern Widget
GolitFetchLastWidgetID OL_ARGS (( GolitWidgetDescPtr ));
	/* register GolitWidgetDescPtr,		wdp			*/

extern XtVarArgsList
GolitMergeArgLists OL_ARGS(( GolitWidgetDescPtr, ArgList, Cardinal));
	/* register GolitWidgetDescPtr,		wdp			*/
	/* ArgList,				override_args		*/
	/* Cardinal,				num_override_args	*/

extern Widget
GolitFetchWidget OL_ARGS((String, GolitWidgetDescPtr, Widget, ArgList, 
			  Cardinal, XtPointer));
	/* String,				name		*/
	/* register GolitWidgetDescPtr,		wdp		*/
	/* Widget,				parent		*/
	/* ArgList,				args		*/
	/* Cardinal,				num_args	*/
	/* XtPointer,				closure		*/

extern Widget
GolitFetchWidgetUnmanaged OL_ARGS((String, GolitWidgetDescPtr, Widget, ArgList,
				   Cardinal, XtPointer));
	/* String,				name		*/
	/* register GolitWidgetDescPtr,		wdp		*/
	/* Widget,				parent		*/
	/* ArgList,				args		*/
	/* Cardinal,				num_args	*/
	/* XtPointer,				closure		*/

extern Widget
GolitFetchChildren OL_ARGS((String, GolitWidgetDescPtr, Widget, ArgList,
			    Cardinal, XtPointer));
	/* String,				name		*/
	/* register GolitWidgetDescPtr,		wdp		*/
	/* Widget,				parent		*/
	/* ArgList,				args		*/
	/* Cardinal,				num_args	*/
	/* XtPointer,				closure		*/

extern Widget
GolitFetchChildrenUnmanaged OL_ARGS((String, GolitWidgetDescPtr, Widget,
				     ArgList, Cardinal, XtPointer));
	/* String,				name		*/
	/* register GolitWidgetDescPtr,		wdp		*/
	/* Widget,				parent		*/
	/* ArgList,				args		*/
	/* Cardinal,				num_args	*/
	/* XtPointer,				closure		*/

extern Widget
GolitFetchPopups OL_ARGS((String, GolitWidgetDescPtr, Widget, ArgList,
			  Cardinal, XtPointer));
	/* String,				name		*/
	/* register GolitWidgetDescPtr,		wdp		*/
	/* Widget,				parent		*/
	/* ArgList,				args		*/
	/* Cardinal,				num_args	*/
	/* XtPointer,				closure		*/

extern Widget
GolitFetchPopupsUnmanaged OL_ARGS((String, GolitWidgetDescPtr, Widget,
				   ArgList, Cardinal, XtPointer));
	/* String,				name		*/
	/* register GolitWidgetDescPtr,		wdp		*/
	/* Widget,				parent		*/
	/* ArgList,				args		*/
	/* Cardinal,				num_args	*/
	/* XtPointer,				closure		*/

extern Widget
GolitFetchWidgetHier OL_ARGS((String, GolitWidgetDescPtr, Widget, ArgList,
			      Cardinal, XtPointer));
	/* String,				name		*/
	/* register GolitWidgetDescPtr,		wdp		*/
	/* Widget,				parent		*/
	/* ArgList,				args		*/
	/* Cardinal,				num_args	*/
	/* XtPointer,				closure		*/

extern Widget
GolitFetchWidgetHierUnmanaged OL_ARGS((String, GolitWidgetDescPtr, Widget,
				       ArgList, Cardinal, XtPointer));
	/* String,				name		*/
	/* register GolitWidgetDescPtr,		wdp		*/
	/* Widget,				parent		*/
	/* ArgList,				args		*/
	/* Cardinal,				num_args	*/
	/* XtPointer,				closure		*/

extern Widget
GolitFetchShellHier OL_ARGS((String, String, GolitWidgetDescPtr, Display *,
			     ArgList, Cardinal, XtPointer));
	/* String,				app_name	*/
	/* String,				app_class	*/
	/* register GolitWidgetDescPtr,		wdp		*/
	/* Display,				*dpy		*/
	/* ArgList,				args		*/
	/* Cardinal,				num_args	*/
	/* XtPointer,				closure		*/

typedef Widget
(*GolitWidgetCreateProc) OL_ARGS((String, GolitWidgetDescPtr, Widget, ArgList,
				  Cardinal, XtPointer));
	/* String,				name		*/
	/* register GolitWidgetDescPtr,		wdp		*/
	/* Widget,				parent		*/
	/* ArgList,				args		*/
	/* Cardinal,				num_args	*/
	/* XtPointer,				closure		*/


typedef Widget
(*GolitShellWidgetCreateProc) OL_ARGS(( String, String, GolitWidgetDescPtr,
					Display *, ArgList, 
					Cardinal, XtPointer));
	/* String,				app_name	*/
	/* String,				app_class	*/
	/* register GolitWidgetDescPtr,		wdp		*/
	/* Display,				*dpy		*/
	/* ArgList,				args		*/
	/* Cardinal,				num_args	*/
	/* XtPointer,				closure		*/

extern	GolitWidgetCreateProc		_GolitWDDefaultCPList[];

extern	GolitShellWidgetCreateProc	_GolitWDDefaultShellCPList[];

extern	GolitWidgetCreateProc		_GolitWDDefaultUnmanagedCPList[];

extern	GolitWidgetCreateProc		_GolitWDOnlyCreatedCPList[];

extern	GolitWidgetCreateProc		_GolitWDChildrenOnlyCPList[];

extern	GolitWidgetCreateProc		_GolitWDMenuButtonCPList[];

extern	GolitWidgetCreateProc		_GolitWDPopupWindowCPList[];

extern Widget
GolitPopupWindowCreateProc OL_ARGS((String, GolitWidgetDescPtr, Widget, 
				    ArgList, Cardinal, XtPointer));
	/* String,				name		*/
	/* register GolitWidgetDescPtr,		wdp		*/
	/* Widget,				parent		*/
	/* ArgList,				args		*/
	/* Cardinal,				num_args	*/
	/* XtPointer,				closure		*/

extern Widget
GolitMenuButtonCreateProc OL_ARGS((String, GolitWidgetDescPtr, Widget, 
				   ArgList, Cardinal, XtPointer));
	/* String,				name		*/
	/* register GolitWidgetDescPtr,		wdp		*/
	/* Widget,				parent		*/
	/* ArgList,				args		*/
	/* Cardinal,				num_args	*/
	/* XtPointer,				closure		*/

OLEndFunctionPrototypeBlock
#endif	/* libgolit_h */
