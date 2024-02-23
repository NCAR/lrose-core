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
/**********************************************************************
 * COLORSCALES.C : Routines to read color table files and establish
 *     colormaps
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */


#define COLORSCALES 
#include "cidd.h"
#include <toolsa/str.h>
#include <string>
#include <vector>
#include <map>

#define NUM_PARSE_FIELDS    8
#define PARSE_FIELD_SIZE    1024
#define INPUT_LINE_LEN      1024

typedef vector<Val_color_t> CvalVector;
typedef pair<string, CvalVector> CvalPair;
typedef map<string, CvalVector> CvalMap;
typedef CvalMap::iterator CvapMapIt;
static CvalMap cvalMap;

/**********************************************************************
 * GET_COLOR_MAPPING: A routine to read color table files.
 *        This routine allocates memory for the entries.
 *        Returns number of entries found, -1 on error
 */

static int _get_color_mapping(const char *color_file_subdir,
                              const char *fname,         /* file name */
                              Val_color_t *cval[]) /* RETURN -  pointer to array of structs */
{

  cerr << "aaaaaaaaaaaa color_file_subdir: " << color_file_subdir << endl;
  cerr << "aaaaaaaaaaaa fname: " << fname << endl;

  FILE   *cfile;
    struct stat sbuf;
    char   *cs_buf;
    char   buf[2048];
    char   *str_ptr;
    char   *cfield[NUM_PARSE_FIELDS];
    int    i,j;
    int    cs_len;
    int    ret_stat;
    int    nstrings;
    int    nentries;
    char   *ptr;
    char   *lptr;
    char   *lasts;
    char    dirname[1024];
    char    name_buf[2048];
    CvalVector cvalVector;

    // first check our map to see if we have already read this file

    CvapMapIt it = cvalMap.find(fname);
    if (it != cvalMap.end()) {
      // colormap file previously read
      const CvalVector &cvalVec = it->second;
      for (size_t ii = 0; ii < cvalVec.size(); ii++) {
        // Get space for this entry
        cval[ii] = (Val_color_t *) calloc(1, sizeof(Val_color_t));
        // copy element
        *cval[ii] = cvalVec[ii];
      }
      nentries = cvalVec.size();
      if(gd.debug) {
        fprintf(stderr,"Reusing colorscale file: %s\n", fname);
        fprintf(stderr,"  nentries: %d\n", nentries);
      }
      return nentries;
    }
    
    cs_len = 0;
    cfile = NULL;
    // Try the local dir first
    if((cfile = fopen(fname,"r")) == NULL) {

	STRcopy(name_buf,color_file_subdir,2048);

	str_ptr = strtok(name_buf,","); // Prime strtok

	do {  // Check each directory in the comma delimited  list

	  while(*str_ptr == ' ') str_ptr++; //skip any leading spaces

	  sprintf(buf,"%s/%s",str_ptr,fname);

	  // Check if its an HTTP URL
	  if(strncasecmp(buf,"http:",5) == 0) {
            if(strlen(_params.http_proxy_url)  > URL_MIN_SIZE) {
	      ret_stat = HTTPgetURL_via_proxy(_params.http_proxy_url,buf,
                            _params.data_timeout_secs * 1000, &cs_buf, &cs_len);
            } else {
	      ret_stat =  HTTPgetURL(buf,_params.data_timeout_secs * 1000, &cs_buf, &cs_len);
            }

	    if(ret_stat <= 0) {
		cs_len = 0;
	    } else {
	      if(gd.debug) fprintf(stderr,"Loading Colorscale %s\n",buf);
	    }


          } else {
	    // Try to open it in the subdir 
	    if((cfile = fopen(buf,"r")) == NULL) {
	      cs_len = 0;
	    } else {
	      if(gd.debug) fprintf(stderr,"Opening Colorscale %s\n",buf);
	    }
	  }
        } while (cfile == NULL && (str_ptr = strtok(NULL,",")) != NULL && cs_len == 0 );

    } else {
	sprintf(buf,"%s",fname);
    }

    if(cfile !=NULL) {
       if(stat(buf,&sbuf) < 0) { // Find the file's size
             fprintf(stderr,"Can't stat %s\n",buf);
	     return -1;
	 }

	 // Allocate space for the whole file plus a null
	 if((cs_buf = (char *)  calloc(sbuf.st_size + 1 ,1)) == NULL) {
	     fprintf(stderr,"Problems allocating %ld bytes for colorscale file\n",
		     (long) sbuf.st_size);
	     return -1;
	}

	// Read
	if((cs_len = fread(cs_buf,1,sbuf.st_size,cfile)) != sbuf.st_size) {
	   fprintf(stderr,"Problems Reading color map: %s\n",buf);
	   return -1;
	}
	cs_buf[sbuf.st_size] = '\0'; // Make sure to null terminate
        fclose(cfile);
    }

    if(cs_len <= 0) {
			errno = 0;
			if(getcwd(dirname,1024) == NULL) {
				perror("Dirname");
			}
            fprintf(stderr,"Couldn't load %s/%s or %s/%s\n",
				 dirname,fname,color_file_subdir,fname);
            fprintf(stderr,"Please install %s and try again\n",fname);
            return -1;
    }

    /* Get temp storage for character strings */
    for(i=0;i < NUM_PARSE_FIELDS; i++) {
        cfield[i] = (char *)  calloc(1,PARSE_FIELD_SIZE);
    }

    nentries = 0;

    // Prime strtok;
    str_ptr = strtok_r(cs_buf,"\n",&lasts);

    /* loop thru buffer looking for valid entries */
    while ((str_ptr  != NULL) && (nentries < MAX_COLORS)) {    
        if(*str_ptr != '#') {
         ptr = strpbrk(str_ptr,"!\n");    /* look for trailing exclamation point */
	 if(ptr != NULL) {   /* replace trailing exclamation point with null  */
	     *ptr = '\0';
             lptr = ptr + 1; /* Set the label pointer */
             /* replace offensive newline in label with null */
             ptr = strpbrk(lptr,"\n");
             if(ptr) *ptr = '\0';
	 } else {
	    lptr = NULL;
	 }

         if((nstrings = STRparse(str_ptr, cfield, INPUT_LINE_LEN, NUM_PARSE_FIELDS, PARSE_FIELD_SIZE)) >= 3) {
            /* Is (hopefully)  a valid entry */
            /* Get space for this entry */
            cval[nentries] = (Val_color_t *) calloc(1,sizeof(Val_color_t));

            /* Set the label for this color scale element */
            if((lptr != NULL) && strlen(lptr) > 0) {
               STRcopy(cval[nentries]->label,lptr,LABEL_LENGTH);
            } else {
              cval[nentries]->label[0] = '\0';
            }

            cval[nentries]->min = atof(cfield[0]);        /* Extract mapping values */
            cval[nentries]->max = atof(cfield[1]);    

            cval[nentries]->cname[0] = '\0';
            for(j=2;j < nstrings; j++) {    /* Some names contain multiple strings so concatenate */
                strcat(cval[nentries]->cname,cfield[j]);
                strcat(cval[nentries]->cname," ");    
            }

            cval[nentries]->cname[strlen(cval[nentries]->cname)-1] = '\0'; /* chop off last space char */
            cvalVector.push_back(*cval[nentries]);
            nentries++;
          }
        }

	//Move to the next token
	str_ptr = strtok_r(NULL,"\n",&lasts);

    }

    /* free temp storage for character strings */
    for (i = 0; i < NUM_PARSE_FIELDS; i++) {
        free(cfield[i]);
    }

    if(cs_len >0 ) free(cs_buf);

    if(nentries <= 0) {
        fprintf(stderr,"No color map entries found in %s",fname);
        return -1;
    }

    if(gd.debug) {
      fprintf(stderr,"Successfully read colorscale file: %s\n", fname);
      fprintf(stderr,"  nentries: %d\n", nentries);
    }
    CvalPair pr(fname, cvalVector);
    cvalMap.insert(pr);

    return nentries;
}

