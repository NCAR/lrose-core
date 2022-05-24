#include <cmath>
#include <deque>
#include <queue>
#include <numeric>

#include <Solo/GeneralDefinitions.hh>
#include <Solo/SoloFunctions.hh>


bool is_data_bad(float data, float bad_data_value) {
  return abs(data - bad_data_value) < 0.0001;
  //  return abs(newData[ssIdx] - bad) < 0.0001;
}

//# define NEW_ALLOC_SCHEME

//# define         BB_USE_FGG 0
//# define     BB_USE_AC_WIND 1
//# define  BB_USE_LOCAL_WIND 2


/* c------------------------------------------------------------------------ 

int alt_gecho (dgi, min_grad, zmax_cell)
  DGI_PTR dgi;
  double min_grad;
  int *zmax_cell;
{
    DDS_PTR dds=dgi->dds;
    int pn, ii, jj, kk, mm, nn, mark, zmax = 0, ival;
    int mark_max = -1, mark_grad = -1, ng_grad;
    short *ss, *zz, scaled30;
    double gs, slope, smin_grad, grad, elev = dds->ra->elevation;
    double rot_angle;

    if((pn = dd_find_field(dgi, "DBZ")) < 0) {	
	uii_printf("Source parameter %s not found for surface removal\n"
		  , "DBZ");
	return(-1);
    }
    // assume this routine is not called unless the antenna is
    // in a position to see the ground
    //
    gs = dds->celvc->dist_cells[1] - dds->celvc->dist_cells[0];
    ss = (short *)dds->qdat_ptrs[pn];
    nn = dgi->dds->celvc->number_cells;
    scaled30 = (int)(dgi->dds->parm[pn]->parameter_scale * 30.);
    smin_grad = min_grad *dgi->dds->parm[pn]->parameter_scale * gs * 2.;
    rot_angle = dds->asib->rotation_angle + dds->asib->roll +
      dds->cfac->rot_angle_corr;

    // scaled change in dBz over two gates 
    // min_grad = .08 => ~20 dBz over two gates. 
       
    ng_grad = (int)(1.075 * (1./fabs (sin (elev)) +1.));
    if (ng_grad < 3 || ng_grad > 60)
      { mark = 0; }

    if (ng_grad < 2)
      { ng_grad = 2; }
    else if (ng_grad > 77)
      { ng_grad = 77; }

    for (ii=ng_grad+1; ii < nn; ii++) {
       ival = ss[ii];
       if (ival < scaled30)
	 { continue; }
       
       if (mark_grad < 0) {
	  for (grad = 0, jj = 0; jj < ng_grad; jj++) {
	     grad += ss[ii-jj] - ss[ii-jj-1];
	     if (grad > smin_grad) {
		mark_grad = ii-jj-1;
		break;
	     }
	  }
       }
       if (ival > scaled30 && ival > zmax) {
	  mark_max = ii; zmax = ival;
       }
    }
				//
    printf ("%8.2f %8.4f %4d %4d %4d %.1f\n"
	    , rot_angle, elev, ng_grad, mark_grad, mark_max
	    , smin_grad *.01);
				 
    *zmax_cell = mark_max;
    return (mark_grad);
}

// c------------------------------------------------------------------------ 

int se_ac_surface_tweak(arg, cmds)
  int arg;
  struct ui_command *cmds;	
{
    // 
     * #remove-surface#
     * #remove-only-surface#
     * #remove-only-second-trip-surface#
     
    struct ui_command *cmdq=cmds+1; // point to the first argument 
    int ii, nc, nn, mark, navg, first_cell=YES, pn, sn;
    int g1, g2, gx, gate_shift, surface_only = NO, only_2_trip = NO;
    int no_footprint = NO, zmax_cell, alt_gecho_flag = NO, alt_g1;
    char *name, *aa;
    short *ss, *tt, *zz, *bnd, v0, v4, vx, bad;
    double rot_ang, earthr, deg_elev, bmwidth, elev_limit = -.0001;
    double range_val, min_range, max_range, alt, range1;
    double ground_intersect, footprint, surf_shift, fudge=1.;
    double d, elev, tan_elev, half_vbw, min_grad = .08;	// dBz/meter 
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;
    struct platform_i *asib;
    struct radar_angles *ra;
    struct solo_edit_stuff *seds, *return_sed_stuff();


    seds = return_sed_stuff();	// solo editing struct 

    if(seds->finish_up) {
	return(1);
    }
    if(seds->process_ray_count == 1) {
	//
	 * 
	 
	mark = 0;
    }
    seds->modified = YES;
    name = (cmdq++)->uc_text;
    sn = strlen(name);

    surface_only = strstr( cmds->uc_text
			  , "ly-sur") != NULL; // only-surface 
    only_2_trip = strstr( cmds->uc_text
			 , "ly-sec") != NULL; // only-second-trip-surface 

    dgi = dd_window_dgi(seds->se_frame);
    bnd = (short *) seds->boundary_mask;
    dds = dgi->dds;
    asib = dds->asib;
    //
     * we probably need to make a calculation to determine if
     * the ground echo goes above the aircraft i.e. is the distance
     * (arc length) defined by a ray of length max_range rotated
     * through half the beam width greater than or equal to the
     * altitude of the aircraft?
     
    bmwidth = RADIANS(seds->optimal_beamwidth ? seds->optimal_beamwidth :
	  dgi->dds->radd->vert_beam_width);
    half_vbw = .5 * bmwidth;

    alt = (asib->altitude_agl)*1000.;
    max_range = dds->celvc->dist_cells[dgi->clip_gate];
    elev = dds->ra->elevation;

    if (surface_only && (aa = getenv ("ALTERNATE_GECHO"))) {
       if( elev > -.002)	// -.10 degrees 
	 { return(1); }
       alt_gecho_flag = YES;
       d = atof (aa);
       if (d > 0)
	 { min_grad = d; }	// dbz per meter 
    }

    if( !surface_only && (d = max_range * fudge * bmwidth) >= alt) {
	d -= alt;
	elev_limit = atan2(d, (double)max_range);

	if( elev > elev_limit)
	    return(1);

	if(d >= 0 && elev > -fudge * bmwidth) {
	    only_2_trip = YES;
	    g1 = 0;
	}
    }

    if( elev > elev_limit)
	  return(1);

    if( !only_2_trip ) {
        earthr = dd_earthr(dd_latitude(dgi));
	elev -= half_vbw;
	tan_elev = tan(elev);
	range1 = ground_intersect = (-(alt)/sin(elev))*
	      (1.+alt/(2.*earthr*1000.*
		       tan_elev*tan_elev));
	if(ground_intersect > max_range || ground_intersect < 0 )
	      return(1);
	//
	 
	g1 = dd_cell_num(dgi->dds, 0, range1);
    }
    gate_shift = seds->surface_gate_shift;

    if (alt_gecho_flag) {
       ii = alt_gecho (dgi, min_grad, &zmax_cell);
       if (ii > 0)
	 { g1 = ii; gate_shift = 0; }
       else
	 { return (1); }
    }

    if((pn = dd_find_field(dgi, name)) < 0) {	
	uii_printf("Source parameter %s not found for surface removal\n"
		  , name);
	seds->punt = YES;
	return(-1);
    }
    g1 += gate_shift;
    if(g1 < 0) g1 = 0;

    ss = (short *)dds->qdat_ptrs[pn];
    zz = ss + dgi->clip_gate +1;
    ss += g1;
    bad = dds->parm[pn]->bad_data;

    for(; ss < zz;)
	  *ss++ = bad;

    return(1);
}
*/

