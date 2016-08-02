// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/* 	$Id: dd_sweepfiles.hh,v 1.5 2016/03/07 01:23:00 dixon Exp $	 */

#ifndef dd_sweepfilesH
#define  dd_sweepfilesH

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <ctime>
#include <sys/time.h>
#include <signal.h>
#include <iostream>
#include <fstream>


#include "dorade_includes.h"
#include "dd_utils.hh"
#include "dd_mapper.hh"
using namespace std;

extern "C"
{
    int dd_tokens(char *, char **);
    int dd_tokenz(char *, char **, char *);
    si32 time_now();
    struct solo_list_mgmt * solo_malloc_list_mgmt( int );
    int get_swp_files( char *, struct solo_list_mgmt * );
    char *solo_list_entry( struct solo_list_mgmt *, int );
    void solo_add_list_entry(solo_list_mgmt *, char *, int);
    void solo_sort_strings( char **sptr, int ns );
    char *solo_modify_list_entry(struct solo_list_mgmt * which
				 , char * entry, int len, int entry_num);
    int generic_sweepfiles( char *dir, struct solo_list_mgmt *lm
			     , char *prefix, char *suffix, int no_suffix );
    char * dd_scan_mode_mne(int scan_mode, char * str);
    int dd_scan_mode(char * str);
    char * dd_radar_type_ascii(int scan_mode, char * str);
    char * slash_path( char *, char * );
}

// c---------------------------------------------------------------------------

class Swp_File_names {
private:

    DD_Time * ddt;

    DTime dtime;
    si32 ttime;
    int size;
    int the_milliseconds;
    int vol_num;
    int sweep_num;
    int scan_mode;
    char keep_name[256];
    char the_radar_name[16];
    char the_qualifier[256];
    char the_file_type[8];
    fl32 the_fixed_angle;

public:

    ~Swp_File_names() { delete ddt; }
    Swp_File_names() { ddt = new DD_Time(); }

    int crack_file_name( char * name );

    DTime return_dtime() { return dtime; }
    si32 return_ttime() { return ttime; }
    int return_milliseconds() { return the_milliseconds; }
    char * file_type() { return the_file_type; }
    char * radar_name() { return the_radar_name; }
    char * qualifier() { return the_qualifier; }
    fl32 fixed_angle() { return the_fixed_angle; }
};


// c---------------------------------------------------------------------------

void dd_Testify(char *);

const int LAST_RAY     = -1;
const int READ_FAILURE = -2;
const int OPEN_FAILURE = -3;

// a return value > 0 implies the read size of a good open

const int DD_BUF_SIZE = 256;    // in kilobytes

// c---------------------------------------------------------------------------
// we assume that an entire SweepFile header is smaller than this,
// and that we'll encounter RYIB's before this we finish processing
// records in this size buffer

const int MAX_D_HEADER = 4*K64; // maximum size of DORADE_HEADER


class dd_sweepfile_access {	// manages accessing a sweepfile

private:
    fstream         * sfile;
    rot_ang_table   * rktb;
    char            * swapped_rktb;
    rot_table_entry * first_rte;
    rot_table_entry * rte;
    si32            * angle_zero; // first entry in angle index
    char 	    * local_buf;
    char            * rotang_buf;
    char            * data_ptr;

    dd_mapper       * ddm;

    int               swp_ray_num;
    int               sizeof_rktb;
    int               prev_vol_num;
    int               dd_new_vol;
    int               suppressing_filename_print;
    int               local_max_rec_size;


public:

    rot_table_entry    rte_view;

    ~dd_sweepfile_access()
    {
	delete[] local_buf;
	if( rotang_buf )
	    delete[] rotang_buf;
	if( swapped_rktb )
	    delete[] swapped_rktb;
    	if (sfile) delete sfile;
    }

