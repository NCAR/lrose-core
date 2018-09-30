/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

# define NEW_ALLOC_SCHEME

# include "dorade_headers.h"
# include "input_sweepfiles.h"
# include "solo_editor_structs.h"
# include "dd_files.h"
# include <seds.h>
# include <ui.h>
# include <ui_error.h>
# include <function_decl.h>
# include <dgi_func_decl.h>
# include <dd_math.h>
# include <glib.h>

extern GString *gs_complaints;

/*
 * se_cpy_field
 * se_dir
 * se_establish_field
 * se_funfold
 * se_hard_zap
 * se_once_only
 * se_readin_cmds
 * se_threshold_field
 *  
 *  
 *  
 */

int se_establish_field();	/* se_catch_all.c */
int dd_get_scan_mode();		/* ../translate/ddb_common.c */

/* c------------------------------------------------------------------------ */

int se_for_each_ray(arg, cmds)		/* #for-each-ray# */
  int arg;
  struct ui_command *cmds;	
{
    int mark=0;
    return(1);
}
/* c------------------------------------------------------------------------ */

int se_header_value(arg, cmds)	/* #header-value# */
  int arg;
  struct ui_command *cmds;	
{
    int below;
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;
    char *name, *s_val;
    float f_val, diff, cdiff, *fptr, *cfptr, rng;
    double d;
    int mark, ii, nn, modified=NO;

    name = (cmdq++)->uc_text;
    s_val = (cmdq)->uc_text;
    f_val = (cmdq)->uc_v.us_v_float;
    cmdq++;

    seds = return_sed_stuff();
    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;

    if(dgi->new_sweep) {
      nn = dds->celv->number_cells;
      fptr = dds->celv->dist_cells;
      cfptr = dds->celvc->dist_cells;
      
      if(strstr("range-to-first-cell", name)) {
	diff = f_val - dds->celv->dist_cells[0];

	for(; nn--; fptr++, cfptr++) {
	  *fptr += diff;
	  *cfptr = *fptr;
	}
	dd_set_uniform_cells(dgi->dds);
      }
      else if(strstr("cell-spacing", name)) {
	rng = dds->celv->dist_cells[0];
	diff = dds->celvc->dist_cells[0] - dds->celv->dist_cells[0];

	for(; nn--; fptr++, cfptr++, rng += f_val) {
	  *fptr = rng;
	  *cfptr = *fptr;
	}
	dd_set_uniform_cells(dgi->dds);
      }
      else if(strstr("nyquist-velocity", name)) {
	dds->radd->eff_unamb_vel = f_val;
      }
      else if(strstr("fixed-angle", name)) {
	dds->swib->fixed_angle = f_val;
      }
      else if(strstr("latitude", name)) {
	dds->radd->radar_latitude = f_val;
      }
      else if(strstr("longitude", name)) {
	dds->radd->radar_longitude = f_val;
      }
      else if(strstr("altitude", name)) {
	dds->radd->radar_altitude = f_val;
      }
      else if(strstr("scan-mode", name)) {
	  ii = dd_get_scan_mode( s_val );
	  dds->radd->scan_mode = ii;
      }
      else if(strstr("ipp1", name)) {
	dds->radd->interpulse_per1 = f_val;
      }
      else if(strstr("ipp2", name)) {
	dds->radd->interpulse_per2 = f_val;
      }
    }

    if(strstr("tilt-angle", name)) {
       dds->asib->tilt = f_val;
    }
    else if(strstr("rotation-angle", name)) {
       dds->asib->rotation_angle = f_val;
    }
    else if(strstr("elevation-angle", name)) {
       dds->ryib->elevation = f_val;
    }
    else if(strstr("diddle-elevation", name)) {
       d = 180. - dds->ryib->elevation;
       dds->ryib->elevation = FMOD360( d );
    }
    else if(strstr("corr-elevation", name)) {
       d = dds->ryib->elevation + f_val;
       dds->ryib->elevation = FMOD360( d );
    }
    else if(strstr("corr-azimuth", name)) {
       d = dds->ryib->azimuth + f_val;
       dds->ryib->azimuth = FMOD360( d );
    }
    else if(strstr("corr-rot-angle", name)) {
	/* this change made after 2.2 released */
       d = dds->asib->rotation_angle + f_val;
       dds->asib->rotation_angle = FMOD360( d );
    }
    else if(strstr("latitude", name)) {
       dds->asib->latitude = f_val;
    }
    else if(strstr("longitude", name)) {
       dds->asib->longitude = f_val;
    }
    else if(strstr("altitude", name)) {
       dds->asib->altitude_msl = f_val;
    }
    else if(strstr("agl-altitude", name)) {
       dds->asib->altitude_agl = f_val;
    }
    else if(strstr("msl-into-agl-corr", name)) {
       dds->asib->altitude_agl = dds->asib->altitude_msl
	 +dds->cfac->pressure_alt_corr +f_val;
    }
    else {
       mark = 0;
    }

