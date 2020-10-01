#include <Solo/GeneralDefinitions.hh>

/*
"    fix-vortex-velocities in <field>",
"!  Replace angle brackets and argument types with appropriate arguments.",
"!  Works only if the fields VS and VL are present. Operation replaces the",
"!    original vield VR by recalculating the final dual prt velocity",
"!    and eliminating some noisy gates.",
"  ",
"!  e.g.",
"    fix-vortex-velocities in VG",
*/

/* #fix-vortex-velocities# */

void se_fix_vortex_vels(const float *data, float *newData, size_t nGates,
			const float *vs_data, const float *vl_data,
			float vs_xmitted_freq, 
			float vs_interpulse_time, float vl_interpulse_time,
			float bad, size_t dgi_clip_gate, bool *boundary_mask) 
{
    /* routine to remove discountinuities (freckles) from the data
     * such as birds and radio towers by comparing a particular
     * data point to a running average that is ahead of the point of interest
     * but switches to a trailing average once enough good points
     * have been encountered
     */
  // TODO: how to handle these static values?
    static int vConsts[16], *ctr, level_bias;
    static double rcp_half_nyqL, d_round=.5;
    int fn, fnd, ii, jj, kk, mm, mark, idiff, vL, vS, levels;
    size_t nc;
    double d, wvl, freq, prfL, prfS, ratioXY;
    double av_nyqL, av_nyqS;
    char *name, *dst_name;
    char *vl="VL", *vs="VS";
    int fn_vl, fn_vs;
    float *fptr, *ipptr, X, Y, scale, bias, rcp_scale;

    const float *ss, *zz;
    float *tt;
    const float  *vl_ptr, *vs_ptr; // , *dst;
    bool *bnd;
    /*
     * boundary mask is set to 1 if the corresponding cell satisfies
     * conditions of the boundaries
     */
    bnd = boundary_mask;
    nc = dgi_clip_gate;

    ss = data;
    zz = ss +nc;

    // memcopy data into newData
    memcpy(newData, data, nGates*sizeof(float));
    tt = newData;

    //    if((fn_vl = dd_find_field(dgi, vl)) < 0) {
    //uii_printf("Vortex velocity field: %s not found\n", name);
    //seds->punt = YES;
    //return(-1);
    //}
    //# ifdef NEW_ALLOC_SCHEME
    vl_ptr = vl_data; // (short *)dds->qdat_ptrs[fn_vl];
    //# else
    //vl_ptr = (short *)((char *)dds->rdat[fn_vl] +sizeof(struct paramdata_d));
    //# endif

      //    if((fn_vs = dd_find_field(dgi, vs)) < 0) {
    //	uii_printf("Vortex velocity field: %s not found\n", name);
    //	seds->punt = YES;
    //	return(-1);
    //}
    //# ifdef NEW_ALLOC_SCHEME
    vs_ptr = vs_data; //(short *)dds->qdat_ptrs[fn_vs];
    //# else
    //    vs_ptr = (short *)((char *)dds->rdat[fn_vs] +sizeof(struct paramdata_d));
    //# endif

    // bad = dds->parm[fn]->bad_data;

    if(seds->process_ray_count == 1) {
	/*
	 * set up constants
	 * first calculate the average frequency
	 */
	//parm = dds->parm[fn_vs];
	//scale = dds->parm[fn_vs]->parameter_scale;
	//bias = dds->parm[fn_vs]->parameter_bias;
	/*
	 * this assumes the scale and the bias are the same for
	 * all three fields
	 */
      fptr = &radd->freq1;   // HERE <=====

	for(d=0, ii=0, kk=1; ii < 5; ii++, kk<<=1) {
	  if(kk & vs_xmitted_freq) { // parm->xmitted_freq) {
		d += *(fptr +ii);
	    }
	}
	/* frequency is in Ghz */
	wvl = SPEED_OF_LIGHT/((d/(double)(radd->num_freq_trans)) * 1.e9);
	/*
	 * get the PRFs
	 */
	for(ii=0, kk=1; ii < 5; ii++, kk<<=1) {
	  if(kk & vs_interpulse_time) { // parm->interpulse_time) {
		break;
	    }
	}
	/* prfS is the greater prf
	 * the interpulse period is in milliseconds
	 */
	ipptr = &radd->interpulse_per1;
	prfS = 1./(*(ipptr + ii) *.001);
	// parm = dds->parm[fn_vl];

	for(ii=0, kk=1; ii < 5; ii++, kk<<=1) {
	  if(kk & vl_interpulse_time) { // parm->interpulse_time) {
		break;
	    }
	}
	prfL = 1./(*(ipptr + ii) *.001);

	X = prfS/(prfS - prfL);
	Y = prfL/(prfS - prfL);
	ratioXY = prfS/prfL;
	levels = (int)(X + Y + .1);
	level_bias = levels/2;
	av_nyqS = .25 * wvl * prfS;
	av_nyqL = .25 * wvl * prfL;
	rcp_half_nyqL = 1./DD_SCALE((.5 * av_nyqL), scale, bias);
	ctr = &vConsts[level_bias];
	*ctr = 0;

	for(ii=1; ii <= level_bias; ii++) {
	    switch(ii) {
		/* these cases represent (vL - vS) normalized
		 * by half the nyquist velocity
		 */
	    case 1:		/*  */
		d = DD_SCALE(av_nyqL * (1. + ratioXY), scale, bias);
		*(ctr - ii) = d;
		*(ctr + ii) = -d;
		break;

	    case 2:
		d = DD_SCALE(2. * av_nyqL * (1. + ratioXY), scale, bias);
		*(ctr - ii) = d;
		*(ctr + ii) = -d;
		break;

	    case 3:
		d = DD_SCALE(av_nyqL * (2. + ratioXY), scale, bias);
		*(ctr - ii) = -d;
		*(ctr + ii) = d;
		break;

	    case 4:
		d = DD_SCALE(av_nyqL, scale, bias);
		*(ctr - ii) = -d;
		*(ctr + ii) = d;
		break;
	    }
	}
	/* vS corresponds to V1 in some of the documentation
	 * and vL corresponds to V2
	 */
	rcp_scale = 1./scale;
	d = -2. * av_nyqL;

	for(ii=0; ii < levels; ii++) {
	    uii_printf("dual prt const: %6.2f for (vs-vl) = %8.4f\n"
		      , DD_UNSCALE(vConsts[ii], rcp_scale, bias)
		      , d);
	    d += .5 * av_nyqL;
	}
    }


    for(; ss < zz; ss++,tt++,bnd++,vl_ptr++,vs_ptr++) {
	if(!bnd)
	      continue;
	vS = *vs_ptr;
	vL = *vl_ptr;
	kk = d = ((double)vS - (double)vL) * rcp_half_nyqL + level_bias
	      + d_round;
	*tt = ((vS + vL) >> 1) + vConsts[kk];
    }

}


