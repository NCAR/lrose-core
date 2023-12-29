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
 * OVERLAY_INIT: Init and Load routines for overlay data
 */

#define OVERLAY_INIT

#include "cidd.h"
#include <shapelib/shapefil.h>

void load_rap_map(Overlay_t *ov, const char *map_file_subdir);
void load_shape_map(Overlay_t *ov, const char    *map_file_subdir);

/************************************************************************
 * LOAD_OVERLAY_INFO:  Scan info file and record info about overlay
 *         sources. Returns number of overlay info lines found &
 *         filled;
 *
 */

int   load_overlay_info(const char *param_buf, long param_buf_len, long line_no,
                        Overlay_t **over, int  max_overlays)
{
  int i,num_overlays;
  int  len,total_len;
  int num_fields;
  const char *start_ptr;
  const char *end_ptr;
  char    *cfield[32];
   
  char full_line[BUFSIZ];
    
  for(i=0; i < 32; i++)  cfield[i] = (char *) calloc(1,64);  /* get space for sub strings */

  /* read all the lines in the information file */
  num_overlays = 0;
  total_len = 0;
  start_ptr = param_buf;
  while((end_ptr = strchr(start_ptr,'\n')) != NULL &&
        (total_len < param_buf_len) && 
        (num_overlays < max_overlays)) {
    len = (end_ptr - start_ptr)+1;
    // Skip over blank, short or commented lines
    if((len > 20)  && *start_ptr != '#') {

      STRcopy(full_line, start_ptr, len);
      usubstitute_env(full_line, BUFSIZ);
	 
      over[num_overlays] = (Overlay_t *) calloc(1,sizeof(Overlay_t));
             
      num_fields = STRparse(full_line,cfield,BUFSIZ,32,64);  /* separate into substrings */
 
      if(num_fields >= 7) {    /* Is a correctly formatted line */
        STRcopy(over[num_overlays]->map_code,cfield[0],LABEL_LENGTH);
        STRcopy(over[num_overlays]->control_label,cfield[1],LABEL_LENGTH);
        STRcopy(over[num_overlays]->map_file_name,cfield[2],NAME_LENGTH);
        over[num_overlays]->default_on_state = atoi(cfield[3]);
        over[num_overlays]->line_width = atoi(cfield[3]);
        if(over[num_overlays]->line_width <=0) 
          over[num_overlays]->line_width = 1;
        over[num_overlays]->detail_thresh_min = atof(cfield[4]);
        over[num_overlays]->detail_thresh_max = atof(cfield[5]);

        over[num_overlays]->active = over[num_overlays]->default_on_state;
                 
        over[num_overlays]->color_name[0] = '\0';
        for(i=6; i < num_fields; i++) {
          strncat(over[num_overlays]->color_name,cfield[i],NAME_LENGTH-1);
          strncat(over[num_overlays]->color_name," ",NAME_LENGTH-1);
        }
        over[num_overlays]->color_name[strlen(over[num_overlays]->color_name) -1] = '\0';

        /* strip underscores out of control label */
        for(i = strlen(over[num_overlays]->control_label)-1;i >0 ; i--) {
          if (gd.replace_underscores && over[num_overlays]->control_label[i] == '_') 
            over[num_overlays]->control_label[i] = ' ';
        }

        num_overlays++;
      }
    }

    total_len += len  +1;
    start_ptr = end_ptr +1; // Skip past the newline
    line_no++;
  }

  for(i=0; i < 32; i++)  free(cfield[i]);         /* free space for sub strings */
     
  return num_overlays;
}


/************************************************************************
 * LOAD_OVERLAY_DATA: Load each Map
 */

int load_overlay_data(Overlay_t **over, int  num_overlays)
{
  int i;
  Overlay_t    *ov;    /* pointer to the current overlay structure */
  const char *map_file_subdir = gd.map_file_subdir;

  /* Read in each overlay file */
  for(i=0; i < num_overlays; i++) {
    ov = over[i];
    ov->num_polylines = 0;
    ov->num_labels = 0;
    ov->num_icons = 0;

    if(strstr(ov->map_file_name,".shp") != NULL  ||
       strstr(ov->map_file_name,".shx") != NULL) {

      load_shape_map(ov,map_file_subdir);

    } else {  // Assume RAP Map Format 
      load_rap_map(ov,map_file_subdir);
    }

    if(gd.debug)
      printf("Overlay File %s contains %ld Polylines, %ld Icon_defns, %ld Icons, %ld Labels\n",
             ov->map_file_name,ov->num_polylines,ov->num_icondefs,ov->num_icons,ov->num_labels);

  }  // End for(i=0; i < num_overlays ...

  return CIDD_SUCCESS;
}

