/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/************************************************************************
 * CIDD_OVERLAY_INIT: Init and Load routines for overlay data
 *
 *
 */

#include "cidd.h"

#define CIDD_OVERLAY_INIT

/************************************************************************
 * LOAD_OVERLAY_INFO:  Scan info file and record info about overlay
 *         sources. Returns number of overlay info lines found &
 *         filled;
 *
 */

int   load_overlay_info(char *info_file_name, Overlay_t **over, int  max_overlays)
{
    int i,num_overlays;
    int num_fields;
    FILE    *info_file;
    char    buf[256];
    char    *cfield[32];
   
    if((info_file = fopen(info_file_name,"r")) == NULL) {
        fprintf(stderr,"Couldn't open %s\n",info_file_name);
        exit(-1);
    }

    for(i=0; i < 32; i++)  cfield[i] = (char *) ucalloc(1,64);  /* get space for sub strings */

    /* read all the lines in the information file */
    num_overlays = 0;
    while(fgets(buf,256,info_file) != NULL && num_overlays < max_overlays) {
        if(buf[0] != '#') {
 
            over[num_overlays] = (Overlay_t *) ucalloc(1,sizeof(Overlay_t));
             
            num_fields = STRparse(buf,cfield,256,32,64);  /* separate into substrings */
 
            if(num_fields >= 6) {    /* Is a correctly formatted line */
                STRcopy(over[num_overlays]->map_code,cfield[0],LABEL_LENGTH);
                STRcopy(over[num_overlays]->control_label,cfield[1],LABEL_LENGTH);
                STRcopy(over[num_overlays]->map_file_name,cfield[2],NAME_LENGTH);
                over[num_overlays]->default_on_state = atoi(cfield[3]);
                over[num_overlays]->detail_thresh = atof(cfield[4]);

		over[num_overlays]->active = over[num_overlays]->default_on_state;
                 
                over[num_overlays]->color_name[0] = '\0';
                for(i=5; i < num_fields; i++) {
                    strncat(over[num_overlays]->color_name,cfield[i],NAME_LENGTH);
                    strncat(over[num_overlays]->color_name," ",NAME_LENGTH);
                }
                over[num_overlays]->color_name[strlen(over[num_overlays]->color_name) -1] = '\0';

                /* strip underscores out of control label */
                for(i = strlen(over[num_overlays]->control_label)-1;i >0 ; i--) {
                    if (over[num_overlays]->control_label[i] == '_') over[num_overlays]->control_label[i] = ' ';
                }

                num_overlays++;
            }
        }
    }
    fclose(info_file);

    for(i=0; i < 32; i++)  ufree(cfield[i]);         /* free space for sub strings */
     
    return num_overlays;
}

/************************************************************************
 * LOAD_OVERLAY_DATA: This version reads the overlay data directly,
 */

