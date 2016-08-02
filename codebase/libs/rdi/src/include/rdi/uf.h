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
/**********************************************************************
	uf.h
	Header file defineing UF structures
**********************************************************************/


struct uf_mandatiry_header {
	char uf[2];
	short rec_len;
	short pos_nman_hd;	
	short pos_loc_hd;	
	short pos_data_hd;	
	short rc_cnt;	
	short v_cnt;
	short r_cnt;
	short rec_cnt;
	short frame_cnt;
	char radar_name[8];
	char site_name[8];	
	short lat_deg;	
	short lat_min;	
	short lat_sec;	
	short lon_deg;	
	short lon_min;	
	short lon_sec;	
	short altitude;	
	short year;	
	short mon;	
	short day;	
	short hour;	
	short min;	
	short sec;
	char t_z[2];	
	short azi;	
	short ele;	
	short sweep_mode;	
	short fix_ang;	
	short sweep_rate;	
	short format_year;	
	short format_mon;	
	short format_day;
	char tape_name[8];	
	short bad_flag;	
};
typedef struct uf_mandatiry_header Uf_mand_header;

struct f_name_pos {
	char name[2];
	short pos;
};
typedef struct f_name_pos Fnp;

struct uf_data_header {
	short n_fields;
	short n_rec;
	short n_f_r;
	Fnp f[8];
};
typedef struct uf_data_header Uf_data_header;

struct uf_field_header {
	short pos_data;
	short scale;
	short range;
	short gate_adj;
	short g_size;
	short n_gates;
	short g_size_r;
	short beam_wid_h;
	short beam_wid_v;
	short bandwidth;
	short polar;
	short wavelength;
	short n_samples;
	char  thresh_fn[2];
	short thresh_val;
	short thr_scale;
	char ed_code[2];
	short prt;
	short wordlength;
};
typedef struct uf_field_header Uf_field_header;

#ifdef __cplusplus             
}
#endif
