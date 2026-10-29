#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)txt_again.c 20.43 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * AGAIN action recorder and interpreter for text subwindows.
 */
#include <xview/pkg.h>
#include <xview/attrol.h>
#include <xview_private/primal.h>
#include <xview_private/txt_impl.h>
#include <xview_private/ev_impl.h>
#include <xview_private/txt_18impl.h>
#ifdef SVR4
#include <stdlib.h>
#include <string.h>
#endif /* SVR4 */

/* Pkg_private CHAR    *STRNCPY(); */
Pkg_private Es_index textsw_do_input();
Pkg_private Es_index textsw_do_pending_delete();

string_t        null_string = {0, 0, 0};

#define	TEXT_DELIMITER	"\\"
char           *text_delimiter = TEXT_DELIMITER;

#ifdef OW_I18N
CHAR            text_delimiter_wc[2] = {'\\','\0'};
#endif

typedef enum {
    CARET_TOKEN,
    DELETE_TOKEN,
    EDIT_TOKEN,
    EXTRAS_TOKEN,
    FIELD_TOKEN,
    FILTER_TOKEN,
    FIND_TOKEN,
    INSERT_TOKEN,
    MENU_TOKEN,
    NULL_TOKEN
}               Token;

char           *cmd_tokens[] = {
    "CARET", "DELETE", "EDIT", "EXTRAS", "FIELD", "FILTER", "FIND", "INSERT", "MENU", NULL
};

typedef enum {
    FORWARD_DIR,
    BACKWARD_DIR,
    NO_DIR,
    EMPTY_DIR
}               Direction;

char           *direction_tokens[] = {
    "FORWARD", "BACKWARD", TEXT_DELIMITER, "", NULL
};
char           *edit_tokens[] = {
    "CHAR", "WORD", "LINE", NULL
};

#define	CHAR_TOKEN	0
#define WORD_TOKEN	1
#define LINE_TOKEN	2

char           *text_tokens[] = {
    "PIECES", "TRASHBIN", TEXT_DELIMITER, NULL
};

#define	PIECES_TOKEN	0
#define TRASH_TOKEN	1
#define DELIMITER_TOKEN	2

#ifdef OW_I18N
#define		    MB_STR_BUFFER_LEN		100
static char	    mb_str[MB_STR_BUFFER_LEN];
#endif

static int
  textsw_string_min_free(register string_t *ptr_to_string,
                         int min_free_desired);

Pkg_private int
match_in_table(to_match, table)	/* Modified from ucb/lpr/lpc.c */
    register char  *to_match;
    register char **table;
{
    register char  *p, *q;
    int             found, index, nmatches, longest;

    longest = nmatches = 0;
    found = index = -1;
    for (p = *table; p; p = *(++table)) {
	index++;
	for (q = to_match; *q == *p++; q++)
	    if (*q == 0)	/* exact match? */
		return (index);
	if (!*q) {		/* the to_match was a prefix */
	    if (q - to_match > longest) {
		longest = q - to_match;
		nmatches = 1;
		found = index;
	    } else if (q - to_match == longest)
		nmatches++;
	}
    }
    if (nmatches > 1)
	return (-1);
    return (found);
}

static	int
textsw_string_append(ptr_to_string, buffer, buffer_length)
    string_t       *ptr_to_string;
    CHAR           *buffer;
    int             buffer_length;
/* Returns FALSE iff it needed to malloc and the malloc failed */
{
    if (textsw_string_min_free(ptr_to_string, buffer_length) != TRUE)
	return (FALSE);
    BCOPY(buffer, ptr_to_string->free, buffer_length);
    ptr_to_string->free += buffer_length;
    *(ptr_to_string->free) = '\0';
    return (TRUE);
}

int             textsw_again_debug;	/* = 0 for -A-R */

static int
  textsw_string_min_free(register string_t *ptr_to_string, int min_free_desired)
/* Returns FALSE iff it needed to malloc and the malloc failed */
{
    int             used = TXTSW_STRING_LENGTH(ptr_to_string);
#define		    STR_INC_SIZE	1024   /* string increment size */    
    register int    desired_max = ((((used + min_free_desired) / STR_INC_SIZE) + 1) * STR_INC_SIZE);
#undef	STR_INC_SIZE    
    /* BEAR TRAP */
    if (ptr_to_string->max_length <
	(ptr_to_string->free - ptr_to_string->base)) {
	while (!textsw_again_debug) {
	}
    }
    if (ptr_to_string->max_length < desired_max) {
     /* 
      * string will be incremented by STR_INC_SIZE if necessary.  
      * This prevent memory fragmentation from small block malloc and free.
      */
	CHAR           *old_string = ptr_to_string->base;
	if (old_string == NULL)
	    ptr_to_string->base = MALLOC(
				 (unsigned) desired_max + 1);
	else			 
	    ptr_to_string->base = realloc(old_string,
				 (unsigned) (desired_max + 1) * sizeof(CHAR));
				 
	if (ptr_to_string->base == NULL) {
	    ptr_to_string->base = old_string;
	    return (FALSE);
	}
	ptr_to_string->max_length = desired_max;
	if (old_string == NULL) {
	    ptr_to_string->free = ptr_to_string->base;
	    *(ptr_to_string->free) = '\0';
	} else
	    ptr_to_string->free = ptr_to_string->base + used;
    } 
    return (TRUE);
}

/*
 * Recording routines
 */

#if defined(__linux) || defined(__APPLE__)
/* The textsw_printf as written created a FILE struct, and called
 * _doprnt() to format the text to an XView internal buffer. If I
 * understand this correctly, it's just a replacement for vsprintf.
 * Anyway, linux FILE isn't similar to the one assumed by the
 * original code, so we'll have to do something.
 * WARNING! This is untested!! */

