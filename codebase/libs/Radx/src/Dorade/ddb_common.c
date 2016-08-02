/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1992 - 2010 */
/* ** University Corporation for Atmospheric Research(UCAR) */
/* ** National Center for Atmospheric Research(NCAR) */
/* ** Research Applications Laboratory(RAL) */
/* ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA */
/* ** 2010/10/7 23:12:35 */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* 	$Id: ddb_common.c,v 1.2 2010/10/07 23:49:58 dave Exp $	 */

#ifndef lint
static char vcid[] = "$Id: ddb_common.c,v 1.2 2010/10/07 23:49:58 dave Exp $";
#endif /* lint */

# define NEW_ALLOC_SCHEME
# define WEN_CHOWS_ALGORITHM

/*
 * This file contains the following routines
 * 
 * 
 * dd_absorb_cfac
 * dd_align_structs
 * dd_alloc_data_field
 * dd_azimuth_angle
 * dd_cell_num
 * dd_clear_pisp
 * dd_clip_gate
 * dd_compress
 * dd_copy_pisp
 * dd_datum_size
 * dd_delimit
 * dd_drift
 * dd_earthr
 * dd_elevation_angle
 * dd_find_desc
 * dd_find_field
 * dd_fixed_angle
 * dd_get_cfac
 * dd_get_difs
 * dd_get_lims
 * dd_get_time_lims
 * dd_givfld
 * dd_heading
 * dd_ini
 * dd_input_strings
 * dgi_interest_really
 * dd_itsa_physical_dev
 * dd_latitude
 * dd_latlon_relative
 * dd_latlon_shift
 * 
 * dd_link_dev_name
 * dd_longitude
 * dd_malloc_radd
 * dd_nav_rotation_angle
 * dd_nav_tilt_angle
 * dd_ndx_name
 * dd_pitch
 * 
 * dor_print_asib
 * dor_print_celv
 * dor_print_parm
 * dor_print_radd
 * dor_print_ryib
 * dor_print_swib
 * dor_print_vold
 * 
 * dd_radar_angles
 * dd_radar_name
 * dd_radar_namec
 * dd_radar_selected
 * dd_range_gate
 * dd_ray_coverage
 * dd_razel_to_xyz
 * dd_reset_d_limits
 * dd_return_id_num
 * dd_return_rotang1
 * dd_roll
 * dd_rotang_seek
 * dd_rotang_table
 * dd_rotation_angle
 * dd_rng_info
 * 
 * dd_site_name
 * dd_src_fid
 * dd_tilt_angle
 * dd_time_keeper
 * dd_tokens
 * dd_uniform_cell_spacing
 * 
 * 
 * dd_whiteout
 * dd_xy_plane_horizontal
 * dgi_buf_rewind
 * fp_bin_search
 * HRD_recompress
 * RDP_compress_bytes
 * swap2
 * swap4
 * 
 * 
 * 
 */

# include <dorade_headers.h>
# include "input_limits.h"
# include "input_sweepfiles.h"
# include "dd_stats.h"
# include <time.h>
# include <sys/time.h>
# ifndef PISP
# include <point_in_space.h>
# endif
# include <dd_math.h>
# include <sys/stat.h>
# include <function_decl.h>
# include <dgi_func_decl.h>

static char *dd_sweep_modes[] =
{ "CAL", "PPI", "COP", "RHI", "VER", "TAR", "MAN", "IDL", "SUR", "AIR"
	, "???", "???", "???", "???", "???", "???" };

char *dd_align_structs();
void dd_get_cfac();
void dd_get_lims();
struct dd_general_info *dd_ini();
void dd_reset_d_limits();
char *put_tagged_string();



/* c...mark */

/* c------------------------------------------------------------------------ */

int
dd_get_scan_mode(mne)
  char *mne;
{
    int jj;
    
    for(jj=0; jj < MAX_MODES; jj++) {
	if(strncmp(mne, dd_sweep_modes[jj], 3) == 0) {
	    return(jj);
	}
    }
    return(-1);
}
/* c------------------------------------------------------------------------ */

struct cfac_wrap *dd_absorb_cfacs(list, radar_name )
  char *list, *radar_name;
{
    char *aa=list;
    char *dd_link_dev_name();
    char str[1024], *sptrs[32], *fnptrs[16];
    FILE *stream;
    struct cfac_wrap *cfw, *cfw_prev, *first_cfw = 0;
    struct correction_d *cfac;
    int ii, nt, nt2;
    char radar[16], *str_terminate();
    static int cfac_print = 0;


    str_terminate(radar, radar_name, 8);
    strcpy(str, list);
    nt = dd_tokens(str, sptrs);	/* split on whitespace */

    for(ii=0; ii < nt; ii += 3) {
       
       nt2 = dd_tokenz( sptrs[ii], fnptrs, ":" ); /* split on ":" */
       
       if(strstr(radar, fnptrs[0])) {
	  if(ii+2 < nt) {
	     
	     cfw = (struct cfac_wrap *)malloc( sizeof( struct cfac_wrap ));
	     cfw->next = 0;
	     cfw->cfac = (struct correction_d *)
	       malloc( sizeof( *cfac ));
	     memset (cfw->cfac, 0, sizeof( *cfac ));
	     strncpy( cfw->cfac->correction_des, "CFAC", 4 );
	     cfw->cfac->correction_des_length = sizeof(*cfac);
	     strcpy( cfw->frib_file_name, fnptrs[1] );
	     strcpy (cfw->radar_name, radar);


	     if( !first_cfw ) {
		first_cfw = cfw;
	     }
	     else {
		cfw_prev->next = cfw;
	     }
	     cfw_prev = cfw;

	     if (cfac_print++ < 8)
	       { printf("Opening cfac file: %s\n", sptrs[ii+2]); }

	     if((stream = fopen( sptrs[ii+2], "r"))) {
		dd_get_cfac( stream, cfw->cfac );
		fclose(stream);
	     }
	     else {
		printf("Unable to open %s\n",sptrs[ii+2] );
		exit(1);
	     }
	  }
       }
    }

    return (first_cfw);
}
/* c------------------------------------------------------------------------ */

void dd_absorb_cfac( list, radar_name, cfac)
  char *list, *radar_name;
  struct correction_d *cfac;
{
    char *a=list;
    char *dd_link_dev_name();
    char str[256];
    FILE *stream;
    static int cfac_print = 0;


    if(strstr(list, "CLEAR_CORR")) {
	/* assume cfac descriptors have been set to zero. i.e. do nothing! */
	printf("Clear Corrections!\n");
	return;
    }

    if(!(a=dd_link_dev_name(radar_name, list, str))) {
	return;
    }

    if (cfac_print++ < 8)
      { printf("Opening cfac file: %s\n", str); }

    if((stream = fopen(str, "r"))) {
	dd_get_cfac(stream, cfac);
	fclose(stream);
    }
    else {
	printf("Unable to open %s\n", str);
	exit(1);
    }
    return;
}
/* c------------------------------------------------------------------------ */

char *
dd_align_structs(c, n)
  char *c;
  int n;
{
    /* pass back a pointer aligned to the next 8 byte boundary */
    int i, sod=sizeof(double);

    if(n % sod)
	  n += sod -(n % sod);
    return(c+n);
}
/* c------------------------------------------------------------------------ */

double
dd_altitude(dgi)
  struct dd_general_info *dgi;
{
    double d=dgi->dds->asib->altitude_msl;
    if(dd_isnanf(d))
	  return((double)0);
    else
	  return(d + dgi->dds->cfac->pressure_alt_corr);
}
/* c------------------------------------------------------------------------ */

double
dd_altitude_agl(dgi)
  struct dd_general_info *dgi;
{
    double d=dgi->dds->asib->altitude_agl;
    if(dd_isnanf(d))
	  return((double)0);
    else
	  return(d + dgi->dds->cfac->radar_alt_corr);
}
/* c------------------------------------------------------------------------ */

void
dd_alloc_data_field(dgi, pn)
  DGI_PTR dgi;
  int pn;
{
    int mm;
    DDS_PTR dds = dgi->dds;

    mm = (dds->celv->number_cells + 2) *
	  dd_datum_size(dds->parm[pn]->binary_format);
    mm = LONGS_TO_BYTES(BYTES_TO_LONGS(mm));

    if(mm != dds->sizeof_qdat[pn]) {
# ifdef obsolete
	printf("dd_alloc_data_field: %d\n  num_cells: %d  mm: %d  %s  %d  %d\n"
	       , pn, dds->celv->number_cells, mm
	       , dts_print(d_unstamp_time(dds->dts))
	       , dds->qdat_ptrs[pn], dds->sizeof_qdat[pn]);
# endif
	if(dds->qdat_ptrs[pn]) {
	    free(dds->qdat_ptrs[pn]);
	}
	if(dds->qdat_ptrs[pn] = (char *)malloc(mm))
	      memset(dds->qdat_ptrs[pn], 0, mm);
	else {
	    printf("Unable to allocate qdat num: %d\n", pn);
	    exit(1);
	}
	dds->sizeof_qdat[pn] = mm;
# ifdef obsolete
	printf(" %d  %d\n", dds->qdat_ptrs[pn], dds->sizeof_qdat[pn]);
# endif
    }
}
/* c------------------------------------------------------------------------ */

double
dd_azimuth_angle(dgi)
  struct dd_general_info *dgi;
{
    double d_rot;

    if(dgi->dds->radd->scan_mode == AIR) {
	/* most aircraft data
	 */
	d_rot = FMOD360(DEGREES(dgi->dds->ra->azimuth)+360.);
    }
    else if(dgi->dds->radd->radar_type == AIR_LF ||
	dgi->dds->radd->radar_type == AIR_NOSE) {
	/* this is meant to apply only to the P3 lower fuselage data */
	d_rot = dgi->dds->ryib->azimuth;
	if(dd_isnanf(d_rot))
	      d_rot = 0;
	else
	      d_rot = FMOD360(d_rot + dgi->dds->cfac->azimuth_corr
			      + dd_heading(dgi));
    }
    else {
	d_rot = dgi->dds->ryib->azimuth;
	if(dd_isnanf(d_rot))
	      d_rot = 0;
	else
	      d_rot = FMOD360(d_rot + dgi->dds->cfac->azimuth_corr);
    }
    return(d_rot);
}
/* c------------------------------------------------------------------------ */

int
dd_cell_num(dds, parameter_num, range)
  struct dds_structs *dds;
  int parameter_num;
  float range;
{
    /* find the cell corresponding to this range using the
     * uniform spacing lookup table
     */
    int ii, ndx=0, *lut, ncMax = dds->celv->number_cells -1;
    float r;

    lut = dds->uniform_cell_lut[ndx];
    ii = .5 + (range - dds->uniform_cell_zero[ndx]) *
	  dds->rcp_uniform_cell_spacing[ndx];
    if(ii < 0)
	  ii = 0;
    else if(ii >= dds->uniform_cell_count[ndx])
	  ii = dds->uniform_cell_count[ndx] -1;

    ii = *(lut +ii);		/* now look it up */
    if(ii > ncMax) {
       ii = ncMax;
    }
    return(ii);
}
/* c------------------------------------------------------------------------ */

void dd_clear_pisp(pisp)
  struct point_in_space *pisp;
{
    int nn=sizeof (*pisp);
    memset(pisp, 0, nn);
    pisp->sizeof_struct = nn;
    strncpy(pisp->name_struct, "PISP", 4);
}
/* c------------------------------------------------------------------------ */

int dd_clip_gate(dgi, elev, alt, lower, upper)
  DGI_PTR dgi;
  float elev;			/* assume elev in radians */
  float alt;			/* assume alt in km */
  double lower, upper;
{
    struct platform_i *asib=dgi->dds->asib;
    struct correction_d *cfac=dgi->dds->cfac;
    struct cell_d *celv=dgi->dds->celvc;
    struct radar_angles *ra=dgi->dds->ra;
    int g, lc=celv->number_cells-1, parameter_num = 0;
    float f, r;
    double alt_msl=alt*1000., d, rcp_sin;
    double dd_elevation_angle();

    elev = RADIANS(dd_elevation_angle(dgi));

    /* radar_angles should already have been called */
    if(fabs(elev*celv->dist_cells[lc]) < .01) {
	/* close enough to horizontal */
	return(lc);
    }
    rcp_sin = 1./sin(elev);

    f = (upper-alt_msl)*rcp_sin;

    if(f > celv->dist_cells[0]) {
	if(f > celv->dist_cells[lc])
	      return(lc);
	g = dd_cell_num(dgi->dds, parameter_num, f);
	return(g);
    }
    f = (lower-alt_msl)*rcp_sin;
    if(f > celv->dist_cells[0]) {
	if(f > celv->dist_cells[lc])
	      return(lc);
	g = dd_cell_num(dgi->dds, parameter_num, f);
	return(g);
    }
    return(lc);
}
/* c------------------------------------------------------------------------ */

int dd_compress( src, dst, flag, n )
  unsigned short *src, *dst, flag;
  int n;
{
    /* implement hrd compression of 16-bit values
     * and return the number of 16-bit words of compressed data
     */
    int mark, kount=0, wcount=0, data_run;
    unsigned short *ss=src, *dd=dst;
    unsigned short *rlcode, *end=src+n-1;

    if(n < 2) {
	printf("Trying to compress less than 2 values\n");
	exit(1);
    }

    for(;ss < end;) {
	/* for each run examine the first two values
	 */
	kount = 2;
	rlcode = dd++;
	if(*(ss+1) != flag || *ss != flag) { /* data run */
	    data_run = YES;
	    *dd++ = *ss++;
	    *dd++ = *ss++;
	}
	else { /* flag run */
	    data_run = NO;
	    ss += 2;
	}

	for(;ss < end;) { /* for rest of the run */
	    if(data_run) {
		if(*(ss-1) == flag && *ss == flag && kount > 2) {
		    /* break data run
		     */
		    *rlcode = SIGN16 | --kount;
		    wcount += kount+1; /* data plus code word */
		    ss--;
		    dd--;
		    break;
		}
		/* continue the data run */
		kount++;
		*dd++ = *ss++;
	    }
	    else { /* flag run */
		if(*ss != flag) { /* break flag run */
		    *rlcode = kount;
		    wcount++; /* code word only */
		    break;
		}
		ss++;
		kount++; /* continue flag run */
	    }
	}
    }
    /* now look at the last value
     */
    if(data_run) { /* just stuff it no matter what it is */
	if(ss == end) {
	    *dd++ = *ss;
	    kount++;
	}
	*rlcode = SIGN16 | kount;
	wcount += kount +1;
    }
    else if(*ss == flag) {
	*rlcode = ++kount;
	wcount++;
    }
    else { /* there is one last datum at the end of a flag run */
	if(kount == 2) {	/* special case for just two flags */
	    *rlcode = SIGN16 | 3;
	    *dd++ = flag;
	    *dd++ = flag;
	    *dd++ = *ss;
	    wcount += 4;
	}
	else {
	    *rlcode = --kount;
	    wcount++;
	    *dd++ = SIGN16 | 2;
	    *dd++ = flag;
	    *dd++ = *ss;
	    wcount += 3;
	}
    }
    *dd++ = END_OF_COMPRESSION;
    wcount++;
    return(wcount);
}
/* c------------------------------------------------------------------------ */

void dd_copy_pisp(p0, p1)
  struct point_in_space *p0, *p1;
{
    memcpy(p1, p0, p0->sizeof_struct);
}
/* c------------------------------------------------------------------------ */

int dd_datum_size(binary_format)
  int binary_format;
{
    if(binary_format == DD_8_BITS) return(1);
    if(binary_format == DD_16_BITS) return(2);
    if(binary_format == DD_24_BITS) return(3);
    if(binary_format == DD_32_BIT_FP) return(4);
    if(binary_format == DD_16_BIT_FP) return(2);
    return(0);
}
/* c------------------------------------------------------------------------ */

char *dd_delimit(c)
  char *c;
{
    /* advance to next delimiter */
    for(;*c != ' ' && *c != '\t' && *c != '\n' &&
	*c != '<' && *c != '>' && *c != '\0'&& *c != ';'; c++ );
    return(c);
}
/* c------------------------------------------------------------------------ */

double
dd_drift(dgi)
  struct dd_general_info *dgi;
{
    double d=dgi->dds->asib->drift_angle;
    if(dd_isnanf(d))
	  return((double)0);
    else
	  return(d +dgi->dds->cfac->drift_corr);
}
/* c------------------------------------------------------------------------ */

double
dd_earthr(lat)
  double lat;
{
    static double *earth_r=NULL;

    double major=6378388;	/* radius in meters */
    double minor=6356911.946;
    double tt;
    double d, theta=0, x, y;
    int ii, nn;
    
    if(!earth_r) {
	earth_r = (double *)malloc(20*sizeof(double));

	for(ii=0; theta < 90.; ii++, theta += 5.) {
	    /* create an entry every 5 degrees
	     */
	    tt = tan(RADIANS(theta));
	    d = sqrt(1.+SQ(tt*major/minor));
	    x = major/d;
	    y = x*tt;
	    *(earth_r +ii) = sqrt(SQ(x) + SQ(y));
	}
    }
    nn = fabs(lat*.2);
    d = nn < 18 ? *(earth_r +nn) : minor;
    d *= .001;			/* km.! */
    return(d);
}
/* c------------------------------------------------------------------------ */

double
dd_elevation_angle(dgi)
  struct dd_general_info *dgi;
{
    int mark;
    double d, d_rot;
    double AzmR, ElR, PitchR, RollR, z;

    if(dgi->dds->radd->scan_mode == AIR) {
	/* most aircraft data	 */
	d_rot = DEGREES(dgi->dds->ra->elevation);
    }
    else if(dgi->dds->radd->radar_type == AIR_LF) {
	/* this is meant to apply only to the P3 lower fuselage data.
	 * the elevation angle is recorded relative to the aircraft
	 * but the antenna is trying to maintain a constant elevation
	 * relative to the earth
	 *
	 * This code courtesy of Bob Hueftle MRD/NOAA
	 */
	d = dgi->dds->ryib->elevation;

	if(dd_isnanf(d)) {
	    d_rot = 0;
	}
	else {
	    ElR = RADIANS(dgi->dds->ryib->elevation
			  + dgi->dds->cfac->elevation_corr);

	    AzmR = RADIANS(dgi->dds->ryib->azimuth
		+ dgi->dds->cfac->azimuth_corr);
	    PitchR = RADIANS(dd_pitch(dgi));
	    RollR = RADIANS(dd_roll(dgi));
	    
	    z = cos(AzmR)*cos(ElR)*sin(PitchR)
		  + sin(ElR)*cos(PitchR)*cos(RollR)
			- sin(AzmR)*cos(ElR)*cos(PitchR)*sin(RollR);
	    
	    if(z > 1.) z = 1.;
	    else if(z < -1.) z = -1.;
	    d_rot = DEGREES(asin(z));
	}
    }
    else {
	switch(dgi->dds->radd->scan_mode) {
	    
	case TAR:
	case VER:
	default:
	    d = dgi->dds->ryib->elevation;
	    
	    if(dd_isnanf(d)) {
		d_rot = 0;
	    }
	    else {
		d_rot = d + dgi->dds->cfac->elevation_corr;
	    }
	    break;
	}
    }
    return(d_rot);
}
/* c------------------------------------------------------------------------ */

