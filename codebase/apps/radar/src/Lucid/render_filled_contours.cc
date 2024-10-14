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
 * RENDER_FILLED_CONTOURS
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#include "cidd.h"

/**********************************************************************
 * COMPUTE_CONT_LEVELS:  Sets up Empty brush_array, and levels. Returns
 *  The number of levels
 */

int compute_cont_levels(QBrush  *brush_array, double *levels, met_record_t *mr, float *tmp_data)
{
   int i,j;
   int len;
   int num_levels;
   double c_val;
   float bad_data;
   float miss_data;
   float *ptr,*ptr2,*t_ptr;
   int using_temp = 0;

   if(mr->h_data == NULL) return 0;

   num_levels = 0;
   tmp_data = NULL;

   if(mr->h_mdvx->getField(0) == NULL) return 0;
   const Mdvx::field_header_t &fhdr = (mr->h_mdvx->getFieldByNum(0))->getFieldHeader(); 
   ptr = (float *)  (mr->h_mdvx->getFieldByNum(0))->getVol(); 

   // if(gd.legends.out_of_range_color->brush != NULL) {

   // Cap the bottom end of the scale.
   levels[num_levels] = FLT_MIN;
   // Default color is background.
   brush_array[num_levels] =  gd.legends.out_of_range_color->brush;
   num_levels++;
   
   // }

   if(mr->h_vcm.vc[0]->min < mr->h_vcm.vc[mr->h_vcm.nentries -1]->max) {
     for(i=0; i < mr->h_vcm.nentries; i++) {
       if (fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
     c_val = log(mr->h_vcm.vc[i]->min);
       } else {
     c_val = mr->h_vcm.vc[i]->min;
       }

       // Make sure the data doesn't map to bad or missing - Could be 0 and 1
       if( c_val != fhdr.bad_data_value &&
           c_val != fhdr.missing_data_value) {
           brush_array[num_levels] =  mr->h_vcm.vc[i]->brush;
           levels[num_levels++] = c_val;
       }
     }

     if (fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
     c_val = log(mr->h_vcm.vc[mr->h_vcm.nentries-1]->max);
     } else {
     c_val = mr->h_vcm.vc[mr->h_vcm.nentries-1]->max;
     }
     if( c_val != fhdr.bad_data_value &&
     c_val != fhdr.missing_data_value) { 
     brush_array[num_levels] =  mr->h_vcm.vc[mr->h_vcm.nentries-1]->brush;
     levels[num_levels++] = c_val;
     }

   } else { // Reverse the order - Filled contouring assumes monotonically increasing levels.

     for(i=mr->h_vcm.nentries -1, j = 0; j < mr->h_vcm.nentries; i--,j++) {
       if (fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
     c_val = log(mr->h_vcm.vc[i]->min);
       } else {
     c_val = mr->h_vcm.vc[i]->min;
       }


       // Make sure the data doesn't map to bad or missing
       if( c_val != fhdr.bad_data_value && 
       c_val != fhdr.missing_data_value) {
           brush_array[num_levels] =  mr->h_vcm.vc[i]->brush;
           levels[num_levels++] = c_val;
       }
     }

     if (fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
     c_val = log(mr->h_vcm.vc[0]->max);
     } else {
     c_val = mr->h_vcm.vc[0]->max;
     }
     if( c_val != fhdr.bad_data_value &&
     c_val != fhdr.missing_data_value) { 
     brush_array[num_levels] =  mr->h_vcm.vc[0]->brush;
     levels[num_levels++] = c_val;
     }
   }

   // if(gd.legends.out_of_range_color->brush != NULL) {
   /* Max out the top contour level to avoid "holes" in the graphics */
   levels[num_levels] = FLT_MAX;
   brush_array[num_levels] =  gd.legends.out_of_range_color->brush;
   num_levels++;
   // }

   bad_data =  fhdr.bad_data_value;
   miss_data =  fhdr.missing_data_value;

   if( bad_data !=  miss_data) { // Map bad to missing
       using_temp = 1;
       if((tmp_data = (float *) calloc(fhdr.nx*fhdr.ny,sizeof(float))) == NULL) {
        perror("CIDD: Alloc failure");
        exit(-1);
       }

       t_ptr = ptr;
       ptr2 = tmp_data;
       len = fhdr.nx*fhdr.ny;
       while(len) {
       if(*t_ptr == bad_data) {
           *ptr2++  = miss_data;
               t_ptr++;
       } else {
           *ptr2++ = *t_ptr++;
       }
       len--;
       }
    }

   if( gd.layers.map_missing_to_min_value) { // Map missing to Min value in levels
       if(!using_temp) { // Allocate a temporary array
         using_temp = 1;
         if((tmp_data = (float *) calloc(fhdr.nx*fhdr.ny,sizeof(float))) == NULL) {
        perror("CIDD: Alloc failure");
        exit(-1);
         }
         t_ptr = ptr;
       } else {  // Already have temp_array 
         t_ptr = tmp_data;
       }
     
       ptr2 = tmp_data;
       len = fhdr.nx*fhdr.ny;
       while(len) {
       if(*t_ptr == miss_data) {
           *ptr2++ = fhdr.min_value;
               t_ptr++;
       } else {
           *ptr2++ = *t_ptr++;
       }
       len--;
       }
    }

   if( gd.layers.map_bad_to_min_value) { // Map bad data to Min value in levels
       if(!using_temp) { // Allocate a temporary array
         using_temp = 1;
         if((tmp_data = (float *) calloc(fhdr.nx*fhdr.ny,sizeof(float))) == NULL) {
        perror("CIDD: Alloc failure");
        exit(-1);
         }
         t_ptr = ptr;
       } else {  // Already have temp_array 
         t_ptr = tmp_data;
       }
     
       ptr2 = tmp_data;
       len = fhdr.nx*fhdr.ny;
       while(len) {
       if(*t_ptr == bad_data) {
           *ptr2++ = fhdr.min_value;
               t_ptr++;
       } else {
           *ptr2++ = *ptr++;
       }
       len--;
       }
    }

   return num_levels;
}