static int
#ifdef ANSI_FUNC_PROTO
textsw_printf(register string_t *ptr_to_string, register char  *fmt, ...)
#else
textsw_printf(ptr_to_string, fmt, va_alist)
    register string_t *ptr_to_string;
    register char  *fmt;
va_dcl
#endif
{
    va_list         args;
    char *          sfree;
    int             result;

    VA_START(args, fmt);
    sfree = (char *)TXTSW_STRING_FREE(ptr_to_string);
    result = vsprintf(sfree, fmt, args);
    va_end(args);
    TXTSW_STRING_FREE(ptr_to_string) += strlen(sfree);
    return (result);
}
#else /* __linux */
/*
 * Following is stolen from 3.2ALPHA sprintf(str, fmt, va_alist) SIDE_EFFECT:
 * TXTSW_STRING_FREE(ptr_to_string) is modified by this routine.
 */
#ifndef SVR4

/* VARARGS2 */

static int
#ifdef ANSI_FUNC_PROTO
textsw_printf(register string_t *ptr_to_string, register char  *fmt, ...)
#else
textsw_printf(ptr_to_string, fmt, va_alist)
    register string_t *ptr_to_string;
    register char  *fmt;
va_dcl
#endif
{
#ifndef OW_I18N

    FILE            _strbuf;
    int             result;
    va_list         args;

    _strbuf._flag = _IOWRT | _IOSTRG;
#ifdef sun
    _strbuf._base = (unsigned char *) TXTSW_STRING_FREE(ptr_to_string);
#else
    _strbuf._base = (char *) TXTSW_STRING_FREE(ptr_to_string);
#endif
    _strbuf._ptr = _strbuf._base;
    _strbuf._cnt = ptr_to_string->max_length -
	TXTSW_STRING_LENGTH(ptr_to_string);
    VA_START(args, fmt);
    result = _doprnt(fmt, args, &_strbuf);
    va_end(args);
    TXTSW_STRING_FREE(ptr_to_string) = (char *) _strbuf._ptr;
#ifndef lint
    if (result >= 0)
	putc('\0', &_strbuf);
#endif
    return (result);

#else /* OW_I18N */

/*
 * OW_I18N:  All the string in va_alist should be in multibyte and
 * textsw_printf() will convert it to wchar_t and write it into ptr_to_string.
 */
    FILE            _strbuf;
    int             result;
    va_list         args;
    int             num_char;

    _strbuf._flag = _IOWRT | _IOSTRG;
    mb_str[0] = NULL;
#ifdef sun
    _strbuf._base = (unsigned char *) mb_str;
#else
    _strbuf._base = (char *) mb_str;
#endif
    _strbuf._ptr = _strbuf._base;
    _strbuf._cnt = MB_STR_BUFFER_LEN;
    VA_START(args, fmt);
    result = _doprnt(fmt, args, &_strbuf);
    va_end(args);
#ifndef lint
    if (result >= 0)
	putc('\0', &_strbuf);
#endif
    num_char = mbstowcs(TXTSW_STRING_FREE(ptr_to_string), mb_str,
	(ptr_to_string->max_length - TXTSW_STRING_LENGTH(ptr_to_string)));
    TXTSW_STRING_FREE(ptr_to_string) += num_char;
    return (result);

#endif /* OW_I18N */
}

#else /* SVR4 */

static int
#ifdef ANSI_FUNC_PROTO
textsw_printf(register string_t *ptr_to_string, register char  *fmt, ...)
#else
textsw_printf(ptr_to_string, fmt, va_alist)
    register string_t *ptr_to_string;
    register char  *fmt;
va_dcl
#endif
{
#ifndef OW_I18N

    FILE            _strbuf;
    int             result;
    va_list         args;

    _strbuf._flag = _IOREAD; /* distinguish dummy file descriptor */
    _strbuf._base = (unsigned char *) TXTSW_STRING_FREE(ptr_to_string);
    _strbuf._ptr = _strbuf._base;
    _strbuf._cnt = ptr_to_string->max_length -
        TXTSW_STRING_LENGTH(ptr_to_string);
    _strbuf._file = _NFILE;
    VA_START(args, fmt);
    result = _doprnt(fmt, args, &_strbuf);
    va_end(args);
    TXTSW_STRING_FREE(ptr_to_string) = (char *) _strbuf._ptr;
#ifndef lint
    if (result >= 0)           
        putc('\0', &_strbuf);
#endif
    return (result);

#else /* OW_I18N */

/*
 * OW_I18N:  All the string in va_alist should be in multibyte and
 * textsw_printf() will convert it to wchar_t and write it into ptr_to_string.
 */
    FILE            _strbuf;
    int             result;
    va_list         args;
    int             num_char;

    mb_str[0] = NULL;

    _strbuf._flag = _IOREAD; /* distinguish dummy file descriptor */
    _strbuf._base = (unsigned char *) mb_str;
    _strbuf._ptr = _strbuf._base;
    _strbuf._cnt = MB_STR_BUFFER_LEN;
    _strbuf._file = _NFILE;
    VA_START(args, fmt);
    result = _doprnt(fmt, args, &_strbuf);
    va_end(args);
#ifndef lint
    if (result >= 0)           
        putc('\0', &_strbuf);
#endif
    num_char = mbstowcs(TXTSW_STRING_FREE(ptr_to_string), mb_str,
	(ptr_to_string->max_length - TXTSW_STRING_LENGTH(ptr_to_string)));
    TXTSW_STRING_FREE(ptr_to_string) += num_char;
    return (result);

#endif /* OW_I18N */
}     

#endif /* SVR4 */
#endif /* __linux */

