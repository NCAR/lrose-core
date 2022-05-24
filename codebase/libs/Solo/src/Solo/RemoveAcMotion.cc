
#include <Solo/GeneralDefinitions.hh>
#include "AcVel.cc"

/* c------------------------------------------------------------------------ */


//int se_remove_ac_motion(arg, cmds)      /* #remove-aircraft-motion# */
//  int arg;
//  struct ui_command *cmds;
//{
//
// on 32-bit machine, same as on 64-bit machine
// short  2 bytes  integer
// int    4 bytes  
// float  4 bytes 
// double 8 bytes
 // dds_qdata_ptrs[pn] or velocity data in this case 

// All data are coming in scaled and biased
void se_remove_ac_motion(float vert_velocity, float ew_velocity, float ns_velocity,
			 float ew_gndspd_corr, float tilt, float elevation,
			 const float *data, float *newData, size_t nGates,
			 float bad, size_t dgi_clip_gate,
			 float dds_radd_eff_unamb_vel,
			 float seds_nyquist_velocity, bool *bnd)
{

    /* remove the aircraft motion from velocities
     */
    //struct ui_command *cmdq=cmds+1; /* point to the first argument */
    // int ii, nc, nn, mark, pn, sn;
    size_t nc, ssIdx;
    int scaled_nyqv, scaled_nyqi, scaled_ac_vel, adjust;
    // char *name;
    // short *ss, *zz, vx; // bnd is a boundary mask
    float vx; 
    float nyqv, nyqi, ac_vel;
    // double d;
    //struct dd_general_info *dgi, *dd_window_dgi();
    //struct dds_structs *dds;
    //struct solo_edit_stuff *seds, *return_sed_stuff();

    /*
    seds = return_sed_stuff();  // solo editing struct 

    if(seds->finish_up) {
        return(1);
    }
    seds->modified = YES;
    name = (cmdq++)->uc_text;
    sn = strlen(name);
    bnd = (short *) seds->boundary_mask; // set the boundary mask; detects inside or outside boundary
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;

    if((pn = dd_find_field(dgi, name)) < 0) {
        uii_printf("Source parameter %s not found for copy\n", name);
        seds->punt = YES;
        return(-1);
    }
    */
    //ac_vel = dd_ac_vel(dgi);
    ac_vel = dd_ac_vel(vert_velocity, ew_velocity, ns_velocity, 
		       ew_gndspd_corr, tilt, elevation);

    if (dgi_clip_gate > nGates) {
      // throw std::invalid_argument("dgi_clip_gate greater than number of gates");
      nc = nGates;
    } else {
      nc = dgi_clip_gate;
    }
    //bnd = boundary_mask;

    // memcopy data into newData                                                                  
    memcpy(newData, data, nGates*sizeof(float));

    size_t zzIdx = nGates;
    if (dgi_clip_gate < nGates)
      zzIdx = dgi_clip_gate;


    // ========

    // nc = dgi_clip_gate +1;
    //# ifdef NEW_ALLOC_SCHEME
    //ss = (short *)dds_qdat_ptrs[pn];
    //# else
    //ss = (short *)((char *)dds_rdat[pn] +sizeof(struct paramdata_d));
    //# endif
    //ss = (short *) data;  // <<----- Here is the setup for the return of the results
    //zz = ss +nc;
    //bad = dds_parm_pn_bad_data; // dds->parm[pn]->bad_data;
    //scale = parameter_scale; // dds_parm_pn_parameter_scale; // dds->parm[pn]->parameter_scale;
    //bias = parameter_bias; // dds_parm_pn_parameter_bias; // dds->parm[pn]->parameter_bias;
    //# ifdef obsolete
    //    nyqv = dds_radd_eff_unamb_vel;
    //# else
    // TODO: ?? if Nyquist velocity set in editor use it? else use unambiguous velocity from the data file?
    if (seds_nyquist_velocity != 0.0)
      nyqv = seds_nyquist_velocity;
    else 
      nyqv = dds_radd_eff_unamb_vel;
    // nyqv = seds_nyquist_velocity ? seds_nyquist_velocity
    //      : dds_radd_eff_unamb_vel;
    //# endif
    // NOTE: DD_SCALE adds 0.5 and converts to an integer 
    //scaled_nyqv = DD_SCALE(nyqv, scale, bias);
    //scaled_nyqi = 2*scaled_nyqv;
    //scaled_ac_vel = DD_SCALE(ac_vel, scale, bias);

    scaled_nyqv = DD_SCALE(nyqv);
    scaled_nyqi = 2*scaled_nyqv;
    scaled_ac_vel = DD_SCALE(ac_vel);
    //printf("scaled_ac_vel = %d\n", scaled_ac_vel);
    //printf("scaled_nyqi = %d\n", scaled_nyqi);
    
    adjust = scaled_ac_vel % scaled_nyqi;

    if(abs(adjust) > scaled_nyqv) {
      if (adjust > 0) 
        adjust = adjust - scaled_nyqi;
      else 
        adjust = adjust + scaled_nyqi;
    }

    // printf("adjust = %d\n", adjust);
    
    // TODO: make delta a variable or parameter and make it consistent
    //       maybe make it a function, then we can easily make it consistent.
    ssIdx = 0;
    while (ssIdx < zzIdx) {
      // if in boundary and not bad
      bool bad_data = abs((data[ssIdx] - bad)) < 0.0001;
      if (bnd[ssIdx] && !bad_data) {
        vx = data[ssIdx] + adjust;
        // printf("abs(%f) = %f\n", vx, abs(vx));
        if(abs(vx) > scaled_nyqv) {
	        //printf("vx = %f, greater than nyquist, %f,  adjusting to ", vx, nyqi);
            if (vx > 0) {
              vx = vx - scaled_nyqi;
	          } else {
              vx = vx + scaled_nyqi;
            }
	          // printf("%f\n", vx);
        }
        newData[ssIdx] = vx;
      }
      ssIdx += 1;
    }

    /* original code 
    for(; ss < zz; ss++,bnd++) {
        if(!(*bnd) || *ss == bad)
              continue;
	//    for(; ss < zz; ss++) {
        //if(*ss == bad)
        //      continue;
        vx = *ss +adjust;
        if(abs(vx) > scaled_nyqv) {
	    printf("vx = %d, greater than nyquist, %d,  adjusting to ", vx, scaled_nyqi);
            vx = vx > 0 ? vx-scaled_nyqi : vx+scaled_nyqi;
	    printf("%d\n", vx);
        }
        *ss = vx;
    }
    */
}
