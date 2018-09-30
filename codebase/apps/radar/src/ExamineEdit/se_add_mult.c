/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

# define NEW_ALLOC_SCHEME

# include "dorade_headers.h"
# include "solo_editor_structs.h"
# include <ui.h>
# include <ui_error.h>
# include <function_decl.h>
# include <dgi_func_decl.h>
# include <dd_math.h>
# include <glib.h>

extern GString *gs_complaints;

/*
 * 
 * se_add_const
 * se_add_fields
 * se_mult_const
 * 
 * 
 */

int se_establish_field();	/* se_catch_all.c */
static char mess[256];

/* c------------------------------------------------------------------------ */

int se_absolute_value(arg, cmds) /* #absolute-value# */
  int arg;
  struct ui_command *cmds;	
{
    int below;
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;
    char *name;

    int nc, nd, nchar, bad, thr_bad, fn, fgg, abs_diff;
    int gg, ii, jj, kk, nn, scaled_ctr, mark, scaled_nyqv, scaled_nyqi, idiff;
    short *anchor, *ss, *zz, *thr=NULL;
    unsigned short *bnd;

    name = (cmdq++)->uc_text;
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
     * find the field 
     */
    if((fn = dd_find_field(dgi, name)) < 0) {
	/* field not found
	 */
	g_string_sprintfa
	  (gs_complaints, "Field to be abs()'d: %s not found\n", name);
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

    for(; ss < zz; ss++, bnd++) {
	if(!(*bnd) || *ss == bad)
	      continue;
	*ss = abs((int)(*ss));
    }
    return(fn);
}  
/* c------------------------------------------------------------------------ */

int se_assign_value(arg, cmds)		/* #assign-value# */
				/* flagged-assign */
  int arg;
  struct ui_command *cmds;	
{
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    char *name;
    struct dds_structs *dds;
    static int count=0;
    int ii, jj, fn, fns, fnd, size=0, mark, ns, nd, nc, bad, scaled_const;
    char *a=NULL, *b=NULL;
    float f_const;
    unsigned short *bnd, *flag;
    short *ss, *tt, *zz;

    count++;
    f_const = (cmdq++)->uc_v.us_v_float;
    name = (cmdq++)->uc_text;
    ns = strlen(name);

    seds = return_sed_stuff();
    if(seds->finish_up)
	  return(1);
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;
    nc = dgi->clip_gate+1;
    bnd = seds->boundary_mask;
    seds->bad_flag_mask = flag = seds->bad_flag_mask_array;

    /*
     * find the field 
     */
    if((fns = dd_find_field(dgi, name)) < 0) {	
	/* field not found
	 */
	g_string_sprintfa
	  (gs_complaints, "Field to be assigned: %s not found\n", name);
	seds->punt = YES;
	return(-1);
    }
    seds->modified = YES;
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[fns];
# else
    ss = (short *)((char *)dds->rdat[fns] + sizeof(struct paramdata_d));
# endif
    bad = dds->parm[fns]->bad_data;
    scaled_const = DD_SCALE(f_const
		   , dds->parm[fns]->parameter_scale
		   , dds->parm[fns]->parameter_bias);
    zz = ss + nc;
    zz = ss + dgi->dds->celv->number_cells;

    /*
     * do it!
     */
    if(strstr(cmds->uc_text, "flag")) { /* flagged-assign */
	for(; ss < zz; ss++,bnd++,flag++) {
	    if(*bnd && *flag) {
		*ss = scaled_const;
	    }
	}
	return(1);
    }

    for(; ss < zz; ss++, bnd++) {
	if(*bnd) {
	    *ss = scaled_const;
	}
    }
    return(1);
}  
/* c------------------------------------------------------------------------ */

int se_add_const(arg, cmds)	/* #add-value# */
  int arg;
  struct ui_command *cmds;	
{
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    char *add_name, *src_name, *dst_name;
    struct dds_structs *dds;
    static int count=0;
    int ii, jj, fn, fns, fnd, size=0, mark, ns, nd, nc, bad, scaled_const;
    char *a=NULL, *b=NULL;
    float f_const;
    unsigned short *bnd;
    short *ss, *tt, *zz;

    count++;
    f_const = (cmdq++)->uc_v.us_v_float;
    src_name = (cmdq++)->uc_text;
    ns = strlen(src_name);
    dst_name = (cmdq++)->uc_text;
    nd = strlen(dst_name);

    seds = return_sed_stuff();
    if(seds->finish_up)
	  return(1);
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;
    nc = dgi->clip_gate+1;
    bnd = seds->boundary_mask;

