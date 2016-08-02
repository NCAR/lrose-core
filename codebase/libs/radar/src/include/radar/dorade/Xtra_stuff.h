/* 	$Id: Xtra_stuff.h,v 1.1 2008/01/24 22:22:32 rehak Exp $	 */

# ifndef XTRA_STUFF_H
# define XTRA_STUFF_H

struct dd_extra_stuff {		/* extra container for non DORADE structs */
  char name_struct[4];		/* "XSTF" */
  long sizeof_struct;

  long one;			/* always set to one (endian flag) */
  long source_format;		/* as per ../include/dd_defines.h */

  long offset_to_first_item;	/* bytes from start of struct */
  long transition_flag;
} ;

typedef struct dd_extra_stuff XTRA_STUFF;

# endif
