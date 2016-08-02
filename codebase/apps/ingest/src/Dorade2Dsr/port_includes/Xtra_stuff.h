/* 	$Id: Xtra_stuff.h,v 1.1 2007/09/22 21:08:21 dixon Exp $	 */

# ifndef XTRA_STUFF_H
# define XTRA_STUFF_H

struct dd_extra_stuff {		/* extra container for non DORADE structs */
  char name_struct[4];		/* "XSTF" */
  si32 sizeof_struct;

  si32 one;			/* always set to one (endian flag) */
  si32 source_format;		/* as per ../include/dd_defines.h */

  si32 offset_to_first_item;	/* bytes from start of struct */
  si32 transition_flag;
} ;

typedef struct dd_extra_stuff XTRA_STUFF;

# endif