char *dd_find_desc(a, b, desc)
  char *a, *b, *desc;
{
    struct generic_descriptor *gd;
    while(a < b) {
	if(strncmp(a, desc, 4) == 0)
	      return(a);
	gd = (struct generic_descriptor *)a;
	a += gd->sizeof_struct;
    }
    return(0);
}
/* c------------------------------------------------------------------------ */

int
dd_find_field(dgi, name)
  struct dd_general_info *dgi;
  char *name;
{
    int ii=0, nc, nx;
    struct dds_structs *dds=dgi->dds;
    char *aa, *bb;

    if((nc = strlen(name)) < 1)
	  return(-1);

    for(ii=0; ii < MAX_PARMS; ii++) {
	if(!dds->field_present[ii])
	      continue;
	aa = bb = dds->parm[ii]->parameter_name;

	for( nx = 0; nx < 8 && *bb != ' ' && *bb != '\0'; nx++, bb++ );
	if( nx != nc )
	    { continue; }

	if(!strncmp(name, aa, nc)) {
	    /* destination field! */
	    return(ii);
	}
    }
    return(-1);
}
/* c------------------------------------------------------------------------ */

void dd_get_cfac(stream, cfac)
  FILE *stream;
  struct correction_d *cfac;
{
    /* routine to absorb an ascii version of the cfac info and
     * stuff it into the cfac struct
     */
    static int complaint_count = 0;
    int nn, nt;
    char str[256], *aa, *bb;
    char string_space[256], *strptrs[16];
    double d, atof();
    float f;

    for(d = 0; aa = fgets(string_space, 256, stream); d = 0) { /* next line */
	if(*aa == '#' || *aa == '!') /* comment */
	      continue;
	strcpy(string_space, aa);
	if((nt = dd_tokenz(string_space, strptrs, " =\t\n")) < 2) {
	    continue;
	}
	if((nn = sscanf(strptrs[1], "%f", &f)) == 1) {
	    d = f;
	}
	bb = strptrs[0];

	if(strstr(bb, "azimuth_corr")) {
	    cfac->azimuth_corr = d;
	}
	else if(strstr(bb, "elevation_corr")) {
	    cfac->elevation_corr = d;
	}
	else if(strstr(bb, "range_delay_corr")) {
	    cfac->range_delay_corr = d;
	}
	else if(strstr(bb, "longitude_corr")) {
	    cfac->longitude_corr = d;
	}
	else if(strstr(bb, "latitude_corr")) {
	    cfac->latitude_corr = d;
	}
	else if(strstr(bb, "pressure_alt_corr")) {
	    cfac->pressure_alt_corr = d;
	}
	else if(strstr(bb, "radar_alt_corr")) {
	    cfac->radar_alt_corr = d;
	}
	else if(strstr(bb, "ew_gndspd_corr")) {
	    cfac->ew_gndspd_corr = d;
	}
	else if(strstr(bb, "ns_gndspd_corr")) {
	    cfac->ns_gndspd_corr = d;
	}
	else if(strstr(bb, "vert_vel_corr")) {
	    cfac->vert_vel_corr = d;
	}
	else if(strstr(bb, "heading_corr")) {
	    cfac->heading_corr = d;
	}
	else if(strstr(bb, "roll_corr")) {
	    cfac->roll_corr = d;
	}
	else if(strstr(bb, "pitch_corr")) {
	    cfac->pitch_corr = d;
	}
	else if(strstr(bb, "drift_corr")) {
	    cfac->drift_corr = d;
	}
	else if(strstr(bb, "rot_angle_corr")) {
	    cfac->rot_angle_corr = d;
	}
	else if(strstr(bb, "tilt_corr")) {
	    cfac->tilt_corr = d;
	}
	else {
	  if (complaint_count++ < 8)
	    { printf("%s\n**** Not a usable correction factor ****\n", aa); }
	}
    }
}
/* c------------------------------------------------------------------------ */

void dd_get_difs(difs)
  struct dd_input_filters *difs;
{
    int ii, jj, kk, nn, nt, sizeof_env=K64, num_ptrs=999;
    char *a, *bb, *get_tagged_string(), str[2048], *str_ptrs[99];
    char *env=NULL, **env_ptrs;
    DD_TIME dts;
    double d, t1=0, t2=0, d_time_stamp(), atof();


    difs->final_stop_time = MAX_FLOAT;
    env = (char *)malloc(sizeof_env);
    memset(env, 0, sizeof_env);
    env_ptrs = (char **)malloc(num_ptrs * sizeof(char *));
    memset(env_ptrs, 0, num_ptrs * sizeof(char *));

    if(a=get_tagged_string("TIME_LIMITS")) {
	strcpy(env, a);
	nt = dd_tokens(env, env_ptrs);
	for(ii=0; ii < nt; ii += 3, t1=t2=0) {
	    bb = *(env_ptrs +ii);
	    if(dd_crack_datime(bb, strlen(bb), &dts)) {
		t1 = d_time_stamp(&dts);
	    }
	    kk = nt == 2 ? 1 : ii+2;
	    /* possibly just two arguemnts
	     * otherwise assume a seperater argument
	     */
	    bb = *(env_ptrs +kk);
	    if(kk < nt && dd_crack_datime(bb, strlen(bb), &dts)) {
		t2 = d_time_stamp(&dts);
	    }
	    if(t1 > 0) {
		if(t2 > 0 && t2 < t1) {d = t2; t2 = t1; t1 = d;}
		jj = difs->num_time_lims++;
		dd_reset_d_limits(&difs->times[jj]);
		difs->times[jj]->lower = t1;
	    }
	    if(t2 > 0)
		difs->times[jj]->upper = t2;

	    if(difs->num_time_lims)
		  printf("Time limits: %.0f - %.0f\n"
			 , difs->times[jj]->lower
			 , difs->times[jj]->upper);
	}
	if(difs->num_time_lims) {
	    difs->final_stop_time = difs->times[0]->upper;
	    for(ii=1; ii < difs->num_time_lims; ii++ ) {
		if(difs->final_stop_time < difs->times[ii]->upper)
		      difs->final_stop_time = difs->times[ii]->upper;
	    }
	}
    }

    if(a=get_tagged_string("SWEEP_MODES")) {
	strcpy(str, a);
	nt = dd_tokens(str, str_ptrs);
	for(ii=0; ii < nt; ii++) {
	    for(jj=0; jj < MAX_MODES; jj++) {
		if(strncmp(str_ptrs[ii], dd_sweep_modes[jj], 3) == 0) {
		    difs->num_modes++;
		    difs->modes[jj] = YES;
		    break;
		}

	    }
	}
    }
    if(a=get_tagged_string("ALTITUDE_LIMITS")) {
	strcpy(str, a);
	nt = dd_tokens(str, str_ptrs);
	str_ptrs[nt] = NULL;

	dd_get_lims(str_ptrs, 0, nt, &difs->altitude_limits);
	difs->altitude_truncations = YES;

	/* assume input in km and convert to meters */
	difs->altitude_limits->lower *= 1000.;
	difs->altitude_limits->upper *= 1000.;

	printf("Altitude limits: %.0f - %.0f\n"
	       , difs->altitude_limits->lower
	       , difs->altitude_limits->upper);
    }
    if(a=get_tagged_string("FIXED_ANGLES")) {
	strcpy(str, a);
	nt = dd_tokens(str, str_ptrs);
	for(ii=0; ii < nt; ii += 3) {
	    jj = difs->num_fixed_lims++;
	    dd_get_lims(str_ptrs, ii, nt, &difs->fxd_angs[jj]);
	    printf("Fixed angle limits: %.1f - %.1f\n"
		   , difs->fxd_angs[jj]->lower
		   , difs->fxd_angs[jj]->upper);
	}
    }
    if(a=get_tagged_string("PRF_LIMITS")) {
	strcpy(str, a);
	nt = dd_tokens(str, str_ptrs);
	for(ii=0; ii < nt; ii += 3) {
	    jj = difs->num_prf_lims++;
	    dd_get_lims(str_ptrs, ii, nt, &difs->prfs[jj]);
	    printf("PRF limits: %.0f - %.0f\n"
		   , difs->prfs[jj]->lower
		   , difs->prfs[jj]->upper);
	}
    }
    if(a=get_tagged_string("RANGE_LIMITS")) {
	strcpy(str, a);
	nt = dd_tokens(str, str_ptrs);
	for(ii=0; ii < nt; ii += 3) {
	    jj = difs->num_ranges++;
	    dd_get_lims(str_ptrs, ii, nt, &difs->ranges[jj]);
	    printf("Range limits: %.3f - %.3f\n"
		   , difs->ranges[jj]->lower
		   , difs->ranges[jj]->upper);
	}
    }
    if(a=get_tagged_string("XOUT_AZIMUTHS")) {
	strcpy(str, a);
	nt = dd_tokens(str, str_ptrs);
	for(ii=0; ii < nt; ii += 3) {
	    jj = difs->num_az_xouts++;
	    dd_get_lims(str_ptrs, ii, nt, &difs->xout_azs[jj]);
	    printf("Xout azimuth limits: %.3f - %.3f\n"
		   , difs->xout_azs[jj]->lower
		   , difs->xout_azs[jj]->upper);
	}
    }
    if(a=get_tagged_string("XOUT_ELEVATIONS")) {
	strcpy(str, a);
	nt = dd_tokens(str, str_ptrs);
	for(ii=0; ii < nt; ii += 3) {
	    jj = difs->num_el_xouts++;
	    dd_get_lims(str_ptrs, ii, nt, &difs->xout_els[jj]);
	    printf("Xout elevation limits: %.3f - %.3f\n"
		   , difs->xout_els[jj]->lower
		   , difs->xout_els[jj]->upper);
	}
    }
    if(a=get_tagged_string("AZ_SECTORS")) {
	strcpy(str, a);
	nt = dd_tokens(str, str_ptrs);
	for(ii=0; ii < nt; ii += 3) {
	    jj = difs->num_az_sectors++;
	    dd_get_lims(str_ptrs, ii, nt, &difs->azs[jj]);
	    printf("Azimuth sector: %.3f - %.3f\n"
		   , difs->azs[jj]->lower
		   , difs->azs[jj]->upper);
	}
    }
    if(a=get_tagged_string("EL_SECTORS")) {
	strcpy(str, a);
	nt = dd_tokens(str, str_ptrs);
	for(ii=0; ii < nt; ii += 3) {
	    jj = difs->num_el_sectors++;
	    dd_get_lims(str_ptrs, ii, nt, &difs->els[jj]);
	    printf("Elevation sector: %.3f - %.3f\n"
		   , difs->els[jj]->lower
		   , difs->els[jj]->upper);
	}
    }
# ifdef obsolete
    if(a=get_tagged_string("MIN_FREE_MB")) {
	if((d = atof(a)) > 0) 
	      difs->min_free_MB = d;
    }
# endif
    if(a=get_tagged_string("SWEEP_SKIP")) {
	if((ii = atoi(a)) > 0) 
	      difs->sweep_skip = ii+1;
    }
    if(a=get_tagged_string("BEAM_SKIP")) {
	if((ii = atoi(a)) > 0) 
	      difs->beam_skip = ii+1;
    }
    if(a=get_tagged_string("VOLUME_COUNT")) {
	if((ii = atoi(a)) > 0) 
	      difs->max_vols = ii;
    }
    if(a=get_tagged_string("SWEEP_COUNT")) {
	if((ii = atoi(a)) > 0) 
	      difs->max_sweeps = ii;
    }
    if(a=get_tagged_string("BEAM_COUNT")) {
	if((ii = atoi(a)) > 0) 
	      difs->max_beams = ii;
    }
    if(a=get_tagged_string("RUN_TIME")) {
	if((ii = atoi(a)) > 0) 
	      difs->run_time = ii;
    }
    if(a=get_tagged_string("COMPRESSION_SCHEME")) {
	if(strstr(a, "HRD") || strstr(a, "RLE")) 
	      difs->compression_scheme = HRD_COMPRESSION;
	if(strstr(a, "RDP_8")) 
	      difs->compression_scheme = RDP_8_BIT_COMPRESSION;
    }
    if(a=get_tagged_string("OPTIONS")) {
	if(strstr(a,"ABSOLUTE_TIME")) {
	    difs->abrupt_start_stop = YES;
	}
    }
    if(a=get_tagged_string("OUTPUT_FLAGS")) {
	if(strstr(a,"CATALOG_ONLY")) {
	    difs->catalog_only = YES;
	    dd_output_flag(NO);
	}
	if(strstr(a,"DORADE_DAT") ||
	   strstr(a,"SWEEP_FIL"))
	      dd_output_flag(YES);
    }
    free(env);
    free(env_ptrs);
    return;
}
/* c------------------------------------------------------------------------ */

void dd_get_lims(str_ptrs, ii, nt, dlims)
  char *str_ptrs[];
  int ii, nt;
  struct d_limits **dlims;
{
    /* extract the next set of limits
     * either two numbers seperated by a string like "<" or "-" or
     * a single number
     */

    int kk;
    float f, f1=-MAX_FLOAT, f2=MAX_FLOAT;

    dd_reset_d_limits(dlims);

    if(ii < nt) {
	sscanf(str_ptrs[ii], "%f", &f1);
    }
    kk = nt == 2 ? 1 : ii+2;
    if(kk < nt) {
	sscanf(str_ptrs[kk], "%f", &f2);
    }

    (*dlims)->lower = f1;
    (*dlims)->upper = f2;
    return;
}
/* c------------------------------------------------------------------------ */

int dd_givfld( dgi, ndx, g1, n, dd, badval )
  DGI_PTR dgi;
  int *ndx, *g1, *n;
  float *dd, *badval;
{
    /*
     * dgi: dd_general_info struct
     * ndx: the index of the desired field
     * g1:  the gate number of where to start (1 => the first gate)
     * n:   the number of gates desired
     * dd:  the destination array for the floating point values
     * bad_val: what the bad data flag looks like in floating point
     */
    DDS_PTR dds=dgi->dds;
    static int err_count=0;
    unsigned char *a;
    char *buf, *deep6=0;
    float bias;
    short gate[MAXCVGATES];
    int bad, i, idx= *ndx, m, df, dd_hrd16(), mark;
    int gndx= *g1-1, ng=dgi->dds->celv->number_cells;
    float rcp_scale, *ff;
    short *ss;
    
    if(idx < 0 || idx >= dgi->num_parms) {
	if(++err_count > 11) {
	    i = *deep6;
	}
	return((int)0);
    }
    bias = dds->parm[idx]->parameter_bias;
# ifdef NEW_ALLOC_SCHEME
    buf = dds->qdat_ptrs[idx];
# else
    buf = (char *)dds->rdat[idx] + sizeof(struct paramdata_d);
# endif
    rcp_scale = 1./dds->parm[idx]->parameter_scale;
    *badval = DD_UNSCALE((float)dds->parm[idx]->bad_data, rcp_scale, bias);
    bad = dds->parm[idx]->bad_data;
    ng = gndx+(*n) < ng ? *n : ng-gndx;

    df = dds->parm[idx]->binary_format;

    if( df == DD_8_BITS ) { 
	a = (unsigned char *)buf;
	for(a+=gndx,i=0; i < ng; i++,a++,dd++){
	    if(*a == bad)
		  *dd = *badval;
	    else
		  *dd = DD_UNSCALE((float)(*a), rcp_scale, bias);
	}
    }
    else if( df == DD_16_BITS ) { 
	ss = (short *)buf;
	for(ss+=gndx,i=0; i < ng; i++,ss++,dd++){
	    if(*ss == bad)
		  *dd = *badval;
	    else
		  *dd = DD_UNSCALE((float)(*ss), rcp_scale, bias);
	}
    }
    else if( df == DD_24_BITS ) { 
    }
    else if( df == DD_32_BIT_FP ) { 
	ff = (float *)buf;
	ff += gndx;
	memcpy((char*)dd, (char *)ff, ng*sizeof(float));
    }
    return(ng);
}
/* c------------------------------------------------------------------------ */

double
dd_heading(dgi)
  struct dd_general_info *dgi;
{
    double d=dgi->dds->asib->heading;
    if(dd_isnanf(d))
	  return((double)0);
    else
	  return(d +dgi->dds->cfac->heading_corr);
}
/* c------------------------------------------------------------------------ */

