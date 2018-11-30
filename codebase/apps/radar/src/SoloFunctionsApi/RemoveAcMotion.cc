
#include "GeneralDefinitions.hh"

/* c------------------------------------------------------------------------ */


//int se_remove_ac_motion(arg, cmds)      /* #remove-aircraft-motion# */
//  int arg;
//  struct ui_command *cmds;
//{
int  se_remove_ac_motion(float vert_velocity, float ew_velocity, float ns_velocity,
		     float ew_gndspd_corr, float tilt, float elevation,
		     short *data, // dds_qdata_ptrs[pn] or velocity data in this case 
		     short bad, float parameter_scale, float parameter_bias, int dgi_clip_gate,
		     short dds_radd_eff_unamb_vel,
		     int seds_nyquist_velocity)


   {

    /* remove the aircraft motion from velocities
     */
    //struct ui_command *cmdq=cmds+1; /* point to the first argument */
    int ii, nc, nn, mark, pn, sn;
    int scaled_nyqv, scaled_nyqi, fold_count, scaled_ac_vel, adjust;
    char *name;
    short *ss, *tt, *zz, *bnd, vx; // bnd is a boundary mask
    float nyqv, scale, bias, rcp_nyqi, ac_vel;
    double d;
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

    // ++++++++++++++++
    // printf("\n** FIXING ac_vel ** \n");
    // ac_vel = 55.77;
    // ================
    nc = dgi_clip_gate +1;
    //# ifdef NEW_ALLOC_SCHEME
    //ss = (short *)dds_qdat_ptrs[pn];
    //# else
    //ss = (short *)((char *)dds_rdat[pn] +sizeof(struct paramdata_d));
    //# endif
    ss = (short *) data;  // <<----- Here is the setup for the return of the results
    zz = ss +nc;
    //bad = dds_parm_pn_bad_data; // dds->parm[pn]->bad_data;
    scale = parameter_scale; // dds_parm_pn_parameter_scale; // dds->parm[pn]->parameter_scale;
    bias = parameter_bias; // dds_parm_pn_parameter_bias; // dds->parm[pn]->parameter_bias;
# ifdef obsolete
    nyqv = dds_radd_eff_unamb_vel;
# else
    nyqv = seds_nyquist_velocity ? seds_nyquist_velocity
          : dds_radd_eff_unamb_vel;
# endif
    scaled_nyqv = DD_SCALE(nyqv, scale, bias);
    scaled_nyqi = 2*scaled_nyqv;
    scaled_ac_vel = DD_SCALE(ac_vel, scale, bias);
    printf("scaled_ac_vel = %d\n", scaled_ac_vel);
    printf("scaled_nyqi = %d\n", scaled_nyqi);

    adjust = scaled_ac_vel % scaled_nyqi;

    if(abs(adjust) > scaled_nyqv) {
        adjust = adjust > 0 ? adjust-scaled_nyqi : adjust+scaled_nyqi;
    }

    printf("adjust = %d\n", adjust);


    // 
    //    for(; ss < zz; ss++,bnd++) {
    //    if(!(*bnd) || *ss == bad)
    //          continue;
    // assume there are no boundary constraints
    for(; ss < zz; ss++) {
        if(*ss == bad)
              continue;
        vx = *ss +adjust;
        if(abs(vx) > scaled_nyqv) {
	    printf("vx = %d, greater than nyquist, %d,  adjusting to ", vx, scaled_nyqi);
            vx = vx > 0 ? vx-scaled_nyqi : vx+scaled_nyqi;
	    printf("%d\n", vx);
        }
        *ss = vx;
    }

    return(1);
}