    return(1);
}  
/* c------------------------------------------------------------------------ */

int se_rewrite(arg, cmds)	/* #rewrite# */
  int arg;
  struct ui_command *cmds;	
{
  /* trigger a rewrite of this ray even though no other editing 
   * is taking place
   */

    struct solo_edit_stuff *seds, *return_sed_stuff();

    seds = return_sed_stuff();
    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    return(1);
}
/* c------------------------------------------------------------------------ */

int se_remove_field(arg, cmds)	/* #ignore-field# */
  int arg;
  struct ui_command *cmds;	
{
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct dd_general_info *dgi, *dd_window_dgi();
    struct solo_edit_stuff *seds, *return_sed_stuff();
    char *name;
    int fn;

    seds = return_sed_stuff();
    if(seds->finish_up) {
	return(1);
    }
    name = (cmdq++)->uc_text;
    dgi = dd_window_dgi(seds->se_frame);

    if((fn = dd_find_field(dgi, name)) >= 0) {
	dgi->dds->field_present[fn] = NO;
	seds->modified = YES;
    }
    return(1);
}
/* c------------------------------------------------------------------------ */

int se_cpy_field(arg, cmds)
  int arg;
  struct ui_command *cmds;	
{
    /*
     * #duplicate#
     * #establish-and-reset#
     * #copy#
     * #flagged-copy#
     * #shift-field#
     */
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    char *src_name, *dst_name;
    struct dds_structs *dds;
    int ii, jj, nn, fn, fns, fnd, size=0, mark, ns, nd, nc, bad, rescale=NO;
    int fshift, src_bad, dst_bad, inc;
    char *a=NULL, *b=NULL;
    unsigned short *bnd, *flag;
    short *ss, *tt, *zz;
    float srs_scale, rcp_scale, srs_bias, dst_scale, dst_bias, x;

    src_name = (cmdq++)->uc_text;
    dst_name = (cmdq++)->uc_text;
    ns = strlen(src_name);
    nd = strlen(dst_name);

    seds = return_sed_stuff();
    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;
    nc = dds->celv->number_cells;
    bnd = seds->boundary_mask;

    if((ii=se_establish_field(dgi, src_name, dst_name, &fns, &fnd)) < 0) {
	seds->punt = YES;
	return(ii);
    }
    if(dgi->dds->parm[fns]->parameter_scale !=
       dgi->dds->parm[fnd]->parameter_scale ||
       dgi->dds->parm[fns]->parameter_bias !=
       dgi->dds->parm[fnd]->parameter_bias)
      {
	srs_scale = dgi->dds->parm[fns]->parameter_scale;
	dst_scale = dgi->dds->parm[fnd]->parameter_scale;
	srs_bias = dgi->dds->parm[fns]->parameter_bias;
	dst_bias = dgi->dds->parm[fnd]->parameter_bias;
	rescale = YES;
	rcp_scale = 1./srs_scale;
      }
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[fns];
    tt = (short *)dds->qdat_ptrs[fnd];
# else
    ss = (short *)((char *)dds->rdat[fns] + sizeof(struct paramdata_d));
    tt = (short *)((char *)dds->rdat[fnd] + sizeof(struct paramdata_d));
# endif
    bad = dds->parm[fnd]->bad_data;
    zz = ss +nc;