static	void
textsw_record_buf(again, buffer, buffer_length)
    register string_t *again;
    CHAR           *buffer;
    int             buffer_length;
{
    (void) textsw_printf(again, "%s %6d %s\n",
			 text_delimiter, buffer_length, text_delimiter);
    (void) textsw_string_append(again, buffer, buffer_length);
    (void) textsw_printf(again, "\n%s\n", text_delimiter);
}

Pkg_private void
textsw_record_caret_motion(textsw, direction, loc_x)
    Textsw_folio    textsw;
    unsigned        direction;
    int             loc_x;
{
    register string_t *again = &textsw->again[0];

    if ((textsw->func_state & TXTSW_FUNC_AGAIN) ||
                    (textsw->state & TXTSW_NO_AGAIN_RECORDING))
        return;

    textsw->again_insert_length = 0;
    if (textsw_string_min_free(again, 15) != TRUE)
	return;			/* Cannot guarantee enough space */

    (void) textsw_printf(again, "%s %x %d\n", cmd_tokens[ord(CARET_TOKEN)],
			 direction, loc_x);
}

Pkg_private void
textsw_record_delete(textsw)
    Textsw_folio    textsw;
{
    register string_t *again = &textsw->again[0];

    if ((textsw->func_state & TXTSW_FUNC_AGAIN) ||
                    (textsw->state & TXTSW_NO_AGAIN_RECORDING))
	return;
    textsw->again_insert_length = 0;
    if (textsw_string_min_free(again, 10) != TRUE)
	return;			/* Cannot guarantee enough space */
    (void) textsw_printf(again, "%s\n", cmd_tokens[ord(DELETE_TOKEN)]);
}

Pkg_private void
textsw_record_edit(textsw, unit, direction)
    Textsw_folio    textsw;
    unsigned        unit, direction;
{
    register string_t *again = &textsw->again[0];

    if ((textsw->func_state & TXTSW_FUNC_AGAIN) ||
                        (textsw->state & TXTSW_NO_AGAIN_RECORDING))
	return;
    textsw->again_insert_length = 0;
    if (textsw_string_min_free(again, 25) != TRUE)
	return;			/* Cannot guarantee enough space */
    (void) textsw_printf(again, "%s %s %s\n", cmd_tokens[ord(EDIT_TOKEN)],
			 edit_tokens[(unit == EV_EDIT_CHAR) ? CHAR_TOKEN
				     : (unit == EV_EDIT_WORD) ? WORD_TOKEN
				     : LINE_TOKEN],
			 direction_tokens[(direction == 0)
					  ? ord(FORWARD_DIR)
					  : ord(BACKWARD_DIR)]);
}

Pkg_private void
textsw_record_extras(folio, cmd_line)
    Textsw_folio    folio;
    CHAR           *cmd_line;
{
    register string_t *again = &folio->again[0];
    int             cmd_len = (cmd_line ? STRLEN(cmd_line) : 0);

    if ((folio->func_state & TXTSW_FUNC_AGAIN) ||
                        (folio->state & TXTSW_NO_AGAIN_RECORDING))
	return;

    folio->again_insert_length = 0;

    if (textsw_string_min_free(again, cmd_len + 30) != TRUE)
	return;			/* Cannot guarantee enough space */

    (void) textsw_printf(again, "%s ", cmd_tokens[ord(EXTRAS_TOKEN)]);

    textsw_record_buf(again, cmd_line, cmd_len);
}

Pkg_private void
textsw_record_find(textsw, pattern, pattern_length, direction)
    Textsw_folio    textsw;
    CHAR           *pattern;
    int             pattern_length, direction;
{
    register string_t *again = &textsw->again[0];

    if ((textsw->func_state & TXTSW_FUNC_AGAIN) ||
                        (textsw->state & TXTSW_NO_AGAIN_RECORDING))
	return;
    if (textsw->state & (TXTSW_AGAIN_HAS_FIND | TXTSW_AGAIN_HAS_MATCH)) {
	(void) textsw_checkpoint_again(
				       VIEW_REP_TO_ABS(textsw->first_view));
    } else {
	textsw->again_insert_length = 0;
    }
    if (textsw_string_min_free(again, pattern_length + 30) != TRUE)
	return;			/* Cannot guarantee enough space */
    (void) textsw_printf(again, "%s %s ", cmd_tokens[ord(FIND_TOKEN)],
			 direction_tokens[(direction == 0)
					  ? ord(FORWARD_DIR)
					  : ord(BACKWARD_DIR)]);
    textsw_record_buf(again, pattern, pattern_length);
    textsw->state |= TXTSW_AGAIN_HAS_FIND;
}

Pkg_private void
textsw_record_filter(textsw, event)
    Textsw_folio    textsw;
    Event          *event;
{
    register string_t *again = &textsw->again[0];

    if ((textsw->func_state & TXTSW_FUNC_AGAIN) ||
                        (textsw->state & TXTSW_NO_AGAIN_RECORDING))
	return;
    textsw->again_insert_length = 0;
    if (textsw_string_min_free(again, 50) != TRUE)
	return;			/* Cannot guarantee enough space */
    (void) textsw_printf(again, "%s %x %x %x ",
			 cmd_tokens[ord(FILTER_TOKEN)],
			 event_action(event),
			 event->ie_flags, event->ie_shiftmask);
    textsw_record_buf(again, textsw->to_insert,
		      textsw->to_insert_next_free - textsw->to_insert);
}

