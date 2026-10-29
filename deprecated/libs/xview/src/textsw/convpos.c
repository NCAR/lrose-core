#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)convpos.c 1.7 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1992 Sun Microsystems, Inc. Sun design patents
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE
 *	file for terms of the license.
 */

#ifdef OW_I18N

#include <xview_private/txt_impl.h>
#include <xview_private/ev_impl.h>

/*
 * Functions declared in this file are used for performance up for calculating
 * position conversion between byte and character based index. These functions
 * will be called, when multibyte API is used by applications or selection.
 * Basic idea is as follows:
 *
 *	1. Cache calculated pos(both byte and char) into table.
 *	2. When calculating new pos, use nearest pos(to the new pos) already
 *	   cached in the table. This aviods to calculate the pos from first
 *	   position in the textsw contents each time.
 *	3. When textsw contents is changed, the table will be cleaned up.
 *
 * 
 *   o 	Each Conv_pos(wc_pos, mb_pos) is cached into cph->table. This table
 *	is filled from the head in order. When cph->table is full, an oldest
 *	one in the table will be overrided with new pos. So, first time of the
 *	override, cph->table[0] will be overrided.
 *	 An order change for cph->table is bit expensive as each element 
 *	consists of 8 bytes. So cph->order has been added to make it reasonable.
 *	cph->order is an array of indices into each elements in cph->table.
 *	cph->order will be lined up in order of small pos in cph->table.
 *
 *		   0     1     2            9    10 
 *		+-----+-----+-----+      +-----+-----+
 *  cph->table	|wc|mb|wc|mb|wc|mb|......|wc|mb|wc|mb| ...... cph->next
 *		|  |  |  |  |  |  |      |  |  |  |  |
 *		+-----+-----+-----+      +-----+-----+
 *		  |        \  /             |
 *		  |         \/              |
 *		  |   +-----/\--------------+
 *		  |   |    /  \
 *		 -------------------
 *  cph->order	| 0 | 9 | 2 | 1 |   | .......
 *		 -------------------
 *
 *   o	New pos will be calculated by using nearest pos(to the new pos) in
 *	the table. For example:
 *	If table[i].wc_pos is nearest to wc_pos1 and wc_pos1 is bigger than
 *	table[i].wc_pos, mb_pos for the wc_pos1 is calculated as follows:
 *
 *	    table[i].mb_pos +
 *		 [the number of bytes between wc_pos1 and table[i].wc_pos]
 *
 *   	The nearest pos is looked up by binary search method.
 *
 *   o	If a distance(character base) between the calculated new pos and the
 *	nearest one is shorter than MIN_BETWEEN_CHECKS, the new pos will not
 *	be cached in order to avoid a complicated update of the tables.
 *	Otherwise, the new pos will be cached in cph->table.
 *	 If an argument mb_pos of textsw_wcpos_from_mbpos() points a portion
 *	of a multibyte character, the front position of the multibyte character
 *	will be cached instead of the argument mb_pos.
 *
 *   o	cph->len is set with length of textsw contents. This is used for
 *	checking whether an argument pos is over the actual contents length or
 *	not, and for popular attribute TEXTSW_LENGTH.
 *
 *   o	When textsw contents is changed, these stuff will be cleaned up.
 *
 */


#define MAX_CHECK_NUM           256  /* This value must not exceed 256, because
					cph->order type is unsigned char. */
#define MIN_BETWEEN_CHECKS      50   /* character based count */
#define TEMP_BUF_SIZE           2048

Pkg_private void	textsw_init_convpos();
Pkg_private void	textsw_destroy_convpos();
Pkg_private int 	textsw_get_mb_length();
Pkg_private Es_index	textsw_wcpos_from_mbpos();
Pkg_private Es_index	textsw_mbpos_from_wcpos();
Pkg_private int 	textsw_get_bytes_span();
static int 		get_chars_span();
static Es_index		get_mbpos();
static Es_index		get_wcpos();
static void		record_pos();