struct dd_general_info *
dd_ini(rn, radar_name)
  int rn;
  char *radar_name;
{
    /*
     * the purpose of this routine is to initialize
     * for producing dorade data
     */
    int i, jj, k, nn, isize, sod=sizeof(double), ssize;
    static int sizeof_dgi=0;
    long time_now();
    char *a, *c, *get_tagged_string(), *dd_align_structs();
    struct dd_input_filters *difs, *dd_return_difs_ptr();
    struct dd_general_info *dgi;
    struct dds_structs *dds;
    struct volume_d *vold;
    struct radar_d *radd, *dd_malloc_radd();
    struct comment_d *comm;
    struct correction_d *cfac;
    struct cell_d *celv, *celvc;
    struct super_SWIB *sswb;
    struct sweepinfo_d *swib;
    struct ray_i *ryib;
    struct platform_i *asib;
    struct lidar_d *lidr;
    struct prev_rays *pr, *pr_last;
    struct prev_swps *ps, *ps_last;
    DD_TIME xdts, *d_unstamp_time();
    struct cfac_wrap *cfw, *ddswp_nab_cfacs ();


    if(!sizeof_dgi) {
    	isize = sizeof(struct dd_general_info);
	isize += sizeof(struct dds_structs);
	isize += MAX_PREV_RAYS*sizeof(struct prev_rays);
	isize += MAX_PREV_SWPS*sizeof(struct prev_swps);
	isize += sizeof(struct platform_i);
	isize += sizeof(struct cell_d);
	isize += sizeof(struct cell_d);	/* corrected cell vector */
	isize += sizeof(struct cell_spacing_d);
	isize += sizeof(struct comment_d);
	isize += sizeof(struct correction_d);
	isize += sizeof(struct d_time_struct);
	isize += MAX_REC_SIZE; /* field_parameter_data */
	isize += MAX_PARMS*sizeof(struct parameter_d);
	isize += MAX_PARMS*
	      (sizeof(struct paramdata_d)+MAXCVGATES*sizeof(float));
	isize += sizeof(struct radar_angles);
	isize += sizeof(struct radar_d);
	isize += sizeof(struct ray_i);
	isize += sizeof(struct super_SWIB);
	isize += sizeof(struct sweepinfo_d);
	isize += sizeof(struct volume_d);
	isize += sizeof(struct null_d);
	isize += sizeof(struct lidar_d);
	isize += sizeof(struct lidar_parameter_data);

	isize += MAX_REC_SIZE; /* volume header */
	isize += MAX_READ;	/* data record */
	sizeof_dgi = (float)isize*1.01;
    }   

# ifdef NEW_ALLOC_SCHEME
# else
# endif

# ifdef NEW_ALLOC_SCHEME
    if(dgi = MALLOC_S(struct dd_general_info))
	  memset(dgi, 0, sizeof(struct dd_general_info));
    else {
	printf("Unable to malloc dgi struct for: %s %d\n"
	       , radar_name, rn);
	exit(1);
    }
# else
    dgi = (struct dd_general_info *)malloc(sizeof_dgi);
    if(!dgi) {
	printf("Unable to malloc dgi struct for: %s %d\n"
	       , radar_name, rn);
	exit(1);
    }
    c = (char *)dgi;
    memset(c, 0, sizeof_dgi);	/* initialize everything to zero */
    c = dd_align_structs(c, sizeof(struct dd_general_info));
# endif

    dgi->prev_scan_mode = dgi->prev_vol_num = -1;
    dgi->sizeof_dgi = sizeof_dgi;
    dgi->radar_num = rn;
    str_terminate(dgi->radar_name, radar_name, 8 );
    if(a=get_tagged_string("DORADE_DIR"))
	  slash_path(dgi->directory_name, a);
    difs = dd_return_difs_ptr();
    dgi->compression_scheme = difs->compression_scheme;
    dgi->disk_output = dd_output_flag(EMPTY_FLAG);

    /* all the dorade struct pointers */
# ifdef NEW_ALLOC_SCHEME
    dds = dgi->dds = MALLOC_S(struct dds_structs);
    memset(dds, 0, sizeof(struct dds_structs));
# else
    dds = dgi->dds = (struct dds_structs *)c;
    c = dd_align_structs(c, sizeof(struct dds_structs));
# endif

    /* time struct */
# ifdef NEW_ALLOC_SCHEME
    dds->dts = MALLOC_S(struct d_time_struct);
    memset(dds->dts, 0, sizeof(struct d_time_struct));
# else
    dds->dts = (struct d_time_struct *)c;
    c = dd_align_structs(c, sizeof(struct d_time_struct));
# endif

    /* radar angles struct */
# ifdef NEW_ALLOC_SCHEME
    dds->ra = MALLOC_S(struct radar_angles);
    memset(dds->ra, 0, sizeof(struct radar_angles));
# else
    dds->ra = (struct radar_angles *)c;
    c = dd_align_structs(c, sizeof(struct radar_angles));
# endif

    /* establish stacks of previous ray and sweep info */
    for(i=0; i < MAX_PREV_RAYS; i++){
# ifdef NEW_ALLOC_SCHEME
	pr = MALLOC_S(struct prev_rays);
	memset(pr, 0, sizeof(struct prev_rays));
# else
	pr = (struct prev_rays *)c;
# endif
	if(!i) {
	    dgi->ray_que = pr;
	}
	else {
	    pr_last->next = pr;
	    pr->last = pr_last;
	}
	pr->next = dgi->ray_que;
	dgi->ray_que->last = pr_last = pr;
	c = dd_align_structs(c, sizeof(struct prev_rays));
    }

    for(i=0; i < MAX_PREV_SWPS; i++){
# ifdef NEW_ALLOC_SCHEME
	ps = MALLOC_S(struct prev_swps);
	memset(ps, 0, sizeof(struct prev_swps));
# else
	ps = (struct prev_swps *)c;
# endif
	if(!i)
	      dgi->swp_que = ps;
	else {
	    ps_last->next = ps;
	    ps->last = ps_last;
	}
	ps->next = dgi->swp_que;
	dgi->swp_que->last = ps_last = ps;
	c = dd_align_structs(c, sizeof(struct prev_swps));
    }

    /* establish all the other headers */
    /* volume header */

# ifdef NEW_ALLOC_SCHEME
    vold = dds->vold = MALLOC_S(struct volume_d);
    memset(vold, 0, sizeof(struct volume_d));
# else
    vold = dds->vold = (struct volume_d *)c;
    c = dd_align_structs(c, sizeof(struct volume_d));
# endif

    strncpy( vold->volume_des, "VOLD", 4 );
    vold->volume_des_length = sizeof( struct volume_d );
    vold->format_version = 1;
    vold->maximum_bytes = MAX_REC_SIZE;

    if(a=get_tagged_string("GEN_FACILITY")) {
	  strncpy(vold->gen_facility, "         ", 8 );
	  jj = strlen(a) > 8 ? 8 : strlen(a);
	  strncpy(vold->gen_facility, a, jj);
    }
    else
	  strncpy( vold->gen_facility, "NCAR/ATD   ", 8 );

    xdts.time_stamp = time_now();
    d_unstamp_time(&xdts);
    vold->gen_year = xdts.year;
    vold->gen_month = xdts.month;
    vold->gen_day = xdts.day;
    vold->number_sensor_des = 1;

    /* radar descriptor */
# ifdef NEW_ALLOC_SCHEME
    /*
    radd = dds->radd = MALLOC_S(struct radar_d);
    memset(radd, 0, sizeof(struct radar_d));
     */
    radd  = dds->radd = dd_malloc_radd(dgi, "");
# else
    radd = dds->radd = (struct radar_d *)c;
    c = dd_align_structs(c, sizeof(struct radar_d));
    strncpy( radd->radar_des, "RADD", 4 );
    radd->radar_des_length = sizeof(struct radar_d);
# endif

    /* lidar descriptor */
# ifdef NEW_ALLOC_SCHEME
    lidr = dds->lidr = MALLOC_S(struct lidar_d);
    memset(lidr, 0, sizeof(struct lidar_d));
# else
    lidr = dds->lidr = (struct lidar_d *)c;
    c = dd_align_structs(c, sizeof(struct lidar_d));
# endif

    strncpy(lidr->lidar_des, "LIDR", 4 );
    lidr->lidar_des_length = sizeof(struct lidar_d);

    /* correction factors */
# ifdef NEW_ALLOC_SCHEME
    cfac = dds->cfac = MALLOC_S(struct correction_d);
    memset(cfac, 0, sizeof(struct correction_d));
# else
    cfac = dds->cfac = (struct correction_d *)c;
    c = dd_align_structs(c, sizeof(struct correction_d));
# endif

    strncpy( cfac->correction_des, "CFAC", 4 );
    cfac->correction_des_length = sizeof(*cfac);
    dgi->dds->first_cfac = 0;

    if (strlen(radar_name)) {
       cfw = ddswp_nab_cfacs (radar_name);
       if (cfw) {
	  dgi->dds->first_cfac = cfw;
	  dgi->ignore_cfacs = YES;
       }
    }

    /* parameter descriptors */
    for(i=0; i < MAX_PARMS; i++ ) {
# ifdef NEW_ALLOC_SCHEME
	dds->parm[i] = MALLOC_S(struct parameter_d);
	memset(dds->parm[i], 0, sizeof(struct parameter_d));
# else
	dds->parm[i] = (struct parameter_d *)c;
	c = dd_align_structs(c, sizeof(struct parameter_d));
# endif
	strncpy( dds->parm[i]->parameter_des, "PARM", 4 );
	dds->parm[i]->parameter_des_length = sizeof( struct parameter_d );
	dds->parm[i]->binary_format = DD_16_BITS;
	dds->parm[i]->bad_data = DELETE_FLAG;
    }

    /* parameter data */
    for(i=0; i < MAX_PARMS; i++ ) {
# ifdef NEW_ALLOC_SCHEME
	dds->qdat[i] = MALLOC_S(struct qparamdata_d);
	memset(dds->qdat[i], 0, sizeof(struct qparamdata_d));
	strncpy(dds->qdat[i]->pdata_desc, "RDAT", 4 );
	/* initially we assume we will be writing an RDAT type descripter.
	 * constructors for fields requiring the QDAT struct
	 * should change the name to QDAT
	 */
# else
	dds->rdat[i] = (struct paramdata_d *)c;
	c = dd_align_structs
	      (c, MAXCVGATES*sizeof(float)+sizeof(struct paramdata_d));
	dds->qdat_ptrs[i] = (char *)dds->rdat[i] +
	      sizeof(struct paramdata_d);
	strncpy( dds->rdat[i]->pdata_desc, "RDAT", 4 );
# endif
    }

    /* comments */
# ifdef NEW_ALLOC_SCHEME
    comm = dds->comm = MALLOC_S(struct comment_d);
    memset(comm, 0, sizeof(struct comment_d));
# else
    comm = dds->comm = (struct comment_d *)c;
    c = dd_align_structs(c, sizeof(struct comment_d));
# endif
    strncpy( comm->comment_des, "COMM", 4 );
    comm->comment_des_length = sizeof( struct comment_d );
    strcpy(comm->comment, "\nDORADE\n");
    strcat(comm->comment
	   , "DOppler RAdar Data Exchange format\n");

    /* cell spacing vector */
    nn = sizeof(struct cell_d) + 2 * MAXCVGATES * sizeof(float);

# ifdef NEW_ALLOC_SCHEME
    celv = dds->celv = (struct cell_d *)malloc(nn);
    memset(celv, 0, nn);
    celv->number_cells = MAXCVGATES;
# else
    celv = dds->celv = (struct cell_d *)c;
    c = dd_align_structs(c, sizeof(struct cell_d));
# endif
    strncpy( celv->cell_spacing_des, "CELV", 4 );
    celv->cell_des_len = sizeof( struct cell_d );

    /* corrected cell spacing vector */
# ifdef NEW_ALLOC_SCHEME
    celvc = dds->celvc = (struct cell_d *)malloc(nn);
    memcpy( celvc, celv, nn );
# else
    celvc = dds->celvc = (struct cell_d *)c;
    c = dd_align_structs(c, sizeof(struct cell_d));
    strncpy( celvc->cell_spacing_des, "CELV", 4 );
    celvc->cell_des_len = sizeof( struct cell_d );
# endif

    /* field cell spacing info */
# ifdef NEW_ALLOC_SCHEME
    dds->cspd = MALLOC_S(struct cell_spacing_d);
    memset(dds->cspd, 0, sizeof(struct cell_spacing_d));
# else
    dds->cspd = (struct cell_spacing_d *)c;
    c = dd_align_structs(c, sizeof(struct cell_spacing_d));
# endif


    /* super sweep info block */
# ifdef NEW_ALLOC_SCHEME
    sswb = dds->sswb = MALLOC_S(struct super_SWIB);
    memset(sswb, 0, sizeof(struct super_SWIB));
# else
    sswb = dds->sswb = (struct super_SWIB *)c;
    c = dd_align_structs(c, sizeof(struct super_SWIB));
# endif
    strncpy( sswb->name_struct, "SSWB", 4 );
    sswb->sizeof_struct = sizeof( struct super_SWIB );

    /* sweep info block */
# ifdef NEW_ALLOC_SCHEME
    swib = dds->swib = MALLOC_S(struct sweepinfo_d);
    memset(swib, 0, sizeof(struct sweepinfo_d));
# else
    swib = dds->swib = (struct sweepinfo_d *)c;
    c = dd_align_structs(c, sizeof(struct sweepinfo_d));
# endif
    strncpy( swib->sweep_des, "SWIB", 4 );
    swib->sweep_des_length = sizeof( struct sweepinfo_d );

    /* ray info block */
# ifdef NEW_ALLOC_SCHEME
    ryib = dds->ryib = MALLOC_S(struct ray_i);
    memset(ryib, 0, sizeof(struct ray_i));
# else
    ryib = dds->ryib = (struct ray_i *)c;
    c = dd_align_structs(c, sizeof(struct ray_i));
# endif
    strncpy( ryib->ray_info, "RYIB", 4 );
    ryib->ray_info_length = sizeof( struct ray_i );

    /* platform info block */
# ifdef NEW_ALLOC_SCHEME
    asib = dds->asib = MALLOC_S(struct platform_i);
    memset(asib, 0, sizeof(struct platform_i));
# else
    asib = dds->asib = (struct platform_i *)c;
    c = dd_align_structs(c, sizeof(struct platform_i));
# endif
    strncpy( asib->platform_info, "ASIB", 4 );
    asib->platform_info_length = sizeof( struct platform_i );

    /* field parameter data */
# ifdef NEW_ALLOC_SCHEME
# else
    dds->frad = (struct field_parameter_data *)c;
    dds->raw_data = c + sizeof(struct field_parameter_data);
    c = dd_align_structs(c, MAX_REC_SIZE);
    strncpy( dds->frad->field_param_data, "FRAD", 4);
# endif

    /* lidar parameter data */
# ifdef NEW_ALLOC_SCHEME
    dds->ldat = MALLOC_S(struct lidar_parameter_data);
    memset(dds->ldat, 0, sizeof(struct lidar_parameter_data));
# else
    dds->ldat = (struct lidar_parameter_data *)c;
    c = dd_align_structs(c, sizeof(struct lidar_parameter_data));
# endif
    strncpy(dds->ldat->lidar_param_data, "LDAT", 4);

    /* the NULL descriptor */
# ifdef NEW_ALLOC_SCHEME
    dds->NULL_d = MALLOC_S(struct null_d);
    memset(dds->NULL_d, 0, sizeof(struct null_d));
# else
    dds->NULL_d = (struct null_d *)c;
    c = dd_align_structs(c, sizeof(struct null_d));
# endif
    strncpy(dds->NULL_d->name_struct, "NULL", 4 );
    dds->NULL_d->sizeof_struct = sizeof(struct null_d);

# ifdef NEW_ALLOC_SCHEME
# else
    dgi->dd_vol_buf = c;
    c = dd_align_structs(c, MAX_REC_SIZE);
# endif

# ifdef NEW_ALLOC_SCHEME
# else
    dgi->dd_buf = c;
    c = dd_align_structs(c, MAX_READ);
# endif

# ifdef NEW_ALLOC_SCHEME
# else
# endif

# ifdef obsolete
    /* do format dependent stuff */
    dd_ini_unique(dds);
# endif

    return(dgi);
}
/* c------------------------------------------------------------------------ */

