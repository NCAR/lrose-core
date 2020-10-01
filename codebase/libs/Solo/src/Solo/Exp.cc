
#include <Solo/GeneralDefinitions.hh>

  // #exponentiate#

void se_mult_const(float f_const, const float *data, float *newData, size_t nGates,
		   float bad, size_t dgi_clip_gate,
		   bool *boundary_mask)
{

  //int ii, jj, fn, fns, fnd, size=0, mark, ns, nd, nc, bad, dst_bad;

  size_t nc;
  bool *bnd;
  const float *ss, *zz;
  float *tt;
  double d, d_const;

  nc = dgi_clip_gate;
  bnd = boundary_mask;

  ss = data;
  zz = ss +nc;

  // memcopy data into newData
  memcpy(newData, data, nGates*sizeof(float));
  tt = newData;

  for(; ss < zz; ss++,tt++,bnd++) {
    if(*bnd) {
      if(*ss == bad) {
	continue; // *tt = bad;
      }
      else {
	//d = DD_UNSCALE((double)(*ss), rcp_scale, srs_bias);
	d = *ss;
	d = pow(d, d_const);
	*tt = d; //DD_SCALE(d, dst_scale, dst_bias);
      }
    }
  }

}  


