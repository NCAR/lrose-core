#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)path.c 1.18 93/06/29";
#endif
#endif

/*
 *	(c) Copyright 1992, 1993 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE file
 *	for terms of the license.
 */


#include <stdio.h>
#include <unistd.h>
#include <sys/param.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview_private/path_impl.h>
#include <xview_private/i18n_impl.h>
#include <xview_private/xv_path_util.h>

Pkg_private Panel_setting	xv_path_name_notify_proc();



/*
 * xv_create() method
 */
/* ARGSUSED */
Pkg_private int
path_init_avlist ( owner, public, avlist )
     Xv_opaque owner;
     Path_public *public;
     Attr_avlist avlist;
{
    Path_private *private = xv_alloc( Path_private );
    static Attr_avlist path_avlist = NULL;

    public->private_data = (Xv_opaque) private;
    private->public_self = (Xv_opaque) public;

    private->frame = xv_get(owner, WIN_FRAME);
    private->notify_status = XV_OK;

    if ( !path_avlist )
	path_avlist 
	    = attr_create_list( PANEL_NOTIFY_PROC,	xv_path_name_notify_proc,
			       PANEL_VALUE_STORED_LENGTH, MAXPATHLEN+1,
			       PANEL_PAINT, 		PANEL_NONE,
			       NULL );
    /*
     *  Note:  PANEL_NOTIFY_PROC is dealt with in the Panel Item package, not the
     *  Panel Text.  since we don't have the scope operator in C, fudge the issue
     *  by calling xv_super_set_avlist() on Panel Item pakcage as being the parent
     *  of our parent, the Panel Text package.  what we really need here is an
     *  xv_super_duper_set_avlist() function.
     */
    (void) xv_super_set_avlist((Xv_opaque) public, path_pkg.parent_pkg, path_avlist);
    
    return XV_OK;
} /* path_init() */




/*
 * xv_set() method
 */
Pkg_private Xv_opaque
path_set_avlist ( public, avlist )
     Path_name public;
     Attr_avlist avlist;
{
    Path_private *private = PATH_PRIVATE(public);
    Attr_avlist attrs;

    for (attrs=avlist; *attrs; attrs=attr_next(attrs)) {
	switch ( (int) attrs[0] ) {
	case PATH_IS_DIRECTORY:
	    ATTR_CONSUME(attrs[0]);

	    /*
	     * Changing this value may invalidate the last
	     * valid path if it is set TRUE, so NULL it out.
	     */
	    if ( private->is_directory != (int) attrs[1]
		&& (int) attrs[1] == TRUE
		&& private->valid_path 
		&& !xv_isdir(private->valid_path) ) {
			xv_free_ref( private->valid_path );
		}
	    private->is_directory = (int) attrs[1];
	    break;

	case PATH_USE_FRAME:
	    ATTR_CONSUME(attrs[0]);
	    private->use_frame = (int) attrs[1];
	    break;

#ifdef OW_I18N
	case PATH_RELATIVE_TO_WCS:
	    ATTR_CONSUME(attrs[0]);

	    xv_free_ref( private->relative );
	    private->relative = _xv_wcstombsdup( (wchar_t *)attrs[1] );
	    break;
#endif	/* OW_I18N */

	case PATH_RELATIVE_TO:
	    ATTR_CONSUME(attrs[0]);
	    private->relative = xv_strcpy( private->relative, 
					  (char *)attrs[1] 
					  );
	    break;

	case PATH_LAST_VALIDATED:
#ifdef OW_I18N
	case PATH_LAST_VALIDATED_WCS:
#endif
	    ATTR_CONSUME(attrs[0]);
	    xv_error( public,
		     ERROR_CANNOT_SET,	attrs[0],
		     ERROR_PKG,		PATH_NAME,
		     NULL );
	    break;

	case PATH_IS_NEW_FILE:
	    ATTR_CONSUME(attrs[0]);
	    private->new_file = (unsigned) attrs[1];
	    break;

	case PANEL_NOTIFY_PROC:
#ifdef OW_I18N
	case PANEL_NOTIFY_PROC_WCS:
#endif
	    ATTR_CONSUME(attrs[0]);
	    private->client_notify = (Panel_setting (*)()) attrs[1];
	    break;

	case PANEL_NOTIFY_STATUS:
	    ATTR_CONSUME(attrs[0]);
	    private->notify_status = (int) attrs[1];
	    break;

	case XV_END_CREATE:
	    break;

	default:
	    xv_check_bad_attr(PATH_NAME, attrs[0]);
	    break;
	} /* switch() */
    } /* for() */

    return XV_OK;
} /* path_set() */



/*
 * xv_get() method
 */
/* ARGSUSED */
Pkg_private Xv_opaque
path_get_attr ( public, status, attr, args )
     Path_public     *public;
     int             *status;
     Attr_attribute  attr;
     Attr_avlist     args;
{
    Path_private *private = PATH_PRIVATE(public);

    switch ( (int) attr ) {
    case PATH_IS_DIRECTORY:
	return (Xv_opaque) private->is_directory;

    case PATH_USE_FRAME:
	return (Xv_opaque) private->use_frame;

    case PATH_RELATIVE_TO:
	return (Xv_opaque) private->relative;

    case PATH_LAST_VALIDATED:
	return (Xv_opaque) private->valid_path;

#ifdef OW_I18N
    case PATH_RELATIVE_TO_WCS: {
	xv_free_ref( private->relative_wcs );
	private->relative_wcs = _xv_mbstowcsdup( private->relative );
	return (Xv_opaque) private->relative_wcs;
    }

    case PATH_LAST_VALIDATED_WCS: {
	xv_free_ref( private->valid_path_wcs );
	private->valid_path_wcs = _xv_mbstowcsdup( private->valid_path );
	return (Xv_opaque) private->valid_path_wcs;
    }
#endif /* OW_I18N */

    case PATH_IS_NEW_FILE:
	return (Xv_opaque) private->new_file;

    case PANEL_NOTIFY_PROC:
	return (Xv_opaque) private->client_notify;

    case PANEL_NOTIFY_STATUS:
	return (Xv_opaque) private->notify_status;

    default :
	*status = xv_check_bad_attr(PATH_NAME, attr);
	return (Xv_opaque)XV_OK;
    } /* switch */

} /* path_get() */



