#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <Solo/GeneralDefinitions.hh>

// Bad flags can be set using values in one data field,
// then applied to another data field.
// So, the flag "mask" survives from one function to another.
// The bad flag mask is like a boundary mask.
// Pass in a bad_flag_mask, set it using a data vector and some criteria,
//  then, pass back the bad_flag_mask for use in another function.


// need a bad flags mask array; will this be like a boundary mask??
// need to be persistent? static?
// Q: how is bad flag mask used?
// editor/se_for_each.c:      seds->bad_flag_mask = flag = seds->bad_flag_mask_array;
//      for flagged-copy ... consider bad_flag_mask and boundary_mask
// editor/se_defrkl.c:    seds->bad_flag_mask = flag = seds->bad_flag_mask_array;
//      for flag-freckles ... consider bad_flag_mask and boundary_mask
//      for flag-glitches ...    "       "
//bool *bad_flag_mask_array;  // unsigned short, originally in Soloii
//bool *bad_flag_mask;        // unsigned short, originally in Soloii
//bool use_bad_flag_mask;     // int, originally in Soloii


/*
 * se_assert_bad_flags
 * se_bad_flags_logic
 * se_clear_bad_flags
 * se_copy_bad_flags
 * se_set_bad_flags
 * 
 */

// TODO: how to report errors?



/* c------------------------------------------------------------------------ */

void se_do_clear_bad_flags_array(bool *bad_flag_mask, size_t nn)
{

  memset(bad_flag_mask, false, nn * sizeof(bool)); 
  // (nn > 0 ? nn : seds->max_gates) * sizeof(*seds->bad_flag_mask_array));
}
/* c------------------------------------------------------------------------ */

//int se_set_bad_flags(arg, cmds)	/* #set-bad-flags# */
//  int arg;
//  struct ui_command *cmds;
// 
// In params:
// data    pointer to data values
// bad     value to indicate bad value in data

// In/Out params:
// bad_flags_mask send preallocated space, same size as data
//                returns boolean array: 
//                  true if data value meets bad flag criteria AND in inside boundary mask
void se_set_bad_flags(Where where, float scaled_thr1, float scaled_thr2, const float *data, size_t nGates,
		      float bad, size_t dgi_clip_gate, bool *boundary_mask, bool *bad_flag_mask)
{
  size_t nc; 
    const float *zz;
    const float *thr;
    bool *bnd, *flag; // was unsigned short

    if (dgi_clip_gate >= nGates)
      nc = nGates;
    else 
      nc = dgi_clip_gate;

    bnd = boundary_mask;
    flag = bad_flag_mask;

    // thr is a pointer to the data
    thr = data;
    zz = thr +nc;
    se_do_clear_bad_flags_array(bad_flag_mask, nc);
   
    switch(where) {
    case BELOW: 
	for(; thr < zz; thr++,bnd++,flag++) {
	    if(!(*bnd) || *thr == bad)
	       continue;
	    if(*thr < scaled_thr1) {
		*flag = true;
	    }
	}
	break;
    case ABOVE: 
	for(; thr < zz; thr++,bnd++,flag++) {
	    if(!(*bnd) || *thr == bad)
	      continue;
	    if(*thr > scaled_thr1) {
		*flag = true;
	    }
	}
	break;
    default:		/* between */
	for(; thr < zz; thr++,bnd++,flag++) {
	    if(!(*bnd) || *thr == bad)
	      continue;
	    if(*thr >= scaled_thr1 && *thr <= scaled_thr2) {
		*flag = true;
	    }
	}
    }
}  
/* c------------------------------------------------------------------------ */


/* c------------------------------------------------------------------------ */

/* #assert-bad-flags# */

void se_assert_bad_flags(const float *data, float *newData, size_t nGates,
			 float bad, size_t dgi_clip_gate,
			 bool *boundary_mask, const bool *flag) // *bad_flag_mask)
{
    size_t  nc;
    float *ss, *zz;
    const float *orig;
    bool *bnd; // , *flag;

    if (dgi_clip_gate >= nGates)
      nc = nGates;
    else 
      nc = dgi_clip_gate;

    bnd = boundary_mask;
    //flag = bad_flag_mask;
    orig = data;
    ss = newData;
    zz = ss + nc;
    
    /*
     * loop through the data
     */

    for(; ss < zz; ss++,bnd++,flag++,orig++) {
      if(*bnd && *flag)
	*ss = bad;
      else 
	*ss = *orig;
    }
}  
/* c------------------------------------------------------------------------ */