void dd_input_strings()
{
    char *a, *getenv(), *put_tagged_strings();

    /* c...mark */

# ifdef obsolete
    if(a=getenv("")) {
	put_tagged_string("", a);
    }
# endif

    if(a=getenv("A_SPECKLE")) {
	/* if a run of cells less than or equal to this
	 * number is surrounded by bad flagged data,
	 * the run is also bad flagged
	 */
	put_tagged_string("A_SPECKLE", a);
    }
    if(a=getenv("AC_NETCDF_ALIASES")) {
	put_tagged_string("AC_NETCDF_ALIASES", a);
    }
    if(a=getenv("AC_NETCDF_FILES")) {
	put_tagged_string("AC_NETCDF_FILES", a);
    }
    if(a=getenv("AC_TIME_CORRECTION")) {
	put_tagged_string("AC_TIME_CORRECTION", a);
    }
    if(a=getenv("ADS_START_DATE")) {
	put_tagged_string("ADS_START_DATE", a);
    }
    if(a=getenv("ADS_IO_TYPE")) {
	put_tagged_string("ADS_IO_TYPE", a);
    }
    if(a=getenv("ADS_TAPES")) {
	/* Electra raw data tapes */
	put_tagged_string("ADS_TAPES", a);
    }
    if(a=getenv("AFT_ANGLE_LIMITS")) {
	put_tagged_string("AFT_ANGLE_LIMITS", a);
    }
    if(a=getenv("AFT_REFL_CORR")) {
	put_tagged_string("AFT_REFL_CORR", a);
    }
    if(a=getenv("ALTITUDE_LIMITS")) {
	/* upper and lower limits for altitude clipping */
	put_tagged_string("ALTITUDE_LIMITS", a);
    }
    if(a=getenv("ASCENDING_ONLY")) {
	/* rejects non-ascending fixed angles */
	put_tagged_string("ASCENDING_ONLY", a);
    }
    if(a=getenv("AZ_SECTORS")) {
	put_tagged_string("AZ_SECTORS", a);
    }
    if(a=getenv("BATCH_MODE")) {
	put_tagged_string("BATCH_MODE", a);
    }
    if(a=getenv("BEAM_COUNT")) {
	/* stop after reading this number of beams */
	put_tagged_string("BEAM_COUNT", a);
    }
    if(a=getenv("BEAM_SKIP")) {
	/* only for ELDORA */
	put_tagged_string("BEAM_SKIP", a);
    }
    if(a=getenv("BIAS")) {
	put_tagged_string("BIAS", a);
    }
    if(a=getenv("CAPPI_HEIGHT")) {
	/* altitude msl in meters of the cappi plane */
	put_tagged_string("CAPPI_HEIGHT",a);
    }
    if(a=getenv("CAPPI_LATITUDES")) {
	put_tagged_string("CAPPI_LATITUDES", a);
    }
    if(a=getenv("CAPPI_LEVELS")) {
	put_tagged_string("CAPPI_LEVELS", a);
    }
    if(a=getenv("CAPPI_LONGITUDES")) {
	put_tagged_string("CAPPI_LONGITUDES", a);
    }
    if(a=getenv("CAPPI_RADAR")) {
	put_tagged_string("CAPPI_RADAR", a);
    }
    if(a=getenv("CAPPI_TIMES")) {
	put_tagged_string("CAPPI_TIMES", a);
    }
    if(a=getenv("CAPPI_RANGE_RES")) { /* resolution in meters */
	put_tagged_string("CAPPI_RANGE_RES",a);
    }
    if(a=getenv("CAPPI_REFL")) { /* source field name */
	put_tagged_string("CAPPI_REFL",a);
    }
    if(a=getenv("CAPPI_RESOLUTION")) {
	put_tagged_string("CAPPI_RESOLUTION", a);
    }
    if(a=getenv("CAPPI_VEL")) {
	put_tagged_string("CAPPI_VEL",a);
    }
    if(a=getenv("CAPPI_X_AXIS")) {
	put_tagged_string("CAPPI_X_AXIS", a);
    }
    if(a=getenv("CAPPI_Y_AXIS")) {
	put_tagged_string("CAPPI_Y_AXIS", a);
    }
    if(a=getenv("CATALOG_FLUSH_COUNT")) {
	/* the catalog routine writes the info for a volume when
	 * it is complete but Unix may not actually write to the disk
	 * this option forces a disk write if the number of scans
	 * since the last flush exceeds this value
	 */
	put_tagged_string("CATALOG_FLUSH_COUNT",a);
    }
    if(a=getenv("CATALOG_SWEEP_SUMMARY")) {
	/* causes catalog scan info to cover at least the time span
	 * in seconds if it's aircraft data
	 * otherwise there is one catalog entry per scan
	 */
	put_tagged_string("CATALOG_SWEEP_SUMMARY",a);
    }
    if(a=getenv("CELL_VECTOR_BIAS")) {
	/* the value in meters that will be subtracted from
	 * each of the dist_cells
	 */
	put_tagged_string("CELL_VECTOR_BIAS",a);
    }
    if(a=getenv("CFAC_FILES")) {
	put_tagged_string("CFAC_FILES", a);
    }
    if( a=getenv("CFAC_FILE")) {
	put_tagged_string("CFAC_FILES", a);
    }
    if(a=getenv("ELD_CFAC_FILES")) {
      put_tagged_string("ELD_CFAC_FILES", a);
    }
    if(a=getenv("ELD_CFAC_FILE")) {
	put_tagged_string("ELD_CFAC_FILES", a);
    }
    if(a=getenv("COMPRESSION_SCHEME")) {
	put_tagged_string("COMPRESSION_SCHEME",a);
    }
    if(a=getenv("DAP_HEADER_FILE")) {
	put_tagged_string("DAP_HEADER_FILE", a);
    }
    if(a=getenv("DAP_DATA_FILE")) {
	put_tagged_string("DAP_DATA_FILE", a);
    }
    if((a=getenv("DD_DIR")) || (a=getenv("DORADE_DIR"))) {
	/* the directory where swp, vol, and cat files
	 * will be written
	 */
	put_tagged_string("DORADE_DIR", a);
	put_tagged_string("DD_DIR", a);
    }
    if(a=getenv("DERIVED_FIELDS")) {
	put_tagged_string("DERIVED_FIELDS", a);
    }
    if(a=getenv("DORADE_DEV")) {
	/* the device name if not a standard named file
	 * to which DORADE tape format data is written
	 */
	put_tagged_string("DORADE_DEV",a);
    }
    if(a=getenv("DORADE_IO_TYPE")) {
	/* io types include PHYSICAL_TAPE, BINARY_IO, and
	 * FB_IO which stands for fortran-binary io
	 * which are the kind files understood by todisk
	 * and other RDP software
	 */
	put_tagged_string("DORADE_IO_TYPE",a);
    }
    if(a=getenv("DORADE_VOLUME_INTERVAL")) {
	/* specifies the maximum time in seconds that
	 * a DORADE volume can span
	 */
	put_tagged_string("DORADE_VOLUME_INTERVAL", a);
    }
    if(a=getenv("DRIFT_REFERENCE_TIME")) {
	put_tagged_string("DRIFT_REFERENCE_TIME", a);
    }
    if(a=getenv("EL_SECTORS")) {
	put_tagged_string("EL_SECTORS", a);
    }
    if(a=getenv("ELDORA_VOLUME_HEADER")) {
	/* specifies the complete file name of a surrogate
	 * volume header record
	 */
	put_tagged_string("ELDORA_VOLUME_HEADER",a);
    }
    if(a=getenv("FIRST_GOOD_GATE")) {
	/* causes bad flags to be inserted in the first n gates
	 * during the thresholding process
	 */
	put_tagged_string("FIRST_GOOD_GATE", a);
    }
    if(a=getenv("FIXED_ANGLES")) {
	/* specified as "lower < upper"
	 * if only one number is present it is considered as lower
	 * and upper becomes a very large number
	 */
	put_tagged_string("FIXED_ANGLES", a);
    }
    if(a=getenv("FLIGHT_ID")) {
	put_tagged_string("FLIGHT_ID", a);
    }
    if(a=getenv("FOF_OUTPUT_FIELDS")) {
	put_tagged_string("FOF_OUTPUT_FIELDS", a);
    }
    if(a=getenv("FOF_OMIT_TRANSITIONS")) {
	put_tagged_string("FOF_OMIT_TRANSITIONS", a);
    }
    if(a=getenv("FOLD_SHEAR")) {
	/* triggers an unfold if the shear is greater
	 * than this number
	 */
	put_tagged_string("FOLD_SHEAR", a);
    }
    if(a=getenv("FORE_ANGLE_LIMITS")) {
	put_tagged_string("FORE_ANGLE_LIMITS", a);
    }
    if(a=getenv("FORE_REFL_CORR")) {
	put_tagged_string("FORE_REFL_CORR", a);
    }
    if(a=getenv("GECHO_MAX_ROT_ANGLE")) { /* in degrees */
	put_tagged_string("GECHO_MAX_ROT_ANGLE", a);
    }
    if(a=getenv("GECHO_MIN_GATES")) {
	/* the ground echo program should always produce
	 * at least this number of gates per beam
	 */
	put_tagged_string("GECHO_MIN_GATES", a);
    }
    if(a=getenv("GECHO_MIN_ROT_ANGLE")) {
	put_tagged_string("GECHO_MIN_ROT_ANGLE", a);
    }
    if(a=getenv("GECHO_MSL")) {
	put_tagged_string("GECHO_MSL", a);
    }
    if(a=getenv("GECHO_REFL")) {
	put_tagged_string("GECHO_REFL",a);
    }
    if(a=getenv("GECHO_VEL")) {
	put_tagged_string("GECHO_VEL",a);
    }
    if(a=getenv("GENERATE_SUBSECOND_TIMES")) {
	put_tagged_string("GENERATE_SUBSECOND_TIMES", a);
    }
    if(a=getenv("GENERATE_THRESHOLDED_FIELDS")) {
	put_tagged_string("GENERATE_THRESHOLDED_FIELDS",a);
    }
    if(a=getenv("GPRO_TAPES")) {
	put_tagged_string("GPRO_TAPES", a);
    }
    if(a=getenv("HRD_ASCII_TAPE")) {
	put_tagged_string("HRD_ASCII_TAPE", a);
    }
    if(a=getenv("HRD_IO_TYPE")) {
	put_tagged_string("HRD_IO_TYPE", a);
    }
    if(a=getenv("HRD_RANGE_DELAY")) {
	/* the number of gates to throw away
	 * at the beginning of the beam
	 */
	put_tagged_string("HRD_RANGE_DELAY",a);
    }
    if(a=getenv("HRD_STD_TAPE")) {
	put_tagged_string("HRD_STD_TAPE", a);
    }
    if(a=getenv("HRD_VOLUME_HEADER")) {
	/* specifies the complete file name of a surrogate
	 * volume header record
	 */
	put_tagged_string("HRD_VOLUME_HEADER",a);
    }
    if(a=getenv("IMPROVE_HZO_RCONST_CORR")) {
	put_tagged_string("IMPROVE_HZO_RCONST_CORR", a);
    }
    if(a=getenv("INCLUDE_DH_DP")) {
	put_tagged_string("INCLUDE_DH_DP", a);
    }
    if(a=getenv("INPUT_FORMAT")) {
	put_tagged_string("INPUT_FORMAT", a);
    }
    if(a=getenv("INPUT_FILES_LIST")) {
	put_tagged_string("INPUT_FILES_LIST", a);
    }
    if(a=getenv("INTERLEAVE")) {
	put_tagged_string("INTERLEAVE", a);
    }
    if(a=getenv("IO_TYPE")) {	/* see DORADE_IO_TYPE */
	put_tagged_string("IO_TYPE", a);
    }
    if(a=getenv("JULIAN_DAY_BIAS")) {
	/* the number of days that will be subtracted from the
	 * julian day in the ray info descriptor
	 */
	put_tagged_string("JULIAN_DAY_BIAS", a);
    }
    if(a=getenv("KEEP_ORTHOG0NAL_DATA")) {
	put_tagged_string("KEEP_ORTHOG0NAL_DATA", a);
    }
    if(a=getenv("LIDAR_AVERAGING")) {
	put_tagged_string("LIDAR_AVERAGING", a);
    }
    if(a=getenv("LIDAR_CELL_SPACING")) {
	put_tagged_string("LIDAR_CELL_SPACING", a);
    }
    if(a=getenv("LIDAR_OPTIONS")) {
	put_tagged_string("LIDAR_OPTIONS", a);
    }
    if(a=getenv("LIDAR_SWEEP_TIME_LIMIT")) {
	put_tagged_string("LIDAR_SWEEP_TIME_LIMIT", a);
    }
    if(a=getenv("LINES_IN_WINDOW")) {
	put_tagged_string("LINES_IN_WINDOW", a);
    }
    if(a=getenv("MAX_DORADE_TAPE_SIZE")) {
	put_tagged_string("MAX_DORADE_TAPE_SIZE", a); /* in GB */
    }
    if((a=getenv("MAX_MEDIA_SIZE"))) {
	/* in MB */
	put_tagged_string("MAX_MEDIA_SIZE", a);
    }
    if(a=getenv("MAX_NOTCH_VELOCITY")) {
	/* The maximum (in m/s) that a notch velocity
	 * can attain
	 */
	put_tagged_string("MAX_NOTCH_VELOCITY", a);
    }
    if(a=getenv("MAX_RAYS_PER_SWEEP")) {
	put_tagged_string("MAX_RAYS_PER_SWEEP", a);
    }
    if(a=getenv("MAX_RHI_DIFF")) {
	put_tagged_string("MAX_RHI_DIFF", a);
    }
    if(a=getenv("MAX_SWEEPS_PER_VOLUME")) {
	put_tagged_string("MAX_SWEEPS_PER_VOLUME", a);
    }
    if(a=getenv("MAX_UF_CELLS")) {
	/* fixes the number of uf range bins at some max value */
	put_tagged_string("MAX_UF_CELLS", a);
    }
    if(a=getenv("MAX_VOLUME_SIZE")) {
	/* in MagaBytes
	 * DORADE volume must only slightly exceed this size
	 */
	put_tagged_string("MAX_VOLUME_SIZE", a);
    }
    if(a=getenv("MAX_WIND")) {
	put_tagged_string("MAX_WIND", a);
    }
    if(a=getenv("MAX_AZ_DIFF")) {
	put_tagged_string("MAX_AZ_DIFF", a);
    }
    if(a=getenv("MC_DATA")) {
	put_tagged_string("MC_DATA", a);
    }
    if(a=getenv("MIN_AZ_DIFF")) {
	put_tagged_string("MIN_AZ_DIFF", a);
    }
    if(a=getenv("MIN_BAD_COUNT")) {
	/* In some cases is used to stop an algorithm
	 * after more than n consecutive bad cells
	 */
	put_tagged_string("MIN_BAD_COUNT", a);
    }
    if(a=getenv("MIN_EL_DIFF")) {
	put_tagged_string("MIN_EL_DIFF", a);
    }
    if(a=getenv("MIN_FXD_DIFF")) {
	put_tagged_string("MIN_FXD_DIFF", a);
    }
    if(a=getenv("MIN_FREE_MB")) {
	/* stops creating new sweep files when free space on disk
	 * drops below this value
	 */
	put_tagged_string("MIN_FREE_MB", a);
    }
    if(a=getenv("MIN_RAYS_PER_SWEEP")) {
	put_tagged_string("MIN_RAYS_PER_SWEEP", a);
    }
    if(a=getenv("MIN_SWEEPS_PER_VOLUME")) {
	put_tagged_string("MIN_SWEEPS_PER_VOLUME", a);
    }
    if(a=getenv("MIN_TIME_GAP")) {
	/* In some cases is used to start a new volume
	 * after more than n seconds of missing data
	 */
	put_tagged_string("MIN_TIME_GAP", a);
    }
    if(a=getenv("MIN_VOLUME_TIME_SPAN")) {
	/* in seconds
	 * DORADE volume must be a least this long
	 */
	put_tagged_string("MIN_VOLUME_TIME_SPAN", a);
    }
    if(a=getenv("MRD_DZ_FIELD")) {
	put_tagged_string("MRD_DZ_FIELD", a);
    }
    if(a=getenv("MRD_MAX_RANGE")) {
	put_tagged_string("MRD_MAX_RANGE", a);
    }
    if(a=getenv("MRD_VE_FIELD")) {
	put_tagged_string("MRD_VE_FIELD", a);
    }
    if((a=getenv("NETCDF_DIRECTORY")) || (a=getenv("NC_DIR"))) {
	put_tagged_string("NC_DIR", a);
    }
    if(a=getenv("NETCDF_FIELDS")) {
	put_tagged_string("NETCDF_FIELDS", a);
    }
    if(a=getenv("NEW_SWEEP_FLAGS")) {
	put_tagged_string("NEW_SWEEP_FLAGS", a);
    }
    if(a=getenv("NOTCH_SHEAR")) {
	/* triggers denotching if the shear is greater
	 * than this number
	 */
	put_tagged_string("NOTCH_SHEAR", a);
    }
    if(a=getenv("NCP_THRESHOLD_VAL")) {
	put_tagged_string("NCP_THRESHOLD_VAL", a);
    }
    if(a=getenv("NEW_SWEEP_HESITATION")) {
	put_tagged_string("NEW_SWEEP_HESITATION", a);
    }
    if(a=getenv("NO_WIND_INFO")) {
	/* do not use wind info in unfolding */
	put_tagged_string("NO_WIND_INFO", a);
    }
    if(a=getenv("NUM_AZ_DIFFS_AVGD")) {
	put_tagged_string("NUM_AZ_DIFFS_AVGD", a);
    }
    if(a=getenv("NUM_EL_DIFFS_AVGD")) {
	put_tagged_string("NUM_EL_DIFFS_AVGD", a);
    }
    if(a=getenv("NUM_SHORT_AVG")) {
	put_tagged_string("NUM_SHORT_AVG", a);
    }
    if(a=getenv("NYQUIST_VELOCITY")) {
	put_tagged_string("NYQUIST_VELOCITY", a);
    }
    if(a=getenv("OPTIONS")) {
	put_tagged_string("OPTIONS", a);
    }
    if(a=getenv("OUTPUT_FIELDS")) {
	put_tagged_string("OUTPUT_FIELDS", a);
    }
    if(a=getenv("OUTPUT_FLAGS")) {
	/* recognized strings/flags are:
	 * "NO_CATALOG" default is to produce a catalot
	 * "NO_SWEEP_FILES" default is to produce sweep files
	 * "CATALOG_ONLY" all other output is disabled regardless
	 * "DORADE_DATA" DORADE tape format output
	 * "UF_DATA"
	 * "CAPPI_DATA"
	 * "GECHO_DATA"
	 */
	put_tagged_string("OUTPUT_FLAGS", a);
    }
    if(a=getenv("PCT_STATS")) {
	put_tagged_string("PCT_STATS", a);
    }
    if(a=getenv("PHASE_OFFSET")) {
	put_tagged_string("PHASE_OFFSET", a);
    }
    if(a=getenv("PPI_EL_TOL")) {
	put_tagged_string("PPI_EL_TOL", a);
    }
    if(a=getenv("RHI_AZ_TOL")) {
	put_tagged_string("RHI_AZ_TOL", a);
    }
    if(a=getenv("ZDR_OFFSET")) {
	put_tagged_string("ZDR_OFFSET", a);
    }
    if(a=getenv("PIRAQ_FORCE_POLYPP")) {
	put_tagged_string("PIRAQ_FORCE_POLYPP", a);
    }
    if(a=getenv("PRESERVE_SWEEP_FILES")) {
	/* prevents DORADE tape output routine from
	 * deleting used up sweep files
	 */
	put_tagged_string("PRESERVE_SWEEP_FILES", a);
    }
    if(a=getenv("PRF_LIMITS")) { /* see fixed angle */
	put_tagged_string("PRF_LIMITS", a);
    }
    if(a=getenv("PROJECT_NAME")) {
	put_tagged_string("PROJECT_NAME", a);
    }
    if(a=getenv("PWR_THRESHOLD_VAL")) {
	put_tagged_string("PWR_THRESHOLD_VAL", a);
    }
    if(a=getenv("RADAR_ALTITUDE")) {
	put_tagged_string("RADAR_ALTITUDE", a);
    }
    if(a=getenv("RCONST_CORRECTION")) {
	put_tagged_string("RCONST_CORRECTION", a);
    }
    if(a=getenv("RADAR_LATITUDE")) {
	put_tagged_string("RADAR_LATITUDE", a);
    }
    if(a=getenv("RADAR_LONGITUDE")) {
	put_tagged_string("RADAR_LONGITUDE", a);
    }
    if(a=getenv("RADAR_WAVELENGTH")) {
	put_tagged_string("RADAR_WAVELENGTH", a);
    }
    if(a=getenv("RANGE_CORRECTION")) {
	put_tagged_string("RANGE_CORRECTION", a);
    }
    if(a=getenv("RANGE_DELAY")) {
	/* the number of gates to throw away
	 * at the beginning of the beam
	 */
	put_tagged_string("RANGE_DELAY",a);
    }
    if(a=getenv("RADAR_NAME")) {
	put_tagged_string("RADAR_NAME", a);
    }
    if(a=getenv("RANGE_LIMITS")) {
	put_tagged_string("RANGE_LIMITS", a);
    }
    if(a=getenv("RENAME")) {
	put_tagged_string("RENAME", a);
    }
    if(a=getenv("ROTATION_ANGLE_TOLERANCE")) {
	put_tagged_string("ROTATION_ANGLE_TOLERANCE", a);
    }
    if(a=getenv("SCAN_LIST")) {
	put_tagged_string("SCAN_LIST", a);
    }
    if((a=getenv("SELECT_RADAR")) || (a=getenv("SELECT_RADARS"))) {
	put_tagged_string("SELECT_RADARS", a);
    }
    if(a=getenv("SHANES_DATA_FIELDS")) {
	put_tagged_string("SHANES_DATA_FIELDS", a);
    }
    if(a=getenv("SHANES_DIRECTORY")) {
	put_tagged_string("SHANES_DIRECTORY", a);
    }
    if(a=getenv("SITE_NAME")) {
	put_tagged_string("SITE_NAME", a);
    }
    if(a=getenv("SIDELOBE_RING")) {
	put_tagged_string("SIDELOBE_RING", a);
    }
    if(a=getenv("SNR_THRESHOLD_VAL")) {
	put_tagged_string("SNR_THRESHOLD_VAL", a);
    }
    if(a=getenv("SOURCE_DEV")) {
	/* the device or file name
	 */
	put_tagged_string("SOURCE_DEV",a);
    }
    if(a=getenv("SOURCE_FILE")) {
	/* names the input file/device */
	put_tagged_string("SOURCE_FILE", a);
    }
    if(a=getenv("SOURCE_TAPE_ID")) {
	/* goes into the catalog as the source tape id */
	put_tagged_string("SOURCE_TAPE_ID", a);
    }
    if(a=getenv("START_RANGE")) {
	put_tagged_string("START_RANGE", a);
    }
    if(a=getenv("STOP_RANGE")) {
	put_tagged_string("STOP_RANGE", a);
    }
    if(a=getenv("STORM_NAME")) {
	put_tagged_string("STORM_NAME", a);
    }
    if(a=getenv("SW_THRESHOLD_VAL")) {
	put_tagged_string("SW_THRESHOLD_VAL", a);
    }
    if(a=getenv("SWEEP_COUNT")) {
	/* stops input after reading n sweeps */
	put_tagged_string("SWEEP_COUNT", a);
    }
    if((a=getenv("SWEEP_MODE")) || (a=getenv("SWEEP_MODES"))) {
	/* filters on sweep modes such as "PPI", "AIR", etc. */
	put_tagged_string("SWEEP_MODES", a);
    }
    if(a=getenv("SWEEP_SKIP")) {
	/* uses every nth sweep of a particular radar */
	put_tagged_string("SWEEP_SKIP", a);
    }
    if(a=getenv("SWEEP_TIME_TOLERANCE")) {
	put_tagged_string("SWEEP_TIME_TOLERANCE", a);
    }
    if(a=getenv("SWEEP_TRIP_ANGLE")) {
	put_tagged_string("SWEEP_TRIP_ANGLE", a);
    }
    if(a=getenv("SWEEP_TRIP_DELTA")) {
	put_tagged_string("SWEEP_TRIP_DELTA", a);
    }
    if(a=getenv("TAPE_DIR")) {
	put_tagged_string("TAPE_DIR", a);
    }
    if(a=getenv("TEST_PULSE_RANGES")) {
	put_tagged_string("TEST_PULSE_RANGES", a);
    }
    if(a=getenv("TIME_CORRECTION")) {
	put_tagged_string("TIME_CORRECTION", a);
    }
    if(a=getenv("TIME_DEFINED_VOLUMES")) {
	put_tagged_string("TIME_DEFINED_VOLUMES", a);
    }
    if(a=getenv("TIME_DEPENDENT_FIXES")) {
	put_tagged_string("TIME_DEPENDENT_FIXES", a);
    }
    if(a=getenv("TIME_LIMITS")) {
	/* one or more sets of limits of the form
	 * "02/04/93:22:14:13 < 02/04/93:22:21:18"
	 */
	put_tagged_string("TIME_LIMITS", a);
    }
    if(a=getenv("TIME_DRIFT")) {
	put_tagged_string("TIME_DRIFT", a);
    }
    if(a=getenv("TOLERANCE_LEVEL_1")) {
	put_tagged_string("TOLERANCE_LEVEL_1", a);
    }
    if(a=getenv("TOLERANCE_LEVEL_2")) {
	put_tagged_string("TOLERANCE_LEVEL_2", a);
    }
    if(a=getenv("UF_ALIASES")) {
	put_tagged_string("UF_ALIASES", a);
    }
    if((a=getenv("UF_DEV")) || (a=getenv("UF_DEVS"))) {
	/* the device name if not a standard named file
	 * to which UF tape format data is written
	 */
	put_tagged_string("UF_DEV",a);
    }
    if(a=getenv("UF_DIRECTORY")) {
	/* destination of uf files */
	put_tagged_string("UF_DIRECTORY", a);
    }
    if(a=getenv("UF_IO_TYPE")) { /* see dorade io type */
	put_tagged_string("UF_IO_TYPE",a);
    }
    if(a=getenv("UF_OUTPUT_FIELDS")) {
	put_tagged_string("UF_OUTPUT_FIELDS", a);
    }
    if(a=getenv("UNFOLD_QUE_SIZE")) {
	/* defines the number of cells that determine
	 * a non-shear run and the number of cells
	 * averaged for unfolding
	 */
	put_tagged_string("UNFOLD_QUE_SIZE", a);
    }
    if(a=getenv("UNIFORM_CELL_SPACING")) {
	put_tagged_string("UNIFORM_CELL_SPACING", a);
    }
    if(a=getenv("VOLUME_COUNT")) {
	/* stops input after reading n volumes */
	put_tagged_string("VOLUME_COUNT", a);
    }
    if(a=getenv("VOLUME_HEADER")) {
	/* specifies the complete file name of a surrogate
	 * volume header record
	 */
	put_tagged_string("VOLUME_HEADER",a);
    }
    if(a=getenv("XOUT_AZIMUTHS")) {
	put_tagged_string("XOUT_AZIMUTHS", a);
    }
    if(a=getenv("XOUT_ELEVATIONS")) {
	put_tagged_string("XOUT_ELEVATIONS", a);
    }
    if(a=getenv("WSR_QC_FILE")) {
	put_tagged_string("WSR_QC_FILE", a);
    }
    if(a=getenv("WATCH_FIXED_ANGLE")) {
	put_tagged_string("WATCH_FIXED_ANGLE", a);
    }
    if(a=getenv("YEAR_OF_DATA")) {
	put_tagged_string("YEAR_OF_DATA", a);
    }
    if(a=getenv("ZDR_BIAS")) {
	put_tagged_string("ZDR_BIAS", a);
    }

# ifdef obsolete
    if(a=getenv("x")) {
	put_tagged_string("x", a);
    }
# endif
}
/* c------------------------------------------------------------------------ */

