/*
 * @(#)gdd.h	2.19 91/10/15 Copyright 1991 Sun Microsystems.
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
 * Drag-n-drop interface.
 */

#ifndef guide_gdd_DEFINED
#define guide_gdd_DEFINED

#include	<devguide/c_varieties.h>

/*
 * Data structure used by the drag and drop package to exchange
 * information with the application.
 */
typedef struct Drop_info {
	char	*app_name;		/* Application's name. Set/Get     */
	char	*data_label;		/* Label of chunk of data. Set/Get */
	char	*source_host;		/* Hostname of source. Get         */
	char	*filename;		/* Filename. Set/Get	           */
	int	 length;		/* Length of data in bytes. Set/Get*/
	char 	*data;			/* Pointer to chunk of data.Set/Get*/
	char	*tmpfile;		/* Tempfile name. Get              */
	} GDD_DROP_INFO;

/*
 * Public Defines
 */
#define GDD_DRAG_STARTED	1
#define GDD_DRAG_COMPLETED 	2
/*
 * Public functions.
 */
EXTERN_FUNCTION( void	gdd_init_dragdrop, (Xv_Window) );
EXTERN_FUNCTION( void	gdd_register_drop_site, (Xv_opaque, void (*)(Xv_opaque, Event *, GDD_DROP_INFO *)) );
EXTERN_FUNCTION( void	gdd_register_drop_target, (Xv_opaque, void (*)(Xv_opaque, Event *, GDD_DROP_INFO *), void (*)(Xv_opaque, Event *, GDD_DROP_INFO *, int)) );
EXTERN_FUNCTION( void	gdd_unregister_drop_site, (Xv_opaque) );
EXTERN_FUNCTION( void	gdd_activate_drop_site, (Xv_opaque, Rect *) );
EXTERN_FUNCTION( Notify_value	gdd_load_event_proc, (Xv_window, Event *, Notify_arg, Notify_event_type) );
EXTERN_FUNCTION( int	gdd_drop_target_notify_proc, (Panel_item, unsigned int, Event *) );
EXTERN_FUNCTION( void	gdd_print_drop_info, (GDD_DROP_INFO *) );
EXTERN_FUNCTION( int	gdd_get_drag_name, (Xv_Window, char *) );
EXTERN_FUNCTION( int	gdd_set_drag_name, (Xv_window, Xv_window, int, int, char *) );
EXTERN_FUNCTION( int	gdd_drag_file, (Xv_window, Server_image, char *) );

#endif