// add or multiply each flagged data value by a constant
void se_flagged_add(float f_const, bool multiply, const float *data, float *newData, size_t nGates,
		    float bad, size_t dgi_clip_gate,
		    bool *boundary_mask, const bool *flag) // bad_flag_mask)
// #flagged-add# 
{
  // #flagged-add#
  // #flagged-multiply#
  //
    char *name, *where;

    size_t nc;
    //  bad, fn;
    int gg, ii, jj, kk, nn, scaled_const, mark;
    float *ss, *zz;
    const float *orig;
    bool *bnd; // , *flag, 
    bool *uflag;

    //f_const = (cmdq++)->uc_v.us_v_float;
    //name = (cmdq++)->uc_text;

    if (dgi_clip_gate >= nGates)
      nc = nGates;
    else 
      nc = dgi_clip_gate;

    bnd = boundary_mask;
    //flag = bad_flag_mask;
    ss = newData;
    zz = ss +nc;
    orig = data;
 
    memcpy(newData, data, nGates*sizeof(float));
 
    //
    // loop through the data
    //

    if(multiply) { // strstr(cmds->uc_text, "d-m")) { // multiply 
      for(; ss < zz; ss++,bnd++,flag++,orig++) {
	if(*bnd && *ss != bad && *flag) {
	  *ss = *orig * f_const;
	} else
	  *ss = *orig;
      }
    }
    else {			// add 
      for(; ss < zz; ss++,bnd++,flag++,orig++) {
	if(*bnd && *ss != bad && *flag) {
	  *ss = *orig + f_const;
	} else
	  *ss = *orig;
      }
    }
}
/* c------------------------------------------------------------------------ */

// sets bad_flag_mask In/out parameter
void se_bad_flags_logic(float scaled_thr1, float scaled_thr2, enum Where where, 
			enum Logical logical_operator, const float *data, size_t nGates,
			float bad, size_t dgi_clip_gate,
			bool *boundary_mask, const bool *flag, // *bad_flag_mask,
			bool *updated_bad_flag_mask)
{
  //
  // #and-bad-flags#
  // #or-bad-flags#
  // #xor-bad-flags#
  //

    size_t nc; 
    int gg, ii, jj, kk, nn;
    const float *zz;
    const float *thr;
    bool *bnd;
    //  *flag,
    bool *uflag;

    if (dgi_clip_gate >= nGates)
      nc = nGates;
    else 
      nc = dgi_clip_gate;

    bnd = boundary_mask;
    //    flag = bad_flag_mask;
    uflag = updated_bad_flag_mask;
    thr = data;

    zz = thr +nc;

    /*
     * loop through the data
     */

    switch(where) {
    case BELOW:
      switch (logical_operator) {
      case AND:
	  for(; thr < zz; thr++,bnd++,flag++,uflag++) {
		if(!(*bnd))
		      continue;
		if(*thr == bad) {
		    *uflag = false;
		}
		else {
		  *uflag = *flag & (*thr < scaled_thr1);
		}
	    }
	  break;
      case XOR:
	  for(; thr < zz; thr++,bnd++,flag++,uflag++) {
		if(!(*bnd))
		      continue;
		if(*thr != bad)
		  *uflag = *flag ^ (*thr < scaled_thr1);
	    }
	  break;
      case OR:
	  for(; thr < zz; thr++,bnd++,flag++,uflag++) {
		if(!(*bnd))
		      continue;
		if(*thr != bad)
		  *uflag = *flag | (*thr < scaled_thr1);
	    }
	  break;
      }
      break;
    case ABOVE:
      switch (logical_operator) {
      case AND:
	  for(; thr < zz; thr++,bnd++,flag++,uflag++) {
		if(!(*bnd))
		      continue;
		if(*thr == bad) {
		    *uflag = false;
		}
		else {
		  *uflag = *flag & (*thr > scaled_thr1);
		}
	    }
	  break;
      case XOR:
	  for(; thr < zz; thr++,bnd++,flag++,uflag++) {
		if(!(*bnd))
		      continue;
		if(*thr != bad)
		  *uflag = *flag ^ (*thr > scaled_thr1);
	    }
	  break;
      case OR:
	  for(; thr < zz; thr++,bnd++,flag++,uflag++) {
		if(!(*bnd))
		      continue;
		if(*thr != bad)
		  *uflag = *flag | (*thr > scaled_thr1);
	    }
	  break;
      }
      break;
    case BETWEEN:
      switch (logical_operator) {
      case AND:
	  for(; thr < zz; thr++,bnd++,flag++,uflag++) {
		if(!(*bnd))
		      continue;
		if(*thr == bad) {
		    *uflag = false;
		}
		else {
		  *uflag = *flag & (*thr >= scaled_thr1 && *thr <= scaled_thr2);
		}
	    }
	  break;
      case XOR:
	  for(; thr < zz; thr++,bnd++,flag++,uflag++) {
		if(!(*bnd))
		      continue;
		if(*thr != bad) {
		  *uflag = *flag ^ (*thr >= scaled_thr1 && *thr <= scaled_thr2);
		}
	    }
	  break;
      case OR:
	  for(; thr < zz; thr++,bnd++,flag++,uflag++) {
		if(!(*bnd))
		      continue;
		if(*thr != bad) {
		  *uflag = *flag | (*thr >= scaled_thr1 && *thr <= scaled_thr2);
		}
	    }
	  break;
      }
    }
}
/* c------------------------------------------------------------------------ */