/**********************************************************************
 * DRAW_FILLED_CONTOURS: Render an image using filled contours on a
 * rectangular grid
 */

void draw_filled_contours( QPaintDevice *pdev, int x_start[], int y_start[], met_record_t *mr)
{
#ifdef NOTYET
   int num_levels;
   QBrush    brush_array[MAX_COLORS];
   double levels[MAX_COLORS];
   float *ptr,*t_ptr;
   int using_temp = 0;
   float *tmp_data = NULL;

   if(mr->h_data == NULL) return;

   num_levels =  compute_cont_levels(brush_array, levels, mr, tmp_data);
   if(num_levels == 0) return;

   if(tmp_data != NULL) using_temp = 1;

   if(mr->h_mdvx->getField(0) == NULL) return;

   const Mdvx::field_header_t &fhdr = (mr->h_mdvx->getFieldByNum(0))->getFieldHeader(); 
   ptr = (float *)  (mr->h_mdvx->getFieldByNum(0))->getVol(); 


   if(gd.layers.missing_data_color->brush != NULL) {
       XFillRectangle(gd.dpy,xid,gd.layers.missing_data_color->brush,
              x_start[0], y_start[fhdr.ny-1],
              (x_start[fhdr.nx-2] - x_start[0]),
              (y_start[0] - y_start[fhdr.ny-2]));
   }

   if(gd.layers.bad_data_color->brush != NULL) {
       XFillRectangle(gd.dpy,xid,gd.layers.bad_data_color->brush,
              x_start[0],y_start[fhdr.ny-1],
              (x_start[fhdr.nx-2] - x_start[0]),
              (y_start[0] - y_start[fhdr.ny-2]));
   }


   int cont_features = RASCON_NOLABELS;
   if (fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
     cont_features |= RASCON_DATA_IS_LOG;
   }
   if(gd.layers.smooth_contours == 1 )  cont_features |= RASCON_DOSMOOTH; 
   if(gd.layers.smooth_contours == 2 )  cont_features |= RASCON_DOSMOOTH2;
   if(gd.layers.add_noise)  cont_features |= RASCON_ADDNOISE;

   RASCONinit(gd.dpy,xid,fhdr.nx,fhdr.ny,x_start,y_start);
   if(using_temp) { 
       t_ptr = tmp_data;
   } else {
       t_ptr = ptr;
   }
   RASCONcontour(brush_array,t_ptr,(void *) &(fhdr.missing_data_value),RASCON_FLOAT,
      num_levels,
      levels,
      RASCON_FILLED_CONTOURS, cont_features,
      1.0,0.0);

   if(using_temp) free(tmp_data);
#endif

}