void dgi_interest_really(dgi, verbosity, preamble, postamble, swib)
  struct dd_general_info *dgi;
  int verbosity;
  char *preamble, *postamble;
  struct sweepinfo_d *swib;
{
    /* 
     * passing in the swib is necessary because there is more than
     * on around during DORADE input and this routine is used by 
     * other front ends such as UF output from sweep files.
     */
    int ii, jj, nn;
    char str[256], *a, *dts_print(), *str_terminate();
    DD_TIME *d_unstamp_time();
    struct dds_structs *dds;

    dds = dgi->dds;

    if(verbosity)
	  printf("\n");
    printf("%s ", preamble);
    printf("%s ", dts_print(d_unstamp_time(dgi->dds->dts)));
    printf("%s ", dgi->radar_name);
    if(dgi->dds->radd->scan_mode == AIR) {
	printf("t:%5.1f ", dgi->dds->asib->tilt);
	printf("rt:%6.1f ", dgi->dds->asib->rotation_angle);
	printf("rl:%5.1f ", dgi->dds->asib->roll);
	printf("h:%6.1f ", dgi->dds->asib->heading);
	printf("al:%6.3f ", dgi->dds->asib->altitude_msl);
    }
    else {
	printf("fx:%6.1f ", swib->fixed_angle);
	printf("az:%6.1f ", dgi->dds->ryib->azimuth);
	printf("el:%6.2f ", dgi->dds->ryib->elevation);
    }
    printf("swp: %2d ", dgi->dds->ryib->sweep_num);
    printf("%s ", postamble);
    printf("\n");
    if(verbosity) {
	printf("la:%9.4f ", dgi->dds->asib->latitude);
	printf("lo:%9.4f ", dgi->dds->asib->longitude);
	if(dgi->dds->radd->scan_mode == AIR) {
	    printf("p:%5.1f ", dgi->dds->asib->pitch);
	    printf("d:%5.1f ", dgi->dds->asib->drift_angle);
	    printf("ag:%6.3f ", dgi->dds->asib->altitude_agl);
	    printf("\n");
	    printf("ve:%6.1f ", dgi->dds->asib->ew_velocity);
	    printf("vn:%6.1f ", dgi->dds->asib->ns_velocity);
	    printf("vv:%6.1f ", dgi->dds->asib->vert_velocity);
	    printf("we:%6.1f ", dgi->dds->asib->ew_horiz_wind);
	    printf("wn:%6.1f ", dgi->dds->asib->ns_horiz_wind);
	    printf("wv:%6.1f ", dgi->dds->asib->vert_wind);
	}
	else {
	    printf("al:%6.3f ", dgi->dds->asib->altitude_msl);
	}
	printf("\n");
	printf("Num parameters: %d  ", dgi->source_num_parms);
	for(ii=0; ii < dgi->source_num_parms; ii++) {
	    printf("%s ", str_terminate
		   (str, dgi->dds->parm[ii]->parameter_name, 8));
	}
	printf("\n");
    }
}
/* c------------------------------------------------------------------------ */

int dd_itsa_physical_dev(s)
  char *s;
{
    /* check the string to see if it contains a physical device name */
    if(strstr(s, "/dev/"))
	  return(YES);
    return(NO);
}
/* c------------------------------------------------------------------------ */

double
dd_latitude(dgi)
  struct dd_general_info *dgi;
{
    double d=dgi->dds->asib->latitude;
    if(dd_isnanf(d))
	  return((double)0);
    else
	  return(d + dgi->dds->cfac->latitude_corr);
}
/* c------------------------------------------------------------------------ */

void dd_latlon_relative(p0, p1)
  struct point_in_space *p0, *p1;
{
    /* routine to calculate (x,y,z) for p1 so as to line up
     * with (0,0,0) for p0
     */
    double del_lat, del_lon, lat=RADIANS(p1->latitude);
    double dd_earthr(), r_earth, xx, yy, zz, R_earth;

    R_earth = dd_earthr(p1->latitude); 
    loop_ll2xy_v3( &p0->latitude, &p0->longitude, &p0->altitude
		   , &xx, &yy, &zz
		   , p1->latitude, p1->longitude, p1->altitude
		   , R_earth, 1 );
    p1->x = (float)xx;
    p1->y = (float)yy;
    p1->z = (float)zz;
}
/*c----------------------------------------------------------------------*/

int loop_ll2xy_v3( plat, plon, palt, x, y, z, olat, olon, oalt
		   , R_earth, num_pts )
    double *plat, *plon, *palt, olat, olon, oalt, R_earth;
    double *x, *y, *z;
    int num_pts;
{
    /* calculate (x,y,z) of (plat,plon) relative to (olat,olon) */
    /* all dimensions in km. */

    /* transform to earth coordinates and then to lat/lon/alt */

    /* These calculations are from the book
     * "Aerospace Coordinate Systems and Transformations"
     * by G. Minkler/J. Minkler
     * these are the ECEF/ENU point transformations
     */

    int ii, nn = num_pts;
    double R_o, R_o_pr, delta_o, lambda_o, R_p, R_p_pr, delta_p, lambda_p, y_pr;
    double R, rr, abscissa, dlat, dlon;
    double xe, ye, ze, sinLambda, cosLambda, sinDelta, cosDelta;
    double h, a, b, c;


    h = R_earth + oalt;
    delta_o = RADIANS( olat );	/* read delta sub oh */
    lambda_o = RADIANS( olon );

    sinLambda = sin( lambda_o );
    cosLambda = cos( lambda_o );
    sinDelta = sin( delta_o );
    cosDelta = cos( delta_o );
    /*
    printf( "\n" );
     */
    
    for(; nn--; plat++, plon++, palt++, x++, y++, z++ ) {

       R_p = R_earth + (*palt);
       delta_p = RADIANS( *plat );
       lambda_p = RADIANS( *plon );
       R_p_pr = R_p * cos( delta_p );

       xe = R_p * sin( delta_p );
       ye = -R_p_pr * sin( lambda_p );
       ze = R_p_pr * cos( lambda_p );

	/* transform to ENU coordinates */

       a = -h * sinDelta + xe;
       b =  h * cosDelta * sinLambda + ye;
       c = -h * cosDelta * cosLambda + ze;

       *x = -cosLambda * b  -sinLambda * c;
       *y = cosDelta * a  +  sinLambda * sinDelta * b
	 -cosLambda * sinDelta * c;
       *z = sinDelta * a  -sinLambda * cosDelta * b
	 +cosLambda * cosDelta * c;
       /*
       printf( "%f %f %f      %f %f %f\n", *plat, *plon, *palt, *x, *y, *z );
	*/
    }	
    return num_pts;
}
/* c------------------------------------------------------------------------ */

void dd_latlon_shift(p0, p1)
  struct point_in_space *p0, *p1;
{
    /* routine to calculate lat/lon/alt for p1 so as to line up
     * with (x,y,z) for p0
     */
    double d, r, xx=p0->x, yy=p0->y, zz=p0->z;
    double dd_earthr(), r_earth;
    double dlon = -p1->longitude;
    double R_earth;

    R_earth = dd_earthr(p0->latitude); 
    loop_xy2ll_v3( &p1->latitude, &p1->longitude, &p1->altitude
		   , &xx, &yy, &zz
		   , p0->latitude, p0->longitude, p0->altitude
		   , R_earth, 1 );
}
/*c----------------------------------------------------------------------*/

int loop_xy2ll_v3( plat, plon, palt, x, y, z, olat, olon, oalt
		   , R_earth, num_pts )
    double *plat, *plon, *palt, olat, olon, oalt, R_earth;
    double *x, *y, *z;
    int num_pts;
{
    /* calculate (plat,plon) of a point at (x,y) relative to (olat,olon) */
    /* all dimensions in km. */

    /* transform to earth coordinates and then to lat/lon/alt */

    /* These calculations are from the book
     * "Aerospace Coordinate Systems and Transformations"
     * by G. Minkler/J. Minkler
     * these are the ECEF/ENU point transformations
     */

    int ii, nn = num_pts;
    double R_o, R_o_pr, delta_o, lambda_o, R_p, R_p_pr, delta_p, lambda_p, y_pr;
    double R, rr, abscissa, dlat, dlon;
    double xe, ye, ze, sinLambda, cosLambda, sinDelta, cosDelta;
    double h;


    h = R_earth + oalt;
    delta_o = RADIANS( olat );	/* read delta sub oh */
    lambda_o = RADIANS( olon );

    sinLambda = sin( lambda_o );
    cosLambda = cos( lambda_o );
    sinDelta = sin( delta_o );
    cosDelta = cos( delta_o );
    /*
    printf( "\n" );
     */
    
    for(; nn--; plat++, plon++, palt++, x++, y++, z++ ) {

	/* transform to earth coordinates */

	xe = h * sinDelta + cosDelta * (*y) + sinDelta * (*z);

	ye = -h * cosDelta * sinLambda   -cosLambda * (*x)
	  + sinLambda * sinDelta * (*y) -sinLambda * cosDelta * (*z);

	ze = h * cosDelta * cosLambda   -sinLambda * (*x)
	  -cosLambda * sinDelta * (*y) + cosLambda * cosDelta * (*z);

	lambda_p = atan2( -ye, ze );
	delta_p = atan2( xe, sqrt( ye * ye + ze * ze ));

	*plat = DEGREES( delta_p );
	*plon = DEGREES( lambda_p );
	*palt = sqrt( xe * xe + ye * ye + ze * ze ) - R_earth;
	/*
	printf( "%f %f %f      %f %f %f\n", *plat, *plon, *palt, *x, *y, *z );
	 */
    }	
    return num_pts;
}
/* c------------------------------------------------------------------------ */

char *dd_link_dev_name(radar_name, dev_links, str)
  char *radar_name, *dev_links, *str;
{
    /* this routine is looking for pairs of the form
     * "TA > /dev/nrst0 TF > /dev/nrst1"
     * it want to return a pointer the the device name
     * to be associated with the radar
     */
    char *str_ptrs[32], string_space[256];
    char radar[16], *str_terminate();
    int ii, nt;

    str_terminate(radar, radar_name, 8);
    strcpy(string_space, dev_links);
    nt = dd_tokens(string_space, str_ptrs);

    for(ii=0; ii < nt; ii += 3) {
	if(strstr(radar, str_ptrs[ii])) {
	    if(ii+2 < nt) {
		strcpy(str, str_ptrs[ii+2]);
		return(str);
	    }
	}
    }
    return(NULL);
}
/* c------------------------------------------------------------------------ */

double
dd_longitude(dgi)
  struct dd_general_info *dgi;
{
    double d=dgi->dds->asib->longitude;
    if(dd_isnanf(d))
	  return((double)0);
    else
	  return(d + dgi->dds->cfac->longitude_corr);
}
/* c------------------------------------------------------------------------ */

struct radar_d *
dd_malloc_radd(dgi, config_name)
  DGI_PTR dgi;
  char *config_name;
{
    /* return a pointer to the radar descriptor with the
     * corresponding config_name or establish a new one
     * with this config name
     */
    struct radar_d *radd=NULL, **radd_ptr=dgi->dds->radd_list;
    DDS_PTR dds=dgi->dds;
    int ii, mm, nn, found=NO;
    char *aa;

    /* search for a match
     */
    if(strlen(config_name)) {
	for(mm=dds->radar_desc_count; mm-- ; radd_ptr++) {
	    if(!strncmp((*radd_ptr)->config_name, config_name, 8)) {
		radd = *radd_ptr;
		break;
	    }
	}
    }
    if(!radd) {			/* need to add a new radar descriptor */
	if(dds->radar_desc_count+1 > dds->max_radar_desc) {
	    /* need to expand the list of pointers */
	    dds->max_radar_desc += 8;
	    if(dds->radar_desc_count) {
		dds->radd_list = (struct radar_d **)
		      realloc(dds->radd_list, dds->max_radar_desc *
			      sizeof(struct radar_d *));
	    }
	    else {
		dds->radd_list = (struct radar_d **)
		      malloc(dds->max_radar_desc * sizeof(struct radar_d *));
		memset(dds->radd_list, 0, dds->max_radar_desc *
		       sizeof(struct radar_d *));
	    }
	}
	if(radd = (struct radar_d *)malloc(sizeof(struct radar_d)))
	      memset(radd, 0, sizeof(struct radar_d));
	else {
	    printf("Unable to allocate the next radar descriptor\n");
	    exit(1);
	}
	if(dds->radar_desc_count) {
	    /* copy first one to this one */
	    memcpy(radd, *dds->radd_list, sizeof(struct radar_d *));
	}
	else {			/* first one */
	    strncpy(radd->radar_des, "RADD", 4 );
	    strncpy(radd->config_name, "NONE    ", 8);
	    radd->radar_des_length = sizeof(struct radar_d);
	}
	*(dds->radd_list + dds->radar_desc_count++) = radd;
    }
    return(radd);
}
/* c------------------------------------------------------------------------ */

double
dd_nav_rotation_angle(dgi)
  struct dd_general_info *dgi;
{
    double d_rot, dd_azimuth_angle();

    switch(dgi->dds->radd->scan_mode) {

    case AIR:
	d_rot = FMOD360(dgi->dds->asib->rotation_angle +
			dgi->dds->cfac->rot_angle_corr );
	break;

    case RHI:
	d_rot = FMOD360((450.-(dgi->dds->ryib->elevation
			       + dgi->dds->cfac->elevation_corr)));
	break;

    case TAR:
	d_rot = dgi->dds->radd->radar_type != GROUND ?
	      FMOD360(dgi->dds->asib->rotation_angle +
			dgi->dds->cfac->rot_angle_corr) :
			      dd_azimuth_angle(dgi);
	break;

    default:
	d_rot = dd_azimuth_angle(dgi);
	break;
    }
    return(d_rot);
}
/* c------------------------------------------------------------------------ */

double
dd_nav_tilt_angle(dgi)
  struct dd_general_info *dgi;
{
    double d_rot;

    switch(dgi->dds->radd->scan_mode) {
	
    case AIR:
	d_rot = dgi->dds->asib->tilt + dgi->dds->cfac->tilt_corr;
	break;

    case RHI:
	d_rot = CART_ANGLE
	      (dgi->dds->ryib->azimuth +dgi->dds->cfac->azimuth_corr);
	break;

    default:
	d_rot = dd_elevation_angle (dgi);
	break;
    }
    return(d_rot);
}
/* c------------------------------------------------------------------------ */

int dd_ndx_name(dgi, name2)
  DGI_PTR dgi;
  char *name2;
{
    DDS_PTR dds=dgi->dds;
    int ii, n=strlen(name2);

    if(n < 1) {
	return(-1);
    }
    ii = dd_find_field(dgi, name2);
    return(ii);
}
/* c------------------------------------------------------------------------ */

double
dd_pitch(dgi)
  struct dd_general_info *dgi;
{
    double d=dgi->dds->asib->pitch;
    if(dd_isnanf(d))
	  return((double)0);
    else
	  return(d +dgi->dds->cfac->pitch_corr);
}
/* c------------------------------------------------------------------------ */

void
dor_print_asib(asib, slm)
  struct platform_i *asib;
  struct solo_list_mgmt *slm;
{
    char *aa, bstr[128];

    aa = bstr;
    /* routine to print the contents of the descriptor */

    solo_add_list_entry(slm, " ", 1);

    sprintf(aa, "Contents of the platform descriptor  len: %d"
	    , sizeof(struct platform_i));

    /* c...mark */

    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " platform_info[4]       %.4s", asib->platform_info);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " platform_info_lengtth  %d", asib->platform_info_length);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " longitude              %f", asib->longitude);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " latitude               %f", asib->latitude);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " altitude_msl           %f", asib->altitude_msl);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " altitude_agl           %f", asib->altitude_agl);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " ew_velocity            %f", asib->ew_velocity);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " ns_velocity            %f", asib->ns_velocity);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " vert_velocity          %f", asib->vert_velocity);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " heading                %f", asib->heading);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " roll                   %f", asib->roll);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " pitch                  %f", asib->pitch);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " drift_angle            %f", asib->drift_angle);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " rotation_angle         %f", asib->rotation_angle);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " tilt                   %f", asib->tilt);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " ew_horiz_wind          %f", asib->ew_horiz_wind);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " ns_horiz_wind          %f", asib->ns_horiz_wind);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " vert_wind              %f", asib->vert_wind);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " heading_change         %f", asib->heading_change);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " pitch_change           %f", asib->pitch_change);
    solo_add_list_entry(slm, aa, strlen(aa));

    return;
}
/* c------------------------------------------------------------------------ */

