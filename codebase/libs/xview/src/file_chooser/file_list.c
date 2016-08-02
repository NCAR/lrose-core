#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)file_list.c 1.30 93/06/28";
#endif
#endif
 

/*
 *	(c) Copyright 1992, 1993 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE file
 *	for terms of the license.
 */

#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/types.h>
#include <unistd.h>
#include <xview/xview.h>
#include <xview/svrimage.h>
#include <xview_private/flist_impl.h>
#include <xview_private/portable.h>



/* X bitmaps for default glyphs */
#include <images/fl_arrow.xbm>
#include <images/fl_folder.xbm>
#include <images/fl_doc.xbm>


static int		can_change_to_dir();
static int		flist_load_dir();
static File_list_row *	flist_next_row();
static void		flist_update_list();
static void		flist_compile_regex();
static int              flist_match_regex();
static int		go_up_one_directory();
static int		go_down_one_directory();
static int		flist_list_notify();
static int		validate_new_directory();
static void		flist_new_dir();

#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
static void	flist_error(File_list_private *private, char *format, ...);
#else
static void	flist_error();
#endif

	

/*
 * xv_create() method
 */
Pkg_private int
file_list_init( owner, public, avlist )
     Xv_opaque owner;
     File_list_public *public;
     Attr_avlist avlist;
{
    File_list_private *private = xv_alloc( File_list_private );
    Xv_Screen screen = XV_SCREEN_FROM_WINDOW(owner);
    Attr_avlist list_avlist;
    int row_height;
    int font_height;
    Xv_font font;

    public->private_data = (Xv_opaque) private;
    private->public_self = (Xv_opaque) public;

    private->owner = owner;
    private->frame = (Frame)xv_get(private->owner, WIN_FRAME);
    private->f.auto_update = TRUE;
    private->f.show_dotfiles = FALSE;
    private->dotdot_string = xv_strcpy(NULL, DEFAULT_DOTDOT_STRING);
    private->f.abbrev_view = FALSE;
    private->compare_func = FILE_LIST_DEFAULT_COMPARE_FUNC;
    private->filter_mask = FL_MATCHED_FILES_MASK;
    private->directory = (char *)getcwd(NULL, MAXPATHLEN);
    (void) can_change_to_dir( private, private->directory );


    /*
     * Figure out how tall to make each row.  the rule of thumb
     * being used here, which seems to come out fairly close to
     * the OL spec, is that the row height equals the font height
     * plus 30% on top and 30% beneath.
     */
    font = xv_get(owner, XV_FONT);
    font_height = (int) xv_get(font, FONT_DEFAULT_CHAR_HEIGHT);
    row_height = font_height + (2 * (.3 * font_height));


    /* add our notify func so we can detect double clicks */
    list_avlist 
	= attr_create_list( PANEL_NOTIFY_PROC,	flist_list_notify,
			   PANEL_LIST_DO_DBL_CLICK,	TRUE,
			   PANEL_READ_ONLY,		TRUE,
			   PANEL_LIST_DISPLAY_ROWS,	DEFAULT_LIST_ROWS,
			   PANEL_LIST_ROW_HEIGHT,	row_height,
			   PANEL_PAINT, 		PANEL_NONE,
			   NULL );
    (void) xv_super_set_avlist((Xv_opaque) public, FILE_LIST, list_avlist);
    xv_free( list_avlist );


    /*
     * Initialize the default glyphs
     */
    private->directory_glyph
	= xv_create(screen, SERVER_IMAGE,
		    XV_WIDTH, 			fl_folder_width,
		    XV_HEIGHT, 			fl_folder_height,
		    SERVER_IMAGE_X_BITS, 	fl_folder_bits,
		    NULL);
    
    private->file_glyph
	= xv_create(screen, SERVER_IMAGE,
		    XV_WIDTH, 			fl_doc_width,
		    XV_HEIGHT, 			fl_doc_height,
		    SERVER_IMAGE_X_BITS, 	fl_doc_bits,
		    NULL);
    
    private->dotdot_glyph
	= xv_create(screen, SERVER_IMAGE,
		    XV_WIDTH, 			fl_arrow_width,
		    XV_HEIGHT, 			fl_arrow_height,
		    SERVER_IMAGE_X_BITS, 	fl_arrow_bits,
		    NULL);
    
    return XV_OK;
} /* file_list_init() */




/*
 * xv_set() method
 */