    dd_sweepfile_access()
    {
	swp_ray_num = 0;
	sizeof_rktb = 0;
	sfile = 0;
	prev_vol_num = -999999;

	int nb = MAX_D_HEADER + MAX_PARMS * sizeof(short) * MAXCVGATES;
	local_buf = new char [nb];
	memset(local_buf, 0, nb);
	int ii = 4 * 1024;
	local_max_rec_size = nb - ii;
	rotang_buf = 0;
	data_ptr = local_buf;
	swapped_rktb = NULL;
	suppressing_filename_print = 1;
	sfile = NULL;
    }

    int new_vol() { return dd_new_vol; }

    int sweep_ray_num() { return swp_ray_num; }

    int angle_to_ray_num( fl32 );
    
    int access_sweepfile( const char *, dd_mapper * );

    int next_ray();

    void close_sweepfile() { delete sfile; sfile = NULL; }

    int suppress_filename_print()
    {
	suppressing_filename_print = suppressing_filename_print ? 0 : 1;
	return suppressing_filename_print;
    }

    rot_table_entry *
    get_rotang_entry( int ray_number )
    {
	if(ray_number < 0 || ray_number >= rktb->num_rays) {
	    return NULL;
	}
	memcpy((char *)&rte_view, (char *)(first_rte + ray_number)
	       , sizeof(*first_rte));
	return &rte_view;
    }

    double
    return_rotation_angle( int ray_num )
    {
	if( ray_num < 0 || ray_num >= rktb->num_rays )
	    { return -1.; }
	return (double) (first_rte + ray_num)->rotation_angle;
    }

    int seek_ray( int ray_num ) {

	if(ray_num < 0 || ray_num >= rktb->num_rays)
	    { return -1; }

	rte = first_rte + ray_num;
	swp_ray_num = ray_num;

	sfile->seekg((streampos)rte->offset);
	return this->next_ray();
    }
};

// c---------------------------------------------------------------------------

class dd_sweepfile {		// manages the writing of a sweepfile

private:
    fstream         * sfile;
    char 	      dir_name[128];
    char 	      filename[128];
    char 	      tmp_filename[128];
    char              permanent_name[256];
    char              the_qualifier[128];
    char              ascii_fixed_angle[32];
    generic_descriptor * null_des;
    char 	    * local_buf;
    char 	    * free_at;
    char            * rotang_buf;
    VOLUME          * vold;
    RADARDESC       * radd;
    RADARDESC       * radds[MAX_RADARDESC];
    SWEEPINFO       * swib;
    char            * compressed_data;
    rot_ang_table   * rktb;
    rot_table_entry * first_rte;
    rot_table_entry * rte;
    si32            * angle_zero; // first entry in angle index

    dd_mapper       * ddm;
    DD_Time         * ddt;
    
    streampos         offset_to_swib;

    static const int         angle_ndx_size;
    int               max_rays;
    int               sizeof_rotang_table;
    int               min_rays_per_sweep;
    int               swp_ray_num;
    int               swp_count_out;
    int               suppressing_filename_print;
    int               the_typeof_compression;
    int               num_radardesc;

    streampos         offset_to_cfac;

    PARAMETER         * tmp_parm;
    CELLSPACINGFP     * csfd;
    int               convert_to_8_bit;
    int               produce_CSFD;
    int               csfd_gate_skip;
    int               csfd_num_gates;
    fl32             csfd_distToFirst;
    fl32 scale8[64];
    fl32 bias8[64];

    void update_rotang_table();

    void reset_rotang_table() {
	si32 * lp = angle_zero;
	si32 * lpEnd = angle_zero + angle_ndx_size;
	for(; lp < lpEnd; *lp++ = -1);
	rktb->num_rays = 0;
	rktb->sizeof_struct = rktb->first_key_offset;
    }

    void enlarge_rotang_table();

public:

    dd_sweepfile( int min_rays = 11 );

    super_SWIB      * sswb;

    ~dd_sweepfile();

    int begin_sweepfile(dd_mapper *, char *, char *, int );

    int add_to_sweepfile( );

    int sweep_ray_num() { return swp_ray_num; }

