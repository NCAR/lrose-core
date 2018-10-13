/* 	$Id: dd_sweepfiles.hh,v 1.3 2018/10/13 22:42:50 dixon Exp $	 */

#ifndef dd_sweepfilesH
#define  dd_sweepfilesH

# include <signal.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <sys/types.h>
# include <time.h>
# include <sys/time.h>

# include <fstream>
# include <iostream>


# include <radar/dorade/dorade_includes.h>
# include <radar/dorade/dd_utils.hh>
# include <radar/dorade/dd_mapper.hh>


extern "C"
{
    int dd_tokens(char *, char **);
    int dd_tokenz(char *, char **, const char *);
    long time_now();
    struct solo_list_mgmt * solo_malloc_list_mgmt( int );
    int get_swp_files( char *, struct solo_list_mgmt * );
    char *solo_list_entry( struct solo_list_mgmt *, int );
    void solo_add_list_entry(solo_list_mgmt *, char *, int);
    void solo_sort_strings( char **sptr, int ns );
    char *solo_modify_list_entry(struct solo_list_mgmt * which
				 , char * entry, int len, int entry_num);
    int generic_sweepfiles( const char *dir, struct solo_list_mgmt *lm
			     , const char *prefix, const char *suffix,
                            int no_suffix );
    char * dd_scan_mode_mne(int scan_mode, const char * str);
    int dd_scan_mode(const char * str);
    char * dd_radar_type_ascii(int scan_mode, const char * str);
    char * slash_path( const char *, const char * );
}

// c---------------------------------------------------------------------------

class Swp_File_names {
private:

    DD_Time * ddt;

    DTime dtime;
    long ttime;
    int size;
    int the_milliseconds;
    int vol_num;
    int sweep_num;
    int scan_mode;
    char keep_name[256];
    char the_radar_name[16];
    char the_qualifier[256];
    char the_file_type[8];
    float the_fixed_angle;

public:

    ~Swp_File_names() { delete ddt; }
    Swp_File_names() { ddt = new DD_Time(); }

    int crack_file_name( const char * name );

    DTime return_dtime() { return dtime; }
    long return_ttime() { return ttime; }
    int return_milliseconds() { return the_milliseconds; }
    const char * file_type() { return the_file_type; }
    const char * radar_name() { return the_radar_name; }
    const char * qualifier() { return the_qualifier; }
    float fixed_angle() { return the_fixed_angle; }
};


// c---------------------------------------------------------------------------

void dd_Testify(const char *);

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
    std::ifstream         * sfile;
    rot_ang_table   * rktb;
    char            * swapped_rktb;
    rot_table_entry * first_rte;
    rot_table_entry * rte;
    long            * angle_zero; // first entry in angle index
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
    if (sfile) {
        delete sfile;
        sfile = NULL;
    }
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

    int angle_to_ray_num( float );
    
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

	sfile->seekg((std::streampos)rte->offset);
	return this->next_ray();
    }
};

// c---------------------------------------------------------------------------

class dd_sweepfile {		// manages the writing of a sweepfile

private:
    std::ofstream         * sfile;
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
    long            * angle_zero; // first entry in angle index

    dd_mapper       * ddm;
    DD_Time         * ddt;
    
    std::streampos         offset_to_swib;

    static const int         angle_ndx_size;
    int               max_rays;
  // int               sizeof_rotang_table;
    int               min_rays_per_sweep;
    int               swp_ray_num;
    int               swp_count_out;
    int               suppressing_filename_print;
    int               the_typeof_compression;
    int               num_radardesc;

    std::streampos         offset_to_cfac;

    PARAMETER         * tmp_parm;
    CELLSPACINGFP     * csfd;
    int               convert_to_8_bit;
    int               produce_CSFD;
    int               csfd_gate_skip;
    int               csfd_num_gates;
  // float             csfd_distToFirst;
    float scale8[64];
    float bias8[64];