/**********************************************************************
 * DRAW_FILLED_CONTOURS_D: Render an image using filled contours on a
 * Distorted grid
 */

void draw_filled_contours_d( QPaintDevice *pdev,  met_record_t *mr)
{
#ifdef NOTYET
   int i,j;
   int num_levels;
   QBrush    brush_array[MAX_COLORS];
   double levels[MAX_COLORS];
   float *ptr;
   int using_temp = 0;
   float *tmp_data = NULL;

   RASCONBoxEdge *bedge; 
   double lat,lon;
   double x_km,y_km;
   int x1,y1,x2,y2;
   float *row1_ptr;
   float *row2_ptr;

   if(mr->h_data == NULL) return;

   // tmp_data will either be NULL or will be allocated in compute_cont_levels()
   num_levels =  compute_cont_levels(brush_array, levels, mr, tmp_data);

   if(tmp_data != NULL) using_temp = 1;

   if(num_levels == 0){
     if(using_temp) free(tmp_data);
     return;
   }

   if(mr->h_mdvx->getField(0) == NULL){
     if(using_temp) free(tmp_data);
     return;
   }

   const Mdvx::field_header_t &fhdr = (mr->h_mdvx->getFieldByNum(0))->getFieldHeader(); 
   if(using_temp) { 
       ptr = tmp_data;
   } else {
       ptr = (float *)  (mr->h_mdvx->getFieldByNum(0))->getVol(); 
   }


   if((bedge = (RASCONBoxEdge *) calloc(fhdr.nx,sizeof(RASCONBoxEdge))) == NULL) {
         perror("Unable to allocate temp space for contours ");
	 if(using_temp) free(tmp_data);
        return;
   }

   mr->proj->xyIndex2latlon(0,0,lat,lon); // Grid to World 
   gd.proj.latlon2xy(lat,lon,x_km,y_km); // World to Map
   disp_proj_to_pixel(&(gd.h_win.margin),x_km,y_km,&x1,&y1);

   mr->proj->xyIndex2latlon((int)fhdr.nx-1,(int)fhdr.ny-1,lat,lon); // Grid to World 
   gd.proj.latlon2xy(lat,lon,x_km,y_km); // World to Map
   disp_proj_to_pixel(&(gd.h_win.margin),x_km,y_km,&x2,&y2);

   if(gd.layers.missing_data_color->brush != NULL) {
       XFillRectangle(gd.dpy,xid,gd.layers.missing_data_color->brush,
              x1, y1,x2-x1,y2-y1);
   }

   if(gd.layers.bad_data_color->brush != NULL) {
       XFillRectangle(gd.dpy,xid,gd.layers.bad_data_color->brush,
              x1, y1,x2-x1,y2-y1);
   }


   int cont_features = RASCON_NOLABELS;
   if (fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
     cont_features |= RASCON_DATA_IS_LOG;
   }
   if(gd.layers.smooth_contours == 1 )  cont_features |= RASCON_DOSMOOTH; 
   if(gd.layers.smooth_contours == 2 )  cont_features |= RASCON_DOSMOOTH2;
   if(gd.layers.add_noise)  cont_features |= RASCON_ADDNOISE;

   // Compute the first row of points
   int clip_flag = 0;
   int minx = -CLIP_BUFFER;
   int miny = -CLIP_BUFFER;
   int maxx = gd.h_win.can_dim.width + CLIP_BUFFER;
   int maxy = gd.h_win.can_dim.height + CLIP_BUFFER;

   for(i=0; i < fhdr.nx; i++) {  // for each Column
	   clip_flag = 0;
       mr->proj->xyIndex2latlon(i,0,lat,lon); // Grid to World 
       gd.proj.latlon2xy(lat,lon,x_km,y_km); // World to Map

       // Map to Screen
       disp_proj_to_pixel(&(gd.h_win.margin),x_km,y_km,&(bedge[i].x2),&(bedge[i].y2));
	   if(_params.check_clipping) {
   		if(bedge[i].x2 < minx) clip_flag = 1; 
   		if(bedge[i].x2 > maxx) clip_flag = 1;
   		if(bedge[i].y2 < miny) clip_flag = 1;
   		if(bedge[i].y2 > maxy) clip_flag = 1;
		if(clip_flag) *(ptr+i) = fhdr.missing_data_value;
	   }
   }

   double ratio = 0.0;
   int diffx = 0;
   int diffy = 0;
   for(j = 1; j < fhdr.ny; j++) { // For each row
       for(i=0; i < fhdr.nx; i++) {  // for each Column
           // First rotate point 2 to point 1.
           bedge[i].x1 = bedge[i].x2;
           bedge[i].y1 = bedge[i].y2;

           mr->proj->xyIndex2latlon(i,j,lat,lon); // Grid to World
           gd.proj.latlon2xy(lat,lon,x_km,y_km); // World to Map

           // Map to Screen 
           disp_proj_to_pixel(&(gd.h_win.margin),x_km,y_km,&(bedge[i].x2),&(bedge[i].y2));
	       if(_params.check_clipping) {
	        clip_flag = 0;
   		    if(bedge[i].x2 < minx) clip_flag = 1; 
   		    if(bedge[i].x2 > maxx) clip_flag = 1;
   		    if(bedge[i].y2 < miny) clip_flag = 1;
   		    if(bedge[i].y2 > maxy) clip_flag = 1;

			if( i > 0) { 
			  diffx = abs(bedge[i].x2 - bedge[i-1].x2) +1;
			  diffy = abs(bedge[i].y2 - bedge[i-1].y2) +1;
			  ratio = (double) diffy / diffx;
		      if(ratio > MAX_ASPECT_RATIO ) clip_flag = 1;
		      if(ratio < 1/MAX_ASPECT_RATIO ) clip_flag = 1;
			}
		  }
		  if(clip_flag) {
				 *(ptr + (j * fhdr.nx) +i) = fhdr.missing_data_value;
		  }
	  }

        row1_ptr = ptr + ((j-1) * fhdr.nx);
        row2_ptr = ptr + (j * fhdr.nx);

        RASCONcontour2Rows(gd.dpy,xid,brush_array,fhdr.nx,j,
                   (char *) row1_ptr, (char *) row2_ptr, (char *) &(fhdr.missing_data_value),
                   RASCON_FLOAT, num_levels,levels,bedge,
                   RASCON_FILLED_CONTOURS, cont_features, 1.0,0.0);


   }

   if(using_temp) free(tmp_data);
   free(bedge);
   return;

#endif

}

