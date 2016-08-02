#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)txt_e_menu.c 20.50 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Allow a user definable "Extras" menu in the textsw.  The search path to
 * find the name of the file to initially look in is:
 *
 *  1. text.extrasMenuFilename{.<locale>} (Xdefaults)
 *  2. $(EXTRASMENU){.<locale>} (environment variable),
 *  3. $(HOME)/.text_extras_menu{.<locale>} (home dir),
 *  4. locale sensitive system default
 *		("$OPENWINHOME/lib/locale/<locale>/XView/.text_extras_menu")
 *  4. fall back to SunView1 ("/usr/lib/.text_extras_menu")
 *
 * Always try locale specific name first, if not there, try without
 * locale name.  In the #4, we will fall back to the "C" locale.
 * Much of this code was borrowed from the suntools dynamic rootmenu code.
 */

#include <sys/file.h>
#include <xview_private/primal.h>
#include <xview_private/txt_impl.h>
#include <errno.h>
#include <ctype.h>
#include <xview/openmenu.h>
#include <xview/defaults.h>
#include <xview/wmgr.h>
#include <xview/icon.h>
#include <xview/icon_load.h>
#include <sys/stat.h>
#include <string.h>
#include <xview/str_utils.h>
#include <unistd.h>

/*
 * Declare errno. Some BSD systems do not have errno declared in
 * <errno.h>
 */
extern int errno;

#define	ERROR	-1

#define	MAX_FILES	40
#define MAX_PATH_LEN	1024
#define	EXTRASMENU	"text_extras_menu"
#define	MAXSTRLEN	256
#define	MAXARGS		20

struct stat_rec {
    char           *name;	/* Dynamically allocated menu file name */
    time_t          mftime;	/* Modified file time */
};

static char    *textsw_savestr(), *textsw_save2str();
extern char    *getenv();
void            expand_path();
static struct stat_rec Extras_stat_array[MAX_FILES];
static int      Textsw_nextfile;

Pkg_private Menu_item	textsw_handle_extras_menuitem();
Pkg_private int 	textsw_build_extras_menu_items();
Pkg_private char      **textsw_string_to_argv();
Pkg_private Textsw_view textsw_from_menu();
static	int      extras_menufile_changed();
static	int      walk_getmenu();
static	int      free_argv();
static	int      Nargs;
static	char	*check_filename_locale();
static int any_shell_meta(register char  *s);

extern int      EXTRASMENU_FILENAME_KEY;

Pkg_private char *
textsw_get_extras_filename(mi)
    Menu_item       mi;
{
    char           *filename;
    char            full_file[MAX_PATH_LEN];
    char	   *locale;
    char	   *result;
    char  	   tmp[MAX_PATH_LEN + 1];


    filename = (char *) xv_get(mi, XV_KEY_DATA, EXTRASMENU_FILENAME_KEY);
    if ((filename != NULL) && ((int) strlen(filename) > 0))
	return filename;
	
    /*
     * FIX_ME: We should get the locale by the XV_LC_DISPLAY_LANG, but
     * there are no easy way to get into the window or server object
     * from here.
     */
    locale = (char *) setlocale(LC_MESSAGES, NULL);


    filename = defaults_get_string("text.extrasMenuFilename",
				   "Text.ExtrasMenuFilename", NULL);
    if (filename && (int)strlen(filename) > 0) {
	expand_path(filename, full_file);
	if ((result = check_filename_locale(locale, full_file, 1)) != NULL)
	    goto found;
    }

    if ((filename = getenv("EXTRASMENU")) != NULL
     && (result = check_filename_locale(locale, filename, 0)) != NULL)
	goto found;

    /*
     * FIX_ME: Using $HOME might not be a very portable way to to find
     * out the home directory when port to the various UNIX flavors.
     */
    if ((filename = getenv("HOME")) != NULL) {
	(void) sprintf(tmp, "%s/.%s", filename, EXTRASMENU);
	if ((result = check_filename_locale(locale, tmp, 1)) != NULL)
		goto found;
    }

#ifdef OPENWINHOME_DEFAULT
    /* martin-2.buck@student.uni-ulm.de */
    if (!(filename = getenv("OPENWINHOME"))) {
	filename = OPENWINHOME_DEFAULT;
    }
#else
    if ((filename = getenv("OPENWINHOME")) != NULL) {
#endif
	(void) sprintf(tmp, "%s/lib/locale/%s/xview/.%s",
		       filename, locale, EXTRASMENU);
	if ((result = check_filename_locale(NULL, tmp, 1)) != NULL)
		goto found;

	if (strcmp(locale, "C") != 0) {
	    (void) sprintf(tmp, "%s/lib/locale/C/xview/.%s",
			   filename, EXTRASMENU);
	    if ((result = check_filename_locale(NULL, tmp, 1)) != NULL)
	            goto found;
	}

/*#ifdef notdef*/
	/* Gee, still?... We will try the old fashioned way */
        (void) sprintf(tmp, "%s/lib/.%s",
		       filename, EXTRASMENU);
	if ((result = check_filename_locale(NULL, tmp, 1)) != NULL)
		goto found;
/*#endif*/
#ifndef OPENWINHOME_DEFAULT
    }
#endif

    /* Giving up, try with ancient way (SunView1) */
    (void) sprintf(tmp, "/usr/lib/.%s", EXTRASMENU);
    result = xv_strsave(tmp);

found:
    (void) xv_set(mi, XV_KEY_DATA, EXTRASMENU_FILENAME_KEY, result, NULL);

    return result;
}