#ifdef OW_I18N	    
static int
my_wstoi (num_string)
    CHAR	*num_string;
{
    int		i = 0;
    int		result = 0;
    
    while (((num_string + i) != NULL) &&
           (*(num_string + i) == L' '))
           i++;
           
    while (((num_string + i) != NULL) &&
           (*(num_string + i) >= L'0')  &&
           (*(num_string + i) <= L'9')) {
           result = (result * 10) + (*(num_string + i) - L'0');
           i++;
    }
    return(result);     
}
#endif /* OW_I18N */

Pkg_private void
textsw_record_input(textsw, buffer, buffer_length)
    Textsw_folio    textsw;
    CHAR           *buffer;
    long int        buffer_length;
{
    register string_t *again = &textsw->again[0];

    if ((textsw->func_state & TXTSW_FUNC_AGAIN) ||
                (textsw->state & TXTSW_NO_AGAIN_RECORDING))
	return;
    if (textsw_string_min_free(again, buffer_length + 25) != TRUE)
	return;			/* Cannot guarantee enough space */
    /* Above guarantees enough space */
    if (textsw->again_insert_length == 0) {
	(void) textsw_printf(again, "%s ", cmd_tokens[ord(INSERT_TOKEN)]);
	textsw->again_insert_length =
#ifdef OW_I18N
	    TXTSW_STRING_LENGTH(again) + STRLEN(text_delimiter_wc) + 1;
#else
	    TXTSW_STRING_LENGTH(again) + strlen(text_delimiter) + 1;
#endif
	textsw_record_buf(again, buffer, buffer_length);
    } else {
	/*
	 * Following is a disgusting efficiency hack to compress a sequence
	 * of INSERTs.
	 */
	CHAR           *insert_length, new_length_buf[7];
	int             i, old_length;
	insert_length = TXTSW_STRING_BASE(again) +
	    textsw->again_insert_length;
#ifdef OW_I18N	    
	/*  Should use wstoi() when its performance problem is fixed */
	old_length = my_wstoi(insert_length);
#else /* OW_I18N */
	old_length = atoi(insert_length);
#endif /* OW_I18N */	
	ASSUME(old_length > 0);
	(void) SPRINTF(new_length_buf, "%6d", old_length + buffer_length);
	for (i = 0; i < 6; i++) {
	    insert_length[i] = new_length_buf[i];
	}
#ifdef OW_I18N
	TXTSW_STRING_FREE(again) -= STRLEN(text_delimiter_wc) + 2;
#else
	TXTSW_STRING_FREE(again) -= strlen(text_delimiter) + 2;
#endif
	(void) textsw_string_append(again, buffer, buffer_length);
	(void) textsw_printf(again, "\n%s\n", text_delimiter);
    }
}

Pkg_private void
textsw_record_match(textsw, flag, start_marker)
    Textsw_folio    textsw;
    unsigned        flag;
    CHAR           *start_marker;
{
    register string_t *again = &textsw->again[0];
#ifdef OW_I18N
    char	   *start_marker_mb;
#endif

    if ((textsw->func_state & TXTSW_FUNC_AGAIN) ||
                (textsw->state & TXTSW_NO_AGAIN_RECORDING))
	return;
    if (textsw->state & TXTSW_AGAIN_HAS_MATCH) {
	(void) textsw_checkpoint_again(
				       VIEW_REP_TO_ABS(textsw->first_view));
    } else {
	textsw->again_insert_length = 0;
    }
    if (textsw_string_min_free(again, 15) != TRUE)
	return;			/* Cannot guarantee enough space */

#ifdef OW_I18N
    start_marker_mb = _xv_wcstombsdup(start_marker);
    (void) textsw_printf(again, "%s %x %s\n", cmd_tokens[ord(FIELD_TOKEN)], flag, start_marker_mb);
    if(start_marker_mb)
	free(start_marker_mb);
#else
    (void) textsw_printf(again, "%s %x %s\n", cmd_tokens[ord(FIELD_TOKEN)], flag, start_marker);
#endif
    textsw->state |= TXTSW_AGAIN_HAS_MATCH;
}


Pkg_private void
textsw_record_piece_insert(textsw, pieces)
    Textsw_folio    textsw;
    Es_handle       pieces;
{
    register string_t *again = &textsw->again[0];

    if ((textsw->func_state & TXTSW_FUNC_AGAIN) ||
                        (textsw->state & TXTSW_NO_AGAIN_RECORDING))
	return;
    textsw->again_insert_length = 0;
    if (textsw_string_min_free(again, 25) != TRUE)
	return;			/* Cannot guarantee enough space */
    (void) textsw_printf(again, "%s %s %d\n",
			 cmd_tokens[ord(INSERT_TOKEN)],
			 text_tokens[PIECES_TOKEN], pieces);
}

Pkg_private void
textsw_record_trash_insert(textsw)
    Textsw_folio    textsw;
{
    register string_t *again = &textsw->again[0];

    if ((textsw->func_state & TXTSW_FUNC_AGAIN) ||
                (textsw->state & TXTSW_NO_AGAIN_RECORDING))
	return;
    textsw->again_insert_length = 0;
    if (textsw_string_min_free(again, 20) != TRUE)
	return;			/* Cannot guarantee enough space */
    (void) textsw_printf(again, "%s %s\n",
			 cmd_tokens[ord(INSERT_TOKEN)],
			 text_tokens[TRASH_TOKEN]);
}

/*
 * Replaying routines
 */

#if defined(__linux) || defined(__APPLE__)
/* The textsw_scanf as written created a FILE struct, and called
 * _doscan() to do the scanning on the instring.
 * This depends heavily on the internal structure of FILE, and is not
 * applicable to linux. Instead I use a quickly hacked together
 * scanf. The normal scanf isn't used because we need to get a
 * pointer to where scanning of the instring stopped. I know no way
 * to get that from sscanf(). */

