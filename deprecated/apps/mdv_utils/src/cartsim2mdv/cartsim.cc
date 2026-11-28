// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/*********************************************************************
 * cartsim.cc
 *
 * Wrapper for cart_sim library calls. 
 *
 * Dave Albo
 *
 *********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

# include <sim/cart_sim.h>
# include <sim/mdv_sim.h>
# include <ctetwws/smem.h>
# include "loc_cart_sim.h"
# include "cartsim_le.h"

static CART_SIM_parms_t P;
static CART_SIM_grid_parms_t G;
static CART_SIM_location_t L;
static unsigned char *cbuf;
static int Polar = 0;

/*----------------------------------------------------------------*/
/*
 * Transform to/from radar/math
 */
static double polar_angle(double angle)
{
    double yy;
    yy = 90.0 - angle;
    while (yy < 0.0)
	yy += 360.0;
    while (yy >= 360.0)
	yy -= 360.0;
    return yy;
}

/*----------------------------------------------------------------*/
/*
 * Transform to/from radar/math and convert to radians
 */
static double polar_theta(double angle)
{
    double yy;
    
    /*
     * Get angle (true north) for the angle value (MDV uses radar angle)
     */
    yy = polar_angle(angle);
    yy = RADIANS(yy);
    return yy;
}

/*----------------------------------------------------------------*/
static unsigned char
compute_1_value(double theta, double phi, double x, double y, double z,
		int i, int j, int k, e_loc_sim_fields_t e)
{
    double v;
    CART_SIM_radar_data_t D;

    /*
     * Figure out where
     */
    if (Polar == 1)
    {
	L.loc.x = cos(theta)*cos(phi)*x;
	L.loc.y = sin(theta)*cos(phi)*x;
	L.loc.z = sin(phi)*x;
	/*printf("r=%lf  translate to location x,y,z=%lf,%lf,%lf\n",
	  x, L.loc.x, L.loc.y, L.loc.z);*/
    }
    else
    {
	L.loc.x = x;
	L.loc.y = y;
	L.loc.z = z;
    }

    /*
     * Compute the value
     */
    CART_SIM_value(&P, &L, &D);
    /*printf("%vel=(5.2lf %5.2lf %5.2lf)\n", D.vel.x,
      D.vel.y, D.vel.z);*/

    /*
     * Scale it based on type
     */
    switch (e)
    {
    case LOC_SIM_FIELD_VEL_X:
	v = D.vel.x;
	break;
    case LOC_SIM_FIELD_VEL_Y:
	v = D.vel.y;
	break;
    case LOC_SIM_FIELD_VEL_Z:
	v = D.vel.z;
	break;
    case LOC_SIM_FIELD_REF:
	v = D.ref;
	break;
    case LOC_SIM_FIELD_SNR:
	v = D.snr;
	break;
    case LOC_SIM_FIELD_SW:
	v = D.sw;
	break;
    case LOC_SIM_FIELD_RADIAL_VEL:
	/*
	 * Here compute the radial velocity
	 */
	v = CART_SIM_xyz_component_in_direction(&D.vel, &L.loc);
	break;
    default:
	printf("ERROR in logic\n");
	v = CART_SIM_BAD_VALUE;
    }

    return (MDV_SIM_fields_mdv_data_value(e, v, CART_SIM_BAD_VALUE,
					  i, j, k));
}

/*----------------------------------------------------------------*/
/*
 * Initialize the Cartesian simulation module.
 */
void CARTSIM_init(char *progname, char *confname, char *gridname,
		  int doprint)
{
    CARTSIM_LE_init(progname);
    CART_SIM_init(confname, &P, (doprint?CART_SIM_TRUE:CART_SIM_FALSE));
    CART_SIM_grid_init(gridname, &G, (doprint?CART_SIM_TRUE:CART_SIM_FALSE));
}

/*----------------------------------------------------------------*/
/*
 * Create the parameters for the input values and return.
 */
void CARTSIM_get_params(int t, int delta_t, int vert, int proj,
			int nfields, char **fields,
			MDV_SIM_inputs_t *in, CART_SIM_grid_parms_t *Gout)
{
    int i;

    /*
     * Set time to start time of simulation plus t
     */
    L.time = P.time + t;

    /*
     * Set other time values
     */
    in->time = L.time;
    in->beg_time = L.time;
    in->end_time = L.time + delta_t;

    in->n_fields = nfields;
    in->fields = MEM_CALLOC(nfields, e_loc_sim_fields_t);
    for (i=0; i<nfields; ++i)
    {
	in->fields[i] = MDV_SIM_fields_local_name_match(fields[i]);
	if (in->fields[i] == LOC_SIM_FIELD_UNKNOWN)
	{
	    printf("Bad field in param file '%s'\n", fields[i]);
	    exit(-1);
	}
	if (MDV_SIM_fields_is_raw_radar_field(in->fields[i]) == 0)
	{
	    printf("Non-radar field in param file '%s'\n", fields[i]);
	    exit(-1);
	}
    }
    in->nx = G.x.num;
    in->ny = G.y.num;
    in->nz = G.z.num;

    sprintf(in->note, "Cartesian simulated data");
    sprintf(in->file_name, "Cartesian simulation file");

    *Gout = G;

    /*
     * Figure out whether its polar or cartesian data
     */
    if (proj == MDV_PROJ_POLAR_RADAR)
    {
	if (vert == MDV_VERT_TYPE_ELEV || vert == MDV_VERT_VARIABLE_ELEV)
	{
	    /*printf("POLAR DATA\n");*/
	    Polar = 1;
	}
	else
	{
	    printf("ERROR proj = polar radar but not vert type elev %d\n",
		   vert);
	    exit(0);
	}
    }
    else
    {
	/*printf("CARTESIAN DATA\n");*/
	Polar = 0;
    }
}

/*----------------------------------------------------------------*/
/*
 * Return pointer to plane of data
 */
unsigned char *
CARTSIM_get_plane(e_loc_sim_fields_t e, int vlevel)
{
    int i, j;
    double x, y, z, theta, phi;
    unsigned char *c;

    /*
     * Get the data for the plane indicated
     */
    if (vlevel < 0 || vlevel >= G.z.num)
    {
	printf("ERROR requested invalid vert level %d in CARTSIM_get_plane\n",
	       vlevel);
	return NULL;
    }
    /*
     * Go to center of z box
     */
    z = G.z.p0 + vlevel*G.z.delta + G.z.delta/2.0;
    if (Polar == 1)
    {
	phi = RADIANS(z);
	/*printf("getting plane for z=%lf\n", z);*/
    }
    else
        phi = 0.0;
    
    cbuf = (unsigned char *)malloc(G.x.num*G.y.num);

    /*
     * 0,0 point is at lower lefthand corner, and we increase x fastest.
     * Go to center point of each box.
     */
    c = cbuf;
    for (y=G.y.p0+G.y.delta/2.0,j=0; j<G.y.num; y+=G.y.delta,++j)
    {
	if (Polar == 1)
	{
	    theta = polar_theta(y);
	    /*printf("Getting beam for theta=%lf\n", y);*/
	}
	else
	  theta = 0.0;
	
	for (x=G.x.p0+G.x.delta/2.0, i=0; i<G.x.num; x+=G.x.delta, ++i, ++c)
	    *c = compute_1_value(theta, phi, x, y, z, i, j, vlevel, e);
    }
    return cbuf;
}


#ifdef __cplusplus
}
#endif


