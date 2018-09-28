/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

# define NEW_ALLOC_SCHEME

# include <dorade_headers.h>
# include "solo_editor_structs.h"
# include <ui.h>
# include <ui_error.h>
# include <dd_math.h>
# include <function_decl.h>
# include <dgi_func_decl.h>
# include <glib.h>

extern GString *gs_complaints;

/*
 * 
 * 
 * se_defrekl
 * se_despeckle
 * se_flag_freckles
 *  
 */

void se_do_clear_bad_flags_array();

/* c------------------------------------------------------------------------ */

int se_rescale_field(arg, cmds)		/* #rescale-field# */
  int arg;
  struct ui_command *cmds;	
{
    /* routine to remove a ring of data; e.g. a test pulse
     */
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;

    char *dst_name;
    float scale, bias, rcp_scale, x, old_bias;
    int ii, mark;
    int nc, fn, bad;
    short  *ss, *zz, *tt;


    dst_name = (cmdq++)->uc_text;
    scale = (cmdq++)->uc_v.us_v_float;
    bias = (cmdq++)->uc_v.us_v_float;

    seds = return_sed_stuff();
    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    dgi = dd_window_dgi(seds->se_frame);
    nc = dgi->dds->celv->number_cells;
    /*
     * find the field to be thresholded
     */
    if((fn = dd_find_field(dgi, dst_name)) < 0) {
	/* field not found
	 */
	uii_printf("Field to be rescaled: %s not found\n", dst_name);
	seds->punt = YES;
	return(-1);
    }
    if(dgi->new_sweep) {
      /* nab and keep the old scale and bias */
      seds->old_scale[fn] = dgi->dds->parm[fn]->parameter_scale;
      seds->old_bias[fn] = dgi->dds->parm[fn]->parameter_bias;
      dgi->dds->parm[fn]->parameter_scale = scale;
      dgi->dds->parm[fn]->parameter_bias = bias;
    }
    rcp_scale = 1./seds->old_scale[fn];
    old_bias = seds->old_bias[fn];

# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dgi->dds->qdat_ptrs[fn];
# else
    ss = (short *)((char *)dgi->dds->rdat[fn] + sizeof(struct paramdata_d));
# endif
    zz = ss +nc;
    bad = dgi->dds->parm[fn]->bad_data;
    
    /*
     * loop through the data
     */

    for(; ss < zz; ss++) {
	if(*ss == bad)
	      continue;	
	x = DD_UNSCALE((float)(*ss), rcp_scale, old_bias);
	*ss = DD_SCALE(x, scale, bias);
    }
    return(fn);
}
/* c------------------------------------------------------------------------ */

int se_ring_zap(arg, cmds)		/* #remove-ring# */
  int arg;
  struct ui_command *cmds;	
{
    /* routine to remove a ring of data; e.g. a test pulse
     */
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;

    char *dst_name;
    float rr, r1, r2=1.e22;
    int ii, mark, gg, g1, g2;
    int nc, nd, fn, bad, a_speckle;
    short  *ss, *zz, *tt;
    unsigned short *bnd;


    dst_name = (cmdq++)->uc_text;
    r1 = KM_TO_M((cmdq++)->uc_v.us_v_float); /* get it to meters */
    if(cmdq->uc_ctype == UTT_VALUE)
	  r2 = KM_TO_M((cmdq)->uc_v.us_v_float);

    seds = return_sed_stuff();
    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    dgi = dd_window_dgi(seds->se_frame);
    nc = dgi->clip_gate+1;
    bnd = seds->boundary_mask;
    /*
     * find the field to be thresholded
     */
    if((fn = dd_find_field(dgi, dst_name)) < 0) {
	/* field not found
	 */
	uii_printf("Field to be de-ringed: %s not found\n", dst_name);
	seds->punt = YES;
	return(-1);
    }
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dgi->dds->qdat_ptrs[fn];
# else
    ss = (short *)((char *)dgi->dds->rdat[fn] + sizeof(struct paramdata_d));
# endif
    zz = ss +nc;
    bad = dgi->dds->parm[fn]->bad_data;
    g1 = dd_cell_num(dgi->dds, 0, r1);
    g2 = dd_cell_num(dgi->dds, 0, r2) +1;
    ss += g1;
    bnd += g1;
    