static char *
check_filename_locale(locale, filename, copy)
    char	*locale;
    char	*filename;
    int		copy;
{
    char	tmp[MAX_PATH_LEN + 1];

    if ((int)strlen(filename) <= 0)
	return NULL;

    if (locale != NULL) {
	(void) sprintf(tmp, "%s.%s", filename, locale);
	if (open(tmp, O_RDONLY) != -1) {
		copy = 1;
		filename = tmp;
		goto found;
	}
    }

    if (open(filename, O_RDONLY) != -1)
	goto found;

    return NULL;

found:
    if (copy)
	filename = xv_strsave(filename);

    return filename;
}


static int
extras_menufile_changed()
{
    int             i;
    struct stat     statb;

    /* Whenever an existing menu goes up, stat menu files */
    for (i = 0; i < Textsw_nextfile; i++) {
	if (stat(Extras_stat_array[i].name, &statb) < 0) {
	    if (errno == ENOENT)
		return (TRUE);
	    xv_error(XV_ZERO,
		     ERROR_LAYER, ERROR_SYSTEM,
		     ERROR_STRING, Extras_stat_array[i].name,
		     ERROR_PKG, TEXTSW,
		     NULL);
	    return (ERROR);
	}
	if (statb.st_mtime > Extras_stat_array[i].mftime)
	    return (TRUE);
    }

    return (FALSE);
}

static void
textsw_remove_all_menu_items(menu)
    Menu            menu;
{
    int             n = (int) xv_get(menu, MENU_NITEMS);
    Menu_item       mi;
    int             i;

    if (!menu)
	return;

    for (i = n; i >= 1; i--) {
	mi = xv_get(menu, MENU_NTH_ITEM, i);
	xv_set(menu, MENU_REMOVE_ITEM, mi, NULL);
	xv_destroy(mi);
    }
}

/*
 * Check to see if there is a valid extrasmenu file.  If the file
 * exists then turn on the Extras item.  If the file does not exist and
 * there isn't an Extras menu to display then gray out the Extras
 * item.  Otherwise leave it alone.
 */
Menu_item
textsw_extras_gen_proc(mi, operation)
    Menu_item       mi;
    Menu_generate   operation;
{
    char            full_file[MAX_PATH_LEN];
    struct stat     statb;
    char           *filename;
    int             file_exists;

    if (operation != MENU_DISPLAY)
	return mi;

    filename = textsw_get_extras_filename(mi);

    expand_path(filename, full_file);

    file_exists = (stat(full_file, &statb) >= 0);

    xv_set(mi, MENU_INACTIVE, !file_exists, NULL);

    if (file_exists && extras_menufile_changed()) {
	Menu        menu_handle = xv_get(mi, MENU_PULLRIGHT);
	Textsw_view textsw_view = textsw_from_menu(menu_handle);

	(void) textsw_remove_all_menu_items(menu_handle);
	(void) textsw_build_extras_menu_items(textsw_view, full_file,
					      menu_handle);
    }
    return (mi);
}