/************************************************************************
 * LOAD_RAP_MAP
 */

void  load_rap_map(Overlay_t *ov, const char    *map_file_subdir)
{
  int    i,j;
  int    index,found;
  int    len,point;
  int    num_points;        
  int    num_fields;  /* number of fields (tokens) found in input line */
  int    map_len;
  int    ret_stat;
  char   *str_ptr;
  char   *map_buf;         // Buffer to hold map file
  char    name_buf[2048];  /* Buffer for input lines */
  char    dirname[2048];   /* Buffer for directories to search */
  FILE    *mapfile;
  char    *cfield[32];
  struct stat sbuf;
  char *lasts;

  for(i=0; i < 32; i++)  cfield[i] = (char *) calloc(1,64);  /* get space for sub strings */

  // Add . to list to start.
  strncpy(dirname,".,",1024);
  strncat(dirname,map_file_subdir,1024);

  str_ptr = strtok(dirname,","); // Prime strtok

  do{  // Try each comma delimited subdir

    while(*str_ptr == ' ') str_ptr++; //skip any leading space
    sprintf(name_buf,"%s/%s",str_ptr,ov->map_file_name);

    // Check if it's a HTTP URL
    if(strncasecmp(name_buf,"http:",5) == 0) {
      if(strlen(gd.http_proxy_url)  > URL_MIN_SIZE) {
        ret_stat =  HTTPgetURL_via_proxy(gd.http_proxy_url,
                                         name_buf,gd.data_timeout_secs * 1000,
                                         &map_buf, &map_len);
      } else {
        ret_stat =  HTTPgetURL(name_buf,
                               gd.data_timeout_secs * 1000,
                               &map_buf, &map_len);
      }
      if(ret_stat <=0 ) {
        map_len = 0;
        map_buf = NULL;
      }
      if(gd.debug) fprintf(stderr,"Map: %s: Len: %d\n",name_buf,map_len);
    } else {
      if(stat(name_buf,&sbuf) < 0) { // Stat to find the file's size
        map_len = 0;
        map_buf = NULL;
      }
      if((mapfile = fopen(name_buf,"r")) == NULL) {
        map_len = 0;
        map_buf = NULL;
      } else {
        if((map_buf = (char *)  calloc(sbuf.st_size + 1 ,1)) == NULL) {
          fprintf(stderr,"Problems allocating %ld bytes for map file\n",
                  (long) sbuf.st_size);
          exit(-1);
        }

        // Read
        if((map_len = fread(map_buf,1,sbuf.st_size,mapfile)) != sbuf.st_size) {
          fprintf(stderr,"Problems Reading %s\n",name_buf);
          exit(-1);
        }
        map_buf[sbuf.st_size] = '\0'; // Make sure to null terminate
        fclose(mapfile);
      }
    }
  } while ((str_ptr = strtok(NULL,",")) != NULL && map_len == 0 );

  if(map_len == 0 || map_buf == NULL) {
    fprintf(stderr,"Warning!: Unable to load map file: %s\n",ov->map_file_name);
    for(i=0; i < 32; i++)  free(cfield[i]);
    return;
  }

  // Prime strtok_r;
  str_ptr = strtok_r(map_buf,"\n",&lasts);

  while (str_ptr != NULL) {        /* read all lines in buffer */
    if(*str_ptr != '#') {

      if(strncasecmp(str_ptr,"MAP_NAME",8) == 0) {    /* Currently Ignore */
        str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
        continue;
      }

      if(strncasecmp(str_ptr,"TRANSFORM",9) == 0) {    /* Currently Ignore */
        str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
        continue;
      }

      if(strncasecmp(str_ptr,"PROJECTION",10) == 0) {    /* Currently Ignore */
        str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
        continue;
      }

      if(strncasecmp(str_ptr,"ICONDEF",7) == 0) {        /* describes an icon's coordinates in pixels */
        index = ov->num_icondefs;
        if(index >= ov->num_alloc_icondefs) {
          if(ov->num_alloc_icondefs == 0) { /* start with space for 2 */
            ov->geo_icondef = (Geo_feat_icondef_t **)
              calloc(2,sizeof(Geo_feat_icondef_t *));
            ov->num_alloc_icondefs = 2;
          } else { /* Double the space */
            ov->num_alloc_icondefs *= 2;
            ov->geo_icondef = (Geo_feat_icondef_t **) 
              realloc(ov->geo_icondef, ov->num_alloc_icondefs * sizeof(Geo_feat_icondef_t *)); 
          }
        }
        if(ov->geo_icondef == NULL) {
          fprintf(stderr,"Unable to allocate memory for Icon definition pointer array!\n");
          exit(-1);
        }

        if(STRparse(str_ptr,cfield,256,32,64) != 3) {
          fprintf(stderr,"Error in ICONDEF line: %s\n",str_ptr);
          exit(-1);
        }
        /* get space for the icon definition */
        ov->geo_icondef[index] = (Geo_feat_icondef_t *) calloc(1,sizeof(Geo_feat_icondef_t));
        ZERO_STRUCT(ov->geo_icondef[index]);

        if(ov->geo_icondef[index] == NULL) {
          fprintf(stderr,"Unable to allocate memory for Icon definition!\n");
          exit(-1);
        }
        STRcopy(ov->geo_icondef[index]->name,cfield[1],NAME_LENGTH);
        num_points = atoi(cfield[2]);

        /* Get space for points in the icon */
        ov->geo_icondef[index]->x = (short *) calloc(1,num_points * sizeof(short));
        ov->geo_icondef[index]->y = (short *) calloc(1,num_points * sizeof(short));

        if(ov->geo_icondef[index]->x == NULL || ov->geo_icondef[index]->y == NULL) {
          fprintf(stderr,"Error!: Unable to allocate space for icon points in file %s, num points: %d\n",
                  ov->map_file_name,num_points);
          exit(-1);
        }

        /* Read in all of the points */
        for(j=0,point = 0; j < num_points; j++) {

          str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line

          if(str_ptr != NULL && STRparse(str_ptr,cfield,256,32,64) == 2) {
            ov->geo_icondef[index]->x[point] = atoi(cfield[0]);
            ov->geo_icondef[index]->y[point] = atoi(cfield[1]);
            point++;
          }
        }
        ov->geo_icondef[index]->num_points = point;
        ov->num_icondefs++;
        str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
        continue;
      }

      if(strncasecmp(str_ptr,"ICON ",5) == 0) {    
        index = ov->num_icons;
        if(index >= ov->num_alloc_icons) {
          if(ov->num_alloc_icons == 0) { /* start with space for 2 */
            ov->geo_icon = (Geo_feat_icon_t **)
              calloc(2,sizeof(Geo_feat_icon_t *));
            ov->num_alloc_icons = 2;
          } else {  /* Double the space */
            ov->num_alloc_icons *= 2;
            ov->geo_icon = (Geo_feat_icon_t **) 
              realloc(ov->geo_icon, ov->num_alloc_icons * sizeof(Geo_feat_icon_t *)); 
          }
        }
        if(ov->geo_icon == NULL) {
          fprintf(stderr,"Unable to allocate memory for Icon pointer array!\n");
          exit(-1);
        }

        /* get space for the Icon */
        ov->geo_icon[index] = (Geo_feat_icon_t *) calloc(1,sizeof(Geo_feat_icon_t));
        ZERO_STRUCT(ov->geo_icon[index]);

        if(ov->geo_icon[index] == NULL) {
          fprintf(stderr,"Unable to allocate memory for Icon definition!\n");
          exit(-1);
        }

        if((num_fields = STRparse(str_ptr,cfield,256,32,64)) < 6) {
          fprintf(stderr,"Error in ICON line: %s\n",str_ptr);
          exit(-1);
        }

        /* find the definition for the line segments that make up the icon */
        ov->geo_icon[index]->icon = NULL;
        found = 0;
        for(j=0; j < ov->num_icondefs && found == 0; j++) {
          if(strcmp(ov->geo_icondef[j]->name,cfield[1]) == 0) {
            ov->geo_icon[index]->icon = ov->geo_icondef[j];
            found = 1;
          }
        }

        if(found == 0) {    
          fprintf(stderr,"No Icon definition: %s found in file %s!\n",cfield[1],ov->map_file_name);
          str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
          continue;
        }

        /* record its position */
        ov->geo_icon[index]->lat = atof(cfield[2]);
        ov->geo_icon[index]->lon = atof(cfield[3]);
        ov->geo_icon[index]->text_x = atoi(cfield[4]);
        ov->geo_icon[index]->text_y = atoi(cfield[5]);

        /* gather up remaining text fields */
        ov->geo_icon[index]->label[0] = '\0';
        len = 2;
        for(j = 6; j < num_fields && len < LABEL_LENGTH; j++ ) {
          strncat(ov->geo_icon[index]->label,cfield[j],LABEL_LENGTH - len);
          len = strlen(ov->geo_icon[index]->label) +1;

          // Separate multiple text label fiedds with spaces.
          if( j < num_fields -1) {
            strncat(ov->geo_icon[index]->label," ",LABEL_LENGTH - len);
            len = strlen(ov->geo_icon[index]->label) +1;
          }
        }
        {
          int labellen = strlen(ov->geo_icon[index]->label);
          if (labellen > 1) {
            if (ov->geo_icon[index]->label[labellen-1] == ' ') {
              ov->geo_icon[index]->label[labellen-1] = '\0';
            }
          }
        }

        ov->num_icons++;
        str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
        continue;
      }

      if(strncasecmp(str_ptr,"POLYLINE",8) == 0) {    
        index = ov->num_polylines;
        if(index >= ov->num_alloc_polylines) {
          if(ov->num_alloc_polylines == 0) { /* start with space for 2 */
            ov->geo_polyline = (Geo_feat_polyline_t **)
              calloc(2,sizeof(Geo_feat_polyline_t *));
            ov->num_alloc_polylines = 2;
          } else {  /* Double the space */
            ov->num_alloc_polylines *= 2;
            ov->geo_polyline = (Geo_feat_polyline_t **) 
              realloc(ov->geo_polyline, ov->num_alloc_polylines * sizeof(Geo_feat_polyline_t *)); 
          }
        }
        if(ov->geo_polyline == NULL) {
          fprintf(stderr,"Unable to allocate memory for Polyline pointer array!\n");
          exit(-1);
        }

        if((STRparse(str_ptr,cfield,256,32,64)) != 3) {
          fprintf(stderr,"Error in POLYLINE line: %s\n",str_ptr);
          exit(-1);
        }
        /* get space for the Polyline definition */
        ov->geo_polyline[index] = (Geo_feat_polyline_t *) calloc(1,sizeof(Geo_feat_polyline_t));
        ZERO_STRUCT(ov->geo_polyline[index]);

        if(ov->geo_polyline[index] == NULL) {
          fprintf(stderr,"Unable to allocate memory for Polyline definition!\n");
          exit(-1);
        }
        STRcopy(ov->geo_polyline[index]->label,cfield[1],LABEL_LENGTH);
        num_points = atoi(cfield[2]);
        if(num_points <=0 ) {
          fprintf(stderr,"Warning!: Bad POLYLINE Definition. File: %s, Line: %s\n",name_buf,str_ptr);
          fprintf(stderr,"        : Format should be:    POLYLINE Label #points\n");
          fprintf(stderr,"        : Skipping \n");
          str_ptr = strtok_r(NULL,"\n",&lasts); // move to next line
          continue;
        }

        /* Get space for points in the polyline */
        ov->geo_polyline[index]->lat = (double *) calloc(1,num_points * sizeof(double));
        ov->geo_polyline[index]->lon = (double *) calloc(1,num_points * sizeof(double));
        ov->geo_polyline[index]->local_x = (double *) calloc(1,num_points * sizeof(double));
        ov->geo_polyline[index]->local_y = (double *) calloc(1,num_points * sizeof(double));

        if(ov->geo_polyline[index]->lat == NULL || ov->geo_polyline[index]->lon == NULL) {
          fprintf(stderr,"Error!: Unable to allocate space for polyline points in file %s, num points: %d\n",
                  ov->map_file_name,num_points);
          exit(-1);
        }

        /* Read in all of the points */
        for(j=0,point = 0; j < num_points; j++) {
          str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
          if(str_ptr != NULL && STRparse(str_ptr,cfield,256,32,64) >= 2) {
            ov->geo_polyline[index]->lat[point] = atof(cfield[0]);
            ov->geo_polyline[index]->lon[point] = atof(cfield[1]);
            point++;
          }
        }
        ov->geo_polyline[index]->num_points = point;
        ov->num_polylines++;
        str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
        continue;
      }

      if(strncasecmp(str_ptr,"LABEL",5) == 0) {    
        index = ov->num_labels;
        if(index >= ov->num_alloc_labels) {
          if(ov->num_alloc_labels == 0) { /* start with space for 2 */
            ov->geo_label = (Geo_feat_label_t **)
              calloc(2,sizeof(Geo_feat_label_t *));
            ov->num_alloc_labels = 2;
          } else {  /* Double the space */
            ov->num_alloc_labels *=2;
            ov->geo_label = (Geo_feat_label_t **) 
              realloc(ov->geo_label, ov->num_alloc_labels * sizeof(Geo_feat_label_t *)); 
          }
        }
        if(ov->geo_label == NULL) {
          fprintf(stderr,"Unable to allocate memory for Label pointer array!\n");
          exit(-1);
        }

        ov->num_labels++;
                     
        /* get space for the Label definition */
        ov->geo_label[index] = (Geo_feat_label_t *) calloc(1,sizeof(Geo_feat_label_t));
        ZERO_STRUCT(ov->geo_label[index]);

        if(ov->geo_label[index] == NULL) {
          fprintf(stderr,"Unable to allocate memory for Label definition!\n");
          exit(-1);
        }

        if((num_fields = STRparse(str_ptr,cfield,256,32,64)) < 8) {
          fprintf(stderr,"Too few fields in LABEL line: %s\n",str_ptr);
          str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
          continue;
        } 
        ov->geo_label[index]->min_lat = atof(cfield[1]);
        ov->geo_label[index]->min_lon = atof(cfield[2]);
        ov->geo_label[index]->max_lat = atof(cfield[3]);
        ov->geo_label[index]->max_lon = atof(cfield[4]);
        ov->geo_label[index]->rotation = atof(cfield[5]);
        ov->geo_label[index]->attach_lat = atof(cfield[6]);
        ov->geo_label[index]->attach_lon = atof(cfield[7]);

        ov->geo_label[index]->string[0] = '\0';
        len = 2;
        for(j = 8; j < num_fields && len < NAME_LENGTH; j++) {
          strncat(ov->geo_label[index]->string,cfield[j],NAME_LENGTH - len);
          len = strlen(ov->geo_label[index]->string) +1;
          if(j < num_fields -1)
            strncat(ov->geo_label[index]->string," ",NAME_LENGTH - len);
          len = strlen(ov->geo_label[index]->string) +1;
        }
        str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
        continue;
      }


      if(strncasecmp(str_ptr,"SIMPLELABEL",11) == 0) {    
        index = ov->num_labels;
        if(index >= ov->num_alloc_labels) {
          if(ov->num_alloc_labels == 0) { /* start with space for 2 */
            ov->geo_label = (Geo_feat_label_t **)
              calloc(2,sizeof(Geo_feat_label_t *));
            ov->num_alloc_labels = 2;
          } else {  /* Double the space */
            ov->num_alloc_labels *=2;
            ov->geo_label = (Geo_feat_label_t **) 
              realloc(ov->geo_label, ov->num_alloc_labels * sizeof(Geo_feat_label_t *)); 
          }
        }
        if(ov->geo_label == NULL) {
          fprintf(stderr,"Unable to allocate memory for Label pointer array!\n");
          exit(-1);
        }
        ov->num_labels++;

        /* get space for the Label definition */
        ov->geo_label[index] = (Geo_feat_label_t *) calloc(1,sizeof(Geo_feat_label_t));
        ZERO_STRUCT(ov->geo_label[index]);

        if(ov->geo_label[index] == NULL) {
          fprintf(stderr,"Unable to allocate memory for Label definition!\n");
          exit(-1);
        }

        if((num_fields = STRparse(str_ptr,cfield,256,32,64)) < 4) {
          fprintf(stderr,"Too few fields in SIMPLELABEL line: %s\n",str_ptr);
          str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
          continue;
        } 
        ov->geo_label[index]->min_lat = atof(cfield[1]);
        ov->geo_label[index]->min_lon = atof(cfield[2]);
        ov->geo_label[index]->max_lat = atof(cfield[1]);
        ov->geo_label[index]->max_lon = atof(cfield[2]);
        ov->geo_label[index]->rotation = 0;
        ov->geo_label[index]->attach_lat = atof(cfield[1]);
        ov->geo_label[index]->attach_lon = atof(cfield[2]);

        ov->geo_label[index]->string[0] = '\0';
        len = 2;
        for(j = 3; j < num_fields && len < NAME_LENGTH; j++) {
          strncat(ov->geo_label[index]->string,cfield[j],NAME_LENGTH - len);
          len = strlen(ov->geo_label[index]->string) +1;
          if(j < num_fields -1)
            strncat(ov->geo_label[index]->string," ",NAME_LENGTH - len);
          len = strlen(ov->geo_label[index]->string) +1;
        }
        str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
        continue;
      }

    } 

    // Nothing matches
    str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line

  }  // End of while additional lines exist in buffer

  if(map_buf!= NULL) {
    free(map_buf);
    map_buf = NULL;
  }

  for(i=0; i < 32; i++)  free(cfield[i]);         /* free space for sub strings */
  return;

}

