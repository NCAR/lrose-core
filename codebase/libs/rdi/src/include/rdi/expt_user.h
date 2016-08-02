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


#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/errno.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>


#define MAX_CPT 68   /* 42 points for box mark and point marks */
#define MAX_GPT 110
#define MAX_DRAW 12
    /* MAX_GPT*(MAX_DRAW+1)+20 < 1536 */
#define SHM_KEY_EXPT 3694
#define EXPT_KEY 91123456
#define DISP_KEY 89112345
#define ANAL_KEY 78911234
#define DIC_XT 0x7fff  /* separation */
#define SYM_XT 0x7ffe  /* terminate a symbol */

struct commsg {
    int keyd;  /* a key words, 11234567, set by display */
    int keye;  /* a key words, 91123456, set by expt */
    int wind;   /* window id of display */
    int wine;   /* window id of expt */
    int nd;   /* number of drawings in the data base */
    short flag;  /* a flag indicating the data status */
    short newly_updated; /* a flag indicating the graphics needs to be updated */
    int ddata;  /* data from display */
    int dtime;  /* data from display, time of data */
    int adata;  /* data from the third party */
    int scale;  /* scaling factor sent by display */
    int parm;   /* additional parameter */
};
typedef struct commsg Com_msg;

struct draw_data {    
        int nt; /* number of control points */
        int ng; /* number of graphical points */
        char show;  /* to be shown */
        char act; /* the mpline is active */
        char displaied; /* this is displaied */
        char update; /* this needs to be updated */

        short dclass;  /* type of the products */
        short index;
             /* index in a type */
        unsigned int time;  /* creation time */
        unsigned int d_time; /* the data time */
        int timer;

        int xt[MAX_GPT]; /* the mpline coord. */
        int yt[MAX_GPT];

        short lab_x;
        short lab_y;
        char lab_str[16];
};
typedef struct draw_data Draw_data;


struct wcgraph {    
        short np; /* number of control points */

        short pid;  /* type of the products */
        short index; /* index in a type */
        short parm; 
        unsigned int time;  

        short x[MAX_CPT]; /* the mpline coord. */
        short y[MAX_CPT];
        short z[MAX_CPT];
};
typedef struct wcgraph Wcgraph;

#ifdef __cplusplus             
}
#endif