/**********************************************************************
 * COMBINE_COLOR_MAPS: Allocates colors and cells (in pseudocolor mode )
 * for color mapping based on   total number of unique colors used in
 * all the color scale files, plus  a    short list of colors used
 * for overlay drawing.  Fills in the global color array: gd.color[] 
 */

static int _combine_color_maps(Display *dpy, Colormap cmap)
{
    int    i,j,k;
    int    status;
    int    found;
    int    nentries;        /* number of color entries in a color scale */
    int    v_class;
    unsigned long    mask; /*  */
    unsigned long    pix_vals[MAX_COLORS];
    double    mult;
    const char    *color_file_subdir;
    const char    *color_name;
    char    str_buf[128];

    XColor    rgb_def;
    XColor    rgb_exact;
    XGCValues gcvalues;    


    color_file_subdir = _params.color_scales_url;
     

    gd.num_colors = 0;
    gd.num_draw_colors = 0;

   /* Start with colors for Background, Foreground */
   /* NOTE: IT is Assumed later that color 0 is the background! */
   color_name = _params.background_color;
   STRcopy(gd.color[gd.num_colors].name,color_name,NAME_LENGTH);
   gd.legends.background_color = &(gd.color[gd.num_colors]);
   gd.num_colors++;

   // Use Background color as the default for the Margins
   color_name = _params.margin_color;
   STRcopy(gd.color[gd.num_colors].name,color_name,NAME_LENGTH);
   gd.legends.margin_color = &(gd.color[gd.num_colors]);
   gd.num_colors++;

   color_name = _params.foreground_color;
   STRcopy(gd.color[gd.num_colors].name,color_name,NAME_LENGTH);
   gd.legends.foreground_color = &(gd.color[gd.num_colors]);
   gd.num_colors++;

   color_name = _params.out_of_range_color;
   if(!strstr(color_name,"transparent") && !_params.transparent_images) {
     STRcopy(gd.color[gd.num_colors].name,color_name,NAME_LENGTH);
     gd.legends.out_of_range_color = &(gd.color[gd.num_colors]);
     gd.num_colors++;
   } else { // Is rendered as transparent
     gd.legends.out_of_range_color = &gd.null_color;
   }

   // Add Range ring & azmiuth overlay color
   color_name = _params.range_ring_color;
   STRcopy(gd.color[gd.num_colors].name,color_name,NAME_LENGTH);
   gd.legends.range_ring_color = &(gd.color[gd.num_colors]);
   gd.num_colors++;

   // Add flight route path - Cross section reference line color
   color_name = _params.route_path_color;
   STRcopy(gd.color[gd.num_colors].name,color_name,NAME_LENGTH);
   gd.legends.route_path_color = &(gd.color[gd.num_colors]);
   gd.num_colors++;

   // Add Time Axis color
   color_name = _params.time_axis_color;
   STRcopy(gd.color[gd.num_colors].name,color_name,NAME_LENGTH);
   gd.legends.time_axis_color = &(gd.color[gd.num_colors]);
   gd.num_colors++;

   // Add Movie frame limit indicator color
   color_name = _params.time_frame_color;
   STRcopy(gd.color[gd.num_colors].name,color_name,NAME_LENGTH);
   gd.legends.time_frame_color = &(gd.color[gd.num_colors]);
   gd.num_colors++;

   // Add epoch indicator color
   color_name = _params.epoch_indicator_color;
   STRcopy(gd.color[gd.num_colors].name,color_name,NAME_LENGTH);
   gd.legends.epoch_indicator_color = &(gd.color[gd.num_colors]);
   gd.num_colors++;

   // Add Color of the Now Line
   color_name = _params.now_time_color;
   STRcopy(gd.color[gd.num_colors].name,color_name,NAME_LENGTH);
   gd.legends.now_time_color = &(gd.color[gd.num_colors]);
   gd.num_colors++;

   // Add  colors for time ticks
   for(i=0; i < NUM_TICK_COLORS; i++) {  
       sprintf(str_buf,"cidd.time_tick_color%d",i+1);
       color_name = _params._time_tick_colors[i];
       STRcopy(gd.color[gd.num_colors].name,color_name,NAME_LENGTH);
       gd.legends.time_tick_color[i] = &(gd.color[gd.num_colors]);
       gd.num_colors++;
    }

   // Add Height selector axis and text color
   color_name = _params.height_axis_color;
   STRcopy(gd.color[gd.num_colors].name,color_name,NAME_LENGTH);
   gd.legends.height_axis_color = &(gd.color[gd.num_colors]);
   gd.num_colors++;

   // Add Current Height Indicator color
   color_name = _params.height_indicator_color;
   STRcopy(gd.color[gd.num_colors].name,color_name,NAME_LENGTH);
   gd.legends.height_indicator_color = &(gd.color[gd.num_colors]);
   gd.num_colors++;

   // Add color for mark for latest click location
   color_name = _params.latest_click_mark_color;
   STRcopy(gd.color[gd.num_colors].name,color_name,NAME_LENGTH);
   gd.legends.latest_click_mark_color = &(gd.color[gd.num_colors]);
   gd.num_colors++;

   // Add color for mark for latest client location
   color_name = _params.latest_client_mark_color;
   STRcopy(gd.color[gd.num_colors].name,color_name,NAME_LENGTH);
   gd.legends.latest_client_mark_color = &(gd.color[gd.num_colors]);
   gd.num_colors++;

   // Add colors for each Layer of contours
   for(i=0; i < NUM_CONT_LAYERS; i++) {
       STRcopy(gd.color[gd.num_colors].name,gd.layers.cont[i].color_name,NAME_LENGTH);
       gd.layers.cont[i].color = &(gd.color[gd.num_colors]);
       gd.num_colors++;
   }

   // Add missing_data_color 
   color_name = _params.missing_data_color;
   if(!strstr(color_name,"transparent") && !_params.transparent_images) {
     STRcopy(gd.color[gd.num_colors].name,color_name,NAME_LENGTH);
     gd.layers.missing_data_color = &(gd.color[gd.num_colors]);
     gd.num_colors++;
   } else {// Is rendered as transparent
     gd.layers.missing_data_color = &gd.null_color;
   }

   // Add bad_data_colo 
   color_name = _params.bad_data_color;
   if(!strstr(color_name,"transparent") && !_params.transparent_images) {
     STRcopy(gd.color[gd.num_colors].name,color_name,NAME_LENGTH);
     gd.layers.bad_data_color = &(gd.color[gd.num_colors]);
     gd.num_colors++;
   } else { // Is rendered as transparent
     gd.layers.bad_data_color = &gd.null_color;
   }
   
   /* Add Winds colors */
    for(i=0; i <   gd.layers.num_wind_sets; i++) {
      STRcopy(gd.color[gd.num_colors].name,
              gd.layers.wind[i].color_name,NAME_LENGTH);
      gd.layers.wind[i].color = &gd.color[gd.num_colors];
      gd.num_colors++;
    }

    /* Add Contour colors */
    
    for(i=0; i <   NUM_CONT_LAYERS; i++) {
      STRcopy(gd.color[gd.num_colors].name,
              gd.layers.cont[i].color_name,NAME_LENGTH);
      gd.layers.cont[i].color = &gd.color[gd.num_colors];
      gd.num_colors++;
    }

    // Add the TERRAIN Colors
    
    STRcopy(gd.color[gd.num_colors].name,
	    _params.terrain_earth_color1, NAME_LENGTH);
    gd.layers.earth.color1 = &gd.color[gd.num_colors];
    gd.num_colors++;

    STRcopy(gd.color[gd.num_colors].name,
	    _params.terrain_earth_color2, NAME_LENGTH);
    gd.layers.earth.color2 = &gd.color[gd.num_colors];
    gd.num_colors++;
    
    /* Record where the map_overlay colors start */
    gd.map_overlay_color_index_start = gd.num_colors;
    
    /* Now go through each overlay and add their colors to lists */
    for(i=0; i < gd.num_map_overlays ; i++) {        /* each color map file */
      STRcopy(gd.color[gd.num_colors].name,gd.over[i]->color_name,NAME_LENGTH);
      gd.over[i]->color =  &gd.color[gd.num_colors];
      gd.num_colors++;
      
    }
    
    /* Record the total number of non- colorscale colors */
    gd.num_draw_colors = gd.num_colors;
    
    /* Now go through each data field color scale file and pick out unique colors */
    int iret = 0;
    for(i=0; i < gd.num_datafields ; i++) {        /* each color map file */
      nentries = _get_color_mapping(color_file_subdir,gd.mrec[i]->color_file,gd.mrec[i]->h_vcm.vc);
      if (nentries < 0) {
        fprintf(stderr, "ERROR - setup_colorscales\n");
        fprintf(stderr, "  field_label: %s\n", gd.mrec[i]->field_label);
        fprintf(stderr, "  url: %s\n", gd.mrec[i]->url);
        fprintf(stderr, "  color_file: %s\n", gd.mrec[i]->color_file);
        exit (1);
      }
      
      if(gd.debug) {
        fprintf(stderr, "DEBUG - setup_colorscales - got color scale\n");
        fprintf(stderr, "  field_label: %s\n", gd.mrec[i]->field_label);
        fprintf(stderr, "  url: %s\n", gd.mrec[i]->url);
        fprintf(stderr, "  color_file: %s\n", gd.mrec[i]->color_file);
      }
      
      gd.mrec[i]->h_vcm.nentries = nentries;
      gd.mrec[i]->v_vcm.nentries = nentries;
      memcpy(gd.mrec[i]->v_vcm.vc, gd.mrec[i]->h_vcm.vc,
             sizeof(gd.mrec[i]->v_vcm.vc));
      
      if(gd.debug) {
        XColor  rgb_def;
        fprintf(stderr,"Colorfile: %s\n", gd.mrec[i]->color_file);
        fprintf(stderr,"Min     Max       R   G   B\n");
        fprintf(stderr,"-----------------------------\n\n");
        for (int indx = 0; indx < nentries; indx++ ) {
          XParseColor(gd.dpy,gd.cmap,gd.mrec[i]->h_vcm.vc[indx]->cname,&rgb_def);
#ifdef GTK_DISPLAY
          fprintf(stderr,"color%.2d = %.3d;%.3d;%.3d\n",
                  indx,
                  rgb_def.red / 257,
                  rgb_def.green / 257,
                  rgb_def.blue / 257);
#else
          fprintf(stderr,"%6g\t%6g\t%3d %3d %3d\n",
                  gd.mrec[i]->h_vcm.vc[indx]->min,
                  gd.mrec[i]->h_vcm.vc[indx]->max,
                  rgb_def.red / 257,
                  rgb_def.green / 257,
                  rgb_def.blue / 257);
#endif
        }
        fprintf(stderr,"-----------------------------\n");
      }
      
      /* Set overlay and colorscale thresholds to current min,max of defined colorscale */
      gd.mrec[i]->overlay_min = gd.mrec[i]->h_vcm.vc[0]->min;
      gd.mrec[i]->overlay_max = gd.mrec[i]->h_vcm.vc[gd.mrec[i]->h_vcm.nentries-1]->max;
      gd.mrec[i]->cscale_min = gd.mrec[i]->h_vcm.vc[0]->min;
      gd.mrec[i]->cscale_delta = (gd.mrec[i]->h_vcm.vc[gd.mrec[i]->h_vcm.nentries-1]->max - gd.mrec[i]->cscale_min) / gd.mrec[i]->h_vcm.nentries;
      
      if(gd.debug)
        fprintf(stderr,"Loaded color file %s - %d entries\n",gd.mrec[i]->color_file,nentries);
      
      if(nentries <=0 )  {
        return -1; 
      }
      for(j=0; j < nentries; j++) {        /* each entry in color map */
        found = 0;
        /* look through current list of colors (excluding draw colors)  */
        for(k=gd.num_draw_colors; k < gd.num_colors; k++ ) {
          if(strcmp(gd.mrec[i]->h_vcm.vc[j]->cname,gd.color[k].name) == 0) {    /* is the same */
            found = 1;    
          } 
        }
        if((found == 0) && (gd.num_colors < MAX_COLORS)) {
          STRcopy(gd.color[gd.num_colors++].name,gd.mrec[i]->h_vcm.vc[j]->cname,NAME_LENGTH);
        }
        
        if(gd.num_colors >= MAX_COLORS) {  
          fprintf(stderr,"Too many unique colors defined in colormaps. Please use a more limited\n");
          fprintf(stderr,"set of colors in your colorscales. MAX_COLORS currently compiled in (set) is: %d\n",MAX_COLORS);
          fprintf(stderr,"Turn on debug parameter (-v1)  to see a report on the number of colors in each scale.\n");
          exit(-3);
        }
        
      }
      if(gd.debug) fprintf(stderr,"Total Unique colors used so far: %d\n",gd.num_colors);
    } // i
    if (iret < 0) {
      exit (1);
    }
    
    if( gd.layers.earth.landuse_active) {
      // LOAD up the color scale for the Lands Use field
      nentries = _get_color_mapping(color_file_subdir,
                                   gd.layers.earth.land_use->color_file,
                                   gd.layers.earth.land_use->h_vcm.vc);
      gd.layers.earth.land_use->h_vcm.nentries = nentries;
      gd.layers.earth.land_use->v_vcm.nentries = nentries;
      memcpy(gd.layers.earth.land_use->v_vcm.vc,
	     gd.layers.earth.land_use->h_vcm.vc,
	     sizeof(gd.layers.earth.land_use->v_vcm.vc));
      
      /* Set overlay and colorscale thresholds to currnet min,max of defined colorscale */
      gd.layers.earth.land_use->overlay_min = gd.layers.earth.land_use->h_vcm.vc[0]->min;
      gd.layers.earth.land_use->overlay_max =
        gd.layers.earth.land_use->h_vcm.vc[gd.layers.earth.land_use->h_vcm.nentries-1]->max;
      
      gd.layers.earth.land_use->cscale_min = gd.layers.earth.land_use->h_vcm.vc[0]->min;
      gd.layers.earth.land_use->cscale_delta = 
        (gd.layers.earth.land_use->h_vcm.vc[gd.layers.earth.land_use->h_vcm.nentries-1]->max - 
         gd.layers.earth.land_use->cscale_min) / gd.layers.earth.land_use->h_vcm.nentries;
      
      if(gd.debug)
        fprintf(stderr,"Loaded color file %s - %d entries\n",
                gd.layers.earth.land_use->color_file,nentries);
      
      if(nentries <=0 )  {
        return -1; 
      }
      for(j=0; j < nentries; j++) {        /* each entry in color map */
        found = 0;
        /* look through current list of colors (excluding draw colors)  */
        for(k=gd.num_draw_colors; k < gd.num_colors; k++ ) {
          if(strcmp(gd.layers.earth.land_use->h_vcm.vc[j]->cname,
                    gd.color[k].name) == 0) {    /* is the same */
            found = 1;    
          } 
        }
        if((found == 0) && (gd.num_colors < MAX_COLORS)) {
          STRcopy(gd.color[gd.num_colors++].name,
                  gd.layers.earth.land_use->h_vcm.vc[j]->cname,NAME_LENGTH);
        }
        
        if(gd.num_colors >= MAX_COLORS) {  
          fprintf(stderr,"Too many unique colors defined in colormaps. Please use a more limited\n");
          fprintf(stderr,"set of colors in your colorscales. MAX_COLORS currently set to : %d\n",MAX_COLORS);
          fprintf(stderr,"Turn on debug parameter to see a report on the number of colors in each scale.\n");
          exit(-3);
	}
        
      }
    }
    if(gd.debug) fprintf(stderr,"Total Unique colors used so far: %d\n",gd.num_colors);
    
    // switch(v_class = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_VISUAL_CLASS)) {
    switch(v_class = PseudoColor) {
      default:
        fprintf(stderr,"Visual Class %d of Display not supported - Sorry \n",v_class);
        return -1;
        break;
        
      case PseudoColor:
        /* Allocate the color cells needed -  Can Modify colors */
        status = XAllocColorCells(dpy, cmap, False,&mask, 0, pix_vals, gd.num_colors);
        if(!gd.quiet_mode) fprintf(stderr,"Detected 8 Bit display - Allocating %d colors\n",gd.num_colors);
        if(status == 0) {
          //   cerr << "HHHHHHHHHHHHHHHHHHHh" << endl;
          //   return -1;
          }
        break;
        
      case TrueColor: /* Colors are read only */
      case DirectColor: /* Colors are read only */
        if(!gd.quiet_mode) fprintf(stderr,"Detected TrueColor display\n");
        break;
    }
    
    /* Get the Pixel value for the background */
    XParseColor(dpy,cmap,gd.legends.background_color->name,&rgb_def);
    gcvalues.background = rgb_def.pixel;
    
    /* Put the correct color into its color cell (or get the closest cell) and create a GC for it */
    for(i=0; i< gd.num_colors; i++) {
      mult = (i < gd.num_draw_colors)? 1.0 : _params.data_inten;
      
      XParseColor(dpy,cmap,gd.color[i].name,&rgb_def);
      
      gd.color[i].r = rgb_def.red;    /* save the color values & pixel */
      gd.color[i].g = rgb_def.green;
      gd.color[i].b = rgb_def.blue;
      
      rgb_def.red = (short unsigned int) (gd.color[i].r * mult);    /* Start with the default intensity */
      rgb_def.green = (short unsigned int) (gd.color[i].g * mult);
      rgb_def.blue = (short unsigned int) (gd.color[i].b * mult);
      rgb_def.flags = DoRed | DoGreen | DoBlue;
      
      // if ( PseudoColor == xv_get(gd.h_win_horiz_bw->horiz_bw,XV_VISUAL_CLASS)) {
      if ( PseudoColor == 0 ) {
        gd.color[i].pixval = pix_vals[i];
        rgb_def.pixel = pix_vals[i];
        XStoreColor(dpy,cmap,&rgb_def);
        gcvalues.foreground =  pix_vals[i];    
        gcvalues.background =  pix_vals[0];    
      } else {
        XAllocColor(dpy,cmap,&rgb_def);
        gd.color[i].pixval = rgb_def.pixel;
        gcvalues.foreground =  gd.color[i].pixval;
        gcvalues.background =  gd.color[0].pixval;
      }
      
      gd.color[i].gc = XCreateGC(gd.dpy,gd.hcan_xid,(GCForeground | GCBackground),&gcvalues);
      
    }
    
    /* Set Up Rubber Band - "Overlay" gc */
    switch(v_class) {
      case TrueColor: /* Colors are read only */
      case DirectColor: /* Colors are read only */
        XAllocNamedColor(dpy,cmap,OVERLAY_COLOR,&rgb_def,&rgb_exact);
        
    	/* Set up  GC's to use when drawing in the overlay plane - */
    	gcvalues.foreground =  rgb_def.pixel; 
    	gcvalues.background =  gd.color[0].pixval; 
        
    	gd.ol_gc = XCreateGC(gd.dpy,gd.hcan_xid,GCForeground,&gcvalues);
        XSetFunction(gd.dpy, gd.ol_gc, GXxor); /*  */
        
        
    	gcvalues.foreground =  rgb_def.pixel; 
    	gd.clear_ol_gc = XCreateGC(gd.dpy,gd.hcan_xid,GCForeground,&gcvalues);
        XSetFunction(gd.dpy, gd.clear_ol_gc, GXxor); /*  */
        break;
        
      case PseudoColor: 
        /* Can use Xor GC for rubberband lines */
        gd.ol_gc = XCreateGC(gd.dpy, DefaultRootWindow(gd.dpy), 0, 0);/* */
        XSetForeground(gd.dpy, gd.ol_gc, 5L); /*  */
        XSetFunction(gd.dpy, gd.ol_gc, GXxor); /*  */
        XSetLineAttributes(gd.dpy, gd.ol_gc, 1, LineSolid, CapButt, JoinMiter); /*  */
        break;
    }
    
    /* Copy the X color cell values into the overlay data structs */
    for(i=0,j = gd.map_overlay_color_index_start; i < gd.num_map_overlays; i++,j++) { 
      gd.over[i]->pixval = gd.color[j].pixval; 
    }
    
    // Copy the background pixval into the missing and bad for filled images
    if(gd.layers.missing_data_color->gc == NULL) 
      gd.layers.missing_data_color->pixval = gd.legends.background_color->pixval;
    if(gd.layers.bad_data_color->gc == NULL) 
      gd.layers.bad_data_color->pixval = gd.legends.background_color->pixval;
    
    return 0;
}