/************************************************************************
 * LOAD_SHAPE_OVERLAY_DATA: This version reads Shape files
 */

void   load_shape_map(Overlay_t *ov, const char    *map_file_subdir)
{
  int    i,j;
  int    index,found,is_http;
  int    point;
  int    num_points;        
  int    ret_stat;
  char   *str_ptr;
  char    name_base[1024];  /* Buffer for input names */
  char    dirname[4096];   /* Buffer for directories to search */
  char    name_buf[2048];  /* Buffer for input names */
  char    name_buf2[2048]; /* Buffer for input names */
  char    *map_buf;
  int     map_len;

  SHPHandle SH;
  SHPObject *SO;
  FILE    *map_file;

  int pid = getpid();

  // Add . to list of dirs to search  to start.
  strncpy(dirname,".,",2048);
  strncat(dirname,map_file_subdir,2048);


  found = 0;
  is_http = 0;

  // Search each subdir
  str_ptr = strtok(dirname,","); // Prime strtok
  do{  //  Search 

    while(*str_ptr == ' ') str_ptr++; //skip any leading space

    sprintf(name_buf,"%s/%s,",str_ptr,ov->map_file_name);

    // Check if it's a HTTP URL
    if(strncasecmp(name_buf,"http:",5) == 0) {

      // Extract name base
      strncpy(name_base,ov->map_file_name,1023);
      char *ptr = strrchr(name_base,'.');
      if(ptr != NULL) *ptr = '\0';

      // Download  SHP Part of shapefile
      sprintf(name_buf,"%s/%s.shp",str_ptr,name_base);
      if(strlen(gd.http_proxy_url)  > URL_MIN_SIZE) {
        ret_stat = HTTPgetURL_via_proxy(gd.http_proxy_url,
                                        name_buf,gd.data_timeout_secs * 1000,
                                        &map_buf, &map_len);
      } else {
        ret_stat =  HTTPgetURL(name_buf,
                               gd.data_timeout_secs * 1000,
                               &map_buf, &map_len);
      }
      if(ret_stat > 0 && map_len > 0 ) { // Succeeded
        is_http = 1;

        if(gd.debug) fprintf(stderr,"Read Shape File: %s: Len: %d\n",name_buf,map_len);

        sprintf(name_buf2,"/tmp/%d_%s.shp",pid,name_base);
        if((map_file = fopen(name_buf2,"w")) == NULL) {
          fprintf(stderr,"Problems Opening %s for writing\n",name_buf2);
          perror("CIDD ");
          exit(-1);
        }
        if(fwrite(map_buf,map_len,1,map_file) != 1) {
          fprintf(stderr,"Problems Writing to %s \n",name_buf2);
          perror("CIDD ");
          exit(-1);
        }
        fclose(map_file);
        if(map_buf != NULL) free(map_buf);
      }

      // Download  SHX Part of shapefile
      sprintf(name_buf,"%s/%s.shx",str_ptr,name_base);
      if(strlen(gd.http_proxy_url)  > URL_MIN_SIZE) {
        ret_stat = HTTPgetURL_via_proxy(gd.http_proxy_url,
                                        name_buf,gd.data_timeout_secs * 1000,
                                        &map_buf, &map_len);

      } else {
        ret_stat =  HTTPgetURL(name_buf,
                               gd.data_timeout_secs * 1000,
                               &map_buf, &map_len);
      }
      if(ret_stat > 0  && map_len > 0) { // Succeeded

        if(gd.debug) fprintf(stderr,"Read Shape File: %s: Len: %d\n",name_buf,map_len);

        sprintf(name_buf2,"/tmp/%d_%s.shx",pid,name_base);
        if((map_file = fopen(name_buf2,"w")) == NULL) {
          fprintf(stderr,"Problems Opening %s for writing\n",name_buf2);
          perror("CIDD ");
          exit(-1);
        }
        if(fwrite(map_buf,map_len,1,map_file) != 1) {
          fprintf(stderr,"Problems Writing to %s \n",name_buf2);
          perror("CIDD ");
          exit(-1);
        }
        fclose(map_file);
        if(map_buf != NULL) free(map_buf);
      }

      sprintf(name_buf,"/tmp/%d_%s",pid,name_base);
      if((SH = SHPOpen(name_buf,"rb")) != NULL) {
        found = 1;
      } else {
        fprintf(stderr,"Problems with SHPOpen on %s \n",name_buf);
      }

    } else {  // Looks like a regular file

      sprintf(name_buf,"%s/%s,",str_ptr,ov->map_file_name);
      if((SH = SHPOpen(name_buf,"rb")) != NULL) {
        found = 1;
      }
    }

  } while ((str_ptr = strtok(NULL,",")) != NULL && found == 0 );

  if( found == 0) {
    fprintf(stderr,"Warning!: Unable to load map file: %s\n",ov->map_file_name);
    if(is_http) {  // Unlink temporary files
      sprintf(name_buf2,"/tmp/%d_%s.shp",pid,name_base);
      unlink(name_buf2);
      sprintf(name_buf2,"/tmp/%d_%s.shx",pid,name_base);
      unlink(name_buf2);
    }
		
    return;
  }


  // Shape File is Found and Open

  int n_objects;
  int shape_type;
  int part_num;

  SHPGetInfo(SH, &n_objects, &shape_type, NULL, NULL);

  if(gd.debug) {
    fprintf(stderr,"Found %d objects, type %d  in %s\n",n_objects, shape_type, ov->map_file_name);
  }

  for(i=0; i < n_objects; i++ ) {  // Loop through each object

    SO = SHPReadObject(SH,i);    // Load the shape object

    switch(SO->nSHPType) {

      case SHPT_POLYGON:  // Polyline
      case SHPT_ARC:
      case SHPT_ARCM:
      case SHPT_ARCZ:
        index = ov->num_polylines;
        if(index >= ov->num_alloc_polylines) {
          if(ov->num_alloc_polylines == 0) { /* start with space for 2 */
            ov->geo_polyline = (Geo_feat_polyline_t **)
              calloc(2,sizeof(Geo_feat_polyline_t *));
            ov->num_alloc_polylines = 2;
          } else {  /* Double the space */
            ov->num_alloc_polylines *= 2;
            ov->geo_polyline = (Geo_feat_polyline_t **) 
              realloc(ov->geo_polyline, ov->num_alloc_polylines * sizeof(Geo_feat_polyline_t *)); 
          }
        }
        if(ov->geo_polyline == NULL) {
          fprintf(stderr,"Unable to allocate memory for Polyline pointer array!\n");
          exit(-1);
        }

        /* get space for the Polyline definition */
        ov->geo_polyline[index] = (Geo_feat_polyline_t *) calloc(1,sizeof(Geo_feat_polyline_t));
        ZERO_STRUCT(ov->geo_polyline[index]);
        if(ov->geo_polyline[index] == NULL) {
          fprintf(stderr,"Unable to allocate memory for Polyline definition!\n");
          exit(-1);
        }

        STRcopy(ov->geo_polyline[index]->label,"Shape",LABEL_LENGTH);

        /* Get space for points in the polyline */
        ov->geo_polyline[index]->lat = (double *) calloc(1,(SO->nVertices + SO->nParts) * sizeof(double));
        ov->geo_polyline[index]->lon = (double *) calloc(1,(SO->nVertices + SO->nParts) * sizeof(double));
        ov->geo_polyline[index]->local_x = (double *) calloc(1,(SO->nVertices + SO->nParts) * sizeof(double));
        ov->geo_polyline[index]->local_y = (double *) calloc(1,(SO->nVertices + SO->nParts) * sizeof(double));

        if(ov->geo_polyline[index]->lat == NULL || ov->geo_polyline[index]->lon == NULL) {
          fprintf(stderr,"Error!: Unable to allocate space for polyline points in file %s, num points: %d\n",
                  ov->map_file_name,SO->nVertices);
          exit(-1);
        }

        /* Read in all of the points */
        part_num = 1;
        for(j=0,point = 0; j < SO->nVertices; j++) {
          ov->geo_polyline[index]->lat[point] = SO->padfY[j];
          ov->geo_polyline[index]->lon[point] = SO->padfX[j];
          if(j+1 == SO->panPartStart[part_num]) {     // Insert a pen up in the data stream.
            point++;
            ov->geo_polyline[index]->lat[point] = -1000.0;
            ov->geo_polyline[index]->lon[point] = -1000.0;
            part_num++;
          }
          point++;
        }
        ov->geo_polyline[index]->num_points = point;
        ov->num_polylines++;

        break;

      case SHPT_POINT :  // Icon Instance
      case SHPT_POINTZ:
      case SHPT_POINTM:
        if(ov->num_icondefs == 0) {  // No Icon definition yet.
          if(ov->num_alloc_icondefs == 0) { /* start with space for 2 */
            ov->geo_icondef = (Geo_feat_icondef_t **) calloc(1,sizeof(Geo_feat_icondef_t *));
            ov->num_icondefs = 1;
            ov->num_alloc_icondefs = 1;
          }
                
          if(ov->geo_icondef == NULL) {
            fprintf(stderr,"Unable to allocate memory for Icon definition pointer array!\n");
            exit(-1);
          }

          /* get space for the icon definition */
          ov->geo_icondef[0] = (Geo_feat_icondef_t *) calloc(1,sizeof(Geo_feat_icondef_t));
          ZERO_STRUCT(ov->geo_icondef[0]);

          if(ov->geo_icondef[0] == NULL) {
            fprintf(stderr,"Unable to allocate memory for Icon definition!\n");
            exit(-1);
          }
          num_points = 6;  // A Predefined Box.

          /* Get space for points in the icon */
          ov->geo_icondef[0]->x = (short *) calloc(1,num_points * sizeof(short));
          ov->geo_icondef[0]->y = (short *) calloc(1,num_points * sizeof(short));

          if(ov->geo_icondef[0]->x == NULL || ov->geo_icondef[0]->y == NULL) {
            fprintf(stderr,"Error!: Unable to allocate space for icon points in file %s, num points: %d\n",
                    ov->map_file_name,num_points);
            exit(-1);
          }

          // Set all of the points - Draws a Small Box
          ov->geo_icondef[0]->x[0] = -1;
          ov->geo_icondef[0]->y[0] = -1;

          ov->geo_icondef[0]->x[1] = 1;
          ov->geo_icondef[0]->y[1] = -1;

          ov->geo_icondef[0]->x[2] = 1;
          ov->geo_icondef[0]->y[2] = 1;

          ov->geo_icondef[0]->x[3] = -1;
          ov->geo_icondef[0]->y[3] = 1;

          ov->geo_icondef[0]->x[4] = -1;
          ov->geo_icondef[0]->y[4] = -1;

          ov->geo_icondef[0]->x[5] = 32767;
          ov->geo_icondef[0]->y[5] = 32767;

          ov->geo_icondef[0]->num_points = num_points;
        }

        index = ov->num_icons;
        if(index >= ov->num_alloc_icons) {
          if(ov->num_alloc_icons == 0) { /* start with space for 2 */
            ov->geo_icon = (Geo_feat_icon_t **) calloc(2,sizeof(Geo_feat_icon_t *));
            ov->num_alloc_icons = 2;
          } else {  /* Double the space */
            ov->num_alloc_icons *= 2;
            ov->geo_icon = (Geo_feat_icon_t **) 
              realloc(ov->geo_icon, ov->num_alloc_icons * sizeof(Geo_feat_icon_t *)); 
          }
        }
        if(ov->geo_icon == NULL) {
          fprintf(stderr,"Unable to allocate memory for Icon pointer array!\n");
          exit(-1);
        }

        /* get space for the Icon */
        ov->geo_icon[index] = (Geo_feat_icon_t *) calloc(1,sizeof(Geo_feat_icon_t));
        ZERO_STRUCT(ov->geo_icon[index]);

        if(ov->geo_icon[index] == NULL) {
          fprintf(stderr,"Unable to allocate memory for Icon definition!\n");
          exit(-1);
        }

        // The definition for the Icon is fixed 
        ov->geo_icon[index]->icon = ov->geo_icondef[0];

        /* record its position */
        ov->geo_icon[index]->lat = SO->padfY[0];
        ov->geo_icon[index]->lon = SO->padfX[0];

        ov->num_icons++;
        break;

      default:
      case SHPT_NULL:
      case SHPT_MULTIPOINT:
      case SHPT_POLYGONZ:
      case SHPT_MULTIPOINTZ:
      case SHPT_POLYGONM:
      case SHPT_MULTIPOINTM:
      case SHPT_MULTIPATCH:
        if(gd.debug) {
          fprintf(stderr,"Encountered Unsupported Shape type %d\n",SO->nSHPType);
        }
        break;
    }

    if(SO != NULL) SHPDestroyObject(SO);
  }  // End of each object

  if(is_http) {  // Unlink temporary files
    sprintf(name_buf2,"/tmp/%d_%s.shp",pid,name_base);
    unlink(name_buf2);
    sprintf(name_buf2,"/tmp/%d_%s.shx",pid,name_base);
    unlink(name_buf2);
	
  }

}

/************************************************************************
 * INIT_OVER_DATA_LINKS:  Scan cidd_overlays.info file and setup
 *
 */ 
void init_over_data_links(const char *param_buf, long param_buf_len, long line_no)
  
{
  gd.num_map_overlays = load_overlay_info(param_buf,param_buf_len,line_no,gd.over,MAX_OVERLAYS);
  
  if(load_overlay_data(gd.over,gd.num_map_overlays) != CIDD_SUCCESS) {
    fprintf(stderr,"Problem loading overlay data\n");
    exit(0);
  }
  
  calc_local_over_coords();
}
