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
/************************************************************************
 * ROUTE_WINDS_INIT
 *
 */
#define ROUTE_WINDS_INIT

#include "cidd.h"

#define NUM_PARSE_FIELDS    (MAX_ROUTE_SEGMENTS +2) * 3
#define PARSE_FIELD_SIZE    1024

/************************************************************************
 * route_winds_init:  Scan each route string and parse 
 */

void route_winds_init()
{
    int    i,k,index;
    int    num_fields;
    char   *cfield[NUM_PARSE_FIELDS];
    met_record_t *mr;

    // Allocate and FILL the U WINDS Met Record
    if(strlen(gd.layers.route_wind._P->u_url) > 1) {
        gd.layers.route_wind.u_wind = (met_record_t *) calloc(sizeof(met_record_t), 1);
	 if(gd.layers.route_wind.u_wind == NULL) {
	     fprintf(stderr,"Unable to allocate space for Route U Wind\n");
	     perror("CIDD route_winds_init");
	     exit(-1);
         }

	 mr = gd.layers.route_wind.u_wind;
         mr->h_data_valid = 0;
         mr->v_data_valid = 0;
         mr->v_vcm.nentries = 0;
         mr->h_vcm.nentries = 0;
         mr->h_fhdr.scale = -1.0;
         mr->h_last_scale = 0.0;
         STRcopy(mr->legend_name,"ROUTE_U_WIND",NAME_LENGTH);
         STRcopy(mr->button_name,"ROUTE_U_WIND",NAME_LENGTH);
         STRcopy(mr->url,gd.layers.route_wind._P->u_url,URL_LENGTH);

         STRcopy(mr->field_units,"unknown",LABEL_LENGTH);
         mr->currently_displayed = 1;
	 mr->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval;
         mr->h_fhdr.proj_origin_lon = 0.0;
         mr->h_fhdr.proj_origin_lat = 0.0;

	 // instantiate DsMdvxThreaded class
	 mr->v_mdvx = new DsMdvxThreaded;
	 mr->v_mdvx_int16 = new MdvxField;
    }

    // Allocate and FILL the V WINDS Met Record
    if(strlen(gd.layers.route_wind._P->v_url) > 1) {
        gd.layers.route_wind.v_wind = (met_record_t *) calloc(sizeof(met_record_t), 1);
	 if(gd.layers.route_wind.v_wind == NULL) {
	     fprintf(stderr,"Unable to allocate space for Route V Wind\n");
	     perror("CIDD route_winds_init");
	     exit(-1);
	 }

	 mr = gd.layers.route_wind.v_wind;
         mr->h_data_valid = 0;
         mr->v_data_valid = 0;
         mr->v_vcm.nentries = 0;
         mr->h_vcm.nentries = 0;
         mr->h_fhdr.scale = -1.0;
         mr->h_last_scale = 0.0;
         STRcopy(mr->legend_name,"ROUTE_V_WIND",NAME_LENGTH);
         STRcopy(mr->button_name,"ROUTE_V_WIND",NAME_LENGTH);
         STRcopy(mr->url,gd.layers.route_wind._P->v_url,URL_LENGTH);

         STRcopy(mr->field_units,"unknown",LABEL_LENGTH);
         mr->currently_displayed = 1;
	 mr->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval;
         mr->h_fhdr.proj_origin_lon = 0.0;
         mr->h_fhdr.proj_origin_lat = 0.0;

	 // instantiate DsMdvxThreaded class
	 mr->v_mdvx = new DsMdvxThreaded;
	 mr->v_mdvx_int16 = new MdvxField;
    }

    // Allocate and FILL the TURB Met Record
    if(strlen(gd.layers.route_wind._P->turb_url) > 1) {
        gd.layers.route_wind.turb = (met_record_t *) calloc(sizeof(met_record_t), 1);
	 if(gd.layers.route_wind.turb == NULL) {
	     fprintf(stderr,"Unable to allocate space for Route TURB\n");
	     perror("CIDD route_winds_init");
	     exit(-1);
	}

	 mr = gd.layers.route_wind.turb;
         mr->h_data_valid = 0;
         mr->v_data_valid = 0;
         mr->v_vcm.nentries = 0;
         mr->h_vcm.nentries = 0;
         mr->h_fhdr.scale = -1.0;
         mr->h_last_scale = 0.0;
         STRcopy(mr->legend_name,"ROUTE_TURB",NAME_LENGTH);
         STRcopy(mr->button_name,"ROUTE_TURB",NAME_LENGTH);
         STRcopy(mr->url,gd.layers.route_wind._P->turb_url,URL_LENGTH);

         STRcopy(mr->field_units,"unknown",LABEL_LENGTH);
         mr->currently_displayed = 1;
	 mr->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval;
         mr->h_fhdr.proj_origin_lon = 0.0;
         mr->h_fhdr.proj_origin_lat = 0.0;

	 // instantiate DsMdvxThreaded class
	 mr->v_mdvx = new DsMdvxThreaded;
	 mr->v_mdvx_int16 = new MdvxField;
    }

    // Allocate and FILL the ICING met Record
    if(strlen(gd.layers.route_wind._P->icing_url) > 1) {
        gd.layers.route_wind.icing = (met_record_t *) calloc(sizeof(met_record_t), 1);
	 if(gd.layers.route_wind.icing == NULL) {
	     fprintf(stderr,"Unable to allocate space for Route Icing\n");
	     perror("CIDD route_winds_init");
	     exit(-1);
	 }

	 mr = gd.layers.route_wind.icing;
         mr->h_data_valid = 0;
         mr->v_data_valid = 0;
         mr->v_vcm.nentries = 0;
         mr->h_vcm.nentries = 0;
         mr->h_fhdr.scale = -1.0;
         mr->h_last_scale = 0.0;
         STRcopy(mr->legend_name,"ROUTE_ICING",NAME_LENGTH);
         STRcopy(mr->button_name,"ROUTE_ICING",NAME_LENGTH);
         STRcopy(mr->url,gd.layers.route_wind._P->icing_url,URL_LENGTH);

         STRcopy(mr->field_units,"unknown",LABEL_LENGTH);
         mr->currently_displayed = 1;
	 mr->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval;
         mr->h_fhdr.proj_origin_lon = 0.0;
         mr->h_fhdr.proj_origin_lat = 0.0;

	 // instantiate DsMdvxThreaded class
	 mr->v_mdvx = new DsMdvxThreaded;
	 mr->v_mdvx_int16 = new MdvxField;
    }

    // How many are route are defined in the file
    gd.layers.route_wind.num_predef_routes = gd.layers.route_wind._P->route_paths_n;

    // Allocate space for num_predef_routes + 1 for the custom/user defined route
    if((gd.layers.route_wind.route =(route_track_t *) 
	  calloc(gd.layers.route_wind.num_predef_routes+1,sizeof(route_track_t))) == NULL) {

	 fprintf(stderr,"Unable to allocate space for %d Routes\n",
		 gd.layers.route_wind.num_predef_routes+1);
	 perror("CIDD route_winds_init");
	 exit(-1);
    }

    /* get temp space for string parsing */
    for(i = 0; i < NUM_PARSE_FIELDS; i++) {
        cfield[i] =(char *)  calloc(PARSE_FIELD_SIZE, 1);
    }


    for(i=0; i < gd.layers.route_wind.num_predef_routes; i++) {
        num_fields = STRparse(gd.layers.route_wind._P->_route_paths[i], cfield,
                              strlen(gd.layers.route_wind._P->_route_paths[i]),
                              NUM_PARSE_FIELDS, PARSE_FIELD_SIZE);
	if(num_fields == NUM_PARSE_FIELDS) {
	    fprintf(stderr,"Warning: Route path: %s\n Too long. Only %d segments allowed \n",
		   gd.layers.route_wind._P->_route_paths[i],MAX_ROUTE_SEGMENTS);
	    }

	    // Collect Label
	    strncpy(gd.layers.route_wind.route[i].route_label,cfield[0],64);

	    // Collect the number of points  & segments 
	    gd.layers.route_wind.route[i].num_segments = atoi(cfield[1]) -1;
	    if(gd.layers.route_wind._P->debug) {
		fprintf(stderr,"\nRoute: %s - %d segments\n",
		       gd.layers.route_wind.route[i].route_label,
		       gd.layers.route_wind.route[i].num_segments);
	    }

	    // Sanity check
	    if(gd.layers.route_wind.route[i].num_segments <= 0 || 
	       gd.layers.route_wind.route[i].num_segments >  MAX_ROUTE_SEGMENTS) {
	        fprintf(stderr,"Warning: Route path: %s\n Error Only 1-%d segments allowed \n",
		   gd.layers.route_wind._P->_route_paths[i],MAX_ROUTE_SEGMENTS);
		continue;
	    }

	    index = 2; // The first triplet.
	    // Pick up each triplet
	    for(k = 0; k <= gd.layers.route_wind.route[i].num_segments; k++, index+=3 ) {
		strncpy(gd.layers.route_wind.route[i].navaid_id[k],cfield[index],16);
		gd.layers.route_wind.route[i].y_world[k] = atof(cfield[index +1]);
		gd.layers.route_wind.route[i].x_world[k] = atof(cfield[index +2]);

		switch (gd.display_projection) {
		  case  Mdvx::PROJ_LATLON:
	            normalize_longitude(gd.h_win.min_x, gd.h_win.max_x, &gd.layers.route_wind.route[i].x_world[k]);
		  break;

		  default :
	            normalize_longitude(-180.0, 180.0, &gd.layers.route_wind.route[i].x_world[k]);
		  break;

		} 

	        gd.proj.latlon2xy(gd.layers.route_wind.route[i].y_world[k],
			      gd.layers.route_wind.route[i].x_world[k],
			      gd.layers.route_wind.route[i].x_world[k],
			      gd.layers.route_wind.route[i].y_world[k]);

	        if(gd.layers.route_wind._P->debug) {
		    fprintf(stderr,"%s:  %g,    %g\n",
			gd.layers.route_wind.route[i].navaid_id[k],
			gd.layers.route_wind.route[i].x_world[k],
			gd.layers.route_wind.route[i].y_world[k]);
	        }
	    }

	    // Compute the segment lengths
	    gd.layers.route_wind.route[i].total_length = 0.0;
	    for(k = 0; k < gd.layers.route_wind.route[i].num_segments; k++ ) {
		gd.layers.route_wind.route[i].seg_length[k] = 
		    disp_proj_dist(gd.layers.route_wind.route[i].x_world[k],
				   gd.layers.route_wind.route[i].y_world[k],
				   gd.layers.route_wind.route[i].x_world[k+1],
				   gd.layers.route_wind.route[i].y_world[k+1]);
		gd.layers.route_wind.route[i].total_length += gd.layers.route_wind.route[i].seg_length[k];
	    }
		
    }

    // Copy the initial route definition into the space reserved for the Custom route
    memcpy(gd.layers.route_wind.route + gd.layers.route_wind.num_predef_routes,
				 &gd.h_win.route,sizeof(route_track_t));

    /* free temp space */
    for(i = 0; i < NUM_PARSE_FIELDS; i++) {
            free(cfield[i]);
    }
}
