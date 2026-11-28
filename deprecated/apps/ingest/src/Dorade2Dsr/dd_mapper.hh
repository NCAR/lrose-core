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
/* 	$Id: dd_mapper.hh,v 1.5 2016/03/07 01:23:00 dixon Exp $	 */

#ifndef dd_mapperH
#define dd_mapperH

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

#include <dorade_includes.h>
#include "IndexFields.hh"
#include <dd_utils.hh>
using namespace std;

extern "C"
{
  int dd_tokens( char *, char ** );
    void ddin_crack_vold(char *, char *, int);
    void ddin_crack_radd(char *, char *, int);
    void ddin_crack_parm(char *, char *, int);
    void ddin_crack_celv(char *, char *, int);
    void ddin_crack_cfac(char *, char *, int);
    void ddin_crack_swib(char *, char *, int);
    void ddin_crack_ryib(char *, char *, int);
    void ddin_crack_asib(char *, char *, int);
    void ddin_crack_qdat(char *, char *, int);
    void ddin_crack_sswb(char *, char *, int);
    void ddin_crack_sswbLE(char *, char *, int);
    void ddin_crack_rktb(char *, char *, int);
    void swack_long( char *, char *, int );
    void swack_short( char *, char *, int );
    void swack4( char *, char * );
    void swack2( char *, char * );
    char * str_terminate(char *, char *, int);
    int dd_hrd16LE_uncompressx( unsigned short *, unsigned short *
				, int, int *, int );
    int dd_hrd16_uncompressx( unsigned short *, unsigned short *
			      , int, int *, int );
    int dd_compress( unsigned short *, unsigned short *
		     , unsigned short, int );
}


# ifndef ROTANG_STRUCTS
# define ROTANG_STRUCTS

struct rot_table_entry {
    fl32 rotation_angle;
    si32 offset;
    si32 size;
};

struct rot_ang_table {
    char name_struct[4];	/* "RKTB" */
    si32 sizeof_struct;
    fl32 angle2ndx;
    si32 ndx_que_size;
    si32 first_key_offset;
    si32 angle_table_offset;
    si32 num_rays;
};

# endif /* ROTANG_STRUCTS */


# ifndef GNERIC_DESC
# define GNERIC_DESC

struct generic_descriptor {
    char name_struct[4];	/* "????" */
    si32 sizeof_struct;
};
typedef struct generic_descriptor *GD_PTR;

# endif  /* GNERIC_DESC */

# ifndef AC_RADAR_ANGLES
# define AC_RADAR_ANGLES

struct ac_radar_angles {
    /* all angles are in radians
     */
    double azimuth;
    double elevation;
    double x;
    double y;
    double z;
    double psi;
    double rotation_angle;
    double tilt;
    double ac_velocity_component;
};
# endif

# ifndef DD_MAPPER_HH
# define DD_MAPPER_HH

// c---------------------------------------------------------------------------


// c---------------------------------------------------------------------------

typedef void * dd_obj_ptr;

// types of thresholding

const int           REMOVE_LT = 1;
const int           REMOVE_GT = 2;
const int    REMOVE_LT_AND_GT = 4;
const int  PRESERVE_LT_AND_GT = 8; // notch!

// error states

const int  DATA_UNDECIPHERABLE = 1;

static const int MAX_RADARDESC = 16;
static const int MAX_COMMENTS = 64;

static const int MAX_FIELDS = 64;
static const int MAX_GATES=1500;



class dd_mapper {

private:
    // pointers for dorade stuff

    char        * local_buf;
    char        * data_buf;	// may be needed for byte swapping fields
    char        * free_at;
    char        * scr0;

    double      dd_latitude;
    double      dd_longitude;
    double      dd_altitude;
    double      prev_time;

    fl32       the_min_cell_spacing;
    fl32       minThreshold;
    fl32       maxThreshold;
    char        thrFieldName[16];
    int         thrFieldIndex;
    int         typeofThresholding;

    int         count;
    int         dd_complete;
    int         dd_sweep_file;
    int         dd_error_state;

    int         dd_vol_count;
    int         dd_sweep_count;
    int         dd_ray_count;
    int         dd_asib_count;
    int         dd_sweep_ray_count;
    int         dd_rays_prev_sweep;
    int         sizeof_volume;
    int         dd_sizeof_ray;
    int         sizeof_comment;
    int         comment_count;
    int         ignore_cfac;
    int         dd_swapped_data;
    int         result;
    char        the_radar_name[16];
    int its_8_bit[64];
    fl32 scale8[64];
    fl32 bias8[64];
    int * cell_lut;
    int sizeof_xstf;

    int constructed_parm_count;
    int dd_found_ryib;
    int max_scan_modes;
    char * scan_mode_mnes[32];
    char ** dd_aliases;
    int num_alias_sets;
    char * radar_types_ascii[64];
    void ac_velocity_component();
  

public:

