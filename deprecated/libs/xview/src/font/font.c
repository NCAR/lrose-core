#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)font.c 20.119 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <string.h>
#include <sys/types.h>
#include <pixrect/pixrect.h>
#include <pixrect/pixfont.h>
#include <X11/Xlib.h>
#include <xview_private/xv_debug.h>
#include <xview_private/font_impl.h>
#include <xview/generic.h>
#include <xview/notify.h>
#include <xview/server.h>
#include <xview/window.h>
#include <xview_private/i18n_impl.h>
#include <xview_private/portable.h>
#ifdef OW_I18N
#include <euc.h>
#include <sys/param.h>
#endif /*OW_I18N*/

/*
 * Public
 */

extern Pixfont *xv_pf_sys;
extern char    *defaults_get_string();
/* extern char    *strcpy(); */
extern Xv_opaque xv_default_server;
extern Display *xv_default_display;

#ifdef OW_I18N 
extern char     *getenv();
extern char     *xv_app_name;
static char 	**construct_font_set_list();
static char	*get_attr_str_from_opened_names();

#define FONT_FIND_DEFAULT_FAMILY(linfo) \
                (linfo) ? (linfo)->default_family : DEFAULT_FONT_FAMILY
#define FONT_FIND_DEFAULT_SIZE(linfo) \
                (linfo) ? (linfo)->medium_size : DEFAULT_MEDIUM_FONT_SIZE

Pkg_private  XFontSet	xv_load_font_set();
#define IS_C_LOCALE(_locale) \
	(strcmp(_locale, "C") == 0)

#endif /*OW_I18N*/ 

/*
 * Private
 */
Xv_private void xv_x_char_info();
Xv_private Xv_Font xv_find_olglyph_font();
Pkg_private XID xv_load_x_font();
Pkg_private void xv_unload_x_font();
Xv_private char *xv_font_regular();
Xv_private char *xv_font_regular_cmdline();
Xv_private char *xv_font_scale_cmdline();
Xv_private char *xv_font_scale();

/*
 * delimiters
 */
#define		PERIOD		'.'
#define		DASH		'-'

/*
 * Definitions for decrypting xlfd names
 */
#define		NUMXLFDFIELDS	14
#define		FOUNDRYPOS	1
#define		FAMILYPOS	2
#define		WEIGHTPOS	3
#define		SLANTPOS	4
#define		SETWIDTHNAMEPOS	5
#define		ADDSTYLENAMEPOS	6
#define		PIXSIZEPOS	7
#define		PTSIZEPOS	8
#define		XRESOLUTIONPOS	9

#define DEFAULT_SMALL_FONT		"-b&h-lucida-medium-r-normal-sans-*-100-*-*-*-*-*-*"
#define DEFAULT_SMALL_FONT_SIZE		10
#define DEFAULT_MEDIUM_FONT		"-b&h-lucida-medium-r-normal-sans-*-120-*-*-*-*-*-*"
#define DEFAULT_MEDIUM_FONT_SIZE	12
#define DEFAULT_LARGE_FONT		"-b&h-lucida-medium-r-normal-sans-*-140-*-*-*-*-*-*"
#define DEFAULT_LARGE_FONT_SIZE		14
#define DEFAULT_XLARGE_FONT		"-b&h-lucida-medium-r-normal-sans-*-190-*-*-*-*-*-*"
#define DEFAULT_XLARGE_FONT_SIZE	19

#ifndef _XVIEW_DEFAULT_FONT
#define DEFAULT_DEFAULT_FONT_NAME	"fixed"
#else
#define DEFAULT_DEFAULT_FONT_NAME	"xview-default-font"
#endif				/* _XVIEW_DEFAULT_FONT */

/*
 * Note: changed DEFAULT_FONT_STYLE to normal
 */
#define DEFAULT_FONT_FAMILY		"lucida"
#define DEFAULT_FONT_FIXEDWIDTH_FAMILY	"lucidatypewriter"
#ifdef OW_I18N
#define DEFAULT_FONT_STYLE              "FONT_STYLE_NORMAL"
#else
#define DEFAULT_FONT_STYLE		"normal"
#endif

/*
 * Default font weight, slant
 */
#define DEFAULT_FONT_WEIGHT		"medium"
#define DEFAULT_FONT_SLANT		"r"
#define DEFAULT_FONT_SETWIDTHNAME	"normal"
#define DEFAULT_FONT_ADDSTYLENAME	"sans"
#define DEFAULT_FONT_SIZE		12
#define DEFAULT_FONT_SCALE		(int) WIN_SCALE_MEDIUM
#ifdef OW_I18N
#define DEFAULT_FONT_SCALE_STR          "Medium"
#endif /*OW_I18N*/

static char    *sunview1_prefix = "/usr/lib/fonts/fixedwidthfonts/";

#ifndef OW_I18N
static Font_locale_info         *fs_locales = NULL;
#endif

Xv_private Xv_Font xv_font_with_name();
static void     font_default_font();
static char    *font_default_font_from_scale();
static char    *font_determine_font_name();
static char    *font_rescale_from_font();
static int      font_read_attrs();
static int      font_string_compare();
static int      font_string_compare_nchars();
static char	*font_strip_name();
static int	font_delim_count();
static XID font_try_misc_name();
static Font_locale_info *find_font_locale_info();
static void	font_setup_known_families();
static void	font_init_known_families();
static void	font_init_known_styles();
static void	font_init_sizes();
static void	font_reduce_wildcards();

#ifdef CHECK_VARIABLE_HEIGHT
static void
  font_check_var_height(int *variable_height_font, XFontStruct *x_font_info);
#endif

#ifdef CHECK_OVERLAPPING_CHARS
static void
  font_check_overlapping(int *neg_left_bearing, XFontStruct *x_font_info);
#endif

typedef struct family_foundry {
    char           *family;
    char           *foundry;
}Family_foundry;

typedef struct wildcards{
    char	*foundry;
    char	*family;
    char	*weight;
    char	*slant;
    char	*setwidthname;
    char	*addstylename;
    char	*registry;
    char	*encoding;
}Wildcards;

struct font_return_attrs {
    char	*name;
    char	*orig_name;
    char	*family;
    char	*style;
    char	*foundry;
    char	*weight;
    char	*slant;
    char	*setwidthname;
    char	*addstylename;
    int		size;
    int		small_size;
    int		medium_size;
    int		large_size;
    int		extra_large_size;
    int		scale;
    Font_info	*resize_from_font;
    int		rescale_factor;
    int		free_name, free_family, free_style, 
		free_weight, free_slant, free_foundry, 
		free_setwidthname, free_addstylename;
    char	delim_used;
#ifdef OW_I18N
    int		    type;
    char            *locale;
    char            **names;
    short	    free_names;
    char            *specifier;
#endif /*OW_I18N*/ 
    char	*encoding;
    char	*registry;
    Font_locale_info	*linfo;
    unsigned	no_size:1;
    unsigned	no_style:1;
};
typedef struct font_return_attrs *Font_return_attrs;

/*
 * Known delimeters
 * - characters that appear between fields in a font name
 */
static char	known_delimiters[] = {
    '.',
    '-',
    '\0'
};

/*
 * Family defs - conversion table from a given family name,
 * possibly a sunview name or a semantic name to a name that 
 * can be used in the family field of the xlfd name format.
 */
static Family_defs	default_family_translation[] = {
    /*
     * real entries
     */
    "cmr", "times",
    "cour", "courier",
    "gallant", "lucidatypewriter",
    "serif", "times",
    "lucidasans", "lucida",
    "olglyph", "open look glyph",
    "olcursor", "open look cursor",

    /*
     * Semantic entries
     */
    FONT_FAMILY_DEFAULT, "lucida",
    FONT_FAMILY_DEFAULT_FIXEDWIDTH, "lucidatypewriter",
    FONT_FAMILY_LUCIDA, "lucida",
    FONT_FAMILY_LUCIDA_FIXEDWIDTH, "lucidatypewriter",
    FONT_FAMILY_ROMAN, "times",
    FONT_FAMILY_SERIF, "times",
    FONT_FAMILY_CMR, "times",
    FONT_FAMILY_GALLENT, "times",
    FONT_FAMILY_COUR, "courier",
    FONT_FAMILY_HELVETICA, "helvetica",
    FONT_FAMILY_OLGLYPH, "open look glyph",
    FONT_FAMILY_OLCURSOR, "open look cursor",
#ifdef OW_I18N
    FONT_FAMILY_SANS_SERIF, "lucida",
#endif /*OW_I18N*/
    NULL, NULL
};
#ifdef OW_I18N
#define FONT_NUM_KNOWN_FAMILIES		20
#else
#define FONT_NUM_KNOWN_FAMILIES		19
#endif /*OW_I18N*/

/*
 * Style defs - conversion table from style -> (weight, slant)
 * or vice versa
 */
static Style_defs	default_style_translation[] = {
	/*
	 * Sunview font styles
	 */
	"r", "medium", "r", "normal",
	"b", "bold", "r", "bold",
	"i", "medium", "i", "italic",
	"o", "medium", "o", "oblique",

	/*
	 * 'real' entries
	 */
	"normal", "medium", "r", "normal",
	"bold", "bold", "r", "bold",
	"italic", "medium", "i", "italic",
	"oblique", "medium", "o", "oblique",
	"bolditalic", "bold", "i", "bolditalic",
	"boldoblique", "bold", "o", "boldoblique",
	"demibold", "demibold", "r", "demibold",
	"demiitalic", "demibold", "i", "demiitalic",

	/*
	 * semantic entries
	 */
	DEFAULT_FONT_STYLE, "medium", "r", "normal",
	FONT_STYLE_DEFAULT, "medium", "r", "normal",
	FONT_STYLE_NORMAL, "medium", "r", "normal",
	FONT_STYLE_BOLD, "bold", "r", "bold",
	FONT_STYLE_ITALIC, "medium", "i", "italic",
	FONT_STYLE_OBLIQUE, "medium", "o", "oblique",
	FONT_STYLE_BOLD_ITALIC, "bold", "i", "bolditalic",
	FONT_STYLE_BOLD_OBLIQUE, "bold", "o", "boldoblique",
	0, 0, 0, 0
};
#define FONT_NUM_KNOWN_STYLES		20
#ifdef OW_I18N
static Font_locale_info		C_locale = {
	"C",
	NULL,
	10, 
	12,
	14,
	19,
	default_family_translation,
	default_style_translation,
	FONT_FAMILY_LUCIDA,
	FONT_FAMILY_LUCIDA_FIXEDWIDTH,
	FONT_STYLE_NORMAL,
	"medium",
	"r",
	(int)WIN_SCALE_MEDIUM,
	"Medium",
	12,
	"-*-lucida-medium-r-*-*-*-100-*-*-*-*-*-*",
	"-*-lucida-medium-r-*-*-*-120-*-*-*-*-*-*",
	"-*-lucida-medium-r-*-*-*-140-*-*-*-*-*-*",
	"-*-lucida-medium-r-*-*-*-190-*-*-*-*-*-*"
};
static Font_locale_info		*fs_locales = &C_locale;
#endif

/*
 * Font families that don't have style
 */
static Family_foundry style_less[] = {
    FONT_FAMILY_OLCURSOR, "sun", 
    "open look cursor", "sun",
    FONT_FAMILY_OLGLYPH, "sun", 
    "open look glyph", "sun",
    0, 0
};

/*
 * Font families that don't have size
 */
static Family_foundry size_less[] = {
    FONT_FAMILY_OLCURSOR, "sun", 
    "open look cursor", "sun",
    0, 0
};


/*
 * Special table to get rid of wildcard'd fields for
 * certain font families.
 */