    /*
     * here's where we finally do the copying
     */
    if(strncmp(cmds->uc_text, "duplicate", 3) == 0) {
      if(rescale) {
	for(; ss < zz; ss++,tt++) { /* dont use boundary */
	  if(*ss == bad) {
	    *tt = bad;
	  }
	  else {
	    x = DD_UNSCALE((float)(*ss), rcp_scale, srs_bias);
	    *tt = DD_SCALE(x, dst_scale, dst_bias);
	  }
	}
      }
      else {
	for(; ss < zz; ss++,tt++) { /* dont use boundary */
	  *tt = *ss;
	}
      }
    }

    else if(strncmp(cmds->uc_text, "shift-field", 4) == 0) {
	fshift = seds->gates_of_shift;
	
	ii = fshift > 0 ? nc -1 : 0;
	inc = fshift > 0 ? -1 : 1;
	nn = fshift > 0 ? nc - fshift : nc + fshift;

	if(!rescale) {
	    for( ; nn-- ; ii += inc ) {
		*( tt + ii ) = *( ss + ii - fshift );
	    }
	}
	else {
	    for( ; nn-- ; ii += inc ) {
		if(*( ss + ii - fshift ) == bad) {
		    *( tt + ii ) = bad;
		}
		else {
		    x = DD_UNSCALE((float)( *ss + ii - fshift )
				   , rcp_scale, srs_bias);
		    *( tt + ii ) = DD_SCALE(x, dst_scale, dst_bias);
		}
	    }
	}
	/* fill in at whichever end
	 */
	ii = fshift < 0 ? nc -1 : 0;
	inc = fshift < 0 ? -1 : 1;
	nn = fshift < 0 ? -fshift : fshift;

	for( ; nn-- ; ii += inc ) {
	    *( tt + ii ) = bad;
	}
    }
    
    else if(strncmp(cmds->uc_text, "establish-and-reset", 3) == 0) {
	zz = tt +nc;
	for(; tt < zz; *tt++ = bad); /* just fill with bad flags */
    }

    else if(strncmp(cmds->uc_text, "flagged-copy", 3) == 0) { 
      /* copy using flags and boundary */
      seds->bad_flag_mask = flag = seds->bad_flag_mask_array;
      
      for(; ss < zz; ss++,tt++,bnd++,flag++) {
# ifdef perhaps
	if(*bnd && *ss != bad && *flag) {
# else
	if(*bnd && *flag) {	/* copies everything including missing data */
# endif
	  if(rescale) {
	    x = DD_UNSCALE((float)(*ss), rcp_scale, srs_bias);
	    *tt = DD_SCALE(x, dst_scale, dst_bias);
	  }
	  else {
	    *tt = *ss;
	  }
	}
      }
    }

    else {			/* copy using boundary */
      for(; ss < zz; ss++,tt++,bnd++) {
	if(*bnd) {
	  if(rescale) {
	    if(*ss == bad) {
	      *tt = bad;
	    }
	    else {
	      x = DD_UNSCALE((float)(*ss), rcp_scale, srs_bias);
	      *tt = DD_SCALE(x, dst_scale, dst_bias);
	    }
	  }
	  else {
	    *tt = *ss;
	  }
	}
      }
    }
    return(1);
}  
/* c------------------------------------------------------------------------ */

int se_funfold(arg, cmds)	/* #forced-unfolding# */
  int arg;
  struct ui_command *cmds;	
{
    int below;
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;
    char *name;
    float ctr, nyqv, rcp_nyqi;

    int nc, nd, nchar, bad, thr_bad, fn, fgg, abs_diff;
    int gg, ii, jj, kk, nn, scaled_ctr, mark, scaled_nyqv, scaled_nyqi, idiff;
    short *anchor, *ss, *zz, *thr=NULL;
    unsigned short *bnd;

    name = (cmdq++)->uc_text;
    ctr = (cmdq++)->uc_v.us_v_float;
    nd = strlen(name);