    void update_rotang_table();

    void reset_rotang_table() {
	long * lp = angle_zero;
	long * lpEnd = angle_zero + angle_ndx_size;
	for(; lp < lpEnd; *lp++ = -1);
	rktb->num_rays = 0;
	rktb->sizeof_struct = rktb->first_key_offset;
    }

    void enlarge_rotang_table();

public:

    dd_sweepfile( int min_rays = 11 );

    super_SWIB      * sswb;

    ~dd_sweepfile();

  int end_sweepfile_size;

    int begin_sweepfile(dd_mapper *, const char *, const char *, int );

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

    void change_fixed_angle( float fixed_angle )
    {
	sprintf( ascii_fixed_angle, ".%.1f_", fixed_angle );
    }

    void change_qualifier( char * qualifier )
    {
	if(!qualifier || !strlen(qualifier))
	    { return; }
	int nn = strlen(qualifier);
	nn = nn >= (int)sizeof(the_qualifier) ? sizeof(the_qualifier) -1 : nn;
	strncpy( the_qualifier, qualifier, nn );
	the_qualifier[nn] = '\0';
	return;
    }

    const char * end_sweepfile( int ); // may be passed a kill flag

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
  // char 	      dir_name[192];
  // char 	      filename[128];
  //  char 	      tmp_filename[128];
  //  char              permanent_name[256];
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
    long            * angle_zero; // first entry in angle index

    dd_mapper       * ddm;
    DD_Time         * ddt;
    

    // initialized in dd_sweepfiles.cc to be standards compliant
    // and portable to Sun CC
    static const int         angle_ndx_size;
    int               max_rays;
  // int               sizeof_rotang_table;
    int               min_rays_per_sweep;
    int               swp_ray_num;
    int               swp_count_out;


    char            * mem_sweepfile;
  // char            * data_buf;
    int               the_max_file_size;
    int               current_offset;
    int               offset_to_cfac;
    int               offset_to_swib;

    // c...mark

    void update_rotang_table() {
	if(rktb->num_rays +1 > max_rays)
	    { enlarge_rotang_table(); }

	int rnum = rktb->num_rays++;
	rte = first_rte + rnum;

	float rot = ddm->rotation_angle();
	int ii = (int)(rot * rktb->angle2ndx);
	*(angle_zero + ii) = rnum;
	rte->rotation_angle = rot;
	rte->offset = sswb->sizeof_file;
	rktb->sizeof_struct += sizeof(*rte);
    }

    void reset_rotang_table() {
	long * ll = angle_zero;
	long * llEnd = angle_zero + angle_ndx_size;
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

    const char * end_sweepfile( int ); // may be passed a kill flag

    int seek_ray( int );

    int reread_ray( int, char * );

    int rewrite_ray( int );
};

// c---------------------------------------------------------------------------

class DD_File_names {
private:

    DD_Time * ddt;

    DTime dtime;
    long ttime;
    int size;
    int the_milliseconds;
    int the_version;
    char keep_name[256];
    char the_radar_name[16];
    char the_qualifier[256];
    char the_file_type[8];
    float the_fixed_angle;
    char the_qualifier_no_fxd[256];

public:

    ~DD_File_names() { }
    DD_File_names() { ddt = new DD_Time(); }

    int crack_file_name( char * name );
    DTime return_dtime() { return dtime; }
    long return_ttime() { return ttime; }
    int return_milliseconds() { return the_milliseconds; }
    int return_version() { return the_version; }
    char * file_type() { return the_file_type; }
    char * radar_name() { return the_radar_name; }
    char * comment() { return the_qualifier; }
    char * qualifier() { return the_qualifier; }
    char * qualifier_no_fxd() { return the_qualifier_no_fxd; }
    float fixed_angle() { return the_fixed_angle; }
};

#endif // dd_sweepfilesH

// c---------------------------------------------------------------------------



// c---------------------------------------------------------------------------