static Wildcards known_wildcards[] = {
    "b&h", "lucida", NULL, NULL, NULL, NULL, NULL, NULL,
    "b&h", "lucidatypewriter", NULL, NULL, NULL, NULL, NULL, NULL,
    "sun", "open look cursor", "", "", "", "", "sunolcursor", "1",
    "sun", "open look glyph", "", "", "", "", "sunolglyph", "1",
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

/* static prototypes */

static void font_init_create_attrs(Font_return_attrs font_attrs);

static void font_setup_known_styles(Font_locale_info *linfo);
static void font_setup_defaults(Font_locale_info *linfo);

#ifdef OW_I18N
static int font_construct_names(Display *display, Font_return_attrs font_attrs);
#endif

static int font_construct_name(Font_return_attrs font_attrs);

static void
  font_free_font_return_attr_strings(struct font_return_attrs *attrs);

/*
 * Normalize font name.
 * - get rid of sunview style file name
 * - make everything low case
 */
static char    *
normalize_font_name(name, linfo)
register char  *name;
Font_locale_info	*linfo;
{
    if (name == NULL) {
	char           *default_scale;

#ifdef OW_I18N
	/*  What locale is this?
	 *  Should I look for font.name.<locale>?
	 */

	 defaults_set_locale(linfo->locale, NULL);

#endif /* OW_I18N */
	/*
	 * NULL for name => use default font name. Warning: Database may have
	 * "" rather than NULL.
	 */
	name = xv_font_regular();

#ifdef OW_I18N
	/*  Undo the locale feature of the defaults
	 *  package
	 */

	 defaults_set_locale(NULL, NULL);

#endif /* OW_I18N */

	if (name == NULL || (strlen(name) == 0)) {
            if (!(default_scale = xv_font_scale()))
                default_scale = linfo->default_scale_str;
	    name = font_default_font_from_scale(default_scale, linfo);
	}
    }
    if (font_string_compare_nchars(name, sunview1_prefix,
				   strlen(sunview1_prefix)) == 0) {
	/* Map SunView1.X font name into server name. */
	name += strlen(sunview1_prefix);	/* index into str */
    }


    return (name);
}

static char    *
font_default_font_from_scale(scale, linfo)
register char  *scale;
Font_locale_info	*linfo;
{
    if (!scale)
	return(linfo->default_medium_font);

    if ((font_string_compare(scale, "small") == 0) ||
	(font_string_compare(scale, "Small") == 0)) {
	return(linfo->default_small_font);
    }else if ((font_string_compare(scale, "medium") == 0) ||
	       (font_string_compare(scale, "Medium") == 0)) {
	return(linfo->default_medium_font);
    }else if ((font_string_compare(scale, "large") == 0) ||
	       (font_string_compare(scale, "Large") == 0)) {
	return(linfo->default_large_font);
    }else if ((font_string_compare(scale, "Extra_large") == 0) ||
	       (font_string_compare(scale, "Extra_Large") == 0) ||
	       (font_string_compare(scale, "extra_Large") == 0) ||
	       (font_string_compare(scale, "extra_large") == 0)) {
	return(linfo->default_xlarge_font);
    }else
	return(linfo->default_medium_font);
}


#ifdef OW_I18N

static char *
skip_space(p)
register char   *p;
{
        while (isspace(*p))
                p++;
        return p;
}

static char *
skip_space_back(p)
register char   *p;
{
        while (isspace(*p))
                p--;
        return p;
}


static char *
parse_font_list(db, list, count)
XrmDatabase             db;
register char           *list;
int                     count;
{
        XrmValue        xrm_result;
        int             len;
        char            *key, *type;
 
        /* Enforce a limit of 15 recursions */
        if (count > 15)
                return NULL;
 
        if (strncmp(list, FS_DEF, FS_DEF_LEN) == 0) {
                if ((key = strchr(list, ',')) != NULL) {
                        key = skip_space(key + 1);
                        return key;
                }
        } else if (strncmp(list, FS_ALIAS, FS_ALIAS_LEN) == 0) {
                xrm_result.size = 0;
                xrm_result.addr = NULL;
                if ((key = strchr(list, ',')) != NULL) {
                        key = skip_space(key + 1);
                        if (XrmGetResource(db, key, key, &type, &xrm_result))
                                return parse_font_list(db,
                                                xrm_result.addr, count++);
                }
        }
        return NULL;
}

/*
 * Given a database handle and a font set specfier, return a comma
 * separated list of fonts if a font set definition is found.
 * If not, return NULL.
 */
static char *
get_font_set_list(db, key)
    XrmDatabase     db;
    char            *key;
{
    XrmValue    xrm_result;
    char        *type;
 
    if ((db == NULL) || (key == NULL))
        return (NULL);
 
    xrm_result.size = 0;
    xrm_result.addr = NULL;
 
    if (XrmGetResource(db, key, key, &type, &xrm_result) == True)
        return(parse_font_list(db, xrm_result.addr, 0));
 
    return(NULL);
}
 
static
free_font_set_list(list)
    char    **list;
{
    register int    i;
 
    for (i = 0; list[i] != NULL; i++)
        free(list[i]);
    free(list);
}

static char **
construct_font_set_list(str)
char    *str;
{
        register char   *p1, *p2;
        register int    i, j;
        char            **list;
        int             count;
 
        if (str == NULL)
                return(NULL);
 
        for (count = 0, p1 = str; *p1 != NULL; count++, p1++)
                if ((p1 = strchr(p1, ',')) == NULL)
                        break;
 
        count += 2 /* one for last part, another one for the NULL */;
        list = (char **) malloc(count * sizeof(char *));
 
        for (i = 0, p2 = p1 = str; (p2 = strchr(p1, ',')) != NULL; i++) {
                p1 = skip_space(p1);
                j = skip_space_back(p2 - 1) - p1 + 1;
                list[i] = (char *) malloc(j + 1);
                strncpy(list[i], p1, j);
                list[i][j] = '\0';
                p1 = skip_space(p2 + 1);
        }
 
        list[i++] = strdup(skip_space(p1));
        list[i] = NULL;
 
        return(list);
}

static char *
find_font_locale(server, avlist)
    Xv_opaque           server;
    Attr_avlist         avlist;
{
    char                 *locale = NULL; 
    register Attr_avlist attrs;
 
    for (attrs = avlist; attrs && *attrs; attrs = attr_next(attrs)) {
        switch ((int) attrs[0]) {
            case FONT_LOCALE:
                locale = (char *) attrs[1];
                break;
 	    case FONT_TYPE:
 	         if (attrs[1] != FONT_TYPE_TEXT)
 	             return("C");
 	         break;                
            default:
                break;
        }
    }
       
    if (locale == NULL)
       locale = (char *)xv_get(server, XV_LC_BASIC_LOCALE);
 
    return(locale);
}

static void
initialize_locale_info(linfo)
    Font_locale_info    *linfo;
{
    XrmValue    xrm_result;
    char        *type;
 
    if (XrmGetResource(linfo->db, FS_SMALL_SIZE, FS_SMALL_SIZE, &type,
                &xrm_result) == True)
        linfo->small_size = atoi(xrm_result.addr);
    else
        linfo->small_size = DEFAULT_SMALL_FONT_SIZE;
 
    if (XrmGetResource(linfo->db, FS_MEDIUM_SIZE, FS_MEDIUM_SIZE, &type,
                &xrm_result) == True)
        linfo->medium_size = atoi(xrm_result.addr);
    else
        linfo->medium_size = DEFAULT_MEDIUM_FONT_SIZE;
 
    if (XrmGetResource(linfo->db, FS_LARGE_SIZE, FS_LARGE_SIZE, &type,
                &xrm_result) == True)
        linfo->large_size = atoi(xrm_result.addr);
    else
        linfo->large_size = DEFAULT_LARGE_FONT_SIZE;
 
    if (XrmGetResource(linfo->db, FS_XLARGE_SIZE, FS_XLARGE_SIZE, &type,
                &xrm_result) == True)
        linfo->xlarge_size = atoi(xrm_result.addr);
    else
        linfo->xlarge_size = DEFAULT_XLARGE_FONT_SIZE;

    if (XrmGetResource(linfo->db, FS_DEFAULT_FAMILY, FS_DEFAULT_FAMILY, &type,
                &xrm_result) == True)
        linfo->default_family = xrm_result.addr;
    else
        linfo->default_family = DEFAULT_FONT_FAMILY;

    /* 
     * The rest of the data in Font_locale_info is static for now ie. it 
     * is not read from an external file. 
     */
    linfo->known_families = default_family_translation;
    linfo->known_styles = default_style_translation;

#ifndef OW_I18N
    linfo->default_family = DEFAULT_FONT_FAMILY;
#endif
    linfo->default_fixedwidth_family = DEFAULT_FONT_FIXEDWIDTH_FAMILY;
    linfo->default_style = DEFAULT_FONT_STYLE;
    linfo->default_weight = DEFAULT_FONT_WEIGHT;
    linfo->default_slant = DEFAULT_FONT_SLANT;
    linfo->default_scale = (int)WIN_SCALE_MEDIUM;
    linfo->default_scale_str = DEFAULT_FONT_SCALE_STR;
#ifdef OW_I18N
    /* Use linfo->medium_size because linfo->default_scale is WIN_SCALE_MEDIUM */
    linfo->default_size = linfo->medium_size;
#else    
    linfo->default_size = DEFAULT_FONT_SIZE;
#endif    

    /*	ASK ISA */
    linfo->default_small_font = "-*-lucida-medium-r-*-*-*-100-*-*-*-*-*-*";
    linfo->default_medium_font = "-*-lucida-medium-r-*-*-*-120-*-*-*-*-*-*";
    linfo->default_large_font = "-*-lucida-medium-r-*-*-*-140-*-*-*-*-*-*";
    linfo->default_xlarge_font = "-*-lucida-medium-r-*-*-*-190-*-*-*-*-*-*";
}

static Font_locale_info *
find_font_locale_info(server, avlist)
    Xv_opaque           server;
    Attr_avlist         avlist;
{
    char                *locale;
    Font_locale_info    *linfo;
    XrmDatabase         db = NULL, app_db;
    char                *str;
    char                filename[MAXPATHLEN];


    if ((locale = find_font_locale(server, avlist)) == NULL)
       return((Font_locale_info *)NULL);

    /* check list to see if font set locale info exists */
    for (linfo = fs_locales; linfo != NULL; linfo = linfo->next)
        if (!strcmp(linfo->locale, locale))
            return(linfo);

    /* create locale specific font set database */
#ifdef OPENWINHOME_DEFAULT
    /* martin-2.buck@student.uni-ulm.de */
    if (!(str = getenv("OPENWINHOME"))) {
	str = OPENWINHOME_DEFAULT;
    }
#else
    if (str = getenv("OPENWINHOME")) {
#endif
            sprintf(filename,
                "%s/lib/locale/%s/OW_FONT_SETS/OpenWindows.fs",
                str, locale);
                db = XrmGetFileDatabase(filename);
#ifndef OPENWINHOME_DEFAULT
    }
#endif

    if (str = (char *)xv_get(server, XV_LOCALE_DIR)) {
        sprintf(filename, "%s/%s/OW_FONT_SETS/%s.fs", str, locale,
            xv_app_name);
        if (db == NULL)
            db = XrmGetFileDatabase(filename);
        else if (app_db = XrmGetFileDatabase(filename))
            XrmMergeDatabases(app_db, &db);
    }

/*
 *  If no OpenWindows.fs file, then assume it is Latin 1.
 *  initialize_locale_info(), will initialize it to C anyway.
 */
    /* add to the font locale info list */
    linfo = (Font_locale_info *)malloc(sizeof(Font_locale_info));
    linfo->locale = strdup(locale);
    linfo->db = db;
    linfo->next = fs_locales;
    fs_locales = linfo;
    initialize_locale_info(linfo);
    
    return(linfo);
 
}
 

#else

/*
 * Initialize everything except the 'locale' field.
 */
static void
initialize_locale_info(linfo)
    Font_locale_info	*linfo;
{
    /*
     * Fill linfo with "C" values, for now
     */
    font_init_sizes(linfo);
    font_setup_known_families(linfo);
    font_setup_known_styles(linfo);
    font_setup_defaults(linfo);
}

/*
 * This routine taken from Sundae implementation
 * In V3 it simply returns C locale information
 */
static Font_locale_info *
find_font_locale_info()
{
    char 	    	*locale = "C";
    Font_locale_info	*linfo;

    /*
     * For now, only have one locale - "C"
     */

    /* check list to see if font set locale info exists */
    for (linfo = fs_locales; linfo != NULL; linfo = linfo->next)
	if (!strcmp(linfo->locale, locale))
	    return(linfo);

    /* 
     * Don't create font_set X resource db
     */
 
    /* add to the font locale info list */
    linfo = (Font_locale_info *)xv_malloc(sizeof(Font_locale_info));
    linfo->locale = strdup(locale);
    linfo->next = fs_locales;
    fs_locales = linfo;
    initialize_locale_info(linfo);

    return(linfo);
}
#endif /*OW_I18N*/


/* Called to free the font list when server is being destroyed. */
/*ARGSUSED*/
static void
font_list_free(server, key, data)
    Xv_object       server;
    Font_attribute  key;
    Xv_opaque       data;
{
    register Font_info *font, *next;
    register int    ref_count;

    ASSERT(key == FONT_HEAD, _svdebug_always_on);
    for (font = (Font_info *) data; font; font = next) {
	next = font->next;	/* Paranoia in case xv_destroy is immediate */
	ref_count = (int) xv_get(FONT_PUBLIC(font), XV_REF_COUNT);
	if (ref_count == 0) {
	    xv_destroy(FONT_PUBLIC(font));
#ifdef _XV_DEBUG
	} else {
	    fprintf(stderr,
		    XV_MSG("Font %s has %d refs but server being destroyed.\n"),
#ifdef OW_I18N
		    font->names[0], ref_count);
#else /* OW_I18N */
		    font->name, ref_count);
#endif /* OW_I18N */		    
#endif
	}
    }
}


#ifdef OW_I18N
/*
 * Initialize font. This routine creates a new font (because xv_create always
 * allocates a new instance) and can not be used to get a handle to an
 * existing font.
 */
Pkg_private int
font_init(parent_public, font_public, avlist)
    Xv_opaque       parent_public;
    Xv_font_struct *font_public;
    Attr_avlist     avlist;
{
    register Font_info	*font;
    int			font_attrs_exist;
    Xv_opaque		server;
    Display		*display;
    XFontStruct		*x_font_info;
    XID			xid;
    int			default_x, default_y, max_char, min_char;
    Font_locale_info	*linfo;
    Font_info		*font_head;
    struct font_return_attrs my_attrs;
    int			error_code;
    XFontSet            font_set = NULL;
    XFontSetExtents	*font_set_extents;


    if (!parent_public) {
	/* xv_create ensures that xv_default_server is valid. */
	parent_public = (Xv_opaque) xv_default_server;
	display = (Display *)xv_get(parent_public, XV_DISPLAY);
	server = (Xv_opaque) xv_default_server;
    } else {
	Xv_pkg         *pkgtype = (Xv_pkg *) xv_get(parent_public, XV_TYPE);
	display = (Display *)xv_get(parent_public, XV_DISPLAY);
	if (!display) {
	    if ((Xv_pkg *) pkgtype == (Xv_pkg *) & xv_font_pkg) {
		Xv_Font         real_parent_public = 0;
		Font_info      *real_parent_private = 0;

		XV_OBJECT_TO_STANDARD(parent_public, "font_init", real_parent_public);
		real_parent_private = FONT_PRIVATE(real_parent_public);
		parent_public = real_parent_private->parent;
		display = (Display *)real_parent_private->display;
	    } else
		display = (Display *)xv_default_display;
	}
	if ((Xv_pkg *) pkgtype == (Xv_pkg *) & xv_server_pkg) {
	    server = (Xv_opaque) parent_public;
	} else {
	    server = (Xv_opaque) XV_SERVER_FROM_WINDOW(parent_public);
	}
    }

    /*
     * Get locale information
     */
    linfo = find_font_locale_info(server, avlist);
    if (!linfo) {
       xv_error(XV_ZERO, 
	  ERROR_STRING, "Unable to find font locale information", 
	  ERROR_PKG, FONT, 
	  NULL);
       return(XV_ERROR);
    }
    

    /*
     * initialization
     */
    my_attrs.linfo = linfo;
    font_init_create_attrs(&my_attrs);

    /*
     * Get the optional creation arguments
     */
    font_attrs_exist = font_read_attrs(&my_attrs, TRUE, avlist);
    if (!font_attrs_exist)
	(void)font_default_font(&my_attrs);

    if (my_attrs.type == FONT_TYPE_TEXT)  {
        error_code = font_construct_names(display, &my_attrs);
    } else  {
        error_code = font_construct_name(&my_attrs);
    }

    if (error_code != XV_OK)  {
        font_free_font_return_attr_strings(&my_attrs);
	return(error_code);
    }

    if (my_attrs.type == FONT_TYPE_TEXT)  {
        /* load the font set */
        font_set = xv_load_font_set(display,  my_attrs.locale,
		    my_attrs.names);
	
	if ((font_set == NULL) && 
	    (my_attrs.specifier || 
	    (my_attrs.name && (my_attrs.specifier != my_attrs.name))) &&
	    (my_attrs.linfo->db == NULL)) {
	/*
	 *  Try it again iff last time specifier is used as is and
	 *  no db was used to expand specifier's alias, or
	 *  my_attrs.name was used as is.
	 *  This time font_construct_names() will try to
	 *  expand specifier or name to xlfd name.
	 */
	    if (my_attrs.free_names) {
        	free_font_set_list(my_attrs.names);
        	my_attrs.names = (char **) NULL;
        	my_attrs.free_names = FALSE;
            }
            error_code = font_construct_names(display, &my_attrs);
            if (error_code != XV_OK)  {
        	font_free_font_return_attr_strings(&my_attrs);
		return(error_code);
    	    }
            font_set = xv_load_font_set(display,  my_attrs.locale,
		    			my_attrs.names);
	}
    
        /* If load failed, print error msg and return error code */
        if (!font_set) {
	    char	dummy[256];
	    char	*message_name;

	    if (my_attrs.specifier)
		    message_name = my_attrs.specifier;
	    else if (my_attrs.orig_name)
		    message_name = my_attrs.orig_name;
	    else if (my_attrs.name)	
		    message_name = my_attrs.name;

	    (void) sprintf(dummy, XV_MSG("Cannot load font set '%s'"),
		    message_name);

	    xv_error(XV_ZERO,
	        ERROR_STRING, dummy,
	        ERROR_PKG, FONT,
	        NULL);

 	    font_free_font_return_attr_strings(&my_attrs);
	    return XV_ERROR;
        }
    } else  {
	char	dummy[128];
        /*
         * Try to load font with (possibly) decrypted name
         */
        xid = xv_load_x_font((Display *) display,
			 my_attrs.name,
			 (Xv_opaque *)&x_font_info,
			 &default_x, &default_y,
			 &max_char, &min_char);
	/*
	 * If load failed, print error msg and return error code
	 */
	if (!xid)  {
	    (void) sprintf(dummy, XV_MSG("Cannot load font '%s'"),
		       (my_attrs.orig_name) ? my_attrs.orig_name 
					: my_attrs.name);
	    xv_error(XV_ZERO,
		 ERROR_STRING, dummy,
		 ERROR_PKG, FONT,
		 NULL);

            font_free_font_return_attr_strings(&my_attrs);

	    return XV_ERROR;
	}
    }

    font = (Font_info *) xv_alloc(Font_info);
    font_public->private_data = (Xv_opaque) font;
    font->public_self = (Xv_opaque) font_public;
    font->parent = parent_public;
 
    font->display = (Xv_opaque)display;
    font->server = server;
    font->pkg = ATTR_PKG_FONT;
    font->pixfont = (char *)NULL;
    font->locale_info = linfo;
    if (my_attrs.type == FONT_TYPE_TEXT)  {
        font->set_id = font_set;
        /* 
         *  font->names is a pointer to the names stored in core of font_set.
         *  If font->names has its own copy, then the cleanup code in font_destroy_struct()
         *  will need to be changed.
         */
        (void) XFontsOfFontSet(font_set, &font->font_structs, &font->names);
        font_set_extents = XExtentsOfFontSet(font_set);
        font->def_char_width = font_set_extents->max_logical_extent.width;
        font->def_char_height = font_set_extents->max_logical_extent.height;
	/*
	 * Using "n" to determining the columns width is solely based
	 * on the historical reason (read compatibility).
	 */
	font->column_width = XmbTextEscapement(font_set, "n", 1);
	
    }
    else  {
        font->xid = xid;
        font->x_font_info = (Xv_opaque)x_font_info;
        font->def_char_width = ((XFontStruct *)x_font_info)->max_bounds.width;
        font->def_char_height = ((XFontStruct *)x_font_info)->ascent 
	                      + ((XFontStruct *)x_font_info)->descent;
	font->column_width = font->def_char_width;
    }
    (void) xv_set((Xv_opaque)font_public, XV_RESET_REF_COUNT, NULL);
    font->type = my_attrs.type;
 
    font->small_size = (int) my_attrs.small_size;
    font->medium_size = (int) my_attrs.medium_size;
    font->large_size = (int) my_attrs.large_size;
    font->extra_large_size = (int) my_attrs.extra_large_size;

    if ((my_attrs.type != FONT_TYPE_TEXT) || (my_attrs.size != FONT_NO_SIZE))
        font->size = (int) my_attrs.size;
    else  {
	short	size_found = FALSE;
	char	*ptsize_str = 
		get_attr_str_from_opened_names(font->names, PTSIZEPOS);
	
	/*
	 * Get point size from the xlfd font names of font set
	 */
	if (ptsize_str && *ptsize_str && (*ptsize_str != DASH) && 
						(*ptsize_str != '*'))  {
	     int	ptsize = atoi(ptsize_str);

	     /*
	      * use point size only if it is positive
	      */
	     if (ptsize >= 0)  {
	         my_attrs.size = ptsize / 10;
		 size_found = TRUE;
	     }
	}

	/*
	 * A size was not obtained from the font names
	 */
	if (!size_found && multibyte)  {
            /*
             * Set up the size as the smaller of the ascent+descent
             * or bounds->max_bounds.ascent + bounds->max_bounds.descent.
             * There is no good way to find out the point size.
	     * 
	     * Do this only for multibyte (Asian) case
	     * In single byte locales, if a font size could not be obtained,
	     * i.e. if the font name was non-XLFD e.g.
	     *		"fixed", "9x15", "8x13", etc.
	     * the font size == max height.
	     * This is not consistent/compatible with previous releases, 
	     * where the font size for "fixed" == FONT_NO_SIZE.
	     *
	     * Removing this size guessing code is risky, as the Asian locales
	     * might depend on it. So, for now, it will only be executed for
	     * multibyte locales.
             */
	     my_attrs.size = font_set_extents->max_ink_extent.height;	
        }

        font->size = (int) my_attrs.size;
    }
 
    if (my_attrs.scale != FONT_NO_SCALE)
        font->scale = (int)my_attrs.scale;
    else {
        if (font->size <= font->small_size)
            font->scale = WIN_SCALE_SMALL;
        else if (font->size <= font->medium_size)
            font->scale = WIN_SCALE_MEDIUM;
        else if (font->size <= font->large_size)
            font->scale = WIN_SCALE_LARGE;
        else
            font->scale = WIN_SCALE_EXTRALARGE;
    }

    if (my_attrs.foundry) {
	font->foundry = (my_attrs.free_foundry) ? my_attrs.foundry : 
	                 xv_strsave(my_attrs.foundry);    
    } else {
        font->foundry = get_attr_str_from_opened_names(font->names, FOUNDRYPOS);
    }
    
    if (my_attrs.family) {
	font->family = (my_attrs.free_family) ? my_attrs.family : 
	                 xv_strsave(my_attrs.family);    
    } else {
        font->family = get_attr_str_from_opened_names(font->names, FAMILYPOS);
    }
    
    /*
     * If style not present, but weight/slant is, try to convert
     * weight/slant to style
     */
    if (!my_attrs.style && my_attrs.weight && my_attrs.slant) {
	font_convert_weightslant(&my_attrs);
    }

    if (my_attrs.style) {
	font->style = (my_attrs.free_style) ? my_attrs.style : 
	                 xv_strsave(my_attrs.style);    
    } 
    
    if (my_attrs.weight) {
	font->weight = (my_attrs.free_weight) ? my_attrs.weight : 
	                 xv_strsave(my_attrs.weight);    
    } else {
        font->weight = get_attr_str_from_opened_names(font->names, WEIGHTPOS);
    }
    
    if (my_attrs.slant) {
	font->slant = (my_attrs.free_slant) ? my_attrs.slant : 
	                 xv_strsave(my_attrs.slant);    
    } else {
        font->slant = get_attr_str_from_opened_names(font->names, SLANTPOS);
    }

    if (my_attrs.setwidthname) {
	font->setwidthname = (my_attrs.free_setwidthname) ? my_attrs.setwidthname : 
	                 xv_strsave(my_attrs.setwidthname);    
    } else {
        font->setwidthname = get_attr_str_from_opened_names(font->names, SETWIDTHNAMEPOS);
    }

    if (my_attrs.addstylename) {
	font->addstylename = (my_attrs.free_addstylename) ? my_attrs.addstylename : 
	                 xv_strsave(my_attrs.addstylename);    
    } else {
        font->addstylename = get_attr_str_from_opened_names(font->names, ADDSTYLENAMEPOS);
    }

    
    if (my_attrs.type == FONT_TYPE_TEXT)  {
        if (my_attrs.specifier)
            font->specifier = strdup(my_attrs.specifier);

        font->name = font->names[0];
    } else  {
        if (my_attrs.name) 
           font->name = (my_attrs.free_name) ? my_attrs.name : 
	                 xv_strsave(my_attrs.name);
    }
        

    if (my_attrs.orig_name)  {
        free(my_attrs.orig_name);
    }
    if (my_attrs.free_names) 
  	free_font_set_list(my_attrs.names);

    /*
     * Add new font to server's list
     */
    if (font_head = (Font_info *) xv_get(server, XV_KEY_DATA, FONT_HEAD)) {
	font->next = font_head->next;
	font_head->next = font;
    } else {
	font->next = (Font_info *) 0;
	(void) xv_set(server,
		      XV_KEY_DATA, FONT_HEAD, font,
		      XV_KEY_DATA_REMOVE_PROC, FONT_HEAD, font_list_free,
		      NULL);
    }

    /*
     * SunView1.X compatibility: set this font as default if appropriate.
     */
    if ((xv_pf_sys == (Pixfont *) 0) &&
	(parent_public == (Xv_opaque) xv_default_server)) {
	if (!my_attrs.name || ((font_string_compare(my_attrs.name,
			normalize_font_name((char *)NULL, linfo)) == 0))) {
	    xv_pf_sys = (Pixfont *)font_public;
	    (void) xv_set((Xv_opaque)font_public, XV_INCREMENT_REF_COUNT, NULL);
	}
    }
    return XV_OK;
}

