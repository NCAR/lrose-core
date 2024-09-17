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
 * RENDER_XSECT_LINE_CONTOURS.CC
 *        
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define RENDER_XSECT_LINE_CONTOURS
#include "cidd.h"

#define IDEAL_NUM_VERT_CONT_LEVELS 15.0

void render_xsect_line_contours(QPaintDevice *pdev,contour_info_t *crec)
{
    int i,len;
    int tmp,num_vals;
    int cont_features;
    int startx,starty,endx,endy;
    int    x_grid[MAX_COLS],y_grid[MAX_ROWS];
    int using_temp = 0;

    double high_avg,low_avg, dyn_range,c_interval,start_value;
    double dist;
    double delt;
    double levels[MAX_CONT_LEVELS];

    float    val;
    float    bad_data;
    float    miss_data;
    float   *ptr,*t_ptr;
    float   *ptr2 = NULL;
    float *tmp_data = NULL;
    met_record_t *mr;       /* convienence pointer to a data record */

    GC    gc_array[MAX_CONT_LEVELS];
    GC    special_value_gc = gd.def_gc;
    GC    temp_gc;

    mr = gd.mrec[crec->field];

    MdvxField *f = mr->v_mdvx->getField(0);
    if(f == NULL) return;

    const Mdvx::field_header_t &fhdr = f->getFieldHeader();

    ptr = (float *) f->getVol(); // Float data

    if(ptr == NULL) return;

        /* Calculate data to Pixel mapping */
        disp_proj_to_pixel_v(&gd.v_win.margin,0.0,mr->vert[0].min,&startx,&starty);
    
        dist = gd.h_win.route.total_length;
        disp_proj_to_pixel_v(&gd.v_win.margin,dist,mr->vert[fhdr.nz -1].max,&endx,&endy);
    
        if(startx > endx) {
            tmp = startx;
            startx = endx;
            endx = tmp;
         }

	 // compute average of lowest row;
	t_ptr = ptr;
	num_vals = 0;
	low_avg = 0.0;
        for(i=0; i < fhdr.nx; i++,t_ptr++) {
	  if( *t_ptr != fhdr.missing_data_value) {
	    if (fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
	      low_avg += exp(*t_ptr);
	    } else {
	      low_avg += *t_ptr;
	    }
	      num_vals++;
	  }
        }
	if(num_vals > 0) low_avg /= num_vals;

	// compute average of highest plane
	t_ptr = ptr + ((fhdr.nz -1) * fhdr.nx);
	num_vals = 0;
	high_avg = 0.0;
        for(i=0; i < fhdr.nx; i++,t_ptr++) {
	  if( *t_ptr != fhdr.missing_data_value) {
	    if (fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
	      high_avg += exp(*t_ptr);
	    } else {
	      high_avg += *t_ptr;
	    }
	      num_vals++;
	  }
        }
	if(num_vals > 0) high_avg /= num_vals;

	dyn_range = fabs(high_avg - low_avg);

	// Compute a reasonable contour interval 
	c_interval=  compute_cont_interval(dyn_range  / IDEAL_NUM_VERT_CONT_LEVELS);

	// Compute a contour minimum that allows us to use the special value
	num_vals = (int) ((gd.layers.special_contour_value - crec->min )/ c_interval);
	num_vals += 1; // make sure to exceed the range of the minimum
	start_value = gd.layers.special_contour_value - (num_vals * c_interval);

	// Compute where the grid cells are..
        delt = (endx - startx) / (double) (fhdr.nx -1);
        for(i=0; i < fhdr.nx; i++) {
          x_grid[i] = (int)(startx + (i * delt));
		  //fprintf(stderr,"XGRID %d: %d\n",i,x_grid[i]);
        }

        for(i=0; i < fhdr.nz; i++) {
          disp_proj_to_pixel_v(&gd.v_win.margin,dist,mr->vert[i].max,&endx,&endy);
          y_grid[i] = endy;
		  //fprintf(stderr,"YGRID %d: %d\n",i,y_grid[i]);
        }
     
	 
	cont_features = 0;
	if(crec->labels_on)  cont_features |= RASCON_DOLABELS;
        if(gd.layers.smooth_contours == 1 )  cont_features |= RASCON_DOSMOOTH;
	if(gd.layers.smooth_contours == 2 )  cont_features |= RASCON_DOSMOOTH2;
	if(gd.layers.add_noise)  cont_features |= RASCON_ADDNOISE;
	if (fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
	  cont_features |= RASCON_DATA_IS_LOG;
	}

		// PICK Cont levels based on dynamic range.
        crec->num_levels = 0;
        val = start_value;
        for(i=0; (i < MAX_CONT_LEVELS ) && (val <= crec->max);i++) {
             val = start_value + ( i * c_interval);
             levels[crec->num_levels] = val;
	     temp_gc = Val2GC(&mr->v_vcm, (double)val);
	     if(temp_gc == 0) temp_gc = crec->color->gc;

	     // Check for the Special Contour value 
	     if(val == gd.layers.special_contour_value) {
                     gc_array[crec->num_levels] =  special_value_gc;

		     XCopyGC(gd.dpy,temp_gc,0xFFFFFFFF,special_value_gc);

	             XSetLineAttributes(gd.dpy, gc_array[crec->num_levels],
	                 gd.layers.contour_line_width*2 , LineSolid, CapButt, JoinMiter);

	     } else {
                     gc_array[crec->num_levels] = temp_gc;
	             XSetLineAttributes(gd.dpy, gc_array[crec->num_levels],
	                 gd.layers.contour_line_width + 1, LineSolid, CapButt, JoinMiter);
	     }
             crec->num_levels++;
	     // Choose a small font for cross sections 
	     XSetFont(gd.dpy,crec->color->gc,gd.ciddfont[gd.prod.prod_font_num]);
        }

        crec->num_levels = 0;
        val = crec->min;
        for(i=0; (crec->num_levels < MAX_CONT_LEVELS ) && (val <= crec->max);i++) {
            val = crec->min + ( i * crec->interval);
            levels[crec->num_levels] = val;
        	if(fabs(levels[crec->num_levels] - gd.layers.special_contour_value) < 10e-6) {
              XCopyGC(gd.dpy,crec->color->gc,0xFFFFFFFF,special_value_gc);
              gc_array[crec->num_levels] = special_value_gc;
              XSetLineAttributes(gd.dpy, gc_array[crec->num_levels],
                 gd.layers.contour_line_width * 2 , LineSolid, CapButt, JoinMiter);
        	} else {
                  gc_array[crec->num_levels] = crec->color->gc;
                  XSetLineAttributes(gd.dpy, gc_array[crec->num_levels],
                  gd.layers.contour_line_width , LineSolid, CapButt, JoinMiter);
      		}
            crec->num_levels++;
      	}
        XSetFont(gd.dpy,crec->color->gc,gd.ciddfont[gd.prod.prod_font_num]);
        XSetLineAttributes(gd.dpy, crec->color->gc,
         gd.layers.contour_line_width , LineSolid, CapButt, JoinMiter);

        bad_data =  fhdr.bad_data_value;
        miss_data = fhdr.missing_data_value; 


       if( bad_data !=  miss_data) { // Map bad to missing
           using_temp = 1;
           if((tmp_data = (float *) calloc(fhdr.nx*fhdr.nz,sizeof(float))) == NULL) {
                perror("CIDD: Alloc failure");
                exit(-1);
           }

	   ptr2 = tmp_data;

           len = fhdr.nx*fhdr.nz;
           while(len) {
               if(*ptr2 == bad_data) {
		   *t_ptr++ = miss_data;
		   ptr2++;
	       } else {
		   *t_ptr++ = *ptr++;
	       }
               len--;
           }
        }

       if( gd.layers.map_missing_to_min_value) { // Map missing to Min value in levels
           if(!using_temp) { // Allocate a temporary array
             using_temp = 1;
             if((tmp_data = (float *) calloc(fhdr.nx*fhdr.nz,sizeof(float))) == NULL) {
                perror("CIDD: Alloc failure");
                exit(-1);
             }
             ptr2 = ptr;
           } else {  // Already have temp_array
             ptr2 = tmp_data;
           }

	   t_ptr = tmp_data;
           len = fhdr.nx*fhdr.nz;
           while(len) {
               if(*ptr2 == miss_data) {
		   *t_ptr++ = fhdr.min_value;
                   ptr2++;
	       } else {
		   *t_ptr++ = *ptr++;
	       }
               len--;
           }
        }

         if( gd.layers.map_bad_to_min_value) { // Map bad data to Min value in levels
           if(!using_temp) { // Allocate a temporary array
             using_temp = 1;
             if((tmp_data = (float *)  calloc(fhdr.nx*fhdr.nz,sizeof(float))) == NULL) {
                perror("CIDD: Alloc failure");
                exit(-1);
             }
             ptr2 = ptr;
           } else {  // Already have temp_array
             ptr = tmp_data;
           }

	   t_ptr = tmp_data;
           len = fhdr.nx*fhdr.nz;
           while(len) {
               if(*ptr2 == bad_data) {
		   *t_ptr =  fhdr.min_value;
                   ptr2++;
	       } else {
		   *t_ptr++ = *ptr2++;
	       }
               len--;
           }
        }
    
        RASCONinit(gd.dpy,xid,fhdr.nx,fhdr.nz,x_grid,y_grid);

	if(using_temp) {
	    ptr2 = tmp_data;
	} else {
	    ptr2 = ptr;
	}
        RASCONcontour(gc_array,ptr2,&bad_data,RASCON_FLOAT,
            crec->num_levels,
            levels,
            RASCON_LINE_CONTOURS, cont_features ,
            1.0,0.0);

    if(using_temp) free(tmp_data);

    return;
}