Pkg_private int
textsw_build_extras_menu_items(textsw_view, file, menu)
    Textsw_view     textsw_view;
    char           *file;
    Menu            menu;
{
    FILE           *mfd;
    int             lineno = 1;	/* Needed for recursion */
    char            full_file[MAX_PATH_LEN];
    struct stat     statb;

    expand_path(file, full_file);
    if ((mfd = fopen(full_file, "r")) == NULL) {
	char *error_string;

	error_string = malloc(strlen(full_file) + 
		strlen(XV_MSG("extras menu file ")) +
			      2);
	strcpy(error_string, XV_MSG("extras menu file "));
	strcat(error_string, full_file);
	xv_error(XV_ZERO,
		 ERROR_LAYER, ERROR_SYSTEM,
		 ERROR_STRING, error_string,
		 ERROR_PKG, TEXTSW,
		 NULL);
	free(error_string);
	return (ERROR);
    }
    if (Textsw_nextfile >= MAX_FILES - 1) {
	char            dummy[128];
	(void) sprintf(dummy, 
	XV_MSG("textsw: max number of menu files is %ld"),
		       MAX_FILES);
	xv_error(XV_ZERO,
		 ERROR_STRING, dummy,
		 ERROR_PKG, TEXTSW,
		 NULL);

	(void) fclose(mfd);
	return (ERROR);
    }
    if (stat(full_file, &statb) < 0) {
	xv_error(XV_ZERO,
		 ERROR_LAYER, ERROR_SYSTEM,
		 ERROR_STRING, full_file,
		 ERROR_PKG, TEXTSW,
		 NULL);
	(void) fclose(mfd);
	return (ERROR);
    }
    Extras_stat_array[Textsw_nextfile].mftime = statb.st_mtime;
    Extras_stat_array[Textsw_nextfile].name = textsw_savestr(full_file);
    Textsw_nextfile++;

    if (walk_getmenu(textsw_view, menu, full_file, mfd, &lineno) < 0) {
	free(Extras_stat_array[--Textsw_nextfile].name);
	(void) fclose(mfd);
	return (ERROR);
    } else {
	(void) fclose(mfd);
	return (TRUE);
    }
}

#ifndef IL_ERRORMSG_SIZE
#define IL_ERRORMSG_SIZE	256
#endif