Pkg_private Xv_opaque
file_list_set ( public, avlist )
     File_list public;
     Attr_avlist avlist;
{
    File_list_private *private = FILE_LIST_PRIVATE(public);
    Attr_avlist attrs;
    int do_update = FALSE;	/* update the list */


    for (attrs=avlist; *attrs; attrs=attr_next(attrs)) {
	switch ( (int) attrs[0] ) {
#ifdef OW_I18N
	case FILE_LIST_DIRECTORY_WCS:
#endif
	case FILE_LIST_DIRECTORY: {
	    char *new_dir;
	    char *real_path;

#ifdef OW_I18N
	    if ( attrs[0] == FILE_LIST_DIRECTORY_WCS )
		new_dir = _xv_wcstombsdup( (wchar_t *) attrs[1] );
	    else
#endif
		new_dir = xv_strcpy(NULL, (char *)attrs[1]);
	    
	    ATTR_CONSUME(attrs[0]);

	    if ( no_string(new_dir) ) {
		xv_free_ref( private->directory );
		if ( private->dir_ptr ) {
		    (void) closedir( private->dir_ptr );
		    private->dir_ptr = (DIR *)NULL;
		}
		private->f.new_dir = FALSE;
		xv_set(public,
		       PANEL_LIST_DELETE_ROWS, 
		       		0, xv_get(public, PANEL_LIST_NROWS),
		       PANEL_PAINT, PANEL_NONE,
		       NULL);
		if ( private->f.show_dir )
		    xv_set(public, 
			   PANEL_LIST_TITLE, "",
			   PANEL_PAINT, PANEL_NONE,
			   NULL);
		xv_free_ref( new_dir );
		break;
	    }

	    real_path = xv_expand_path( new_dir );

	    if ( !validate_new_directory(private, real_path) ) {
		char msg[1024];
		(void) sprintf(msg, XV_MSG("Can't change to %s"), new_dir);
		xv_error( public,
			 ERROR_PKG,	FILE_LIST,
			 ERROR_STRING,	msg,
			 NULL);
		xv_free_ref( new_dir );
		xv_free_ref( real_path );
		break;
	    }
	    xv_free_ref( new_dir );
	    xv_free_ref( real_path );

	    if ( private->f.show_dir )
		xv_set(public, 
		       PANEL_LIST_TITLE, private->directory, 
		       PANEL_PAINT, PANEL_NONE,
		       NULL);
	    if ( private->f.auto_update )
		do_update = TRUE;
	    break;
	}			/* FILE_LIST_DIRECTORY */



#ifdef OW_I18N
	case FILE_LIST_FILTER_STRING_WCS:
#endif
	case FILE_LIST_FILTER_STRING:
#ifdef OW_I18N
	    if ( attrs[0] == FILE_LIST_FILTER_STRING_WCS ) {
		xv_free_ref( private->regex_pattern );
		private->regex_pattern = _xv_wcstombsdup( (wchar_t *) attrs[1] );
	    } else
#endif
	    private->regex_pattern 
		= xv_strcpy(private->regex_pattern, (char *)attrs[1]);
	    if ( private->regex_pattern )
		flist_compile_regex( private );
	    if ( private->f.auto_update )
		do_update = TRUE;
	    ATTR_CONSUME(attrs[0]);
	    break;


	case FILE_LIST_MATCH_GLYPH:
	    ATTR_CONSUME(attrs[0]);
	    private->match_glyph = (Server_image) attrs[1];
	    if ( private->f.auto_update )
		do_update = TRUE;
	    break;

	case FILE_LIST_MATCH_GLYPH_MASK:
	    ATTR_CONSUME(attrs[0]);
	    private->match_glyph_mask = (Server_image) attrs[1];
	    if ( private->f.auto_update )
		do_update = TRUE;
	    break;

	case FILE_LIST_FILTER_FUNC:
	    ATTR_CONSUME(attrs[0]);
	    private->client_filter = (File_list_op (*)()) attrs[1];
	    if ( private->f.auto_update )
		do_update = TRUE;
	    break;

	case FILE_LIST_FILTER_MASK:
	    ATTR_CONSUME(attrs[0]);
	    private->filter_mask = (unsigned short) attrs[1];
	    if ( private->f.auto_update )
		do_update = TRUE;
	    break;

	case FILE_LIST_COMPARE_FUNC:
	    ATTR_CONSUME(attrs[0]);
	    private->compare_func = (int (*)()) attrs[1];
	    if ( private->f.auto_update )
		do_update = TRUE;
	    break;

	case FILE_LIST_CHANGE_DIR_FUNC:
	    ATTR_CONSUME(attrs[0]);
	    private->cd_func = (int (*)()) attrs[1];
	    break;

	case FILE_LIST_SHOW_DOT_FILES:
	    ATTR_CONSUME(attrs[0]);
	    private->f.show_dotfiles = (unsigned) ((attrs[1]) ? TRUE : FALSE);
	    if ( private->f.auto_update )
		do_update = TRUE;
	    break;

	case FILE_LIST_AUTO_UPDATE:
	    ATTR_CONSUME(attrs[0]);
	    /* should be set BEFORE anything changes */
	    private->f.auto_update = (unsigned) ((attrs[1]) ? TRUE : FALSE);
	    break;

	case FILE_LIST_UPDATE:
	    ATTR_CONSUME(attrs[0]);
	    do_update = TRUE;
	    break;

	case FILE_LIST_SHOW_DIR:
	    ATTR_CONSUME(attrs[0]);
	    private->f.show_dir = (unsigned) ((attrs[1]) ? TRUE : FALSE);
	    xv_set(public, 
		   PANEL_LIST_TITLE, (private->f.show_dir) 
		   			? private->directory : NULL,
		   PANEL_PAINT, PANEL_NONE,
		   NULL );
	    break;

	case FILE_LIST_USE_FRAME:
	    ATTR_CONSUME(attrs[0]);
	    private->f.use_frame = (unsigned) ((attrs[1]) ? TRUE : FALSE);
	    break;


#ifdef OW_I18N
	case FILE_LIST_DOTDOT_STRING_WCS:
#endif
	case FILE_LIST_DOTDOT_STRING:

#ifdef OW_I18N
	    if ( attrs[0] == FILE_LIST_DOTDOT_STRING_WCS ) {
		xv_free_ref( private->dotdot_string );
		private->dotdot_string = _xv_wcstombsdup( (wchar_t *) attrs[1] );
	    } else
#endif
	    private->dotdot_string 
		= xv_strcpy(private->dotdot_string, (char *) attrs[1]);
	    if ( private->f.auto_update )
		do_update = TRUE;
	    ATTR_CONSUME(attrs[0]);
	    break;



	case FILE_LIST_ABBREV_VIEW:
	    ATTR_CONSUME(attrs[0]);
	    private->f.abbrev_view = (unsigned) ((attrs[1]) ? TRUE : FALSE);
	    if ( private->f.auto_update )
		do_update = TRUE;
	    break;

	case FILE_LIST_ROW_TYPE:
	    xv_error( public,
		     ERROR_PKG,		FILE_LIST,
		     ERROR_CANNOT_SET,	attrs[0],
		     NULL );
	    break;

#ifdef OW_I18N
	case FILE_LIST_WCHAR_NOTIFY:
	    ATTR_CONSUME(attrs[0]);
	    private->f.wchar_notify = (unsigned) attrs[1];
	    break;
#endif

	case PANEL_LIST_SORT:
	    /* FILE_LIST has a different sorting mechanism */
	    ATTR_CONSUME(attrs[0]);
	    break;

	case PANEL_LIST_DISPLAY_ROWS:
	    /* don't consume! */

	    /*
	     * OL Spec says list cannot be smaller than 3 rows:
	     * one for the "go up one" entry, and 2 files.
	     */
	    if ( attrs[1] < MIN_LIST_ROWS )
		attrs[1] = MIN_LIST_ROWS;

	    /* BUG:  should call flist_error? */
	    break;


	case PANEL_NOTIFY_PROC:
	    ATTR_CONSUME(attrs[0]);
	    /*
	     * must intercept this, because the Panel_list's notify
	     * func is needed by the File_list to do double-click.
	     */
	    private->client_notify = (int (*)()) attrs[1];
#ifdef OW_I18N
	    private->f.wchar_list_notify = FALSE;
#endif
	    break;

#ifdef OW_I18N
	case PANEL_NOTIFY_PROC_WCS:
	    ATTR_CONSUME(attrs[0]);
	    private->client_notify = (int (*)()) attrs[1];
	    private->f.wchar_list_notify = TRUE;
	    break;
#endif

	case XV_END_CREATE:
	    private->f.created = TRUE;
	    if ( private->directory )
		do_update = TRUE;
	    break;

	default:
	    xv_check_bad_attr ( FILE_LIST, attrs[0] );
	    break;
	} /* switch() */
    } /* for() */


    if ( do_update && private->directory && private->f.created )
	(void) flist_load_dir( private, private->directory );

    return XV_OK;
} /* file_list_set() */



