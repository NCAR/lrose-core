/* 	$Id: Comment.h,v 1.1 2007/09/22 21:08:21 dixon Exp $	 */

struct comment_d {
    char comment_des[4];	/* Comment descriptor identifier: ASCII */
				/* characters "COMM" stand for Comment */
				/* Descriptor. */
    si32  comment_des_length;	/* Comment descriptor length in bytes. */
    char  comment[500];	        /* comment*/

}; /* End of Structure */



typedef struct comment_d comment_d;
typedef struct comment_d COMMENT;