    /*
     * loop through the data
     */

    for(; g1++ < g2; ss++, bnd++) {
	if(!(*bnd))
	      continue;	
	*ss = bad;
    }
    return(fn);
}
/* c------------------------------------------------------------------------ */

int se_fix_vortex_vels(arg, cmds) /* #fix-vortex-velocities# */
  int arg;
  struct ui_command *cmds;	
{
    /* routine to remove discountinuities (freckles) from the data
     * such as birds and radio towers by comparing a particular
     * data point to a running average that is ahead of the point of interest
     * but switches to a trailing average once enough good points
     * have been encountered
     */
    static int vConsts[16], *ctr, level_bias;
    static double rcp_half_nyqL, d_round=.5;
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    int fn, fnd, ii, jj, kk, mm, mark, nc, idiff, bad, vL, vS, levels;
    double d, wvl, freq, prfL, prfS, ratioXY;
    double av_nyqL, av_nyqS;
    char *name, *dst_name;
    char *vl="VL", *vs="VS";
    int fn_vl, fn_vs;
    float *fptr, *ipptr, X, Y, scale, bias, rcp_scale;

    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;
    struct radar_d *radd;
    struct parameter_d *parm;

    short *ss, *zz, *vl_ptr, *vs_ptr, *dst;
    short *bnd;
    /*
     * boundary mask is set to 1 if the corresponding cell satisfies
     * conditions of the boundaries
     */
    seds = return_sed_stuff();	/* solo editing struct */
    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    bnd = (short *) seds->boundary_mask;
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;
    radd = dds->radd;
    name = (cmdq++)->uc_text;
    nc = dgi->clip_gate +1;

    if((fn = dd_find_field(dgi, name)) < 0) {
	uii_printf("Vortex velocity field: %s not found\n", name);
	seds->punt = YES;
	return(-1);
    }
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[fn];
# else
    ss = (short *)((char *)dds->rdat[fn] +sizeof(struct paramdata_d));
# endif
    zz = ss +nc;

    if((fn_vl = dd_find_field(dgi, vl)) < 0) {
	uii_printf("Vortex velocity field: %s not found\n", name);
	seds->punt = YES;
	return(-1);
    }
# ifdef NEW_ALLOC_SCHEME
    vl_ptr = (short *)dds->qdat_ptrs[fn_vl];
# else
    vl_ptr = (short *)((char *)dds->rdat[fn_vl] +sizeof(struct paramdata_d));
# endif

    if((fn_vs = dd_find_field(dgi, vs)) < 0) {
	uii_printf("Vortex velocity field: %s not found\n", name);
	seds->punt = YES;
	return(-1);
    }
# ifdef NEW_ALLOC_SCHEME
    vs_ptr = (short *)dds->qdat_ptrs[fn_vs];
# else
    vs_ptr = (short *)((char *)dds->rdat[fn_vs] +sizeof(struct paramdata_d));
# endif

    bad = dds->parm[fn]->bad_data;