/*
 * xv_get() method
 */
Pkg_private Xv_opaque
file_list_get( public, status, attr, valist )
     File_list_public   *public;
     int             	*status;
     Attr_attribute  	attr;
     va_list         	valist;
{
    File_list_private *private = FILE_LIST_PRIVATE(public);

    switch ( (int) attr ) {
    case FILE_LIST_ROW_TYPE:
	return xv_get( FILE_LIST_PUBLIC(private),
		      PANEL_LIST_EXTENSION_DATA, 
/* Alpha compatibility, mbuck@debian.org */
#if 1
		      va_arg(valist, int *)
#else
		      va_arg(valist, int)
#endif
		      );

    case FILE_LIST_DIRECTORY:
	return (Xv_opaque) private->directory;

#ifdef OW_I18N
    case FILE_LIST_DIRECTORY_WCS:
	xv_free_ref( private->directory_wcs );
	private->directory_wcs = _xv_mbstowcsdup( private->directory );
	return (Xv_opaque) private->directory_wcs;
#endif

    case FILE_LIST_FILTER_STRING:
	return (Xv_opaque) private->regex_pattern;

    case FILE_LIST_MATCH_GLYPH:
	return (Xv_opaque) private->match_glyph;

    case FILE_LIST_MATCH_GLYPH_MASK:
	return (Xv_opaque) private->match_glyph_mask;

    case FILE_LIST_FILTER_FUNC:
	return (Xv_opaque) private->client_filter;

    case FILE_LIST_FILTER_MASK:
	return (Xv_opaque) private->filter_mask;

    case FILE_LIST_COMPARE_FUNC:
	return (Xv_opaque) private->compare_func;

    case FILE_LIST_SHOW_DOT_FILES:
	return (Xv_opaque) private->f.show_dotfiles;

    case FILE_LIST_AUTO_UPDATE:
	return (Xv_opaque) private->f.auto_update;

    case FILE_LIST_SHOW_DIR:
	return (Xv_opaque) private->f.show_dir;

    case FILE_LIST_CHANGE_DIR_FUNC:
	return (Xv_opaque) private->cd_func;

    case FILE_LIST_USE_FRAME:
	return (Xv_opaque) private->f.use_frame;

    case FILE_LIST_DOTDOT_STRING:
	return (Xv_opaque) private->dotdot_string;

    case FILE_LIST_ABBREV_VIEW:
	return (Xv_opaque) private->f.abbrev_view;

    case PANEL_NOTIFY_PROC:
	return (Xv_opaque) private->client_notify;

#ifdef OW_I18N
    case FILE_LIST_FILTER_STRING_WCS:
	xv_free_ref( private->regex_pattern_wcs );
	private->regex_pattern_wcs = _xv_mbstowcsdup( private->regex_pattern );
	return (Xv_opaque) private->regex_pattern_wcs;

    case FILE_LIST_DOTDOT_STRING_WCS:
	xv_free_ref( private->dotdot_string_wcs );
	private->dotdot_string_wcs = _xv_mbstowcsdup( private->dotdot_string );
	return (Xv_opaque) private->dotdot_string_wcs;

    case FILE_LIST_WCHAR_NOTIFY:
	return (Xv_opaque) private->f.wchar_notify;

    case PANEL_NOTIFY_PROC_WCS:
	return (Xv_opaque) private->client_notify;
#endif /* OW_I18N */

    default :
	*status = xv_check_bad_attr ( FILE_LIST, attr );
	return (Xv_opaque) XV_OK;
    } /* switch */

} /* file_list_get() */



/*
 * xv_destroy() method
 */
