
#include <Solo/GeneralDefinitions.hh>

/* c------------------------------------------------------------------------ */
				/* flagged-assign */

void se_flagged_assign_value(float constant, const float *data, float *newData, size_t nGates,
		    size_t dgi_clip_gate,
		    bool *boundary_mask, const bool *bad_flag_mask)
{
    int ii, jj, fn, fns, fnd, size=0, mark, ns, nd, nc, bad, scaled_const;
    bool *bnd;
    const bool *flag;
    const float *ss, *zz;
    float  *tt;

    if (dgi_clip_gate > nGates) {
      nc = nGates;
    } else {
      nc = dgi_clip_gate;
    }
    bnd = boundary_mask;
    flag = bad_flag_mask;

    ss = data;
    scaled_const = DD_SCALE(constant);
    zz = ss + nc;
    //zz = ss + dgi->dds->celv->number_cells;
    // memcopy data into newData                                                            
    memcpy(newData, data, nGates*sizeof(float));
    tt = newData;

    for(; ss < zz; ss++,tt++,bnd++,flag++) {
	    if(*bnd && *flag) {
        *tt = scaled_const;
	    }
    }
}  