int    load_overlay_data(Overlay_t **over, int  num_overlays)
{
    int    i,j;
    int    index,found;
    int    len,point;
    int    num_points;        
    int    num_fields;        /* number of fields (tokens) founs in input line */
    double field[16];    /* space for numerical fields */
    char    buf[256];    /* Buffer for input lines */
    FILE    *infile;
    char    *cfield[32];
    Overlay_t    *ov;    /* pointer to the current overlay structure */


    for(i=0; i < 32; i++)  cfield[i] = (char *) ucalloc(1,64);  /* get space for sub strings */
    /* Read in the data points for each overlay file */
    for(i=0; i < num_overlays; i++) {
        ov = over[i];
        ov->num_polylines = 0;
        ov->num_labels = 0;
        ov->num_icons = 0;
        if((infile = fopen(ov->map_file_name,"r")) == NULL) {
            fprintf(stderr,"Warning!: Unable to find map file: %s\n",ov->map_file_name);
        } else {
            while(fgets(buf,256,infile) != NULL) {        /* read all lines in file */
                if(buf[0] == '#') continue;

                if(strncmp(buf,"MAP_NAME",8) == 0) {    /* Currently Ignore */
                    continue;
                }

                if(strncmp(buf,"TRANSFORM",9) == 0) {    /* Currently Ignore */
                    continue;
                }

                if(strncmp(buf,"PROJECTION",10) == 0) {    /* Currently Ignore */
                    continue;
                }

                if(strncmp(buf,"ICONDEF",7) == 0) {        /* describes an icon's coordinates in pixels */
                    index = ov->num_icondefs;
                    if(index >= ov->num_alloc_icondefs) {
			if(ov->num_alloc_icondefs == 0) { /* start with space for 2 */
			   ov->geo_icondef = (Geo_feat_icondef_t **)
			       ucalloc(2,sizeof(Geo_feat_icondef_t *));
			   ov->num_alloc_icondefs = 2;
			} else { /* Double the space */
			   ov->num_alloc_icondefs *= 2;
			   ov->geo_icondef = (Geo_feat_icondef_t **) 
			    urealloc(ov->geo_icondef, ov->num_alloc_icondefs * sizeof(Geo_feat_icondef_t *)); 
			}
                    }
                    if(ov->geo_icondef == NULL) {
                        fprintf(stderr,"Unable to allocate memory for Icon definition pointer array!\n");
                        exit(-1);
                    }

                    if(STRparse(buf,cfield,256,32,64) != 3) {
                        fprintf(stderr,"Error in ICONDEF line: %s\n",buf);
                        exit(-1);
                    }
                    /* get space for the icon definition */
                    ov->geo_icondef[index] = (Geo_feat_icondef_t *) ucalloc(1,sizeof(Geo_feat_icondef_t));
                    ZERO_STRUCT(ov->geo_icondef[index]);

                    if(ov->geo_icondef[index] == NULL) {
                        fprintf(stderr,"Unable to allocate memory for Icon definition!\n");
                        exit(-1);
                    }
                    STRcopy(ov->geo_icondef[index]->name,cfield[1],NAME_LENGTH);
                    num_points = atoi(cfield[2]);

                    /* Get space for points in the icon */
                    ov->geo_icondef[index]->x = (short *) ucalloc(1,num_points * sizeof(short));
                    ov->geo_icondef[index]->y = (short *) ucalloc(1,num_points * sizeof(short));

                     if(ov->geo_icondef[index]->x == NULL || ov->geo_icondef[index]->y == NULL) {
                        fprintf(stderr,"Error!: Unable to allocate space for icon points in file %s, num points: %d\n",
                            ov->map_file_name,num_points);
                        exit(-1);
                    }

                    /* Read in all of the points */
                    for(j=0,point = 0; j < num_points; j++) {
                        if(fgets(buf,256,infile) == NULL) continue;
                        if(STRparse(buf,cfield,256,32,64) == 2) {
                            ov->geo_icondef[index]->x[point] = atoi(cfield[0]);
                            ov->geo_icondef[index]->y[point] = atoi(cfield[1]);
                            point++;
                        }
                    }
                    ov->geo_icondef[index]->num_points = point;
                    ov->num_icondefs++;
                    continue;
                }

                if(strncmp(buf,"ICON ",5) == 0) {    
                    index = ov->num_icons;
                    if(index >= ov->num_alloc_icons) {
			if(ov->num_alloc_icons == 0) { /* start with space for 2 */
			   ov->geo_icon = (Geo_feat_icon_t **)
			       ucalloc(2,sizeof(Geo_feat_icon_t *));
			   ov->num_alloc_icons = 2;
			} else {  /* Double the space */
			   ov->num_alloc_icons *= 2;
			   ov->geo_icon = (Geo_feat_icon_t **) 
			    urealloc(ov->geo_icon, ov->num_alloc_icons * sizeof(Geo_feat_icon_t *)); 
			}
                    }
                    if(ov->geo_icon == NULL) {
                        fprintf(stderr,"Unable to allocate memory for Icon pointer array!\n");
                        exit(-1);
                    }

                    /* get space for the Icon */
                    ov->geo_icon[index] = (Geo_feat_icon_t *) ucalloc(1,sizeof(Geo_feat_icon_t));
                    ZERO_STRUCT(ov->geo_icon[index]);

                    if(ov->geo_icon[index] == NULL) {
                        fprintf(stderr,"Unable to allocate memory for Icon definition!\n");
                        exit(-1);
                    }

                    if((num_fields = STRparse(buf,cfield,256,32,64)) < 6) {
                        fprintf(stderr,"Error in ICON line: %s\n",buf);
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
                        continue;
                    }

                    /* record its position */
                    ov->geo_icon[index]->lat = atof(cfield[2]);
                    ov->geo_icon[index]->lon = atof(cfield[3]);
                    ov->geo_icon[index]->text_x = atoi(cfield[4]);
                    ov->geo_icon[index]->text_y = atoi(cfield[5]);

                    /* gather up remaining text fields */
                    ov->geo_icon[index]->label[0] = '\0';
                    len = 0;
                    for(j = 6; j < num_fields; j++ ) {
                        strncat(ov->geo_icon[index]->label,cfield[j],LABEL_LENGTH - len);
                        len = strlen(ov->geo_icon[index]->label) +1;
                        strncat(ov->geo_icon[index]->label," ",LABEL_LENGTH - len);
                        len = strlen(ov->geo_icon[index]->label) +1;
                    }

                    ov->num_icons++;
                    continue;
                }

                if(strncmp(buf,"POLYLINE",8) == 0) {    
                    index = ov->num_polylines;
                    if(index >= ov->num_alloc_polylines) {
			if(ov->num_alloc_polylines == 0) { /* start with space for 2 */
			   ov->geo_polyline = (Geo_feat_polyline_t **)
			       ucalloc(2,sizeof(Geo_feat_polyline_t *));
			   ov->num_alloc_polylines = 2;
			} else {  /* Double the space */
			   ov->num_alloc_polylines *= 2;
			   ov->geo_polyline = (Geo_feat_polyline_t **) 
			    urealloc(ov->geo_polyline, ov->num_alloc_polylines * sizeof(Geo_feat_polyline_t *)); 
			}
                    }
                    if(ov->geo_polyline == NULL) {
                        fprintf(stderr,"Unable to allocate memory for Polyline pointer array!\n");
                        exit(-1);
                    }

                    if((STRparse(buf,cfield,256,32,64)) != 3) {
                        fprintf(stderr,"Error in POLYLINE line: %s\n",buf);
                        exit(-1);
                    }
                    /* get space for the Polyline definition */
                    ov->geo_polyline[index] = (Geo_feat_polyline_t *) ucalloc(1,sizeof(Geo_feat_polyline_t));
                    ZERO_STRUCT(ov->geo_polyline[index]);

                    if(ov->geo_polyline[index] == NULL) {
                        fprintf(stderr,"Unable to allocate memory for Polyline definition!\n");
                        exit(-1);
                    }
                    STRcopy(ov->geo_polyline[index]->label,cfield[1],LABEL_LENGTH);
                    num_points = atoi(cfield[2]);

                    /* Get space for points in the icon */
                    ov->geo_polyline[index]->lat = (double *) ucalloc(1,num_points * sizeof(double));
                    ov->geo_polyline[index]->lon = (double *) ucalloc(1,num_points * sizeof(double));
                    ov->geo_polyline[index]->local_x = (double *) ucalloc(1,num_points * sizeof(double));
                    ov->geo_polyline[index]->local_y = (double *) ucalloc(1,num_points * sizeof(double));

                     if(ov->geo_polyline[index]->lat == NULL || ov->geo_polyline[index]->lon == NULL) {
                        fprintf(stderr,"Error!: Unable to allocate space for polyline points in file %s, num points: %d\n",
                            ov->map_file_name,num_points);
                        exit(-1);
                    }

                    /* Read in all of the points */
                    for(j=0,point = 0; j < num_points; j++) {
                        if(fgets(buf,256,infile) == NULL) continue;
                        if(STRparse(buf,cfield,256,32,64) == 2) {
                            ov->geo_polyline[index]->lat[point] = atof(cfield[0]);
                            ov->geo_polyline[index]->lon[point] = atof(cfield[1]);
                            point++;
                        }
                    }
                    ov->geo_polyline[index]->num_points = point;
                    ov->num_polylines++;
                    continue;
                }

                if(strncmp(buf,"LABEL",5) == 0) {    
                    index = ov->num_labels;
                    if(index >= ov->num_alloc_labels) {
			if(ov->num_alloc_labels == 0) { /* start with space for 2 */
			   ov->geo_label = (Geo_feat_label_t **)
			       ucalloc(2,sizeof(Geo_feat_label_t *));
			   ov->num_alloc_labels = 2;
			} else {  /* Double the space */
			   ov->num_alloc_labels *=2;
			   ov->geo_label = (Geo_feat_label_t **) 
			    urealloc(ov->geo_label, ov->num_alloc_labels * sizeof(Geo_feat_label_t *)); 
			}
                    }
                    if(ov->geo_label == NULL) {
                        fprintf(stderr,"Unable to allocate memory for Label pointer array!\n");
                        exit(-1);
                    }

                    ov->num_labels++;
		     
                    /* get space for the Label definition */
                    ov->geo_label[index] = (Geo_feat_label_t *) ucalloc(1,sizeof(Geo_feat_label_t));
                    ZERO_STRUCT(ov->geo_label[index]);

                    if(ov->geo_label[index] == NULL) {
                        fprintf(stderr,"Unable to allocate memory for Label definition!\n");
                        exit(-1);
                    }

                    if((num_fields = STRparse(buf,cfield,256,32,64)) < 8) {
                        fprintf(stderr,"Too few fields in LABEL line: %s\n",buf);
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
                    len = 0;
                    for(j = 8; j < num_fields; j++) {
                        strncat(ov->geo_label[index]->string,cfield[j],NAME_LENGTH - len);
                        len = strlen(ov->geo_label[index]->string) +1;
                        strncat(ov->geo_label[index]->string," ",NAME_LENGTH - len);
                        len = strlen(ov->geo_label[index]->string) +1;
                    }
                    continue;
                }


                if(strncmp(buf,"SIMPLELABEL",11) == 0) {    
                    index = ov->num_labels;
                    if(index >= ov->num_alloc_labels) {
			if(ov->num_alloc_labels == 0) { /* start with space for 2 */
			   ov->geo_label = (Geo_feat_label_t **)
			       ucalloc(2,sizeof(Geo_feat_label_t *));
			   ov->num_alloc_labels = 2;
			} else {  /* Double the space */
			   ov->num_alloc_labels *=2;
			   ov->geo_label = (Geo_feat_label_t **) 
			    urealloc(ov->geo_label, ov->num_alloc_labels * sizeof(Geo_feat_label_t *)); 
			}
                    }
                    if(ov->geo_label == NULL) {
                        fprintf(stderr,"Unable to allocate memory for Label pointer array!\n");
                        exit(-1);
                    }
                    ov->num_labels++;

                    /* get space for the Label definition */
                    ov->geo_label[index] = (Geo_feat_label_t *) ucalloc(1,sizeof(Geo_feat_label_t));
                    ZERO_STRUCT(ov->geo_label[index]);

                    if(ov->geo_label[index] == NULL) {
                        fprintf(stderr,"Unable to allocate memory for Label definition!\n");
                        exit(-1);
                    }

                    if((num_fields = STRparse(buf,cfield,256,32,64)) < 4) {
                        fprintf(stderr,"Too few fields in SIMPLELABEL line: %s\n",buf);
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
                    len = 0;
                    for(j = 3; j < num_fields; j++) {
                        strncat(ov->geo_label[index]->string,cfield[j],NAME_LENGTH - len);
                        len = strlen(ov->geo_label[index]->string) +1;
                        strncat(ov->geo_label[index]->string," ",NAME_LENGTH - len);
                        len = strlen(ov->geo_label[index]->string) +1;
                    }
                    continue;
                }

            }
            if(gd.debug) printf("Overlay File %s contains %d Polylines, %d Icon_defns, %d Icons, %d Labels\n",
            ov->map_file_name,ov->num_polylines,ov->num_icondefs,ov->num_icons,ov->num_labels);
            fclose(infile);
        }
    }

    for(i=0; i < 32; i++)  ufree(cfield[i]);         /* free space for sub strings */
    return SUCCESS;
}