    void rename_file( DTime dtime, char * radar_name, int version_num )
    {
	if( radar_name && strlen( radar_name ) )
	    { str_terminate(this->sswb->radar_name, radar_name, 8); }

	DD_Time * tddt = ddm->ddt;
	DTime keep_time = tddt->time_stamp();
	tddt->accept_time_stamp( dtime );
	tddt->unstamp_the_time();
	int vm = (version_num >= 0) ? version_num * 1000 + tddt->millisecond()
	    :  tddt->millisecond();

	sprintf( filename
		 , "%s%s%02d%02d%02d%02d%02d%02d%s%s%s%d"
		 , "swp"
		 , DD_NAME_DELIMITER
		 , tddt->year() -1900
		 , tddt->month()
		 , tddt->day()
		 , tddt->hour()
		 , tddt->minute()
		 , tddt->second()
		 , DD_NAME_DELIMITER
		 , this->sswb->radar_name
		 , DD_NAME_DELIMITER
		 , vm
	    );
	tddt->accept_time_stamp( keep_time );
	tddt->unstamp_the_time();
    }

    void change_fixed_angle( fl32 fixed_angle )
    {
	sprintf( ascii_fixed_angle, ".%.1f_", fixed_angle );
    }

    void change_qualifier( char * qualifier )
    {
	if(!qualifier || !strlen(qualifier))
	    { return; }
	int nn = strlen(qualifier);
	nn = nn >= (int) sizeof(the_qualifier) ? sizeof(the_qualifier) -1 : nn;
	strncpy( the_qualifier, qualifier, nn );
	the_qualifier[nn] = '\0';
	return;
    }

    char * end_sweepfile( int ); // may be passed a kill flag

    int seek_ray( int );

    int reread_ray( int, char * );

    void rewrite_ray( int );

    int suppress_filename_print()
    {
	suppressing_filename_print = suppressing_filename_print ? 0 : 1;
	return suppressing_filename_print;
    }

    void set_typeof_compression( int typeof_compression )
    { the_typeof_compression = typeof_compression; }

    void set_8_bit_output() { convert_to_8_bit = YES; }
    void unset_8_bit_output() { convert_to_8_bit = NO; }

    void set_CSFD_output( int gate_skip, int num_gates )
    {
	produce_CSFD = YES;
	csfd_gate_skip = gate_skip;
	csfd_num_gates = num_gates;
    }
};

// c---------------------------------------------------------------------------

class dd_mem_sweepfile {	// manages the writing of a memory sweepfile

private:
    fstream         * sfile;
    char 	      dir_name[192];
    char 	      filename[128];
    char 	      tmp_filename[128];
    char              permanent_name[256];
    generic_descriptor * null_des;
    char 	    * local_buf;
    char 	    * free_at;
    char            * rotang_buf;

    VOLUME          * vold;
    RADARDESC       * radd;
    SWEEPINFO       * swib;
    rot_ang_table   * rktb;
    rot_table_entry * first_rte;
    rot_table_entry * rte;
    si32            * angle_zero; // first entry in angle index

    dd_mapper       * ddm;
    DD_Time         * ddt;
    
    streampos         offset_to_swib;

    // initialized in dd_sweepfiles.cc to be standards compliant
    // and portable to Sun CC
    static const int         angle_ndx_size;
    int               max_rays;
    int               sizeof_rotang_table;
    int               min_rays_per_sweep;
    int               swp_ray_num;
    int               swp_count_out;


    char            * mem_sweepfile;
    char            * data_buf;
    int               the_max_file_size;
    int               current_offset;

    streampos         offset_to_cfac;

    // c...mark

    void update_rotang_table() {
	if(rktb->num_rays +1 > max_rays)
	    { enlarge_rotang_table(); }

	int rnum = rktb->num_rays++;
	rte = first_rte + rnum;

	fl32 rot = ddm->rotation_angle();
	int ii = (int)(rot * rktb->angle2ndx);
	*(angle_zero + ii) = rnum;
	rte->rotation_angle = rot;
	rte->offset = sswb->sizeof_file;
	rktb->sizeof_struct += sizeof(*rte);
    }