Pkg_private int
file_list_destroy ( public, status )
     File_list_public *public;
     Destroy_status status;
{
    File_list_private *private = FILE_LIST_PRIVATE(public);

    if (status == DESTROY_CLEANUP) {
	xv_free_ref( private->directory );
	xv_free_ref( private->regex_pattern );
#ifdef __linux
	if (private->regex_compile != NULL && private->regex_compile->allocated)
		xv_free_ref( private->regex_compile->buffer);
#endif
	xv_free_ref( private->regex_compile );
	xv_free_ref( private->dotdot_string );
	if ( private->dir_ptr )
	    (void) closedir( private->dir_ptr );

	/* destroy the default images */
	xv_destroy( private->file_glyph );
	xv_destroy( private->directory_glyph );
	xv_destroy( private->dotdot_glyph );

#ifdef OW_I18N
	xv_free_ref( private->directory_wcs );
	xv_free_ref( private->regex_pattern_wcs );
	xv_free_ref( private->dotdot_string_wcs );
#endif /* OW_I18N */

        xv_free( private );
    }
    return XV_OK;
} /* file_list_destroy() */


/*******************************************************************************/



/*
 * Give error message.  if the user has OK'd it for us to
 * use the frame, put message in the footer.  else, use xv_error().
 */
static void
#ifdef ANSI_FUNC_PROTO
flist_error( File_list_private *private, char *format, ... )
#else
flist_error(private, format,  va_alist)
     File_list_private *private;
     char *format;
va_dcl
#endif
{
    char buf[MAX_MSG_SIZE];
    AVLIST_DECL;
    va_list list;

    VA_START(list, format);
    (void) vsprintf(buf, format, list);
    va_end(list);

    if ( private->f.use_frame 
	&& (int)xv_get(private->frame, FRAME_SHOW_FOOTER) ) {
	window_bell( private->frame );
	xv_set(private->frame, FRAME_LEFT_FOOTER, buf, NULL);
    } else {
	xv_error( FILE_LIST_PUBLIC(private),
		 ERROR_PKG,	FILE_LIST,
		 ERROR_STRING,	buf,
		 NULL );
    }
} /* flist_error() */





/*
 * Load directory into File List.
 */