/*
static const gchar * BB_ac_unfolding[] = {
"!  Help file for the \"BB-ac-unfolding\" command which has the form:",
" ",
"    BB-ac-unfolding in <field-name>",
" ",
"!  Replace angle brackets and argument types with appropriate arguments.",
"  ",
"!  This operation does a Bargen-Brown unfold of a velocity field",
"!  based on the difference between a given point and a running",
"!  average of a certain number of gates being greater than the",
"!  Nyquist Velocity. This command is meant for aircraft data.",
"  ",
"!  Other \"one-time\" commands that affect BB-ac-unfolding are:",
" ",
"    BB-gates-averaged is <integer> gates",
"!  The number of gates in the running average",
"    BB-max-neg-folds is <integer>",
"    BB-max-pos-folds is <integer>",
"!  Stops the number of unfolds from exceeding this value",
"    BB-use-first-good-gate",
"!  This is the default!",
"!  Causes the running average to initialize on the first good gate",
"!  encountered in the first ray in the sweep. Subsequent rays ",
"!  initialize on the first good gate in the previous ray.",
"    BB-use-ac-wind",
"!  Is for aircraft data and means to use the wind information from the",
"!  ac data present. ",
"    BB-use-local-wind",
"!  Users can define the local wind in m/s. with",
"    ew-wind <float>",
"    ns-wind <float>",
};

*/