    seds = return_sed_stuff();
    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;
    nc = dgi->clip_gate+1;
    bnd = seds->boundary_mask;
    /*
     * find the field to be unfolded
     */
    if((fn = dd_find_field(dgi, name)) < 0) {
	/* field not found
	 */
	g_string_sprintfa
	  (gs_complaints, "Field to be unfolded: %s not found\n", name);
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
    scaled_ctr = DD_SCALE(ctr, dds->parm[fn]->parameter_scale
			  , dds->parm[fn]->parameter_bias);
    nyqv = seds->nyquist_velocity ? seds->nyquist_velocity
	  : dds->radd->eff_unamb_vel;
    if(nyqv <=0)
	  return(-1);

    if(seds->process_ray_count == 1) {
	printf("Nyquist vel: %.1f\n", nyqv);
    }
    scaled_nyqv = DD_SCALE(nyqv, dds->parm[fn]->parameter_scale
			  , dds->parm[fn]->parameter_bias);
    scaled_nyqi = 2*scaled_nyqv;
    rcp_nyqi = 1./(float)scaled_nyqi;

    /*
     * loop through the data
     */

    for(; ss < zz; ss++,bnd++) {
	if(!(*bnd) || *ss == bad)
	      continue;

	if(abs(idiff = scaled_ctr - (*ss)) > scaled_nyqv) {
	    nn = idiff*rcp_nyqi + (idiff < 0 ? -.5 : .5);
	    *ss += nn*scaled_nyqi;
	}
    }
    return(fn);
}  
/* c------------------------------------------------------------------------ */

int se_hard_zap(arg, cmds)		/* #unconditional-delete# */
  int arg;
  struct ui_command *cmds;	
{
    int below;			
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;
    char *dst_name;

    int nc, nd, fn, bad;
    int ii, nn, mark;
    short  *ss, *zz;
    unsigned short *bnd;


    dst_name = (cmdq++)->uc_text;
    nd = strlen(dst_name);

    seds = return_sed_stuff();
    if(!seds->boundary_exists) {
	return(1);
    }
    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    dgi = dd_window_dgi(seds->se_frame); dds = dgi->dds;
    nc = dgi->clip_gate+1;
    bnd = seds->boundary_mask;
    /*
     * find the field to be thresholded
     */
    if((fn = dd_find_field(dgi, dst_name)) < 0) {	
	/* field not found
	 */
      g_string_sprintfa
	(gs_complaints, "Field to be deleted: %s not found\n", dst_name);
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
	if(*bnd)
	      *ss = bad;
    }
    return(fn);
}  
/* c------------------------------------------------------------------------ */

int se_threshold_field(arg, cmds) /* #threshold# */
  int arg;
  struct ui_command *cmds;	
{
    int below;
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;
    char *thr_name, *dst_name, *where;
    float what, what2;

    int nc, nd, nchar, bad, thr_bad, fn, fgg;
    int gg, ii, jj, kk, nn, scaled_thr1, mark, fthr, scaled_thr2;
    short *anchor, *ss, *zz, *thr=NULL;
    unsigned short *bnd;


    dst_name = (cmdq++)->uc_text;
    thr_name = (cmdq++)->uc_text;
    where =    (cmdq++)->uc_text;
    what = (cmdq++)->uc_v.us_v_float;
    if(cmdq->uc_ctype == UTT_VALUE)
	  what2 = (cmdq)->uc_v.us_v_float;
    below = strstr(where, "below") ? YES : NO;
    nd = strlen(dst_name);
    nchar = strlen(thr_name);