static int
walk_getmenu(textsw_view, m, file, mfd, lineno)
    Textsw_view     textsw_view;
    Menu            m;
    char           *file;
    FILE           *mfd;
    int            *lineno;
{
    char            line[256], tag[32], prog[256], args[256];
    register char  *p;
    Menu            nm;
    Menu_item       mi = (Menu_item) 0;
    char           *nqformat, *qformat, *iformat, *format;
    char            err[IL_ERRORMSG_SIZE], icon_file[MAX_PATH_LEN];
    struct pixrect *mpr;

    nqformat = "%[^ \t\n]%*[ \t]%[^ \t\n]%*[ \t]%[^\n]\n";
    qformat = "\"%[^\"]\"%*[ \t]%[^ \t\n]%*[ \t]%[^\n]\n";
    iformat = "<%[^>]>%*[ \t]%[^ \t\n]%*[ \t]%[^\n]\n";

    (void) menu_set(m, MENU_CLIENT_DATA, textsw_view, NULL);

    for (; fgets(line, sizeof(line), mfd); (*lineno)++) {

	if (line[0] == '#') {
	    if (line[1] == '?') {
		char            help[256];

		for (p = line + 2; isspace(*p); p++);

		if (*p != '\0' && sscanf(p, "%[^\n]\n", help) > 0)
		    menu_set((mi != XV_ZERO ? mi : m), XV_HELP_DATA, help, NULL);
	    }
	    continue;
	}
	for (p = line; isspace(*p); p++);

	if (*p == '\0')
	    continue;

	args[0] = '\0';
	format = *p == '"' ? qformat : *p == '<' ? iformat : nqformat;

	if (sscanf(p, format, tag, prog, args) < 2) {
	    char            dummy[128];

	    (void) sprintf(dummy, 
	    XV_MSG("textsw: format error in %s: line %d"),
			   file, *lineno);
	    xv_error(XV_ZERO,
		     ERROR_STRING, dummy,
		     ERROR_PKG, TEXTSW,
		     NULL);
	    return (ERROR);
	}
	if (strcmp(prog, "END") == 0)
	    return (TRUE);

	if (format == iformat) {
	    expand_path(tag, icon_file);
	    if ((mpr = icon_load_mpr(icon_file, err)) == NULL) {
		char *error_string;

		error_string = malloc(strlen(err) +
		    strlen(XV_MSG("textsw: icon file format error: ")) + 2);
		strcpy(error_string, 
		XV_MSG("textsw: icon file format error: "));
		strcat(error_string, err);
		xv_error(XV_ZERO,
			 ERROR_STRING, error_string,
			 ERROR_PKG, TEXTSW,
			 NULL);
		free(error_string);
		return (ERROR);
	    }
	} else
	    mpr = NULL;

	if (strcmp(prog, "MENU") == 0) {
	    nm = menu_create(
			     MENU_NOTIFY_PROC, menu_return_item,
			     XV_HELP_DATA, "textsw:extrasmenu",
			     NULL);

	    if (args[0] == '\0') {
		if (walk_getmenu(textsw_view, nm, file, mfd, lineno) < 0) {
		    menu_destroy(nm);
		    return (ERROR);
		}
	    } else {
		if (textsw_build_extras_menu_items(textsw_view, args, nm) < 0) {
		    menu_destroy(nm);
		    return (ERROR);
		}
	    }
	    if (mpr)
		mi = menu_create_item(MENU_IMAGE, mpr,
				      MENU_PULLRIGHT, nm,
				      MENU_RELEASE,
				      MENU_RELEASE_IMAGE,
				      NULL);
	    else
		mi = menu_create_item(MENU_STRING, textsw_savestr(tag),
				      MENU_PULLRIGHT, nm,
				      MENU_RELEASE,
				      MENU_RELEASE_IMAGE,
				      NULL);
	} else {
	    if (mpr)
		mi = menu_create_item(MENU_IMAGE, mpr,
				      MENU_CLIENT_DATA,
				      textsw_save2str(prog, args),
				      MENU_RELEASE,
				      MENU_RELEASE_IMAGE,
			    MENU_ACTION_PROC, textsw_handle_extras_menuitem,
				      NULL);
	    else
		mi = menu_create_item(MENU_STRING, textsw_savestr(tag),
				      MENU_CLIENT_DATA,
				      textsw_save2str(prog, args),
				      MENU_RELEASE,
				      MENU_RELEASE_IMAGE,
			    MENU_ACTION_PROC, textsw_handle_extras_menuitem,
				      NULL);
	}
	(void) menu_set(m, MENU_APPEND_ITEM, mi, NULL);
    }

    return (TRUE);
}

/* ARGSUSED */
/* static */
Pkg_private	Menu_item
textsw_handle_extras_menuitem(menu, item)
    Menu            menu;
    Menu_item       item;
{
    char           *prog, *args;
    char            command_line[MAX_PATH_LEN];
    char          **filter_argv;
    pkg_private char **textsw_string_to_argv();
    Textsw_view     textsw_view = textsw_from_menu(menu);
    register Textsw_view_handle view;
    register Textsw_folio folio;
    int             again_state;
#ifdef OW_I18N
    CHAR           cmd_line_wcs[MAX_PATH_LEN];
#endif

    if AN_ERROR
	(textsw_view == 0)
	    return XV_ZERO;

    view = VIEW_ABS_TO_REP(textsw_view);
    folio = FOLIO_FOR_VIEW(view);

    prog = (char *) xv_get(item, MENU_CLIENT_DATA);
    args = XV_INDEX(prog, '\0') + 1;

    sprintf(command_line, "%s %s", prog, args);
    filter_argv = textsw_string_to_argv(command_line);

    textsw_flush_caches(view, TFC_STD);
    folio->func_state |= TXTSW_FUNC_FILTER;
    again_state = folio->func_state & TXTSW_FUNC_AGAIN;
#ifdef OW_I18N
    (void) mbstowcs(cmd_line_wcs, command_line, MAX_PATH_LEN);
    textsw_record_extras(folio, cmd_line_wcs);
#else
    textsw_record_extras(folio, command_line);
#endif
    folio->func_state |= TXTSW_FUNC_AGAIN;

    textsw_checkpoint_undo(VIEW_REP_TO_ABS(view),
			   (caddr_t) TEXTSW_INFINITY - 1);

    (void) textsw_call_filter(view, filter_argv);

    textsw_checkpoint_undo(VIEW_REP_TO_ABS(view),
			   (caddr_t) TEXTSW_INFINITY - 1);

    folio->func_state &= ~TXTSW_FUNC_FILTER;
    if (again_state == 0)
	folio->func_state &= ~TXTSW_FUNC_AGAIN;
    free_argv(filter_argv);
    return (item);
}

