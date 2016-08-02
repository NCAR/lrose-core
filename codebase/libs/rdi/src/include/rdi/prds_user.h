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
/********************************************************************
 * PRDS_USER.H: header file for prds user
 * 
 * The basic structure of the Product selector system is as follows:
 *
 * The Display program and the selector interact mainly through
 * a segment of shared memory. This segment contains structures
 * for output messages (from the selector), input messages (from
 * the display), and product information. Following these structures
 * is an area for the text and vector data. The Display is responsible for
 * polling the segment of shared memory to determine which products
 * should be rendered and to read the vector lists and render them
 * if they are selected. The product headers contain pointers (indices)
 * to their text and vector data as well as several variables the display
 * can (optionally) use to increase rendering efficiency. 
 * The Selector window visiblility is controlled by sending a signal
 * to the selector process (PID is in the Out_msg struct), which
 * causes it to read the input message and take the appropriate action.
 * Periodically the selector will rearrange the product structures and their
 * associated data to remove out of data products, deleted ones and to
 * recover space for new ones. It is therefore important that the display use
 * the locking mechanism in order to avoid problems.
 * 
 * The vector data is currently stored as short integers where the
 * value represents units of 10.0 meters. (I divide by 100.0 to get
 * Km). Line segment end points are signaled when the X coordinate is set
 * to the maximum positive short integer (32767). Text data is represented
 * by a string of characters which contains the X,Y location (in 10.0 meter 
 * units) and the string to be rendered. No font metrics are included in this
 * version.
 ********************************************************************/

/*#include <xview/font.h>*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>

#define SHM_KEY 6713
#define SHM_SIZE 300
#define VALID 1234567891
#define LIST_SIZE  128
#define N_PROD     1024

/*
 * Define command messages received from clients
 */

#define PRDS_CLOSE_WIN_CMD               "Close import win"
#define PRDS_SHOW_WIN_CMD                "Show import win"
#define PRDS_CHANGE_TIME_CMD             "Change time range"


/* Define Color Conventions */
#define PRDS_NUM_COLORS             8
#define PRDS_COLOR0             "red"
#define PRDS_COLOR1             "green"
#define PRDS_COLOR2             "blue"
#define PRDS_COLOR3             "yellow"
#define PRDS_COLOR4             "cyan"
#define PRDS_COLOR5             "magenta"
#define PRDS_COLOR6             "white"
#define PRDS_COLOR7             "black"       

struct outmsg {
    int key;  /* a key words, 1234567891*/
    int pid;   /* pid of prds */
    unsigned int np;   /* number of products in the data base */
    int newly_updated; /* >1: new data base, 0: processed by display */
    int loop; /* the loop is on/off (v_intvl/0) */
    char res[12];
};

typedef struct outmsg Out_msg;


struct inmsg {
    char msg[20];  /* a msg string */
    unsigned long start_time;   /* time range for desired data ... */
    unsigned long end_time;     /*    0 = use current data         */
    unsigned int flag; /* flag for communacation */
};

typedef struct inmsg In_msg;

#ifdef USE_OLD_PRDS_STRUCTS
/* product struct */
struct prds_product {    
        unsigned char pid;  /* id of the products */
        unsigned char line_type;  /* line type, 0, 1, ... */
        unsigned short tlen; /* text data length in long words */
        char color_name[20];  /* color name */
        short index; /* index in a class */
        short tindex; /* time index in a time sequence */
                         /* if <0, this is a map */
        unsigned short llen; /* # of points for the line drawing */
        short id; /* a product id from sender */
        unsigned int time;  /* also used for pointor to map name string */
        unsigned int lpt;   /* offset in data field to line data */
        unsigned int tpt; /* offset in data field to text data */
        unsigned char deleted;
        unsigned char selected;
        unsigned char active;   /* used by display */
        unsigned char display;   /* used by display */
        unsigned short period; /* number of seconds product is valid for */
        unsigned short next_tms; /* next item in the time series */
        short timer1; /* timer for display */
        short cf_ind; /* index in configuration file */
        int timer2; /* timer for life */
};

#else

struct prds_product {        /* product struct used for data base */
	unsigned char pid;  
			/* id of the products */
	unsigned char line_type;  
			/* line type, 0, 1, ... */
	unsigned short tlen;
			/* text data length in long words */
	short index;    /* index in a class */
	short tindex;   /* time index in a time sequence */
		        /* if <0, this is a map */
	unsigned short llen;
			/* # of points for the line drawing */
	short id;       /* a product id from sender */
	int time;  	/* also used for pointor to map name string */
	int v_time;	/* valid time */
	int e_time;	/* expiring time */
	unsigned int lpt; 
			/* offset (in long int) in data field */
	unsigned int tpt;
	unsigned char deleted;
	unsigned char selected;
	unsigned char active;    /* used by display */
	unsigned char display;   /* used by display */
	unsigned short next_tms; /* next item in the time series */
	short cf_ind;            /* index in configuration file */
	int timer1;            	 /* timer for display */
	int timer2;              /* timer for life */
	double lat_org;		 /* the origin used for the coordinates */
	double lon_org;		
};
typedef struct prds_product Prds_product;

#endif

#ifdef __cplusplus             
}
#endif