#else 

/*
 * Initialize font. This routine creates a new font (because xv_create always
 * allocates a new instance) and can not be used to get a handle to an
 * existing font.
 */
Pkg_private int
font_init(parent_public, font_public, avlist)
    Xv_opaque       parent_public;
    Xv_font_struct *font_public;
    Attr_avlist     avlist;
{
    register Font_info	*font;
    int			font_attrs_exist;
    Xv_opaque		server;
    Display		*display;
    XFontStruct		*x_font_info;
    XID			xid;
    int			default_x, default_y, max_char, min_char;
#ifdef CHECK_OVERLAPPING_CHARS
    int			neg_left_bearing;
#endif
#ifdef CHECK_VARIABLE_HEIGHT
    int			variable_height_font;
#endif				/* CHECK_VARIABLE_HEIGHT */
    Font_locale_info	*linfo;
    Font_info		*font_head;
    struct font_return_attrs my_attrs;
    int			error_code;


    if (!parent_public) {
	/* xv_create ensures that xv_default_server is valid. */
	parent_public = (Xv_opaque) xv_default_server;
	display = (Display *)xv_get(parent_public, XV_DISPLAY);
	server = (Xv_opaque) xv_default_server;
    } else {
	Xv_pkg         *pkgtype = (Xv_pkg *) xv_get(parent_public, XV_TYPE);
	display = (Display *)xv_get(parent_public, XV_DISPLAY);
	if (!display) {
	    if ((Xv_pkg *) pkgtype == (Xv_pkg *) & xv_font_pkg) {
		Xv_Font         real_parent_public = 0;
		Font_info      *real_parent_private = 0;

		XV_OBJECT_TO_STANDARD(parent_public, "font_init", real_parent_public);
		real_parent_private = FONT_PRIVATE(real_parent_public);
		parent_public = real_parent_private->parent;
		display = (Display *)real_parent_private->display;
	    } else
		display = (Display *)xv_default_display;
	}
	if ((Xv_pkg *) pkgtype == (Xv_pkg *) & xv_server_pkg) {
	    server = (Xv_opaque) parent_public;
	} else {
	    server = (Xv_opaque) XV_SERVER_FROM_WINDOW(parent_public);
	}
    }

    /*
     * Get locale information
     */
    linfo = find_font_locale_info();

    /*
     * initialization
     */
    my_attrs.linfo = linfo;
    font_init_create_attrs(&my_attrs);

    /*
     * Get the optional creation arguments
     */
    font_attrs_exist = font_read_attrs(&my_attrs, TRUE, avlist);
    if (!font_attrs_exist)
	(void)font_default_font(&my_attrs);

    error_code = font_construct_name(&my_attrs);

    if (error_code != XV_OK)  {
        font_free_font_return_attr_strings(&my_attrs);

	return(error_code);
    }

    /*
     * Try to load font with (possibly) decrypted name
     */
    xid = xv_load_x_font((Display *) display,
			 my_attrs.name,
			 (Xv_opaque *)&x_font_info,
			 &default_x, &default_y,
			 &max_char, &min_char);
    /*
     * If load failed, try to use other names, if possible
     * This piece of code does not fit well into FONT_SETs
     * because the loading of fonts is done by XCreateFontSet()
     */
    if (!xid) {
	char            dummy[128];

	/*
	 * Try to construct xlfd name if it isn't one yet
	 */
        if (font_delim_count(my_attrs.name, DASH) != NUMXLFDFIELDS)  {
            (void)font_determine_font_name(&my_attrs);

            /*
             * Try to load font with (possibly) decrypted name
             */
            xid = xv_load_x_font((Display *) display,
			     my_attrs.name,
			     (Xv_opaque *)&x_font_info,
			     &default_x, &default_y,
			     &max_char, &min_char);
	}


	/*
	 * Keep trying with other names if previous load failed
	 */
	if (!xid)  {
	    /*
	     * Try diferent combinations of family-style-size
	     */
	    xid = font_try_misc_name(&my_attrs, display, &x_font_info, 
			 &default_x, &default_y, &max_char, &min_char);
	}

	/*
	 * If load failed, print error msg and return error code
	 */
	if (!xid)  {
	    (void) sprintf(dummy, XV_MSG("Cannot load font '%s'"),
		       (my_attrs.orig_name) ? my_attrs.orig_name 
					: my_attrs.name);
	    xv_error(XV_ZERO,
		 ERROR_STRING, dummy,
		 ERROR_PKG, FONT,
		 NULL);

            font_free_font_return_attr_strings(&my_attrs);

	    return XV_ERROR;
	}
    }

#ifdef CHECK_VARIABLE_HEIGHT
    font_check_var_height(&variable_height_font, &x_font_info);
#endif

#ifdef CHECK_OVERLAPPING_CHARS
    font_check_overlapping(&neg_left_bearing, &x_font_info);
#endif

    font = (Font_info *) xv_alloc(Font_info);

    /*
     * set forward and back pointers
     */
    font_public->private_data = (Xv_opaque) font;
    font->public_self = (Xv_opaque) font_public;
    font->parent = parent_public;

    font->display = (Xv_opaque)display;
    font->server = server;
    font->pkg = ATTR_PKG_FONT;
    font->xid = xid;
    font->pixfont = (char *)NULL;
    font->locale_info = linfo;
    font->def_char_width = ((XFontStruct *)x_font_info)->max_bounds.width;
    font->def_char_height = ((XFontStruct *)x_font_info)->ascent 
	+ ((XFontStruct *)x_font_info)->descent;
    font->x_font_info = (Xv_opaque)x_font_info;
    (void) xv_set((Xv_opaque)font_public, XV_RESET_REF_COUNT, NULL);
    font->type = FONT_TYPE_TEXT;

#ifdef CHECK_OVERLAPPING_CHARS
    if (neg_left_bearing
#ifdef CHECK_VARIABLE_HEIGHT
	|| variable_height_font
#endif				/* CHECK_VARIABLE_HEIGHT */
	)
	font->overlapping_chars = TRUE;
#endif				/* CHECK_OVERLAPPING_CHARS */

    if (my_attrs.foundry) {
	if (my_attrs.free_foundry) {
	    font->foundry = my_attrs.foundry;	/* take malloc'd ptr */
	} else
	    font->foundry = xv_strsave(my_attrs.foundry);
    }
    if (my_attrs.family) {
	if (my_attrs.free_family) {
	    font->family = my_attrs.family;	/* take malloc'd ptr */
	} else
	    font->family = xv_strsave(my_attrs.family);
    }
    if (my_attrs.style) {
	if (my_attrs.free_style) {
	    font->style = my_attrs.style;	/* take malloc'd ptr */
	} else
	    font->style = xv_strsave(my_attrs.style);
    }
    if (my_attrs.weight) {
	if (my_attrs.free_weight) {
	    font->weight = my_attrs.weight;	/* take malloc'd ptr */
	} else
	    font->weight = xv_strsave(my_attrs.weight);
    }
    if (my_attrs.slant) {
	if (my_attrs.free_slant) {
	    font->slant = my_attrs.slant;	/* take malloc'd ptr */
	} else
	    font->slant = xv_strsave(my_attrs.slant);
    }
    if (my_attrs.setwidthname) {
	if (my_attrs.free_setwidthname) {
	    font->setwidthname = my_attrs.setwidthname;	/* take malloc'd ptr */
	} else
	    font->setwidthname = xv_strsave(my_attrs.setwidthname);
    }
    if (my_attrs.addstylename) {
	if (my_attrs.free_addstylename) {
	    font->addstylename = my_attrs.addstylename;	/* take malloc'd ptr */
	} else
	    font->addstylename = xv_strsave(my_attrs.addstylename);
    }
    if (my_attrs.name) {
	if (my_attrs.free_name) {
	    font->name = my_attrs.name;
	} else
	    font->name = xv_strsave(my_attrs.name);
    }

    font->size = (int) my_attrs.size;
    font->scale = (int) my_attrs.scale;
    font->small_size = (int) my_attrs.small_size;
    font->medium_size = (int) my_attrs.medium_size;
    font->large_size = (int) my_attrs.large_size;
    font->extra_large_size = (int) my_attrs.extra_large_size;

    if (my_attrs.orig_name)  {
        free(my_attrs.orig_name);
    }

    /*
     * Add new font to server's list
     */
    if (font_head = (Font_info *) xv_get(server, XV_KEY_DATA, FONT_HEAD)) {
	font->next = font_head->next;
	font_head->next = font;
    } else {
	font->next = (Font_info *) 0;
	(void) xv_set(server,
		      XV_KEY_DATA, FONT_HEAD, font,
		      XV_KEY_DATA_REMOVE_PROC, FONT_HEAD, font_list_free,
		      NULL);
    }

    /*
     * SunView1.X compatibility: set this font as default if appropriate.
     */
    if ((xv_pf_sys == (Pixfont *) 0) &&
	(parent_public == (Xv_opaque) xv_default_server)) {
	if ((font_string_compare(my_attrs.name,
				 normalize_font_name((char *)NULL, linfo)) == 0)
	/*
	 * || (font_string_compare(my_attrs.name, DEFAULT_DEFAULT_FONT_NAME)
	 * == 0)
	     */ ) {
	    xv_pf_sys = (Pixfont *)font_public;
	    (void) xv_set((Xv_opaque)font_public, XV_INCREMENT_REF_COUNT, NULL);
	}
    }
    return XV_OK;
}
#endif /*OW_I18N*/

#ifdef OW_I18N
char *
get_attr_str_from_opened_names(names, attr_pos)
    char	**names;
    int		attr_pos;
{
    char	*temp_name = NULL;
    char	*result = NULL;
    short	i;
    
    if ((names == NULL)  || (attr_pos > NUMXLFDFIELDS) || (attr_pos < 1)) {
        return(NULL);
    } else {
        for (i= 0; names[i] ; i++) {
            if (font_delim_count(names[i], DASH) == NUMXLFDFIELDS) {
                 temp_name =  names[i];
                 break;
            }
        }
    }
    
    if (temp_name) {
        char     *temp_str = font_strip_name(temp_name, attr_pos, DASH);
        char	 *temp_str2 = NULL;
        char	 dummy[80];
        
        if ((temp_str == NULL) || (strncmp(temp_str, "*", 1) == 0)) {
            return(NULL);
        } 
        
        if (attr_pos < NUMXLFDFIELDS) {
            temp_str2 =  font_strip_name(temp_name, attr_pos + 1, DASH);
            strncpy(dummy, temp_str, (temp_str2 - temp_str) - 1);
            dummy[(temp_str2 - temp_str) - 1] = NULL;
        } else {
            strcpy(dummy, temp_str);
            dummy[strlen(temp_str)] = NULL;
        }
        result = xv_strsave(dummy);
    }
    
    return(result);
    
}
#endif /* OW_I18N */

Pkg_private int
font_destroy_struct(font_public, status)
    Xv_font_struct *font_public;
    Destroy_status  status;
{
    register Font_info *font = FONT_PRIVATE(font_public);
    register Font_info *prev;
    register int    i;
    register struct pixchar
                   *pfc;
    Font_info      *font_head;
    Xv_opaque       display, server = font->server;
    Pixfont        *zap_font_public = 
	(Pixfont *) font->pixfont;

    if (status == DESTROY_CLEANUP) {
	/* PERFORMANCE ALERT: revive list package to consolidate code. */
	/* Remove the font from SunView's server list. */
	font_head = (Font_info *) xv_get(server, XV_KEY_DATA, FONT_HEAD);
	if (!font_head) {
	    server = (Xv_opaque) xv_default_server;
	    if (server)  {
	        font_head = (Font_info *) xv_get(server, XV_KEY_DATA, FONT_HEAD);
	    }
	}
	if (font_head)  {
	    if (((Xv_Font) FONT_PUBLIC(font_head)) == (Xv_Font) font_public) {
	        /* at head of list */
	        (void) xv_set(server, XV_KEY_DATA, FONT_HEAD, font->next, NULL);
	    } else {
	        for (prev = font_head; prev; prev = prev->next) {
		    if (prev->next == font) {
		        prev->next = font->next;
		        break;
		    }
	        }
#ifdef _XV_DEBUG
	        if (prev == 0)
		    abort();
#endif
	    }
	}
	/* Free the storage allocated for glyphs. */
	if (zap_font_public)  {
#ifdef OW_I18N
	    XFontStruct *x_font_info = (XFontStruct *)font->font_structs[0];
#else
	    XFontStruct	*x_font_info = (XFontStruct *)font->x_font_info;
#endif /*OW_I18N*/
            int max_char = MIN(255, x_font_info->max_char_or_byte2);
            int min_char = MIN(255, x_font_info->min_char_or_byte2);

	    for (i = min_char, pfc = &(zap_font_public->pf_char[i]);
	         i <= max_char; i++, pfc++) {
	        if (pfc->pc_pr) {
		    xv_mem_destroy(pfc->pc_pr);
	        }
	    }
	}
	/* free string storage */
#ifdef OW_I18N
        if (font->specifier)
            free(font->specifier);
	if (font->name) {
	/* font->names is a pointer to the names stored in core, so don't free it */
	    if (font->names) {
	       if (font->name != font->names[0])
	           free(font->name);
	    } else {
	       free(font->name);
	    }
	}
#else /*OW_I18N*/           
	if (font->name)
	    free(font->name);
#endif /*OW_I18N*/
	    
	if (font->foundry)
	    free(font->foundry);
	if (font->family)
	    free(font->family);
	if (font->style)
	    free(font->style);
	if (font->weight)
	    free(font->weight);
	if (font->slant)
	    free(font->slant);
	if (font->setwidthname)
	    free(font->setwidthname);
	if (font->addstylename)
	    free(font->addstylename);
	/* Remove the font from X server list, and free our private data. */
	display = font->display;
	if (!display)
	    display = (Xv_opaque) xv_get(xv_default_server, XV_DISPLAY);
#ifdef OW_I18N
	if (font->set_id)
	    XFreeFontSet((Display *) display, font->set_id);
#else
	xv_unload_x_font((Display *) display, font->x_font_info);
#endif /*OW_I18N*/
	free(font);
    }
    return XV_OK;
}