void se_clear_bad_flags(bool complement, 
			const bool *flag, // *bad_flag_mask, 
			bool *complement_mask, size_t nGates)
{
  /* #clear-bad-flags#       complement == false
   * #complement-bad-flags#  complement == true
   */
  size_t nn;
  //bool *flag
  bool *cflag;

  //flag = bad_flag_mask;
  cflag = complement_mask;

    if(complement) {
      nn = nGates;
      for(; nn--; flag++, cflag++) {
	*cflag = *flag ? 0 : 1;
      }
    }
    else {
      se_do_clear_bad_flags_array(cflag, nGates);
    }
}  
/* c------------------------------------------------------------------------ */

// NOTE: bad_flag_mask In/Out parameter; flags outside boundary are NOT changed.
//
void se_copy_bad_flags(const float *data, size_t nGates,
			 float bad, size_t dgi_clip_gate,
			 bool *boundary_mask, bool *bad_flag_mask)
// #copy-bad-flags# 
{
    /* creates a bad flag mask corresponding to the bad flagged
     * gates in the test field
     */

    // char *name, *where;

    size_t nc;
    //  nd, nchar, bad, thr_bad, fn, fgg;
    int gg, ii, jj, kk, nn, scaled_thr1, scaled_thr2, mark;
    const float *zz, *thr;
    bool *bnd, *flag;

    if (dgi_clip_gate >= nGates)
      nc = nGates;
    else 
      nc = dgi_clip_gate;

    //fgg = seds->first_good_gate;
    bnd = boundary_mask;
    flag = bad_flag_mask;
    /*
     * find the thr field
     */
    thr = data; // ??? (short *)((char *)dds->rdat[fn] + sizeof(struct paramdata_d));

    //thr_bad = bad; // dds->parm[fn]->bad_data;
    zz = thr + nc;
    //bad = dds->parm[fn]->bad_data;

    /*
     * loop through the data
     */

    for(; thr < zz; thr++,bnd++,flag++) {
	if(!(*bnd))
	      continue;
	*flag = *thr == bad ? true : false;
    }
}  



/* c------------------------------------------------------------------------ */

// sets bad flag mask
//
void se_flag_glitches(float deglitch_threshold, int deglitch_radius,
		      int deglitch_min_bins,  // aka deglitch_min_gates
		      const float *data, size_t nGates,
		      float bad, size_t dgi_clip_gate,
		      bool *boundary_mask, bool *bad_flag_mask) 