// TODO: should the velocities be converted to ints? or keep as float?
float running_average(std::queue<float> const& v) {
  return 0.0; // 1.0 * std::accumulate(v.begin(), v.end(), 0.0) / v.size();
}
/*
// c------------------------------------------------------------------------ 

//int se_BB_ac_unfold(arg, cmds)	// #BB-unfolding# 
//  int arg;
//  struct ui_command *cmds;	
//{
// BB_init_on is an enumeration
*/

// call these to get v0, then call generic unfold

void se_BB_unfold_ac_wind(const float *data, float *newData, size_t nGates,
			  float nyquist_velocity, float dds_radd_eff_unamb_vel,
			  float azimuth_angle_degrees, float elevation_angle_degrees,
			  float ew_horiz_wind,
			  float ns_horiz_wind,
			  float vert_wind,
			  int max_pos_folds, int max_neg_folds,
			  size_t ngates_averaged,
			  float bad_data_value, size_t dgi_clip_gate, bool *bnd) {


  se_BB_unfold_local_wind(data, newData, nGates,
			  nyquist_velocity, dds_radd_eff_unamb_vel,
			  azimuth_angle_degrees, elevation_angle_degrees,
			  ew_horiz_wind,
			  ns_horiz_wind,
			  vert_wind,
			  max_pos_folds, max_neg_folds,
			  ngates_averaged, 
			  bad_data_value, dgi_clip_gate, bnd);

  // I think the code is the same as local_wind ...

  /*    
  int ii, nn, mark, navg, pn, sn;
  size_t nc;
  int scaled_nyqv, scaled_nyqi, fold_count;
  char *aa, *name;
  //    short *ss, *tt, *zz, *bnd, v0, v4, vx, bad;
  //static short last_good_v0;
  float nyqv, scale, bias, rcp_qsize, rcp_nyqi;
  double u, v, w, insitu_wind, folds;
  double dazm, dele;
  float bad;
  float v0;

  // TODO: validate arguments
  if (ngates_averaged > nGates) throw "number of gates in average exceeds the number of gates in the ray";
  if (ngates_averaged > nGates) throw "clip gate is greater than the number of gates in the ray";

  //nc = dgi_clip_gate;
  //bad = bad_data_value;

  if (nyquist_velocity)
    nyqv = nyquist_velocity;
  else
    nyqv = dds_radd_eff_unamb_vel;

  //scaled_nyqv = DD_SCALE(nyqv);
  //scaled_nyqi = 2*scaled_nyqv;
  //rcp_nyqi = 1./((float)scaled_nyqi);

    
  printf("Nyq. vel: %.1f; Initializing on the wind; Averaging %lu cells\n",
	 nyqv, ngates_averaged);

  u = ew_horiz_wind;
  v = ns_horiz_wind;
  w = vert_wind != -999 ? vert_wind : 0; // Magic number -999
  dazm = RADIANS(azimuth_angle_degrees);
  dele = RADIANS(elevation_angle_degrees);
  insitu_wind = cos(dele) * (u*sin(dazm) + v*cos(dazm)) + w*sin(dele);
  v0 = DD_SCALE(insitu_wind); // , scale, bias);

  se_BB_generic_unfold(data, newData, nGates,
			  &v0, ngates_averaged,
			  nyqv, 
			  max_pos_folds, max_neg_folds,
			  bad_data_value, dgi_clip_gate, bnd);

  */
}