#ifdef OW_I18N

Xv_private      Xv_Font
xv_font_with_name(server, name)
    Xv_opaque       server;
    char           *name;
{
    Xv_Font     font_public = NULL;
    char        *str;
    char		*save_name = NULL;

    defaults_set_locale(NULL, XV_LC_BASIC_LOCALE);

    if (name || (save_name = name = xv_font_regular()))  {
        short	free_name = FALSE;

	/*
	 * If name was returned from defaults pkg, cache it
	 */
	if (save_name)  {
	    save_name = xv_strsave(name);
	    free_name = TRUE;
	}
	else  {
	    save_name = name;
	}

        font_public = (Xv_Font)xv_find(server, FONT,
                                    FONT_SET_SPECIFIER, save_name,
                                    NULL);
	/*
	 * Free cached name
	 */
        if (free_name)  {
	    xv_free(save_name);
        }
    }

    defaults_set_locale(NULL, NULL);

    if (font_public == NULL)
        if (str = xv_font_scale())
            font_public = xv_find(server, FONT,
                                FONT_SCALE, font_scale_from_string(str),
                                NULL);
        else
            font_public = xv_find(server, FONT, NULL);


    if (font_public == NULL)
        xv_error(XV_ZERO,
            ERROR_SEVERITY, ERROR_RECOVERABLE,
            ERROR_STRING,
                XV_MSG("Unable to open default font set \n"),            NULL);

    return(font_public);
}

#else

Xv_private      Xv_Font
xv_font_with_name(server, name)
    Xv_opaque       server;
    char           *name;
{
    Font_locale_info	*linfo;
    char		*locale;
    char		*save_name;
    Xv_Font         font_public;

    locale = (char *)xv_get(server, XV_LC_BASIC_LOCALE);
    if (!locale)  {
	locale = "C";
    }

    linfo = find_font_locale_info();

    save_name = name = normalize_font_name(name, linfo);

    /*
     * Cache string obtained from defaults pkg, as it's
     * contents might change
     */
    if (name)  {
	save_name = xv_strsave(name);
    }

    font_public = xv_find(server, FONT,
				FONT_NAME, save_name,
				NULL);
    if (!font_public)  {
        font_public = xv_find(server, FONT, NULL);
    }

    if (save_name)  {
	xv_free(save_name);
    }

    return (font_public);
}
#endif /*OW_I18N*/

Xv_private Xv_font 
xv_find_olglyph_font(font_public)
Xv_font		font_public;
{
    Xv_font		glyph;
    Font_info		*font;
    Font_locale_info	*linfo;
    int			given_size;
    int			ol_size;

    if (!font_public)  {
	return((Xv_Font)NULL);
    }

    font = FONT_PRIVATE(font_public);
    linfo = font->locale_info;
    given_size = xv_get(font_public, FONT_SIZE);

    if (given_size < 0)  {
	ol_size = linfo->default_size;
    }
    else  {
	if (given_size < linfo->medium_size)  {
	    ol_size = linfo->small_size;
	}
	else  {
	    if (given_size < linfo->large_size)  {
	        ol_size = linfo->medium_size;
	    }
	    else  {
	        if (given_size < linfo->xlarge_size)  {
	            ol_size = linfo->large_size;
	        }
	        else  {
	            ol_size = linfo->xlarge_size;
	        }
	    }
	}
    }

    glyph = (Xv_font)xv_find(font->server, FONT,
			FONT_FAMILY, FONT_FAMILY_OLGLYPH,
			FONT_SIZE, ol_size,
#ifdef OW_I18N
			FONT_TYPE, FONT_TYPE_GLYPH,
#endif /*OW_I18N*/
			NULL);
    return(glyph);
}


Xv_private char *
xv_font_regular()
{
    char        *name;

    if (xv_font_scale_cmdline() && !xv_font_regular_cmdline())
        return( (char *)NULL );

    name = defaults_get_string("font.name.cmdline", 
	"Font.Name.Cmdline", NULL);

    if (name == NULL || !strlen(name))
        name = defaults_get_string("openwindows.regularfont",
            "OpenWindows.RegularFont",  (char *)NULL);

    /* if "RegularFont" isn't specified then read "Name" */
    if (name == NULL || !strlen(name))
         name = defaults_get_string("font.name", "Font.Name", NULL);

    /* if "" then set to NULL */
    if (name && !strlen(name))
        name = (char *)NULL;

    return( name );
}



Xv_private char *
xv_font_monospace()
{
    char        *name;

    if (xv_font_scale_cmdline() && !xv_font_regular_cmdline())
        return( (char *)NULL );

    name = defaults_get_string("font.name.cmdline", 
	"Font.Name.Cmdline", NULL);

    if (name == NULL || !strlen(name))
        name = defaults_get_string( "openwindows.monospacefont",
            "OpenWindows.MonospaceFont",  (char *)NULL);

    /* if "RegularFont" isn't specified then read "Name" */
    if (name == NULL || !strlen(name))
         name = defaults_get_string("font.name", "Font.Name", NULL);

    /* if "" then set to NULL */
    if (name && !strlen(name))
        name = (char *)NULL;

    return( name );
}



Xv_private char *
xv_font_bold()
{
    char        *name;

    if (xv_font_scale_cmdline() && !xv_font_regular_cmdline())
        return( (char *)NULL );

    name = defaults_get_string("font.name.cmdline", 
	"Font.Name.Cmdline", NULL);

    if (name == NULL || !strlen(name))
        name = defaults_get_string( "openwindows.boldfont",
            "OpenWindows.BoldFont",  (char *)NULL);

    /* if "" then set to NULL */
    if (name && !strlen(name))
        name = (char *)NULL;

    return( name );
}


Xv_private char *
xv_font_scale_cmdline()
{
    char        *scale_name;
 
    scale_name = defaults_get_string("window.scale.cmdline", 
	"Window.Scale.Cmdline", NULL);

    /* if "" then set to NULL */
    if (scale_name && !strlen(scale_name))
        scale_name = (char *)NULL;

    return( scale_name );
}



Xv_private char *
xv_font_regular_cmdline()
{
    char        *name;
 
    name = defaults_get_string("font.name.cmdline",
	"Font.Name.Cmdline", NULL);

    /* if "" then set to NULL */
    if (name && !strlen(name))
        name = (char *)NULL;
    return( name );
}



Xv_private char *
xv_font_scale()
{
    char        *scale_name;
 
    scale_name = defaults_get_string("window.scale.cmdline", 
	"Window.Scale.Cmdline", NULL);

    if (scale_name == NULL || !strlen(scale_name))
        scale_name = (char *) defaults_get_string("openwindows.scale",
            "OpenWindows.Scale", NULL );
 
    if (scale_name == NULL || !strlen(scale_name))
        scale_name = (char *) defaults_get_string("window.scale",
            "Window.Scale", NULL );

    /* if "" then set to NULL */
    if (scale_name && !strlen(scale_name))
        scale_name = (char *)NULL;

    return( scale_name );
}


static char    *
font_determine_font_name(my_attrs)
    Font_return_attrs my_attrs;
{
    char	name[512];
    char	sizestr[10];

    /*
     * Return null if no family/style specified
     */
    if ( (my_attrs->family == (char *) NULL)
	&& (my_attrs->style == (char *) NULL) 
	&& (my_attrs->weight == (char *) NULL) 
	&& (my_attrs->slant == (char *) NULL) )
	return (char *) NULL;

    /*
     * If the size fields is used for the font
     */
    if (!my_attrs->no_size)  {
	/*
	 * Use only sizes that are non negative
	 */
        if (my_attrs->size >= 0)  {
            sprintf(sizestr, "%d", 10 * my_attrs->size);
        }
        else  {
	    /*
	     * Otherwise wild card the size
	     */
            sprintf(sizestr, "*");
        }
    }
    else  {
        sprintf(sizestr, "*");
    }

    /*
     * If style field not used, nullify style, weight, and slant
     * fields
     */
    if (my_attrs->no_style)  {
	if (my_attrs->style)  {
	    if (my_attrs->free_style)  {
	        free(my_attrs->style);
		my_attrs->free_style = 0;
	    }
	    my_attrs->style = NULL;
	}

	if (my_attrs->weight)  {
	    if (my_attrs->free_weight)  {
	        free(my_attrs->weight);
		my_attrs->free_weight = 0;
	    }
	    my_attrs->weight = NULL;
	}

	if (my_attrs->slant)  {
	    if (my_attrs->free_slant)  {
	        free(my_attrs->slant);
		my_attrs->free_slant = 0;
	    }
	    my_attrs->slant = NULL;
	}
    }

    /*
     * Fill in wildcard'd fields for certain known families
     */
    font_reduce_wildcards(my_attrs);

    name[0] = '\0';

    sprintf(name, "-%s-%s-%s-%s-%s-%s-*-%s-*-*-*-*-%s-%s",
			(my_attrs->foundry ? my_attrs->foundry:"*"), 
			(my_attrs->family ? my_attrs->family:"*"), 
			(my_attrs->weight ? my_attrs->weight:"*"), 
			(my_attrs->slant ? my_attrs->slant:"*"), 
			(my_attrs->setwidthname ? my_attrs->setwidthname:"*"),
			(my_attrs->addstylename ? my_attrs->addstylename:"*"),
			sizestr,
			(my_attrs->registry ? my_attrs->registry:"*"),
			(my_attrs->encoding ? my_attrs->encoding:"*"));

    /*
     * Save and return the font name constructed
     */
    my_attrs->name = xv_strsave(name);
    my_attrs->free_name = 1;

    return (char *) my_attrs->name;
}

#ifdef OW_I18N

Pkg_private int
font_scale_from_string(str)
    char    *str;
{
    int     scale = WIN_SCALE_MEDIUM;

    if (str != NULL) {
        if (!strcmp(str, "small") || !strcmp(str, "Small"))
            scale = WIN_SCALE_SMALL;
        else if (!strcmp(str, "medium") || !strcmp(str, "Medium"))
            scale = WIN_SCALE_MEDIUM;
        else if (!strcmp(str, "large") || !strcmp(str, "Large"))
            scale = WIN_SCALE_LARGE;
        else if (!strcmp(str, "extra_large") || !strcmp(str, "extra_Large") ||
                !strcmp(str, "Extra_large") || !strcmp(str, "Extra_Large"))
            scale = WIN_SCALE_EXTRALARGE;
    }

    return(scale);
}

#endif /*OW_I18N*/
    

static int
font_size_from_scale(font_attrs, scale)
Font_return_attrs	font_attrs;
int			scale;
{
    Font_locale_info	*linfo;
    int	small, med, large, xlarge;

    if (scale == FONT_NO_SCALE)  {
	return(FONT_NO_SIZE);
    }

    if (!font_attrs)  {
	small = DEFAULT_SMALL_FONT_SIZE;
	med = DEFAULT_MEDIUM_FONT_SIZE;
	large = DEFAULT_LARGE_FONT_SIZE;
	xlarge = DEFAULT_XLARGE_FONT_SIZE;
    }
    else  {
	linfo = font_attrs->linfo;
	small = (font_attrs->small_size < 0) 
		? linfo->small_size 
		: font_attrs->small_size;
	med = (font_attrs->medium_size < 0) 
		? linfo->medium_size 
		: font_attrs->medium_size;
	large = (font_attrs->large_size < 0) 
		? linfo->large_size 
		: font_attrs->large_size;
	xlarge = (font_attrs->extra_large_size < 0) 
		? linfo->xlarge_size 
		: font_attrs->extra_large_size;
    }

    switch (scale) {
      case WIN_SCALE_SMALL:
	return small;
      case WIN_SCALE_MEDIUM:
	return med;
      case WIN_SCALE_LARGE:
	return large;
      case WIN_SCALE_EXTRALARGE:
	return xlarge;
      default:
	return FONT_NO_SIZE;
    }
}

static int
font_scale_from_size(font_attrs, size)
Font_return_attrs	font_attrs;
int			size;
{
    Font_locale_info	*linfo;
    int	med, large, xlarge;

    if (size == FONT_NO_SIZE)  {
	return(FONT_NO_SCALE);
    }

    if (!font_attrs)  {
	med = DEFAULT_MEDIUM_FONT_SIZE;
	large = DEFAULT_LARGE_FONT_SIZE;
	xlarge = DEFAULT_XLARGE_FONT_SIZE;
    }
    else  {
	linfo = font_attrs->linfo;
	med = (font_attrs->medium_size < 0) 
		? linfo->medium_size 
		: font_attrs->medium_size;
	large = (font_attrs->large_size < 0) 
		? linfo->large_size 
		: font_attrs->large_size;
	xlarge = (font_attrs->extra_large_size < 0) 
		? linfo->xlarge_size 
		: font_attrs->extra_large_size;
    }

    if (size < med)  {
	return WIN_SCALE_SMALL;
    }
    
    if (size < large)  {
	return WIN_SCALE_MEDIUM;
    }

    if (size < xlarge)  {
	return WIN_SCALE_LARGE;
    }

    return WIN_SCALE_EXTRALARGE;

}

static int
font_get_default_scale(linfo)
Font_locale_info	*linfo;
{
    char	*scale_name;

    if (!(scale_name = xv_font_scale()))
	scale_name=linfo->default_scale_str;

    if ((font_string_compare(scale_name, "small") == 0) ||
	(font_string_compare(scale_name, "Small") == 0))  {
	return(WIN_SCALE_SMALL);
    }

    if ((font_string_compare(scale_name, "medium") == 0) ||
	(font_string_compare(scale_name, "Medium") == 0))  {
	return(WIN_SCALE_MEDIUM);
    }

    if ((font_string_compare(scale_name, "large") == 0)||
	(font_string_compare(scale_name, "Large") == 0))  {
	return(WIN_SCALE_LARGE);
    }

    if ((font_string_compare(scale_name, "Extra_large") == 0) ||
	(font_string_compare(scale_name, "Extra_Large") == 0) ||
	(font_string_compare(scale_name, "extra_Large") == 0) ||
        (font_string_compare(scale_name, "extra_large") == 0))  {
	return(WIN_SCALE_EXTRALARGE);
    }
    return(FONT_NO_SCALE);
}

#ifdef OW_I18N
static char    *
font_rescale_from_font(font, scale, attrs)
    Font_info      *font;
    int             scale;
    struct font_return_attrs *attrs;
{
    Font_locale_info	*linfo;
    char           *font_name = NULL;
    char            new_name[256], name[512];
    int             desired_scale;
    int             fs = 0;

    if (!font)			/* if possibly not set? */
	return (char *) font_name;
    name[0] = '\0';

    linfo = attrs->linfo;

    if ((scale < (int) WIN_SCALE_SMALL) ||
	(scale > (int) WIN_SCALE_EXTRALARGE) ||
	(scale == FONT_NO_SCALE))  {
	char	dummy[128];

	sprintf(dummy, XV_MSG("Bad scale value:%d"), 
			(int)scale);
	xv_error(XV_ZERO,
	        ERROR_STRING, dummy,
	        ERROR_PKG, FONT,
	        NULL);
	return (char *) font_name;	/* no scaling */
    }