static int
flist_load_dir( private, directory )
     File_list_private *private;
     char *directory;
{
    File_list public = FILE_LIST_PUBLIC(private);
    struct dirent *entry;
    struct stat stats;
    int row_num = 0;
    char file_path[MAXPATHLEN+1];
    char xfrm_buf[MAXPATHLEN+1];
    File_list_op status = FILE_LIST_ACCEPT;
    File_list_row *current;
    static File_list_row *rows = NULL; /* array of row entries */
    char *cwd = (char *)NULL;
    int cd_status = XV_OK;

#ifdef OW_I18N
    wchar_t *	dir_wcs = (wchar_t *)NULL;
#endif


    if ( !directory )
	return FALSE;


    if ( private->f.use_frame )
	xv_set(private->frame, FRAME_LEFT_FOOTER, "", NULL);


    /*
     * Notify client that we are about to change directories.
     * this also affords him a last chance to cancel.
     *
     * also, note that this does not call flist_error if an
     * error is returned.  since i don't know why the user 
     * canceled the change, i will anticipate that he will
     * provide the proper status message if appropriate.
     */
    if ( private->cd_func ) {
	(void) xv_stat( private->directory, &stats );
#ifdef OW_I18N
	if ( private->f.wchar_notify ) {
	    dir_wcs = _xv_mbstowcsdup( private->directory );
	    cd_status = (* private->cd_func)(public, 
					     dir_wcs, 
					     &stats,
					     FILE_LIST_BEFORE_CD 
					     );
	} else
#endif
	    cd_status = (* private->cd_func)(public, 
					     private->directory, 
					     &stats,
					     FILE_LIST_BEFORE_CD 
					     );
	}


    /*
     * Gad-Zoooks!  The CD was aborted!
     */
    if ( cd_status == XV_ERROR ) {
#ifdef OW_I18N
	xv_free_ref( dir_wcs );
#endif
	private->directory 
	    = xv_strcpy(private->directory, private->previous_dir);
	return FALSE;
    }




    /*
     * if permission granted, turn frame busy automatically
     */
    if ( private->f.use_frame )
	xv_set(private->frame, FRAME_BUSY, TRUE, NULL);



    /*
     * Performance Note:  stat() spends much of it's time just
     * traversing the path name you give it, so
     */
    cwd = getcwd(NULL, MAXPATHLEN);
    chdir( private->directory );



    /* 
     * Make directory pointer current.  Note:  rewinddir() when the
     * position is already 0 has no system call overhead on 4.x, but
     * makes an lseek system call on 5.0 irreguardless.  to save on
     * this overhead, flag that this is the first time we have read
     * this directory (set in can_change_to_dir()).
     */
    if ( private->f.new_dir )
	private->f.new_dir = FALSE;
    else
	rewinddir( private->dir_ptr );
    




    /**************************************************************************/

    /*
     * POSIX.1 says that readdir() need not return "." and ".." at all,
     * let alone first.  Note that the auto-mounter can make entries
     * appear before them.  since we always want the ".." entry first,
     * synthesize it here, and skip it if it occurs in the loop.
     */

    current = flist_next_row( &rows, row_num );
    current->file_list = public;
    strcpy(file_path, private->directory);
    xv_dirname( file_path );
    (void) xv_stat( file_path, &(current->stats) );
    strcpy(file_path, "..");


    (void) strxfrm(xfrm_buf, file_path, MAXPATHLEN+1);
    current->xfrm = xv_strcpy(NULL, xfrm_buf);

#ifdef OW_I18N
    if ( private->f.wchar_notify )
	current->vals.string = (char *) _xv_mbstowcsdup( private->dotdot_string );
    else
#endif
    current->vals.string = xv_strcpy(NULL, private->dotdot_string);
    current->vals.glyph = private->dotdot_glyph;
    current->vals.mask_glyph = private->dotdot_glyph;
    current->matched = FILE_LIST_MATCHED;


    
    /*
     * Note:  ".." gets filtered, but not put through the regexp matching,
     * since it is not likely to conform to any prefix/suffix rules.  it
     * is assumed to match.
     */
    if ( private->client_filter && (private->filter_mask & FL_DOTDOT_MASK) ) {
#ifdef OW_I18N
	if ( private->f.wchar_notify ) {
	    wchar_t *wpath = _xv_mbstowcsdup( file_path );
	    status = (* private->client_filter)(wpath, current);
	    xv_free( wpath );
	} else
#endif
	status = (* private->client_filter)(file_path, current);
    }
    
    current->vals.extension_data = FILE_LIST_DOTDOT_TYPE;
    current->vals.selected = TRUE;


    /*
     * Do ".." entry as special case.  if item is not validated, then
     * gray it out, don't leave it out.  LAF Note:  this behavior seems
     * reasonable, but has not yet been aproved by OL.
     */
    current->vals.inactive = (status == FILE_LIST_IGNORE 
			      || is_root(private->directory)
			      );

    row_num++;

    /**************************************************************************/



    /*
     * Read rest of directory and load into list
     */
    while ( entry = readdir(private->dir_ptr) ) {

	/* skip "." and ".." entries -- see note above. */
	if ( is_dot(entry->d_name) || is_dotdot(entry->d_name) )
	    continue;

	/* skip "." file's if not selected */
	if ( !private->f.show_dotfiles && is_dot_file(entry->d_name) )
	    continue;


	/* set default flags */
	current = flist_next_row( &rows, row_num );
	current->file_list = public;
	current->vals.selected = FALSE;
	status = FILE_LIST_ACCEPT;
	current->matched = FILE_LIST_MATCHED;


	/*
	 * Parse Regular Expression Pattern.  Note:  matching a regex does
	 * not, necessarily exclude the entry from the list.  the filter func
	 * has a chance to validate it anyway.  this has 2 advantages;  1) is
	 * that it can look at a file and still include it if it wants, and 2).
	 * it can assign it a glyph even though it may get "grayed out".
	 */
	if ( private->regex_pattern && !flist_match_regex(entry->d_name, private) )
	    current->matched = FILE_LIST_NOT_MATCHED;
	


	/*
	 * Since we have to know if each file is a directory or a normal
	 * file, we must stat each file.
	 */
	(void) sprintf( file_path, "%s/%s", directory, entry->d_name );
	if ( xv_stat( entry->d_name, &(current->stats) ) < 0 )
	    continue;


	/*
	 * Figure out what type of entry we have and determine
	 * the actual name and default glyph we are going to put 
	 * in the list.  Also, by default, not-matched files are
	 * grayed out, but not directories.
	 */
#ifdef OW_I18N
	/*
	 * i18n impl note:
	 * the Panel_list_row_values and Panel_list_row_values_wcs
	 * structs are the same other than one has a char * and the
	 * other has a wchar_t * as it's first member.  so, rather
	 * than uglify the code to deal with both strutures, simply type
	 * cast the wchar into a multibyte.  The callbacks for wchar
	 * will be declaired with File_list_row_wcs, so they will be
	 * oblivious to this.
	 */
	if ( private->f.wchar_notify )
	    current->vals.string = (char *) _xv_mbstowcsdup( entry->d_name );
	else
#endif
	current->vals.string = xv_strcpy(NULL, entry->d_name);


	/*
	 * make xform copy for I18N-safe sorting
	 */
	(void) strxfrm(xfrm_buf, entry->d_name, MAXPATHLEN+1);
	current->xfrm = xv_strcpy(NULL, xfrm_buf);




	/*
	 * Set up default values
	 */
	if ( S_ISDIR(current->stats.st_mode) ) {
	    current->vals.glyph = private->directory_glyph;
	    current->vals.mask_glyph = private->directory_glyph;
	    current->vals.inactive = FALSE;
	} else {

	    /* Implements the FILE_LIST_MATCH_GLYPH functionality. */
	    if ( (current->matched == FILE_LIST_MATCHED) && private->match_glyph ) {
		current->vals.glyph = private->match_glyph;
		current->vals.mask_glyph = private->match_glyph_mask;
	    } else {
		current->vals.glyph = private->file_glyph;
		current->vals.mask_glyph = private->file_glyph;
	    }

	    /*
	     * by default, gray out files not matched.  this may be
	     * overridden in the filter func.
	     */
	    if ( current->matched == FILE_LIST_NOT_MATCHED)
		status = FILE_LIST_IGNORE;
	    current->vals.inactive = (current->matched == FILE_LIST_NOT_MATCHED);
	}
	


	/*
	 * call client filter function.  Note:  this function allows
	 * the app to mung up the row-vals structre as bad as it wants.
	 * from the vantage point of a Scrolling List subclass, this is
	 * reasonable.
	 *
	 * Note that the extension_data field is used by us (an extension),
	 * and that the return status may effect the inactive field.
	 */
	if ( private->client_filter ) {
	    int notify_client = FALSE;

	    if ( S_ISDIR(current->stats.st_mode) ) {
		if ( ((private->filter_mask & FL_MATCHED_DIRS_MASK) 
		      && current->matched == FILE_LIST_MATCHED
		      )
		    || ((private->filter_mask & FL_NOT_MATCHED_DIRS_MASK) 
			&& current->matched == FILE_LIST_NOT_MATCHED
			)
		    )
		    notify_client = TRUE;
	    } 
	    else
		/* regular file */
		if ( ((private->filter_mask & FL_MATCHED_FILES_MASK) 
		      && current->matched == FILE_LIST_MATCHED 
		      )
		    || ((private->filter_mask & FL_NOT_MATCHED_FILES_MASK) 
			&& current->matched == FILE_LIST_NOT_MATCHED
			)
		    )
		    notify_client = TRUE;

	    if ( notify_client )
#ifdef OW_I18N
		if ( private->f.wchar_notify ) {
		    wchar_t *wpath = _xv_mbstowcsdup( file_path );
		    status = (* private->client_filter)(wpath, current);
		    xv_free( wpath );
		} else
#endif
		status = (* private->client_filter)(file_path, current);
	}



	/*
	 * Do after notification to make sure our
	 * private-data field is coherent.
	 */
	current->vals.extension_data 
	    = (S_ISDIR(current->stats.st_mode))
		? FILE_LIST_DIR_TYPE : FILE_LIST_FILE_TYPE;


	/*
	 * Default behavior:  all entries should be displayed
	 * with the non-relevant files "grayed out" as opposed to
	 * being not present.  OL has found that this fares better
	 * in user testing.
	 */
	if ( status == FILE_LIST_IGNORE )
	    if ( !private->f.abbrev_view )
		current->vals.inactive = TRUE;
	    else
		continue;

	++row_num;
    } /* while() */



    /*
     * Sort the entries.  Start at rows[1] to make sure that the ".."
     * entry remains at the top of the list.
     *
     * I18N Note:  the File_list_row and File_list_row_wcs structs are
     * of the same size, so the difference is ignored here.  If they have
     * turned on FILE_LIST_WCHAR_NOTIFY, then the strings will be filled
     * in as wchar_t *.
     */
    if ( private->compare_func )
	qsort(&rows[1], row_num-1, sizeof(File_list_row), private->compare_func);


    /*
     * Populate the scrolling list with the new rows.
     */
    flist_update_list(private, rows, row_num);



    /*
     * Notify client that we are done loading directory.
     * too late to worry about the return value.
     */
    if ( private->cd_func ) {
	(void) xv_stat( private->directory, &stats );
#ifdef OW_I18N
	if ( private->f.wchar_notify ) {
	    (void) (* private->cd_func)(public, 
					dir_wcs,
					&stats,
					FILE_LIST_AFTER_CD 
					);
	    xv_free( dir_wcs );
	} else
#endif
	(void) (* private->cd_func)(public, 
				    private->directory,
				    &stats,
				    FILE_LIST_AFTER_CD 
				    );
    }



    /*
     * Jump back to working directory to avoid side effects
     */
    if ( cwd ) {
	chdir( cwd );
	xv_free( cwd );
    }


    /* de-busify the frame */
    if ( private->f.use_frame )
	xv_set(private->frame, FRAME_BUSY, FALSE, NULL);

    return TRUE;
} /* flist_load_dir() */




