
#include "GeneralDefinitions.hh"


/* c------------------------------------------------------------------------ */

//double
//dd_ac_vel(dgi)
//  DGI_PTR dgi;


double dd_ac_vel(float vert_velocity, float ew_velocity, float ns_velocity, float ew_gndspd_corr,
		 float tilt, float elevation)
{
    double vert, d, ac_vel;

    vert =  vert_velocity != -999 ? vert_velocity : 0;
    d = sqrt((double)(SQ(ew_velocity) + SQ(ns_velocity)));
    d += ew_gndspd_corr; // klooge to correct ground speed
    ac_vel = d*sin(tilt) + vert*sin(elevation);
    return(ac_vel);


  /*  original code ...

    DDS_PTR dds=dgi_dds;
    struct radar_angles *ra=dds_ra;
    struct platform_i *asib=dds_asib;

    double vert, d, ac_vel;

    vert =  asib_vert_velocity != -999 ? asib_vert_velocity : 0;
    d = sqrt((double)(SQ(asib_ew_velocity) + SQ(asib_ns_velocity)));
    d += dds_cfac_ew_gndspd_corr; // klooge to correct ground speed
    ac_vel = d*sin(ra_tilt) + vert*sin(ra_elevation);
    return(ac_vel);
*/


}
