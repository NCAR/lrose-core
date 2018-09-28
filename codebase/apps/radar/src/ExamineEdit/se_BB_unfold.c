/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

# define NEW_ALLOC_SCHEME

# include <dorade_headers.h>
# include <solo_editor_structs.h>
# include <dd_math.h>
# include <ui.h>
# include <ui_error.h>
# include <stdio.h>
# include <function_decl.h>
# include <dgi_func_decl.h>
# include <glib.h>

extern GString *gs_complaints;

# define         BB_USE_FGG 0
# define     BB_USE_AC_WIND 1
# define  BB_USE_LOCAL_WIND 2


/* c------------------------------------------------------------------------ */

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
    /* assume this routine is not called unless the antenna is
     * in a position to see the ground
     */
    gs = dds->celvc->dist_cells[1] - dds->celvc->dist_cells[0];
    ss = (short *)dds->qdat_ptrs[pn];
    nn = dgi->dds->celvc->number_cells;
    scaled30 = (int)(dgi->dds->parm[pn]->parameter_scale * 30.);
    smin_grad = min_grad *dgi->dds->parm[pn]->parameter_scale * gs * 2.;
    rot_angle = dds->asib->rotation_angle + dds->asib->roll +
      dds->cfac->rot_angle_corr;

    /* scaled change in dBz over two gates */
    /* min_grad = .08 => ~20 dBz over two gates. */
       
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
				/*
    printf ("%8.2f %8.4f %4d %4d %4d %.1f\n"
	    , rot_angle, elev, ng_grad, mark_grad, mark_max
	    , smin_grad *.01);
				 */
    *zmax_cell = mark_max;
    return (mark_grad);
}

/* c------------------------------------------------------------------------ */

int se_ac_surface_tweak(arg, cmds)
  int arg;
  struct ui_command *cmds;	
{
    /* 
     * #remove-surface#
     * #remove-only-surface#
     * #remove-only-second-trip-surface#
     */
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    int ii, nc, nn, mark, navg, first_cell=YES, pn, sn;
    int g1, g2, gx, gate_shift, surface_only = NO, only_2_trip = NO;
    int no_footprint = NO, zmax_cell, alt_gecho_flag = NO, alt_g1;
    char *name, *aa;
    short *ss, *tt, *zz, *bnd, v0, v4, vx, bad;
    double rot_ang, earthr, deg_elev, bmwidth, elev_limit = -.0001;
    double range_val, min_range, max_range, alt, range1;
    double ground_intersect, footprint, surf_shift, fudge=1.;
    double d, elev, tan_elev, half_vbw, min_grad = .08;	/* dBz/meter */
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;
    struct platform_i *asib;
    struct radar_angles *ra;
    struct solo_edit_stuff *seds, *return_sed_stuff();


    seds = return_sed_stuff();	/* solo editing struct */

    if(seds->finish_up) {
	return(1);
    }
    if(seds->process_ray_count == 1) {
	/*
	 * 
	 */
	mark = 0;
    }
    seds->modified = YES;
    name = (cmdq++)->uc_text;
    sn = strlen(name);

    surface_only = strstr( cmds->uc_text
			  , "ly-sur") != NULL; /* only-surface */
    only_2_trip = strstr( cmds->uc_text
			 , "ly-sec") != NULL; /* only-second-trip-surface */

    dgi = dd_window_dgi(seds->se_frame);
    bnd = (short *) seds->boundary_mask;
    dds = dgi->dds;
    asib = dds->asib;
    /*
     * we probably need to make a calculation to determine if
     * the ground echo goes above the aircraft i.e. is the distance
     * (arc length) defined by a ray of length max_range rotated
     * through half the beam width greater than or equal to the
     * altitude of the aircraft?
     */
    bmwidth = RADIANS(seds->optimal_beamwidth ? seds->optimal_beamwidth :
	  dgi->dds->radd->vert_beam_width);
    half_vbw = .5 * bmwidth;