//
// last_good_v0 must be managed externally; it is an in/out argument??
//
void se_BB_unfold_first_good_gate(const float *data, float *newData, size_t nGates,
				  float nyquist_velocity, float dds_radd_eff_unamb_vel,
				  //				  float azimuth_angle_degrees, float elevation_angle_degrees,
				  int max_pos_folds, int max_neg_folds,
				  size_t ngates_averaged,
				  float *last_good_v0,
				  float bad_data_value, size_t dgi_clip_gate, bool *bnd) {
    
  int ii, nn, mark, navg;
  bool first_cell=true;
  int  pn, sn;
  size_t nc;
  int scaled_nyqv, scaled_nyqi, fold_count;
  //    short *ss, *tt, *zz, *bnd, v0, v4, vx, bad;
  //static short last_good_v0;  // TODO:  how to deal with this static variable?  Must be managed externally
  float nyqv, scale, bias, rcp_qsize, rcp_nyqi;
  double u, v, w, insitu_wind, folds;
  double dazm, dele;    
  //float bad;
  float v0;

  // TODO: validate arguments
  if (ngates_averaged > nGates) throw "number of gates in average exceeds the number of gates in the ray";
  if (ngates_averaged > nGates) throw "clip gate is greater than the number of gates in the ray";


  //nc = dgi_clip_gate;
  //bad = bad_data_value;

  if (nyquist_velocity)
    nyqv = nyquist_velocity;
  else
    nyqv = dds_radd_eff_unamb_vel;


  //scaled_nyqv = DD_SCALE(nyqv); // , scale, bias);
  //scaled_nyqi = 2*scaled_nyqv;
  //rcp_nyqi = 1./((float)scaled_nyqi);

  // TODO: figure out this v0, last_good_v0 stuff; probably should move to calling function
  // initialize of the first good gate
  // in the sweep 
  /*  if(seds->sweep_ray_count == 1)
    v0 = last_good_v0 = bad;
  else
    v0 = last_good_v0;
  */
  v0 = *last_good_v0;

  size_t ttIdx, zzIdx;
  ttIdx = 0;
  zzIdx = nGates;
  if ((dgi_clip_gate > 0) && (dgi_clip_gate < nGates))
    zzIdx = dgi_clip_gate;

  if(is_data_bad(v0, bad_data_value)) {		// find first good gate 
    //for(; *tt == bad && tt < zz; tt++);
    //if(tt == zz) return(1);
    //v0 = *tt;
    //    while ((ttIdx < zzIdx) && ((data[ttIdx] - bad) < 0.0001)) { 
    while ((ttIdx < zzIdx) && (is_data_bad(data[ttIdx], bad_data_value))) { 
      ttIdx += 1;
    }
    if (ttIdx < zzIdx)
      v0 = data[ttIdx];
    else
      throw "ray contains only bad data";
  }

  printf("Nyq. vel: %.1f; Initializing on the first good gate, v0=%.1f; Averaging %lu cells\n",
	 nyqv, v0, ngates_averaged);

  se_BB_generic_unfold(data, newData, nGates,
			  &v0, ngates_averaged,
			  nyqv, 
			  max_pos_folds, max_neg_folds,
			  bad_data_value, dgi_clip_gate, bnd);

  /*
    void se_BB_generic_unfold(const float *data, float *newData, size_t nGates,
    float *v0, size_t ngates_averaged, 
    float nyquist_velocity,
    int BB_max_pos_folds, int BB_max_neg_folds,
    float bad_data_value, size_t dgi_clip_gate, bool *bnd) {
  */

  *last_good_v0 = v0;

}



