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
/* 	$Id: super_SWIB.h,v 1.3 2016/03/07 01:23:00 dixon Exp $	 */


# ifndef INCsuper_SWIBh
# define INCsuper_SWIBh

# define   MAX_KEYS 8

# define      KEYED_BY_TIME 1
# define   KEYED_BY_ROT_ANG 2
# define  SOLO_EDIT_SUMMARY 3

# define        NDX_ROT_ANG 0
# define           NDX_SEDS 1

# define SWIB_NEW_VOL 0x1

struct key_table_info {
    si32 offset;
    si32 size;
    si32 type;
};

struct super_SWIB {
    char name_struct[4];	/* "SSWB" */
    si32 sizeof_struct;
    /* parameters from the first version */
    si32 last_used;		/* Unix time */
    si32 start_time;
    si32 stop_time;
    si32 sizeof_file;
    si32 compression_flag;
    si32 volume_time_stamp;	/* to reference current volume */
    si32 num_params;		/* number of parameters */

    /* end of first version parameters */

    char radar_name[8];

  /* d_start_time and d_stop_time are replaced by char buffers
   * for portability, becasue including a double at that point
   * causes a 64-bit host to pad the struct out by an extra 
   * 4 bytes */
  /*   double d_start_time; */
  /*   double d_stop_time; */

    char start_time_buf[8];
    char end_time_buf[8];
    /*
     * "last_used" is an age off indicator where > 0 implies Unix time
     * of the last access and
     * 0 implies this sweep should not be aged off
     */
    si32 version_num;
    si32 num_key_tables;
    si32 status;
    si32 place_holder[7];
    struct key_table_info key_table[MAX_KEYS];
    /*
     * offset and key info to a table containing key value such as
     * the rot. angle and the offset to the corresponding ray
     * in the disk file
     */
};

struct super_SWIB_v0 {
    char name_struct[4];	/* "SSWB" */
    si32 sizeof_struct;

    si32 last_used;		/* Unix time */
    /*
     * "last_used" is an age off indicator where > 0 implies Unix time
     * of the last access and
     * 0 implies this sweep should not be aged off
     */
    si32 start_time;
    si32 stop_time;
    si32 sizeof_file;
    si32 compression_flag;
    si32 volume_time_stamp;	/* to reference current volume */
    si32 num_params;		/* number of parameters */
};

typedef struct super_SWIB super_SWIB;
typedef struct super_SWIB SUPERSWIB;

# endif  /* INCsuper_SWIBh */

