#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)xv_parse.c 20.59 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

/*
 * Do standard parse on argv to defaults.
 */

#ifdef _XV_DEBUG
#include <xview_private/xv_debug.h>
#else
#include <stdio.h>
#endif
#include <xview_private/i18n_impl.h>
#include <xview_private/portable.h>
#include <xview/defaults.h>
#include <xview/pkg.h>
#include <xview/xv_error.h>
#include <xview/fullscreen.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#ifdef SVR4 
#include <stdlib.h> 
#endif /* SVR4 */

extern XrmDatabase defaults_rdb;

Xv_private int  list_fonts;

#ifdef _XV_DEBUG
Xv_private int  server_gather_stats;
int             sview_gprof_start;
#endif

typedef struct {
    char           *short_name, *long_name;
    char           *def_name[2];/* defaults name(s) */
    char            num_args;
}               Cmd_line_flag;

typedef struct _cmd_line_entry{
    char			*resource_name; /* -xrm, -Wd */
    char			*values[2]; /* one for each possible default name */
    char			*cmdline[3]; /* actual cmdline option passed (3 max) */
    Cmd_line_flag		*cmdline_flag; /* store cmdline info */
    struct _cmd_line_entry	*next;
}		Cmd_line_entry;

/*
 * WARNING: The enumeration Flag_name, the array cmd_line_flags, and the
 * procedure xv_usage must all be updated together whenever a new option is
 * added, and old option is deleted, or the order of the existing options is
 * permuted.
 */
typedef enum {
    FLAG_SCALE,
    FLAG_FONT,
    FLAG_X_FONT,
    FLAG_WIDTH,
    FLAG_HEIGHT,
    FLAG_SIZE,
    FLAG_POSITION,
    FLAG_GEOMETRY,
    FLAG_ICON_POSITION,
    FLAG_LABEL,
    FLAG_TITLE,
    FLAG_ENABLE_ICONIC,
    FLAG_DISABLE_ICONIC,
    FLAG_FOREGROUND_COLOR,
    FLAG_X_FOREGROUND_COLOR,
    FLAG_BACKGROUND_COLOR,
    FLAG_X_BACKGROUND_COLOR,
    FLAG_SET_DEFAULT_COLOR,
    FLAG_ENABLE_REVERSE,
    FLAG_DISABLE_REVERSE,
    FLAG_ICON_IMAGE,
    FLAG_ICON_LABEL,
    FLAG_ICON_FONT,
    FLAG_DEFAULTS_ENTRY,
    FLAG_XRM_ENTRY,
    FLAG_HELP,
    FLAG_ENABLE_SYNC,
    FLAG_DISABLE_SYNC,
    FLAG_SERVER,
    FLAG_VISUAL,
    FLAG_DEPTH,
    FLAG_DISABLE_RETAINED,
    FLAG_DISABLE_XIO_ERROR_HANDLER,
    FLAG_FULLSCREEN_DEBUG,
    FLAG_FULLSCREEN_DEBUG_SERVER,
    FLAG_FULLSCREEN_DEBUG_PTR,
    FLAG_FULLSCREEN_DEBUG_KBD,
    FLAG_DISABLE_PASSIVE_GRAB_SELECT,
    FLAG_NO_SECURITY,
    FLAG_NAME,
    FLAG_LC_BASICLOCALE,
    FLAG_LC_DISPLAYLANG,
    FLAG_LC_INPUTLANG,
    FLAG_LC_NUMERIC,
#ifdef OW_I18N
#ifdef FULL_R5
    FLAG_LC_TIMEFORMAT,	
    FLAG_IM_INPUT_STYLE,
    FLAG_IM_STATUS_STYLE
#else /* FULL_R5 */
    FLAG_LC_TIMEFORMAT
#endif /* FULL_R5 */
#else /* OW_I18N */
    FLAG_LC_TIMEFORMAT
#endif /* OW_I18N */
#ifdef _XV_DEBUG
  , FLAG_LIST_FONTS,
    FLAG_DEBUG,
    FLAG_STATS,
    FLAG_GPROF_START
#endif
}               Flag_name;

static Cmd_line_flag *find_cmd_flag();