void se_BB_unfold_local_wind(const float *data, float *newData, size_t nGates,
			     float nyquist_velocity, float dds_radd_eff_unamb_vel,
			     float azimuth_angle_degrees, float elevation_angle_degrees,
                             float ew_wind, float ns_wind, float ud_wind,
                             int max_pos_folds, int max_neg_folds,
			     size_t ngates_averaged,
			     float bad_data_value, size_t dgi_clip_gate, bool *bnd) {
    
  int ii, nn, mark, navg;
  //  bool first_cell=true;
  int pn, sn;
  size_t nc;
  int scaled_nyqv, scaled_nyqi; // , fold_count;
  //    short *ss, *tt, *zz, *bnd, v0, v4, vx, bad;
  //    static short last_good_v0;
  float nyqv; // , rcp_qsize, rcp_nyqi;
  double u, v, w, insitu_wind; // , folds;
  double  dazm, dele;
  float bad;
  float v0;

  // TODO: validate arguments
  if (ngates_averaged > nGates) throw "number of gates in average exceeds the number of gates in the ray";
  if (ngates_averaged > nGates) throw "clip gate is greater than the number of gates in the ray";


  //nc = dgi_clip_gate;
  //  bad = bad_data_value;

  if (nyquist_velocity)
    nyqv = nyquist_velocity;
  else
    nyqv = dds_radd_eff_unamb_vel;


  // local wind 
  u = ew_wind;
  v = ns_wind;
  // w = ud_wind;
	
  dazm = RADIANS(azimuth_angle_degrees);
  dele = RADIANS(elevation_angle_degrees);
  insitu_wind = cos(dele) * (u*sin(dazm) + v*cos(dazm)) + w*sin(dele);
  v0 = DD_SCALE(insitu_wind); // , scale, bias);

  printf("Nyq. vel: %.1f; Initializing on the wind, v0=%.1f; Averaging %lu cells\n",
	 nyqv, v0, ngates_averaged);

  se_BB_generic_unfold(data, newData, nGates, 
			  &v0, ngates_averaged,
			  nyqv, 
			  max_pos_folds, max_neg_folds,
			  bad_data_value, dgi_clip_gate, bnd);
  /*
    void se_BB_generic_unfold(const float *data, float *newData, size_t nGates,
    float *v0, size_t ngates_averaged, 
    float nyquist_velocity,
    int BB_max_pos_folds, int BB_max_neg_folds,
    float bad_data_value, size_t dgi_clip_gate, bool *bnd) {
  */
}

//
// float v0 is in/out argument  
//          in:  initial velocity 
//          out: last good v0; last good initial velocity found.

void se_BB_generic_unfold(const float *data, float *newData, size_t nGates,
			  float *v0, size_t ngates_averaged, 
			  float nyquist_velocity,
			  int BB_max_pos_folds, int BB_max_neg_folds,
			  float bad_data_value, size_t dgi_clip_gate, bool *bnd) {
    
  int ii, nn, mark, navg;
  bool first_cell=true;
  int pn, sn;
  size_t nc, ssIdx, zzIdx;
  float vx, v4;
  float sum;
  int scaled_nyqv, scaled_nyqi, fold_count;
  char *aa, *name;
  //    short *ss, *tt, *zz, *bnd, v0, v4, vx, bad;
  float last_good_v0;
  float nyqv, scale, bias, rcp_qsize, rcp_nyqi;
  double u, v, w, insitu_wind, folds;
  // TODO: need to implement these:  double dd_azimuth_angle(), dd_elevation_angle(),
    
  float bad;

  zzIdx = nGates;
  if ((dgi_clip_gate > 0) && (dgi_clip_gate < nGates))
    zzIdx = dgi_clip_gate;

  ssIdx = 0;

  bad = bad_data_value;
  //    nyqv = nyquist_velocity ? nyquist_velocity
  //  : dds_radd_eff_unamb_vel;

  nyqv = nyquist_velocity;

  scaled_nyqv = DD_SCALE(nyqv);
  scaled_nyqi = 2*scaled_nyqv;
  rcp_nyqi = 1./((float)scaled_nyqi);  // 'rcp' means reciprocal

  // initialize the running average 
  std::deque<float> raq0(ngates_averaged, *v0); 
  sum = *v0 * ngates_averaged;
  rcp_qsize = 1.0/ngates_averaged;

  // memcopy data into newData
  memcpy(newData, data, nGates*sizeof(float));

  while (ssIdx < zzIdx) {
    bool bad_data = is_data_bad(newData[ssIdx], bad);
    if (bnd[ssIdx] && !bad_data) {
      
      vx = newData[ssIdx];
      v4 = sum * rcp_qsize; // running_average(raq0);
      folds = (v4 - vx) * rcp_nyqi;
      //printf("v4 = %f, vx = %f, raw folds = %f ", v4, vx, folds);
      if (folds < 0)
	      folds = folds - 0.5;
      else
	      folds = folds + 0.5;
      fold_count = (int) folds;
      //printf("folds = %f, ==> fold_count = %d ", folds, fold_count);

      if(fold_count) {
	      if(fold_count > 0) {
          nn = fold_count - BB_max_pos_folds;
  	      if(nn > 0)
  	        fold_count -= nn;
      	} else {
      	  nn = fold_count + BB_max_neg_folds;
          if(nn < 0) 
      	    fold_count -= nn; // subtracting a negative number
      	}
      }
      //printf(" limited to %d\n", fold_count);
      vx += fold_count * scaled_nyqi;
      //printf("vx += fold_count(%d) * scaled_nyqi(%d) = %f\n", fold_count, scaled_nyqi, vx);
      // insert new velocity into running average queue
      sum -= raq0.front();
      sum += vx;
      raq0.pop_front();
      raq0.push_back(vx);
      newData[ssIdx] = vx;
      // What is the difference between last_good_v0 and
      // the running average?  last_good_v0 is actually the first
      // good velocity (maybe unfolded) in this ray.
      if(first_cell) {
      	first_cell = false;
              // return the last good v0
      	last_good_v0 = vx;
      }
    }
    ssIdx += 1;
  }
  *v0 = last_good_v0;
}

