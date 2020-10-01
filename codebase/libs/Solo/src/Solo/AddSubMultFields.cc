#include <Solo/GeneralDefinitions.hh>

/*
 * 
 * se_add_const
 * se_add_fields
 * se_mult_const
 * 
 * 
 */

/*
If the first field value in not bad it is placed in the third field, otherwise the second field value is placed in the third field
*/

//int se_add_fields(arg, cmds)
//  int arg;
//  struct ui_command *cmds;	

void se_merge_fields(const float *data1, const float *data2, 
		     float *newData, size_t nGates,
		     float bad, size_t dgi_clip_gate,
		     bool *boundary_mask)
{
    /*
     * #add-field#
     * #subtract#
     * #mult-fields#
     * #merge-fields#
     */

    static int count=0;
    int ii, jj, fn, fna, fns, fnd, size=0, mark, na, ns, nd;
    size_t nc;
    int add;
    int ival;
    float x, y, z;
    double d;
    //    char *a=NULL, *b=NULL;
    bool *bnd;
    const float *ss, *zz, *aa;
    float *tt;

    count++;

    //    add_name = (cmdq++)->uc_text;
    //    na = strlen(add_name);
    //    src_name = (cmdq++)->uc_text;
    //    dst_name = (cmdq++)->uc_text;

    nc = dgi_clip_gate;
    bnd = boundary_mask;

    aa = data1; //(short *)dds->qdat_ptrs[fn];

    ss = data2; // (short *)dds->qdat_ptrs[fns];

    zz = ss +nc;  // set the end point

    tt = newData; // (short *)dds->qdat_ptrs[fnd];
    //bad = dds->parm[fns]->bad_data;
    //aa_rcp_scale = 1./dgi->dds->parm[fn]->parameter_scale;
    //aa_bias = dgi->dds->parm[fn]->parameter_bias;
    
    //rcp_scale = 1./dgi->dds->parm[fns]->parameter_scale;
    //srs_bias = dgi->dds->parm[fns]->parameter_bias;
    
    //dst_scale = dgi->dds->parm[fnd]->parameter_scale;
    //dst_bias = dgi->dds->parm[fnd]->parameter_bias;

    /*
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
    */

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