// #flag-glitches# 
{
  /* routine to remove discountinuities (freckles) from the data                                    
   * such as birds and radio towers by comparing a particular                                       
   * data point to a running average that is ahead of the point of interest                         
   * but switches to a trailing average once enough good points                                     
   * have been encountered                                                                          
   */

  // uses one queue, and indexes into the queue like an array.
  //struct ui_command *cmdq=cmds+1; /* point to the first argument */
  int jj, kk, mm, nn, navg; // mark, ;
  size_t nc, ndx_ss;
  float scaled_thr;
  int ndx_qend, ndx_qctr, min_bins, good_bins, half, sum, ival;
  double rcp_ngts, davg, diff;
  //char *name;
  //vector<int> que;
  static int que_size = 0;
  static float *que, *qctr;

  const float *ss;
  bool *bnd, *flag;
  /* boundary mask is set to 1 if the corresponding cell satisfies                                  
   * conditions of the boundaries                                                                   
   */



  bnd = boundary_mask;
  flag = bad_flag_mask;
  ss = data;
  scaled_thr = deglitch_threshold;

  if( deglitch_radius < 1 )
    { deglitch_radius = 3; }
  navg = deglitch_radius *2 +1;
  half = navg/2;

  if (navg > nGates)  // TODO: what is a good bound on this number? 100?
    throw "ERROR, deglitch radius must be less than half the number of gates in ray";

  // TODO: check this logic, it seems wacked
  if( deglitch_min_bins > 0 ) {
    if( deglitch_min_bins > navg )
      { deglitch_min_bins = navg; }
    min_bins = deglitch_min_bins;
  }
  else
    { deglitch_min_bins = min_bins = navg; }

  
  //if(seds->process_ray_count == 1) { // TODO: I don't know what this means; process_ray_count???
  //                                                                                            
  // set up                                                                                     
  //
  // TODO: move to new & delete instead of malloc and free
  //if( navg > que_size ) {
  //  if( que_size )
  //    { delete[] que; }
    que = new float[navg]; // (float *)malloc( navg * sizeof( float ));
    que_size = navg;
    //}
  //}
  

    if (dgi_clip_gate >= nGates)
      nc = nGates;
    else 
      nc = dgi_clip_gate;

  // TODO: I don't think this code works ...
  // ndx_ss is an index ( offset into data vector )
  // navg must be the number of values in the running average of good bins
  //
  for( ndx_ss = 0; ndx_ss < nc; ) {
    //                                                                                             
    // move the cell index to the first                                                            
    // gate inside the next boundary                                                               
    //
    //for(; ndx_ss < nc && !(*(bnd + ndx_ss)); ndx_ss++ );
    while (ndx_ss < nc && !(*(bnd + ndx_ss))) {
      ndx_ss++;
    }

    //                                                                                            
    // set up the queue                                                                           
    //
    ndx_qend = 0;
    good_bins = 0;
    sum = 0;

    // and start looking for the good gates count to equal or                                     
    // exceed the min_bins and the center bin not be bad                                          
    //
    // ndx_qend points to next empty spot in que

    for(mm = 0; ndx_ss < nc && *(bnd+ndx_ss); ndx_ss++ ) {
      if( ++mm > navg ) {  /* after the que is full */
	ival = *(que + ndx_qend);
	if( ival != bad )
	  { good_bins--; sum -= ival; }
      }
      ival = *(ss + ndx_ss); /* raw data value */
      *(que + ndx_qend) = ival;
      if( ival != bad )
	{ sum += ival; good_bins++; }

      // ndx_qctr is the index to the center of the queue 
      ndx_qctr = (ndx_qend - half + navg ) % que_size;
      qctr = que + ndx_qctr;

      if( good_bins >= min_bins && *qctr != bad ) {
	//                                                                                      
	// do a test                                                                            
	//
	davg = (double)(sum - *qctr)/(double)(good_bins -1);
	if(( diff = fabs( davg - *qctr)) > scaled_thr ) {
	  sum -= *qctr;
	  good_bins--;
	  *qctr = bad;
	  // ii = ndx_ss - half;
          // TODO: I think this marks the wrong position; maybe remove "- half" ???
	  *(flag + ndx_ss - half) = true; // flag this gate 
	}
      }
      ndx_qend = ++ndx_qend % que_size;
    }
  }
  delete[] que;
}


/* ----

struct que_val {
  int val;
  float f_val;
  double d_val;
  struct que_val *last; // not used
  struct que_val *next;
};
*/
// TODO: use a vector of float? operate as a FIFO que?
/* c------------------------------------------------------------------------ 

struct running_avg_que {
  double sum;
  double rcp_num_vals;
  int in_use;
  int num_vals;
  struct que_val *at; // this is the head of the list of que_val(s)
  struct running_avg_que *last;
  struct running_avg_que *next;
*/