    if((ii=se_establish_field(dgi, src_name, dst_name, &fns, &fnd)) < 0) {
	seds->punt = YES;
	return(ii);
    }
    seds->modified = YES;
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[fns];
# else
    ss = (short *)((char *)dds->rdat[fns] + sizeof(struct paramdata_d));
# endif
    zz = ss +nc;
# ifdef NEW_ALLOC_SCHEME
    tt = (short *)dds->qdat_ptrs[fnd];
# else
    tt = (short *)((char *)dds->rdat[fnd] + sizeof(struct paramdata_d));
# endif
    bad = dds->parm[fns]->bad_data;
    scaled_const = DD_SCALE(f_const
		   , dds->parm[fns]->parameter_scale
		   , dds->parm[fns]->parameter_bias);
    /*
     * here's where we finally do the work
     */
    for(; ss < zz; ss++,tt++,bnd++) {
	if(*bnd)
	      *tt = (*ss == bad) ? bad : *ss + scaled_const;
    }
    return(1);
}  
/* c------------------------------------------------------------------------ */

int se_add_fields(arg, cmds)
  int arg;
  struct ui_command *cmds;	
{
    /*
     * #add-field#
     * #subtract#
     * #mult-fields#
     * #merge-fields#
     */
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    char *add_name, *src_name, *dst_name;
    struct dds_structs *dds;
    static int count=0;
    int ii, jj, fn, fna, fns, fnd, size=0, mark, na, ns, nd, nc, bad, add;
    int rescale=NO, ival;
    float srs_scale, rcp_scale, srs_bias, dst_scale, dst_bias, x, y, z;
    float aa_scale, aa_rcp_scale, aa_bias;
    double d;
    char *a=NULL, *b=NULL;
    unsigned short *bnd;
    short *ss, *tt, *zz, *aa;

    count++;
    add_name = (cmdq++)->uc_text;
    na = strlen(add_name);
    src_name = (cmdq++)->uc_text;
    dst_name = (cmdq++)->uc_text;

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
     * find the add field
     */
    if((fn = dd_find_field(dgi, add_name)) < 0) {	
	g_string_sprintfa
	  (gs_complaints
	   , "First parameter %s not found for add/sub\n", add_name);
	seds->punt = YES;
	return(-1);
    }
# ifdef NEW_ALLOC_SCHEME
    aa = (short *)dds->qdat_ptrs[fn];
# else
    aa = (short *)((char *)dds->rdat[fn] + sizeof(struct paramdata_d));
# endif

    if((fns = dd_find_field(dgi, src_name)) < 0) {	
	g_string_sprintfa
	  (gs_complaints, "Second parameter %s not found for add/sub\n", src_name);
	seds->punt = YES;
	return(-1);
    }
    if((ii=se_establish_field(dgi, src_name, dst_name, &fns, &fnd)) < 0) {
	seds->punt = YES;
	return(ii);
    }
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[fns];
# else
    ss = (short *)((char *)dds->rdat[fns] + sizeof(struct paramdata_d));
# endif
    zz = ss +nc;
# ifdef NEW_ALLOC_SCHEME
    tt = (short *)dds->qdat_ptrs[fnd];
# else
    tt = (short *)((char *)dds->rdat[fnd] + sizeof(struct paramdata_d));
# endif
    bad = dds->parm[fns]->bad_data;
    aa_rcp_scale = 1./dgi->dds->parm[fn]->parameter_scale;
    aa_bias = dgi->dds->parm[fn]->parameter_bias;
    
    rcp_scale = 1./dgi->dds->parm[fns]->parameter_scale;
    srs_bias = dgi->dds->parm[fns]->parameter_bias;
    
    dst_scale = dgi->dds->parm[fnd]->parameter_scale;
    dst_bias = dgi->dds->parm[fnd]->parameter_bias;


    if(dgi->dds->parm[fn]->parameter_scale !=
       dgi->dds->parm[fns]->parameter_scale ||

       dgi->dds->parm[fns]->parameter_scale !=
       dgi->dds->parm[fnd]->parameter_scale ||

       dgi->dds->parm[fn]->parameter_bias !=
       dgi->dds->parm[fns]->parameter_bias ||

       dgi->dds->parm[fns]->parameter_bias !=
       dgi->dds->parm[fnd]->parameter_bias)
      {
	rescale = YES;
      }