    if(seds->process_ray_count == 1) {
	/*
	 * set up constants
	 * first calculate the average frequency
	 */
	parm = dds->parm[fn_vs];
	scale = dds->parm[fn_vs]->parameter_scale;
	bias = dds->parm[fn_vs]->parameter_bias;
	/*
	 * this assumes the scale and the bias are the same for
	 * all three fields
	 */
	fptr = &radd->freq1;

	for(d=0, ii=0, kk=1; ii < 5; ii++, kk<<=1) {
	    if(kk & parm->xmitted_freq) {
		d += *(fptr +ii);
	    }
	}
	/* frequency is in Ghz */
	wvl = SPEED_OF_LIGHT/((d/(double)(radd->num_freq_trans)) * 1.e9);
	/*
	 * get the PRFs
	 */
	for(ii=0, kk=1; ii < 5; ii++, kk<<=1) {
	    if(kk & parm->interpulse_time) {
		break;
	    }
	}
	/* prfS is the greater prf
	 * the interpulse period is in milliseconds
	 */
	ipptr = &radd->interpulse_per1;
	prfS = 1./(*(ipptr + ii) *.001);
	parm = dds->parm[fn_vl];

	for(ii=0, kk=1; ii < 5; ii++, kk<<=1) {
	    if(kk & parm->interpulse_time) {
		break;
	    }
	}
	prfL = 1./(*(ipptr + ii) *.001);

	X = prfS/(prfS - prfL);
	Y = prfL/(prfS - prfL);
	ratioXY = prfS/prfL;
	levels = (int)(X + Y + .1);
	level_bias = levels/2;
	av_nyqS = .25 * wvl * prfS;
	av_nyqL = .25 * wvl * prfL;
	rcp_half_nyqL = 1./DD_SCALE((.5 * av_nyqL), scale, bias);
	ctr = &vConsts[level_bias];
	*ctr = 0;

	for(ii=1; ii <= level_bias; ii++) {
	    switch(ii) {
		/* these cases represent (vL - vS) normalized
		 * by half the nyquist velocity
		 */
	    case 1:		/*  */
		d = DD_SCALE(av_nyqL * (1. + ratioXY), scale, bias);
		*(ctr - ii) = d;
		*(ctr + ii) = -d;
		break;

	    case 2:
		d = DD_SCALE(2. * av_nyqL * (1. + ratioXY), scale, bias);
		*(ctr - ii) = d;
		*(ctr + ii) = -d;
		break;

	    case 3:
		d = DD_SCALE(av_nyqL * (2. + ratioXY), scale, bias);
		*(ctr - ii) = -d;
		*(ctr + ii) = d;
		break;

	    case 4:
		d = DD_SCALE(av_nyqL, scale, bias);
		*(ctr - ii) = -d;
		*(ctr + ii) = d;
		break;
	    }
	}
	/* vS corresponds to V1 in some of the documentation
	 * and vL corresponds to V2
	 */
	rcp_scale = 1./scale;
	d = -2. * av_nyqL;

	for(ii=0; ii < levels; ii++) {
	    uii_printf("dual prt const: %6.2f for (vs-vl) = %8.4f\n"
		      , DD_UNSCALE(vConsts[ii], rcp_scale, bias)
		      , d);
	    d += .5 * av_nyqL;
	}
    }


    for(; ss < zz; ss++,bnd++,vl_ptr++,vs_ptr++) {
	if(!bnd)
	      continue;
	vS = *vs_ptr;
	vL = *vl_ptr;
	kk = d = ((double)vS - (double)vL) * rcp_half_nyqL + level_bias
	      + d_round;
	*ss = ((vS + vL) >> 1) + vConsts[kk];
    }

    return(1);
}
/* c------------------------------------------------------------------------ */

int se_defrekl(arg, cmds)	/* #do-defreckling# */
  int arg;
  struct ui_command *cmds;	
{
    /* routine to remove discountinuities (freckles) from the data
     * such as birds and radio towers by comparing a particular
     * data point to a running average that is ahead of the point of interest
     * but switches to a trailing average once enough good points
     * have been encountered
     */
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    int fn, ii, jj, kk, mm, nn, mark, navg;
    int nc, ndx_ss, ndx_q0, q1_ndx, bad, out_of_bounds, scaled_thr;
    double rcp_ngts;
    char *name;

    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;
    static struct running_avg_que *raq0, *raq1;
    struct running_avg_que *se_return_ravgs();
    struct que_val *qv0, *qv1;

    short *ss, xx, ref0, ref1;
    short *bnd;
    /* boundary mask is set to 1 if the corresponding cell satisfies
     * conditions of the boundaries
     */

    seds = return_sed_stuff();	/* solo editing struct */
    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    bnd = (short *) seds->boundary_mask;
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;
    name = (cmdq++)->uc_text;
    mm = strlen(name);

