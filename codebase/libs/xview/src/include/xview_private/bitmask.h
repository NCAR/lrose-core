/*	@(#)bitmask.h 20.12 93/06/28 SMI	*/

/*
 *  Bitmask handling declarations
 */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#ifndef bitmask_h
#define bitmask_h

typedef struct bm_ {
    unsigned int *bm_mask;
    int bm_max_bits;
    int bm_mask_size;
} Bitmask;

#ifndef BITSPERBYTE
#define BITSPERBYTE 8
#endif

#ifndef bitmask_c

extern Bitmask * xv_bitss_new_mask();
extern void xv_bitss_dispose_mask();
extern Bitmask * xv_bitss_set_mask();
extern unsigned int xv_bitss_get_mask();
extern int xv_bitss_cmp_mask();
extern Bitmask * xv_bitss_copy_mask();

#endif /* bitmask_c */

#endif /* bitmask_h */