static int
#ifdef ANSI_FUNC_PROTO
textsw_scanf(register string_t *ptr_to_string, register char  *fmt, ...)
#else
textsw_scanf(ptr_to_string, fmt, va_alist)
    register string_t *ptr_to_string;
    register char  *fmt;
va_dcl
#endif
{
    va_list         args;
    char *          sbase;
    int             result;
    char *f, *sp;
    int c, *ip;

    VA_START(args, fmt);
    sbase = (char *)TXTSW_STRING_BASE(ptr_to_string);

/* scanf loop, supports %s, %15s, %d, %x because those are the types used
 * by routines calling this function (textsw_scanf is static). */ 
    result = 0;
    for (f = fmt; *f; ++f) {
      switch (*f) {
      case '%':
        ++f;
        c = 999;              
        if (isdigit(*f)) {
          c = atoi(f);
          while (isdigit(*f))
            ++f;
        }
        for (; isspace(*sbase); ++sbase)
          ;
        switch (*f) {
        case 's':
          sp = va_arg(args, char *);
          for (; *sbase && !isspace(*sbase) && c; ++sbase, --c) 
            *sp++ = *sbase;
          *sp = '\0';
          ++result;
          break;
        case 'd':
          ip = va_arg(args, int *);
          if (isdigit(*sbase)) {
            ++result;
            *ip = atoi(sbase);
            while (isdigit(*sbase))
              ++sbase;
          }
          break;
        case 'x':
          ip = va_arg(args, int *);
          result += sscanf(sbase, "%x", ip);
          while (isxdigit(*sbase))
            ++sbase;
          break;
        default:
          break;
        }
        break;
      default:
        break;
      }
    }
    va_end(args);
    TXTSW_STRING_BASE(ptr_to_string) = sbase;
    return (result);
}
#else /* __linux */
/*
 * Following is stolen from sscanf(str, fmt, args) SIDE_EFFECT:
 * TXTSW_STRING_BASE(ptr_to_string) is modified by this routine.
 */
/* VARARGS2 */

static int
#ifdef ANSI_FUNC_PROTO
textsw_scanf(register string_t *ptr_to_string, register char  *fmt, ...)
#else
textsw_scanf(ptr_to_string, fmt, va_alist)
    register string_t *ptr_to_string;
    register char  *fmt;
va_dcl
#endif
{
#ifndef OW_I18N

    FILE            _strbuf;
    int             result;
    va_list         args;

#ifdef SVR4
/*    _strbuf._flag = _IOWRT; */
/*    clever way to insure no physread, but not POSIX compliant */
/*    Posix won't allow doscan on IOWRT streams. Make it 0 instead */
    _strbuf._flag = _IOREAD | _IOWRT;
    _strbuf._base = (unsigned char *)TXTSW_STRING_BASE(ptr_to_string);
    _strbuf._cnt = strlen((char *)ptr_to_string);
    _strbuf._file = _NFILE;
#else /* SVR4 */
    _strbuf._flag = _IOREAD | _IOSTRG; 
#ifdef sun
    _strbuf._base = (unsigned char *) TXTSW_STRING_BASE(ptr_to_string);
#else /* sun */
    _strbuf._base = (char *) TXTSW_STRING_BASE(ptr_to_string);
#endif /* sun */
    _strbuf._bufsiz = _strbuf._cnt = TXTSW_STRING_LENGTH(ptr_to_string);
#endif /* SVR4 */
    _strbuf._ptr = _strbuf._base;
    VA_START(args, fmt);
    result = _doscan(&_strbuf, fmt, args);
    va_end(args);
    TXTSW_STRING_BASE(ptr_to_string) = (char *) _strbuf._ptr;
    return (result);

#else /* OW_I18N */

/*
 *  OW_I18N:  The string within ptr_to_string is wchar_t,
 *  but the string in va_alist should be multibytes
 */
    FILE            _strbuf;
    int             result;
    va_list         args;
    int             num_char;
    CHAR            dummy[MB_STR_BUFFER_LEN];

    mb_str[0] = NULL;
    (void) wcstombs(mb_str, TXTSW_STRING_BASE(ptr_to_string), MB_STR_BUFFER_LEN);

#ifdef sun
    _strbuf._base = (unsigned char *) mb_str;
#else
    _strbuf._base = (char *) mb_str;
#endif
#ifdef SVR4
/*    _strbuf._flag = _IOWRT; */
/*    clever way to insure no physread, but not POSIX compliant */
/*    Posix won't allow doscan on IOWRT streams. Make it 0 instead */
    _strbuf._flag = _IOREAD | _IOWRT;
    _strbuf._base = (unsigned char *) mb_str;
#else /* SVR4 */
    _strbuf._flag = _IOREAD | _IOSTRG;
#endif /* SVR4 */
    _strbuf._ptr = _strbuf._base;
#ifndef SVR4
    _strbuf._bufsiz = _strbuf._cnt = strlen(mb_str);
#else /* SVR4 */
    _strbuf._cnt = strlen(mb_str);
    _strbuf._file = _NFILE;
#endif /* SVR4 */
    VA_START(args, fmt);
    result = _doscan(&_strbuf, fmt, args);
    va_end(args);
    mb_str[(_strbuf._ptr - _strbuf._base)] = NULL;
    num_char = mbstowcs(dummy, mb_str, MB_STR_BUFFER_LEN);
    TXTSW_STRING_BASE(ptr_to_string) += num_char;
    return (result);

#endif /* OW_I18N */
}
#endif /* __linux */

static int
textsw_next_is_delimiter(again)
    string_t       *again;
{
    char            token[2];
    int             count;

    count = textsw_scanf(again, "%1s", token);
    if AN_ERROR
	(count != 1 || token[0] != text_delimiter[0]) {
	return (FALSE);
    } else
	return (TRUE);
}