    /*
     * loop through the data
     */
    if(strncmp(cmds->uc_text, "add", 3) == 0) {
      /* for "add" we do a substitude when only one of the fields
       * is flagged bad
       */
      for(; ss < zz; aa++,ss++,tt++,bnd++) {
	if(*bnd) {
	  if(rescale) {		/* nightmare time! */
	    if(*aa != bad) {
	      x = DD_UNSCALE((float)(*aa), aa_rcp_scale, aa_bias);
	      if(*ss != bad) {
		y = DD_UNSCALE((float)(*ss), rcp_scale, srs_bias);
		*tt = DD_SCALE(x+y, dst_scale, dst_bias);
	      }	/* *ss ok */
	      else {
		*tt = DD_SCALE(x, dst_scale, dst_bias);
	      }	/* ss !ok */
	    } /* *aa ok */
	    else if(*ss != bad) {
	      y = DD_UNSCALE((float)(*ss), rcp_scale, srs_bias);
	      *tt = DD_SCALE(y, dst_scale, dst_bias);
	    }	/* *ss ok */
	    else {
	      *tt = bad;
	    }
	  } /* rescale */

	  else {		/* no rescaling necessary */
	    if(*aa != bad) {
	      if(*ss != bad) {
		*tt = *aa + *ss;
	      }
	      else {
		*tt = *aa;
	      }
	    } /* *aa ok */
	    else if(*ss != bad) {
	      *tt = *ss;
	    }
	    else {
	      *tt = bad;
	    }
	  } /* !rescale */
	} /* end bnd */
      }	/* end for */
    } /* end add */

    else if(strncmp(cmds->uc_text, "merge", 3) == 0) {
      /*
       * in this case the first field "add_name" represented by "aa"
       * is considered the dominant field over the second field
       * "src_name" represented by "ss" and if (*aa) does not contain
       * a bad flag it is plunked into the destination field "tt"
       * otherwise (*ss) is plunked into the destination field
       */
      for(; ss < zz; aa++,ss++,tt++,bnd++) {
	if(*bnd) {
	  if(rescale) {		/* nightmare time! */
	    if(*aa != bad) {
	      x = DD_UNSCALE((float)(*aa), aa_rcp_scale, aa_bias);
	      *tt = DD_SCALE(x, dst_scale, dst_bias);
	    } /* *aa ok */
	    else if(*ss != bad) {
	      y = DD_UNSCALE((float)(*ss), rcp_scale, srs_bias);
	      *tt = DD_SCALE(y, dst_scale, dst_bias);
	    }	/* *ss ok */
	    else {
	      *tt = bad;
	    }
	  } /* rescale */

	  else if(*aa != bad) {
	    *tt = *aa;
	  }
	  else {
	    *tt = *ss;
	  } /* !rescale */
	} /* end bnd */
      }	/* end for */
    } /* end "merge" */


    else if(strncmp(cmds->uc_text, "mult-fields", 3) == 0) {
	/* multiply two fields together */
	
	for(; ss < zz; aa++,ss++,tt++,bnd++) {
	    if(*bnd) {
		if(*aa != bad && *ss != bad) {
		    x = DD_UNSCALE((float)(*aa), aa_rcp_scale, aa_bias);
		    y = DD_UNSCALE((float)(*ss), rcp_scale, srs_bias);
		    d = x * y;
		    *tt = DD_SCALE(d, dst_scale, dst_bias);
		}
		else {
		    *tt = bad;
		}
	    } /* end bnd */
	} /* end for */
    } /* end "mult-fields" */
    
    else {			/* subtract */
      /* for subtract we only subtract when both fields are
       * not flagged bad
       */
      for(; ss < zz; aa++,ss++,tt++,bnd++) {
	if(*bnd) {
	  if(rescale) {	
	    if(*aa != bad && *ss != bad) {
	      x = DD_UNSCALE((float)(*aa), aa_rcp_scale, aa_bias);
	      y = DD_UNSCALE((float)(*ss), rcp_scale, srs_bias);
	      *tt = DD_SCALE(y-x, dst_scale, dst_bias);
	    }
	    else {
	      *tt = bad;
	    }
	  } /* rescale */

	  else if(*aa != bad && *ss != bad) {
	    *tt = *ss - *aa;
	  } 
	  else {
	    *tt = bad;
	  }
	} /* end bnd */
      }	/* end for */
    } /* end "subtract" */
    return(1);
}  
/* c------------------------------------------------------------------------ */

int se_mult_const(arg, cmds)	
  int arg;
  struct ui_command *cmds;	
{
   /* #multiply#
    * #rain-rate#
    * #exponentiate#
    */
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    char *add_name, *src_name, *dst_name;
    struct dds_structs *dds;
    int ii, jj, fn, fns, fnd, size=0, mark, ns, nd, nc, bad, dst_bad;
    int rescale = NO;
    char *a=NULL, *b=NULL;
    float f_const;
    unsigned short *bnd;
    short *ss, *tt, *zz;
    double d, rcp_scale, srs_bias, dst_scale, dst_bias, d_const, xx;
    double A=.003333333, X=.7142857;

    src_name = (cmdq++)->uc_text;
    ns = strlen(src_name);
    d_const = f_const = (cmdq++)->uc_v.us_v_float;
    dst_name = (cmdq++)->uc_text;
    nd = strlen(dst_name);