/*
 * Make sure we have enough space for the new row.
 * also, guarantee that it's zero'd out.
 */
static File_list_row *
flist_next_row( rows, row_num )
     File_list_row **rows;
     int row_num;
{
    static int num_allocs = 4;
    static Panel_list_row_values empty = { 0 };


    /*
     * Increase size of Row Array by ALLOC_INCR rows.  Note that
     * these are treated as cache and never free'd.  Only the strings
     * allocated for the vals.string field are free'd after being
     * inserted.  Pre-allocate 4 increments for cache
     */
    if ( !*rows )
	*rows = xv_malloc(ALLOC_INCR * 4 * sizeof(File_list_row));
    else if ( row_num == (num_allocs * ALLOC_INCR) ) {
	num_allocs++;
	*rows = xv_realloc(*rows, num_allocs * ALLOC_INCR * sizeof(File_list_row));
    }

/*    XV_BZERO(&(* rows)[row_num], sizeof(File_list_row)); */
    (* rows)[row_num].vals = empty; /* all that's necessary */

    return &(* rows)[row_num];
} /* flist_next_row() */






/*
 * Take the row list and stick it into the list.
 */
static void
flist_update_list( private, rows, num_rows )
     File_list_private *private;
     File_list_row rows[];
     int num_rows;
{
    File_list public = FILE_LIST_PUBLIC(private);
    int rows_in_list;
    int ii;
    Attr_attribute attrs[5];

    /* SLIME:  pull set func directly out of panel list pkg structure */
    Xv_opaque (* plist_set)() = xv_panel_list_pkg.set;

    /* more than enough for /bin */
#define FLIST_INSERT_ROWS	512
    Panel_list_row_values row_vals[FLIST_INSERT_ROWS];
    int start_at = 0;
    int num = 0;


    /*
     * Copy data into a contiguous array of Panel_list_row_value
     * sturctures to optimize insertion speed.  Make avlist and
     * call panel list set function directly.   This IS slimey,
     * but it avoids some overhead and works because we *know*
     * that the only attributes here are translated only by the
     * PANEL_LIST.
     */
    for ( ii=0; ii < num_rows; ii++ ) {
	row_vals[num] = rows[ii].vals;
	num++;

	if ( (num == FLIST_INSERT_ROWS) || (ii == num_rows - 1) ) {
#ifdef OW_I18N
	    attrs[0] = (private->f.wchar_notify) 
		? PANEL_LIST_ROW_VALUES_WCS 
		: PANEL_LIST_ROW_VALUES;
#else
	    attrs[0] = PANEL_LIST_ROW_VALUES;
#endif
	    attrs[1] = (Attr_attribute) start_at;
	    attrs[2] = (Attr_attribute) row_vals;
	    attrs[3] = (Attr_attribute) num;
	    attrs[4] = XV_ZERO;
	    (void) (* plist_set)(public, (Attr_avlist) attrs);
	    start_at += FLIST_INSERT_ROWS;
	    num = 0;
	}
    }



    /*
     * Cleanup step:
     * Get rid of any unused rows if no internal sorting done.
     * Select first row to normalize scrollbar.
     */
    rows_in_list = (int)xv_get(public, PANEL_LIST_NROWS);
    if ( rows_in_list > num_rows ) {
	attrs[0] = PANEL_LIST_DELETE_ROWS;
	attrs[1] = (Attr_attribute) num_rows;
	attrs[2] = (Attr_attribute) (rows_in_list - num_rows);
	attrs[3] = XV_ZERO;
	(void) (* plist_set)(public, (Attr_avlist) attrs);
    }


    /*
     * FILE_LIST_SHOW_DIR functionality.
     */
    if ( private->f.show_dir ) {
	attrs[0] = PANEL_LIST_TITLE;
	attrs[1] = (Attr_attribute) private->directory;
	attrs[2] = XV_ZERO;
	(void) (* plist_set)(public, (Attr_avlist) attrs);
    }


    /*
     * Free up the strings copied from the dirent struct
     * and strxfrm().
     */
    for(ii=0; ii<num_rows; ++ii) {
	xv_free_ref( rows[ii].vals.string );
	xv_free_ref( rows[ii].xfrm );
    }

} /* flist_update_list */