// c------------------------------------------------------------------------ 
/*
// TODO: does all of this go away and become script variables, passed 
// as arguments to BB_unfolding?
//

int se_BB_setup(arg, cmds)
  int arg;
  struct ui_command *cmds;	
{
  //
  //   * #BB-gates-averaged#
  //   * #BB-max-pos-folds#
  //   * #BB-max-neg-folds#
  //   * #BB-use-first-good-gate#
  //   * #BB-use-ac-wind#
  //   * #BB-use-local-wind#
  //   * #nyquist-velocity#
  //   * ##
  //   * ##
  //   * ##
     

    // this routine will process all commands associated with
    // the setup-input procedure. It also updates the sfic texts
     
    struct ui_command *cmdq=cmds+1; // point to the first argument 
    int ii, nn, mark;
    struct solo_edit_stuff *seds, *return_sed_stuff();

    seds = return_sed_stuff();

    if(strncmp(cmds->uc_text, "BB-gates-averaged", 11) == 0) {
	seds->BB_avg_count = cmdq->uc_v.us_v_int;
    }
    else if(strncmp(cmds->uc_text, "BB-max-pos-folds", 11) == 0) {
	seds->BB_max_pos_folds = cmdq->uc_v.us_v_int;
    }
    else if(strncmp(cmds->uc_text, "BB-max-neg-folds", 11) == 0) {
	seds->BB_max_neg_folds = cmdq->uc_v.us_v_int;
    }
    else if(strncmp(cmds->uc_text, "BB-use-first-good-gate", 9) == 0) {
	seds->BB_init_on = BB_USE_FGG;
    }
    else if(strncmp(cmds->uc_text, "BB-use-ac-wind", 9) == 0) {
	seds->BB_init_on = BB_USE_AC_WIND;
    }
    else if(strncmp(cmds->uc_text, "BB-use-local-wind", 9) == 0) {
	seds->BB_init_on = BB_USE_LOCAL_WIND;
    }
    else if(strncmp(cmds->uc_text, "nyquist-velocity", 3) == 0) {
	seds->nyquist_velocity = FABS(cmdq->uc_v.us_v_float);
    }
    return(1);
}
*/
// BB_init_on is an enum; 
// it can have values:
// BB_USE_FGG
// BB_USE_AC_WIND
// BB_USE_LOCAL_WIND
/* c------------------------------------------------------------------------ 

int se_remove_ac_motion(arg, cmds)	// #remove-aircraft-motion# 
  int arg;
  struct ui_command *cmds;	
{
    // remove the aircraft motion from velocities
     
    struct ui_command *cmdq=cmds+1; // point to the first argument 
    int ii, nc, nn, mark, pn, sn;
    int scaled_nyqv, scaled_nyqi, fold_count, scaled_ac_vel, adjust;
    char *name;
    short *ss, *tt, *zz, *bnd, vx, bad;
    float nyqv, scale, bias, rcp_nyqi, ac_vel;
    double d, dd_ac_vel();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;
    struct solo_edit_stuff *seds, *return_sed_stuff();


    seds = return_sed_stuff();	// solo editing struct 

    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    name = (cmdq++)->uc_text;
    sn = strlen(name);
    bnd = (short *) seds->boundary_mask;
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;

    if((pn = dd_find_field(dgi, name)) < 0) {	
	uii_printf("Source parameter %s not found for copy\n", name);
	seds->punt = YES;
	return(-1);
    }
    ac_vel = dd_ac_vel(dgi);
    nc = dgi->clip_gate +1;
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[pn];
# else
    ss = (short *)((char *)dds->rdat[pn] +sizeof(struct paramdata_d));
# endif
    zz = ss +nc;
    bad = dds->parm[pn]->bad_data;
    scale = dds->parm[pn]->parameter_scale;
    bias = dds->parm[pn]->parameter_bias;
# ifdef obsolete
    nyqv = dds->radd->eff_unamb_vel;
# else
    nyqv = seds->nyquist_velocity ? seds->nyquist_velocity
	  : dds->radd->eff_unamb_vel;
# endif
    scaled_nyqv = DD_SCALE(nyqv, scale, bias);
    scaled_nyqi = 2*scaled_nyqv;
    scaled_ac_vel = DD_SCALE(ac_vel, scale, bias);
    adjust = scaled_ac_vel % scaled_nyqi;

    if(abs(adjust) > scaled_nyqv) {
	adjust = adjust > 0 ? adjust-scaled_nyqi : adjust+scaled_nyqi;
    }

    for(; ss < zz; ss++,bnd++) {
	if(!(*bnd) || *ss == bad)
	      continue;
	vx = *ss +adjust;
	if(abs(vx) > scaled_nyqv) {
	    vx = vx > 0 ? vx-scaled_nyqi : vx+scaled_nyqi;
	}
	*ss = vx;
    }
    return(1);
}
// c------------------------------------------------------------------------ 

int se_remove_storm_motion(arg, cmds)	// #remove-storm-motion# 
  int arg;
  struct ui_command *cmds;	
{
    // remove the aircraft motion from velocities
     
    struct ui_command *cmdq=cmds+1; // point to the first argument 
    int ii, nc, nn, mark, pn, sn;
    int scaled_vel;
    char *name;
    short *ss, *tt, *zz, *bnd, vx, bad;
    float speed, wind, scale, bias;
    double d, az, cosEl, rcp_cosEl, theta, cosTheta, adjust, scaled_adjust;
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;
    struct solo_edit_stuff *seds, *return_sed_stuff();
    double d_angdiff();


    seds = return_sed_stuff();	// solo editing struct 

    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    name = (cmdq++)->uc_text;
    sn = strlen(name);
    bnd = (short *) seds->boundary_mask;
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;

    if((pn = dd_find_field(dgi, name)) < 0) {	
	uii_printf("Source parameter %s not found for copy\n", name);
	seds->punt = YES;
	return(-1);
    }
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[pn];
# else
    ss = (short *)((char *)dds->rdat[pn] +sizeof(struct paramdata_d));
# endif
    wind = (cmdq++)->uc_v.us_v_float; // angle 
    speed = (cmdq++)->uc_v.us_v_float;
    wind = FMOD360 (wind +180.); // change to wind vector 
    az = dd_rotation_angle (dgi);
    cosEl = cos (RADIANS (dd_elevation_angle (dgi)));
    if (fabs (cosEl) < .0001)
      { return (1); }
    rcp_cosEl = 1./cosEl;
    theta = d_angdiff (az, wind); // clockwise from az to wind 
    adjust = cos (RADIANS (theta)) * speed;

    scale = dds->parm[pn]->parameter_scale;
    bias = dds->parm[pn]->parameter_bias;
    scaled_adjust = DD_SCALE(adjust, scale, bias);
    bad = dds->parm[pn]->bad_data;
    nc = dgi->clip_gate +1;
    zz = ss +nc;

    for(; ss < zz; ss++,bnd++) {
	if(!(*bnd) || *ss == bad)
	      continue;
	d = (*ss * cosEl -scaled_adjust) * rcp_cosEl;
	*ss = (short)d;
    }
    return(1);
}
// c------------------------------------------------------------------------ 
*/