    seds = return_sed_stuff();
    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;
    nc = dgi->clip_gate+1;
    bnd = seds->boundary_mask;

    if((ii=se_establish_field(dgi, src_name, dst_name, &fns, &fnd)) < 0) {
	seds->punt = YES;
	return(ii);
    }
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[fns];
# else
    ss = (short *)((char *)dds->rdat[fns] + sizeof(struct paramdata_d));
# endif
    zz = ss +nc;
# ifdef NEW_ALLOC_SCHEME
    tt = (short *)dds->qdat_ptrs[fnd];
# else
    tt = (short *)((char *)dds->rdat[fnd] + sizeof(struct paramdata_d));
# endif
    bad = dds->parm[fns]->bad_data;
    dst_bad = dds->parm[fnd]->bad_data;

    if(dgi->dds->parm[fns]->parameter_scale !=
       dgi->dds->parm[fnd]->parameter_scale ||

       dgi->dds->parm[fns]->parameter_bias !=
       dgi->dds->parm[fnd]->parameter_bias)
      {
	rescale = YES;
      }

    rcp_scale = 1./dgi->dds->parm[fns]->parameter_scale;
    srs_bias = dgi->dds->parm[fns]->parameter_bias;
    
    dst_scale = dgi->dds->parm[fnd]->parameter_scale;
    dst_bias = dgi->dds->parm[fnd]->parameter_bias;

    /*
     * here's where we finally do the work
     */
    if((strncmp(cmds->uc_text, "rain-rate", 4) == 0) ||
       (strncmp(cmds->uc_text, "exponentiate", 4) == 0)) {

       if((strncmp(cmds->uc_text, "rain-rate", 4) == 0)) {
	  if(d_const) {
	     X = d_const;
	  }
	  for(; ss < zz; ss++,tt++,bnd++) {
	     if(*bnd) {
		if(*ss == bad) {
		   *tt = dst_bad;
		}
		else {
		   d = DD_UNSCALE((double)(*ss), rcp_scale, srs_bias);
		   d = A * pow((double)10.0, (double)(0.1 * d * X));
		   *tt = DD_SCALE(d, dst_scale, dst_bias);
		}
	     }
	  }
       }
       else {			/* exponentiate */
	  for(; ss < zz; ss++,tt++,bnd++) {
	     if(*bnd) {
		if(*ss == bad) {
		   *tt = dst_bad;
		}
		else {
		   d = DD_UNSCALE((double)(*ss), rcp_scale, srs_bias);
		   d = pow(d, d_const);
		   *tt = DD_SCALE(d, dst_scale, dst_bias);
		}
	     }
	  }
       }
    }
    else {
	if( rescale ) {
	    for(; ss < zz; ss++,tt++,bnd++) {
		if(*bnd) {
		    if( *ss == bad ) {
			*tt = dst_bad;
			continue;
		    }
		    xx = DD_UNSCALE((float)(*ss), rcp_scale, srs_bias);
		    xx *= f_const;
		    *tt = DD_SCALE(xx, dst_scale, dst_bias);
		}
	    }
	}
	else {
	    for(; ss < zz; ss++,tt++,bnd++) {
		if(*bnd)
		    *tt = (*ss == bad) ? bad : *ss * f_const;
	    }
	}
    }
    return(1);
}  
/* c------------------------------------------------------------------------ */

int se_mangle_field(arg, cmds)
  int arg;
  struct ui_command *cmds;	
{
    /*
     * #z-linearize#
     * 
     */
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    char *src_name, *dst_name;
    struct dds_structs *dds;
    int ii, jj, fn, fns, fnd, size=0, mark, ns, nd, nc, bad, rescale=NO;
    char *a=NULL, *b=NULL;
    unsigned short *bnd, *flag;
    short *ss, *tt, *zz;
    double srs_scale, rcp_scale, srs_bias, dst_scale, dst_bias, x;
    double d;

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
    rcp_scale = 1./dgi->dds->parm[fns]->parameter_scale;
    dst_scale = dgi->dds->parm[fnd]->parameter_scale;
    srs_bias = dgi->dds->parm[fns]->parameter_bias;
    dst_bias = dgi->dds->parm[fnd]->parameter_bias;

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
     * here's where we finally do the work
     */
    if(strncmp(cmds->uc_text, "z-linearize", 5) == 0) {
       for(; ss < zz; ss++,tt++,bnd++) {
	  if(*bnd) {
	     if(*ss == bad) {
		*tt = bad;
	     }
	     else {
		d = DD_UNSCALE((double)(*ss), rcp_scale, srs_bias);
		d = WATTZ(d);
		*tt = DD_SCALE(d, dst_scale, dst_bias);
	     }
	  }
       }
    }
    return(1);
}  
/* c------------------------------------------------------------------------ */