    if((fn = dd_find_field(dgi, name)) < 0) {
	uii_printf("Field to be defreckled: %s not found\n", name);
	seds->punt = YES;
	return(-1);
    }
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[fn];
# else
    ss = (short *)((char *)dds->rdat[fn] +sizeof(struct paramdata_d));
# endif
    scaled_thr = DD_SCALE(seds->freckle_threshold
		   , dds->parm[fn]->parameter_scale
		   , dds->parm[fn]->parameter_bias);
    bad = dds->parm[fn]->bad_data;
    navg = seds->freckle_avg_count;
    rcp_ngts = 1./navg;

    if(seds->process_ray_count == 1) {
	/*
	 * set up for two running average ques
	 */
	raq0 = se_return_ravgs(navg);
	raq1 = se_return_ravgs(navg);
    }
    nc = dgi->clip_gate +1;
    nn = navg +1;

    for(ndx_ss=0; ndx_ss < nc;) { /* move the cell index to the first
				   * good gate inside the next boundary */
	for(; ndx_ss < nc &&
	    (!(*(bnd+ndx_ss)) || *(ss+ndx_ss) == bad); ndx_ss++);
	/*
	 * see if we can set up a leading queue
	 */
	for(mm=0,jj=ndx_ss; mm < nn && jj < nc && *(bnd+jj); jj++) {
	    if(*(ss+jj) != bad) mm++;
	}
	if(mm < nn) {
	    ndx_ss = jj;
	    continue;		/* can't set up queue */
	}
	out_of_bounds = NO;
	/*
	 * initialize the leading average queue
	 */
	qv0 = raq0->at;
	qv1 = raq1->at;
	raq0->sum = raq1->sum = 0;

	for(ndx_q0=ndx_ss,mm=0;; ndx_q0++) {
	    if((xx = *(ss+ndx_q0)) != bad) {
		if(!mm++)
		      continue;	/* don't use the first good gate in the avg
				 */
		/* put this val in the first queue
		 */
		raq0->sum += xx;
		qv0->val = xx;
		qv0 = qv0->next;
		if(mm >= navg+1) {
		    break;
		}
	    }
	}
	ref0 = raq0->sum * rcp_ngts;
	/*
	 * now loop through the data until we have encountered
	 * navg good gates for the trailing average
	 */
	for(mm=0; ndx_q0 < nc; ndx_ss++) {
	    if((xx = *(ss+ndx_ss)) == bad)
		  continue;
	    if(abs((int)(xx -ref0)) > scaled_thr) {
		*(ss+ndx_ss) = bad;
	    }
	    else {		/* add this point to the trailing average */
		raq1->sum += xx;
		qv1->val = xx;
		qv1 = qv1->next;
		if(++mm >= navg) {
		    break;
		}
	    }
	    /* find the next good point for the leading average
	     */
	    for(ndx_q0++; ndx_q0 < nc; ndx_q0++) {
		if(!(*(bnd+ndx_q0))) { /* we've gone beyond the boundary */
		    out_of_bounds = YES;
		    break;
		}
		if((xx = *(ss+ndx_q0)) != bad) {
		    raq0->sum -= qv0->val;
		    raq0->sum += xx;
		    qv0->val = xx;
		    qv0 = qv0->next;
		    ref0 = raq0->sum * rcp_ngts;
		    break;
		}
	    }
	    if(out_of_bounds || ndx_q0 >= nc)
		  break;
	}
	if(out_of_bounds || ndx_q0 >= nc) {
	    ndx_ss = ndx_q0;
	    continue;
	}
	ref1 = raq1->sum * rcp_ngts;

	/*
	 * else shift to a trailing average
	 */

	for(ndx_ss++; ndx_ss < nc; ndx_ss++) {
	    if(!(*(bnd+ndx_ss)))
		break;		/* we've gone beyond the boundary
				 */
	    if((xx = *(ss+ndx_ss)) == bad)
		  continue;

	    if(abs((int)(xx -ref1)) > scaled_thr) {
		*(ss+ndx_ss) = bad;
	    }
	    else {		/* add this point to the trailing average */
		raq1->sum -= qv1->val;
		raq1->sum += xx;
		qv1->val = xx;
		qv1 = qv1->next;
		ref1 = raq1->sum * rcp_ngts;
	    }
	}
    }
    return(1);
}
/* c------------------------------------------------------------------------ */