static Cmd_line_flag cmd_line_flags[] = {
  { "-Wx", "-scale", { "window.scale.cmdline", 0 }, 1},
  { "-Wt", "-font", { "font.name.cmdline", 0 }, 1},
  { "-fn", "", { "font.name.cmdline", 0 }, 1},
  { "-Ww", "-width", { "window.columns", 0 }, 1},
  { "-Wh", "-height", { "window.rows", 0 }, 1},
  { "-Ws", "-size", { "window.width", "window.height" }, 2},
  { "-Wp", "-position", { "window.x", "window.y" }, 2},
  { "-WG", "-geometry", { "window.geometry", 0 }, 1},
  { "-WP", "-icon_position", { "icon.x", "icon.y" }, 2},
  { "-Wl", "-label", { "window.header", 0 }, 1},
  { "", "-title", {    "window.header", 0 }, 1},
  { "-Wi", "-iconic", { "window.iconic", 0 }, 0},
  { "+Wi", "+iconic", { "window.iconic", 0 }, 0},
  { "-Wf", "-foreground_color", { "window.color.foreground", 0 }, 3},
  { "-fg", "-foreground", { "window.color.foreground", 0 }, 1},
  { "-Wb", "-background_color", { "window.color.background", 0 }, 3},
  { "-bg", "-background", { "window.color.background", 0 }, 1},
  { "-Wg", "-set_default_color", { "window.inheritcolor", 0 }, 0},
  { "-rv", "-reverse", { "window.reverseVideo", 0 }, 0},
  { "+rv", "+reverse", { "window.reverseVideo", 0 }, 0},
  { "-WI", "-icon_image", { "icon.pixmap", 0 }, 1},
  { "-WL", "-icon_label", { "icon.footer", 0 }, 1},
  { "-WT", "-icon_font", { "icon.font.name.cmdline", 0 }, 1},
  { "-Wd", "-default", { 0, 0 }, 2},
  { "",    "-xrm", { 0, 0 }, 1},
  { "-WH", "-help", { 0, 0 }, 0},
  { "-sync", "-synchronous", { "window.synchronous", 0 }, 0},
  { "+sync", "+synchronous", { "window.synchronous", 0 }, 0},
  { "-Wr", "-display", { "server.name", 0 }, 1},
  { "", "-visual", { "window.visual", 0 }, 1},
  { "", "-depth", { "window.depth", 0 }, 1},
  { "-Wdr", "-disable_retained", { "window.mono.disableRetained", 0 }, 0},
  { "-Wdxio", "-disable_xio_error_handler", { 0, 0 }, 0},
  { "-Wfsdb", "-fullscreendebug", { 0, 0 }, 0},
  { "-Wfsdbs", "-fullscreendebugserver", { 0, 0 }, 0},
  { "-Wfsdbp", "-fullscreendebugptr", { 0, 0 }, 0},
  { "-Wfsdbk", "-fullscreendebugkbd", { 0, 0 }, 0},
  { "-Wdpgs", "-disable_pass_grab_select", { "window.passiveGrab.select", 0 }, 0},
  { "-WS", "-defeateventsecurity", { 0, 0 }, 0},
  { "-name", "-name", { 0, 0 }, 1},
  { "-lc_basiclocale", "-lc_basiclocale", { "openWindows.basicLocale", 0 }, 1},
  { "-lc_displaylang", "-lc_displaylang", { "openWindows.displayLang", 0 }, 1},
  { "-lc_inputlang", "-lc_inputlang", { "openWindows.inputLang", 0 }, 1},
  { "-lc_numeric", "-lc_numeric", { "openWindows.numericFormat", 0 }, 1},
  { "-lc_timeformat", "-lc_timeformat", { "openWindows.timeFormat", 0 }, 1},
#ifdef OW_I18N
#ifdef FULL_R5
  { "-preedit_style", "-preedit_style", { "openWindows.imPreeditStyle.cmdline", "OpenWindows.ImPreeditStyle.cmdline" }, 1},
  { "-status_style", "-status_style", { "openWindows.imStatusStyle.cmdline", "OpenWindows.ImStatusStyle.cmdline" }, 1},
#endif /* FULL_R5 */
#endif /* OW_I18N */
#ifdef _XV_DEBUG
  { "", "-list_fonts", { 0, 0 }, 0},
  { "", "-Xv_debug", { 0, 0 }, 1},
  { "", "-stats", { 0, 0 }, 0},
  { "", "-gprof_start", { 0, 0 }, 0},
#endif
  { 0, 0, { 0, 0 } , 0 }
};

static Cmd_line_entry		*cmdline_entered_first = NULL;
static Cmd_line_entry		*cmdline_entered_last = NULL;