    DD_Time     * ddt;
    DTime       dtime;

    super_SWIB  * sswb;
    VOLUME      * vold;
    comment_d   * comm;
    comment_d   * comms[MAX_COMMENTS];
    RADARDESC   * radd;
    RADARDESC   * radds[MAX_RADARDESC];
    CORRECTION  * cfac;
    PARAMETER   * parms[MAX_FIELDS];
    CELLVECTOR  * celv;
    CELLVECTOR  * celvc;	// corrected ranges
    SWEEPINFO   * swib;
    RAY         * ryib;
    PLATFORM    * asib;
    PARAMDATA   * rdats[MAX_FIELDS];
    QPARAMDATA  * qdats[MAX_FIELDS];
    char        * raw_data[MAX_FIELDS];
    char          field_names[MAX_FIELDS][16];

    int         dd_new_vol;
    int         dd_new_sweep;
    int         dd_new_ray;
    int         dd_new_asib;
    int         num_radardesc;
    XTRA_STUFF  * xstf;

    ac_radar_angles *ra;

    dd_mapper();
    ~dd_mapper(); 

    int map_ptrs(char *, int, int * );
    // maps the input buffer upto and including the first ray if possible
    // all descripters are copied into local memory and pointers to 
    // raw data fields are updated

    int within_range( fl32 range ); // in meters!

    int range_cell( fl32 range ); //  in meters!

    int threshold_setup( int, fl32, fl32, char * );

    // sets up the type of thresholding, the min and max values and the
    // threshold field name

    int set_threshold_flags( int * flags );

    // puts a 1 in the passed in flags array if the corresponding gate
    // should be flagged bad for the whole field

    int set_threshold_flags( int * flags, int cell1, int cell2 );

    // same as set_threshold_flags except is just between and including 
    // the two specified cell numbers 

    int return_thresholded_field( char * name, fl32 * vals
				  , fl32 * bad_val );

    // passes in the field name, a pointer to where the thresholded values
    // should go, and a pointer to what the missing data flag should look like
    // as a floating point number
    // this method unscales the data and replaces thresholded data with
    // bad value flags for the whole field

    int return_thresholded_field
    ( char * name, fl32 * vals, fl32 * bad_val, fl32 range1, fl32 range2 );

    // similar to 3 argument version except the data passed back are between
    // the two specified ranges

    int return_thresholded_field
    ( char * name, fl32 * vals, fl32 * bad_val, int cell1, int cell2 );

    // similar to 3 argument version except the data passed back are between
    // and includeing the two specified cells

    int return_field( char *, fl32 *, fl32 * );
         
    // same as return_thr_field but without thresholding

    int replace_field( char *, fl32 *, fl32 * );

    // scales the floating pt values and inserts the "bad_data" flag
    // when *val == *bad_val
    // data are put back in the original raw_data location

    int copy_raw_field( char *, char * );

    // copies the contents of the raw field named into memory
    // at the pointer

    void ray_constructor( dd_mapper *, char *,  char **, int );

    // creates a new mapping in this object based on a source map object and
    // a list of fields some of which may be in the source map and some or
    // all may not. All volume descriptors are copied plus the relevent fields.

    void copy_data( dd_mapper *, char **, int);

    // copies field data descripters and data in list from another map
    inline int volume_num() { return this->vold->volume_num; }
    inline int sweep_num() { return this->swib->sweep_num; }

    inline int new_vol() { return dd_new_vol; }
    inline int new_sweep() { return dd_new_sweep; }
    inline int new_ray() { return dd_new_ray; }
    inline int new_mpb() { return dd_new_asib; } // moving platform block
    inline int volume_count() { return dd_vol_count; }
    inline int sweep_count() { return dd_sweep_count; }
    inline int total_ray_count() { return dd_ray_count; }
    inline int sweep_ray_count() { return dd_sweep_ray_count; }
    inline int sizeof_ray() { return dd_sizeof_ray; }
    inline int complete() { return dd_complete; }
    inline int found_ryib() { return dd_found_ryib; }
    inline int error_state() { return dd_error_state; }
    inline int swapped_data() { return dd_swapped_data; }

    inline int radar_type() { return radd->radar_type; }
    const char * radar_type_ascii() {
      return radar_types_ascii[ radd->radar_type ];
    }
    inline int scan_mode() { return radd->scan_mode; }
    const char * scan_mode_mne() { return scan_mode_mnes[ radd->scan_mode ]; }
    inline int rays_in_sweep() { return swib->num_rays; }
    
    inline double latitude() { return dd_latitude; }
    inline double longitude() { return dd_longitude; }
    inline double altitude_km() { return dd_altitude; }
    inline double azimuth()
    {
	return FMOD360( this->ryib->azimuth +
			this->cfac->azimuth_corr );
    }
    inline double elevation()
    {
	return FMOD360( this->ryib->elevation +
			this->cfac->elevation_corr );
    }
    inline double fixed_angle() { return this->swib->fixed_angle; }
    inline double roll() { return this->asib->roll +
			       this->cfac->roll_corr; }
    