#define	get_convpos(cph, n)	cph->table[cph->order[n]]


/* Create and initialize Conv_pos_object */
Pkg_private	void
textsw_init_convpos(folio)
    register Textsw_folio    folio;
{
    if (folio->locale_is_ale) {
	SET_CONTENTS_UPDATED(folio, TRUE);
	folio->cph = (Conv_pos_handle) xv_malloc(sizeof(Conv_pos_object));
	folio->cph->next = 0;
	folio->cph->len.wc_pos = 0;
	folio->cph->len.mb_pos = 0;
	folio->cph->table =
		(Conv_pos *) xv_malloc(MAX_CHECK_NUM * sizeof(Conv_pos));
	folio->cph->order = (unsigned char *) xv_malloc(MAX_CHECK_NUM);
    }
}


/* Destory Conv_pos_object */
Pkg_private	void
textsw_destroy_convpos(folio)
    Textsw_folio	folio;
{
    if (folio->locale_is_ale) {
	xv_free(folio->cph->table);
	xv_free(folio->cph->order);
	xv_free(folio->cph);
    }
}


/* Return the number of multibyte of the textsw contents. */
Pkg_private	int
textsw_get_mb_length(folio)
    Textsw_folio    folio;
{
    if (!folio->locale_is_ale)
	return (es_get_length(folio->views->esh));
    if (!IS_CONTENTS_UPDATED(folio))
	return (folio->cph->len.mb_pos);
    return (textsw_mbpos_from_wcpos(folio, es_get_length(folio->views->esh)));
}


/*
 *	Convert from character based index to byte based index.
 *	If the textsw contents has been updated, Conv_pos_object will be
 *	initialized.
 *	Search wc_pos from cph->order by binary search. if this wc_pos is
 *	already cached, return the refered mb_pos. Otherwise, get nearest
 *	and bigger index of cph->order to the wc_pos, and call get_mbpos().
 */
Pkg_private	Es_index
textsw_mbpos_from_wcpos(folio, wc_pos)
    register Textsw_folio   folio;
    Es_index		    wc_pos;
{
    Conv_pos_handle	cph = folio->cph;
    register Conv_pos	conv_pos;
    register int	first, middle, last;	/* order indices */

    if (!folio->locale_is_ale || wc_pos == TEXTSW_INFINITY || wc_pos <= 0)
	return (wc_pos);
    if (wc_pos < MIN_BETWEEN_CHECKS)
	return (textsw_get_bytes_span(folio, 0, wc_pos));

    if (IS_CONTENTS_UPDATED(folio)) {
	SET_CONTENTS_UPDATED(folio, FALSE);
	cph->next = 0;
	cph->oldest = 0;
	cph->len.wc_pos = es_get_length(folio->views->esh);
	cph->len.mb_pos = textsw_get_bytes_span(folio, 0, cph->len.wc_pos);
    }
    if (wc_pos >= cph->len.wc_pos)
	return (cph->len.mb_pos);
    if (cph->next == 0) /* Noting is cached yet */
	return (get_mbpos(folio, wc_pos, 0));

    /* binary search method by using cph->order */
    first = 0;
    last = cph->next;
    for (;;) {
	middle = (first + last) / 2;
	conv_pos = get_convpos(cph, middle);

	if (wc_pos == conv_pos.wc_pos)
	    return (conv_pos.mb_pos);
	if (wc_pos < conv_pos.wc_pos) {
		last = middle;
		if (first + 1 >= last) {
			if (last == 1)
			     continue;
			break;
		}
	}
	else {
		first = middle;
		if (first + 1 >= last) {
			middle++;
			break;
		}
	}
    }
    /* wc_pos is between "middle -1" and "middle". */
    return (get_mbpos(folio, wc_pos, middle));
}