Xv_private void		xv_merge_cmdline();
Xv_private void		xv_get_cmdline_str();
Xv_private void		xv_get_cmdline_argv();
static void		xv_add_cmdline_entry();
int xv_parse_one(char *app_name, int argc, char **argv);

static Defaults_pairs known_scales[] = {
  { "small", WIN_SCALE_SMALL },
  { "Small", WIN_SCALE_SMALL },
  { "medium", WIN_SCALE_MEDIUM },
  { "Medium", WIN_SCALE_MEDIUM },
  { "large", WIN_SCALE_LARGE },
  { "Large", WIN_SCALE_LARGE },
  { "extra_large", WIN_SCALE_EXTRALARGE },
  { "Extra_Large", WIN_SCALE_EXTRALARGE },
  { NULL, -1 }
};

Xv_private void
xv_cmdline_scrunch(argc_ptr, argv, remove, n)
    int            *argc_ptr;
    char          **argv, **remove;
    int             n;
/*
 * Takes remove to remove+n-1 out of argv, which is assumed to be NULL
 * terminated, although no use is made of that assumption. The original argv
 * is required from the caller to avoid having to scan the list looking for
 * its end.
 */
{
#ifdef _XV_DEBUG
    if (*argc_ptr < n) {
	(void) fprintf(stderr,
		       XV_MSG("xv_cmdline_scrunch: argc (%d) < count (%d)\n"),
		       *argc_ptr, n);
	return;
    }
    if (argv[*argc_ptr]) {
	(void) fprintf(stderr,
		       XV_MSG("xv_cmdline_scrunch: argv[argc(%d)] (%d:%s) not NULL\n"),
		       *argc_ptr, argv[*argc_ptr], argv[*argc_ptr]);
	return;
    }
#endif
    *argc_ptr = *argc_ptr - n;
    XV_BCOPY((char *) (remove + n), (char *) (remove),
             sizeof(*remove) * (*argc_ptr - (remove - argv) + 1));
}

Xv_public int
xv_parse_cmdline(app_name, argc_ptr, argv_base, scrunch)
    char           *app_name;
    int            *argc_ptr;
    char          **argv_base;
    int             scrunch;
/*
 * Parse the command line, looking for sv flags.  Abort if a partial flag is
 * encountered, but just ignore unrecognized flags. If scrunch, remove
 * recognized flags (and their arguments) from the command line (argv) and
 * adjust the command count (argc_ptr).
 */
{
    register char **argv = argv_base;
    register int    argc = *argc_ptr;
    int             n;

    while (argc > 0) {
	switch ((n = xv_parse_one(app_name, argc, argv))) {
	  case 0:		/* Unrecognized flag: ignore it */
	    argc--;
	    argv++;
	    break;
	  case -1:
#ifdef _XV_DEBUG
	    /* Always print debugging flags, when #define'd. */
	    xv_generic_debug_help(stderr);
#endif
	    return (-1);
	  default:
	    if (scrunch) {
		xv_cmdline_scrunch(argc_ptr, argv_base, argv, n);
	    } else
		argv += n;
	    argc -= n;
	}
    }

    /*
     * Merge cmdline options into defaults database
     * NOTE:
     * defaults_rdb  points to the database hanging off the most
     * recent server created or to nothing, if a server object has
     * not been created yet.
     */
    xv_merge_cmdline(&defaults_rdb);

    return (0);
}

