#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)svr_parse.c 1.9 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1992 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <ctype.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xresource.h>
#include <xview_private/svr_impl.h>
#include <xview_private/portable.h>

/*
 * This file contain only one exported function:
 *
 *	Xv_private int
 *	server_parse_keystr(server_public, keystr, keysym, code, 
 *				modifiers, diamond_mask, qual_str)
 *	Xv_server	server_public;
 *	CHAR		*keystr;
 *	KeySym		*keysym;
 *	short		*code;
 *	unsigned int	*modifiers;
 *	unsigned int	diamond_mask;
 *	char		*qual_str;
 *
 * It parses strings that specify key combinations. It was 
 * introduced for Menu Accelerators, but it should be shared by
 * mouseless as well. It recognizes Xt, OLIT, and XView syntax:
 *	Xt: 		[modifier...] '<keypress>' key
 *	OLIT:		[OLITmodifier...] '<'key'>'
 *	XView:		[modifier ['+' modifier] '+'] key
 *	modifier:	'meta' | 'shift' | 'alt' | 'hyper' | 'ctrl' | 
 *			'mod1' | ... | 'mod5'
 *	OLITmodifier:	modifier | 'm' | 's' | 'a' | 'h' | 'c'
 *	key:		all print characters and keysym names (e.g.
 *			'return', 'tab', 'comma', 'period', etc...)
 */

/*
 * START of declarations for parsing engine
 */

typedef struct acceleratorValue {
    KeySym keysym;
    unsigned meta:1,
	     shift:1,
	     alt:1,
	     ctrl:1,
	     super:1,
	     hyper:1,
	     lock:1,
	     modeswitch:1,
	     mod1:1, mod2:1, mod3:1, mod4:1, mod5:1,
	     error:1,
	     none:1,
	     some:1,
	     reserved:16;
} AcceleratorValue;

typedef struct {
    enum { styleNone, styleXView, styleOLIT, styleXt } style;
    AcceleratorValue av;
    char *pos;
} AVState;

typedef enum {
    modifMeta, modifShift, modifAlt, modifCtrl, modifSuper, modifHyper, 
    modifLock, modifModeswitch,
    modifMod1, modifMod2, modifMod3, modifMod4, modifMod5,
    modifNone
} AVModif;

typedef struct {
    CHAR *string;
    AVModif modif;
} AVKeyword;

#ifdef OW_I18N

/*
 * Macro to define Process code string/character literal
 * e.g. widechar string literals should look like:
 *	L"foo"
 * multibyte strings look like:
 *	"foo"
 */
#define XV_PROC_CODE(s)          L ## s

/*
 * General macros that should be moved to misc/i18n_impl.h
 */
#define STRSPN		wsspn
#define SSCANF		wsscanf
#define ISPUNCT		iswpunct
#define ISSPACE		iswspace
#define ISALNUM		iswalnum

#else /* OW_I18N */

#define XV_PROC_CODE(s)         s

/*
 * General macros that should be moved to misc/i18n_impl.h
 */
#define STRSPN		strspn
#define SSCANF		sscanf
#define ISPUNCT		ispunct
#define ISSPACE		isspace
#define ISALNUM		isalnum

#endif /* OW_I18N */

AVKeyword keywordTbl[] = {
{ XV_PROC_CODE("Meta"), modifMeta },
{ XV_PROC_CODE("Shift"), modifShift },
{ XV_PROC_CODE("Alt"), modifAlt },
{ XV_PROC_CODE("Ctrl"), modifCtrl },
{ XV_PROC_CODE("Super"), modifSuper },
{ XV_PROC_CODE("Hyper"), modifHyper },
{ XV_PROC_CODE("Lock"), modifLock },
{ XV_PROC_CODE("ModeSwitch"), modifModeswitch },
{ XV_PROC_CODE("Mod1"), modifMod1 }, 
{ XV_PROC_CODE("Mod2"), modifMod2 },
{ XV_PROC_CODE("Mod3"), modifMod3 },
{ XV_PROC_CODE("Mod4"), modifMod4 },
{ XV_PROC_CODE("Mod5"), modifMod5 },
{ XV_PROC_CODE("None"), modifNone }
};