    seds = return_sed_stuff();
    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;
    nc = dgi->clip_gate+1;
    fgg = seds->first_good_gate;
    bnd = seds->boundary_mask;
    /*
     * find the thr field
     */
    if((fthr = dd_find_field(dgi, thr_name)) < 0) {	
	/* thr field not found
	 */
	g_string_sprintfa
	  (gs_complaints, "Threshold field: %s not found\n", thr_name);
	seds->punt = YES;
	return(-1);
    }
# ifdef NEW_ALLOC_SCHEME
    thr = (short *)dds->qdat_ptrs[fthr];
# else
    thr = (short *)((char *)dds->rdat[fthr] + sizeof(struct paramdata_d));
# endif
    thr_bad = dds->parm[fthr]->bad_data;
    /*
     * find the field to be thresholded
     */
    if((fn = dd_find_field(dgi, dst_name)) < 0) {	
	/* field not found
	 */
	g_string_sprintfa
	  (gs_complaints, "Field to be thresholded: %s not found\n", dst_name);
	seds->punt = YES;
	return(-1);
    }
    strncpy(dgi->dds->parm[fn]->threshold_field, "          ", 8);
    strncpy(dgi->dds->parm[fn]->threshold_field, thr_name, nchar);
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[fn];
# else
    ss = (short *)((char *)dds->rdat[fn] + sizeof(struct paramdata_d));
# endif
    zz = ss +nc;
    dgi->dds->parm[fn]->threshold_value = what;
    scaled_thr1 = DD_SCALE(what, dds->parm[fthr]->parameter_scale
			  , dds->parm[fthr]->parameter_bias);
    if(cmdq->uc_ctype == UTT_VALUE)
	  scaled_thr2 = DD_SCALE(what2, dds->parm[fthr]->parameter_scale
				 , dds->parm[fthr]->parameter_bias);
    bad = dds->parm[fn]->bad_data;

    /*
     * loop through the data
     */
    for(gg=0;  gg < fgg && ss < zz;  *ss = bad, gg++,ss++,thr++,bnd++);

    if(strncmp(where, "below", 3) == 0) {
	for(; ss < zz; ss++,thr++,bnd++) {
	    if(!(*bnd) || *ss == bad)
		  continue;
	    if(*thr == thr_bad || *thr < scaled_thr1) {
		*ss = bad;
	    }
	}
    }
    else if(strncmp(where, "above", 3) == 0) {
	for(; ss < zz; ss++,thr++,bnd++) {
	    if(!(*bnd) || *ss == bad)
		  continue;
	    if(*thr == thr_bad || *thr > scaled_thr1) {
		*ss = bad;
	    }
	}
    }
    else {			/* between */
	if(cmdq->uc_ctype == UTT_VALUE) {
	    for(; ss < zz; ss++,thr++,bnd++) {
		if(!(*bnd) || *ss == bad)
		      continue;
		if(*thr == thr_bad ||
		   (*thr >= scaled_thr1 && *thr <= scaled_thr2)) {
		    *ss = bad;
		}
	    }
	}
    }

    return(fn);
}  
/* c------------------------------------------------------------------------ */

int se_radial_shear(arg, cmds)	/* #radial-shear# */
  int arg;
  struct ui_command *cmds;	
{
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    char *add_name, *src_name, *dst_name;
    struct dds_structs *dds;
    int ii, jj, fn, fns, fnd, size=0, mark, ns, nd, nc, bad;
    int ndx_ss, ndx_tt, ndx_xx;
    char *a=NULL, *b=NULL;
    unsigned short *bnd;
    short *dd, *ss, *tt, *zz;

    src_name = (cmdq++)->uc_text;
    ns = strlen(src_name);
    dst_name = (cmdq++)->uc_text;
    nd = strlen(dst_name);

    seds = return_sed_stuff();
    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    dgi = dd_window_dgi(seds->se_frame); dds = dgi->dds;
    bnd = seds->boundary_mask;

    if((ii=se_establish_field(dgi, src_name, dst_name, &fns, &fnd)) < 0) {
	seds->punt = YES;
	return(ii);
    }
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[fns];
    tt = (short *)dds->qdat_ptrs[fnd];
# else
    ss = (short *)((char *)dds->rdat[fns] + sizeof(struct paramdata_d));
    tt = (short *)((char *)dds->rdat[fnd] + sizeof(struct paramdata_d));
# endif

    dd = ss +seds->gate_diff_interval;
    zz = ss +dgi->clip_gate+1;
    bad = dds->parm[fns]->bad_data;

    /*
     * loop through the data
     */

    for(; dd < zz;) {		/* move inside the next boundary */
	for(; dd < zz && !(*bnd); dd++,ss++,tt++,bnd++);
	if(dd == zz) break;
	/*
	 * see if we can calculate a shear
	 */
	if(*dd == bad || *ss == bad) {
	    *tt = bad;
	}
	else {
	    *tt = *dd - *ss;
	}
	dd++; ss++; tt++; bnd++;
    }
    return(fnd);
}  
/* c------------------------------------------------------------------------ */