/*
 *	Convert from byte based index to character based index.
 *	If the textsw contents has been updated, Conv_pos_object will be
 *	initialized. 
 *	Search mb_pos from cph->order by binary search. if this mb_pos is
 *	already cached, return the refered wc_pos. Otherwise, get nearest
 *	and bigger index of cph->order to the mb_pos, and call get_wcpos().
 */
Pkg_private	Es_index
textsw_wcpos_from_mbpos(folio, mb_pos)
    register Textsw_folio   folio;
    Es_index		    mb_pos;
{
    Conv_pos_handle	cph = folio->cph;
    register Conv_pos	conv_pos;
    register int	first, middle, last;	/* order indices */
    int			unused;

    if (!folio->locale_is_ale || mb_pos == TEXTSW_INFINITY || mb_pos <= 0)
	return (mb_pos);

    /* vague check as mb_pos is compared with character based value. */
    if (mb_pos < MIN_BETWEEN_CHECKS)
	return (get_chars_span(folio, 0, mb_pos, &unused));

    if (IS_CONTENTS_UPDATED(folio)) {
	SET_CONTENTS_UPDATED(folio, FALSE);
	cph->next = 0;
	cph->oldest = 0;
	cph->len.wc_pos = es_get_length(folio->views->esh);
	cph->len.mb_pos = textsw_get_bytes_span(folio, 0, cph->len.wc_pos);
    }
    if (mb_pos >= cph->len.mb_pos)
	return (cph->len.wc_pos);
    if (cph->next == 0)	/* Nothing is cached yet */
	return (get_wcpos(folio, mb_pos, 0));

    /* binary search method by using cph->order */
    first = 0;
    last = cph->next;
    for (;;) {
	middle = (first + last) / 2;
	conv_pos = get_convpos(cph, middle);

	if (mb_pos == conv_pos.mb_pos)
	    return (conv_pos.wc_pos);
	if (mb_pos < conv_pos.mb_pos) {
		last = middle;
		if (first + 1 >= last) {
			if (last == 1)
			     continue;
			break;
		}
	}
	else {
		first = middle;
		if (first + 1 >= last) {
			middle++;
			break;
		}
	}
    }
    /* mb_pos is between "middle -1" and "middle". */
    return (get_wcpos(folio, mb_pos, middle));
}

/*
 *	get_mbpos() gets the mb_pos by wc_pos and order_inedx.
 *	mb_pos is calculated by using pos nearer to wc_pos (front or back pos
 *	in cph->order).  If a distance(character base) between the calculated
 *	new pos and the nearest one is bigger than MIN_BETWEEN_CHECKS, the new
 *	pos will be cached in cph->table.
 */
static	Es_index
get_mbpos(folio, wc_pos, order_index)
    Textsw_folio	folio;
    Es_index		wc_pos;
    int			order_index; /* index to be inserted in cph->order */
{
    Conv_pos_handle	cph = folio->cph;
    Conv_pos		front, back;
    Es_index		mb_pos;

    if (order_index == 0)
    	front.wc_pos = front.mb_pos = 0;
    else 
	front = get_convpos(cph, order_index - 1);
    back = (order_index < cph->next) ? get_convpos(cph, order_index) : cph->len;

   /* calculate mb_pos by using nearer one (front or back). */
    if (wc_pos - front.wc_pos < back.wc_pos - wc_pos) {
	mb_pos = front.mb_pos +
		 textsw_get_bytes_span(folio, front.wc_pos, wc_pos);
	if (wc_pos < front.wc_pos + MIN_BETWEEN_CHECKS)
	    return (mb_pos);
    }
    else {
	mb_pos= back.mb_pos - textsw_get_bytes_span(folio, wc_pos, back.wc_pos);
	if (back.wc_pos < wc_pos + MIN_BETWEEN_CHECKS)
	    return (mb_pos);
    }
    record_pos(cph, wc_pos, mb_pos, order_index);
    return(mb_pos);
}