AVKeyword shortKeywordTbl[] = {
{ XV_PROC_CODE("m"), modifMeta },
{ XV_PROC_CODE("su"), modifSuper },
{ XV_PROC_CODE("s"), modifShift },
{ XV_PROC_CODE("a"), modifAlt },
{ XV_PROC_CODE("c"), modifCtrl },
{ XV_PROC_CODE("h"), modifHyper },
{ XV_PROC_CODE("l"), modifLock },
{ XV_PROC_CODE("1"), modifMod1 },
{ XV_PROC_CODE("2"), modifMod2 },
{ XV_PROC_CODE("3"), modifMod3 },
{ XV_PROC_CODE("4"), modifMod4 },
{ XV_PROC_CODE("5"), modifMod5 },
{ XV_PROC_CODE("n"), modifNone }
};


#define keywordTblEnd \
	(keywordTbl + sizeof(keywordTbl) / sizeof(AVKeyword))

#define shortKeywordTblEnd \
	(shortKeywordTbl + sizeof(shortKeywordTbl) / sizeof(AVKeyword))

/*
 * Functions to implement parsing engine
 */
static AcceleratorValue getAcceleratorValue();
static void avGetXtAcceleratorValue();
static void avGetOLITAcceleratorValue();
static void avGetXViewAcceleratorValue();
static CHAR *avAddKey();
static void avAddModif();

#define XV_KWRD_KEYPRESS        XV_PROC_CODE("<Key>")

/*
 * END of declarations for parsing engine
 */


/* ACC_XVIEW */
Xv_private int		server_parse_keystr();
#ifdef OW_I18N
Xv_private int		xv_wsncasecmp();
#else
Xv_private int		xv_strncasecmp();
#endif /* OW_I18N */
/* ACC_XVIEW */

extern XrmDatabase defaults_rdb;/* merged defaults database */


/* ACC_XVIEW */
/*
 * Parses 'keystr' and fills in:
 *	keysym		- keysym of key combination
 *	code		- keycode of key 
 *	modifiers	- e.g. ShiftMask | ControlMask
 *	qual_str	- same as modifiers, but in string form. 
 *			  If any mask in modifiers is the same as
 *			  diamond_mask, it is skipped. e.g.
 *			  For diamond_mask = Meta, the mask
 *			  MetaMask | ShiftMask | ControlMask will
 *			  return "ctrl-shift"
 *
 * XV_OK is returned for successful parsing; XV_ERROR otherwise
 *
 * Does not modify the keystr string
 */
Xv_private int
server_parse_keystr(server_public, keystr, keysym, code, modifiers, 
			diamond_mask, qual_str)