int
xv_parse_one(char *app_name, int argc, char **argv)
{
    int             plus;
    int             bad_arg = 0;
    register Cmd_line_flag *slot;
    Flag_name       flag_name;
    char	int_val1[12];
    char	int_val2[12];

    if (argc < 1 || ((**argv != '-') && (**argv != '+')))
	return (0);

    slot = find_cmd_flag(argv[0]);

    if (!slot)
	return 0;

    if (argc <= slot->num_args) {
	char            dummy[128];

	(void) sprintf(dummy, 
			XV_MSG("%s: missing argument after %s"), 
			app_name,
		       argv[0]);
	xv_error(XV_NULL,
		 ERROR_STRING, dummy,
		 NULL);

	return (-1);
    }
    flag_name = (Flag_name) (slot - cmd_line_flags);
    switch (flag_name) {
      case FLAG_WIDTH:
      case FLAG_HEIGHT:
	if ((plus = atoi(argv[1])) < 0) {
	    bad_arg = 1;
	    goto NegArg;
	}
	sprintf(int_val1, "%d", plus);
        xv_add_cmdline_entry(slot, NULL, int_val1, NULL, argv);
	break;

      case FLAG_SIZE:
	if ((plus = atoi(argv[1])) < 0) {
	    bad_arg = 1;
	    goto NegArg;
	}
	sprintf(int_val1, "%d", plus);

	if ((plus = atoi(argv[2])) < 0) {
	    bad_arg = 2;
	    goto NegArg;
	}
	sprintf(int_val2, "%d", plus);

        xv_add_cmdline_entry(slot, NULL, int_val1, int_val2, argv);
	break;

      case FLAG_POSITION:
      case FLAG_ICON_POSITION:
	sprintf(int_val1, "%d", atoi(argv[1]));
	sprintf(int_val2, "%d", atoi(argv[2]));

        xv_add_cmdline_entry(slot, NULL, int_val1, int_val2, argv);
	break;

      case FLAG_GEOMETRY:
        xv_add_cmdline_entry(slot, NULL, argv[1], NULL, argv);
	break;
	
      case FLAG_LABEL:
        xv_add_cmdline_entry(slot, NULL, argv[1], NULL, argv);
	break;

      case FLAG_TITLE:
        xv_add_cmdline_entry(slot, NULL, argv[1], NULL, argv);
	break;
	
      case FLAG_ICON_LABEL:
        xv_add_cmdline_entry(slot, NULL, argv[1], NULL, argv);
	break;

      case FLAG_ICON_IMAGE:
        xv_add_cmdline_entry(slot, NULL, argv[1], NULL, argv);
	break;

      case FLAG_ICON_FONT:
        xv_add_cmdline_entry(slot, NULL, argv[1], NULL, argv);
	break;

      case FLAG_FONT:
	/* this is a hack to allow for Xt -fn default */
      case FLAG_X_FONT:
        xv_add_cmdline_entry(slot, NULL, argv[1], NULL, argv);
	break;
	
      case FLAG_SCALE:
	if (defaults_lookup(argv[1], known_scales) == -1) {
		char dummy[1024];
		
		(void) sprintf(dummy, 
			XV_MSG("%s: unknown scale \"%s\" used with %s option"),
			       app_name, argv[1], argv[0]);
		xv_error(XV_NULL,
		     ERROR_STRING, dummy,
			 NULL);
		return(-1);
	}
	else  {
            xv_add_cmdline_entry(slot, NULL, argv[1], NULL, argv);
	}
	break;

      case FLAG_ENABLE_ICONIC:
        xv_add_cmdline_entry(slot, NULL, "True", NULL, argv);
	break;

      case FLAG_DISABLE_ICONIC:
        xv_add_cmdline_entry(slot, NULL, "False", NULL, argv);
	break;
	
      case FLAG_SET_DEFAULT_COLOR:
	/* this is really just a no op, but we need to consume it
	 * so that old applications won't see it.
	 */
	/* boolean value -- if specified then TRUE */
        xv_add_cmdline_entry(slot, NULL, "True", NULL, argv);
	break;

      case FLAG_ENABLE_REVERSE:
        xv_add_cmdline_entry(slot, NULL, "True", NULL, argv);
	break;

      case FLAG_DISABLE_REVERSE:
        xv_add_cmdline_entry(slot, NULL, "False", NULL, argv);
	break;

      case FLAG_DISABLE_RETAINED:
	/* boolean value -- if specified then TRUE */
	sprintf(int_val1, "%d", TRUE);
        xv_add_cmdline_entry(slot, NULL, int_val1, NULL, argv);
	break;

      case FLAG_DISABLE_XIO_ERROR_HANDLER:
	/* boolean value -- if specified then  */
	(void) XSetIOErrorHandler((int (*) ()) NULL);
	break;

      case FLAG_FULLSCREEN_DEBUG:
	fullscreendebug = 1;
        xv_add_cmdline_entry(slot, NULL, NULL, NULL, argv);
	break;

      case FLAG_FULLSCREEN_DEBUG_SERVER:
	fullscreendebugserver = 1;
        xv_add_cmdline_entry(slot, NULL, NULL, NULL, argv);
	break;

      case FLAG_FULLSCREEN_DEBUG_PTR:
	fullscreendebugptr = 1;
        xv_add_cmdline_entry(slot, NULL, NULL, NULL, argv);
	break;

      case FLAG_FULLSCREEN_DEBUG_KBD:
	fullscreendebugkbd = 1;
        xv_add_cmdline_entry(slot, NULL, NULL, NULL, argv);
	break;

      case FLAG_DISABLE_PASSIVE_GRAB_SELECT:
        xv_add_cmdline_entry(slot, NULL, "False", NULL, argv);
	break;

      case FLAG_FOREGROUND_COLOR:
      case FLAG_BACKGROUND_COLOR:{
	  int             i, rgb[3];
	  char            chars[100];
	  
	  /* convert three ints into one string with three RGB values */
	  for (i = 0; i <= 2; i++) {
	      /* if bad number or neg. then use 0 */
	      if ((sscanf(argv[i + 1], "%d", &(rgb[i])) != 1) ||
		  (rgb[i] < 0))
		rgb[i] = 0;
	  }
	  (void) sprintf(chars, "%d %d %d", rgb[0], rgb[1], rgb[2]);
          xv_add_cmdline_entry(slot, NULL, chars, NULL, argv);
	  break;
      }
	
      case FLAG_X_FOREGROUND_COLOR:
      case FLAG_X_BACKGROUND_COLOR:
        xv_add_cmdline_entry(slot, NULL, argv[1], NULL, argv);
	break;
	
      case FLAG_DEFAULTS_ENTRY:
        xv_add_cmdline_entry(slot, argv[1], argv[2], NULL, argv);
	break;
	
      case FLAG_XRM_ENTRY: {
	      char resource[1000], value[1000];
	      int i = 0, j = 0;

	      /* split the argv in the form of "resource:value"
	       * into two different strings to pass into defaults_set_string
	       */
	      while (argv[1][i] != ':' && argv[1][i] != '\0') {
		      resource[i] = argv[1][i];
		      i++;
	      }
	      resource[i] = '\0';
	      if (argv[1][i] == ':') {
		      while (argv[1][i] != '\0') {
			      i++;
			      value[j++] = argv[1][i];
		      }
		      value[j] = '\0';
                      xv_add_cmdline_entry(slot, resource, value, NULL, argv);
	      }
      }
	break;

      case FLAG_HELP:
	return (-1);

      case FLAG_ENABLE_SYNC:
        xv_add_cmdline_entry(slot, NULL, "True", NULL, argv);
	break;

      case FLAG_DISABLE_SYNC:
        xv_add_cmdline_entry(slot, NULL, "False", NULL, argv);
	break;
	
      case FLAG_SERVER:
        xv_add_cmdline_entry(slot, NULL, argv[1], NULL, argv);
	break;

      case FLAG_VISUAL:
        xv_add_cmdline_entry(slot, NULL, argv[1], NULL, argv);
	break;
	
      case FLAG_DEPTH:
	sprintf(int_val1, "%d", atoi(argv[1]));
        xv_add_cmdline_entry(slot, NULL, int_val1, NULL, argv);
	break;

      case FLAG_NO_SECURITY:
	defeat_event_security = 1;
        xv_add_cmdline_entry(slot, NULL, NULL, NULL, argv);
	break;

      case FLAG_NAME:
        xv_add_cmdline_entry(slot, NULL, NULL, NULL, argv);
	break;

      case FLAG_LC_BASICLOCALE:
      case FLAG_LC_DISPLAYLANG:
      case FLAG_LC_INPUTLANG:
      case FLAG_LC_NUMERIC:
      case FLAG_LC_TIMEFORMAT:
#ifdef OW_I18N
#ifdef FULL_R5
      case FLAG_IM_INPUT_STYLE:
      case FLAG_IM_STATUS_STYLE:
#endif /* FULL_R5 */
#endif /* OW_I18N */
        xv_add_cmdline_entry(slot, NULL, argv[1], NULL, argv);
	break;

#ifdef _XV_DEBUG
      case FLAG_LIST_FONTS:
	list_fonts = TRUE;
	break;

      case FLAG_DEBUG:
	plus = atoi(argv[1]);
SetDebug:
	if (xv_set_debug_flag(plus, TRUE)) {
	    char            dummy[128];

	    (void) sprintf(dummy,
			   XV_MSG("xv_set_debug_flag; '%d' is out of bounds"),
			   plus);
	    xv_error(XV_ZERO,
		     ERROR_STRING, dummy,
		     NULL);
	}
	break;
      case FLAG_STATS:
	server_gather_stats = TRUE;
	break;
      case FLAG_GPROF_START:
	sview_gprof_start = TRUE;
	break;
#endif

      default:
	return (0);

    }

    return (slot->num_args + 1);

/* BadFont:
    {
	char            dummy[128];

	(void) sprintf(dummy, 
		XV_MSG("%s: bad font file (%s)"), 
		app_name, argv[1]);
	xv_error(XV_ZERO,
		 ERROR_STRING, dummy,
		 0);
	return (-1);
    } */

NegArg:
    {
	char            dummy[128];

	(void) sprintf(dummy, 
		XV_MSG("%s: can't have negative argument %s after %s"),
		       app_name, argv[bad_arg], argv[0]);
	xv_error(XV_NULL,
		 ERROR_STRING, dummy,
		 NULL);
	return (-1);
    }
	    
/* NoMsgError:
    return (-1); */
} 