/****************************************************************************/

#if !defined(__linux) && !defined(__APPLE__)

/*
 * Front end to regexp(3).
 *
 * BUG:  regexp is *not* safe for Multibyte characters!
 * The only pattern matching routines on Solaris 2.x at this time
 * that can deal with multibyte are in libgen, which is *only* a
 * '.a' file.  Thus, XView can't reference it without breaking 
 * binary compatibility, and portability.
 *
 * Once the POSIX.2/XPG interface(s) become available on Solaris,
 * that should be used instead.
 */
#define INIT		register char *sp = instring;
#define GETC()		(*sp++)
#define PEEKC()		(*sp)
#define UNGETC(c)	(--sp)
#define RETURN(c)	return c;
#define ERROR(c)	xv_error(0, ERROR_STRING, XV_MSG("Invalid regular expression!"), \
				 ERROR_PKG, FILE_LIST, NULL)

/*
 * Note:  This junk was put here reguarding bug 1120422, CRT#701
 * It WILL generate compiler warnings!  However, it does make the
 * symbols local, which is what we want.  This was verified with 'nm'.
 * 
 * ifdef'd for SVR4 because it causes the 4.x MIT build to fail, not just print
 * warnings.
 */
#ifdef SVR4
#ifndef SUNOS5
static int	sed, nbra, circf;
static char	*loc1, *loc2, *locs;
static int 	advance();
static char	*compile();
static int 	step();
#endif
#endif /* SVR4 */

#include <regexp.h>

static void
flist_compile_regex( private )
     File_list_private *private;
{
    char compile_buf[MAXPATHLEN+1];
    char *end_ptr;
    size_t num_bytes;

    end_ptr
	= compile( private->regex_pattern,
		  compile_buf, 
		  &(compile_buf[MAXPATHLEN+1]),
		  '\0' );

    num_bytes = (size_t) (end_ptr - compile_buf);

    xv_free_ref( private->regex_compile );
    private->regex_compile = xv_alloc_n(char, num_bytes);
    (void) XV_BCOPY(compile_buf, private->regex_compile, num_bytes);
} 


static int
flist_match_regex( s, private )
     char *s;
     File_list_private *private;
{
    return step(s, private->regex_compile);
}

#elif defined(__APPLE__)

/* __APPLE__ uses regex.h */

static void
flist_compile_regex( private )
     File_list_private *private;
{

  if (private->regex_compile == NULL) {
    private->regex_compile = xv_alloc_n(regex_t, 1);
  }
  private->regcomp_return_val =
    regcomp(private->regex_compile, private->regex_pattern,
	    REG_BASIC | REG_EXTENDED | REG_NOSUB);
} 

static int
flist_match_regex( s, private )
     char *s;
     File_list_private *private;
{
  int iret;
  if (private->regex_compile == NULL || private->regcomp_return_val != 0)
    return 0;
  iret = regexec(private->regex_compile, s, 0, NULL, 0);
  regfree(private->regex_compile);
  if (iret == 0) {
    return 0;
  } else {
    return -1;
  }
}


#else /* __linux */

/* Linux does not have regexp.h or compile()/step(). Use regex.h and
 * re_compile_pattern()/re_match() instead. */

static void
flist_compile_regex( private )
     File_list_private *private;
{

    if (private->regex_compile == NULL) {
        private->regex_compile = xv_alloc_n(regex_t, 1);
        private->regex_compile->translate = NULL;
    }
    if (private->regex_compile->allocated == 0) {
        private->regex_compile->buffer = xv_alloc_n(char, MAXPATHLEN + 1);
        private->regex_compile->allocated = MAXPATHLEN + 1;
    }
    re_compile_pattern(private->regex_pattern, strlen(private->regex_pattern),
			private->regex_compile);
} 

static int
flist_match_regex( s, private )
     char *s;
     File_list_private *private;
{
    if (private->regex_compile == NULL || private->regex_compile->allocated == 0)
        return 0;
    return (re_match(private->regex_compile, s, strlen(s), 0, NULL) != -1);
}

#endif /* __linux */

/****************************************************************************/


/*
 * our PANEL_NOTIFY_PROC to handle double-click for changing
 * directories.
 */