int se_flag_freckles(arg, cmds)	/* #flag-freckles# */
  int arg;
  struct ui_command *cmds;	
{
    /* routine to remove discountinuities (freckles) from the data
     * such as birds and radio towers by comparing a particular
     * data point to a running average that is ahead of the point of interest
     * but switches to a trailing average once enough good points
     * have been encountered
     */
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    int fn, ii, jj, kk, mm, nn, mark, navg;
    int nc, ndx_ss, ndx_q0, q1_ndx, bad, out_of_bounds, scaled_thr;
    double rcp_ngts;
    char *name;

    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;
    static struct running_avg_que *raq0, *raq1;
    struct running_avg_que *se_return_ravgs();
    struct que_val *qv0, *qv1;

    short *ss, xx, ref0, ref1;
    unsigned short *bnd, *flag;
    /* boundary mask is set to 1 if the corresponding cell satisfies
     * conditions of the boundaries
     */

    if(strncmp(cmds->uc_text, "flag-glitches", 7) == 0) {
       ii = se_flag_glitches(arg, cmds);
       return ii;
    }

    seds = return_sed_stuff();	/* solo editing struct */
    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    bnd = seds->boundary_mask;
    seds->bad_flag_mask = flag = seds->bad_flag_mask_array;
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;
    name = (cmdq++)->uc_text;
    mm = strlen(name);

    if((fn = dd_find_field(dgi, name)) < 0) {
	uii_printf("Field to be unfolded: %s not found\n", name);
	seds->punt = YES;
	return(-1);
    }
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[fn];
# else
    ss = (short *)((char *)dds->rdat[fn] +sizeof(struct paramdata_d));
# endif
    scaled_thr = DD_SCALE(seds->freckle_threshold
		   , dds->parm[fn]->parameter_scale
		   , dds->parm[fn]->parameter_bias);
    bad = dds->parm[fn]->bad_data;
    navg = seds->freckle_avg_count;
    rcp_ngts = 1./navg;

    if(seds->process_ray_count == 1) {
	/*
	 * set up for two running average queues
	 */
	raq0 = se_return_ravgs(navg);
	raq1 = se_return_ravgs(navg);
    }
    nc = dgi->clip_gate +1;
    nn = navg +1;