void
dor_print_celv(celv, slm)
  struct cell_d *celv;
  struct solo_list_mgmt *slm;
{
    int ii;
    double d, delta1=1.e6, delta2;
    char *aa, bstr[128];

    aa = bstr;

    solo_add_list_entry(slm, " ", 1);

    sprintf(aa, "Contents of the cell vector descriptor  len: %d"
	    , sizeof(struct cell_d));
    solo_add_list_entry(slm, aa, strlen(aa));

    sprintf(aa, "cell_spacing_des	%.4s", celv->cell_spacing_des);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "cell_des_len	%d", celv->cell_des_len);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "number_cells	%d", celv->number_cells);   
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "dist_cells[  0]     %f", celv->dist_cells[0]);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "dist_cells[  1]     %f", celv->dist_cells[1]);
    solo_add_list_entry(slm, aa, strlen(aa));
    
    delta1 = fabs(celv->dist_cells[  1] -celv->dist_cells[0]);

    for(ii=1; ii < celv->number_cells-1; ii++) {
	delta2 = fabs(celv->dist_cells[ii+1] -celv->dist_cells[ii]);
	if(fabs(delta2 -delta1) > 10) {
	    sprintf(aa, "dist_cells[%3d]     %f"
		    , ii, celv->dist_cells[ii]);
	    solo_add_list_entry(slm, aa, strlen(aa));
	    sprintf(aa, "dist_cells[%3d]     %f"
		    , ii+1, celv->dist_cells[ii+1]);
	    solo_add_list_entry(slm, aa, strlen(aa));
	}
	delta1 = delta2;
    }
    sprintf(aa, "dist_cells[%3d]     %f", ii, celv->dist_cells[ii]);
    solo_add_list_entry(slm, aa, strlen(aa));
    return;
}
/* c------------------------------------------------------------------------ */

void
dor_print_cfac(cfac, slm)
  struct correction_d *cfac;
  struct solo_list_mgmt *slm;
{
    char str[64], *str_terminate();
    char *aa, bstr[128];

    /* routine to print the contents of the descriptor */
    aa = bstr;
    solo_add_list_entry(slm, " ", 1);

    sprintf(aa, "Contents of the correction descriptor  len: %d\n"
	    , sizeof(struct correction_d));
    solo_add_list_entry(slm, aa, strlen(aa));

    sprintf(aa, "correction_des %.4s       ", cfac->correction_des       );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "correction_des_length %d", cfac->correction_des_length);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "azimuth_corr %f         ", cfac->azimuth_corr         );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "elevation_corr %f       ", cfac->elevation_corr       );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "range_delay_corr %f     ", cfac->range_delay_corr     );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "longitude_corr %f       ", cfac->longitude_corr       );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "latitude_corr %f        ", cfac->latitude_corr        );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "pressure_alt_corr %f    ", cfac->pressure_alt_corr    );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "radar_alt_corr %f       ", cfac->radar_alt_corr       );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "ew_gndspd_corr %f       ", cfac->ew_gndspd_corr       );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "ns_gndspd_corr %f       ", cfac->ns_gndspd_corr       );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "vert_vel_corr %f        ", cfac->vert_vel_corr        );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "heading_corr %f         ", cfac->heading_corr         );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "roll_corr %f            ", cfac->roll_corr            );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "pitch_corr %f           ", cfac->pitch_corr           );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "drift_corr %f           ", cfac->drift_corr           );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "rot_angle_corr %f       ", cfac->rot_angle_corr       );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "tilt_corr %f            ", cfac->tilt_corr            );
    solo_add_list_entry(slm, aa, strlen(aa));

    return;
}
/* c------------------------------------------------------------------------ */

void
dor_print_parm(parm, slm)
  struct parameter_d *parm;
struct solo_list_mgmt *slm;
{
    char str[64], *str_terminate();
    char *aa, bstr[128];

    aa = bstr;


    /* routine to print the contents of the parameter descriptor */

    solo_add_list_entry(slm, " ", 1);
    sprintf(aa, "Contents of the parameter descriptor  len: %d"
	    , sizeof(struct parameter_d));
    solo_add_list_entry(slm, aa, strlen(aa));

    sprintf(aa, "parameter_des[4]        %.4s", parm->parameter_des       );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "parameter_des_length    %d", parm->parameter_des_length);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "parameter_name[8]       %.8s"			     
	    , str_terminate(str, parm->parameter_name			     
			    , sizeof(parm->parameter_name))                  );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "param_description[40]   %.40s"			     
	    , str_terminate(str, parm->param_description		     
			    , sizeof(parm->param_description))      	     );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "param_units[8]          %.8s"			     
	    , str_terminate(str, parm->param_units			     
			    , sizeof(parm->param_units))                     );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "interpulse_time         %d", parm->interpulse_time     );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "xmitted_freq            %d", parm->xmitted_freq        );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "recvr_bandwidth         %f", parm->recvr_bandwidth     );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "pulse_width             %d", parm->pulse_width         );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "polarization            %d", parm->polarization        );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "num_samples             %d", parm->num_samples         );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "binary_format           %d", parm->binary_format       );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "threshold_field         %s"			     
	    , str_terminate(str, parm->threshold_field			     
			    , sizeof(parm->threshold_field))         	     );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "threshold_value         %f", parm->threshold_value     );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "parameter_scale         %f", parm->parameter_scale     );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "parameter_bias          %f", parm->parameter_bias      );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "bad_data                %d", parm->bad_data            );
    solo_add_list_entry(slm, aa, strlen(aa));

    return;
}
/* c------------------------------------------------------------------------ */

void
dor_print_frib(frib, slm)
  struct field_radar_i *frib;
  struct solo_list_mgmt *slm;
{
  char *sptrs[128], str[512], mess[256];
  int i, jj, nn;
  char *aa=mess, c[128], *str_terminate();
  
  solo_add_list_entry(slm, " ", 1);
  solo_add_list_entry(slm, " ", 1);
  sprintf(aa, "Contents of the field radar descriptor  len: %d\n"
	  , sizeof(struct field_radar_i));
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "field_radar_info[4] %.4s"
	  , frib->field_radar_info);
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "field_radar_info_len %d"
	  , frib->field_radar_info_len);
  solo_add_list_entry(slm, aa, strlen(aa));

  sprintf(aa, "data_sys_id %d"
	  , frib->data_sys_id);
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "loss_out %f		"
	  , frib->loss_out		);
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "loss_in %f		"
	  , frib->loss_in		);
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "loss_rjoint %f		"
	  , frib->loss_rjoint		);
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "ant_v_dim %f		"
	  , frib->ant_v_dim		);
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "ant_h_dim %f		"
	  , frib->ant_h_dim		);
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "ant_noise_temp %f	"
	  , frib->ant_noise_temp	);
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "r_noise_figure %f	"
	  , frib->r_noise_figure	);
  solo_add_list_entry(slm, aa, strlen(aa));

  sprintf(aa, "xmit_power[5]");
  for(i=0; i < 5; i++) {
    sprintf(aa+strlen(aa), " %f", frib->xmit_power[i]);
  }
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "x_band_gain %f", frib->x_band_gain);
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "receiver_gain[5]");
  for(i=0; i < 5; i++) {
    sprintf(aa+strlen(aa), " %f", frib->receiver_gain[i]);
  }
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "if_gain[5]");
  for(i=0; i < 5; i++) {
    sprintf(aa+strlen(aa), " %f", frib->if_gain[i]);
  }
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "conversion_gain %f", frib->conversion_gain);
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "scale_factor[5]");
  for(i=0; i < 5; i++) {
    sprintf(aa+strlen(aa), " %f", frib->scale_factor[i]);
  }
  solo_add_list_entry(slm, aa, strlen(aa));

  sprintf(aa, "processor_const %f      "
	  , frib->processor_const      );
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "dly_tube_antenna %d	"
	  , frib->dly_tube_antenna	);
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "dly_rndtrip_chip_atod %d"
	  , frib->dly_rndtrip_chip_atod);
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "dly_timmod_testpulse %d"
	  , frib->dly_timmod_testpulse);
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "dly_modulator_on %d	"
	  , frib->dly_modulator_on	);
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "dly_modulator_off %d	"
	  , frib->dly_modulator_off	);
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "peak_power_offset %f    "
	  , frib->peak_power_offset    );
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "test_pulse_offset %f    "
	  , frib->test_pulse_offset    );
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "E_plane_angle %f        "
	  , frib->E_plane_angle        );
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "H_plane_angle %f        "
	  , frib->H_plane_angle        );
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "encoder_antenna_up %f   "
	  , frib->encoder_antenna_up   );
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "pitch_antenna_up %f     "
	  , frib->pitch_antenna_up     );
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "indepf_times_flg %d	"
	  , frib->indepf_times_flg	);
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "indep_freq_gate %d	"
	  , frib->indep_freq_gate	);
  solo_add_list_entry(slm, aa, strlen(aa));
  sprintf(aa, "time_series_gate %d	"
	  , frib->time_series_gate	);
  solo_add_list_entry(slm, aa, strlen(aa));


  str_terminate (c, frib->file_name, sizeof (frib->file_name));
  sprintf(aa, "file_name %s", c);
  solo_add_list_entry(slm, aa, strlen(aa));
  solo_add_list_entry(slm, " ", 1);
  return;
}

/* c------------------------------------------------------------------------ */

void
dor_print_radd(radd, slm)
  struct radar_d *radd;
  struct solo_list_mgmt *slm;
{
    char str[128], *str_terminate();
    char *aa, bstr[128];

    aa = bstr;
    /* routine to print the contents of the descriptor */
    
    solo_add_list_entry(slm, " ", 1);
    sprintf(aa, "Contents of the radar descriptor  len: %d"
	    , sizeof(struct radar_d));
    solo_add_list_entry(slm, aa, strlen(aa));

    sprintf(aa, " radar_des[4]       %.4s", radd->radar_des        );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " radar_des_length   %d", radd->radar_des_length );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " radar_name[8]      %.8s"
	    , str_terminate(str, radd->radar_name, sizeof(radd->radar_name)));
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " radar_const        %f", radd->radar_const      );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " peak_power         %f", radd->peak_power       );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " noise_power        %f", radd->noise_power      );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " receiver_gain      %f", radd->receiver_gain    );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " antenna_gain       %f", radd->antenna_gain     );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " system_gain        %f", radd->system_gain      );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " horz_beam_width    %f", radd->horz_beam_width  );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " vert_beam_width    %f", radd->vert_beam_width  );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " radar_type         %d", radd->radar_type       );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " scan_mode          %d", radd->scan_mode        );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " req_rotat_vel      %f", radd->req_rotat_vel    );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " scan_mode_pram0    %f", radd->scan_mode_pram0  );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " scan_mode_pram1    %f", radd->scan_mode_pram1  );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " num_parameter_des  %d", radd->num_parameter_des);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " total_num_des      %d", radd->total_num_des    );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " data_compress      %d", radd->data_compress    );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " data_reduction     %d", radd->data_reduction   );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " data_red_parm0     %f", radd->data_red_parm0   );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " data_red_parm1     %f", radd->data_red_parm1   );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " radar_longitude    %f", radd->radar_longitude  );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " radar_latitude     %f", radd->radar_latitude   );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " radar_altitude     %f", radd->radar_altitude   );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " eff_unamb_vel      %f", radd->eff_unamb_vel    );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " eff_unamb_range    %f", radd->eff_unamb_range  );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " num_freq_trans     %d", radd->num_freq_trans   );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " num_ipps_trans     %d", radd->num_ipps_trans   );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " freq1              %f", radd->freq1            );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " freq2              %f", radd->freq2            );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " freq3              %f", radd->freq3            );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " freq4              %f", radd->freq4            );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " freq5              %f", radd->freq5            );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " interpulse_per1    %f", radd->interpulse_per1  );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " interpulse_per2    %f", radd->interpulse_per2  );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " interpulse_per3    %f", radd->interpulse_per3  );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " interpulse_per4    %f", radd->interpulse_per4  );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " interpulse_per5    %f", radd->interpulse_per5  );
    solo_add_list_entry(slm, aa, strlen(aa));

    return;
}
/* c------------------------------------------------------------------------ */

void
dor_print_rktb(rktb, slm)
  struct rot_ang_table *rktb;	
  struct solo_list_mgmt *slm;
{
    char *aa, *bb, bstr[128];
    int ii;
    struct rot_table_entry *rte;

    aa = bstr;

    /* routine to print the contents of the rotation angle table */

    solo_add_list_entry(slm, " ", 1);
    sprintf(aa, "Contents of the rotation angle table  len: %d"
	    , sizeof(struct rot_ang_table));
    solo_add_list_entry(slm, aa, strlen(aa));

    sprintf(aa, " name_struct[4]     %.4s", rktb->name_struct );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " sizeof_struct      %d", rktb->sizeof_struct );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " angle2ndx          %.3f", rktb->angle2ndx );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " ndx_que_size       %d", rktb->ndx_que_size );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " first_key_offset   %d", rktb->first_key_offset );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " angle_table_offset %d", rktb->angle_table_offset );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " num_rays           %d", rktb->num_rays );
    solo_add_list_entry(slm, aa, strlen(aa));

    bb = (char *)rktb + rktb->first_key_offset;


    for(ii=0; ii < rktb->num_rays; ii++) {
	rte = (struct rot_table_entry *)bb;
	sprintf(aa, "%4d %7.2f %8d %7d\n"
		, ii
		, rte->rotation_angle
		, rte->offset
		, rte->size
		);
	bb += sizeof(struct rot_table_entry);
	solo_add_list_entry(slm, aa, strlen(aa));
     }

    return;
}
/* c------------------------------------------------------------------------ */

void
dor_print_ryib(ryib, slm)
  struct ray_i *ryib;
  struct solo_list_mgmt *slm;
{
    char *aa, bstr[128];

    aa = bstr;

    /* routine to print the contents of the descriptor */

    solo_add_list_entry(slm, " ", 1);
    sprintf(aa, "Contents of the ray descriptor  len: %d"
	    , sizeof(struct ray_i));
    solo_add_list_entry(slm, aa, strlen(aa));

    sprintf(aa, " ray_info[4]      %.4s", ryib->ray_info        );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " ray_info_length  %d", ryib->ray_info_length );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " sweep_num        %d", ryib->sweep_num       );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " julian_day       %d", ryib->julian_day      );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " hour             %d", ryib->hour            );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " minute           %d", ryib->minute          );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " second           %d", ryib->second          );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " millisecond      %d", ryib->millisecond     );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " azimuth          %f", ryib->azimuth         );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " elevation        %f", ryib->elevation       );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " peak_power       %f", ryib->peak_power      );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " true_scan_rate   %f", ryib->true_scan_rate  );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " ray_status       %d", ryib->ray_status      );
    solo_add_list_entry(slm, aa, strlen(aa));

    return;
}
/* c------------------------------------------------------------------------ */

void
dor_print_swib(swib, slm)
  struct sweepinfo_d *swib;
  struct solo_list_mgmt *slm;
{
    char *str_terminate();
    char *aa, bstr[128];

    aa = bstr;

    solo_add_list_entry(slm, " ", 1);
    sprintf(aa, "Contents of the sweep descriptor  len: %d"
	    , sizeof(struct sweepinfo_d));

    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "sweep_des[4]     %.4s", swib->sweep_des);       
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "sweep_des_length %d", swib->sweep_des_length);
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "radar_name[8]    %.8s", swib->radar_name);      
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "sweep_num        %d", swib->sweep_num);       
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "num_rays         %d", swib->num_rays);        
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "start_angle      %f", swib->start_angle);     
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "stop_angle       %f", swib->stop_angle);      
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "fixed_angle      %f", swib->fixed_angle);     
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, "filter_flag      %d", swib->filter_flag);     
    solo_add_list_entry(slm, aa, strlen(aa));

    return;
}
/* c------------------------------------------------------------------------ */

void
dor_print_sswb(sswb, slm)
  struct super_SWIB *sswb;
  struct solo_list_mgmt *slm;
{
    int ii;
    char str[128], *str_terminate();
    char *aa, bstr[128];
    DD_TIME dts;
    double dt;
    time_t ttime;

    aa = bstr;

    /* routine to print the contents of the volume descriptor */

    solo_add_list_entry(slm, " ", 1);
    sprintf(aa, "Contents of the super sweepinfo descriptor  len: %d\n"
	    , sizeof(struct super_SWIB));
    solo_add_list_entry(slm, aa, strlen(aa));

    sprintf(aa, " name_struct        %.4s   ", sswb->name_struct       );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " sizeof_struct      %d   ", sswb->sizeof_struct     );
    solo_add_list_entry(slm, aa, strlen(aa));

    dts.time_stamp = sswb->last_used;
    sprintf(aa, " last_used          %d  %s ", sswb->last_used
	, dts_print( d_unstamp_time( &dts )));
    solo_add_list_entry(slm, aa, strlen(aa));

    dts.time_stamp = sswb->start_time;
    sprintf(aa, " start_time         %d  %s ", sswb->start_time
        , dts_print( d_unstamp_time( &dts )));
    solo_add_list_entry(slm, aa, strlen(aa));

    dts.time_stamp = sswb->stop_time;
    sprintf(aa, " stop_time          %d  %s ", sswb->stop_time
        , dts_print( d_unstamp_time( &dts )));
    solo_add_list_entry(slm, aa, strlen(aa));

    sprintf(aa, " sizeof_file        %d   ", sswb->sizeof_file       );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " compression_flag   %d   ", sswb->compression_flag  );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " volume_time_stamp  %d   ", sswb->volume_time_stamp );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " num_params         %d   ", sswb->num_params        );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " radar_name         %s   "					   
	    , str_terminate(str, sswb->radar_name, sizeof(sswb->radar_name)));
    solo_add_list_entry(slm, aa, strlen(aa));

    dts.time_stamp = sswb->d_start_time;
    sprintf(aa, " d_start_time       %.3f  %s ", sswb->d_start_time
        , dts_print( d_unstamp_time( &dts )));
    solo_add_list_entry(slm, aa, strlen(aa));

    dts.time_stamp = sswb->d_stop_time;
    sprintf(aa, " d_stop_time        %.3f  %s ", sswb->d_stop_time
        , dts_print( d_unstamp_time( &dts )));
    solo_add_list_entry(slm, aa, strlen(aa));

    sprintf(aa, " version_num        %d   ", sswb->version_num       );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " num_key_tables     %d   ", sswb->num_key_tables    );
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " status             %d   ", sswb->status            );
    solo_add_list_entry(slm, aa, strlen(aa));

    for(ii=0; ii < 7; ii++) {
	sprintf(aa, "place_holder[%d] %d\n", ii, sswb->place_holder[ii]);
	solo_add_list_entry(slm, aa, strlen(aa));
    }

    for(ii=0; ii < sswb->num_key_tables; ii++) {
	sprintf(aa, "key_table[%d]->  offset: %d  size: %d  type: %d\n"
		, ii
		, sswb->key_table[ii].offset
		, sswb->key_table[ii].size
		, sswb->key_table[ii].type
		);
	solo_add_list_entry(slm, aa, strlen(aa));
    }

    return;
}
/* c------------------------------------------------------------------------ */