/*
 * xv_destroy() method
 */
Pkg_private int
path_destroy_private ( public, status )
     Path_public *public;
     Destroy_status status;
{
    Path_private *private = PATH_PRIVATE(public);

    if (status != DESTROY_CLEANUP)
	return XV_OK;

    xv_free_ref( private->valid_path );
    xv_free_ref( private->relative );

#ifdef OW_I18N
    xv_free_ref( private->valid_path_wcs );
    xv_free_ref( private->relative_wcs );
#endif

    xv_free ( private );

    return XV_OK;
} /* path_destroy() */


/*******************************************************************************/



/*
 * Notify routine that expands the path and checks it for validity
 * before notifying the user.  even though this seems simple, it has
 * the advantages that it is scallable if someone wants to add filename
 * completion some day, and it hides the reference to expand_path which
 * is not exactly a public, but also not exactly a private call.
 */
Pkg_private Panel_setting
xv_path_name_notify_proc( item, event )
     Path_name item;
     Event *event;
{
    Path_private *private = PATH_PRIVATE(item);
    struct stat sbuf;
    int is_new_file = FALSE;
    char *path = (char *)xv_get(item, PANEL_VALUE);
    char *full_path;
    char *buf;
    char path_buf[MAXPATHLEN+1];


    /*
     * ASSume success...
     *
     * There are situations for which the success of the
     * validation need to be known.  the return value of
     * Panel_setting is not sufficient information, as
     * PANEL_NONE does not necessarily imply an error.
     * One example of this is handling a DefaultAction.
     */
    private->notify_status = XV_OK;


    /*
     * deal with null path.  client may or may not want to deal
     * with this itself.  If there is relative path, ASSume that.
     * is the value of the field.
     */
    if ( no_string(path) ) {
	int status = -1;

	if ( private->relative ) {
	    status = xv_stat(private->relative, &sbuf);
	    private->valid_path = xv_strcpy( private->valid_path,
					    private->relative
					    );
	}

	if ( private->client_notify )
	    return (* private->client_notify)(item, event, 
					      (status == -1) ? NULL : &sbuf
					      );
	return panel_text_notify(item, event);
    }



    /* Expand tilde and shell variables */
    buf = xv_expand_path( path );


    /* 
     * make relative paths absolute as per
     * PATH_RELATIVE_TO attribute.
     */
    if ( !is_root(buf) && private->relative && is_relative(buf) ) {
	full_path = xv_dircat( private->relative, buf );
    } else {
	full_path = xv_strcpy( NULL, buf );
    }
    xv_free( buf );


    /* deal with "../." types of things */
    if ( xv_realpath(full_path, path_buf) ) {
	full_path = xv_strcpy(full_path, path_buf);
    } else {
	xv_error_sprintf( private->frame, private->use_frame, 
			 XV_MSG("The folder name \"%s\" does not exist."), 
			 path 
			 );
	xv_free_ref( full_path );
	private->notify_status = XV_ERROR;
	return PANEL_NONE;
    }



    /* is this a valid entry? */
    if ( xv_stat( full_path, &sbuf ) < 0 ) {

	/* implements PATH_IS_NEW_FILE attr */
	if ( private->new_file ) {
	    char *dir = xv_dirpart( full_path );

	    if ( access( dir, W_OK ) == -1 ) {
		if ( private->client_notify )
		    return (* private->client_notify)(item, event, NULL );
		else
		    xv_error_sprintf( private->frame, private->use_frame, 
				     XV_MSG("Cannot create the document \"%s\"."), 
				     path 
				     );
		xv_free_ref( dir );
		xv_free_ref( full_path );
		private->notify_status = XV_ERROR;
		return PANEL_NONE;
	    }

	    is_new_file = TRUE;
	    xv_free_ref( dir );
	} else {
	    xv_error_sprintf( private->frame, private->use_frame, 
			     XV_MSG("The folder name \"%s\" does not exist."),
			     path 
			     );
	    xv_free_ref( full_path );
	    private->notify_status = XV_ERROR;
	    return PANEL_NONE;
	}
    }

    if (!is_new_file && private->is_directory && !S_ISDIR(sbuf.st_mode) ) {
	xv_error_sprintf( private->frame, private->use_frame, 
			 XV_MSG("\"%s\" is not a folder."),
			 path 
			 );
	xv_free_ref( full_path );
	private->notify_status = XV_ERROR;
	return PANEL_NONE;
    }


    /* save valid path name */
    xv_free_ref( private->valid_path );
    private->valid_path = full_path;

    /* notify the client */
    if ( private->client_notify )
	return (* private->client_notify)(item, event, is_new_file ? NULL : &sbuf );
    return panel_text_notify(item, event);
} /* xv_path_name_notify_proc() */
