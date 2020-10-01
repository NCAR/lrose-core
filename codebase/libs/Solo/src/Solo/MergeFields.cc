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
   * #merge-fields#
   */

  size_t nc;
  bool *bnd;
  const float *ss, *zz, *aa;
  float *tt;

  nc = dgi_clip_gate;
  bnd = boundary_mask;

  aa = data1; //(short *)dds->qdat_ptrs[fn];

  ss = data2; // (short *)dds->qdat_ptrs[fns];

  zz = ss +nc;  // set the end point

  tt = newData; // (short *)dds->qdat_ptrs[fnd];

  /*
   * loop through the data
   */

  /*
   * in this case the first field "add_name" represented by "aa"
   * is considered the dominant field over the second field
   * "src_name" represented by "ss" and if (*aa) does not contain
   * a bad flag it is plunked into the destination field "tt"
   * otherwise (*ss) is plunked into the destination field
   */
  for(; ss < zz; aa++,ss++,tt++,bnd++) {
    if(*bnd) {

      if(*aa != bad) {
	*tt = *aa;
      }
      else {
	*tt = *ss;
      } 
    } // end bnd 
  } // end for 
}  



/* 
      if(rescale) {		// nightmare time! 
	if(*aa != bad) {
	  x = DD_UNSCALE((float)(*aa), aa_rcp_scale, aa_bias);
	  *tt = DD_SCALE(x, dst_scale, dst_bias);
	} // *aa ok 
	else if(*ss != bad) {
	  y = DD_UNSCALE((float)(*ss), rcp_scale, srs_bias);
	  *tt = DD_SCALE(y, dst_scale, dst_bias);
	}	// *ss ok 
	else {
	  *tt = bad;
	}
} // rescale 
*/