static Cmd_line_flag *
find_cmd_flag(key)
    register char  *key;
{
    register Cmd_line_flag *slot = cmd_line_flags;

    for (slot = cmd_line_flags; slot->short_name; slot++)
	if ((strcmp(key, slot->short_name) == 0) ||
	    (strcmp(key, slot->long_name) == 0))
	    return slot;
    return 0;
}

#ifdef _XV_DEBUG
static int
xv_generic_debug_help(fd)
    FILE             *fd;
{
    (void) fprintf(fd, 
	XV_MSG("Generic debugging flags are:\n\
	(-sync)			bool	(run synchronous with server)\n\
	(-list_fonts)\n\
	(-stats)\n\
	(-Xv_debug)		unsigned\n"));
}

#endif

/*
 * xv_merge_cmdline
 * Merges/puts the cmdline options into the passed database
 * It the passed database, does not exist yet (i.e. *db = (nil)),
 * a new one will be created and returned.
 */
Xv_private void
xv_merge_cmdline(db)
XrmDatabase	*db;
{
    Cmd_line_entry	*cur = cmdline_entered_first;
    Cmd_line_flag	*flag_info;

    if (db)  {
        while (cur)  {
	    if (cur->resource_name)  {
		if (cur->values[0])  {
                    XrmPutStringResource(db, cur->resource_name,
					 cur->values[0]);
		}
	    }
	    else  {
		flag_info = cur->cmdline_flag;

		if (flag_info->def_name[0])  {
		    if (cur->values[0])  {
                        XrmPutStringResource(db, flag_info->def_name[0],
					     cur->values[0]);
		    }
		}

		if (flag_info->def_name[1])  {
		    if (cur->values[1])  {
                        XrmPutStringResource(db, flag_info->def_name[1],
					     cur->values[1]);
		    }
		}
	    }

	    cur = cur->next;
        }
    }
}