    void reset_rotang_table() {
	si32 * ll = angle_zero;
	si32 * llEnd = angle_zero + angle_ndx_size;
	for(; ll < llEnd; *ll++ = -1);
	rktb->num_rays = 0;
	rktb->sizeof_struct = rktb->first_key_offset;
    }

    void enlarge_rotang_table();

public:

    dd_mem_sweepfile( int, int min_rays = 11 );

    super_SWIB      * sswb;

    ~dd_mem_sweepfile();

    int begin_sweepfile(dd_mapper *, char *, char *, int );

    int add_to_sweepfile( );

    char * end_sweepfile( int ); // may be passed a kill flag

    int seek_ray( int );

    int reread_ray( int, char * );

    int rewrite_ray( int );
};

// c---------------------------------------------------------------------------

class DD_File_names {
private:

    DD_Time * ddt;

    DTime dtime;
    si32 ttime;
    int size;
    int the_milliseconds;
    int the_version;
    char keep_name[256];
    char the_radar_name[16];
    char the_qualifier[256];
    char the_file_type[8];
    fl32 the_fixed_angle;

public:

    ~DD_File_names() { }
    DD_File_names() { ddt = new DD_Time(); }

    int crack_file_name( char * name )
    {
	char zro = '\0';
	char * dot = ".";
	char * aa = name;
	char * cc = keep_name;

	if(!aa)
	    { return 0; }

	int kn = sizeof( keep_name );
	for( size = 0 ; *aa && size < kn ; size++, *cc++ = *aa++ );
	*cc = zro;

	char str[256], *str_ptrs[32];
	strcpy(str, keep_name);
	dd_tokenz( str, str_ptrs, dot );

	strcpy( the_radar_name, str_ptrs[2] );

	ddt->reset();

	aa = str_ptrs[1];	// main time stamp

	char *bb = aa + strlen( aa ) - 10; // ready to suck off all but year
	int mon, day, hrs, min, secs;

	sscanf( bb, "%2d%2d%2d%2d%2d", &mon, &day, &hrs, &min, &secs );

	ddt->set_day( day );
	ddt->set_month( mon );
	*bb = zro;
	ddt->set_year( atoi( aa ) ); // might be more than two characters

	int ms = atoi( str_ptrs[3] );
	the_milliseconds = ms % 1000;
	the_version = ms/1000;


	ddt->set_additional_seconds( (DTime)D_SECS( hrs, min, secs
						    , the_milliseconds ));

	dtime = ddt->stamp_the_time();
	ttime = (si32)dtime;

	// get to the qualifier (may have dots of its own)
	int nd = 0;
	aa = keep_name;
	for( ; *aa && nd < 4; aa++ ) {
	    if( *aa == *dot )
		{ nd++; }
	}
	strcpy( the_qualifier, aa );

	the_fixed_angle = -999.; // see if we can pull out a fixed angle

	if( (bb = strstr( aa, "_" ))) {
	    char *cc = str;
	    for(; aa < bb; *cc++ = *aa++); *cc = '\0';
	    if( sscanf( str, "%f", &the_fixed_angle ) != 1 )
		{ the_fixed_angle = -999.; }
	}
	return 1;
    }

    DTime return_dtime() { return dtime; }
    si32 return_ttime() { return ttime; }
    int return_milliseconds() { return the_milliseconds; }
    int return_version() { return the_version; }
    char * file_type() { return the_file_type; }
    char * radar_name() { return the_radar_name; }
    char * comment() { return the_qualifier; }
    char * qualifier() { return the_qualifier; }
    fl32 fixed_angle() { return the_fixed_angle; }
};

#endif // dd_sweepfilesH

// c---------------------------------------------------------------------------



// c---------------------------------------------------------------------------





