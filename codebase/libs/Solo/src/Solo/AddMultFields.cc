
/*
 * 
 * se_add_const
 * se_add_fields
 * se_mult_const
 * 
 * 
 */


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