/*
 *	get_wcpos() gets the wc_pos by mb_pos and order_inedx.
 *	wc_pos is always calculated by using front pos of mb_pos. Because
 *	it's expensive and complicate the logic to calculate wc_pos by using
 *	the back one.  If new pos and the nearest one is bigger than
 *	MIN_BETWEEN_CHECKS, the new pos will be cached in cph->table. Then if
 *	mb_pos points a portion of a multibyte character, the front position
 *	of the multibyte character will be cached instead of the mb_pos.
 */
static	Es_index
get_wcpos(folio, mb_pos, order_index)
    Textsw_folio	folio;
    Es_index		mb_pos;
    int			order_index; /* index to be inserted in cph->order */
{
    Conv_pos_handle	cph = folio->cph;
    Conv_pos		front, back;
    Es_index		wc_pos;
    int			adjust;

    if (order_index == 0)
    	front.wc_pos = front.mb_pos = 0;
    else 
	front = get_convpos(cph, order_index - 1);

    wc_pos = front.wc_pos + get_chars_span(folio,
			front.wc_pos, mb_pos - front.mb_pos, &adjust);
    if (wc_pos < front.wc_pos + MIN_BETWEEN_CHECKS)
	return (wc_pos);
    back = (order_index < cph->next) ? get_convpos(cph, order_index) : cph->len;
    if (back.wc_pos < wc_pos + MIN_BETWEEN_CHECKS)
	return (wc_pos);
    mb_pos += adjust;	/* cache adjusted one */
    record_pos(cph, wc_pos, mb_pos, order_index);
    return(wc_pos);
}


/*
 *	This function caches wc_pos and mb_pos into cph->table and updates
 *	cph->order. The new pos will be inserted at order_index in cph->order.
 *	If cph->table is full, override an oldest one with new pos. Otherwise,
 *	the new pos is cached at table[cph->next] sequentially.
 */
static	void
record_pos(cph, wc_pos, mb_pos, order_index)
    Conv_pos_handle	cph;
    Es_index		wc_pos, mb_pos;
    int			order_index;
{
    unsigned char	save[MAX_CHECK_NUM];

    if (cph->next < MAX_CHECK_NUM) {
	cph->table[cph->next].wc_pos = wc_pos;
	cph->table[cph->next].mb_pos = mb_pos;

	/* update cph->order array */
	if (order_index >= cph->next)
	    cph->order[cph->next] = cph->next;
	else {
	    XV_BCOPY(cph->order + order_index, save, cph->next - order_index);
	    cph->order[order_index] = cph->next;
	    XV_BCOPY(save, cph->order + order_index + 1,
		     cph->next - order_index);
	}
        cph->next++;
    }
    else {  /* cph->table is full. Then override an oldest one with new pos. */
	int	remove_pos, i;

	cph->table[cph->oldest].wc_pos = wc_pos; 
	cph->table[cph->oldest].mb_pos = mb_pos; 

	/* Search an order element to be removed. */
	for (i = 0 ; i < MAX_CHECK_NUM ; i++) {
	    if (cph->order[i] == cph->oldest)
		break;
	}
	remove_pos = i;
	/* remove the order element from order array by shift to left. */
	for ( ; i < MAX_CHECK_NUM - 1 ; i++)
	    cph->order[i] = cph->order[i+1];

	/* update order array. */
	if (order_index >= cph->next)
	    cph->order[cph->next - 1] = cph->oldest;
	else {
	    if (remove_pos < order_index)
		order_index--;
	    XV_BCOPY(cph->order + order_index, save,
		     cph->next - order_index - 1);
	    cph->order[order_index] = cph->oldest;
	    XV_BCOPY(save, cph->order + order_index + 1,
		     cph->next - order_index - 1);
	}
	cph->oldest++;
	if (cph->oldest >= MAX_CHECK_NUM)
	    cph->oldest = 0;
    }
}