    double rotation_angle();

    inline int number_of_cells() { return this->celvc->number_cells; }
    inline double meters_to_first_cell() { return this->celvc->dist_cells[0]; }
    inline double meters_between_cells() { return this->celvc->dist_cells[1]
				       - this->celvc->dist_cells[0]; }
    inline double min_cell_spacing() { return the_min_cell_spacing; }
    inline double meters_to_last_cell()
    { return this->celvc->dist_cells[this->celvc->number_cells-1]; }
    int return_num_samples( int field_num )
    { return this->parms[0]->num_samples; }
    void return_asib( PLATFORM * asib_ptr )
    { memcpy( asib_ptr, this->asib, sizeof( *this->asib )); }
    void return_cfac( CORRECTION * cfac_ptr )
    { memcpy( cfac_ptr, this->cfac, sizeof( *this->cfac )); }

    int year() { return this->ddt->year(); }
    int month() { return this->ddt->month(); }
    int day() { return this->ddt->day(); }
    int hour() { return this->ddt->hour(); }
    int minute() { return this->ddt->minute(); }
    int second() { return this->ddt->second(); }
    int millisecond() { return this->ddt->millisecond(); }
    int microsecond() { return this->ddt->microsecond(); }
    char * ascii_time() { return this->ddt->ascii_time(); }

     const char * proj_name() {
       static char pname[24];
       str_terminate( pname, vold->proj_name, 20 );
       return pname;
     }

    const char * param_description( int ndx ) {
      if( ndx < 0 )
	return NULL;
      return this->parms[ndx]->param_description;
    }

    const char * param_units( int ndx ) {
      if( ndx < 0 )
	return NULL;
      return this->parms[ndx]->param_units;
    }

    fl32 return_eff_unamb_vel( int ndx ) {
      return this->radd->eff_unamb_vel;
    }

    fl32 return_eff_unamb_range( int ndx ) {
      return this->radd->eff_unamb_range;
    }

    fl32 return_radar_const( int ndx ) {
      return this->radd->radar_const;
    }

    time_t unix_time() { return this->ddt->unix_time(); }
    DTime d_time() { return this->dtime; }


    inline char * radar_name() { return the_radar_name; }

    inline int num_fields() { return this->radd->num_parameter_des; }

    
    char * field_name(int field_num);
    int field_index_num( char * name ); 

    inline double scale( int field_num ) {
	if(field_num >= 0 && field_num < this->radd->num_parameter_des)
	    { return this->parms[field_num]->parameter_scale; }
	return 1.;
    }
    inline double bias( int field_num ) {
	if(field_num >= 0 && field_num < this->radd->num_parameter_des)
	    { return this->parms[field_num]->parameter_bias; }
	return 0;
    }
    inline int bad_data_flag( int field_num ) {
	if(field_num >= 0 && field_num <this-> radd->num_parameter_des)
	    { return this->parms[field_num]->bad_data; }
	return 0;
    }
    inline char * raw_data_ptr(int field_num)
    {
	if(field_num >= 0 && field_num < this->radd->num_parameter_des)
	    { return this->raw_data[field_num]; }
	return NULL;
    }
    inline PARAMETER * param_ptr( char * name ) {
	int fn = this->field_index_num( name );
	if( fn < 0 )
	    { return NULL; }
	return this->parms[fn];
    }
    inline void reset_comment_count() { comment_count = 0; }
    inline int return_comment_count() { return comment_count; }

  int return_frequencies( int field_num, double * freqs );

  int return_interpulse_periods( int field_num, double * ipps );

    // special convenience utilities here


    double AngDiff(fl32 a1, fl32 a2); 
    
    int inSector(fl32 ang, fl32 ang1, fl32 ang2);

    char * slashPath( char * path );

    int dz_present();

    int vr_present();

    int zdr_present();

    int ldr_present();

    int rho_present();

    int phi_present();

    int alias_index_num( char * name ); 

    void set_radar_angles();

    void zero_fields();

  const int * set_cell_lut( double cell0, double cell_inc, int cell_count );

};

// c---------------------------------------------------------------------------

// can't be a member variable, since Sun CC won't support initializing
// such a static member, or using it as a array size
static const int max_maps = 64;
class index_of_mappers {

private:
    dd_mapper * ddmapper[max_maps];

public:

    index_of_mappers();

    int new_mapper_index();
    dd_mapper * return_mapper( int index );
};

#endif // dd_mapperH

// c---------------------------------------------------------------------------


// c---------------------------------------------------------------------------


// c---------------------------------------------------------------------------

# endif // DD_MAPPER_HH

