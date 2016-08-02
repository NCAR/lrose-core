/* 	$Id: dd_mapper.hh,v 1.3 2009/11/25 22:01:22 dixon Exp $	 */

#ifndef dd_mapperH
#define dd_mapperH

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <sys/types.h>
# include <time.h>



# include <sys/time.h>



# include <signal.h>




# include <radar/dorade/dorade_includes.h>
# include <radar/dorade/IndexFields.hh>
# include <radar/dorade/dd_utils.hh>

int parameter_info( const char *, PARAMETER * );

extern "C"
{
  int dd_tokens( char *, char ** );
    void ddin_crack_vold(char *, char *, int);
    void ddin_crack_radd(char *, char *, int);
    void ddin_crack_frib(char *, char *, int);
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
    float rotation_angle;
    long offset;
    long size;
};

struct rot_ang_table {
    char name_struct[4];	/* "RKTB" */
    long sizeof_struct;
    float angle2ndx;
    long ndx_que_size;
    long first_key_offset;
    long angle_table_offset;
    long num_rays;
};

# endif /* ROTANG_STRUCTS */


# ifndef GNERIC_DESC
# define GNERIC_DESC

struct generic_descriptor {
    char name_struct[4];	/* "????" */
    long sizeof_struct;
};
typedef struct generic_descriptor *GD_PTR;

# endif  /* GNERIC_DESC */

# ifndef AC_RADAR_ANGLES
# define AC_RADAR_ANGLES

struct xac_radar_angles {
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

# if __GNUC__
#  define DD_MAPPER_UNUSED __attribute__ ((__unused__))
# else
#  define DD_MAPPER_UNUSED
# endif

static    double d_bogus DD_MAPPER_UNUSED;
static    float f_bogus DD_MAPPER_UNUSED;

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

// other

static const char * known_aliases [] DD_MAPPER_UNUSED =
{
    "DZ DB DBZ"
  , "VE VR"
  , "RH RX RHO RHOHV"
  , "PH DP PHI PHIDP"
  , "ZD DR ZDR"
  , "LD LC LDR"
  , "NC NCP"
  , "LV LVDR LDRV"
};

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

    float       the_min_cell_spacing;
    float       minThreshold;
    float       maxThreshold;
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
    float scale8[64];
    float bias8[64];
    int * cell_lut;
    int sizeof_xstf;

    int constructed_parm_count;
    int dd_found_ryib;
    int max_scan_modes;
    const char *scan_mode_mnes[32];
    char ** dd_aliases;
    int num_alias_sets;
    const char * radar_types_ascii[64];
  

public:

    DD_Time     * ddt;
    DTime       dtime;

    super_SWIB  * sswb;
    VOLUME      * vold;
    comment_d   * comm;
    comment_d   * comms[MAX_COMMENTS];
    RADARDESC   * radd;
    RADARDESC   * radds[MAX_RADARDESC];
    FIELDRADAR  * frib;
    FIELDRADAR  * fribx;
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

    dd_mapper();
    ~dd_mapper(); 

    int map_ptrs(char *, int, int * );
    // maps the input buffer upto and including the first ray if possible
    // all descripters are copied into local memory and pointers to 
    // raw data fields are updated

    int within_range( float range ); // in meters!

    int range_cell( float range ); //  in meters!

    int threshold_setup( int, float, float, char * );

    // sets up the type of thresholding, the min and max values and the
    // threshold field name

    int set_threshold_flags( int * flags );

    // puts a 1 in the passed in flags array if the corresponding gate
    // should be flagged bad for the whole field

    int set_threshold_flags( int * flags, int cell1, int cell2 );

    // same as set_threshold_flags except is just between and including 
    // the two specified cell numbers 

    int return_thresholded_field( char * name, float * vals
				  , float * bad_val );

    // passes in the field name, a pointer to where the thresholded values
    // should go, and a pointer to what the missing data flag should look like
    // as a floating point number
    // this method unscales the data and replaces thresholded data with
    // bad value flags for the whole field

    int return_thresholded_field
    ( char * name, float * vals, float * bad_val, float range1, float range2 );

    // similar to 3 argument version except the data passed back are between
    // the two specified ranges

    int return_thresholded_field
    ( char * name, float * vals, float * bad_val, int cell1, int cell2 );

    // similar to 3 argument version except the data passed back are between
    // and includeing the two specified cells

    int return_field( const char *, float *, float * );
         
    // same as return_thr_field but without thresholding

    int replace_field( char *, float *, float * );

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

    inline double tilt() { return this->asib->tilt +
			       this->cfac->tilt_corr; }
    inline double roll() { return this->asib->roll +
			       this->cfac->roll_corr; }
    inline double pitch() { return this->asib->pitch +
			       this->cfac->pitch_corr; }
    inline double drift() { return this->asib->drift_angle +
			       this->cfac->drift_corr; }
    inline double heading() { return this->asib->heading +
			       this->cfac->heading_corr; }
    
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

    float return_eff_unamb_vel( int ndx ) {
      return this->radd->eff_unamb_vel;
    }

    float return_eff_unamb_range( int ndx ) {
      return this->radd->eff_unamb_range;
    }

    float return_radar_const( int ndx ) {
      return this->radd->radar_const;
    }

    time_t unix_time() { return this->ddt->unix_time(); }
    DTime d_time() { return this->dtime; }


    inline char * radar_name() { return the_radar_name; }

    inline int num_fields() { return this->radd->num_parameter_des; }

    
    char * field_name(int field_num);
    int field_index_num( const char * name ); 

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

  void set_bad_data_flag (int field_num, int bad)
  { this->parms[field_num]->bad_data = bad; }

    // special convenience utilities here


    double AngDiff(float a1, float a2); 
    
    int inSector(float ang, float ang1, float ang2);

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

  void ac_radar_angles (double *azimuth, double *elevation
			, double *rotation_angle, double *tilt
			, double *ac_vel, CORRECTION *alt_cfac=0);

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

