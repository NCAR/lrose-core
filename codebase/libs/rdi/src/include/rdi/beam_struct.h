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
/******************************************************************
	the header file defining beam_struct
	File: beam_struct.h
	Notes:
******************************************************************/

/* the struct for radar data header */
struct beam_struct {

    short CPI_seq_num;
    short rad_seq_num;
    short tilt_num;
    short scan_num;
    short tilt_type;
    short fixed_angle;

    short scan_dir;
    short range_to_first_gate;
    short num_spacing_segs;
    short num_gates;
    short gate_spacing;
    char rgs_1[2];		/* due to SUN-4 OS 4.0 alignment problems */
    char rgs_2[32];
    char rgs_3[2];
    short prf;

    short mon;
    short day;
    short year;
    short hour;
    short min;
    short sec;
    short azi;
    short ele;

    char radar_name[16];
    char site_name[16];
    char proj_name[16];
    int lat;
    int lon;
    short alt;
    short bw;
    short polar;
    short power_trans;
    short freq;
    short pw;
    char prod_name[4][8];
    short resolution[4];
    short min_val[4];
    char junk1[32];

  /* These shouldn't be here anymore */

/*    short stg; */
/*    short ngat; */
};
typedef struct beam_struct Beam_struct;

/* the struct for radar data header */
struct beam_struct_test {
/*  short CPI_seq_num; 
   short rad_seq_num; */
    short tilt_num;
    short scan_num;
    short tilt_type;
    short fixed_angle;
    short scan_dir;
    short range_to_first_gate;
    short num_spacing_segs;
    short num_gates;
    short gate_spacing;
    char rgs_1[2];		/* due to SUN-4 OS 4.0 alignment problems */
    char rgs_2[32];
    char rgs_3[2];
    short prf;
    short mon;
    short day;
    short year;
    short hour;
    short min;
    short sec;
    short azi;
    short ele;
    char radar_name[16];
    char site_name[16];
    char proj_name[16];
    int lat;
    int lon;
    short alt;
    short bw;
    short polar;
    short power_trans;
    short freq;
    short pw;
    char prod_name[4][8];
    short resolution[4];
    short min_val[4];
    char junk1[32];
    short stg;
    short ngat;
};
typedef struct beam_struct_test Beam_struct_test;
#ifdef __cplusplus             
}
#endif
