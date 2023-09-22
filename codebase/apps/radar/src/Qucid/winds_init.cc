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
 * WINDS_INIT
 *
 */
#define WINDS_INIT

#include "cidd.h"

#define NUM_PARSE_FIELDS    32
#define PARSE_FIELD_SIZE    1024
#define INPUT_LINE_LEN      2048  

/************************************************************************
 * INIT_WIND_DATA_LINKS:  Scan cidd_wind_data.info file and setup link to
 *         data source for the wind fields.
 *
 */

void init_wind_data_links(const char *param_buf, long param_buf_len, long line_no)
{
    int    i;
    int    len,total_len;
    int    num_sets;    /* number of sets of wind data */
    int    num_fields;
    const char   *start_ptr;
    const char   *end_ptr;
    char   *cfield[NUM_PARSE_FIELDS];

    // initialize pointers to NULL

    for(i = 0; i < NUM_PARSE_FIELDS; i++) {
      cfield[i] = NULL;
    }

    /* PASS 1 - Count the wind set lines */
    num_sets = 0;
    total_len = 0;
    start_ptr = param_buf;
    while((end_ptr = strchr(start_ptr,'\n')) != NULL && 
			  (total_len < param_buf_len)) {
	 // Skip over blank, short or commented lines
	len = (end_ptr - start_ptr)+1;
	if( len > 15  && *start_ptr != '#') {
            num_sets++;
	}
	start_ptr = end_ptr +1; // Skip past the newline
	total_len += len  +1;
    }

   
    int default_marker_type = ARROWS;  
    if(num_sets > 0) {  /* ALLOCATE Space */
      /* get temp space for substrings */
      for(i = 0; i < NUM_PARSE_FIELDS; i++) {
         cfield[i] =(char *)  calloc(PARSE_FIELD_SIZE, 1);
      }

      if((gd.layers.wind =(wind_data_t*) calloc(num_sets,sizeof(wind_data_t))) == NULL) {
	 fprintf(stderr,"Unable to allocate space for %d wind sets\n",num_sets);
	 perror("Cidd");
	 exit(-1);
      }

      // Set up global barb preferences
      const char *type_ptr = gd.uparams->getString( "cidd.wind_marker_type", "arrow");

      if(strncasecmp(type_ptr, "tuft", 4) == 0)  default_marker_type = TUFT;
      if(strncasecmp(type_ptr, "barb", 4) == 0)  default_marker_type = BARB;
      if(strncasecmp(type_ptr, "vector", 6) == 0)  default_marker_type = VECTOR;
      if(strncasecmp(type_ptr, "tickvector", 10) == 0)  default_marker_type = TICKVECTOR; 
      if(strncasecmp(type_ptr, "labeledbarb", 11) == 0)  default_marker_type = LABELEDBARB;
      if(strncasecmp(type_ptr, "metbarb", 7) == 0)  default_marker_type = METBARB;
      if(strncasecmp(type_ptr, "barb_sh", 7) == 0)  default_marker_type = BARB_SH;
      if(strncasecmp(type_ptr, "labeledbarb_sh", 14) == 0)  default_marker_type = LABELEDBARB_SH;

      /* PASS 2 - fill in the params in the wind sets */
      num_sets = 0;

      total_len = 0;
      start_ptr = param_buf;
      while((end_ptr = strchr(start_ptr,'\n')) != NULL && 
			(total_len < param_buf_len)) {
	len = (end_ptr - start_ptr)+1; 
        // Skip over blank, short or commented lines
	if( len > 15  && *start_ptr != '#') {
	    num_fields = STRparse(start_ptr, cfield, len, NUM_PARSE_FIELDS, PARSE_FIELD_SIZE); 
            if( *start_ptr != '#' && num_fields >= 7) {

	       // Ask for 128 extra bytes for the null and potential env var  expansion
               gd.layers.wind[num_sets].data_info = (char *) calloc(len+128, 1);
	       if(gd.layers.wind[num_sets].data_info == NULL) {
	          fprintf(stderr,"Unable to allocate %d bytes for wind info\n",len+128);
	          perror("Cidd");
	          exit(-1);
	       }
               STRcopy(gd.layers.wind[num_sets].data_info,start_ptr,len);

	       /* DO Environment variable substitution */
		usubstitute_env(gd.layers.wind[num_sets].data_info, len+128);
               num_sets++;
            }
        }
	start_ptr = end_ptr +1; // Skip past the newline
	total_len += len  +1;
      }
    }

    gd.layers.num_wind_sets = num_sets;

    for(i=0; i < gd.layers.num_wind_sets; i++) {
        num_fields = STRparse(gd.layers.wind[i].data_info, cfield,
                              INPUT_LINE_LEN,
                              NUM_PARSE_FIELDS, PARSE_FIELD_SIZE);
        if(num_fields < 7) {
            fprintf(stderr,
                    "Error in wind field line. Wrong number of parameters,  -Line: \n %s"
                    ,gd.layers.wind[i].data_info);
        }

        /* Allocate Space for U record and initialize */
        gd.layers.wind[i].wind_u = (met_record_t *)
            calloc(sizeof(met_record_t), 1);
        gd.layers.wind[i].wind_u->h_data_valid = 0;
        gd.layers.wind[i].wind_u->v_data_valid = 0;
        gd.layers.wind[i].wind_u->h_vcm.nentries = 0;
        gd.layers.wind[i].wind_u->v_vcm.nentries = 0;
        gd.layers.wind[i].wind_u->h_fhdr.scale = -1.0;
        gd.layers.wind[i].wind_u->h_last_scale = 0.0;
        gd.layers.wind[i].wind_u->time_list.num_alloc_entries = 0;
        gd.layers.wind[i].wind_u->time_list.num_entries = 0;
        STRcopy(gd.layers.wind[i].wind_u->legend_name,cfield[0],NAME_LENGTH);
        STRcopy(gd.layers.wind[i].wind_u->button_name,cfield[0],NAME_LENGTH);
        STRcopy(gd.layers.wind[i].wind_u->url,cfield[1],URL_LENGTH);

	    if(gd.html_mode == 0) { /* Replace Underscores with spaces in names */
          for(int j=strlen(gd.layers.wind[i].wind_u->button_name)-1 ; j >= 0; j--) {
            if(gd.replace_underscores && gd.layers.wind[i].wind_u->button_name[j] == '_') gd.layers.wind[i].wind_u->button_name[j] = ' ';
            if(gd.replace_underscores && gd.layers.wind[i].wind_u->legend_name[j] == '_') gd.layers.wind[i].wind_u->legend_name[j] = ' ';
          }
	    }

	// Append the field name
	strcat(gd.layers.wind[i].wind_u->url,cfield[2]);

        STRcopy(gd.layers.wind[i].wind_u->field_units,cfield[5],LABEL_LENGTH);
        gd.layers.wind[i].wind_u->currently_displayed = atoi(cfield[6]);
	    gd.layers.wind[i].active = (atoi(cfield[6]) > 0)? 1: 0;
        gd.layers.wind[i].line_width = abs(atoi(cfield[6]));
        // Sanity check
        if(gd.layers.wind[i].line_width == 0 ||
           gd.layers.wind[i].line_width > 10) gd.layers.wind[i].line_width = 1;
          
        // Pick out Optional Marker type fields
        gd.layers.wind[i].marker_type = default_marker_type;
        if(strstr(cfield[6], ",tuft") != NULL)  gd.layers.wind[i].marker_type = TUFT;
        if(strstr(cfield[6], ",barb") != NULL)  gd.layers.wind[i].marker_type = BARB;
        if(strstr(cfield[6], ",vector") != NULL)  gd.layers.wind[i].marker_type = VECTOR;
        if(strstr(cfield[6], ",tickvector") != NULL)  gd.layers.wind[i].marker_type = TICKVECTOR; 
        if(strstr(cfield[6], ",labeledbarb") != NULL)  gd.layers.wind[i].marker_type = LABELEDBARB;
        if(strstr(cfield[6], ",metbarb") != NULL)  gd.layers.wind[i].marker_type = METBARB;
        if(strstr(cfield[6], ",barb_sh") != NULL)  gd.layers.wind[i].marker_type = BARB_SH;
        if(strstr(cfield[6], ",labeledbarb_sh") != NULL)  gd.layers.wind[i].marker_type = LABELEDBARB_SH;


        // Pick out Optional Color
        if(num_fields > 7) {
	    STRcopy(gd.layers.wind[i].color_name, cfield[7], NAME_LENGTH);
	} else {
	    STRcopy(gd.layers.wind[i].color_name, "white", NAME_LENGTH);
	}
	gd.layers.wind[i].wind_u->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval;
        gd.layers.wind[i].wind_u->h_fhdr.proj_origin_lon = 0.0;
        gd.layers.wind[i].wind_u->h_fhdr.proj_origin_lat = 0.0;

        // instantiate classes
        gd.layers.wind[i].wind_u->h_mdvx = new DsMdvxThreaded;
        gd.layers.wind[i].wind_u->v_mdvx = new DsMdvxThreaded;
        gd.layers.wind[i].wind_u->h_mdvx_int16 = new MdvxField;
        gd.layers.wind[i].wind_u->v_mdvx_int16 = new MdvxField;
	gd.layers.wind[i].wind_u->proj = new MdvxProj;

        /* Allocate Space for V record and initialize */
        gd.layers.wind[i].wind_v = (met_record_t *)
            calloc(sizeof(met_record_t), 1);
        gd.layers.wind[i].wind_v->h_data_valid = 0;
        gd.layers.wind[i].wind_v->v_data_valid = 0;
        gd.layers.wind[i].wind_v->h_vcm.nentries = 0;
        gd.layers.wind[i].wind_v->v_vcm.nentries = 0;
        gd.layers.wind[i].wind_v->h_fhdr.scale = -1.0;
        gd.layers.wind[i].wind_v->h_last_scale = 0.0;
        gd.layers.wind[i].wind_v->time_list.num_alloc_entries = 0;
        gd.layers.wind[i].wind_v->time_list.num_entries = 0;
        STRcopy(gd.layers.wind[i].wind_v->legend_name,cfield[0],NAME_LENGTH);
        STRcopy(gd.layers.wind[i].wind_v->button_name,cfield[0],NAME_LENGTH);
        STRcopy(gd.layers.wind[i].wind_v->url,cfield[1],URL_LENGTH);


	    if(gd.html_mode == 0) { /* Replace Underscores with spaces in names */
          for(int j=strlen(gd.layers.wind[i].wind_v->button_name)-1 ; j >= 0; j--) {
            if(gd.replace_underscores && gd.layers.wind[i].wind_v->button_name[j] == '_') gd.layers.wind[i].wind_v->button_name[j] = ' ';
            if(gd.replace_underscores && gd.layers.wind[i].wind_v->legend_name[j] == '_') gd.layers.wind[i].wind_v->legend_name[j] = ' ';
          }
	    }
	// Append the field name
	strcat(gd.layers.wind[i].wind_v->url,cfield[3]);

        STRcopy(gd.layers.wind[i].wind_v->field_units,cfield[5],LABEL_LENGTH);
        gd.layers.wind[i].wind_v->currently_displayed = atoi(cfield[6]);
	gd.layers.wind[i].wind_v->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval;
        gd.layers.wind[i].wind_v->h_fhdr.proj_origin_lon = 0.0;
        gd.layers.wind[i].wind_v->h_fhdr.proj_origin_lat = 0.0;

        // instantiate classes
        gd.layers.wind[i].wind_v->h_mdvx = new DsMdvxThreaded();
        gd.layers.wind[i].wind_v->v_mdvx = new DsMdvxThreaded();
        gd.layers.wind[i].wind_v->h_mdvx_int16 = new MdvxField;
        gd.layers.wind[i].wind_v->v_mdvx_int16 = new MdvxField;
	gd.layers.wind[i].wind_v->proj = new MdvxProj;

        /* Allocate Space for W  record (If necessary)  and initialize */
        if(strncasecmp(cfield[4],"None",4) != 0) {
            gd.layers.wind[i].wind_w = (met_record_t *) calloc(sizeof(met_record_t), 1);
            gd.layers.wind[i].wind_w->h_data_valid = 0;
            gd.layers.wind[i].wind_w->v_data_valid = 0;
            gd.layers.wind[i].wind_w->v_vcm.nentries = 0;
            gd.layers.wind[i].wind_w->h_vcm.nentries = 0;
            gd.layers.wind[i].wind_w->h_fhdr.scale = -1.0;
            gd.layers.wind[i].wind_w->h_last_scale = 0.0;
            gd.layers.wind[i].wind_w->time_list.num_alloc_entries = 0;
            gd.layers.wind[i].wind_w->time_list.num_entries = 0;

            STRcopy(gd.layers.wind[i].wind_w->legend_name,cfield[0],NAME_LENGTH);
            sprintf(gd.layers.wind[i].wind_w->button_name,"%s_W ",cfield[0]);
            STRcopy(gd.layers.wind[i].wind_w->url,cfield[1],URL_LENGTH);

	        if(gd.html_mode == 0) { /* Replace Underscores with spaces in names */
              for(int j=strlen(gd.layers.wind[i].wind_w->button_name)-1 ; j >= 0; j--) {
                if(gd.replace_underscores && gd.layers.wind[i].wind_w->button_name[j] == '_') gd.layers.wind[i].wind_w->button_name[j] = ' ';
                if(gd.layers.wind[i].wind_w->legend_name[j] == '_') gd.layers.wind[i].wind_w->legend_name[j] = ' ';
              }
	    }

	    // Append the field name
	    strcat(gd.layers.wind[i].wind_w->url,cfield[4]);

            STRcopy(gd.layers.wind[i].wind_w->field_units,cfield[5],LABEL_LENGTH);
            gd.layers.wind[i].wind_w->currently_displayed = atoi(cfield[6]);
	    gd.layers.wind[i].wind_w->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval;
            gd.layers.wind[i].wind_w->h_fhdr.proj_origin_lon = 0.0;
            gd.layers.wind[i].wind_w->h_fhdr.proj_origin_lat = 0.0;

	    // instantiate classes
	    gd.layers.wind[i].wind_w->h_mdvx = new DsMdvxThreaded();
	    gd.layers.wind[i].wind_w->v_mdvx = new DsMdvxThreaded();
	    gd.layers.wind[i].wind_w->h_mdvx_int16 = new MdvxField;
	    gd.layers.wind[i].wind_w->v_mdvx_int16 = new MdvxField;
	    gd.layers.wind[i].wind_w->proj = new MdvxProj;
        } else {
            gd.layers.wind[i].wind_w =  (met_record_t *) NULL;
        }

	gd.layers.wind[i].units_scale_factor = gd.uparams->getDouble( "cidd.wind_units_scale_factor", 1.0);
	gd.layers.wind[i].reference_speed = gd.uparams->getDouble( "cidd.wind_reference_speed", 10.0);

	gd.layers.wind[i].units_label = gd.uparams->getString( "cidd.wind_units_label", "m/sec");


    }

    /* free temp space */
    for(i = 0; i < NUM_PARSE_FIELDS; i++) {
      if (cfield[i] != NULL) {
	free(cfield[i]);
      }
    }

}
