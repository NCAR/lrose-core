
#include <Solo/GeneralDefinitions.hh>

/*
 * 
 * se_add_const
 * se_add_fields
 * se_mult_const
 * 
 * 
 */


/* c------------------------------------------------------------------------ */

/* #absolute-value# */

void se_absolute_value(const float *data, float *newData, size_t nGates,
		       float bad, size_t dgi_clip_gate, bool *boundary_mask) 
{
    size_t  nc;
    const float *ss, *zz;
    float *tt;
    bool *bnd;

    nc = dgi_clip_gate;
    bnd = boundary_mask;
    ss = data;
    zz = ss +nc;

    // memcopy data into newData
    memcpy(newData, data, nGates*sizeof(float));
    tt = newData;

    for(; ss < zz; ss++, tt++, bnd++) {
	if(!(*bnd) || *ss == bad)
	      continue;
	*tt = abs(*ss);
    }
}  
