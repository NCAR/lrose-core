/*      @(#)convpos.h 1.4 93/06/28 SMI       */

/*
 *      (c) Copyright 1992 Sun Microsystems, Inc. Sun design patents
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE
 *      file for terms of the license.
 */

#ifndef convpos_h_DEFINED
#define convpos_h_DEFINED
#ifdef OW_I18N

typedef struct conv_pos {
        Es_index	wc_pos;  /* character based position */
        Es_index        mb_pos;  /* byte based position */
} Conv_pos;

typedef struct conv_pos_object {
	int		 next;	 /* index of next recording in table */
	int		 oldest; /* override index in table */
	Conv_pos	 len;	 /* for length of contents */
	Conv_pos	*table;	 /* This table will be filled from the head. */
	unsigned char	*order;  /* Array of indeies into each table elements */
} Conv_pos_object;

typedef Conv_pos_object	*Conv_pos_handle;

#define	IS_CONTENTS_UPDATED(folio) \
			((EV_CHAIN_PRIVATE(folio->views)->updated) == TRUE)
#define SET_CONTENTS_UPDATED(folio, val) \
			(EV_CHAIN_PRIVATE(folio->views)->updated = val)

#endif /* OW_I18N */
#endif /* convpos_h_DEFINED */