static          Es_handle
textsw_pieces_for_replay(again)
    register string_t *again;
{
#define CHECK_ERROR(test)	if AN_ERROR(test) goto Again_Error;
    int             count;
    Es_handle       pieces = (Es_handle) NULL;

    count = textsw_scanf(again, "%d", &pieces);
    CHECK_ERROR(count != 1 || pieces == (Es_handle) NULL);
    CHECK_ERROR(*TXTSW_STRING_BASE(again)++ != '\n');
Again_Error:
    return (pieces);
#undef CHECK_ERROR
}

static int
textsw_text_for_replay(again, ptr_to_buffer)
    register string_t *again;
    register CHAR **ptr_to_buffer;
{
#define CHECK_ERROR(test)	if AN_ERROR(test) goto Again_Error;
    int             count, length = -1;

    count = textsw_scanf(again, "%d", &length);
    CHECK_ERROR(count != 1 || length < 0);
    CHECK_ERROR(textsw_next_is_delimiter(again) == 0);
    CHECK_ERROR(*TXTSW_STRING_BASE(again)++ != '\n');
    if (length) {
	*ptr_to_buffer = MALLOC(((unsigned) length + 1) *
				(sizeof(**ptr_to_buffer)));
	(void) STRNCPY(*ptr_to_buffer,
		       TXTSW_STRING_BASE(again), length);
	TXTSW_STRING_BASE(again) += length;
    } else {
	*ptr_to_buffer = NULL;
    }
    CHECK_ERROR(*TXTSW_STRING_BASE(again)++ != '\n');
    CHECK_ERROR(*TXTSW_STRING_BASE(again)++ != text_delimiter[0]);
    CHECK_ERROR(*TXTSW_STRING_BASE(again)++ != '\n');
Again_Error:
    return (length);
#undef CHECK_ERROR
}

static CHAR    *
token_index(string, token)
    CHAR           *string;
    register CHAR  *token;
{
    register CHAR  *result = string, *r, *t;

    if (result == NULL || token == NULL)
	goto NoMatch;
    for (; *result; result++) {
	if (*result == *token) {
	    r = result + 1;
	    for (t = token + 1; *t; r++, t++) {
		if (*r != *t) {
		    if (*r == 0)
			goto NoMatch;
		    else
			break;
		}
	    }
	    if (*t == 0)
		return (result);
	}
    }
NoMatch:
    return (NULL);
}

/* ARGSUSED */
Pkg_private void
textsw_free_again(textsw, again)
    Textsw_folio    textsw;	/* Currently unused */
    register string_t *again;
{
    CHAR           *saved_base = TXTSW_STRING_BASE(again);
    Es_handle       pieces;
#ifdef OW_I18N
    CHAR           text_tokens_wc[20];
#endif

    if (TXTSW_STRING_IS_NULL(again))
	return;
    ASSERT(allock());
#ifdef OW_I18N
    (void) mbstowcs(text_tokens_wc, text_tokens[PIECES_TOKEN], 20);
    while ((TXTSW_STRING_BASE(again) = token_index(
		       TXTSW_STRING_BASE(again), text_tokens_wc))
	   != NULL) {
	TXTSW_STRING_BASE(again) += STRLEN(text_tokens_wc);
	if (pieces = textsw_pieces_for_replay(again))
	    es_destroy(pieces);
    }
#else
    while ((TXTSW_STRING_BASE(again) = token_index(
		       TXTSW_STRING_BASE(again), text_tokens[PIECES_TOKEN]))
	   != NULL) {
	TXTSW_STRING_BASE(again) += strlen(text_tokens[PIECES_TOKEN]);
	if (pieces = textsw_pieces_for_replay(again))
	    es_destroy(pieces);
    }
#endif /* OW_I18N */
    free(saved_base);
    ASSERT(allock());
    *again = null_string;
}

Pkg_private int
textsw_get_recorded_x(view)
    register Textsw_view_handle view;
{
#define CHECK_ERROR(test)	if AN_ERROR(test) goto Again_Error;
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);
    register string_t *again;
    CHAR           *buffer = NULL, *saved_base;
    char            token_buf[20];
    register char  *token = token_buf;
    register int    count;
    Textsw_Caret_Direction direction;
    int             loc_x, result = -1;
    int             found_it_already = FALSE;

    if (!TXTSW_DO_AGAIN(folio))
	return -1;
    again = &folio->again[0];
    if (TXTSW_STRING_IS_NULL(again)) {
	return (result);
    }
    saved_base = TXTSW_STRING_BASE(again);
    ev_set(view->e_view, EV_CHAIN_DELAY_UPDATE, TRUE, NULL);
    FOREVER {
	count = textsw_scanf(again, "%19s", token);
	if (count == -1)
	    break;
	ASSERT(count == 1);
	count = match_in_table(token, cmd_tokens);
	CHECK_ERROR(count < 0);
	if (buffer != NULL) {
	    free(buffer);
	    buffer = NULL;
	}
	if ((Token) count == CARET_TOKEN) {
	    count = textsw_scanf(again, "%x %d", &direction, &loc_x);
	    CHECK_ERROR(count != 2);

	    if ((direction == TXTSW_NEXT_LINE) ||
		(direction == TXTSW_PREVIOUS_LINE)) {
		if (!found_it_already) {
		    result = loc_x;
		    found_it_already = TRUE;
		}
	    } else {
		if (found_it_already) {
		    /*
		     * Any other direction following TXTSW_NEXT_LINE or
		     * TXTSW_PREVIOUS_LINE will clear the loc_x value
		     */
		    result = -1;
		    found_it_already = FALSE;
		}
	    }
	} else if (found_it_already) {
	    /*
	     * Any other event following TXTSW_NEXT_LINE or
	     * TXTSW_PREVIOUS_LINE will clear the loc_x value
	     */
	    result = -1;
	    found_it_already = FALSE;
	}
    }