    for(ndx_ss=0; ndx_ss < nc;) { /* move the cell index to the first
				   * good gate inside the next boundary */
	for(; ndx_ss < nc && (!(*(bnd+ndx_ss)) || *(ss+ndx_ss) == bad);
	    ndx_ss++);
	/*
	 * see if we can set up a leading queue
	 */
	for(mm=0,jj=ndx_ss; mm < nn && jj < nc && *(bnd+jj); jj++) {
	    if(*(ss+jj) != bad) mm++;
	}
	if(mm < nn) {
	    ndx_ss = jj;
	    continue;		/* can't set up queue */
	}
	out_of_bounds = NO;
	/*
	 * initialize the leading average queue
	 */
	qv0 = raq0->at;
	raq0->sum = raq1->sum = 0;
	for(ndx_q0=ndx_ss,mm=0;; ndx_q0++) {
	    if((xx = *(ss+ndx_q0)) != bad) {
		if(!mm++)
		      continue;	/* don't use the first good gate in the avg
				 */
		/* put this val in the first queue
		 */
		raq0->sum += xx;
		qv0->val = xx;
		qv0 = qv0->next;
		if(mm >= navg+1) {
		    break;
		}
	    }
	}
	ref0 = raq0->sum * rcp_ngts;
	qv1 = raq1->at;
	/*
	 * now loop through the data until we have encountered
	 * navg good gates for the trailing average
	 */
	for(mm=0; ndx_q0 < nc; ndx_ss++) {
	    if((xx = *(ss+ndx_ss)) == bad)
		  continue;
	    if(abs((int)(xx -ref0)) > scaled_thr) {
		*(flag+ndx_ss) = 1; /* flag this gate */
	    }
	    else {		/* add this point to the trailing average */
		raq1->sum += xx;
		qv1->val = xx;
		qv1 = qv1->next;
		if(++mm >= navg) {
		    break;
		}
	    }
	    /* find the next good point for the leading average
	     */
	    for(ndx_q0++; ndx_q0 < nc; ndx_q0++) {
		if(!(*(bnd+ndx_q0))) { /* we've gone beyond the boundary */
		    out_of_bounds = YES;
		    break;
		}
		if((xx = *(ss+ndx_q0)) != bad) {
		    raq0->sum -= qv0->val;
		    raq0->sum += xx;
		    qv0->val = xx;
		    qv0 = qv0->next;
		    ref0 = raq0->sum * rcp_ngts;
		    break;
		}
	    }
	    if(out_of_bounds || ndx_q0 >= nc)
		  break;
	}
	if(out_of_bounds || ndx_q0 >= nc) {
	    ndx_ss = ndx_q0;
	    continue;
	}
	ref1 = raq1->sum * rcp_ngts;

	/*
	 * else shift to a trailing average
	 */

	for(ndx_ss++; ndx_ss < nc; ndx_ss++) {
	    if(!(*(bnd+ndx_ss)))
		break;		/* we've gone beyond the boundary
				 */
	    if((xx = *(ss+ndx_ss)) == bad)
		  continue;

	    if(abs((int)(xx -ref1)) > scaled_thr) {
		*(flag+ndx_ss) = 1; /* flag this gate */
	    }
	    else {		/* add this point to the trailing average */
		raq1->sum -= qv1->val;
		raq1->sum += xx;
		qv1->val = xx;
		qv1 = qv1->next;
		ref1 = raq1->sum * rcp_ngts;
	    }
	}
    }
    return(1);
}
/* c------------------------------------------------------------------------ */

int se_flag_glitches(arg, cmds)	/* #flag-glitches# */
  int arg;
  struct ui_command *cmds;	
{
    /* routine to remove discountinuities (freckles) from the data
     * such as birds and radio towers by comparing a particular
     * data point to a running average that is ahead of the point of interest
     * but switches to a trailing average once enough good points
     * have been encountered
     */
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    int fn, ii, jj, kk, mm, nn, mark, navg;
    int nc, ndx_ss, bad, scaled_thr;
    int ndx_qend, ndx_qctr, min_bins, good_bins, half, sum, ival;
    double rcp_ngts, davg, diff;
    char *name;
    static int que_size = 0;
    static int *que, *qctr;

    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;

    short *ss;
    unsigned short *bnd, *flag;
    /* boundary mask is set to 1 if the corresponding cell satisfies
     * conditions of the boundaries
     */

    seds = return_sed_stuff();	/* solo editing struct */
    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    bnd = seds->boundary_mask;
    seds->bad_flag_mask = flag = seds->bad_flag_mask_array;
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;
    name = (cmdq++)->uc_text;
    mm = strlen(name);

    if((fn = dd_find_field(dgi, name)) < 0) {
	uii_printf("Field to be deglitched: %s not found\n", name);
	seds->punt = YES;
	return(-1);
    }
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[fn];
# else
    ss = (short *)((char *)dds->rdat[fn] +sizeof(struct paramdata_d));
# endif
    scaled_thr = DD_SCALE(seds->deglitch_threshold
		   , dds->parm[fn]->parameter_scale
		   , dds->parm[fn]->parameter_bias);
    bad = dds->parm[fn]->bad_data;
    if( seds->deglitch_radius < 1 )
      { seds->deglitch_radius = 3; }
    navg = seds->deglitch_radius *2 +1;
    half = navg/2;