    if (linfo && multibyte && 
	    (font->type == FONT_TYPE_TEXT)) {
	fs = 1;
        sprintf(name, "%s-%s",
        	(font->family ? font->family:"*"),
               	(font->style ? font->style:"*"));
        if (font->family) {
       	    attrs->family = xv_strsave(font->family);
            attrs->free_family = 1;
        }
        if (font->style) {
            attrs->style = xv_strsave(font->style);
            attrs->free_style = 1;
        }
    } else {
	/* munch everything together */
        sprintf(name, "-%s-%s-%s-%s-%s-%s",
			(font->foundry ? font->foundry:"*"), 
			(font->family ? font->family:"*"), 
			(font->weight ? font->weight:"*"), 
			(font->slant ? font->slant:"*"), 
			(font->setwidthname ? font->setwidthname:"*"),
			(font->addstylename ? font->addstylename:"*") );


	if (font->foundry) {
		attrs->foundry = xv_strsave(font->foundry);
		attrs->free_foundry = 1;
	}
	if (font->family) {
		attrs->family = xv_strsave(font->family);
		attrs->free_family = 1;
	}
	if (font->style) {
		attrs->style = xv_strsave(font->style);
		attrs->free_style = 1;
	}
	if (font->weight) {
		attrs->weight = xv_strsave(font->weight);
		attrs->free_weight = 1;
	}
	if (font->slant) {
		attrs->slant = xv_strsave(font->slant);
		attrs->free_slant = 1;
	}
	if (font->setwidthname) {
		attrs->setwidthname = xv_strsave(font->setwidthname);
		attrs->free_setwidthname = 1;
	}
	if (font->addstylename) {
		attrs->addstylename = xv_strsave(font->addstylename);
		attrs->free_addstylename = 1;
	}
    }

    switch (scale) {
      case WIN_SCALE_SMALL:
	desired_scale = font->small_size;
	break;
      case WIN_SCALE_MEDIUM:
	desired_scale = font->medium_size;
	break;
      case WIN_SCALE_LARGE:
	desired_scale = font->large_size;
	break;
      case WIN_SCALE_EXTRALARGE:
	desired_scale = font->extra_large_size;
	break;
      default:
	desired_scale = -1;
    }
    if (desired_scale == -1)
	return (char *) font_name;	/* no font that scale */
    new_name[0] = '\0';

    /*
     * If cannot get a size, give the default
     */
    if ((desired_scale == FONT_NO_SIZE) || (desired_scale <= 0)) {
	desired_scale = linfo->default_size;
    }
	
    if (fs)
        sprintf(new_name, "%s-%d", name, desired_scale);
    else
        sprintf(new_name, "%s-*-%d-*-*-*-*-*-*", name, (10*desired_scale));

    attrs->name = xv_strsave(new_name);
    attrs->free_name = 1;
    attrs->size = desired_scale;
    attrs->scale = scale;
    attrs->small_size = font->small_size;
    attrs->medium_size = font->medium_size;
    attrs->large_size = font->large_size;
    attrs->extra_large_size = font->extra_large_size;

    return (attrs->name);
}

#else

static char    *
font_rescale_from_font(font, scale, attrs)
    Font_info      *font;
    int             scale;
    struct font_return_attrs *attrs;
{
    Font_locale_info	*linfo;
    char           *font_name = NULL;
    char            new_name[256], name[512];
    int             desired_scale;

    if (!font)			/* if possibly not set? */
	return (char *) font_name;
    name[0] = '\0';

    linfo = attrs->linfo;

    if ((scale < (int) WIN_SCALE_SMALL) ||
	(scale > (int) WIN_SCALE_EXTRALARGE) ||
	(scale == FONT_NO_SCALE))  {
	char	dummy[128];

	sprintf(dummy, "Bad scale value:%d", (int)scale);
	xv_error(XV_ZERO,
	        ERROR_STRING, dummy,
	        ERROR_PKG, FONT,
	        NULL);
	return (char *) font_name;	/* no scaling */
    }


	if (font->foundry) {
		attrs->foundry = xv_strsave(font->foundry);
		attrs->free_foundry = 1;
	}
	if (font->family) {
		attrs->family = xv_strsave(font->family);
		attrs->free_family = 1;
	}
	if (font->style) {
		attrs->style = xv_strsave(font->style);
		attrs->free_style = 1;
	}
	if (font->weight) {
		attrs->weight = xv_strsave(font->weight);
		attrs->free_weight = 1;
	}
	if (font->slant) {
		attrs->slant = xv_strsave(font->slant);
		attrs->free_slant = 1;
	}
	if (font->setwidthname) {
		attrs->setwidthname = xv_strsave(font->setwidthname);
		attrs->free_setwidthname = 1;
	}
	if (font->addstylename) {
		attrs->addstylename = xv_strsave(font->addstylename);
		attrs->free_addstylename = 1;
	}

        font_reduce_wildcards(attrs);

	/* munch everything together */
        sprintf(name, "-%s-%s-%s-%s-%s-%s",
			(attrs->foundry ? attrs->foundry:"*"), 
			(attrs->family ? attrs->family:"*"), 
			(attrs->weight ? attrs->weight:"*"), 
			(attrs->slant ? attrs->slant:"*"), 
			(attrs->setwidthname ? attrs->setwidthname:"*"),
			(attrs->addstylename ? attrs->addstylename:"*") );

    switch (scale) {
      case WIN_SCALE_SMALL:
	desired_scale = font->small_size;
	break;
      case WIN_SCALE_MEDIUM:
	desired_scale = font->medium_size;
	break;
      case WIN_SCALE_LARGE:
	desired_scale = font->large_size;
	break;
      case WIN_SCALE_EXTRALARGE:
	desired_scale = font->extra_large_size;
	break;
      default:
	desired_scale = -1;
    }
    if (desired_scale == -1)
	return (char *) font_name;	/* no font that scale */
    new_name[0] = '\0';

    /*
     * If cannot get a size, give the default
     */
    if ((desired_scale == FONT_NO_SIZE) || (desired_scale <= 0)) {
	desired_scale = linfo->default_size;
    }
    
    sprintf(new_name, "%s-*-%d-*-*-*-*-%s-%s", name, (10*desired_scale),
			(attrs->registry ? attrs->registry:"*"),
			(attrs->encoding ? attrs->encoding:"*"));

    attrs->name = xv_strsave(new_name);
    attrs->free_name = 1;
    attrs->size = desired_scale;
    attrs->scale = scale;
    attrs->small_size = font->small_size;
    attrs->medium_size = font->medium_size;
    attrs->large_size = font->large_size;
    attrs->extra_large_size = font->extra_large_size;

    return (attrs->name);
}
#endif /*OW_I18N*/

#ifdef OW_I18N

static int
font_name_is_equivalent(my_attrs, finfo)
    Font_return_attrs 	my_attrs;
    struct font_info	*finfo;
    
{
    char	tempName[260];
    char	*foundry = NULL;
    char	*family = NULL;
    char	*weight = NULL;
    char	*slant = NULL;
    char	*setwidthname = NULL;
    char	*addstylename = NULL;
    char	*pixsize = NULL;
    char	*ptsize = NULL;
    char	*xres = NULL;
    char	*registry = NULL;
    char	*encoding = NULL;

    /* 
     *  This has a big assumption null pointer means wild card.
     *  If this assumption is changed, then this code will not be valid.
     */

    if (!my_attrs || !my_attrs->family || 
	(font_delim_count(finfo->name, DASH) != NUMXLFDFIELDS))  {
	return (FALSE);
    }

    /*
     * Decrypt the XLFD name into it's subparts
     * The different pointers (i.e. foundry, family, etc) will
     * point to different positions in the tempName string.
     * The null padding is inserted as well.
     */
    if (finfo && finfo->name)  {
	sprintf(tempName, "%s", finfo->name);

        foundry = font_strip_name(tempName, FOUNDRYPOS, DASH);
        family = font_strip_name(tempName, FAMILYPOS, DASH);
        weight = font_strip_name(tempName, WEIGHTPOS, DASH);
        slant = font_strip_name(tempName, SLANTPOS, DASH);
        setwidthname = font_strip_name(tempName, SETWIDTHNAMEPOS, DASH);
        addstylename = font_strip_name(tempName, ADDSTYLENAMEPOS, DASH);
        pixsize = font_strip_name(tempName, PIXSIZEPOS, DASH);
        ptsize = font_strip_name(tempName, PTSIZEPOS, DASH);
        xres = font_strip_name(tempName, XRESOLUTIONPOS, DASH);
        registry = font_strip_name(tempName, 13, DASH);
        encoding = font_strip_name(tempName, 14, DASH);

	/*
	 * Null pad strings
	 */
        if (family)  {
            *(family-1) = '\0';
	}
        if (weight)  {
            *(weight-1) = '\0';
	}
        if (slant)  {
            *(slant-1) = '\0';
	}
        if (setwidthname)  {
            *(setwidthname-1) = '\0';
	}
        if (addstylename)  {
            *(addstylename-1) = '\0';
	}
        if (pixsize)  {
            *(pixsize-1) = '\0';
	}
        if (ptsize)  {
            *(ptsize-1) = '\0';
	}
        if (xres)  {
            *(xres-1) = '\0';
	}
        if (registry)  {
            *(registry-1) = '\0';
	}
    }

    if (my_attrs->family)  {
        if (family == NULL) { /* non XLFD name */
            return(FALSE);
        } else if (strncmp(family, "*", 1) != 0) {
            if (strcmp(family, my_attrs->family) != 0) {
                return(FALSE);                              
            }
        }
    }


    if (my_attrs->foundry)  {
        if (foundry == NULL) { /* non XLFD name */
            return(FALSE);
        } else if (strncmp(foundry, "*", 1) != 0) {
            if (strcmp(foundry, my_attrs->foundry) != 0) {
                return(FALSE);                              
            }
        }
    }

    if (my_attrs->weight)  {
        if (weight == NULL) { /* non XLFD name */
            return(FALSE);
        } else if (strncmp(weight, "*", 1) != 0) {
            if (strcmp(weight, my_attrs->weight) != 0) {
                return(FALSE);                              
            }
        }
    }


    if (my_attrs->slant)  {
        if (slant == NULL) { /* non XLFD name */
            return(FALSE);
        } else if (strncmp(slant, "*", 1) != 0) {
            if (strcmp(slant, my_attrs->slant) != 0) {
                return(FALSE);                              
            }
        }
    }

    if (my_attrs->setwidthname)  {
        if (setwidthname == NULL) { /* non XLFD name */
            return(FALSE);
        } else if (strncmp(setwidthname, "*", 1) != 0) {
            if (strcmp(setwidthname, my_attrs->setwidthname) != 0) {
                return(FALSE);                              
            }
        }
    }

    if (my_attrs->addstylename)  {
        if (addstylename == NULL) { /* non XLFD name */
            return(FALSE);
        } else if (strncmp(addstylename, "*", 1) != 0) {
            if (strcmp(addstylename, my_attrs->addstylename) != 0) {
                return(FALSE);                              
            }
        }
    }

    if (my_attrs->size > 0)  {
        if (ptsize == NULL) { /* non XLFD name */
            return(FALSE);
        } else if (strncmp(ptsize, "*", 1) != 0) {
            int		point_size = atoi(ptsize) / 10;
            if (point_size != my_attrs->size) {
                return(FALSE);                              
            }
        }
    }

    if (my_attrs->registry)  {
        if (registry == NULL) { /* non XLFD name */
            return(FALSE);
        } else if (strncmp(registry, "*", 1) != 0) {
            if (strcmp(registry, my_attrs->registry) != 0) {
                return(FALSE);                              
            }
        }
    }

    if (my_attrs->encoding)  {
	if (encoding == NULL) { /* non XLFD name */
            return(FALSE);
        } else if (strncmp(encoding, "*", 1) != 0) {
            if (strcmp(encoding, my_attrs->encoding) != 0) {
                return(FALSE);                              
            }
        }
    }

     return(TRUE);

}


/*ARGSUSED*/
Pkg_private Xv_object
font_find_font(parent_public, pkg, avlist)
    Xv_opaque       parent_public;
    Xv_pkg         *pkg;
    Attr_avlist     avlist;
{
    int			font_attrs_exist;
    Font_locale_info	*linfo;
    struct font_info	*font_list = NULL, *finfo = NULL;
    struct font_return_attrs my_attrs;
    char		*font_name = (char *) NULL;
    Xv_opaque		server;
    char		*locale;


    if (!parent_public) {
	/* xv_create/xv_find ensures that xv_default_server is valid. */
	server = (Xv_opaque) xv_default_server;
    } else {
	Xv_pkg         *pkgtype = (Xv_pkg *) xv_get(parent_public, XV_TYPE);
	if ((Xv_pkg *) pkgtype == (Xv_pkg *) & xv_server_pkg) {
	    server = parent_public;
	} else {
	    server = (Xv_opaque) XV_SERVER_FROM_WINDOW(parent_public);
	}
    }

    linfo = find_font_locale_info(server, avlist);
    if (!linfo) {
	xv_error(XV_ZERO,
	    ERROR_STRING, "Unable to find font locale information",
	    ERROR_PKG, FONT,
	    NULL);
	return ((Xv_object) NULL);
    }

    /* initialization */
    my_attrs.linfo = linfo;
    font_init_create_attrs(&my_attrs);

    font_attrs_exist = font_read_attrs(&my_attrs, FALSE, avlist);
    if (!font_attrs_exist)
	(void) font_default_font(&my_attrs);

    font_list = (Font_info *) xv_get(server, XV_KEY_DATA, FONT_HEAD);
    if (!font_list) {
        font_free_font_return_attr_strings(&my_attrs);
  	return ((Xv_object) NULL);
    }

    if (my_attrs.specifier) {
        for (finfo = font_list; finfo != NULL; finfo = finfo->next)
            if ((linfo == finfo->locale_info) && finfo->specifier &&
		    !strcmp(my_attrs.specifier, finfo->specifier) &&
		    (my_attrs.type == finfo->type))
                break;

	/*
         * If a font set with this font set specifier was not found,
         * check if the specifier is being used as the font name for
         * codeset 0 in any font set in this locale.
         */
        if (finfo == NULL)
            for (finfo = font_list; finfo != NULL; finfo = finfo->next)
           	if ((linfo == finfo->locale_info) && finfo->name && 
                        !strcmp(my_attrs.specifier, finfo->name) &&
			(my_attrs.type == finfo->type))
               	break;
    } else if (my_attrs.names != NULL) {
	/* Bug - FIX me */
    } else if (my_attrs.name) {
	my_attrs.name = normalize_font_name(my_attrs.name, linfo);
        for (finfo = font_list; finfo != NULL; finfo = finfo->next)
            if ((linfo == finfo->locale_info) && finfo->name &&
		    !strcmp(my_attrs.name, finfo->name) &&
		    (my_attrs.type == finfo->type))
                break;
    } else if (my_attrs.resize_from_font) {
	font_name = font_rescale_from_font(my_attrs.resize_from_font,
			my_attrs.rescale_factor, &my_attrs);
	if (font_name && ((int)strlen(font_name) > 0)) {
            for (finfo = font_list; finfo != NULL; finfo = finfo->next)
                if ((linfo == finfo->locale_info) && finfo->specifier &&
		    	!strcmp(font_name, finfo->specifier) &&
			(my_attrs.type == finfo->type))
                    break;

            if (finfo == NULL)
                for (finfo = font_list; finfo != NULL; finfo = finfo->next)
           	    if ((linfo == finfo->locale_info) && finfo->name &&
                   	    !strcmp(font_name, finfo->name) &&
			    (my_attrs.type == finfo->type))
               	    	break;
	}
    } else {
	char        key[256];

	font_fill_in_defaults(&my_attrs);
#ifdef SVR4
        sprintf(key, "%s-%s-%d",
                my_attrs.family ? my_attrs.family : "*",
                my_attrs.style ? my_attrs.style : "*",
                my_attrs.size ? my_attrs.size : 0 );
#else
        sprintf(key, "%s-%s-%d", my_attrs.family, my_attrs.style, my_attrs.size);
#endif /* SVR4 */
	
	for (finfo = font_list; finfo != NULL; finfo = finfo->next)
	    if ((linfo == finfo->locale_info) && finfo->specifier &&
			!strcmp(key, finfo->specifier) &&
			(my_attrs.type == finfo->type))
		break;
	    
	if (finfo == NULL) {
	    char	*str = NULL;
	    if (key && linfo->db ) {
	        str = get_font_set_list(linfo->db, key);
	        
	    } 
	    if (str) {
	        char		** names = construct_font_set_list(str);
	        
	        /* No need to compare the entire name list */
	        for (finfo = font_list; finfo != NULL; finfo = finfo->next)
		    if ((linfo == finfo->locale_info) && finfo->name &&
		            names[0] && (strcmp(names[0], finfo->name) == 0) &&
			    (my_attrs.type == finfo->type))
			break;
	       if (names)
	           free_font_set_list(names);
	    } else {
	        char		*temp_name;
	        (void)font_convert_family(&my_attrs);
	        if (font_convert_style(&my_attrs)) {
		    my_attrs.style = linfo->default_style;
               	    my_attrs.weight = linfo->default_weight;
               	    my_attrs.slant = linfo->default_slant;
 	        }
 	        for (finfo = font_list; finfo != NULL; finfo = finfo->next)
		    if ((linfo == finfo->locale_info) && finfo->name  &&
			    (font_name_is_equivalent(&my_attrs, finfo)) &&
			    (my_attrs.type == finfo->type))
			break;
 	        
	    }        
        }
    }
	    
    font_free_font_return_attr_strings(&my_attrs);

    if (finfo) {
        (void) xv_set(FONT_PUBLIC(finfo), XV_INCREMENT_REF_COUNT, NULL);
  	return (FONT_PUBLIC(finfo));
    } else
        return ((Xv_object) NULL);
}

#else

/*ARGSUSED*/
Pkg_private Xv_object
font_find_font(parent_public, pkg, avlist)
    Xv_opaque       parent_public;
    Xv_pkg         *pkg;
    Attr_avlist     avlist;
{
    int			font_attrs_exist;
    Font_locale_info	*linfo;
    struct font_info	*font_list;
    struct font_return_attrs my_attrs;
    Xv_opaque		server;
    int			error_code;


    if (!parent_public) {
	/* xv_create/xv_find ensures that xv_default_server is valid. */
	server = (Xv_opaque) xv_default_server;
    } else {
	Xv_pkg         *pkgtype = (Xv_pkg *) xv_get(parent_public, XV_TYPE);
	if ((Xv_pkg *) pkgtype == (Xv_pkg *) & xv_server_pkg) {
	    server = parent_public;
	} else {
	    server = (Xv_opaque) XV_SERVER_FROM_WINDOW(parent_public);
	}
    }

    linfo = find_font_locale_info();

    /* initialization */
    my_attrs.linfo = linfo;
    font_init_create_attrs(&my_attrs);

    font_attrs_exist = font_read_attrs(&my_attrs, FALSE, avlist);
    if (!font_attrs_exist)
	(void) font_default_font(&my_attrs);

    error_code = font_construct_name(&my_attrs);

    if (error_code != XV_OK)  {
	return(error_code);
    }

    if (font_list = (Font_info *) xv_get(server, XV_KEY_DATA, FONT_HEAD)) {
	while (font_list) {
	    if (((font_string_compare(my_attrs.name, font_list->name) == 0)
		 && (my_attrs.name != (char *) 0)
		 && (font_list->name != (char *) 0))
	    /*
	     * first above for name, else below for family/style/size/scale
	     * match
		|| (((font_string_compare(my_attrs.family, font_list->family) == 0)
		     && (my_attrs.family != (char *) 0)
		     && (font_list->family != (char *) 0))
	     && (font_string_compare(my_attrs.style, font_list->style) == 0)
		    && ((font_list->size == my_attrs.size) &&
			(font_list->scale == my_attrs.scale)))
	     */
			
			) {
		font_free_font_return_attr_strings(&my_attrs);
		(void) xv_set(FONT_PUBLIC(font_list), XV_INCREMENT_REF_COUNT, NULL);
		return (FONT_PUBLIC(font_list));
	    }
	    font_list = font_list->next;
	}
    }
    font_free_font_return_attr_strings(&my_attrs);
    return ((Xv_object) 0);
}
#endif /*OW_I18N*/

static void
font_free_font_return_attr_strings(struct font_return_attrs *attrs)
{
    if (attrs->orig_name) {
	free(attrs->orig_name);
    }
    if (attrs->free_name) {
	free(attrs->name);
	attrs->free_name = 0;
    }
    if (attrs->free_foundry) {
	free(attrs->foundry);
	attrs->free_foundry = 0;
    }
    if (attrs->free_family) {
	free(attrs->family);
	attrs->free_family = 0;
    }
    if (attrs->free_style) {
	free(attrs->style);
	attrs->free_style = 0;
    }
    if (attrs->free_weight) {
	free(attrs->weight);
	attrs->free_weight = 0;
    }
    if (attrs->free_slant) {
	free(attrs->slant);
	attrs->free_slant = 0;
    }
    if (attrs->free_setwidthname) {
	free(attrs->setwidthname);
	attrs->free_setwidthname = 0;
    }
    if (attrs->free_addstylename) {
	free(attrs->addstylename);
	attrs->free_addstylename = 0;
    }

#ifdef OW_I18N
    if (attrs->free_names) {
	free_font_set_list(attrs->names); 
	attrs->names = (char **) NULL;
    }
#endif /*OW_I18N*/
}

/*
 * the following proc is a wrapper for strcmp() strncmp() such that it will
 * return =0 if both strings are NULL and !=0 if one or other is NULL, and
 * standard strcmp otherwise. BUG: strcmp will seg fault if either str is
 * NULL.
 */
static int
font_string_compare(str1, str2)
    char           *str1, *str2;
{

    if ((str1 == NULL) && (str2 == NULL)) {
	return (int) 0;		/* they're the same (ie. nothing */
    } else if ((str1 == NULL) || (str2 == NULL)) {
	return (int) -1;	/* avoid seg fault */
    } else  {
	/*
        int	i, len;
	char	ch;

	len = strlen(str1);

	for (i=0; i < len; ++i)  {
	    ch = str1[i];
	    if (isalpha(ch) && isupper(ch))  {
		str1[i] = tolower(ch);
	    }
	}

	len = strlen(str2);

	for (i=0; i < len; ++i)  {
	    ch = str2[i];
	    if (isalpha(ch) && isupper(ch))  {
		str2[i] = tolower(ch);
	    }
	}
	*/

	return (int) strcmp(str1, str2);
    }
}

static int
font_string_compare_nchars(str1, str2, n_chars)
    char           *str1, *str2;
    int             n_chars;
{
    int             result;
    int             len1 = (str1) ? strlen(str1) : 0;
    int             len2 = (str2) ? strlen(str2) : 0;
    if ((len1 == 0) && (len2 == 0)) {
	return (int) 0;		/* they're the same (ie. nothing */
    } else if ((len1 && !len2) || (!len1 && len2)) {
	return (int) -1;	/* They're different strings */
    } else if ((!len1) || (!len2)) {
	return (int) -1;	/* avoid seg fault */
    } else  {
	/*
        int	i;
	char	ch;

	for (i=0; i < len1; ++i)  {
	    ch = str1[i];
	    if (isalpha(ch) && isupper(ch))  {
		str1[i] = tolower(ch);
	    }
	}

	for (i=0; i < len2; ++i)  {
	    ch = str2[i];
	    if (isalpha(ch) && isupper(ch))  {
		str2[i] = tolower(ch);
	    }
	}
	*/

	result = strncmp(str1, str2, n_chars);
    }
    return (int) result;
}

static void
font_check_style_less(return_attrs)
    Font_return_attrs return_attrs;
{
    Family_foundry current_entry;
    register int    i;
    char           *requested_family = return_attrs->family;

    if (!return_attrs || !requested_family)  {
	return;
    }

    for (i = 0, current_entry = style_less[i];
            current_entry.family;
            current_entry = style_less[i]) {

        if (font_string_compare_nchars(current_entry.family,
                requested_family, strlen(requested_family)) == 0) {

	    return_attrs->no_style = 1;

            return;
        } else  {
            i++;
        }
    }

    return_attrs->no_style = 0;
}

static void
font_check_size_less(return_attrs)
    Font_return_attrs return_attrs;
{
    Family_foundry current_entry;
    register int    i;
    char           *requested_family = return_attrs->family;

    if (!return_attrs || !requested_family)  {
	return;
    }

    for (i = 0, current_entry = size_less[i];
            current_entry.family;
            current_entry = size_less[i]) {

        if (font_string_compare_nchars(current_entry.family,
                requested_family, strlen(requested_family)) == 0) {

            return_attrs->no_size = 1;
            return;
        } else  {
            i++;
        }
    }

    return_attrs->no_size = 0;
}

static void
font_reduce_wildcards(return_attrs)
    Font_return_attrs return_attrs;
{
    register int    i;
    Wildcards current_entry;
    char           *requested_family = return_attrs->family;

    if (!return_attrs || !requested_family)  {
	return;
    }

    for (i = 0, current_entry = known_wildcards[i];
            current_entry.family;
            current_entry = known_wildcards[i]) {

        if (font_string_compare_nchars(current_entry.family,
                requested_family, strlen(requested_family)) == 0) {

	    if (!return_attrs->foundry)  {
                return_attrs->foundry = current_entry.foundry;
	    }

	    if (!return_attrs->weight)  {
                return_attrs->weight = current_entry.weight;
	    }

	    if (!return_attrs->slant)  {
                return_attrs->slant = current_entry.slant;
	    }

	    if (!return_attrs->setwidthname)  {
                return_attrs->setwidthname = current_entry.setwidthname;
	    }

	    if (!return_attrs->addstylename)  {
                return_attrs->addstylename = current_entry.addstylename;
	    }

	    if (!return_attrs->registry)  {
                return_attrs->registry = current_entry.registry;
	    }

	    if (!return_attrs->encoding)  {
                return_attrs->encoding = current_entry.encoding;
	    }

            return;
        } else  {
            i++;
        }
    }

}

static int
font_read_attrs(return_attrs, consume_attrs, avlist)
    Font_return_attrs	return_attrs;
    int			consume_attrs;
    Attr_avlist		avlist;
{
    register Attr_avlist attrs;
    int             font_attrs_exist = 0;

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch ((int) attrs[0]) {
#ifdef OW_I18N
	  case FONT_LOCALE:
            return_attrs->locale = (char *) attrs[1];
            font_attrs_exist = 1;
            if (consume_attrs)
		ATTR_CONSUME(attrs[0]);
            break;
          case FONT_NAMES:
            return_attrs->names = (char **) attrs[1];
            font_attrs_exist = 1;
            if (consume_attrs)
		ATTR_CONSUME(attrs[0]);
            break;
          case FONT_SET_SPECIFIER:
            return_attrs->specifier = (char *) attrs[1];
            font_attrs_exist = 1;
            if (consume_attrs)
		ATTR_CONSUME(attrs[0]);
            break;
	  case FONT_TYPE:
	    return_attrs->type = (int) attrs[1];
	    font_attrs_exist = 1;
            if (consume_attrs)
		ATTR_CONSUME(attrs[0]);
	    break;
#endif /*OW_I18N*/
	  case FONT_NAME:
	    return_attrs->name = (char *) attrs[1];
	    font_attrs_exist = 1;
	    if (consume_attrs)
		ATTR_CONSUME(attrs[0]);
	    break;
	  case FONT_FAMILY:
	    font_attrs_exist = 1;
	    return_attrs->family = (char *) attrs[1];
            font_check_style_less(return_attrs);
            font_check_size_less(return_attrs);
	    if (consume_attrs)
		ATTR_CONSUME(attrs[0]);
	    break;
	  case FONT_STYLE:
	    font_attrs_exist = 1;
	    return_attrs->style = (char *) attrs[1];
	    if (consume_attrs)
		ATTR_CONSUME(attrs[0]);
	    break;
	  case FONT_SIZE:
	    font_attrs_exist = 1;
	    return_attrs->size = (int) attrs[1];
	    if (consume_attrs)
		ATTR_CONSUME(attrs[0]);
	    break;
	  case FONT_SCALE:
	    font_attrs_exist = 1;
	    return_attrs->scale = (int) attrs[1];
	    if (consume_attrs)
		ATTR_CONSUME(attrs[0]);
	    break;
	  case FONT_SIZES_FOR_SCALE:{
		font_attrs_exist = 1;
		return_attrs->small_size = (int) attrs[1];
		return_attrs->medium_size = (int) attrs[2];
		return_attrs->large_size = (int) attrs[3];
		return_attrs->extra_large_size = (int) attrs[4];
		if (consume_attrs)
		    ATTR_CONSUME(attrs[0]);
		break;
	    }
	  case FONT_RESCALE_OF:{
		Xv_opaque       pf = (Xv_opaque) attrs[1];
		Xv_Font         font = 0;

		XV_OBJECT_TO_STANDARD(pf, "font_read_attrs", font);
		font_attrs_exist = 1;
		return_attrs->resize_from_font = (Font_info *) FONT_PRIVATE(
								      font);
		return_attrs->rescale_factor = (int) attrs[2];
		if (consume_attrs)
		    ATTR_CONSUME(attrs[0]);
		break;
	    }
	  default:
	    break;
	}
    }

    return (font_attrs_exist);
}

static void
font_default_font(return_attrs)
    Font_return_attrs return_attrs;
{
    Font_locale_info	*linfo;

    linfo = return_attrs->linfo;

    return_attrs->family = linfo->default_family;
    return_attrs->style = linfo->default_style;
    return_attrs->weight = linfo->default_weight;
    return_attrs->slant = linfo->default_slant;

    return_attrs->setwidthname = DEFAULT_FONT_SETWIDTHNAME;
    return_attrs->addstylename = DEFAULT_FONT_ADDSTYLENAME;

    return_attrs->scale = (int)linfo->default_scale;
    return_attrs->size = linfo->default_size;
}


static Family_defs *
font_match_family(family, known_families)
char		*family;
Family_defs	*known_families;
{
    int		i, len_of_family;

    len_of_family = (family) ? strlen(family):0;

    for (i=0; i < FONT_NUM_KNOWN_FAMILIES; ++i)  {
	if (font_string_compare_nchars(known_families[i].family, family,
                MAX((int)strlen(known_families[i].family), len_of_family)) == 0) {

	    return(&known_families[i]);
	}
    }

    return((Family_defs *)NULL);
}

/*
 * font_convert_family - checks if given family name is 'known'
 * and translates it if it is. The translation is hoped to be
 * an xlfd family name.
 * Example 'known' family names are:
 *	cmr
 *	cour
 *	gallant
 *	serif
 *	lucidasans
 *	FONT_FAMILY_DEFAULT
 *	FONT_FAMILY_DEFAULT_FIXEDWIDTH
 *	FONT_FAMILY_LUCIDA
 *	FONT_FAMILY_LUCIDA_FIXEDWIDTH
 *	FONT_FAMILY_ROMAN
 *	FONT_FAMILY_SERIF
 *	FONT_FAMILY_CMR
 *	FONT_FAMILY_GALLENT
 *	FONT_FAMILY_COUR
 *	FONT_FAMILY_HELVETICA
 *	FONT_FAMILY_OLGLYPH
 *	FONT_FAMILY_OLCURSOR
 *	etc..
 */
static int
font_convert_family(return_attrs)
Font_return_attrs return_attrs;
{
    Font_locale_info	*linfo;
    Family_defs	*match;
    Family_defs	*known_families;

    if (!return_attrs)  {
	return(XV_ERROR);
    }

    linfo =  return_attrs->linfo;
    known_families = linfo->known_families;

    /*
     * get match for family name in table
     */
    match = font_match_family(return_attrs->family, known_families);

    if (match)  {
	/*
	 * family entry found in table
	 */
        if (!match->translated)  {
	    /*
	     * No translation use the default font family
	     */
            match->translated = linfo->default_family;
        }

	/*
	 * Convert family to entry in table
	 */
        return_attrs->family = match->translated;

	/*
	 * Check for style/size-less family
	 */
        font_check_style_less(return_attrs);
        font_check_size_less(return_attrs);

        return(XV_OK);
    }

    /*
     * No match in table
     */
    return(XV_ERROR);
}

#ifndef OW_I18N
static void
font_init_sizes(linfo)
Font_locale_info	*linfo;
{
    linfo->small_size = 10;
    linfo->medium_size = 12;
    linfo->large_size = 14;
    linfo->xlarge_size = 19;
}

static void
font_init_known_families(linfo)
Font_locale_info	*linfo;
{
    Family_defs	*known_families;
    int		i;

    known_families = (Family_defs *)xv_calloc(FONT_NUM_KNOWN_FAMILIES,
					sizeof(Family_defs));
    
    for (i=0; i < FONT_NUM_KNOWN_FAMILIES; ++i)  {
	known_families[i].family = default_family_translation[i].family;
	known_families[i].translated = (char *)NULL;
    }

    linfo->known_families = known_families;

}

static void
font_init_known_styles(linfo)
Font_locale_info	*linfo;
{
    Style_defs	*known_styles;
    int		i;

    known_styles = (Style_defs *)xv_calloc(FONT_NUM_KNOWN_STYLES,
					sizeof(Style_defs));
    
    for (i=0; i < FONT_NUM_KNOWN_STYLES; ++i)  {
	known_styles[i].style = default_style_translation[i].style;
	known_styles[i].weight = default_style_translation[i].weight;
	known_styles[i].slant = default_style_translation[i].slant;
	known_styles[i].preferred_name = 
		default_style_translation[i].preferred_name;
    }

    linfo->known_styles = known_styles;

}