/*
 * *	textsw_string_to_argv - This function takes a char * that contains * program
 * and it's arguments and returns *			a char ** argument
 * vector for use with execvp *
 * 
 * For example "fmt -65" is turned into * rgv[0] = "fmt" *
 * ] = "-65" * rgv[2] = NULL
 */

Pkg_private char **
textsw_string_to_argv(command_line)
    char           *command_line;
{
    int             i, pos = 0;
    char          **new_argv;
    char           *arg_array[MAXARGS];
    char            scratch[MAXSTRLEN];
    int             use_shell = any_shell_meta(command_line);

    Nargs = 0;

    if (use_shell) {
	/* put in their favorite shell and pass cmd as single string */
	char           *shell;
	extern char    *getenv();

	if ((shell = getenv("SHELL")) == NULL)
	    shell = "/bin/sh";
	new_argv = (char **) malloc((unsigned) 4 * sizeof(char *));
	new_argv[0] = shell;
	new_argv[1] = "-c";
	new_argv[2] = STRDUP(command_line);
	new_argv[3] = '\0';
    } else {
	/* Split command_line into it's individual arguments */
	while (string_get_token(command_line, &pos, scratch, xv_white_space) != NULL)
	    arg_array[Nargs++] = STRDUP(scratch);

	/*
	 * Allocate a new array of appropriate size (Nargs+1 for NULL string)
	 * This is so the caller will know where the array ends
	 */
	new_argv = (char **) malloc(((unsigned) Nargs + 1) *
				    (sizeof(char *)));

	/* Copy the strings from arg_array into it */
	for (i = 0; i < Nargs; i++)
	    new_argv[i] = arg_array[i];
	new_argv[Nargs] = '\0';
    }
    return (new_argv);
}

static int
free_argv(argv)
    char          **argv;
{
    while (Nargs > 0)
	free(argv[--Nargs]);
    free(argv);
}

static char    *
textsw_savestr(s)
    register char  *s;
{
    register char  *p;

    if ((p = malloc((unsigned) (strlen(s) + 1))) == NULL) {
	xv_error(XV_ZERO,
		 ERROR_SEVERITY, ERROR_NON_RECOVERABLE,
		 ERROR_LAYER, ERROR_SYSTEM,
		 ERROR_STRING, XV_MSG("textsw: menu strings"),
		 ERROR_PKG, TEXTSW,
		 NULL);
    }
    (void) strcpy(p, s);
    return (p);
}

static char    *
textsw_save2str(s, t)
    register char  *s, *t;
{
    register char  *p;

    if ((p = malloc((unsigned) (strlen(s) + strlen(t) + 1 + 1))) == NULL) {
	xv_error(XV_ZERO,
		 ERROR_SEVERITY, ERROR_NON_RECOVERABLE,
		 ERROR_LAYER, ERROR_SYSTEM,
		 ERROR_STRING, XV_MSG("textsw: menu strings"),
		 ERROR_PKG, TEXTSW,
		 NULL);
    }
    (void) strcpy(p, s);
    (void) strcpy(XV_INDEX(p, '\0') + 1, t);
    return (p);
}

/*
 * Are there any shell meta-characters in string s?
 */
static int any_shell_meta(register char  *s)
{

    while (*s) {
	if (XV_INDEX("~{[*?$`'\"\\", *s))
	    return (1);
	s++;
    }
    return (0);
}