/* c------------------------------------------------------------------------ */

// TODO: I have attempted to debug and test this function,
// but it is just pretty wonky.  
//
// bad_flag_mask is set
// if a gate is considered a freckle, bad_flag_mask = true;
// TODO: Question: Is the mask updated? or created?  (it is updated)
void se_flag_freckles(float freckle_threshold, size_t freckle_avg_count,
		      const float *data, size_t nGates,
		      float bad, size_t dgi_clip_gate,
		      bool *boundary_mask, bool *bad_flag_mask) 

// (arg, cmds)/* #flag-freckles# */
//  int arg;
//struct ui_command *cmds;
{
  /* routine to remove discountinuities (freckles) from the data
   * such as birds and radio towers by comparing a particular
   * data point to a running average that is ahead of the point of interest
   * but switches to a trailing average once enough good points
   * have been encountered
   */

  // uses two queues and moves through them like a linked list 
  // via last and next pointers; the last pointer is never used.
  //
  //  struct ui_command *cmdq=cmds+1; /* point to the first argument */
  int fn, ii, jj, kk, mm, nn, mark, navg;
  size_t  nc, ndx_ss, ndx_q0;
  bool out_of_bounds;
  // int q1_ndx; // not used
  int scaled_thr;  // TODO: should this be a float?
  double rcp_ngts;
  //  char *name;

  //  struct solo_edit_stuff *seds, *return_sed_stuff();
  //  struct dd_general_info *dgi, *dd_window_dgi();
  //  struct dds_structs *dds;
  //static struct running_avg_que *raq0, *raq1;
  float *raq0, *raq1;
  //struct running_avg_que *se_return_ravgs();  // TODO: HERE <<== keep and manage this struct then fewer code changes; to logic below.
  //struct que_val *qv0, *qv1;
  size_t qv0, qv1; // index into running average que
  const float *ss;
  float  xx, ref0, ref1, raq0_sum, raq1_sum;
  bool *bnd, *flag;
  /* boundary mask is set to 1 if the corresponding cell satisfies
   * conditions of the boundaries
   */

  //  if(strncmp(cmds->uc_text, "flag-glitches", 7) == 0) {
  //    ii = se_flag_glitches(arg, cmds);
  //    return ii;
  //  }

  bnd = boundary_mask;
  flag = bad_flag_mask;

  ss = data; // (short *)dds->qdat_ptrs[fn];

  scaled_thr = freckle_threshold;
  //bad = dds->parm[fn]->bad_data;
  navg = freckle_avg_count;
  if (navg > nGates)  // TODO: what is a good bound on this number? 100?
    throw "ERROR, freckle average count greater than number of gates in ray";

  rcp_ngts = 1./navg;
  printf("rcp_ngts = %f\n", rcp_ngts);

  // TODO: is process_ray_count the number of rays processed?
  // i.e. the rayIdx?  Correct.  If I use an array with mod to make the indexes
  // circular, then I don't need the process_ray_count
  //if(seds->process_ray_count == 1) {
    /*
     * set up for two running average queues
     */
    // TODO: are the queues circular?  as in the last element -> next = head? YES.  
    // TODO: I'll need to setup the pointers, at initialization. 
    // Vector:
    //   need to modify code in algorithm.  
    // struct running_avg_que:
    //   no change to code in algorithm.
    //   bring along raq code; need static variables to retain raqs
    raq0 = new float[navg]; // float[navg]; // se_return_ravgs(navg);
    raq1 = new float[navg]; // se_return_ravgs(navg);
    //}
  if (dgi_clip_gate > nGates) {
    nc = nGates;
  } else {
    nc = dgi_clip_gate;
  }
  nn = navg +1;  // number of average values

  // ndx_ss  is the index through the ray
  // ndx_q0  is the index in the ray for the leading average que
  for(ndx_ss=0; ndx_ss < nc;) { 

    // move the cell index (ndx_ss) to the first
    // good gate inside the next boundary
    for(; ndx_ss < nc && (!(*(bnd+ndx_ss)) || *(ss+ndx_ss) == bad);
	ndx_ss++);
    //
    // see if we can set up a leading queue
    // (mm is the number of data values that are: 
    //    1. inside the boundary AND 
    //    2. are not bad
    //   
    for(mm=0,jj=ndx_ss; mm < nn && jj < nc && *(bnd+jj); jj++) {
      if(*(ss+jj) != bad) mm++;
    }
    if(mm < nn) {
      ndx_ss = jj;
      printf("not enough data for leading queue: mm = %d\n", mm);
      continue;// not enough data to set up queue; move to next boundary
    }
    out_of_bounds = false;
    /*
     * initialize the leading average queue
     */
    qv0 = 0; // raq0->at;
    raq0_sum = raq1_sum = 0; // raq0->sum = raq1->sum = 0;
    for(ndx_q0=ndx_ss,mm=0;; ndx_q0++) {
      if((xx = *(ss+ndx_q0)) != bad) {
	if(!mm++)
	  continue; // don't use the first good gate in the avg
		   
	// put this val in the leading queue
	raq0_sum += xx;   // raq0->sum += xx;
	raq0[qv0] = xx;   // qv0->val = xx;
	qv0 = (qv0 + 1) % navg;	//  qv0 = qv0->next;  
	if(mm >= navg+1) {
          // print que
	  for (int i=0; i<navg; i++) {
	    printf("leading que[%d] = %f\n", i, raq0[i]);
	  }
	  break; // exit the loop; the que is full
	}
      }
    }
    // at this point ...
    // ndx_q0 = index in ray to last data point in leading que
    //
    // set the running average of the leading queue
    ref0 = raq0_sum * rcp_ngts; // ref0 = raq0->sum * rcp_ngts;
    printf("leading avg (ref0) = %f\n", ref0);
    qv1 = 0; // raq1->at;  // ??
    //
    // now loop through the data until we have encountered
    // navg good gates for the trailing average
    //
    for(mm=0; ndx_q0 < nc; ndx_ss++) {  // trailing-average-queue
      if((xx = *(ss+ndx_ss)) == bad)
	continue;
      if(abs((int)(xx -ref0)) > scaled_thr) {
	printf("leading avg: setting flag at %lu true; xx = %f; ref0 = %f\n", ndx_ss, xx, ref0);
	*(flag+ndx_ss) = true; // flag this gate
      }
      else {  // add this point to the trailing average
	raq1_sum += xx;
	raq1[qv1] = xx; // qv1->val = xx;
	qv1 = (qv1 + 1) % navg; // qv1 = qv1->next;
	if(++mm >= navg) {
	  break;  // breaks out of trailing-average-queue
	}
      }
      //
      // find the next good point for the leading average
      //
      for(ndx_q0++; ndx_q0 < nc; ndx_q0++) {
	if(!(*(bnd+ndx_q0))) { // we've gone beyond the boundary 
	  out_of_bounds = true;
	  break; 
	}
	if((xx = *(ss+ndx_q0)) != bad) {
	  raq0_sum -= raq0[qv0]; // qv0->val;
	  raq0_sum += xx;
	  raq0[qv0] = xx; // qv0->val = xx;
	  qv0 = (qv0 + 1) % navg; // qv0 = qv0->next;
	  ref0 = raq0_sum * rcp_ngts;
	  break;
	}
      }
      if(out_of_bounds || ndx_q0 >= nc)
	break;
    } // end set trailing average

    if(out_of_bounds || ndx_q0 >= nc) {
      ndx_ss = ndx_q0;
      continue;
    }
    ref1 = raq1_sum * rcp_ngts;

    //
    // else shift to a trailing average
    //

    for(ndx_ss++; ndx_ss < nc; ndx_ss++) {
      if(!(*(bnd+ndx_ss)))
	break;  // we've gone beyond the boundary
	      
      if((xx = *(ss+ndx_ss)) == bad)
	continue;

      if(abs((int)(xx -ref1)) > scaled_thr) {
	printf("trailing avg: setting flag at %lu true; ref1 = %f\n", ndx_ss, ref1);
	*(flag+ndx_ss) = true; // flag this gate
      }
      else { // add this point to the trailing average 
	raq1_sum -= raq1[qv1]; // qv1->val;
	raq1_sum += xx;
	raq1[qv1] = xx; // qv1->val = xx;
	qv1 = (qv1 + 1) % navg; // qv1 = qv1->next;
	ref1 = raq1_sum * rcp_ngts;
      }
    }
  }

  delete[] raq0;
  delete[] raq1;

  // return(1);
}
