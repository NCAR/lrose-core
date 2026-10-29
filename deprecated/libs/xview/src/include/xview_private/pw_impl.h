/*	@(#)pw_impl.h 20.17 89/08/18 SMI	*/

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#ifndef xv_pw_impl_h_already_defined
#define	xv_pw_impl_h_already_defined

#include <sys/types.h>
#include <pixrect/pixrect.h>
#include <xview/pkg.h>
#include <xview/pixwin.h>
#include <X11/Xlib.h>
#include <xview_private/draw_impl.h>

#ifdef i386
extern 	   struct pixrectops mem_ops;
#endif /* ~i386 */
Xv_private struct pixrectops	server_image_ops;
Xv_private int			xv_to_xop[];

#define PIX_OP_SHIFT    1
#ifndef PIX_OP
#define	PIX_OP(_op)	((_op) & PIX_NOT(0))
#endif /* PIX_OP */
#define XV_TO_XOP(_op)	(xv_to_xop[PIX_OP(_op) >> PIX_OP_SHIFT])

#define SERVER_IMAGE_PR  	1
#define MEMORY_PR 		2
#define OTHER_PR  		3
#define PR_IS_MPR(pr)		(((Pixrect *)pr)->pr_ops == &mem_ops)
#define PR_NOT_MPR(pr)		(((Pixrect *)pr)->pr_ops != &mem_ops)
#define PR_IS_SERVER_IMAGE(pr)	(((Pixrect *)pr)->pr_ops == &server_image_ops)
#define PR_NOT_SERVER_IMAGE(pr)	(((Pixrect *)pr)->pr_ops != &server_image_ops)
#define PR_TYPE(pr)		PR_IS_MPR(pr) ? MEMORY_PR : \
				(PR_IS_SERVER_IMAGE(pr) ? SERVER_IMAGE_PR :  OTHER_PR)

Xv_private void		xv_set_gc_op();
Xv_private GC		xv_find_proper_gc();

struct gc_chain {
        struct gc_chain *next;
        GC               gc;
        int              depth;
        Drawable         xid;
	short		 clipping_set;
};

#endif /* xv_pw_impl_h_already_defined */