/**********************************************************************
 * DRAW_XSECT_FILLED_CONTOURS: Render an image using filled contours 
 *
 */

void draw_xsect_filled_contours( QPaintDevice *pdev, int x_start[], int y_start[], met_record_t *mr)
{

#ifdef NOTYET
  int i,j;
   int len;
   int num_levels;
   QBrush    brush_array[MAX_COLORS];
   double levels[MAX_COLORS];

   double c_val;
   float bad_data;
   float miss_data;
   float *ptr,*ptr2,*t_ptr = NULL;
   float *tmp_data = NULL;

   int using_temp = 0;

   if(mr->v_data == NULL) return;
   if(mr->v_mdvx->getField(0) == NULL) return;

   const Mdvx::field_header_t &fhdr = (mr->v_mdvx->getFieldByNum(0))->getFieldHeader(); 
   ptr = (float *)  (mr->v_mdvx->getFieldByNum(0))->getVol();

   num_levels = 0;
   // Cap the bottom end of the scale.
   if(gd.legends.out_of_range_color->brush != NULL) {
       levels[num_levels] = FLT_MIN; 
       brush_array[num_levels] =  gd.legends.out_of_range_color->brush;
       num_levels++;
   }

   if(mr->v_vcm.vc[0]->min < mr->v_vcm.vc[mr->v_vcm.nentries -1]->max) {
     for(i=0; i < mr->v_vcm.nentries; i++) {
       if (fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
     c_val = log(mr->v_vcm.vc[i]->min);
       } else {
     c_val = mr->v_vcm.vc[i]->min;
       }
       brush_array[num_levels] =  mr->v_vcm.vc[i]->brush;
       levels[num_levels++] = c_val;
     }

     if (fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
     c_val = log(mr->v_vcm.vc[mr->v_vcm.nentries-1]->max);
     } else {
     c_val = mr->v_vcm.vc[mr->v_vcm.nentries-1]->max;
     }
     if( c_val != fhdr.bad_data_value &&
     c_val != fhdr.missing_data_value) { 
     brush_array[num_levels] =  mr->v_vcm.vc[mr->v_vcm.nentries-1]->brush;
     levels[num_levels++] = c_val;
     }

   } else { // Reverse the order - Filled contouring assumes monotonically increasing levels.

     for(i=mr->v_vcm.nentries -1, j = 0; j < mr->v_vcm.nentries; i--,j++) {

       if (fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
     c_val = log(mr->v_vcm.vc[i]->min);
       } else {
     c_val = mr->v_vcm.vc[i]->min;
       }

       brush_array[num_levels] =  mr->v_vcm.vc[i]->brush;
       levels[num_levels++] = c_val;

     }

     if (fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
     c_val = log(mr->v_vcm.vc[0]->max);
     } else {
     c_val = mr->v_vcm.vc[0]->max;
     }
     if( c_val != fhdr.bad_data_value &&
     c_val != fhdr.missing_data_value) { 
     brush_array[num_levels] =  mr->v_vcm.vc[0]->brush;
     levels[num_levels++] = c_val;
     }
   }

   if(gd.legends.out_of_range_color->brush != NULL) { 
       /* Max out the top contour level to avoid "holes" in the graphics */
       levels[num_levels] = FLT_MAX;
       brush_array[num_levels] =  gd.legends.out_of_range_color->brush;
       num_levels++;
   }

   bad_data =  fhdr.bad_data_value;
   miss_data = fhdr.missing_data_value;

   if( bad_data !=  miss_data) { // Map bad to missing
       using_temp = 1;
       if((tmp_data = (float* ) calloc(fhdr.nx*fhdr.nz,sizeof(float))) == NULL) {
            perror("CIDD: Alloc failure");
            exit(-1);
       }

       len = fhdr.nx*fhdr.nz;
       t_ptr = ptr;
       ptr2 = tmp_data;
       while(len) {
           if(*t_ptr == bad_data) {
           *ptr2++ = miss_data;
               t_ptr++;
       } else {
           *ptr2++ = *t_ptr++;
       }
       len--;
       }
    }
        
   if( gd.layers.map_missing_to_min_value) { // Map missing to Min value in levels
       if(!using_temp) { // Allocate a temporary array
         using_temp = 1;
         if((tmp_data = (float* ) calloc(fhdr.nx*fhdr.nz,sizeof(float))) == NULL) {
            perror("CIDD: Alloc failure");
            exit(-1);
         }
         t_ptr = t_ptr;
       } else {  // Already have temp_array
         ptr = tmp_data;
       }

       ptr2 = tmp_data;
       len = fhdr.nx*fhdr.nz;
       while(len) {
           if(*t_ptr == miss_data) {
           *ptr2++ = fhdr.min_value;
               t_ptr++;
       } else {
           *ptr2++ = *t_ptr++;
       }
           len--;
       }
    }
     
   if( gd.layers.map_bad_to_min_value) { // Map bad data to Min value in levels
       if(!using_temp) { // Allocate a temporary array
         using_temp = 1;
         if((tmp_data = (float* ) calloc(fhdr.nx*fhdr.nz,sizeof(float))) == NULL) {
            perror("CIDD: Alloc failure");
            exit(-1);
         }
         t_ptr = ptr;
       } else {  // Already have temp_array
         ptr = tmp_data;
       }

       ptr2 = tmp_data;
       len = fhdr.nx*fhdr.nz;
       while(len) {
           if(*t_ptr == bad_data) {
               *ptr2++ = fhdr.min_value;
               t_ptr++;
           } else {
               *ptr2++ = *t_ptr++;
           }
           len--;
       }
    }

   int cont_features = RASCON_NOLABELS;
   if (fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
     cont_features |= RASCON_DATA_IS_LOG;
   }
   if(gd.layers.smooth_contours == 1 )  cont_features |= RASCON_DOSMOOTH; 
   if(gd.layers.smooth_contours == 2 )  cont_features |= RASCON_DOSMOOTH2;
   if(gd.layers.add_noise)  cont_features |= RASCON_ADDNOISE;

   if(using_temp) { 
       t_ptr = tmp_data;
   } else {
       t_ptr = ptr;
   }
   RASCONinit(gd.dpy,xid,fhdr.nx,fhdr.nz,x_start,y_start);
   RASCONcontour(brush_array,t_ptr,&bad_data,RASCON_FLOAT,
      num_levels ,
      levels,
      RASCON_FILLED_CONTOURS, cont_features,
      1.0,0.0);

   if(using_temp) free(tmp_data);

#endif
   
}