/*
 * Calculate number of bytes in span between character based first and last.
 * If argument last is bigger than actual length of contents, this function
 * will calculate number of bytes until last byte of the contents.
 *
 *      assumption: first or last is not TEXTSW_INFINITY and not minus.
 */
Pkg_private     int
textsw_get_bytes_span(folio, first, last)
    register Textsw_folio  folio;
    Es_index	first, last;	/* character based index */
{
    CHAR	wbuf[TEMP_BUF_SIZE + 1];
    int		read, bytes = 0;
    register	wc_len;

    if (first >= last)
	return (0);
    wc_len = last - first;
    if (!folio->locale_is_ale)
	return (wc_len);

    while (wc_len > 0) {
        es_set_position(folio->views->esh, first);
	first = es_read(folio->views->esh,
			(wc_len > TEMP_BUF_SIZE) ? TEMP_BUF_SIZE : wc_len,
			wbuf, &read);
	wbuf[read] = NULL;
	if (read <= 0)
	    break;
	bytes += bytes_in_wcs(wbuf, folio->euc_width);
	wc_len -= read;
    }
    return (bytes);
}


/*
 * Calculate number of characters from character based first until byte_len is
 * read. If argument byte_len is bigger than actual length of contents, this
 * function will calculate number of characters until last byte of the contents.
 */
static	int
get_chars_span(folio, first, byte_len, adjust)
    register Textsw_folio  folio;
    Es_index	first;	  /* character based index */
    int		byte_len; /* number of bytes */
    int	       *adjust;	  /* adjusted length */
{
    CHAR	wbuf[TEMP_BUF_SIZE + 1];
    int		read;
    register	chars = 0;

    *adjust = 0;
    while (byte_len > 0) {
	es_set_position(folio->views->esh, first);
	first = es_read(folio->views->esh,
			(byte_len > TEMP_BUF_SIZE) ? TEMP_BUF_SIZE : byte_len,
			wbuf, &read);
	wbuf[read] = NULL;
	if (read <= 0)
	    break;
	chars += wclen_by_mblen(wbuf, &byte_len, folio->euc_width, adjust);
    }
    return (chars);
}


/*
 * Calculate number of characters until mlen bytes is read in wstr.
 * When mlen points the portion of a multibyte character, *adjust will
 * be set with minu value.
 */
static int
wclen_by_mblen(wstr, mlen, euc_width, adjust)
register CHAR	*wstr;
register	*mlen;
register eucwidth_t	euc_width;
int		*adjust;
{
    CHAR		*wstr_org = wstr;

    if (wstr == NULL)
	return (0);

    /* Using iscodeset?() is faster than using macro wcsetno() */
    while (*wstr) {
	if (iscodeset0(*wstr))
	    *mlen -= 1;
	else if (iscodeset1(*wstr))
	    *mlen -= euc_width._eucw1;
	else if (iscodeset2(*wstr))
	    *mlen -= euc_width._eucw2;
	else if (iscodeset3(*wstr))
	    *mlen -= euc_width._eucw3;

	if (*mlen <= 0) {
	    *adjust = *mlen;
	    break;
	}
	wstr++;
    }
    if (*mlen == 0)
	wstr++;
    return (wstr - wstr_org);
}


/*
 * Calculate number of bytes in wide character buffer.
 */
static int
bytes_in_wcs(wstr, euc_width)
register CHAR	*wstr;
register eucwidth_t	euc_width;
{
    register int	bytes = 0;

    if (wstr == NULL)
	return (0);

    /* Using iscodeset?() is faster than using macro wcsetno() */
    while (*wstr) {
	if (iscodeset0(*wstr))
	    bytes += 1;
	else if (iscodeset1(*wstr))
	    bytes += euc_width._eucw1;
	else if (iscodeset2(*wstr))
	    bytes += euc_width._eucw2;
	else if (iscodeset3(*wstr))
	    bytes += euc_width._eucw3;
	wstr++;
    }
    return(bytes);
}

#endif /* OW_I18N */
