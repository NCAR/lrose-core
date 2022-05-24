
#include <Solo/GeneralDefinitions.hh>

/* #remove-ring# */

/*
"    remove-ring in <field> from <real> to <real> km.",
" ",
"!  Replace angle brackets and argument types with appropriate arguments.",
"!  Inserts a bad data flag between the two indicated ranges.",
" ",
"!  e.g.",
"    remove-ring in VQ from 75.15 to 75.8 km.",
*/

// NOTE: map the from_km and to_km to a cell index in the ray                                    
// Q:Does this need to happend in the calling function? and send                                 
// the gate/cell index?  YES!
// parameters:
// from_km gate index of ring in data 
// to_km   gate index of ring in data 
void se_ring_zap(size_t from_km, size_t to_km, 
		 const float *data, float *newData, size_t nGates,
		 float bad, size_t dgi_clip_gate, bool *boundary_mask)
{
  // routine to remove a ring of data; e.g. a test pulse
  //
    float rr, r1, r2=1.e22;
    int ii, mark, gg;
    size_t g1, g2;
    size_t nc;
    int nd, fn, a_speckle;
    const float *ss, *zz;
    float *tt;
    bool *bnd;

    // TODO: move this to calling function ...
    //r1 = KM_TO_M(from_km); // get it to meters 
    //if(cmdq->uc_ctype == UTT_VALUE)
    //	  r2 = KM_TO_M(to_km);

    nc = dgi_clip_gate;
    if (dgi_clip_gate > nGates)
      nc = nGates;

    bnd = boundary_mask;
    //ss = data;
    //zz = ss +nc;

    // memcopy data into newData                          
    memcpy(newData, data, nGates*sizeof(float));

    tt = newData;

    // 
    // Q: What if ring crosses the clip gate? or is outside the clip_gate? 
    if (from_km > nc) from_km = nc;
    if (to_km > nc) to_km = nc;
    g1 = from_km; // dd_cell_num(dgi->dds, 0, r1);
    g2 = to_km;   // dd_cell_num(dgi->dds, 0, r2) +1;

    //ss += g1;
    bnd += g1;
    tt += g1;

    //
    // loop through the data
    //

    for(; g1++ < g2; bnd++, tt++) {  // ss++ need ss ??
	    if(!(*bnd)) {
	      continue;	
      }
	    *tt = bad; // *ss = bad;
    }
}
/* c------------------------------------------------------------------------ */

