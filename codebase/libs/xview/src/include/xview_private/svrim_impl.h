/* @(#)svrim_impl.h 20.21 93/06/28 */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#ifndef _xview_server_image_impl_h_already_included
#define _xview_server_image_impl_h_already_included

#include <xview/svrimage.h>
#include <xview/screen.h>
#include <xview/pixwin.h>
#include <xview_private/draw_impl.h>
#include <xview_private/pw_impl.h>

typedef struct {
    Server_image	public_self; /* Back pointer */
    Xv_Screen		screen; /* screen for the server_image */
    short		save_pixmap;     
}   Server_image_info;

#define SERVER_IMAGE_PRIVATE(image) \
	XV_PRIVATE(Server_image_info, Xv_server_image, image)
#define SERVER_IMAGE_PUBLIC(image)  XV_PUBLIC(image)

/* default values for server image attributes */
#define  SERVER_IMAGE_DEFAULT_DEPTH	1
#define  SERVER_IMAGE_DEFAULT_WIDTH	16
#define  SERVER_IMAGE_DEFAULT_HEIGHT	16


/* from server_image_public.c */
Pkg_private int              server_image_create_internal();
Pkg_private Xv_opaque        server_image_set_internal();
Pkg_private Xv_opaque        server_image_get_internal();
Pkg_private int              server_image_destroy_internal();

Xv_private GC       xv_find_proper_gc();
Xv_private Pixrect  *xv_mem_create();

#endif /*  _xview_server_image_impl_h_already_included */
