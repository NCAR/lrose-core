/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
#ifdef __cplusplus                                                        
 extern "C" {                                                         
#endif
/*****************************************************************

	Definition of the overlay objects import to and export
	from rdi.

	file: overlay_object.h

*****************************************************************/


/* Each object belongs to a class which has a unique pid;
   Each object in a class has an index to identify itself;
   In a class objects can have the same index. In this case the
   objects with the same index are assumed to belong to a group;

   There are three lists, the class list, the product list and
   the group list, in the prds user interface.

   A group of objects is listed as a single item in the product list
   and all objects in the group are listed in the group list. When
   they are list in the group list, they are listed by their times,
   or their group id - gind, depending on the user's configuration 
   (prds.conf).

 */

/* #include <toolsa_globals.h> */
#define OVER_OBJ_MSG_ID 65710324

typedef struct overlay_object {
    unsigned long int mlen;	/* message length in bytes */
    unsigned long int type;	/* message type = OVER_OBJ_MSG_ID */
    unsigned short pid;		/* id of the object class */
    unsigned short index;	/* index in the object type  */
    unsigned long dlen;		/* data length in long words */
    unsigned long int time;	/* data time */
    unsigned long int dpt;	/* data offset in the message, in bytes */
    int v_time;			/* valid time */
    int e_time;			/* expiring time */
    unsigned short gind;	/* index in a group */
    short parm1;		/* additional parameters, which meaning
				   depends on pid */
    long parm2;
    char prod_name[4];
} Overlay_object;


typedef struct {
    unsigned char reserved;
    unsigned char colorindex;
    unsigned char attributes;
    unsigned char function;
} SessionHeader;

typedef struct {
    short    y;
    short    x;
} PrdsPoint;


/* Function definitions for Session Header */
#define RESET_ORIGIN 2
#define DRAW_LINES 0
#define DRAW_TEXT 1
#define DRAW_ICON 3

/* Special data point values that indicate special functions */
#define PEN_UP      0x7ffffffd
#define PEN_UP_x      0xfffd
#define PEN_UP_y      0x7fff
#define END_OF_DATA 0x7fffffff
#define END_SESSION 0x7ffffffe

/* The graphical data are organized as the following 

   1. Data contains a number of sessions. Each session defines a
   graphics primitive, which has the same color and attributes. 
   A session is terminated by 0x7ffffffe. The defined primitives 
   are: lines, icons, char string, and setting origin.

   2. Each session starts with a 4 byte header:
   First (highest) byte: not used.
   Second byte: color index; 0 reserved for erase;
   Third byte: an attribute of the primitive (line or font code etc);
   Fourth byte: Type of this session:
   0 - a line; 1 - text; 2 - reset the origin; 3 - icon;
   (Other primitives such as
   filled polygone, circle, etc, will be defined in the future).

   3. In a session, after the header is the (y, x) of the location
   of the primitive. Both x and
   y are short integers. y is at the leading position. The unit
   is in 10 meters. The origin is set by the data and its default
   is at (0, 0). The coordinate
   system used is: y pointing down (south) and x pointing right
   (east) with the default origin at the display origin.
   The location for a text string is the location of the center
   of the string.

   4. After the location:

   If text: A character string terminated by NULL.
   If lines: The sequence of the line points, which are defined in 
   the same way as the primitive location. Note that the first point 
   in the line is the location of the
   primitive as specified immediately following the header. 
   If icon: The line points that form the icon. The points are
   specified by the offset from the primitive location in pixels.
   If reset origin: session termination;

   We use 0x7ffffffd for pen up for both lines and icon.

   5. The data set must be terminated by 0x7fffffff;

   6. We assume the maximum session length is 1400 long words. Lines
   bigger than that can be send by multiple sessions.

   7. The color code:
   1: Red; 2: Green; 3: Blue; 4: Yellow; 5: Cyan; 6. Magenta;
   7: White ...

 */

enum {BLACK = 0, RED , GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE };

#ifdef __cplusplus             
}
#endif