void
dor_print_vold(vold, slm)
  struct volume_d *vold;
  struct solo_list_mgmt *slm;
{
    char str[128], *str_terminate();
    char *aa, bstr[128];

    aa = bstr;

    /* routine to print the contents of the volume descriptor */

    solo_add_list_entry(slm, " ", 1);
    sprintf(aa, "Contents of the volume descriptor  len: %d"
	    , sizeof(struct volume_d));
    solo_add_list_entry(slm, aa, strlen(aa));

    sprintf(aa, " volume_des[4]      %.4s", vold->volume_des);                                                     
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " volume_des_length  %d", vold->volume_des_length);                                              
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " format_version     %d", vold->format_version     );                                            
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " volume_num         %d", vold->volume_num           );                                          
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " maximum_bytes      %d", vold->maximum_bytes        );                                          
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " proj_name[20]      %.20s"
	    , str_terminate(str, vold->proj_name, sizeof(vold->proj_name)));         
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " year               %d", vold->year                 );                                          
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " month              %d", vold->month                );                                          
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " day                %d", vold->day                  );                                          
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " data_set_hour      %d", vold->data_set_hour        );                                          
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " data_set_minute    %d", vold->data_set_minute      );                                          
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " data_set_second    %d", vold->data_set_second      );                                          
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " flight_num[8]      %.8s"
	    , str_terminate(str, vold->flight_num, sizeof(vold->flight_num)));       
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " gen_facility[8]    %.8s",
	    str_terminate(str, vold->gen_facility, sizeof(vold->gen_facility)));   
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " gen_year           %d", vold->gen_year             );                                          
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " gen_month          %d", vold->gen_month            );                                          
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " gen_day            %d", vold->gen_day              );                                          
    solo_add_list_entry(slm, aa, strlen(aa));
    sprintf(aa, " number_sensor_des  %d", vold->number_sensor_des    );                                          
    solo_add_list_entry(slm, aa, strlen(aa));

    return;
}
/* c------------------------------------------------------------------------ */

void dd_radar_angles( asib, cfac, ra, dgi )
  struct platform_i *asib;
  struct correction_d *cfac;
  struct radar_angles *ra;
  struct dd_general_info *dgi;
{
    /* compute the true azimuth, elevation, etc. from platform
     * parameters using Testud's equations with their different
     * definitions of rotation angle, etc.
     */
    double R, H, P, D, T, theta_a, tau_a, theta_t, tau_t, lambda_t, phi_t;
    double sinP, cosP, sinT, cosT, sinD, cosD, sinR, cosR;
    double sin_theta_rc, cos_theta_rc, sin_tau_a, cos_tau_a;
    double xsubt, ysubt, zsubt, lambda, phi;

    double d, bigR, az;
    double sin_el, cos_az, cos_el, cos_psi, cos_phi;
    double sin_pitch, cos_pitch, cos_drift, sin_tilt, cos_tilt;
    double rtilt, rtrack, rphi;
    double rpitch=RADIANS(asib->pitch +cfac->pitch_corr);
    double rhead=RADIANS(asib->heading +cfac->heading_corr);
    double rdrift=RADIANS(asib->drift_angle +cfac->drift_corr);
    double azr, azd;
    double lambda_a, phi_a;
    double sin_lambda_a, cos_lambda_a, sin_phi_a, cos_phi_a;


# ifdef WEN_CHOWS_ALGORITHM
    /*
     * see Wen-Chau Lee's paper
     * "Mapping of the Airborne Doppler Radar Data"
     */
    d = asib->roll;
    R = dd_isnanf(d) ? 0 : RADIANS(d +cfac->roll_corr);

    d = asib->pitch;
    P = dd_isnanf(d) ? 0 : RADIANS(d +cfac->pitch_corr);

    d = asib->heading;
    H = dd_isnanf(d) ? 0 : RADIANS(d +cfac->heading_corr);

    d = asib->drift_angle;
    D = dd_isnanf(d) ? 0 : RADIANS(d +cfac->drift_corr);

    sinP = sin(P);
    cosP = cos(P);
    sinD = sin(D);
    cosD = cos(D);
    
    T = H + D;

    if( dgi->dds->radd->radar_type == AIR_LF ||
	dgi->dds->radd->radar_type == AIR_NOSE ) {

	d = ( dgi->dds->ryib->azimuth );
	lambda_a = dd_isnanf(d) ? 0 : RADIANS
	  ( CART_ANGLE( d +cfac->azimuth_corr ));
	sin_lambda_a = sin(lambda_a);
	cos_lambda_a = cos(lambda_a);

	d = ( dgi->dds->ryib->elevation );
	phi_a = dd_isnanf(d) ? 0 : RADIANS( d +cfac->elevation_corr );
	sin_phi_a = sin(phi_a);
	cos_phi_a = cos(phi_a);

	sinR = sin(R);
	cosR = cos(R);

	ra->x = xsubt = cos_lambda_a * cos_phi_a *
	    (cosD * cosR - sinD * sinP * sinR) -
	    sinD * cosP * sin_lambda_a * cos_phi_a +
	    sin_phi_a * ( cosD * sinR + sinD * sinP * cosR );

	ra->y = ysubt = cos_lambda_a * cos_phi_a *
	    ( sinD * cosR + cosD * sinP * sinR ) +
	    cosD * cosP * sin_lambda_a * cos_phi_a +
	    sin_phi_a * ( sinD * sinR - cosD * sinP * cosR );

	ra->z = zsubt = -cosP * sinR * cos_lambda_a * cos_phi_a +
	    sinP * sin_lambda_a * cos_phi_a +
	    cosP * cosR * sin_phi_a;

	ra->rotation_angle = theta_t = atan2(xsubt, zsubt);
	ra->tilt = tau_t = asin(ysubt);
	lambda_t = atan2(xsubt, ysubt);
	ra->azimuth = fmod(lambda_t + T, TWOPI);
	ra->elevation = phi_t = asin(zsubt);

	return;
    }

    sinT = sin(T);
    cosT = cos(T);

    d = asib->rotation_angle;
    theta_a = dd_isnanf(d) ? 0 : RADIANS(d +cfac->rot_angle_corr);
    
    d = asib->tilt;
    tau_a = dd_isnanf(d) ? 0 : RADIANS(d +cfac->tilt_corr);
    sin_tau_a = sin(tau_a);
    cos_tau_a = cos(tau_a);
    sin_theta_rc = sin(theta_a + R); /* roll corrected rotation angle */
    cos_theta_rc = cos(theta_a + R); /* roll corrected rotation angle */

    ra->x = xsubt = (cos_theta_rc * sinD * cos_tau_a * sinP
	     + cosD * sin_theta_rc * cos_tau_a
	     -sinD * cosP * sin_tau_a);
    ra->y = ysubt = (-cos_theta_rc * cosD * cos_tau_a * sinP
	     + sinD * sin_theta_rc * cos_tau_a
	     + cosP * cosD * sin_tau_a);
    ra->z = zsubt = (cosP * cos_tau_a * cos_theta_rc
	     + sinP * sin_tau_a);

    ra->rotation_angle = theta_t = atan2(xsubt, zsubt);
    ra->tilt = tau_t = asin(ysubt);
    lambda_t = atan2(xsubt, ysubt);
    ra->azimuth = fmod(lambda_t + T, TWOPI);
    ra->elevation = phi_t = asin(zsubt);
    return;

# else

    rtrack = rhead +rdrift;
    ra->rotation_angle = RADIANS(asib->rotation_angle +cfac->rot_angle_corr
			+asib->roll +cfac->roll_corr);
    ra->tilt = RADIANS(asib->tilt +cfac->tilt_corr);
    azr = rhead +PI -(PIOVR2+ra->tilt)*sin(ra->rotation_angle);
    
    rphi = (ra->rotation_angle + PI);
    sin_pitch = sin(rpitch); cos_pitch = cos(rpitch);
    sin_tilt = sin(ra->tilt); cos_tilt = cos(ra->tilt);
    cos_drift = cos(rdrift);
    cos_phi = cos(rphi);
    
    cos_psi = cos_pitch*cos_drift*sin_tilt
	  + sin_pitch*cos_drift*cos_tilt*cos_phi
		- sin(rdrift)*cos_tilt*sin(rphi);
    /* psi is the angle between the beam and the
     * horizontal component of the aircraft velocity vector
     */
    ra->psi = acos(cos_psi);

    sin_el = sin_tilt*sin_pitch-cos_tilt*cos_pitch*cos_phi;
    ra->elevation = asin(sin_el);
    cos_el = cos(ra->elevation);
    cos_az = (sin_tilt-sin_pitch*sin_el)/(cos_pitch*cos_el);

    if(fabs(cos_az) <= 1.0 )
	  az = acos(cos_az);
    else {
	az = cos_az > 1.0 ? 0 : PI;
	printf( "cos_az:%f\n", cos_az );
    }

    if(ra->rotation_angle >= PI && ra->rotation_angle < TWOPI ) {
	az = TWOPI -az;
    }
    az -= rdrift;

    /* compute x,y,z */
    ra->x = sin(az)*cos_el;
    ra->y = cos(az)*cos_el;
    ra->z = sin_el;

    if( ra->elevation >= 0 ) {
	ra->z = fabs((double)ra->z);
    }
    else {
	ra->z = -fabs((double)ra->z);
    }
    if(az >= 0 && az <= PIOVR2) {
	ra->x = fabs((double)ra->x);
	ra->y = fabs((double)ra->y);
    }
    else if( az > PIOVR2 && az <= PI ) {
	ra->x = fabs((double)ra->x);
	ra->y = -fabs((double)ra->y);
    }
    else if( az > PI && az <= PI+PIOVR2 ) {
	ra->x = -fabs((double)ra->x);
	ra->y = -fabs((double)ra->y);
    }
    else if( az > PI+PIOVR2 && az < TWOPI ) {
	ra->x = -fabs((double)ra->x);
	ra->y = fabs((double)ra->y);
    }
    ra->azimuth = fmod(az+rtrack, TWOPI);
    d = ra->x*ra->x + ra->z*ra->z;
    bigR = sqrt(d);
    ra->tilt = atan2((double)ra->y, bigR);
    ra->rotation_angle = atan2((double)ra->x, (double)ra->z);

    return;
# endif
}
/* c------------------------------------------------------------------------ */

char *dd_radar_name(dds)
  struct dds_structs *dds;
{
    int i;
    static char name[12];
    char *a;
    struct radar_d *radd=dds->radd;
    
    strncpy( name, radd->radar_name, 8 );

    /* remove trailing blanks or extra nulls */
    for(i=8,a=name+7; i && (*a == ' ' || *a == '\0'); i--, a-- ) ;
    *(name+i) = '\0';

    /* replace any blanks in the name with underscores */
    for(a=name; *a; a++ ) {
	if(*a == ' ' || *a == '/')
	      *a = '_';
    }
    return( name );
}
/* c------------------------------------------------------------------------ */

char *dd_radar_namec(name0)
  char *name0;
{
    /* try and cleanup the radar name
     */
    int ii, nn=8;		/* process at most 8 characters */
    static char name[12];
    char *a, *b, *c;
    
    a = name0;
    for(; nn && *a == ' '; nn--, a++); /* in case there are
					* leading blanks */
    c = name;
    for(; nn && *a; nn--, *c++ = *a++);
    for(; name < c && *(c-1) == ' '; c--); /* remove trailing blanks */
    *c = '\0';
    for(c=name; *c; c++)
	  if(*c == ' ' || *c == '/')
		*c = '_';	/* substitute for blanks or slashes */
    return(name);
}
/* c------------------------------------------------------------------------ */

void dd_radar_selected( radar_name, radar_num, difs)
  char *radar_name;
  int radar_num;
  struct dd_input_filters *difs;
{
    int ii, jj, kk, nt;
    char *a, *b, *c, str[256], *get_tagged_string(), radar[16];
    char *str_ptrs[32];
    char *dd_whiteout(), *dd_delimit(), *str_terminate();

    if(!(a=get_tagged_string("SELECT_RADARS"))) {
	for(ii=0; ii < MAX_SENSORS; ii++ )
	    difs->radar_selected[ii] = YES;
	return;
    }
    str_terminate(radar, radar_name, 8);
    strcpy(str, a);
    nt = dd_tokens(str, str_ptrs);
    a = dd_whiteout(a);

# ifdef notyet
    for(ii=0; ii < nt; ii++) {
	if(strstr(radar,str_ptrs[ii])) {
	    difs->radar_selected[radar_num] = YES;
	    difs->num_selected_radars++;
	}
    }
# else
    while(*a) {
	b = dd_delimit(a);
	/* see if we can construct a select string */
	for(c=str; a < b; *c++ = *a++);
	if(c == str) break;
	*c = '\0';
	if(strstr(radar,str)) {
	    difs->radar_selected[radar_num] = YES;
	    difs->num_selected_radars++;
	}
	a = dd_whiteout(a);
    }
# endif
}
/* c------------------------------------------------------------------------ */

void dd_range_gate( dgi, req_range, gate, val )
  DGI_PTR dgi;
  int *gate;
  float *req_range, *val;
{
    /* return the gate and range of the gate nearest the requested range
     */
    DDS_PTR dds=dgi->dds;
    struct cell_d *celv=dds->celvc;
    int i, j, parameter_num=0;
    double diff=1.e11, last_d=1.e11;
    double d, r= *req_range; /* assume meters */
    double deltag;

    if( r <= celv->dist_cells[0]) {
	*gate = 1;
	*val = celv->dist_cells[0];
	return;
    }
    if( r >= celv->dist_cells[celv->number_cells-1]) {
	*gate = celv->number_cells;
	*val = celv->dist_cells[celv->number_cells-1];
	return;
    }
# ifdef obsolete
    *gate = 1 +fp_bin_search( celv->dist_cells, celv->number_cells, r );
# else
    *gate = 1 + dd_cell_num(dgi->dds, parameter_num, r);
# endif
    *val = celv->dist_cells[*gate-1];

    return;
}
/* c------------------------------------------------------------------------ */

struct dd_ray_sector *
dd_ray_coverage(dgi, rat, rn, expansion)
  DGI_PTR dgi;
  struct rot_ang_table *rat;
  int rn;			/* ray number (starts at 1) */
  float expansion;
{
    /* this routine returns the sector in degrees covered by
     * the request ray
     */
    struct rot_table_entry *ray0, *ray1, *ray2, *return_rotang1();
    static struct dd_ray_sector rs;
    double d, angdiff(), sector;
    
    ray0 = (struct rot_table_entry *)((char *)rat +
					rat->first_key_offset);
    if(rn == 1) {
	ray1 = ray0 +1;
	rs.sector = angdiff(ray0->rotation_angle, ray1->rotation_angle);
	rs.angle0 = FMOD360(ray0->rotation_angle - .5 * rs.sector +360.);
	rs.angle1 = FMOD360(ray0->rotation_angle + .5 * rs.sector +360.);
	rs.rotation_angle = ray0->rotation_angle;
    }
    else if(rn >= rat->num_rays) {
	rn = rat->num_rays;
	ray0 += rn-2;
	ray1 = ray0 +1;
	rs.sector = angdiff(ray0->rotation_angle
		    , ray1->rotation_angle);
	rs.angle0 = FMOD360(ray1->rotation_angle - .5 * rs.sector +360.);
	rs.angle1 = FMOD360(ray1->rotation_angle + .5 * rs.sector +360.);
	rs.rotation_angle = ray1->rotation_angle;
    }
    else {
	ray0 += rn-2;
	ray1 = ray0 +1;
	ray2 = ray0 +2;
	
	rs.angle0 = ray0->rotation_angle + .5 * angdiff
	      (ray0->rotation_angle, ray1->rotation_angle);
	rs.angle1 = ray1->rotation_angle + .5 * angdiff
	      (ray1->rotation_angle, ray2->rotation_angle);
	
	rs.sector = angdiff(rs.angle0, rs.angle1);
	rs.rotation_angle = ray1->rotation_angle;
    }
    if(expansion) {
	if(expansion < .9) {
	    sector = rs.sector < 0 ? -expansion : expansion;
	}
	else
	      sector = expansion*rs.sector;
	rs.angle1 = FMOD360(rs.angle0 + sector +360.);
    }
    return(&rs);
}
/* c------------------------------------------------------------------------ */

void dd_razel_to_xyz(p0)
  struct point_in_space *p0;
{
    /* routine to calculate (x,y,z) (from az,el,rng)
     *
     */
    double d, r, rxy, theta, phi;

    phi = RADIANS(p0->elevation);
    p0->z = p0->range * sin(phi);
    rxy = p0->range * cos(phi);
    theta = RADIANS(CART_ANGLE(p0->azimuth));
    p0->x = rxy * cos(theta);
    p0->y = rxy * sin(theta);
}
/* c------------------------------------------------------------------------ */

void dd_reset_d_limits(at)
  struct d_limits **at;
{
    if(!(*at)) {
	*at = (struct d_limits *)malloc(sizeof(struct d_limits));
	memset(*at, 0, sizeof(struct d_limits));
    }
    (*at)->lower = -MAX_FLOAT;
    (*at)->upper =  MAX_FLOAT;
}
/* c------------------------------------------------------------------------ */

int dd_return_id_num(name)
  char *name;
{
    char *a, *b, str[16], *str_terminate();

    a = str_terminate(str, name, 8);
    b = "DBZ";
    if(strstr(a, b)) {
	return(DBZ_ID_NUM);
    }
    b = "SW";
    if(strstr(a, b)) {
	return(SW_ID_NUM);
    }
    b = "VR";
    if(strstr(a, b)) {
	return(VR_ID_NUM);
    }
    b = "NCP";
    if(strstr(a, b)) {
	return(NCP_ID_NUM);
    }
    b = "DZ";
    if(strstr(a, b)) {
	return(DZ_ID_NUM);
    }
    b = "UZ";
    if(strstr(a, b)) {
	return(DZ_ID_NUM);
    }
    b = "UDBZ";
    if(strstr(a, b)) {
	return(DZ_ID_NUM);
    }
    b = "VE";
    if(strstr(a, b)) {
	return(VE_ID_NUM);
    }
    b = "VG";
    if(strstr(a, b)) {
	return(VG_ID_NUM);
    }
    b = "VU";
    if(strstr(a, b)) {
	return(VU_ID_NUM);
    }

    return(0);
}
/* c------------------------------------------------------------------------ */

struct rot_table_entry *
dd_return_rotang1(rat)
  struct rot_ang_table *rat;
{
    struct rot_table_entry *ray1;

    ray1 = (struct rot_table_entry *)((char *)rat + rat->first_key_offset);
    return(ray1);
}
/* c------------------------------------------------------------------------ */

double
dd_roll(dgi)
  struct dd_general_info *dgi;
{
    double d=dgi->dds->asib->roll;
    if(dd_isnanf(d))
	  return((double)0);
    else
	  return(d +dgi->dds->cfac->roll_corr);
}
/* c------------------------------------------------------------------------ */

