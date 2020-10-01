
#include <Solo/GeneralDefinitions.hh>

/*
"    offset-for-radial-shear is <integer> gates",
"!  used by \"radial-shear\" command to create a field of the",
"!  differences between the current gate and the gate n gates ahead",

"!  Help file for the \"radial-shear\" command which has the form:",
" ",
"    radial-shear in <field> put-in <field>",
" ",
"!  Replace angle brackets and argument types with appropriate arguments.",
" ",
"!  If the \"put-in\" field does not exist it will be added using",
"!  the header information from the source field.",
" ",
"!  Other \"one-time\" commands that affect radial-shear are:",
" ",
"    offset-for-radial-shear is <integer> gates",
" ",
NOTE: ==> "!  if n is the number of gates, then the shear value is computed by",
          "!  shear = val[i+n] - val[i];",


OFFSET_FOR_RADIAL_SHEAR_IN_GATES = 5
VEL_SHEAR_5 = RADIAL_SHEAR(VEL, OFFSET_FOR_RADIAL_SHEAR_IN_GATES)
*/

/* c------------------------------------------------------------------------ */

// offset_in_gates = seds_gate_diff_interval

void se_radial_shear(size_t seds_gate_diff_interval, const float *data, float *newData, size_t nGates,
		     float bad_data_value, size_t dgi_clip_gate,
		     bool *boundary_mask)	// #radial-shear# 
{
    char *add_name, *src_name, *dst_name;
    int ii, jj, fn, fns, fnd, size=0, mark, ns, nd, nc;
    float bad;
    int ndx_ss, ndx_tt, ndx_xx;
    char *a=NULL, *b=NULL;
    bool *bnd;
    const float *dd;
    const float *ss, *zz;
    float *tt;

    bnd = boundary_mask;

    ss = data;
    tt = newData;

    if (seds_gate_diff_interval > nGates)
      seds_gate_diff_interval = nGates;

    if (dgi_clip_gate > nGates)
      dgi_clip_gate = nGates;

    dd = ss + seds_gate_diff_interval;
    zz = ss + dgi_clip_gate;
    bad = bad_data_value;

    //
    // loop through the data
    //

    for(; dd < zz;) {		// move inside the next boundary
	for(; dd < zz && !(*bnd); dd++,ss++,tt++,bnd++);
	if(dd == zz) break;
	//
	// see if we can calculate a shear
	//
	if(*dd == bad || *ss == bad) {
	    *tt = bad;
	}
	else {
	    *tt = *dd - *ss;
	}
	dd++; ss++; tt++; bnd++;
    }
}  
/* c------------------------------------------------------------------------ */