    alt = (asib->altitude_agl)*1000.;
    max_range = dds->celvc->dist_cells[dgi->clip_gate];
    elev = dds->ra->elevation;

    if (surface_only && (aa = getenv ("ALTERNATE_GECHO"))) {
       if( elev > -.002)	/* -.10 degrees */
	 { return(1); }
       alt_gecho_flag = YES;
       d = atof (aa);
       if (d > 0)
	 { min_grad = d; }	/* dbz per meter */
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
	/*
	 */
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
/* c------------------------------------------------------------------------ */

int se_BB_ac_unfold(arg, cmds)	/* #BB-unfolding# */
  int arg;
  struct ui_command *cmds;	
{
    /* 
     * 
     */
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    int ii, nc, nn, mark, navg, first_cell=YES, pn, sn;
    int scaled_nyqv, scaled_nyqi, fold_count;
    char *aa, *name;
    short *ss, *tt, *zz, *bnd, v0, v4, vx, bad;
    static short last_good_v0;
    float nyqv, scale, bias, rcp_qsize, rcp_nyqi;
    double u, v, w, insitu_wind, folds;
    double dd_azimuth_angle(), dd_elevation_angle(), dazm, dele;
    static struct running_avg_que *raq0, *raq1;
    struct running_avg_que *se_return_ravgs();
    struct que_val *qv0, *qv1;
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;
    struct solo_edit_stuff *seds, *return_sed_stuff();


    seds = return_sed_stuff();	/* solo editing struct */

    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    name = (cmdq++)->uc_text;
    sn = strlen(name);
    bnd = (short *) seds->boundary_mask;
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;

# ifdef obsolete
    for(pn=0; pn < dgi->num_parms; pn++) {
	if(strncmp(name, dds->parm[pn]->parameter_name, sn) == 0)
	      break;
    }
    if(pn == dgi->num_parms) {
# else
    if((pn = dd_find_field(dgi, name)) < 0) {
# endif
	uii_printf("Source parameter %s not found for copy\n", name);
	seds->punt = YES;
	return(-1);
    }
    nc = dgi->clip_gate +1;
# ifdef NEW_ALLOC_SCHEME
    ss = tt = (short *)dds->qdat_ptrs[pn];
# else
    ss = tt = (short *)((char *)dds->rdat[pn] +sizeof(struct paramdata_d));
# endif
    zz = ss +nc;
    bad = dds->parm[pn]->bad_data;
    scale = dds->parm[pn]->parameter_scale;
    bias = dds->parm[pn]->parameter_bias;
    nyqv = seds->nyquist_velocity ? seds->nyquist_velocity
	  : dds->radd->eff_unamb_vel;
    scaled_nyqv = DD_SCALE(nyqv, scale, bias);
    scaled_nyqi = 2*scaled_nyqv;
    rcp_nyqi = 1./((float)scaled_nyqi);

    if(seds->process_ray_count == 1) {
	/*
	 * set up for two running average ques
	 */
	raq0 = se_return_ravgs(seds->BB_avg_count);
	aa = seds->BB_init_on == BB_USE_FGG ? "the first good gate"
	      : "the wind";
	printf("Nyq. vel: %.1f; Initializing on %s; Averaging %d cells\n"
		  , nyqv, aa, seds->BB_avg_count);
    }
    rcp_qsize = raq0->rcp_num_vals;

    if(seds->BB_init_on == BB_USE_FGG) { /* initialize of the first good gate
					  * in the sweep */
	if(seds->sweep_ray_count == 1)
	      v0 = last_good_v0 = bad;
	else
	      v0 = last_good_v0;

	if(v0 == bad) {		/* find first good gate */
	    for(; *tt == bad && tt < zz; tt++);
	    if(tt == zz) return(1);
	    v0 = *tt;
	}
    }
    else {
	if(seds->BB_init_on == BB_USE_AC_WIND) {
	    u = dds->asib->ew_horiz_wind;
	    v = dds->asib->ns_horiz_wind;
	    w = dds->asib->vert_wind != -999 ? dds->asib->vert_wind : 0;
	}
	else {			/* local wind */
	    u = seds->ew_wind;
	    v = seds->ns_wind;
	    w = seds->ud_wind;
	}
	dazm = RADIANS(dd_azimuth_angle(dgi));
	dele = RADIANS(dd_elevation_angle(dgi));
	insitu_wind = cos(dele) * (u*sin(dazm) + v*cos(dazm)) + w*sin(dele);
	v0 = DD_SCALE(insitu_wind, scale, bias);
    }
    raq0->sum = 0;
    qv0 = raq0->at;
    /* initialize the running average queue
     */
    for(ii=0; ii < raq0->num_vals; ii++) {
	raq0->sum += v0;
	qv0->val = v0;
	qv0 = qv0->next;
    }

    /*
     * loop through the data
     */

    for(; ss < zz;) {		/* find the next good gate
				 */
	for(; ss < zz && (!(*bnd) || *ss == bad); ss++,bnd++);
	if(ss == zz) break;
	vx = *ss;
	v4 = raq0->sum * rcp_qsize;
	folds = (v4 -vx) * rcp_nyqi;
	fold_count = folds = folds < 0 ? folds -0.5 : folds +0.5;

	if(fold_count) {
	    if(fold_count > 0) {
		if((nn=fold_count -seds->BB_max_pos_folds) > 0)
		      fold_count -= nn;
	    }
	    else if((nn=fold_count +seds->BB_max_neg_folds) < 0) 
		  fold_count -= nn;
	}
	vx += fold_count*scaled_nyqi;

	raq0->sum -= qv0->val;
	raq0->sum += vx;
	qv0->val = vx;
	qv0 = qv0->next;

	*ss++ = vx;
	bnd++;

	if(first_cell) {
	    first_cell = NO;
	    last_good_v0 = vx;
	}
    }
    return(1);
}
/* c------------------------------------------------------------------------ */

int se_BB_setup(arg, cmds)
  int arg;
  struct ui_command *cmds;	
{
    /*
     * #BB-gates-averaged#
     * #BB-max-pos-folds#
     * #BB-max-neg-folds#
     * #BB-use-first-good-gate#
     * #BB-use-ac-wind#
     * #BB-use-local-wind#
     * #nyquist-velocity#
     * ##
     * ##
     * ##
     */

    /* this routine will process all commands associated with
     * the setup-input procedure. It also updates the sfic texts
     */
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
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
/* c------------------------------------------------------------------------ */

int se_remove_ac_motion(arg, cmds)	/* #remove-aircraft-motion# */
  int arg;
  struct ui_command *cmds;	
{
    /* remove the aircraft motion from velocities
     */
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    int ii, nc, nn, mark, pn, sn;
    int scaled_nyqv, scaled_nyqi, fold_count, scaled_ac_vel, adjust;
    char *name;
    short *ss, *tt, *zz, *bnd, vx, bad;
    float nyqv, scale, bias, rcp_nyqi, ac_vel;
    double d, dd_ac_vel();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;
    struct solo_edit_stuff *seds, *return_sed_stuff();


    seds = return_sed_stuff();	/* solo editing struct */

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
/* c------------------------------------------------------------------------ */

int se_remove_storm_motion(arg, cmds)	/* #remove-storm-motion# */
  int arg;
  struct ui_command *cmds;	
{
    /* remove the aircraft motion from velocities
     */
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
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


    seds = return_sed_stuff();	/* solo editing struct */

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
    wind = (cmdq++)->uc_v.us_v_float; /* angle */
    speed = (cmdq++)->uc_v.us_v_float;
    wind = FMOD360 (wind +180.); /* change to wind vector */
    az = dd_rotation_angle (dgi);
    cosEl = cos (RADIANS (dd_elevation_angle (dgi)));
    if (fabs (cosEl) < .0001)
      { return (1); }
    rcp_cosEl = 1./cosEl;
    theta = d_angdiff (az, wind); /* clockwise from az to wind */
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
/* c------------------------------------------------------------------------ */