/*****************************************************************
 * SETUP_COLORSCALES: Establish X value to pixel color value arrays
 *    for each data field.
 */
   
void setup_colorscales(Display *dpy)
{
    int    i,k,l;
    int status;
    int    found_color;
    Visual *visual;
 
    status = 0;

    gd.cmap = DefaultColormap(dpy,DefaultScreen(dpy));
    status = _combine_color_maps(dpy,gd.cmap);
    if(status < 0) {
        fprintf(stderr,"Warning: Cannot allocate enough colors from standard colormap\n");
        fprintf(stderr,"Creating one - Other windows may be affected\n");
        visual = DefaultVisual(gd.dpy,0);
        gd.cmap = XCreateColormap(gd.dpy,RootWindow(gd.dpy,0),visual,AllocNone);
        status = _combine_color_maps(dpy,gd.cmap);
        if(status < 0) {
            fprintf(stderr,"Error allocating colors from new colormap - \n");
            exit(-1);
        }
    }

    /* Get the pixel value to use for each color scale entry */
    for(i=0; i < gd.num_datafields; i++ ) {        /* for all data fields */
        for(k=0; k < gd.mrec[i]->h_vcm.nentries ; k++) {    /* look through color map */
            found_color = 0;
            for(l=gd.num_draw_colors; (l < gd.num_colors) && (found_color == 0); l++) {    /* look thru all colors */
                if(strcmp(gd.mrec[i]->h_vcm.vc[k]->cname,gd.color[l].name) == 0) { /* if it matches*/
                    found_color = 1;
                    gd.mrec[i]->h_vcm.vc[k]->pixval = gd.color[l].pixval;    /* use this colors's pixel value  */
                    gd.mrec[i]->v_vcm.vc[k]->pixval = gd.color[l].pixval;    /* use this colors's pixel value  */
                    gd.mrec[i]->h_vcm.vc[k]->gc = gd.color[l].gc;  
                    gd.mrec[i]->v_vcm.vc[k]->gc = gd.color[l].gc; 
                }
            }
        }
    }

     if( gd.layers.earth.landuse_active) {
      //  Get the pixel value to use for each color scale entry in  LAND-USE
      for(k=0; k < gd.layers.earth.land_use->h_vcm.nentries ; k++) {    /* look through color map */
        found_color = 0;
        for(l=gd.num_draw_colors; (l < gd.num_colors) && (found_color == 0); l++) {    /* look thru all colors */
            if(strcmp(gd.layers.earth.land_use->h_vcm.vc[k]->cname,gd.color[l].name) == 0) { /* if it matches*/
                found_color = 1;
                gd.layers.earth.land_use->h_vcm.vc[k]->pixval = gd.color[l].pixval;    /* use this colors's pixel value  */
                gd.layers.earth.land_use->v_vcm.vc[k]->pixval = gd.color[l].pixval;    /* use this colors's pixel value  */
                gd.layers.earth.land_use->h_vcm.vc[k]->gc = gd.color[l].gc;  
                gd.layers.earth.land_use->v_vcm.vc[k]->gc = gd.color[l].gc; 
            }
        }
      }
    }
    
}