static void
font_setup_known_families(linfo)
Font_locale_info	*linfo;
{
    Family_defs	*known_families, *match;
    int		null_entry_found,
		numresolved = 0, oldresolved, i;
    char	*family;

    font_init_known_families(linfo);

    known_families = linfo->known_families;

    /*
     * Keep on doing ....
     */
    while (1)  {
	null_entry_found = 0;
	oldresolved = numresolved;
	/*
	 * Search for NULL translation entries
	 */
        for (i=0; i< FONT_NUM_KNOWN_FAMILIES; ++i)  {

	    if (!known_families[i].translated)  {
		/*
		 * NULL entry found
		 */
		null_entry_found = 1;

		/*
		 * DON'T get family name from resource db - not now
		 * anyways
		 */

		/*
                family = (char *) defaults_get_string(known_families[i].family, 
                                known_families[i].family, (char *)NULL);
		*/
                family = (char *)NULL;
		if (family)  {
		    /*
		     * name found in resource db
		     * see if it exists in table
		     */
                    match = font_match_family(family, known_families);
		    if (match)  {
			/*
			 * Name already exists in table
			 * Check if it has a translation
			 */
			if (match->translated)  {
			    /*
			     * It has a translation
			     * Use it
			     */
                            known_families[i].translated = match->translated;
		            ++numresolved;
			}
		    }
		    else  {
			/*
			 * Name NOT already in table
			 * Alloc space and store it in table
			 */
                        known_families[i].translated = xv_strsave(family);
		        ++numresolved;
		    }
		}
		else  {
		    /*
		     * Name does not exist in resource db
		     * Use default font family
		     */
		    if (!default_family_translation[i].translated)  {
                        known_families[i].translated = DEFAULT_FONT_FAMILY;
		    }
		    else  {
		        known_families[i].translated = 
				default_family_translation[i].translated;
		    }
		    ++numresolved;
		}
	    }
        }

	/*
	 * Check if resolved entries don't get resolved
	 * This might be because of recursive family definitions
	 */
	if (null_entry_found && (numresolved == oldresolved))  {
            xv_error(XV_ZERO,
                    ERROR_STRING, 
			XV_MSG("Initialization of font families failed. Possible recursive family definition"),
                    ERROR_PKG, FONT,
                    NULL);
	    return;
	}

	/*
	 * If no more null entries found, we are done initializing
	 */
	if (!null_entry_found)  {
	    return;
	}

    }
}

static void
font_setup_known_styles(Font_locale_info *linfo)
{
    font_init_known_styles(linfo);
}

static void font_setup_defaults(Font_locale_info *linfo)
{
    linfo->default_family = strdup("lucida");
    linfo->default_fixedwidth_family = strdup("lucidatypewriter");
    linfo->default_style = strdup("normal");
    linfo->default_weight = strdup("medium");
    linfo->default_slant = strdup("r");
    linfo->default_scale = (int)WIN_SCALE_MEDIUM;
    linfo->default_scale_str = strdup("Medium");
    linfo->default_size = 12;

    linfo->default_small_font = 
	strdup("-b&h-lucida-medium-r-*-*-*-100-*-*-*-*-*-*");
    linfo->default_medium_font = 
	strdup("-b&h-lucida-medium-r-*-*-*-120-*-*-*-*-*-*");
    linfo->default_large_font = 
	strdup("-b&h-lucida-medium-r-*-*-*-140-*-*-*-*-*-*");
    linfo->default_xlarge_font = 
	strdup("-b&h-lucida-medium-r-*-*-*-190-*-*-*-*-*-*");
}
#endif /* OW_I18N */

/*
 * font_convert_style - checks if given style name is 'known'
 * and translates it if it is. The translation is hoped to be
 * an xlfd weight/slant combination.
 * Example 'known' style names are:
 *	r
 *	b
 *	i
 *	o
 *	normal
 *	bold
 *	italic
 *	oblique
 *	bolditalic
 *	boldoblique
 *	demibold
 *	demiitalic
 *	DEFAULT_FONT_STYLE
 *	FONT_STYLE_DEFAULT
 *	FONT_STYLE_NORMAL
 *	FONT_STYLE_BOLD
 *	FONT_STYLE_ITALIC
 *	FONT_STYLE_OBLIQUE
 *	FONT_STYLE_BOLD_ITALIC
 *	FONT_STYLE_BOLD_OBLIQUE
 *	etc..
 */
Pkg_private int
font_convert_style(return_attrs)
Font_return_attrs return_attrs;
{
    Font_locale_info	*linfo;
    Style_defs	current_entry, *known_styles;
    int		i, len_of_style;

    if (return_attrs->no_style)  {
        return(XV_OK);
    }

    linfo = return_attrs->linfo;
    known_styles = linfo->known_styles;
    len_of_style = (return_attrs->style) ? strlen(return_attrs->style):0;

    for (i=0, current_entry = known_styles[i]; 
		i < FONT_NUM_KNOWN_STYLES; 
		current_entry = known_styles[++i])  {
	if (font_string_compare_nchars(current_entry.style, return_attrs->style,
		MAX((int)strlen(current_entry.style), len_of_style)) == 0) {

	    return_attrs->style = current_entry.preferred_name;
	    return_attrs->weight = current_entry.weight;
	    return_attrs->slant = current_entry.slant;

	    return(XV_OK);
	}
    }

    return(XV_ERROR);
}

Pkg_private int
font_convert_weightslant(return_attrs)
Font_return_attrs return_attrs;
{
    Style_defs	current_entry, *known_styles;
    int		i, len_of_weight, len_of_slant;

    if (return_attrs->no_style)  {
        return(0);
    }

    known_styles = return_attrs->linfo->known_styles;
    len_of_weight = (return_attrs->weight) ? strlen(return_attrs->weight):0;
    len_of_slant = (return_attrs->slant) ? strlen(return_attrs->slant):0;

    for (i=0, current_entry = known_styles[i]; 
		i < FONT_NUM_KNOWN_STYLES; 
		current_entry = known_styles[++i])  {
	if ( (font_string_compare_nchars(current_entry.weight, return_attrs->weight,
			      MAX((int)strlen(current_entry.weight), len_of_weight)) == 0) &&
	     (font_string_compare_nchars(current_entry.slant, return_attrs->slant,
			      MAX((int)strlen(current_entry.slant), len_of_slant)) == 0) ) {

	    return_attrs->style = current_entry.preferred_name;

	    return(0);
	}
    }

    return(-1);
}

static int
font_decrypt_xlfd_name(my_attrs)
Font_return_attrs my_attrs;
{
    Font_locale_info	*linfo;
    int			tempSize;
    int			medsize, largesize, xlargesize;
    char		tempName[255];
    char		*foundry = NULL;
    char		*family = NULL;
    char		*weight = NULL;
    char		*slant = NULL;
    char		*setwidthname = NULL;
    char		*addstylename = NULL;
    char		*pixsize = NULL;
    char		*ptsize = NULL;
    char		*xres = NULL;

    if (my_attrs->name == NULL)  {
	return(-1);
    }

    linfo = my_attrs->linfo;

    sprintf(tempName, "%s", my_attrs->name);

    foundry = font_strip_name(tempName, FOUNDRYPOS, DASH);
    family = font_strip_name(tempName, FAMILYPOS, DASH);
    weight = font_strip_name(tempName, WEIGHTPOS, DASH);
    slant = font_strip_name(tempName, SLANTPOS, DASH);
    setwidthname = font_strip_name(tempName, SETWIDTHNAMEPOS, DASH);
    addstylename = font_strip_name(tempName, ADDSTYLENAMEPOS, DASH);
    pixsize = font_strip_name(tempName, PIXSIZEPOS, DASH);
    ptsize = font_strip_name(tempName, PTSIZEPOS, DASH);
    xres = font_strip_name(tempName, XRESOLUTIONPOS, DASH);

    *(family-1) = '\0';
    *(weight-1) = '\0';
    *(slant-1) = '\0';
    *(setwidthname-1) = '\0';
    *(addstylename-1) = '\0';
    *(pixsize-1) = '\0';
    *(ptsize-1) = '\0';
    *(xres-1) = '\0';

    if ((*foundry) && (*foundry != DASH))  {
	my_attrs->foundry = xv_strsave(foundry);
	my_attrs->free_foundry = 1;
    }

    if ((*family) && (*family != DASH))  {
	my_attrs->family = xv_strsave(family);
	my_attrs->free_family = 1;
    }

    if ((*weight) && (*weight != DASH))  {
	my_attrs->weight = xv_strsave(weight);
	my_attrs->free_weight = 1;
    }

    if ((*slant) && (*slant != DASH))  {
	my_attrs->slant = xv_strsave(slant);
	my_attrs->free_slant = 1;
    }

    if ( (*setwidthname) && (*setwidthname != DASH))  {
	my_attrs->setwidthname = xv_strsave(setwidthname);
	my_attrs->free_setwidthname = 1;
    }

    if ( (*addstylename) && (*addstylename != DASH))  {
	my_attrs->addstylename = xv_strsave(addstylename);
	my_attrs->free_addstylename = 1;
    }

    if ((*ptsize) && (*ptsize != DASH) && (*ptsize != '*'))  {
	tempSize = atoi(ptsize);
	my_attrs->size = tempSize / 10;

	medsize = (my_attrs->medium_size < 0) 
		? linfo->medium_size : my_attrs->medium_size;
	if (my_attrs->size < medsize)  {
	    my_attrs->scale = (int)WIN_SCALE_SMALL;
	}
	else  {
	    largesize = (my_attrs->large_size < 0) 
		? linfo->large_size : my_attrs->large_size;
	    if (my_attrs->size < largesize)  {
	        my_attrs->scale = (int)WIN_SCALE_MEDIUM;
	    }
	    else  {
	        xlargesize = (my_attrs->extra_large_size < 0) 
		    ? linfo->xlarge_size : my_attrs->extra_large_size;
		if (my_attrs->size < xlargesize)  {
	            my_attrs->scale = (int)WIN_SCALE_LARGE;
		}
		else  {
	            my_attrs->scale = (int)WIN_SCALE_EXTRALARGE;
		}
	    }
	}
    }

    if ((my_attrs->weight) && (my_attrs->slant))  {
	font_convert_weightslant(my_attrs);
    }

    return (0);
}

/*
 * Decrypts the font name given into family, style, and size.
 * The font names recognized are:
 *	<family>delim<style>delim<size>
 *	<family>delim<size>
 *	<family>
 * where delim is any delimiter in the array known_delimiters.
 *
 * On any error condition, XV_ERROR is returned, else XV_OK is
 * returned
 */
static int
font_decrypt_misc_name(my_attrs)
Font_return_attrs my_attrs;
{
    Font_locale_info	*linfo;
    int			tempSize;
    char		tempName[255];
    char		*family = NULL;
    char		*style = NULL;
    char		*pixsize = NULL;
    char		delim;
    int			i;

    if (my_attrs->name == NULL)  {
	return(XV_ERROR);
    }

    linfo = my_attrs->linfo;
    sprintf(tempName, "%s", my_attrs->name);

    /* 
     * For all known delimiters, try to decrypt the family, style,
     * and size
     */ 
    for (i=0, delim = known_delimiters[i]; 
	delim; 
	++i, delim = known_delimiters[i])  {
        if (font_delim_count(tempName, delim) == 0)  {
	    continue;
	}
        family = font_strip_name(tempName, 0, delim);
        style = font_strip_name(tempName, 1, delim);
        pixsize = font_strip_name(tempName, 2, delim);

	if (family || style || pixsize)  {
	    break;
	}
    }

    /*
     * If cannot decrypt family/style/size for any delimiter
     * return
     */
    if (!family && !style && !pixsize)  {
	return(XV_ERROR);
    }

    if (style)  {
        *(style-1) = '\0';
    }

    if (pixsize)  {
        *(pixsize-1) = '\0';
    }

    /*
     * Remember the delimiter used in case have to reconstruct 
     * font name
     */
    my_attrs->delim_used = delim;

    if (family && (*family) && (*family != delim))  {
        my_attrs->family = family;
	/*
	 * See if family obtained is sunview font family or
	 * any other font that can be translated.
	 */
        if (font_convert_family(my_attrs))  {
	    /*
	     * If conversion not done, alloc new space 
	     * for family to avoid dangling pointer
	     * to string. 'family' is a local variable.
	     */
            my_attrs->family = xv_strsave(family);
            my_attrs->free_family = 1;
	}
    }

    /*
     * Try to translate style
     */
    if (style && (*style) && (*style != delim))  {
	/*
	 * Check if font family is style-less eg. Open Look
	 * Glyph/Cursor fonts
	 */
	if (my_attrs->no_style)  {
	    if (!pixsize)  {
		/*
		 * If a style was specified for a styless family,
		 * and a size was not specified, try to use the 
		 * style as a size.
		 * This catches the case of the font name:
		 *	olglyph-12
		 * where olglyph is a style-less font family.
		 * Size decryption is handled below.
		 */
		pixsize = style;
	    }
	}
	else  {
	    /*
	     * the font family is NOT style-less
	     */
            my_attrs->style = style;

            if (font_convert_style(my_attrs))  {

	        /*
	         * Style given not recognized
	         */

	        /*
                 * If cannot convert style into any weight/slant combination
                 * give 'normal' style - medium/roman.
	         * Don't have to alloc new space here because
	         * these macros are static.
	         */
                my_attrs->style = linfo->default_style;
                my_attrs->weight = linfo->default_weight;
                my_attrs->slant = linfo->default_slant;

	        /*
	         * If a size was not specified as the 3rd field, it may have
	         * been passed as the 2nd field. i.e. instead of a name like
	         * lucida-b-12, a name like lucida-12 might be used.
	         * To detect this, see if the size field (the 3rd field) is set.
	         */
	        if (!pixsize)  {
		    /*
		     * Try to use the style as a size.
		     * Size decryption is handled below.
		     */
		    pixsize = style;
	        }
            }
        }
    }

    /*
     * Try to translate size
     */
    if (pixsize && (*pixsize) && (*pixsize != delim) && (*pixsize != '*'))  {
        tempSize = atoi(pixsize);
        if (tempSize > 0)  {
            my_attrs->size = tempSize;
        }

    }

    return (XV_OK);
}

