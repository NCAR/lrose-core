#include <Solo/GeneralDefinitions.hh>

/*
It the literature, the standard legacy way for converting reflectivity to rain rate is as follows:

  Z = a R^b

where

Z = pow(10.0, dbz / 10.0)
R = rain rate in mm/hr
a, b are coefficients.

The canonical ZR relationship is called Marshall Palmer: Z = 200 R ^ 1.6

You can see a bunch of examples at:

 https://www.weather.gov/ohrfc/ZRRelationships

From your point of view, to compute R from DBZ, you need:

  R = ((1/a) Z)^1/b

where Z is pow(10.0, dbz / 10.0) as above.

See also:

https://courses.comet.ucar.edu/pluginfile.php/3721/mod_imscp/content/1/reflectivityrainfall_relationship.html

Well, I'm not quite sure what that a = A = 200 in the code below.
I'm pretty sure that b = 1.4 ==> X = 1/b, in the code below

*/


//int se_mult_const(arg, cmds)	
//  int arg;
//  struct ui_command *cmds;	
//{
void se_rain_rate(float d_const, const float *data, float *newData, size_t nGates,
		  float bad, size_t dgi_clip_gate,
		  bool *boundary_mask)
{
   
  // #rain-rate#
  int ii, jj, fn, fns, fnd, size=0, mark, ns, nd;
  size_t nc;
  int dst_bad;
  // int rescale = NO;
  char *a=NULL, *b=NULL;
  float f_const;
  bool *bnd;
  const float *ss, *zz;
  float *tt;
  double d, rcp_scale, srs_bias, dst_scale, dst_bias;
  // double d_const;
  double xx;
  double A=.003333333, X=.7142857;

  //d_const = f_const = (cmdq++)->uc_v.us_v_float;

  nc = dgi_clip_gate;
  bnd = boundary_mask;

  ss = data; // (short *)dds->qdat_ptrs[fns];
  zz = ss +nc;
  tt = newData; // (short *)dds->qdat_ptrs[fnd];

  //bad = dds->parm[fns]->bad_data;
  //dst_bad = dds->parm[fnd]->bad_data;

  if(d_const) {
    X = d_const;
  }
  for(; ss < zz; ss++,tt++,bnd++) {
    if(*bnd) {
      if(*ss == bad) {
	*tt = bad; // dst_bad
      }
      else {
	d = *ss; // DD_UNSCALE((double)(*ss), rcp_scale, srs_bias);
	d = A * pow((double)10.0, (double)(0.1 * d * X));
	*tt = d; // DD_SCALE(d, dst_scale, dst_bias);
      }
    }
  }
}  