static int
flist_list_notify ( item, entry_name, client_data, op, event, row )
     Panel_item item;
     char *entry_name;
     Xv_opaque client_data;
     Panel_list_op op;
     Event *event;
     int row;
{
    File_list_private *private = FILE_LIST_PRIVATE(item);
    
    switch ( op ) {
	
	/*
	 * Note: this returns if the File List handles the condition
	 * (i.e. when it initiates a change of directory).  this condition
	 * can be caught via the FILE_LIST_CHANGE_DIR_FUNC if necessary.
	 * This way the client only sees the double-click if it applies
	 * to a file that the client may wish to open or save.
	 */
    case PANEL_LIST_OP_DBL_CLICK: {
	char *path = xv_dircat(private->directory, entry_name);
	
	switch ( (int)xv_get(item, PANEL_LIST_EXTENSION_DATA, row) ) {
	case FILE_LIST_DOTDOT_TYPE:
	    if ( go_up_one_directory(private) ) {
		(void) flist_load_dir(private, private->directory);
		(void) panel_paint(item, PANEL_CLEAR);
	    }
	    xv_free( path );
	    return XV_OK;
	    
	case FILE_LIST_DIR_TYPE:
	    if ( go_down_one_directory(private, entry_name) ) {
		(void) flist_load_dir(private, private->directory);
		(void) panel_paint(item, PANEL_CLEAR);
	    }
	    xv_free( path );
	    return XV_OK;
	    
	case FILE_LIST_FILE_TYPE:
	default:
	    xv_free( path );
	    break;
	    
	} /* switch(entry_type) */
	break;
    } /* case PANEL_LIST_OP_DBL_CLICK: */
	
    case PANEL_LIST_OP_SELECT:
    case PANEL_LIST_OP_DESELECT:
    case PANEL_LIST_OP_VALIDATE:
    case PANEL_LIST_OP_DELETE:
	break;
    } /* switch(op) */
    

    /*
     * call client's PANEL_NOTIFY_PROC if he installed one.  Note that this
     * differs from the PANEL_LIST's notify proc in that we also pass in the
     * directory name as well as the selected name.
     */
    if ( private->client_notify )
#ifdef OW_I18N
	if ( private->f.wchar_list_notify ) {
	    wchar_t *wdir = _xv_mbstowcsdup( private->directory );
	    wchar_t *wfile = _xv_mbstowcsdup( entry_name );
	    int status = (* private->client_notify)(item, wdir, wfile, 
						    client_data, op,
						    event, row
						    );
	    xv_free_ref( wdir );
	    xv_free_ref( wfile );
	    return status;
	} else
#endif
	return (* private->client_notify)(item, private->directory, entry_name, 
					  client_data, op, event, row);

    return XV_OK;
} /* flist_list_notify() */




/*
 * determine if we can "cd" to a particualar directory.
 * save a stat call by doing this via opendir then
 * caching this DIR pointer for loading the directory.
 * Note:  opendir will call fstat anyway.
 */
static int
can_change_to_dir ( private, path )
     File_list_private *private;
     char *path;
{
    DIR *dir = opendir(path);

    if ( !dir ) {
	flist_error(private, XV_MSG("Unable to open the folder \"%s\""), path );
	return FALSE;
    }

    if ( private->dir_ptr )
	(void) closedir( private->dir_ptr );
    private->dir_ptr = dir;
    private->f.new_dir = TRUE;

    return TRUE;
} /* can_change_to_dir() */





/*
 * "Backup" string by one directory.  since we are essentailly
 * truncating the path string, we don't have to worry about going
 * up symlinks.
 */
static int
go_up_one_directory( private )
     File_list_private *private;
{
    private->previous_dir = xv_strcpy(private->previous_dir, private->directory);
    xv_dirname( private->directory );
    return can_change_to_dir( private, private->directory );
} /* go_up_one_directory() */



/*
 * Add "name" to directory name.
 */
static int
go_down_one_directory( private, name )
     File_list_private *private;
     char *name;
{
    char *new = xv_dircat(private->directory, name);

    if ( !new ) {
	flist_error(private, XV_MSG("out of memory openining folder"));
	return FALSE;
    }

    if ( !can_change_to_dir( private, new ) ) {
	xv_free( new );
	return FALSE;
    }

    flist_new_dir( private, new );
    return TRUE;
} /* go_down_one_directory() */




/*
 * see if we can make heads or tails out of a string someone
 * passed in.  this will concatinate a relative path to the
 * existing one, and handle "..", but makes no attempt to expand
 * out "../../junk/whatever" type entries.  what do you expect
 * from a scrolling list?
 */
static int
validate_new_directory( private, new )
     File_list_private *private;
     char *new;
{
    int ok = FALSE;

    if ( is_dotdot(new) ) {
	ok = go_up_one_directory( private );
    } else if ( is_relative(new) ) {
	char *path = xv_dircat(private->directory, new);
	if ( can_change_to_dir(private, path) ) {
	    flist_new_dir(private, path);
	    ok = TRUE;
	} else
	    free( path );
    } else if ( is_absolute(new) ) {
	if ( can_change_to_dir(private, new) ) {
	    flist_new_dir(private, xv_strcpy(NULL, new));
	    ok = TRUE;
	} 
    } /* else -- absolute */
	
    return ok;
} /* validate_new_directory() */



/*
 * Replace the current directory string with the new one.
 * keep copy of old string in case cd-func returns XV_ERROR.
 */
static void
flist_new_dir( private, new )
     File_list_private *private;
     char *new;
{
    private->previous_dir = xv_strcpy(private->previous_dir, private->directory);
    xv_free_ref( private->directory );
    private->directory = new;
} /* flist_new_dir() */




/*
 * Built in sort comparisons.
 */
Xv_public int
file_list_no_case_ascend_compare( row1, row2 )
     File_list_row *row1;
     File_list_row *row2;
{
    return xv_strcasecmp(row1->xfrm, row2->xfrm);
}

Xv_public int
file_list_no_case_descend_compare( row1, row2 )
     File_list_row *row1;
     File_list_row *row2;
{
    return xv_strcasecmp(row2->xfrm, row1->xfrm);
}

Xv_public int
file_list_case_ascend_compare( row1, row2 )
     File_list_row *row1;
     File_list_row *row2;
{
    return strcmp(row1->xfrm, row2->xfrm);
}

Xv_public int
file_list_case_descend_compare( row1, row2 )
     File_list_row *row1;
     File_list_row *row2;
{
    return strcmp(row2->xfrm, row1->xfrm);
}