Again_Error:
    if (buffer != NULL)
	free(buffer);
    ev_set(view->e_view, EV_CHAIN_DELAY_UPDATE, FALSE, NULL);
    ev_update_chain_display(folio->views);
    TXTSW_STRING_BASE(again) = saved_base;
    ASSERT(allock());
#undef CHECK_ERROR
    return (result);
}

Pkg_private	void
textsw_do_again(view, x, y)
    register Textsw_view_handle view;
    int             x, y;
{
#define CHECK_ERROR(test)	if AN_ERROR(test) goto Again_Error;
    Pkg_private int ev_get_selection();
    Pkg_private void textsw_move_caret();
    register Textsw_folio textsw = FOLIO_FOR_VIEW(view);
    register string_t *again;
    CHAR           *buffer = NULL, *saved_base;
    char            token_buf[20];
    register char  *token = token_buf;
    register int    buffer_length, count;
    Es_index        first, last_plus_one;

    if (!TXTSW_DO_AGAIN(textsw))
	return;
    again = &textsw->again[0];
    if (TXTSW_STRING_IS_NULL(again)) {
	again = &textsw->again[1];
	if (TXTSW_STRING_IS_NULL(again))
	    return;
    }
    saved_base = TXTSW_STRING_BASE(again);
    (void) ev_get_selection(textsw->views, &first, &last_plus_one,
			    EV_SEL_PRIMARY);
    ev_set(view->e_view, EV_CHAIN_DELAY_UPDATE, TRUE, NULL);
    FOREVER {
	count = textsw_scanf(again, "%19s", token);
	if (count == -1)
	    break;
	ASSERT(count == 1);
	count = match_in_table(token, cmd_tokens);
	CHECK_ERROR(count < 0);
	if (buffer != NULL) {
	    free(buffer);
	    buffer = NULL;
	}
	switch ((Token) count) {
	  case CARET_TOKEN:{
		unsigned        direction;
		int             unused;	/* This is an x location */

		count = textsw_scanf(again, "%x %d", &direction, &unused);
		CHECK_ERROR(count != 2);
		(void) textsw_move_caret(view, (Textsw_Caret_Direction) direction);
		break;
	    }
	  case DELETE_TOKEN:
	    CHECK_ERROR(*TXTSW_STRING_BASE(again)++ != '\n');
	    /*
	     * Do the delete and update the selection.
	     */
	    (void) textsw_delete_span(view, first, last_plus_one,
				      TXTSW_DS_ADJUST | TXTSW_DS_SHELVE);
	    first = last_plus_one;
	    break;
	  case EDIT_TOKEN:{
		unsigned        unit, direction;
		/*
		 * Determine the type of edit.
		 */
		count = textsw_scanf(again, "%19s", token);
		CHECK_ERROR(count != 1);
		count = match_in_table(token, edit_tokens);
		switch (count) {
		  case CHAR_TOKEN:
		    unit = EV_EDIT_CHAR;
		    break;
		  case WORD_TOKEN:
		    unit = EV_EDIT_WORD;
		    break;
		  case LINE_TOKEN:
		    unit = EV_EDIT_LINE;
		    break;
		  default:	/* includes case -1 */
		    LINT_IGNORE(CHECK_ERROR(TRUE));
		}
		/*
		 * Determine the direction of edit.
		 */
		count = textsw_scanf(again, "%19s", token);
		CHECK_ERROR(count != 1);
		count = match_in_table(token, direction_tokens);
		switch ((Direction) count) {
		  case FORWARD_DIR:
		    direction = 0;
		    break;
		  case BACKWARD_DIR:
		    direction = EV_EDIT_BACK;
		    break;
		  default:	/* includes case -1 */
		    LINT_IGNORE(CHECK_ERROR(TRUE));
		}
		CHECK_ERROR(*TXTSW_STRING_BASE(again)++ != '\n');
		/*
		 * Process the selection.
		 */
		if (first < last_plus_one) {
		    (void) textsw_set_selection(
						VIEW_REP_TO_ABS(view),
				  ES_INFINITY, ES_INFINITY, EV_SEL_PRIMARY);
		    first = last_plus_one = ES_INFINITY;
		}
		/*
		 * Finally, do the actual edit.
		 */
#ifdef OW_I18N
		(void) textsw_do_edit(view, unit, direction, 0);
#else
		(void) textsw_do_edit(view, unit, direction);
#endif
		break;
	    }

	  case EXTRAS_TOKEN:{
		Pkg_private char **textsw_string_to_argv();
		char          **filter_argv;

		CHECK_ERROR(textsw_next_is_delimiter(again) == 0);
		buffer_length = textsw_text_for_replay(again, &buffer);
		CHECK_ERROR(buffer_length <= 0);

		buffer[buffer_length] = XV_ZERO;	/* In case garbage follow */
		filter_argv = textsw_string_to_argv(buffer);
		(void) textsw_call_filter(view, filter_argv);

		break;
	    }

	  case FIELD_TOKEN:{
		unsigned        field_flag;
		int             matched;
#ifdef OW_I18N
		CHAR            start_marker[4];
		char            start_marker_mb[8];
#else
		char            start_marker[3];
#endif

		count = textsw_scanf(again, "%x", &field_flag);
		CHECK_ERROR(count != 1);
#ifdef OW_I18N
		count = textsw_scanf(again, "%3s", start_marker_mb);
		(void) mbstowcs(start_marker, start_marker_mb, 3);
#else
		count = textsw_scanf(again, "%3s", start_marker);
#endif
		CHECK_ERROR(count != 1);

		textsw_flush_caches(view, TFC_STD);

		if ((field_flag == TEXTSW_FIELD_ENCLOSE) ||
		    (field_flag == TEXTSW_DELIMITER_ENCLOSE)) {
		    int             first, last_plus_one;

		    first = last_plus_one = EV_GET_INSERT(textsw->views);
		    matched = textsw_match_field_and_normalize(view, &first, &last_plus_one, start_marker, STRLEN(start_marker), field_flag, TRUE);
		} else {
		    matched = textsw_match_selection_and_normalize(view, start_marker, field_flag);
		}
		CHECK_ERROR(!matched);
		(void) ev_get_selection(textsw->views, &first, &last_plus_one,
					EV_SEL_PRIMARY);
		break;
	    }
	  case FILTER_TOKEN:{
		Event           event;
		int             dummy;

		count = textsw_scanf(again, "%x", &dummy);
		CHECK_ERROR(count != 1);
		event_set_action(&event, dummy);
		count = textsw_scanf(again, "%x", &dummy);
		CHECK_ERROR(count != 1);
		event.ie_flags = dummy & (~IE_NEGEVENT);
		count = textsw_scanf(again, "%x", &dummy);
		CHECK_ERROR(count != 1);
		event.ie_shiftmask = dummy;
		event.ie_locx = x;
		event.ie_locy = y;
		event.ie_time = textsw->last_point;
		/* Something reasonable */
		CHECK_ERROR(textsw_next_is_delimiter(again) == 0);
		/*
		 * Enter the filtering state
		 */
		(void) textsw_do_filter(view, &event);
		buffer_length = textsw_text_for_replay(again, &buffer);
		if (buffer_length > 0) {
		    BCOPY(buffer, textsw->to_insert, buffer_length);
		    textsw->to_insert_next_free =
			textsw->to_insert + buffer_length;
		}
		/*
		 * Actually invoke the filter
		 */
		event.ie_flags |= IE_NEGEVENT;
		(void) textsw_do_filter(view, &event);
		/*
		 * Find out what the selection looks like now
		 */
		(void) ev_get_selection(textsw->views, &first,
					&last_plus_one, EV_SEL_PRIMARY);
		break;
	    }
	  case FIND_TOKEN:{
		unsigned        flags = EV_FIND_DEFAULT;
		Es_index        insert = EV_GET_INSERT(textsw->views);

		/*
		 * Determine the direction of the search.
		 */
		count = textsw_scanf(again, "%19s", token);
		CHECK_ERROR(count != 1);
		count = match_in_table(token, direction_tokens);
		CHECK_ERROR(count < 0);
		switch ((Direction) count) {
		  case BACKWARD_DIR:
		    CHECK_ERROR(textsw_next_is_delimiter(again) == 0);
		    flags = EV_FIND_BACKWARD;
		    if (first >= last_plus_one) {
			first = insert;
		    }
		    break;
		  case FORWARD_DIR:
		  case EMPTY_DIR:
		    CHECK_ERROR(textsw_next_is_delimiter(again) == 0);
		    if (first >= last_plus_one)
			first = insert;
		    else
			first = last_plus_one;

		    break;
		  default:
		    break;
		}
		/*
		 * Get the pattern for the search, then do it.
		 */
		buffer_length = textsw_text_for_replay(again, &buffer);
		CHECK_ERROR(buffer_length <= 0);
		textsw_find_pattern_and_normalize(
					 view, x, y, &first, &last_plus_one,
				      buffer, (u_int) buffer_length, flags);
		CHECK_ERROR(first == ES_CANNOT_SET);
		/*
		 * Find out what the selection looks like now
		 */
		(void) ev_get_selection(textsw->views, &first,
					&last_plus_one, EV_SEL_PRIMARY);
		break;
	    }
	  case INSERT_TOKEN:{
		(void) textsw_do_pending_delete(view, EV_SEL_PRIMARY,
						TFC_INSERT | TFC_SEL);
		first = last_plus_one;
		/*
		 * Determine the type of text to be inserted.
		 */
		count = textsw_scanf(again, "%19s", token);
		CHECK_ERROR(count != 1);
		count = match_in_table(token, text_tokens);
		switch (count) {
		  case DELIMITER_TOKEN:
		    buffer_length = textsw_text_for_replay(again, &buffer);
		    CHECK_ERROR(buffer_length <= 0);
		    (void) textsw_do_input(view, buffer, (long) buffer_length,
					   TXTSW_UPDATE_SCROLLBAR_IF_NEEDED);
		    break;
		  case PIECES_TOKEN:{
			Es_handle       pieces;
			Es_index        pos = EV_GET_INSERT(textsw->views);
			pieces = textsw_pieces_for_replay(again);
			(void) textsw_insert_pieces(view, pos, pieces);
			break;
		    }
		  case TRASH_TOKEN:{
			Es_index        pos = EV_GET_INSERT(textsw->views);
			(void) textsw_insert_pieces(view, pos, textsw->trash);
			break;
		    }
		  default:
		    LINT_IGNORE(CHECK_ERROR(TRUE));	/* includes case -1 */
		}
		break;
	    }
	  default:
	    LINT_IGNORE(CHECK_ERROR(TRUE));
	}
    }
Again_Error:
    if (buffer != NULL)
	free(buffer);
    ev_set(view->e_view, EV_CHAIN_DELAY_UPDATE, FALSE, NULL);
    ev_update_chain_display(textsw->views);
    TXTSW_STRING_BASE(again) = saved_base;
    ASSERT(allock());
#undef CHECK_ERROR
}
