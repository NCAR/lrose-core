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
 * INIT_DATA_LINKS:  Scan cidd_grid_data.info file and establish links to data
 *         sources.
 */

#define DATA_INIT 1

#include "cidd.h"

#define NUM_PARSE_FIELDS    32
#define PARSE_FIELD_SIZE    1024
#define INPUT_LINE_LEN      2048

void init_data_links(const char *param_buf, long param_buf_len, long line_no,
                     Params &params)
{
    int  i,j;
    int  len,total_len;
    const char *start_ptr;
    const char *end_ptr;
    char *cfield[NUM_PARSE_FIELDS];

    gd.num_datafields = 0;
    total_len = 0;
    start_ptr = param_buf;

    // read all the lines in the data information buffer
    while((end_ptr = strchr(start_ptr,'\n')) != NULL && (total_len < param_buf_len)) {
	// Skip over blank, short or commented lines
	len = (end_ptr - start_ptr)+1;
	if( (len < 20)  || (*start_ptr == '#')) {
	    total_len += len  +1;
	    start_ptr = end_ptr +1; // Skip past the newline
	    line_no++;
	    continue;
	}

	if(gd.num_datafields < MAX_DATA_FIELDS -1) {
	  // Ask for 128 extra bytes for the null and potential env var  expansion
          gd.data_info[gd.num_datafields] = (char *)  calloc(len+128, 1);
          STRcopy(gd.data_info[gd.num_datafields],start_ptr,len);

	  /* Do Environment variable substitution */
	  usubstitute_env(gd.data_info[gd.num_datafields], len+128);
          gd.num_datafields++;
	} else {
		fprintf(stderr,
		  "Cidd: Warning. Too many Data Fields. Data field not processed\n");
		fprintf(stderr,"Line %ld \n",line_no);
	 
        }

	total_len += len + 1;   // Count characters processed 
	start_ptr = end_ptr +1; // Skip past the newline
	line_no++;
    }

    if(gd.num_datafields <=0) {
	 fprintf(stderr,"CIDD requires at least one valid gridded data field to be defined\n");
	 exit(-1);
    }

    /* get temp space for substrings */
    for(i = 0; i < NUM_PARSE_FIELDS; i++) {
        cfield[i] = (char *)  calloc(PARSE_FIELD_SIZE, 1);
    }

    /* scan through each of the data information lines */
    for(i = 0; i < gd.num_datafields; i++) {

        /* get space for data info */
        gd.mrec[i] =  (met_record_t *) calloc(sizeof(met_record_t), 1);

        /* separate into substrings */
        STRparse(gd.data_info[i], cfield, INPUT_LINE_LEN, NUM_PARSE_FIELDS, PARSE_FIELD_SIZE);
        STRcopy(gd.mrec[i]->legend_name,cfield[0],NAME_LENGTH);
        STRcopy(gd.mrec[i]->button_name,cfield[1],NAME_LENGTH);

	if(gd.html_mode == 0) {
          /* Replace Underscores with spaces in names */
          for(j=strlen(gd.mrec[i]->button_name)-1 ; j >= 0; j--) {
            if(gd.replace_underscores && gd.mrec[i]->button_name[j] == '_') gd.mrec[i]->button_name[j] = ' ';
            if(gd.replace_underscores && gd.mrec[i]->legend_name[j] == '_') gd.mrec[i]->legend_name[j] = ' ';
          }
	}
        STRcopy(gd.mrec[i]->url,cfield[2],URL_LENGTH);

        STRcopy(gd.mrec[i]->color_file,cfield[3],NAME_LENGTH);

	// if units are "" or --, set to zero-length string
	if (!strcmp(cfield[4], "\"\"") || !strcmp(cfield[4], "--")) {
	  STRcopy(gd.mrec[i]->field_units,"",LABEL_LENGTH);
	} else {
	  STRcopy(gd.mrec[i]->field_units,cfield[4],LABEL_LENGTH);
	}

        gd.mrec[i]->cont_low = atof(cfield[5]);
        gd.mrec[i]->cont_high = atof(cfield[6]);
        gd.mrec[i]->cont_interv = atof(cfield[7]);

        gd.mrec[i]->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval;

        if (strncasecmp(cfield[8],"rad",3) == 0) {
            gd.mrec[i]->render_method = POLYGONS;
        } else {
            gd.mrec[i]->render_method = POLYGONS;
        }

        if (strncasecmp(cfield[8],"cont",4) == 0) {
            gd.mrec[i]->render_method = FILLED_CONTOURS;
        }

        if (strncasecmp(cfield[8],"lcont",4) == 0) {
            gd.mrec[i]->render_method = LINE_CONTOURS;
        }

        if (strncasecmp(cfield[8],"dcont",4) == 0) {
            gd.mrec[i]->render_method = DYNAMIC_CONTOURS;
        }

        if (strstr(cfield[8],"comp") != NULL) {
            gd.mrec[i]->composite_mode = TRUE;
        }

        if (strstr(cfield[8],"autoscale") != NULL) {
            gd.mrec[i]->auto_scale = TRUE;
        }

        gd.mrec[i]->currently_displayed = atoi(cfield[9]);

		if(gd.run_once_and_exit) {
		  gd.mrec[i]->auto_render = 1;
	    } else {
		  gd.mrec[i]->auto_render = atoi(cfield[10]);
		}

        gd.mrec[i]->last_elev = (char *)NULL;
        gd.mrec[i]->elev_size = 0;

        gd.mrec[i]->plane = 0;
        gd.mrec[i]->h_data_valid = 0;
        gd.mrec[i]->v_data_valid = 0;
        gd.mrec[i]->h_last_scale  = -1.0;
        gd.mrec[i]->h_last_bias  = -1.0;
        gd.mrec[i]->h_last_missing  = -1.0;
        gd.mrec[i]->h_last_bad  = -1.0;
	gd.mrec[i]->h_last_transform  = -1;
        gd.mrec[i]->v_last_scale  = -1.0;
        gd.mrec[i]->v_last_bias  = -1.0;
        gd.mrec[i]->v_last_missing  = -1.0;
        gd.mrec[i]->v_last_bad  = -1.0;
        gd.mrec[i]->v_last_transform  = -1;
        gd.mrec[i]->h_fhdr.proj_origin_lat  = 0.0;
        gd.mrec[i]->h_fhdr.proj_origin_lon  = 0.0;
        gd.mrec[i]->time_list.num_alloc_entries = 0;
        gd.mrec[i]->time_list.num_entries = 0;

        STRcopy(gd.mrec[i]->units_label_cols,"KM",LABEL_LENGTH);
        STRcopy(gd.mrec[i]->units_label_rows,"KM",LABEL_LENGTH);
        STRcopy(gd.mrec[i]->units_label_sects,"KM",LABEL_LENGTH);

	// instantiate classes
	gd.mrec[i]->h_mdvx = new DsMdvxThreaded;
	gd.mrec[i]->v_mdvx = new DsMdvxThreaded;
	gd.mrec[i]->h_mdvx_int16 = new MdvxField;
	gd.mrec[i]->v_mdvx_int16 = new MdvxField;
	gd.mrec[i]->proj = new MdvxProj;

    }
    /* Make sure the first field is always on */
    gd.mrec[0]->currently_displayed = 1;
    
    /* free up temp storage for substrings */
    for(i = 0; i < NUM_PARSE_FIELDS; i++) {
        free(cfield[i]);
    }

}