/*****************************************************************
 * SETUP_COLOR_MAPPING: Determine the mapping between data values and
 *        color to use
 *
 */

void setup_color_mapping( Valcolormap_t *vcm, double scale, double bias,
			  int transform_type, double  bad,  double missing)
{
    int    j,k,l;
    int    found_range;
    int    found_color;
    double    value;

    if(vcm->nentries == 0 || vcm->nentries > MAX_COLORS ) return;
     
    for(j=0; j < MAX_COLOR_CELLS; j++) {        /* calculate all possible data values that we represent*/

      // Trap Bad values (Out of band data - IE. before scaling)
      if((double) j == bad) {
	    vcm->val_gc[j] =  gd.layers.bad_data_color->gc;
            vcm->val_pix[j] = gd.layers.bad_data_color->pixval;
	    continue;
      }

      // Trap Bad or missing values (Out of band data - IE. before scaling)
      if((double) j == missing) {
	    vcm->val_gc[j] =  gd.layers.missing_data_color->gc;
            vcm->val_pix[j] = gd.layers.missing_data_color->pixval;
	    continue;
      }

      if (transform_type == Mdvx::DATA_TRANSFORM_LOG) {
        value = exp((double) j * scale + bias);
      } else {
        value = (double) j * scale + bias;
      }
        found_range = 0;

        /* look through color scale for proper range  */
        for(k=0; (k < vcm->nentries) && (found_range ==0); k++) {

            if((value >= vcm->vc[k]->min) && value < (vcm->vc[k]->max)) {
                found_range = 1;    /* this value maps into the range for this color */
                found_color = 0;

                /* Now find the matching color's info */
                for(l=gd.num_draw_colors; (l < gd.num_colors) && (found_color == 0); l++) {

                    if(strcmp(vcm->vc[k]->cname,gd.color[l].name) == 0) { /* if it matches*/
                        found_color = 1;
                        vcm->val_gc[j] = gd.color[l].gc;            /* use this colors's gc */
                        vcm->val_pix[j] = gd.color[l].pixval;    /* use this colors's pixel value */
                    }
                }
                if(found_color == 0) {    /* logic error, color should be in the list */
                    fprintf(stderr,"Couldn't find color %s in color array\n",vcm->vc[k]->cname);
                    exit(-3);
                }
            }
        }
         
        if(found_range == 0) {   /* this value doesn't map into any range specified in color scale */
            vcm->val_gc[j] = gd.legends.out_of_range_color->gc;
            vcm->val_pix[j] = gd.legends.out_of_range_color->pixval;
        }

    }
}

/*****************************************************************
 * AUTOSCALE_VCM: Autoscale the value-color mapping
 *        color to use
 *
 */

void autoscale_vcm( Valcolormap_t *vcm, double min, double max)
{
    int    i;
    double delta;

    delta = (max - min) / (double) vcm->nentries;

    if(delta == 0.0) delta = 0.1;

    for(i=0; i < vcm->nentries; i++) {
	vcm->vc[i]->min = min + (delta * i);
	vcm->vc[i]->max = vcm->vc[i]->min + delta;
    }

}

/*****************************************************************
 * Val2GC: Find the proper GC given a value and a Valcolormap_t *
 *
 */

GC Val2GC( Valcolormap_t *vcm, double val)
{
    int    i;

    for(i=0; i < vcm->nentries; i++) {
	if(val >= vcm->vc[i]->min && val < vcm->vc[i]->max)
	return vcm->vc[i]->gc;
    }

    return (GC) 0;

}