Xv_server	server_public;
CHAR		*keystr;
KeySym		*keysym;	/* return */
short		*code;		/* return */
unsigned int	*modifiers;	/* return */
unsigned int	diamond_mask;
char		*qual_str;	/* return */
{
    Server_info		*server;
    Display		*dpy;
    unsigned int	alt_modmask,
                        meta_modmask;
    int			keycode, ret_val = XV_OK, 
			shift_ksym_exist = FALSE, 
			shifted = FALSE;
    KeySym		unmod_keysym, shifted_ksym;
    CHAR		*tmp_str = NULL;
    AcceleratorValue	av;
#ifdef OW_I18N
    _xv_pswcs_t		pswcs = {0, NULL, NULL};
#endif /* OW_I18N */

    if (!server_public || !keystr || !keysym || !code || !modifiers)  {
	return(XV_ERROR);
    }

    /*
     * Get server private data and display connection handle 
     */
    server = SERVER_PRIVATE(server_public);
    dpy = server->xdisplay;

    /*
     * Get Meta, Alt masks
     */
    meta_modmask = (unsigned int)xv_get(server_public, 
                        SERVER_META_MOD_MASK);
    alt_modmask = (unsigned int)xv_get(server_public, 
                        SERVER_ALT_MOD_MASK);

    /*
     * Make duplicate of keystr - our actions here will modify strings
     */
#ifdef OW_I18N
    _xv_pswcs_wcsdup(&pswcs, keystr);
    tmp_str = pswcs.value;
#else
    tmp_str = xv_strsave(keystr);
#endif /* OW_I18N */

    /*
     * Parse string
     */
    av = getAcceleratorValue(tmp_str, defaults_rdb);

    if (av.error)  {
        if (tmp_str)  {
#ifdef OW_I18N
	    if (pswcs.storage != NULL)
	        xv_free(pswcs.storage);
#else
	    xv_free(tmp_str);
#endif /* OW_I18N */
        }
	return(XV_ERROR);
    }

    /*
     * The following strings are equivalent:
     *	Shift+a, Shift+A, A
     *	Shift+plus, Shift+equals, plus
     *
     * It is not recommended to use ShiftMask, but we try to be 
     * compatible with strings that have shift and (un)shifted 
     * keysyms in them.
     *
     * Basically:
     *	If keysym is unshifted, and ShiftMask is specified, shift the
     * 	keysym (only if a shifted keysym exists for the keycode).
     *	If keysym is already shifted and Shift is specified, remove
     *	ShiftMask.
     */

    /*
     * Check if keysym is shifted. This is done by checking if the 
     * returned keysym is in entry # 1 in the keysym list for the 
     * keycode.
     * So, first we get the keycode using XKeysymToKeycode(), then we 
     * check using XKeycodeToKeysym().
     */
    keycode = *code = XKeysymToKeycode(dpy, av.keysym);
    if (keycode)  {
	/*
	 * Get keysym of shifted keycode. This is obtained by using
	 * index 1.
	 */
        unmod_keysym = XKeycodeToKeysym(dpy, keycode, 0);
        shifted_ksym = XKeycodeToKeysym(dpy, keycode, 1);
	shift_ksym_exist = ((shifted_ksym != NoSymbol) && 
			(unmod_keysym != shifted_ksym));
	if (shift_ksym_exist)  {
	    shifted = (shifted_ksym == av.keysym);
	}
    }

    /*
     * If this is not a shifted keysym, Shift was specified
     * and a shifted keysym does exist, return the shifted
     * keysym.
     * Also, set shifted flag to TRUE
     */
    if (!shifted && av.shift && shift_ksym_exist)  {
        *keysym = shifted_ksym;
	shifted = TRUE;
    }
    else  {
        *keysym = av.keysym;
    }

    /*
     * If the keysym is already shifted, and Shift is specified,
     * remove Shift
     */
    if (shifted && av.shift)  {
	av.shift = 0;
    }

    /*
     * Set modifier masks
     */
    if (av.meta)  {
	*modifiers |= meta_modmask;
    }
    if (av.shift)  {
	*modifiers |= ShiftMask;
    }
    if (av.alt)  {
	*modifiers |= alt_modmask;
    }
    if (av.ctrl)  {
	*modifiers |= ControlMask;
    }


    /*
     * 'modifiers' now contains all the OR'd bits of the modifier masks
     * We need to return this info in string format i.e. "ctrl-alt" in
     * 'qual_str'. The modifier 'diamond_mask' is skipped.
     */
    if (!av.error && qual_str)  {
	short		empty = TRUE;

	qual_str[0] = '\0';

	/*
	 * CONTROL
	 */
        if (av.ctrl && (diamond_mask != ControlMask))  {
	    (void)strcat(qual_str, XV_MSG("ctrl"));
	    empty = FALSE;
        }

	/*
	 * SHIFT
	 * Check first if this key is 'Shift'able
	 */
        if ( ( (isascii((int)*keysym) && isalpha((int)*keysym)) || 
			(!shift_ksym_exist) ) && 
			(diamond_mask != ShiftMask) )  {
	    /*
	     * We print Shift if the keysym is a shifted one,
	     * or the modifier mask contains ShiftMask
	     */
	    if (shifted || av.shift)  {
	        if (!empty)  {
	            (void)strcat(qual_str, "-");
	        }
	        (void)strcat(qual_str, XV_MSG("shift"));
	        empty = FALSE;
	    }
        }

	/*
	 * META
	 */
        if (av.meta && (diamond_mask != meta_modmask))  {
	    if (!empty)  {
	        (void)strcat(qual_str, "-");
	    }
	    (void)strcat(qual_str, XV_MSG("meta"));
	    empty = FALSE;
        }

	/*
	 * ALT
	 */
        if (av.alt && (diamond_mask != alt_modmask))  {
	    if (!empty)  {
	        (void)strcat(qual_str, "-");
	    }
	    (void)strcat(qual_str, XV_MSG("alt"));
	    empty = FALSE;
        }
    }

    if (tmp_str)  {
#ifdef OW_I18N
        if (pswcs.storage != NULL)
            xv_free(pswcs.storage);
#else
	xv_free(tmp_str);
#endif /* OW_I18N */
    }

    return(ret_val);
}