static void
font_fill_in_defaults(font_attrs)
Font_return_attrs	font_attrs;
{
    Font_locale_info	*linfo;

    linfo = font_attrs->linfo;

#ifdef OW_I18N
    if (!font_attrs->family || 
	    !strcmp(font_attrs->family, FONT_FAMILY_DEFAULT))  {
	font_attrs->family = linfo->default_family;
    }
#else
    if (!font_attrs->family)  {
	font_attrs->family = linfo->default_family;
    }
#endif /*OW_I18N*/

#ifdef OW_I18N
    if ((!font_attrs->style && !font_attrs->no_style) || 
        (font_attrs->style && !strcmp(font_attrs->style, FONT_STYLE_DEFAULT))) {
#else
    if (!font_attrs->style && !font_attrs->no_style)  {
#endif /*OW_I18N*/
	font_attrs->style = linfo->default_style;
	font_attrs->weight = linfo->default_weight;
	font_attrs->slant = linfo->default_slant;
    }

    if (((font_attrs->size == FONT_SIZE_DEFAULT) || 
	 (font_attrs->size == FONT_NO_SIZE)) && !font_attrs->no_size)  {
	if (font_attrs->scale == FONT_NO_SCALE)  {
	    font_attrs->scale = font_get_default_scale(linfo);
	}

	font_attrs->size = font_size_from_scale(font_attrs, 
	                    font_attrs->scale);
    }

    if ((font_attrs->scale == FONT_NO_SCALE) && !font_attrs->no_size)  {
        font_attrs->scale = font_scale_from_size(font_attrs, 
					font_attrs->size);
    }
}

#ifdef OW_I18N

/*
 * font_construct_names
 * Uses the creation attributes to obtain a set of font names for creating 
 * the font set.
 * The precedence is:
 *      by specifier
 *      by names
 *	by name
 *	by resizing
 *	by specifying family/style/size
 */
static int
font_construct_names(Display *display, Font_return_attrs font_attrs)
{
    Font_locale_info	*linfo = font_attrs->linfo;
    char		*font_name;
    char		*str;
    static char		*base_list[2];

    /*
     * Font name is specified.
     * The font name has precedence over other attributes.
     */
    if ((font_attrs->specifier) && (font_attrs->name != font_attrs->specifier)) {
    /*
     *  If name == specifier, then we have tried to open the font with specifier
     *  once already.  So got to the font_attrs_name section.  Otherwise
     *  try to look up specifier first.
     */
        font_attrs->name = font_attrs->specifier;
        if (str = get_font_set_list(linfo->db, font_attrs->specifier)) {
            font_attrs->names = construct_font_set_list(str);
        } else if (linfo->db) {
        /* if specifier is not in db, then try expanding it to xlfd */
           goto font_attrs_name;
        } else {
            /* 
             * If no db to lookup, then open with specifier first.  If it
             * doesn't work, the next call to font_construct_names()
             * name == specifier and it will be at the font_attrs_name
             * section to expand specifier to xlfd.
             */
            font_attrs->names = construct_font_set_list(font_attrs->specifier);
	}
	font_attrs->free_names = TRUE;
    } else if ((font_attrs->names) && (font_attrs->names != base_list)) {
	font_attrs->free_names = FALSE;
    } else if (font_attrs->name) {
font_attrs_name:
        if (font_attrs->specifier || font_attrs->orig_name) {
        /*
         *  Only in here if either we tried to open the font using
         *  font_attrs->specifier or font_attrs->name, but it didn't work. 
         *  So now we'll try to expand font_attrs->name to xlfd name.
         */
            if (font_attrs->orig_name)
                free(font_attrs->orig_name);
	    font_attrs->name = normalize_font_name(font_attrs->name, linfo);

        /*
	 * If name is xlfd conforming, decrypt it to fill in other
	 * fields of font object - style, size, etc.
	 *
         * Must use some sort of xlfd-detecting engine here
         * instead of just counting the number of dashes.
         */
            if (font_delim_count(font_attrs->name, DASH) == NUMXLFDFIELDS)  {
	        (void)font_decrypt_xlfd_name(font_attrs);
	    } else  {
	    /*
	     * Non-xlfd name given.
	     * Check if it can still be decrypted, i.e. if it is of the form
	     *     <family>delim<style>delim<size>
	     * where delim is a delimiter like '-' and '.'
	     */
	        if (!font_decrypt_misc_name(font_attrs))  {
		/*
		 * If decryption successful, construct new xlfd font 
		 * name with new fields
		 */
                    font_fill_in_defaults(font_attrs);
	            (void)font_determine_font_name(font_attrs);
	        }
	    }
        }

	
	font_attrs->orig_name = xv_strsave(font_attrs->name);
	base_list[0] = font_attrs->name;
	base_list[1] = NULL;
	font_attrs->names = base_list;
	font_attrs->free_names = FALSE;
    } else if (font_attrs->resize_from_font) {
	/* Resize from font specified.*/
	font_name = font_rescale_from_font(font_attrs->resize_from_font,
                            font_attrs->rescale_factor, font_attrs);

	/* If rescaling failed, return error code */
	if ((font_name == NULL) || (strlen(font_name) == 0))  {
	    xv_error(XV_ZERO,
		ERROR_STRING, "Attempt to rescale from font failed",
		ERROR_PKG, FONT,
		NULL);
	    return (XV_ERROR);
	}

	font_attrs->name = font_name;

        if ((str = get_font_set_list(linfo->db, font_name)) != NULL) {
	    font_attrs->names = construct_font_set_list(str);
            font_attrs->free_names = TRUE;
            font_attrs->specifier = font_name;
        }
 
        if (font_attrs->names == NULL) {
            base_list[0] = font_name;
            base_list[1] = NULL;
	    font_attrs->names = base_list;
	    font_attrs->free_names = FALSE;
        } 
    } else {
	/*
         * Attempt to find a font set definition using a concatenation of the
         * font family, style and pixel size. If a definition is not found,
         * generate a font name assuming only one font is required for that
         * locale.
         */
	font_fill_in_defaults(font_attrs);
	font_attrs->names = NULL;
	if (linfo->db && (font_attrs->type == FONT_TYPE_TEXT)) {
            char        key[256];
	    char	*family;
	    char	*style;
	    int		size;

	    if (font_attrs->family == NULL)
		family = linfo->default_family;
	    else 
		family = font_attrs->family;

	    if (font_attrs->style == NULL)
		style = linfo->default_style;
	    else 
		style = font_attrs->style;

	    if (font_attrs->size == FONT_NO_SIZE)
		size = linfo->default_size;
	    else
		size = font_attrs->size;
 
            sprintf(key, "%s-%s-%d", family, style, size);
 
            str = get_font_set_list(linfo->db, key);
            if (str != NULL) {
                font_attrs->names = construct_font_set_list(str);
                font_attrs->free_names = TRUE;
                font_attrs->specifier = strdup(key);
            }
        }      

	if ((font_attrs->names == NULL) ) {
	    /* Construct name using the family/style/size attributes */
            (void)font_convert_family(font_attrs);

            if (font_convert_style(font_attrs))  {
                char	dummy[128];

                /*
                 * If cannot convert style into any weight/slant combination
                 * give 'normal' style - medium/roman
                 */
                sprintf(dummy, 
	            XV_MSG("Font style %s is not known, using default style instead"),
                            font_attrs->style);
                xv_error(XV_ZERO,
                    ERROR_STRING, dummy,
                    ERROR_PKG, FONT,
                    NULL);

                font_attrs->style = linfo->default_style;
                font_attrs->weight = linfo->default_weight;
                font_attrs->slant = linfo->default_slant;
            }

	    font_name = font_determine_font_name(font_attrs);
	    if ((font_name == NULL) || (strlen(font_name) == 0)) {
	        char            dummy[128];

	        (void) sprintf(dummy, 
			XV_MSG("Cannot load font '%s'"), 
			font_name);
	        xv_error(XV_ZERO,
		         ERROR_STRING, dummy,
		         ERROR_PKG, FONT,
		         NULL);
	        return (XV_ERROR);
	    }
	    font_attrs->name = font_name;

	    base_list[0] = font_name;
	    base_list[1] = NULL;
	    font_attrs->names = base_list;
	    font_attrs->free_names = FALSE;
	}
    }

    if (!font_attrs->names)  {
        xv_error(XV_ZERO,
            ERROR_STRING, XV_MSG("Failed to find font names"),
            ERROR_PKG, FONT,
            NULL);
        return (XV_ERROR);
    }

    return(XV_OK);
}

#endif /*OW_I18N*/

/*
 * font_construct_name
 * Uses the creation attributes to create a font name for XLoadQueryFont
 * The precedence is:
 *	by name
 *	by resizing
 *	by specifying family/style/size
 */
static int
font_construct_name(Font_return_attrs font_attrs)
{
    Font_locale_info	*linfo = font_attrs->linfo;
    char	*font_name;

    /*
     * Font name is specified.
     * The font name has precedence over other attributes.
     */
    if (font_attrs->name) {
        font_attrs->orig_name = xv_strsave(font_attrs->name);

	font_attrs->name = normalize_font_name(font_attrs->name, linfo);

        /*
	 * If name is xlfd conforming, decrypt it to fill in other
	 * fields of font object - style, size, etc.
	 *
         * Must use some sort of xlfd-detecting engine here
         * instead of just counting the number of dashes.
         */
        if (font_delim_count(font_attrs->name, DASH) == NUMXLFDFIELDS)  {
	    (void)font_decrypt_xlfd_name(font_attrs);
	}
	else  {
	    /*
	     * Non-xlfd name given.
	     * Check if it can still be decrypted, i.e. if it is of the form
	     *     <family>delim<style>delim<size>
	     * where delim is a delimiter like '-' and '.'
	     */
	    if (!font_decrypt_misc_name(font_attrs))  {
		/*
		 * If decryption successful, construct new xlfd font 
		 * name with new fields
		 */
                font_fill_in_defaults(font_attrs);
	    }
	}
    }
    else  {
	/*
	 * Resize from font specified.
	 */
        if (font_attrs->resize_from_font) {
	    font_name = font_rescale_from_font(font_attrs->resize_from_font,
                            font_attrs->rescale_factor, font_attrs);
	    /*
	     * If rescaling failed, return error code
	     */
	    if ((font_name == NULL) || (strlen(font_name) == 0))  {
	        char            dummy[128];

	        sprintf(dummy, 
			XV_MSG("Attempt to rescale from font failed"));
	        xv_error(XV_ZERO,
		     ERROR_STRING, dummy,
		     ERROR_PKG, FONT,
		     NULL);
	        return (XV_ERROR);
	    }
        } else {
            font_fill_in_defaults(font_attrs);

	    /*
	     * Construct name using the family/style/size attributes
	     */
            (void)font_convert_family(font_attrs);

            if (font_convert_style(font_attrs))  {
                char	dummy[128];

                /*
                 * If cannot convert style into any weight/slant combination
                 * give 'normal' style - medium/roman
                 */
                sprintf(dummy, 
	            XV_MSG("Font style %s is not known, using default style instead"),
                            font_attrs->style);
                xv_error(XV_ZERO,
                    ERROR_STRING, dummy,
                    ERROR_PKG, FONT,
                    NULL);

                font_attrs->style = linfo->default_style;
                font_attrs->weight = linfo->default_weight;
                font_attrs->slant = linfo->default_slant;
            }

	    font_name = font_determine_font_name(font_attrs);
	    if ((font_name == NULL) || (strlen(font_name) == 0)) {
	        char            dummy[128];

	        (void) sprintf(dummy, 
			XV_MSG("Cannot load font '%s'"), 
			font_name);
	        xv_error(XV_ZERO,
		         ERROR_STRING, dummy,
		         ERROR_PKG, FONT,
		         NULL);
	        return (XV_ERROR);
	    }
        }
    }

    if (!font_attrs->name)  {
        xv_error(XV_ZERO,
            ERROR_STRING, XV_MSG("Failed to construct font name"),
            ERROR_PKG, FONT,
            NULL);
        return (XV_ERROR);
    }

    return(XV_OK);
}


#ifndef OW_I18N

static XID
font_try_misc_name(font_attrs, display, x_font_info, default_x, 
		default_y, max_char, min_char)
Font_return_attrs	font_attrs;
Display			*display;
XFontStruct		**x_font_info;
int			*default_x;
int			*default_y;
int			*max_char;
int			*min_char;
{

    Font_locale_info	*linfo;
    XID		xid;
    Style_defs	cur_style, *known_styles;
    char	cur_delim;
    int		style_index,
		delim_index;
    int		size;
    int		no_size;
    char	*family;
    char	*style;
    char	cur_name[256];
    char	temp[256];

    /*
     * If no font attributes
     * return NULL
     */
    if (!font_attrs)  {
	return((XID)NULL);
    }

    linfo = font_attrs->linfo;
    known_styles = linfo->known_styles;

    family = font_attrs->family;
    style = font_attrs->style;
    size = font_attrs->size;
    no_size = font_attrs->no_size;
    cur_delim = font_attrs->delim_used;

    /*
     * If no family or (no style AND no size)
     * return NULL
     */
    if (!family || (!style && ((size < 0) || no_size)))  {
	return((XID)NULL);
    }

    for (style_index=0, cur_style = known_styles[style_index];
	 style_index < FONT_NUM_KNOWN_STYLES;
	 cur_style = known_styles[++style_index])  {

	/*
	 * Try for every style that is equivalent to the one for this font
	 */
	if  (!font_string_compare(style, cur_style.preferred_name))  {
	    /*
	     * If a delimiter was known to be used to decrypt the 
	     * original font name, use it.
	     */
	    if (cur_delim)  {
                /*
                 * Construct name
                 */
                sprintf(cur_name, "%s", family);
    
                if (style)  {
                    /*
                     * family-style
                     */
                    sprintf(temp, "%c%s", cur_delim, cur_style.style);
                    strcat(cur_name, temp);
    
                }

                if (!no_size && (size > 0))  {
                    /*
                     * family-style-size
                     * or 
                     * family-size
                     */
                    sprintf(temp, "%c%d", cur_delim, size);
                    strcat(cur_name, temp);
                }

                /*
                 * Try to load font with constructed name
                 */
                xid = xv_load_x_font((Display *) display,
                        cur_name,
                        x_font_info, default_x, default_y,
                        max_char, min_char);

                if (xid)  {
                    if (font_attrs->free_name)  {
                        free(font_attrs->name);
                    }
                    font_attrs->name = xv_strsave(cur_name);
                    return(xid);
                }
	    }
	    else  {
	        /*
	         * Try every delimiter known
	         */
	        for (delim_index=0; known_delimiters[delim_index]; 
		     ++delim_index)  {

		    cur_delim = known_delimiters[delim_index];

		    /*
		     * Construct name
		     */
		    sprintf(cur_name, "%s", family);

		    if (style)  {
		        /*
		         * family-style
		         */
		        sprintf(temp, "%c%s", cur_delim, cur_style.style);
		        strcat(cur_name, temp);

		    }

		    if (!no_size && (size > 0))  {
		        /*
		         * family-style-size
		         * or 
		         * family-size
		         */
		        sprintf(temp, "%c%d", cur_delim, size);
		        strcat(cur_name, temp);
		    }

                    /*
                     * Try to load font with constructed name
                     */
                    xid = xv_load_x_font((Display *) display,
			     cur_name,
			     x_font_info, default_x, default_y,
			     max_char, min_char);

		    if (xid)  {
	                if (font_attrs->free_name)  {
	                    free(font_attrs->name);
	                }
		        font_attrs->name = xv_strsave(cur_name);
		        return(xid);
		    }
	        }
	    }
	}
    }

    return((XID)NULL);
}
#endif /*OW_I18N*/


static void
font_setup_pixfont(font_public)
Xv_font_struct	*font_public;
{
    register struct pixchar *pfc;
    Font_info		*xv_font_info = FONT_PRIVATE(font_public);
#ifdef OW_I18N
    XFontStruct         *x_font_info = xv_font_info->font_structs[0];
#else
    XFontStruct		*x_font_info = (XFontStruct *)xv_font_info->x_font_info;
#endif /*OW_I18N*/
    Pixfont		*pixfont = (Pixfont *)xv_get((Xv_opaque)font_public, FONT_PIXFONT);
    int		i, default_x, default_y, 
		max_char, min_char;

    default_x = xv_font_info->def_char_width;
    default_y = xv_font_info->def_char_height;

    /*
     * Pixfont compat
     */
    max_char = MIN(255, x_font_info->max_char_or_byte2);
    min_char = MIN(255, x_font_info->min_char_or_byte2);

    pixfont->pf_defaultsize.x = default_x;
    pixfont->pf_defaultsize.y = default_y;
    for (i = min_char, pfc = &(pixfont->pf_char[i]);
	 i <= MIN(255, max_char);	/* "i" cannot ever be >255 - pixfont
					 * compat */
	 i++, pfc++) {
	    xv_x_char_info((XFontStruct *) x_font_info, i - min_char,
		       &pfc->pc_home.x, &pfc->pc_home.y,
		       &pfc->pc_adv.x, &pfc->pc_adv.y,
		       &pfc->pc_pr);
    }
}

#ifdef CHECK_VARIABLE_HEIGHT
static void
font_check_var_height(int *variable_height_font, XFontStruct *x_font_info)
{
    *variable_height_font = FALSE;
}
#endif

#ifdef CHECK_OVERLAPPING_CHARS
static void
font_check_overlapping(int *neg_left_bearing, XFontStruct *x_font_info)
{
    *neg_left_bearing = (x_font_info->min_bounds.lbearing < 0);
}
#endif


Pkg_private int
font_init_pixfont(font_public)
Xv_font_struct	*font_public;
{
    Font_info		*xv_font_info = FONT_PRIVATE(font_public);
    Pixfont_struct	*pf_rec;
    
    pf_rec = (Pixfont_struct *)xv_malloc(sizeof(Pixfont_struct));

    xv_font_info->pixfont = (char *)(pf_rec);

    font_setup_pixfont(font_public);
    pf_rec->public_self = (Xv_font)font_public;

    return(XV_OK);

}

static void font_init_create_attrs(Font_return_attrs font_attrs)
{
    Font_locale_info	*linfo;

    linfo = font_attrs->linfo;

    font_attrs->free_name = font_attrs->free_foundry = 
    font_attrs->free_family = font_attrs->free_style = 
    font_attrs->free_weight = font_attrs->free_slant = 
    font_attrs->free_setwidthname = 
    font_attrs->free_addstylename = (int)0;

    font_attrs->orig_name = 
    font_attrs->name = font_attrs->foundry = 
    font_attrs->family = font_attrs->style = 
    font_attrs->weight = font_attrs->slant = 
    font_attrs->setwidthname = 
    font_attrs->addstylename = 
    font_attrs->encoding = 
    font_attrs->registry = (char *)NULL;
    font_attrs->delim_used = '\0';

    font_attrs->size = (int) FONT_NO_SIZE;
    if (linfo)  {
	/*
	 * Set default sizes according to locale
	 */
        font_attrs->small_size = linfo->small_size;
        font_attrs->medium_size = linfo->medium_size;
        font_attrs->large_size = linfo->large_size;
        font_attrs->extra_large_size = linfo->xlarge_size;
    }
    else  {
        font_attrs->small_size = DEFAULT_SMALL_FONT_SIZE;
        font_attrs->medium_size = DEFAULT_MEDIUM_FONT_SIZE;
        font_attrs->large_size = DEFAULT_LARGE_FONT_SIZE;
        font_attrs->extra_large_size = DEFAULT_XLARGE_FONT_SIZE;
    }

    font_attrs->scale = FONT_NO_SCALE;
    font_attrs->rescale_factor = (int) 0;
    font_attrs->resize_from_font = (Font_info *) 0;
    font_attrs->no_size = font_attrs->no_style = 0;

#ifdef OW_I18N
    font_attrs->type = FONT_TYPE_TEXT;
    font_attrs->locale = font_attrs->linfo->locale;
    font_attrs->specifier = (char *)NULL;
    font_attrs->names = (char **)NULL;
    font_attrs->free_names = (short)0;
#endif /*OW_I18N*/
}

static int
font_delim_count(str, delim)
char	*str, delim;
{
    register char	*p1, *p2 = str;
    int		count = 0;

    if (str == NULL)  {
	return(0);
    }

    p1 = XV_INDEX(str, delim);

    if (p1 == NULL)  {
	return(0);
    }

    ++count;

    while(p2 != NULL)  {
	p2 = XV_INDEX(p1+1, delim);

	if (p2 != NULL)  {
	    ++count;
	    p1 = p2;
	}
    }

    return(count);
}

static char *
font_strip_name(str, pos, delim)
char	*str;
int	pos;
char	delim;
{
    char	*p1 = str, *p2;

    if ((str == NULL) || (pos < 0) || (delim == '\0'))  {
	return((char *)NULL);
    }
    
    while(pos)  {
	p2 = XV_INDEX(p1, delim);
	if (p2 == NULL)  {
	    return((char *)NULL);
	}
	--pos;
	p1 = p2 + 1;
    }

    /*
     * the string at position `pos` spans p1 to 
     * strip_xlfd(str, pos+1)-1
     */
    return(p1);
}