/*
 * Appends to 'str' the command line flags and options passed when the application
 * was invoked.
 * Important:
 *	window size (-Ws)
 *	window position (-Wp)
 *	icon position (-WP)
 *	disable_iconic (+Wi)
 *	enable iconic (-Wi) 
 * are skipped here because the actual values for these defaults are calculated
 * and added to the string in function get_cmdline_option() (in win/win_input.c).
 */	
Xv_private void
xv_get_cmdline_str(str)
char	*str;
{
    Cmd_line_entry	*cur = cmdline_entered_first;
    Cmd_line_flag	*slot;
    Flag_name		flag_name;
    int			i;
    char		*cmd_flag;

    /*
     * Check if a str was actually passed in 
     */
    if (str)  {
        while (cur)  {
	    /*
	     * Get flag info
	     */
            slot = cur->cmdline_flag;

	    /*
	     * get flag id
	     */
            flag_name = (Flag_name) (slot - cmd_line_flags);

	    /* 
	     * Check if flag is window size/position or icon position
	     */
	    if ((flag_name != FLAG_SIZE) && 
	    	(flag_name != FLAG_POSITION) && 
		(flag_name != FLAG_ICON_POSITION) && 
		(flag_name != FLAG_ENABLE_ICONIC) && 
		(flag_name != FLAG_DISABLE_ICONIC))  {

		/*
		 * The command line option is the short flag.
		 * If it is a null string or is NULL, use the long
		 * flag
		 */
		if (slot->short_name && strlen(slot->short_name))  {
		    cmd_flag = slot->short_name;
		}
		else  {
		    cmd_flag = slot->long_name;
		}

		/*
		 * cat SPACE CMDLINE_FLAG
		 */
		(void)strcat(str, " ");
		(void)strcat(str, cmd_flag);

		/*
		 * Do for each cmdline option
		 */
		for (i = 0; i < slot->num_args; ++i)  {
		    /*
		     *	cat SPACE DOUBLE_QUOTE CMDLINE_OPTION DOUBLE_QUOTE
		     */
		    (void)strcat(str, " \"");
		    (void)strcat(str, cur->cmdline[i]);
		    (void)strcat(str, "\"");
		}
	    }

	    cur = cur->next;
        }
    }
}