static AcceleratorValue 
getAcceleratorValue(resourceString, db) 
CHAR		*resourceString; 
XrmDatabase	db;
{
    AcceleratorValue av;

    /* if its starts with 'coreset', look for coreset resource */
#ifdef OW_I18N
    if( !xv_wsncasecmp
#else
    if( !xv_strncasecmp
#endif /* OW_I18N */
	( resourceString, XV_PROC_CODE("coreset"), STRLEN(XV_PROC_CODE("coreset"))) ) {

	char funcname[100], resname[100];
	XrmValue value;
	char *strtype;

	*funcname = '\0';
	SSCANF( resourceString, "%*s%s", funcname );

	/*
	 * Put resource name in multibyte buffer to pass to XrmGetResource()
	 */
	sprintf( resname, "OpenWindows.MenuAccelerator.%s", funcname );
	if( False == XrmGetResource( db, resname, "*", &strtype, &value ) )
	    av.error = 1;
	else  {
#ifdef OW_I18N
	    _xv_pswcs_t     pswcs = {0, NULL, NULL};

	    /*
	     * Convert back to widechar before call parsing engine recursively
	     */
	    _xv_pswcs_mbsdup(&pswcs, (char *)value.addr);
	    av = getAcceleratorValue( pswcs.value, db );
	    if (pswcs.storage != NULL)
	        xv_free(pswcs.storage);
#else
	    av = getAcceleratorValue( value.addr, db );
#endif /* OW_I18N */
	}
	return av;
    }

    /* try the three syntaxes, until one parses resourceString */
    XV_BZERO(&av, sizeof(av));
    avGetXtAcceleratorValue( &av, resourceString );
    if( av.error || !av.keysym ) {
        XV_BZERO(&av, sizeof(av));
    	avGetOLITAcceleratorValue( &av, resourceString );
    }
    if( av.error || !av.keysym ) {
        XV_BZERO(&av, sizeof(av));
    	avGetXViewAcceleratorValue( &av, resourceString );
    }
    if( !av.keysym )
	av.error = 1;

    return av;
}

static void 
avGetXtAcceleratorValue(avp, pos) 
AcceleratorValue	*avp; 
CHAR			*pos;
{
    AVKeyword *kp;

    /* skip blanks */
    pos += STRSPN( pos, XV_PROC_CODE(" \t") );
    if( !*pos )
	return;

    /* look for one of the regular or abbrv. keywords */
    for( kp = keywordTbl ; kp < keywordTblEnd ; kp++ )
	if( !STRNCMP( kp->string, pos, STRLEN( kp->string ) ) )
	    break;
    if( kp == keywordTblEnd )
    	for( kp = shortKeywordTbl ; kp < shortKeywordTblEnd ; kp++ )
	    if( !STRNCMP( kp->string, pos, STRLEN( kp->string ) ) )
		break;
    if( kp != shortKeywordTblEnd ) {
	/* disallow modifs after keysym is known */
	if( avp->keysym ) {
	    avp->error = 1;
	    return;
	}
	avAddModif( avp, kp->modif );
	avGetXtAcceleratorValue( avp, pos + STRLEN( kp->string ) );
	return;
    }

    /* look for '<' Key '>' <key-spec> and then nothing */
    if( !STRNCMP( XV_KWRD_KEYPRESS, pos, STRLEN( XV_KWRD_KEYPRESS ) ) ) {
	pos += STRLEN( XV_KWRD_KEYPRESS );
	pos += STRSPN( pos, XV_PROC_CODE(" \t") );
	pos = avAddKey( avp, pos );
    	pos += STRSPN( pos, XV_PROC_CODE(" \t") );
	if( *pos )
		avp->error = 1;
	return;
    }

    /* an error occured */
    avp->error = 1;
    return;
}

static void 
avGetOLITAcceleratorValue(avp, pos) 
AcceleratorValue	*avp; 
CHAR			*pos;
{
    AVKeyword *kp;

    /* skip blanks */
    pos += STRSPN( pos, XV_PROC_CODE(" \t") );
    if( !*pos )
	return;

    /* look for one of the regular or abbrv. keywords */
    for( kp = keywordTbl ; kp < keywordTblEnd ; kp++ )
	if( !STRNCMP( kp->string, pos, STRLEN( kp->string ) ) )
		break;
    if( kp == keywordTblEnd )
    	for( kp = shortKeywordTbl ; kp < shortKeywordTblEnd ; kp++ )
	    if( !STRNCMP( kp->string, pos, STRLEN( kp->string ) ) )
		break;
    if( kp != shortKeywordTblEnd ) {
	/* disallow modifs after keysym is known */
	if( avp->keysym ) {
	    avp->error = 1;
	    return;
	}
	avAddModif( avp, kp->modif );
	avGetOLITAcceleratorValue( avp, pos + STRLEN( kp->string ) );
	return;
    }

    /* look for '<' key '>' and then nothing */
    if ( *pos == XV_PROC_CODE('<') ) {
	pos = avAddKey( avp, pos + 1 );
    	if( avp->error ) return;
    	pos += STRSPN( pos, XV_PROC_CODE(" \t") );
	if( *pos != XV_PROC_CODE('>') )
	    avp->error = 1;
	else {
    	    pos += 1 + STRSPN( pos+1, XV_PROC_CODE(" \t") );
	    if( *pos )
		avp->error = 1;
	} 
	return;
    }

    /* an error occured */
    avp->error = 1;
    return;
}

static void 
avGetXViewAcceleratorValue(avp, pos) 
AcceleratorValue	*avp; 
CHAR			*pos;
{
    AVKeyword *kp;

    /* skip blanks */
    pos += STRSPN( pos, XV_PROC_CODE(" \t") );
    if( !*pos )
	return;

    /* if mods or keysyms already found, look for '+' */
    if( avp->keysym || avp->some || avp->none )
	if( *pos != XV_PROC_CODE('+') ) {
	    avp->error = 1;
	    return;
	} else
	    pos += 1 + STRSPN( pos+1, XV_PROC_CODE(" \t") );

    /* look for one of the regular keywords */
    for( kp = keywordTbl ; kp < keywordTblEnd ; kp++ )
	if( !STRNCMP( kp->string, pos, STRLEN( kp->string ) ) )
		break;

    if( kp != keywordTblEnd ) {
	avAddModif( avp, kp->modif );
	avGetXViewAcceleratorValue( avp, pos + STRLEN( kp->string ) );
	return;
    } 

    /* if no keysym name found yet, look for keysym */
    if( avp->keysym )
	avp->error = 1;
    else {
	pos = avAddKey( avp, pos );
	if( !avp->error )
	    avGetXViewAcceleratorValue( avp, pos );
    }

    return;
}


static void 
avAddModif(avp, modif) 
AcceleratorValue 	*avp; 
AVModif			modif;
{
    if( modif == modifNone )
	avp->none = 1;
    else {
   	avp->some = 1; 
    	switch( modif ) {
	case modifMeta:	avp->meta = 1;	break;
	case modifShift:avp->shift = 1;	break;
	case modifAlt:	avp->alt = 1;	break;
	case modifCtrl:	avp->ctrl = 1;	break;
	case modifSuper:avp->super = 1;	break;
	case modifHyper:avp->hyper = 1;	break;
	case modifLock:	avp->lock = 1;	break;
	case modifModeswitch:	avp->modeswitch = 1;	break;
	case modifMod1:	avp->mod1 = 1;	break;
	case modifMod2:	avp->mod2 = 1;	break;
	case modifMod3:	avp->mod3 = 1;	break;
	case modifMod4:	avp->mod4 = 1;	break;
	case modifMod5:	avp->mod5 = 1;	break;
    	}
    }

    if( avp->none && avp->some )
	avp->error = 1;
}


static CHAR *
avAddKey(avp, pos) 
AcceleratorValue	*avp; 
CHAR			*pos;
{
    CHAR *sp, *dp, strbuf[100];
#ifdef OW_I18N
    char	strbuf_mb[100];
#endif /* OW_I18N */

    /* if keysym already set, that's an error */
    if( avp->keysym ) {
	avp->error = 1;
	return -1;
    }

    /* look for 'raw' space or punctuation */
    if( ISPUNCT( *pos ) || ISSPACE( *pos )) { 
	avp->keysym = *pos;
	pos++;
    } else {				/* look for valid keysym name */
	for( sp = pos, dp = strbuf ; 
	     dp < strbuf + sizeof( strbuf ) && (ISALNUM( *sp ) || *sp == XV_PROC_CODE('_')) ; 
	     *dp++ = *sp++ );
	*dp = XV_PROC_CODE('\0');
#ifdef OW_I18N
	/*
	 * Convert to multibyte to pass to XStringToKeysym()
	 */
	sprintf(strbuf_mb, "%ws", strbuf);
	if( avp->keysym = XStringToKeysym( strbuf_mb ) )
#else
	if( avp->keysym = XStringToKeysym( strbuf ) )
#endif /* OW_I18N */
	    pos = sp;
        else
	    avp->error = 1;		/* nothing parses as a key */
    }

    return pos;
}

/* ACC_XVIEW */