    if( seds->deglitch_min_bins > 0 ) {
      if( seds->deglitch_min_bins > navg )
	{ seds->deglitch_min_bins = navg; }
      min_bins = seds->deglitch_min_bins;
    }
    else
      { seds->deglitch_min_bins = min_bins = navg; }

    if(seds->process_ray_count == 1) {
	/*
	 * set up 
	 */
       if( navg > que_size ) {
	  if( que_size )
	    { free( que ); }
	  que = (int *)malloc( navg * sizeof( int ));
	  que_size = navg;
       }
    }
    nc = dgi->clip_gate +1;

    for( ndx_ss = 0; ndx_ss < nc; ) {
       /*
	* move the cell index to the first
	* gate inside the next boundary
	*/
	for(; ndx_ss < nc && !(*(bnd + ndx_ss)); ndx_ss++ );
	/*
	 * set up the queue
	 */
	ndx_qend = 0;
	good_bins = 0;
	sum = 0;

	/* and start looking for the good gates count to equal or
	 * exceed the min_bins and the center bin not be bad
	 */

	for(mm = 0; ndx_ss < nc && *(bnd+ndx_ss); ndx_ss++ ) {
	   if( ++mm > navg ) {	/* after the que is full */
	      ival = *(que + ndx_qend);
	      if( ival != bad )
		{ good_bins--; sum -= ival; }
	   }
	   ival = *(ss + ndx_ss); /* raw data value */
	   *(que + ndx_qend) = ival;
	   if( ival != bad )
	     { sum += ival; good_bins++; }
	   else
	     { mark = 0; }

	   ndx_qctr = (ndx_qend - half + navg ) % que_size;
	   qctr = que + ndx_qctr;

	   if( good_bins >= min_bins && *qctr != bad ) {
	      /*
	       * do a test
	       */
	      davg = (double)(sum - *qctr)/(double)(good_bins -1);
	      if(( diff = FABS( davg - *qctr)) > scaled_thr ) {
		 sum -= *qctr;
		 good_bins--;
		 *qctr = bad;
		 ii = ndx_ss - half;
		 *(flag + ndx_ss - half) = 1; /* flag this gate */
	      }
	   }
	   ndx_qend = ++ndx_qend % que_size;
	}
    }
    return(1);
}
/* c------------------------------------------------------------------------ */

int se_despeckle(arg, cmds)		/* #despeckle# */
  int arg;
  struct ui_command *cmds;	
{
    int below;
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;
    char *dst_name;

    int nc, nd, fn, bad, a_speckle;
    int ii, nn, mark;
    short  *ss, *zz, *tt;
    unsigned short *bnd;


    dst_name = (cmdq++)->uc_text;
    nd = strlen(dst_name);

    seds = return_sed_stuff();
    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    a_speckle = seds->a_speckle;
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;
    nc = dgi->clip_gate+1;
    bnd = seds->boundary_mask;
    /*
     * find the field to be thresholded
     */
    if((fn = dd_find_field(dgi, dst_name)) < 0) {
	/* field not found
	 */
	uii_printf("Field to be thresholded: %s not found\n", dst_name);
	seds->punt = YES;
	return(-1);
    }
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[fn];
# else
    ss = (short *)((char *)dds->rdat[fn] + sizeof(struct paramdata_d));
# endif
    zz = ss +nc;
    bad = dds->parm[fn]->bad_data;

    /*
     * loop through the data
     */

    for(; ss < zz; ss++,bnd++) {
	/* move to first good gate inside a boundary
	 */
	for(; ss < zz && (*ss == bad || !(*bnd)); ss++,bnd++);
	if(ss >= zz) break;
	/* now move forward to the next bad flag
	 */
	for(tt=ss,nn=0; ss < zz && *bnd && *ss != bad; nn++,ss++,bnd++);
	if(!(*bnd) || nn > a_speckle)
	      continue;		/* not a speckle or outside boundary */
	for(;tt < ss;)
	      *tt++ = bad;	/* zap speckle */
    }
    return(fn);
}  
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */



