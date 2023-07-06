
#include <Solo/GeneralDefinitions.hh>
#include <string.h>
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

/*
"    threshold <dst_field> on <thr_field> <below>|<above>|<between> <real>",
"!  Replace angle brackets and argument types with appropriate arguments.",
"  ",
"!  Two example commands are:",
"    threshold VT on NCP below 0.333",
"    threshold VT on SW above 5.0",
*/

/* #threshold# */

// parameters:
// first_good_gate 
//
void se_threshold_field(Where where, float scaled_thr1, float scaled_thr2,
			int first_good_gate,
			const float *data, const float *thr_data, size_t nGates,
			float *newData,
			float bad, float thr_bad, size_t dgi_clip_gate,
			bool *boundary_mask, const bool *bad_flag_mask) 
{

  size_t nc;
  int fgg;
  int gg, ii, jj, kk, nn;
  //  scaled_thr1, mark, fthr, scaled_thr2;
  const float *ss, *zz, *thr;
  float *tt;
  bool *bnd;

  if (dgi_clip_gate > nGates) {
    nc = nGates;
  } else {
    nc = dgi_clip_gate;
  }

  fgg = first_good_gate;
  bnd = boundary_mask;
  //
  //  find the thr field
  //
  thr = thr_data; // (short *)dds->qdat_ptrs[fthr];
  // allow for different bad value for threshold field?
  //thr_bad = dds->parm[fthr]->bad_data;
    
  // find the field to be thresholded
    
  ss = data; // (short *)dds->qdat_ptrs[fn];
  zz = ss +nc;

  // memcopy data into newData
  memcpy(newData, data, nGates*sizeof(float));
  tt = newData;

  //
  // loop through the data
  //
  for(gg=0;  gg < fgg && ss < zz;  *tt = bad, gg++,ss++,thr++,tt++,bnd++);

  switch(where) {
  case BELOW:
    for(; ss < zz; ss++,thr++,tt++,bnd++) {
      if(!(*bnd) || *ss == bad)
	continue;
      if(*thr == thr_bad || *thr < scaled_thr1) {
	*tt = bad;
      }
    }
    break;
  case ABOVE:
    for(; ss < zz; ss++,thr++,tt++,bnd++) {
      if(!(*bnd) || *ss == bad)
	continue;
      if(*thr == thr_bad || *thr > scaled_thr1) {
	*tt = bad;
      }
    }
    break;
  default:
    // between 
    //if(cmdq->uc_ctype == UTT_VALUE) { // TODO: not sure what this does
    for(; ss < zz; ss++,thr++,tt++,bnd++) {
      if(!(*bnd) || *ss == bad)
	continue;
      if(*thr == thr_bad ||
	 (*thr >= scaled_thr1 && *thr <= scaled_thr2)) {
	*tt = bad;
      }
    }
  }
}

/* c------------------------------------------------------------------------ */

//int se_hard_zap(arg, cmds)              /* #unconditional-delete# */
//  int arg;
//  struct ui_command *cmds;

 void se_hard_zap(
      const float *data, size_t nGates,
      float *newData,
      float bad, size_t dgi_clip_gate,
      bool *boundary_mask) 
{
    //int below;
    //struct ui_command *cmdq=cmds+1; /* point to the first argument */
    //struct solo_edit_stuff *seds, *return_sed_stuff();
    //struct dd_general_info *dgi, *dd_window_dgi();
    //struct dds_structs *dds;
    //char *dst_name;

    size_t nc;
    //int nd, fn;
    //float bad;
    //int ii, nn, mark;
    //const float  *ss;
    float *zz;
    float *tt;
    bool *bnd;

    if (dgi_clip_gate > nGates) {
      nc = nGates;
    } else {
      nc = dgi_clip_gate;
    }

    //fgg = first_good_gate;
    bnd = boundary_mask;    


    //dst_name = (cmdq++)->uc_text;
    //nd = strlen(dst_name);

    //if(!seds->boundary_exists) {
    //    return(1);
    //}

    //seds->modified = YES;
    //dgi = dd_window_dgi(seds->se_frame); dds = dgi->dds;
    //nc = dgi->clip_gate+1;
    //bnd = seds->boundary_mask;
    /*
     * find the field to be thresholded
     */
//    if((fn = dd_find_field(dgi, dst_name)) < 0) {
        /* field not found
         */
//      g_string_sprintfa
//        (gs_complaints, "Field to be deleted: %s not found\n", dst_name);
//        seds->punt = YES;
//        return(-1);
//    }
//# ifdef NEW_ALLOC_SCHEME
//    ss = (short *)dds->qdat_ptrs[fn];
//# else
//    ss = (short *)((char *)dds->rdat[fn] + sizeof(struct paramdata_d));
//# endif
    //ss = data;
    //zz = ss +nc;
    //bad = dds->parm[fn]->bad_data;
    memcpy(newData, data, nGates*sizeof(float));
    tt = newData;
    zz = tt +nc;
    /*
     * loop through the data
     */

    for(; tt < zz; tt++, bnd++) {
      if(*bnd)
        *tt = bad;
    }
    //return(fn);
} 

void se_unconditional_delete(const float *data, float *newData, size_t nGates,
         float bad, size_t dgi_clip_gate, bool *boundary_mask) {
  se_hard_zap(data, nGates, newData, bad, dgi_clip_gate, boundary_mask);
}

void se_assign_value(const float *data, float *newData, size_t nGates,
         float value, size_t dgi_clip_gate, bool *boundary_mask) {
  se_hard_zap(data, nGates, newData, value, dgi_clip_gate, boundary_mask);
}

/* c------------------------------------------------------------------------ */

  