/*
 * Appends to 'str' the command line flags and options passed when the application
 * was invoked.
 * Important:
 *	window size (-Ws)
 *	window position (-Wp)
 *	icon position (-WP)
 *	disable_iconic (+Wi)
 *	enable iconic (-Wi) 
 * are skipped here because the actual values for these defaults are calculated
 * and added to the string in function get_cmdline_option() (in win/win_input.c).
 */	
Xv_private void
xv_get_cmdline_argv(argv, argc_ptr)
char	**argv;
int	*argc_ptr;
{
    Cmd_line_entry	*cur = cmdline_entered_first;
    Cmd_line_flag	*slot;
    Flag_name		flag_name;
    int			i;
    char		*cmd_flag;

    /*
     * Check if argv was actually passed in 
     */
    if (argv)  {
        while (cur)  {
	    /*
	     * Get flag info
	     */
            slot = cur->cmdline_flag;

	    /*
	     * get flag id
	     */
            flag_name = (Flag_name) (slot - cmd_line_flags);

	    /* 
	     * Check if flag is window size/position or icon position
	     */
	    if ((flag_name != FLAG_SIZE) && 
	    	(flag_name != FLAG_POSITION) && 
		(flag_name != FLAG_ICON_POSITION) && 
		(flag_name != FLAG_ENABLE_ICONIC) && 
		(flag_name != FLAG_DISABLE_ICONIC))  {

		/*
		 * The command line option is the short flag.
		 * If it is a null string or is NULL, use the long
		 * flag
		 */
		if (slot->short_name && strlen(slot->short_name))  {
		    cmd_flag = slot->short_name;
		}
		else  {
		    cmd_flag = slot->long_name;
		}

		/*
		 * Add cmdline option to argv:
		 */

		/*
		 * Add the cmdline flag
		 */
		argv[(*argc_ptr)++] = cmd_flag;


		/*
		 * Add the cmdline values passed
		 */
		for (i=0; i < slot->num_args; ++i)  {
		    argv[(*argc_ptr)++] = cur->cmdline[i];
		}

	    }

	    cur = cur->next;
        }
    }
}

static void
xv_add_cmdline_entry(slot, resource_name, value1, value2, argv)
Cmd_line_flag	*slot;
char		*resource_name;
char		*value1;
char		*value2;
char		**argv;
{
    Cmd_line_entry	*new;
    int			num_args = slot->num_args;
    int			i;

    /*
     * Allocate new node - append to END of list
     * We must maintain the order of cmdline options
     */
    new = (Cmd_line_entry *)xv_alloc(Cmd_line_entry);
    new->cmdline_flag = slot;
    new->next = (Cmd_line_entry *)NULL;

    /* 
     * If first node
     */
    if (!cmdline_entered_last)  {
        /*
         * Make this the first node
         */
        cmdline_entered_first = new;
    }
    else  {
        /*
         * Link current last node with new node
         */
        cmdline_entered_last->next = new;
    }

    /*
     * Make this the last node
     */
    cmdline_entered_last = new;

    /*
     * Initialize every string to NULL
     */
    new->resource_name = new->values[0] = new->values[1] = (char *)NULL;
    for (i = 0; i < num_args; ++i)  {
        new->cmdline[i] = (char *)NULL;
    }

    /*
     * Store strings...
     */
    if (resource_name)  {
        new->resource_name = strdup(resource_name);
    }

    if (value1)  {
        new->values[0] = strdup(value1);
    }

    if (value2)  {
        new->values[1] = strdup(value2);
    }

    /*
     * Store argv command line options, if any
     */
    for (i = 0; i < num_args; ++i)  {
	/*
	 * Use argv[i+1] to skip the cmdline option.
	 * We want to store only the values passed.
	 */
        new->cmdline[i] = strdup(argv[i+1]);
    }

}