int dd_rotang_seek(rat, rotang)
  struct rot_ang_table *rat;
  float rotang;
{
    /* returns the index of the ray closest to the requested
     * rotation angle
     */
    int i, j, k, n=rat->ndx_que_size, ang_ndx, ndx;
    long *iptr;
    char *c=(char *)rat;
    struct rot_table_entry *entry1;
    double diff0, diff1, diff2;

    if(rat->num_rays < 1)
	  return(-1);

    for(; rotang < 0; rotang+=360.);
    for(; rotang >= 360.; rotang-=360.);

    entry1 = (struct rot_table_entry *)((char *)rat +
					rat->first_key_offset);
    iptr = (long *)(c + rat->angle_table_offset);
    ang_ndx = rotang*rat->angle2ndx;

    /* get ray index(s) from the angle index table
     */
    if(*(iptr+ang_ndx) >= 0) {	/* found one */
       ndx = j = *(iptr+ang_ndx);
       i = DEC_NDX(j, n);
       k = INC_NDX(j, n);
       diff1 = fabs((double)(rotang-(entry1+i)->rotation_angle));
       diff0 = fabs((double)(rotang-(entry1+j)->rotation_angle));
       diff2 = fabs((double)(rotang-(entry1+k)->rotation_angle));
       if (diff1 < diff0)
	 { diff0 = diff1; ndx = i; }
       if (diff2 < diff0)
	 { return k; }
       return ndx;
    }
    if(*(iptr+ang_ndx) >= 0)	/* found one */
	  return(*(iptr+ang_ndx));

    /* otherwise we have an empty slot
     * look for an entries on either side of the rotang
     */
    j = DEC_NDX(ang_ndx, n);
    for(i=0; i < n && *(iptr+j) < 0; i++) {
	j = DEC_NDX(j,n);
    }
    j = *(iptr+j);		/* this is the ray index now */

    k = INC_NDX(ang_ndx, n);
    for(i=0; i < n && *(iptr+k) < 0; i++) {
	k = INC_NDX(k,n);
    }
    k = *(iptr+k);

    /* we now have one entry on either side of the requested
     * rotation angle
     */
    diff1 = fabs((double)(rotang-(entry1+j)->rotation_angle));
    diff2 = fabs((double)(rotang-(entry1+k)->rotation_angle));
    i = diff1 < diff2 ? j : k;
    return(i);
}
/* c------------------------------------------------------------------------ */

void dd_rotang_table(dgi, func)
  DGI_PTR dgi;
  int func;
{
    struct rot_ang_table *rat=dgi->rat;
    struct rot_table_entry *next;
    int inc_entries=480, size;
    static int count=0, trip=170670;
    char *c;
    int i, j, k, mm, ndx, mark, angle_ndx_size=480, first=NO, glitch=NO;
    long *lp;
    float f, rot, angle;
    double dd_rotation_angle();
    unsigned long nan = 0x7fffff, *langle;

    /* c...mark */

    if(++count >= trip) {
	mark = 0;
    }
    if(!rat || rat->num_rays >= dgi->max_rat_entries) {
	/* Initialize or resize rat
	 */
	dgi->max_rat_entries += inc_entries;
	mm = sizeof(struct rot_ang_table);
	mm = ((mm -1)/8 +1) * 8; /* start tables on an 8 byte boundary */

	size = mm + angle_ndx_size * sizeof(long) +
	  dgi->max_rat_entries * sizeof(struct rot_table_entry);

	if(!rat) {
	    first = YES;
	    c = (char *)malloc(size);
	    memset(c, 0, size);
	}
	else {
	    c = (char *)realloc((char *)rat, size);
	}
	if(!c) {
	    printf("Unable to malloc rotang table\n");
	    exit(1);
	}
	dgi->rat = rat = (struct rot_ang_table *)c;
	strncpy(rat->name_struct, "RKTB", 4);
	rat->sizeof_struct = size;

	rat->angle_table_offset = mm;
	dgi->rat_angle_ndx1 = (long *)(c + rat->angle_table_offset);
	rat->angle2ndx = (float)angle_ndx_size/360.;
	rat->ndx_que_size = angle_ndx_size;

	rat->first_key_offset = rat->angle_table_offset +
	      rat->ndx_que_size * sizeof(long);
	dgi->rat_entry1 = (struct rot_table_entry *)
	      (c + rat->first_key_offset);
	mark = 0;
    }

    if(func == RESET || first) {
	/* reset angle index table */
	for(i=0; i < angle_ndx_size; *(dgi->rat_angle_ndx1 + i++) = -1);
	if(func == RESET) {
	    rat->num_rays = 0;
	    return;
	}
    }

    /* put this entry in the table */

    ndx = rat->num_rays++;

    if((angle = dd_rotation_angle(dgi)) < 0) {
	if((angle = angle +360.) < 0)
	      i = 0;
	else
	      i = angle * rat->angle2ndx;
    }
    else if(angle >= 360.)
	  i = angle_ndx_size -1;
    else
	  i = angle * rat->angle2ndx;

    if(i >= angle_ndx_size || i < 0) {
	i = 0;
    }
    next = dgi->rat_entry1 + ndx;
    next->rotation_angle = angle;
    next->offset = dgi->ray_que->disk_offset;
    next->size = dgi->ray_que->sizeof_ray;
    *(dgi->rat_angle_ndx1+i) = ndx;
}
/* c------------------------------------------------------------------------ */

double
dd_rotation_angle(dgi)
  struct dd_general_info *dgi;
{
# define Not_Track_Relative

    double d_rot, dd_azimuth_angle();


    switch(dgi->dds->radd->scan_mode) {

    case AIR:
# ifdef Not_Track_Relative
	d_rot = FMOD360(dgi->dds->asib->rotation_angle +
			dgi->dds->cfac->rot_angle_corr +
			dd_roll(dgi));
# else
	d_rot = FMOD360(DEGREES(dgi->dds->ra->rotation_angle)+360.);
# endif
	break;

    case RHI:
	d_rot = FMOD360((450.-(dgi->dds->ryib->elevation
			       + dgi->dds->cfac->elevation_corr)));
	break;

    case TAR:
	d_rot = dgi->dds->radd->radar_type != GROUND ?
	      FMOD360(dgi->dds->asib->rotation_angle +
			dgi->dds->cfac->rot_angle_corr) :
			      dd_azimuth_angle(dgi);
	break;

    default:
	d_rot = dd_azimuth_angle(dgi);
	break;
    }
    return(d_rot);
}
/* c------------------------------------------------------------------------ */

void dd_rng_info( dgi,  g1, m, rngs, ng )
  DGI_PTR dgi;
  int *g1, *m, *ng;
  float *rngs;
{
    DDS_PTR dds=dgi->dds;
    /* routine to return "m" range values */
    struct cell_d *celv=dds->celvc;
    int i=(*g1-1), n, nc;

    nc = celv->number_cells;

    if((n=i+(*m)) > nc)
	  n = nc;

    for(; i < n; i++)
	  *rngs++ = celv->dist_cells[i]; /* return in meters */

    *ng = nc;
}
/* c------------------------------------------------------------------------ */

void
dd_set_uniform_cells(dds)
  struct dds_structs *dds;
{
    /* set up to do quick calculation of a cell number for a given range
     * the assumption here is that the original cell vector contains
     * data that is non-uniformly spaced
     */
    struct cell_d *celv=dds->celvc;
    int ndx=0, nc, just_the_cell_count=YES;
    int nu;
    float cell_spacing, cx, r0, r;
    void dd_uniform_cell_spacing();


    /* the lowest resolution cell spacing is based on the
     * distance between the first two cells
     */
    dds->uniform_cell_count[ndx] = 0;
    dds->rcp_uniform_cell_spacing[ndx] = 0;

    nc = celv->number_cells-1;
    if((nc = celv->number_cells-1) < 1) {
       printf("\n**** Bad cell count info in dd_set_uniform_cells(dds)\n");
       return;
    }
    cell_spacing = celv->dist_cells[1]
	  - celv->dist_cells[0];
    if(cell_spacing <= 0) {
       printf("\n**** Bad cell spacing info in dd_set_uniform_cells(dds)\n");
       return;
    }
    r0 = celv->dist_cells[0];
    r = celv->dist_cells[nc] -r0;

    if((nc = 1.5 + r/cell_spacing) > 32000) {
       printf("\n**** REALLY Bad cell count info in dd_set_uniform_cells(dds)\n");
       return;
    }
    
    /*
     * make sure there is space for it
     */
    if(!dds->uniform_cell_lut[ndx] || dds->uniform_cell_count[ndx] < nc) {
	if(dds->uniform_cell_lut[ndx])
	      free(dds->uniform_cell_lut[ndx]);
	dds->uniform_cell_lut[ndx] = (int *)malloc(nc*sizeof(int));
	memset(dds->uniform_cell_lut[ndx], 0, nc*sizeof(int));
	dds->uniform_cell_count[ndx] = nc;
    }
    /* now really create a table of equivalent cell numbers
     * to the uniform value
     */
    dd_uniform_cell_spacing(celv->dist_cells
			    , celv->number_cells
			    , cell_spacing
			    , dds->uniform_cell_lut[ndx]
			    , r0, nc);

    dds->rcp_uniform_cell_spacing[ndx] = 1./cell_spacing;
    dds->uniform_cell_zero[ndx] = r0;
    return;
}
/* c------------------------------------------------------------------------ */

void dd_site_name(proj_name, site_name)
  char *proj_name, *site_name;
{
    /* add the site name to the end of the project name */

    int m, n;

    m = strlen(site_name) > 8 ? 8 : strlen(site_name);

    for(n=0; n < 20 && *(proj_name+n); n++); /* length of project name */

    if( n > 19-m ) {
	n = 19-m;
	*(proj_name +n) = '\0';
	n++;
    }
    strncpy(proj_name+n, site_name, m);
    if(m+n < 20)
	  *(proj_name+m+n) = '\0';
}
/* c------------------------------------------------------------------------ */

int dd_src_fid(io_type)
  int *io_type;
{
    char *a, *get_tagged_string();
    int fid;

    if((a=get_tagged_string("SOURCE_DEV"))) {
	/* see if the string contains a physical device name */
	if(dd_itsa_physical_dev(a))
	      *io_type = PHYSICAL_TAPE;
	else
	      *io_type = FB_IO;
	/*
	 * open the file
	 */
	printf( "Input file name: %s\n", a );
	if((fid = open(a, 0 )) < 0) { 
	    printf( "Could not open input file %s  error=%d\n", a, fid);
	    exit(1);
	}
	return(fid);
    }
    printf("No SOURCE_DEV specification!\n");
    exit(1);
}
/* c------------------------------------------------------------------------ */

double
dd_tilt_angle(dgi)
  struct dd_general_info *dgi;
{
    double d_rot;

    switch(dgi->dds->radd->scan_mode) {
	
    case AIR:
	d_rot = DEGREES(dgi->dds->ra->tilt+dgi->dds->cfac->tilt_corr);
	break;

    case RHI:
	d_rot = CART_ANGLE
	      (dgi->dds->ryib->azimuth +dgi->dds->cfac->azimuth_corr);
	break;

    default:
	d_rot = dgi->dds->ryib->elevation +dgi->dds->cfac->elevation_corr;
	break;
    }
    return(d_rot);
}
/* c------------------------------------------------------------------------ */

int dd_tokens(att, str_ptrs)
  char *att, *str_ptrs[];
{
    int nargs=0;
    char *b=att, *dlims=" \t\n"; /* blank, tab, and new line */
    char *strtok();

    for(b=strtok(b, dlims);  b; b = strtok(NULL, dlims)) {
	str_ptrs[nargs++] = b;
    }
    return(nargs);
}
/* c------------------------------------------------------------------------ */

int dd_tokenz(att, str_ptrs, dlims)
  char *att, *str_ptrs[], *dlims; /* token delimiters */
{
    int nargs=0;
    char *b=att;
    char *strtok();

    for(b=strtok(b, dlims); b;  b = strtok(NULL, dlims)) {
	str_ptrs[nargs++] = b;
    }
    return(nargs);
}
/* c------------------------------------------------------------------------ */

void
dd_uniform_cell_spacing(src_cells, num_cells, cell_spacing, cell_lut
			, r0, nuc)
  float src_cells[], cell_spacing, r0;
  int num_cells, *cell_lut, nuc;
{
    /* create a lookup table that facilitates getting from a range
     * in meters to a cell number quickly
     * i.e.
     * uniform_cell_num = (range - r0)/cell_spacing;
     * source_cell_num = *(cell_lut + uniform_cell_num);
     */
    int ii=0, gg=0;
    float rr, d1, d2;
    int *lut=cell_lut;

    if(cell_spacing <=0)
	  return;
    rr = r0;
    d1 = src_cells[0];
    d2 = src_cells[1];

    for(--nuc; ii < nuc; ++ii, ++lut, rr += cell_spacing) {

	if(FABS(rr - d1) > .5 * FABS(d2 - d1)) {
	    d1 = d2;
	    d2 = src_cells[++gg];
	}
	*lut = gg;
    }
    *lut = gg;
    return;
}
/* c------------------------------------------------------------------------ */

char *dd_whiteout(c)
  char *c;
{
    /* advance beyond whitespace */
    for(; c && (*c == ' ' || *c == '\t' || *c == '\n'); c++ );
    return(c);
}
/* c------------------------------------------------------------------------ */

int dd_xy_plane_horizontal(dgi)
  struct dd_general_info *dgi;
{
    int ii, nn;
    
    nn = dgi->dds->radd->radar_type == AIR ? NO : YES;
    return(nn);
}
/* c------------------------------------------------------------------------ */

int fp_bin_search( f, nf, val )
  float *f, val;
  int nf;
{
    /* returns the offset to the nearest value
     */
    int i, j=0;
    int low=0, mid, high=nf-1;

    while(low <= high) {
	mid = (low+high)/2;
	j++;
	if(val < *(f+mid))
	      high = mid-1;
	else
	      low = mid +1;
    }

    if(fabs((double)(*(f+mid)-val)) < fabs((double)(*(f+high)-val)))
	  return(mid);
    else
	  return(low);
}
/* c------------------------------------------------------------------------ */

int dgi_buf_rewind(dgi)
    DGI_PTR dgi;
{
    int mark;

    dgi->ray_num = 0;
    dgi->buf_bytes_left = dgi->file_byte_count = 0;
    if(!dgi->in_buf) {
	dgi->in_buf = (char *)malloc(MAX_READ);
	if(!dgi->in_buf) {
	    printf("Unable to malloc dgi->in_buf for: %s %d\n"
		   , dgi->radar_name, dgi->radar_num);
	    exit(1);
	}
	memset(dgi->in_buf, 0, MAX_READ);
    }
    memset(dgi->in_buf, 0, 8);
    dgi->in_next_block = dgi->in_buf;

    if(mark = lseek(dgi->in_swp_fid, 0L, 0L)) {
	mark = 0;
    }

    return(1);
}
/* c------------------------------------------------------------------------ */

int HRD_recompress( src, dst, flag, n, compression )
  unsigned short *src, *dst, flag;
  int n, compression;
{
    /* implement hrd compression of 16-bit values
     * and return the number of 16-bit words of compressed data
     */
    int mark, bcount=0;
    struct dd_input_filters *dd_return_difs_ptr();


    if( n < 1 ) {
	return(0);
    }
    if(dd_return_difs_ptr()->catalog_only) {
	return(n);
    }
    if( compression == NO_COMPRESSION ) {
	memcpy((char *)dst, (char *)src, n*sizeof(short));
	return(n);
    }

    bcount = dd_compress( src, dst, flag, n );
    return(bcount);
}
/* c------------------------------------------------------------------------ */

int RDP_compress_bytes( src, dst, flag, n, compression )
  unsigned char *src, *dst, flag;
  int n, compression;
{
# define MAX_BYTE_COMPRESSION 127
    /* implement hrd compression of bytes values
     * and return the number of bytess of compressed data
     */

    /* WARNING!
     * this probably won't work for the case of the very last word
     * is a datum at the end of a flag run
     */

    unsigned char *s=dst+1, *rlcode=dst, lastval;
    int mark, i, j, kount=0, bcount=0, end_of_run;


    if( n < 1 ) {
	return(0);
    }
    if( compression == NO_COMPRESSION ) {
	memcpy(dst, src, n);
	return(n);
    }

    for(; --n;) {
	if(kount < 3) {
	    if(kount == 2) {	/* examine first 3 values of run */
		if(*src == flag) {
		    if(lastval != flag || *(src-2) != flag) {
			*rlcode = 0x82;	/* 2 data points in run */
			bcount += 2; /* count 3rd val and rlcode position */
			rlcode = s++; /* start a new run */
			*s++ = flag; /* stuff the current val */
			kount = 1;
		    }
		    else {	/* 3 consecutive flags */
			s -= 2; bcount -= 2; kount++;
		    }
		}
		else {		/*  3rd value not a flag */
		    *s++ = *src; kount++; bcount++;
		}
	    }
	    else {		/* first or second value of run */
		*s++ = *src; kount++; bcount++;
	    }
	}
	else {
	    if(kount >= MAX_BYTE_COMPRESSION) {
		end_of_run = YES;
	    }
	    else if( *src == flag && lastval == flag ) {
		kount++;		/* counting missing flags */
		end_of_run = NO;
	    }
	    else if( *src != flag && lastval != flag ) {
		kount++;		/* counting data */
		bcount++;		/* up the word count */
		*s++ = *src;
		end_of_run = NO;
	    }
	    else
		end_of_run = YES;
		  
	    if(end_of_run) {
		if( lastval != flag ) {
		    *rlcode = kount | SIGN8;
		}
		else {
		    *rlcode = kount;
		}
		rlcode = s++;	/* mark the new run-length code word */
		*s++ = *src;	/* stuff the current val */
		bcount += 2;	/* this counts the run-length code position */
		kount = 1;
	    }
	}
	lastval = *src++;
    }

    if( kount ) {
	if( lastval != flag ) {
	    *rlcode = kount | SIGN8;
	}
	else {
	    *rlcode = kount;
	}
	bcount++;		/* this counts the run-length code word */
    }
    *s++ = END_OF_COMPRESSION;
    bcount++;

    return(bcount);
}
/* c------------------------------------------------------------------------ */

short swap2( ov )		/* swap integer*2 */
  char *ov;
{
    union {
	short newval;
	char nv[2];
    }u;
    u.nv[1] = *ov++; u.nv[0] = *ov++;
    return(u.newval);
}
/* c------------------------------------------------------------------------ */

long swap4( ov )		/* swap integer*4 */
  char *ov;
{
    union {
	long newval;
	char nv[4];
    }u;
    u.nv[3] = *ov++; u.nv[2] = *ov++; u.nv[1] = *ov++; u.nv[0] = *ov++;
    return(u.newval);
}
/* c------------------------------------------------------------------------ */

long xswap4( v_lower, v_upper ) /* assemble an integer*4 variable */
  char *v_lower, *v_upper;
{
    union {
	long newval;
	char nv[4];
    }u;
    u.nv[3] = *v_lower++;
    u.nv[2] = *v_lower++;
    u.nv[1] = *v_upper++;
    u.nv[0] = *v_upper++;
    return(u.newval);
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

