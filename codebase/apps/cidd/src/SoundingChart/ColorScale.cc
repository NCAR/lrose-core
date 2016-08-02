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
 * COLORSCALE.CC : Routines to read color table files and establish
 *     colormaps
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */


#define COLORSCALES_CC 

using namespace std;

#include "ColorScale.hh"
#include <cstdlib>

#define NUM_PARSE_FIELDS    8
#define PARSE_FIELD_SIZE    128
#define INPUT_LINE_LEN      1024
#define URL_MIN_SIZE        12
#define LABEL_LENGTH 32
#define MAX_COLOR_LABELS 16

/**********************************************************************
 * ColorScale: Constructor - Reads the Colorscale file
 *        This routine allocates memory for the entries.
 *        Returns number of entries found.
 */

ColorScale::ColorScale(Display *dpy,
                       const char *color_file_subdir,
                       const char *fname, /* file name */
                       int  debug_level,
                       const char *out_of_range_color
                       )
				   
{
    FILE   *cfile;
    struct stat sbuf;

    int    i,j;
    int    cs_len;
    int    ret_stat;
    int    nstrings;
    int    nentries;

    char   *ptr;
    char   *lptr;
    char   *lasts;
    char   *str_ptr;
    char   *cs_buf; // Colorscale buffer

    char    dirname[1024];
    char    name_buf[2048];

    char   buf[MAX_PATH_LEN];
    char   *cfield[NUM_PARSE_FIELDS];

    Val_color_t vct;
    XColor    rgb_def;

    debug = debug_level;

    display = dpy;
    Colormap cmap = DefaultColormap(dpy,DefaultScreen(dpy));

    cs_len = 0;
    cfile = NULL;

    // Try the local dir first
    if((cfile = fopen(fname,"r")) == NULL) {

	strncpy(name_buf,color_file_subdir,2048);

	str_ptr = strtok(name_buf,","); // Prime strtok

	do {  // Check each directory in the comma delimited  list

	  while(*str_ptr == ' ') str_ptr++; //skip any leading spaces

	  sprintf(buf,"%s/%s",str_ptr,fname);

	  // Check if its an HTTP URL
	  if(strncasecmp(buf,"http:",5) == 0) {
	      ret_stat =  HTTPgetURL(buf,10000, &cs_buf, &cs_len);

	    if(ret_stat <= 0) {
		cs_len = 0;
	    } else {
	      if(debug) fprintf(stderr,"Loading Colorscale %s ...",buf);
	    }


          } else {
	    // Try to open it in the subdir 
	    if((cfile = fopen(buf,"r")) == NULL) {
	      cs_len = 0;
	    } else {
	      if(debug) fprintf(stderr,"Opening Colorscale %s ...",buf);
	    }
	  }
        } while (cfile == NULL && (str_ptr = strtok(NULL,",")) != NULL && cs_len == 0 );

    } else {
	sprintf(buf,"%s",fname);
    }

    if(cfile !=NULL) {
       if(stat(buf,&sbuf) < 0) { // Find the file's size
             fprintf(stderr,"Can't stat %s\n",buf);
	     exit(-1);
	 }

	 // Allocate space for the whole file plus a null
	 if((cs_buf = (char *)  calloc(sbuf.st_size + 1 ,1)) == NULL) {
           fprintf(stderr,"Problems allocating %d bytes for colorscale file\n",
                   (int) sbuf.st_size);
	     exit(-1);
	}

	// Read
	if((cs_len = fread(cs_buf,1,sbuf.st_size,cfile)) != sbuf.st_size) {
	   fprintf(stderr,"Problems Reading %s\n",buf);
	   exit(-1);
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
            exit(-1);
    }

    /* Get temp storage for character strings */
    for(i=0;i < NUM_PARSE_FIELDS; i++) {
        cfield[i] = (char *)  calloc(1,PARSE_FIELD_SIZE);
    }

    nentries = 0;

    // Prime strtok;
    str_ptr = strtok_r(cs_buf,"\n",&lasts);

    /* loop thru buffer looking for valid entries */
    while (str_ptr  != NULL) {    
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
            /* Set the label for this color scale element */
            if((lptr != NULL) && strlen(lptr) > 0) {
               STRcopy(vct.label,lptr,LABEL_LENGTH);
            } else {
              vct.label[0] = '\0';
            }

            vct.min = atof(cfield[0]);        /* Extract mapping values */
            vct.max = atof(cfield[1]);    

            vct.cname[0] = '\0';
            for(j=2;j < nstrings; j++) {    /* Some names contain multiple strings so concatenate */
                strcat(vct.cname,cfield[j]);
                strcat(vct.cname," ");    
            }

            vct.cname[strlen(vct.cname)-1] = '\0'; /* chop off last space char */


	    XParseColor(display,cmap,vct.cname,&rgb_def);

	    XAllocColor(display,cmap,&rgb_def);

        vct.pixval = rgb_def.pixel; 

	    vct.r = rgb_def.red; 
	    vct.g = rgb_def.green; 
	    vct.b = rgb_def.blue; 
	    vct.a = 255;

	    // Add this entry to our Value to Color vector
	    vc.push_back(vct);
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

    if(vc.size() == 0) {
        fprintf(stderr,"No color map entries found in %s\n",fname);
    } else {
         if(debug) fprintf(stderr," Found %d Colors\n",vc.size());
    }

    strncpy(oor_color.cname,out_of_range_color,COLOR_NAME_LENGTH); 
    oor_color.label[0] = '\0';
    oor_color.min = 0.0;
    oor_color.max = 0.0;

    XParseColor(display,cmap,oor_color.cname,&rgb_def);
    XAllocColor(display,cmap,&rgb_def);

    oor_color.pixval = rgb_def.pixel; 
	 
    oor_color.r = rgb_def.red; 
    oor_color.g = rgb_def.green; 
    oor_color.b = rgb_def.blue; 
    oor_color.a = 255;

}

//////////////
// DESTRUCTOR

ColorScale::~ColorScale()
{
}

/*****************************************************************
 * VAL2COLOR: Return The proper Color given the value
 */

long  ColorScale::val2color(double val)
{
    for(uint i=0; i < vc.size(); i++) {
	    if(val >= vc[i].min && val < vc[i].max ) return vc[i].pixval;
    }

    return (oor_color.pixval);
}

/*****************************************************************
 * GET_MIN_VALUE: return the Color scale minimum value.
 */

double  ColorScale::get_min_value(void)
{
	double val1 = vc[0].min;
	double val2 = vc[vc.size()-1].min;

    if(val1 < val2) {
 	    return  val1;
	} else {
 	    return  val2;
	}
}

/*****************************************************************
 * GET_MAX_VALUE: return the Color scale maximum value.
 */

double  ColorScale::get_max_value(void)
{
	double val1 = vc[0].max;
	double val2 = vc[vc.size()-1].max;

    if(val1 > val2) {
 	    return  val1;
	} else {
 	    return  val2;
	}
}

/*****************************************************************
 * DRAW_COLORBAR: 
 */

void
ColorScale:: draw_colorbar(Display *dpy, Drawable xid, GC gc, XFontStruct *fst,  int x1,  int y1,  int width,  int height)
	      
{
    uint 	i;
    uint       len;
    int		decimate;
    int 	bar_wd;
    int 	bar_xstart;
    int 	bar_ystart;
    int		bar_yend;
    int 	bar_xend;
    int		y_pos, x_pos;
    int	     	bar_width;
    int	     	bar_height;
    int		orient;

    int         ival;
    int         reverse_scale;
    char    	label[LABEL_LENGTH];

    int direct,ascent,descent;
    XCharStruct overall;

    orient = 0;                     // Vertical orientation
    if(height < width) orient = 1;  // Horizontal orientation 

    switch(orient) {
    case 0:  /* Vertical orientation */
	bar_height = (int)(((double) height / ((double) vc.size() +2.5)) + 0.5);
	bar_width = width - 2;
    break;
    case 1:  /* Horizontal orientation */
	bar_width = (int)(((double) width / ((double) vc.size() +2.5)) + 0.5);
	bar_height = height - 2 ;
    break;
    }
	
	//fprintf(stderr,"ColorScale Len: %d\n",vc.size());

    // Determine if the order of the color scale goes low to high
    reverse_scale = vc[0].min < vc[vc.size() -1].min ? 0 : 1;
    
  switch(orient) {
  case 0:  /* Vertical orientation */
        decimate = vc.size()/ MAX_COLOR_LABELS + 1; /* don't put more than MAX_COLOR_LABELS labels on */
	if(decimate < 1) decimate = 1;

	bar_xstart = x1 + 1 ;
	bar_ystart = y1 + height - bar_height;
	x_pos = bar_xstart + (bar_width/2);

	if(reverse_scale) {
	  /* place a max val at the bottom of the scale if appropriate */
	  if(strlen(vc[0].label) == 0 && (fabs(vc[0].max - vc[0].min) != 1.0)) {
		y_pos = bar_ystart  + 1;
		ival =  (int) vc[vc.size()-1].max;   /* convert to an integer */
	    if(ival == vc[0].max) {
			sprintf(label,"%d",ival);
		} else {
			sprintf(label,"%.3g",vc[0].max);
		}
		if(strncmp(label,"       ",strlen(label)) != 0)  {
	  	    XTextExtents(fst,label,strlen(label),&direct,&ascent,&descent,&overall);
		    XDrawImageString(dpy,xid,gc,x_pos - (overall.width /2) , y_pos, label, strlen(label));
		}
	  }
	}
	 
	for(i=0; i < vc.size(); i++ ) {
	    bar_yend = bar_ystart - bar_height;
	    
	    /* fill in color rectangle */
	    XSetForeground(dpy,gc,vc[i].pixval);
	    //fprintf(stderr,"Box: %d,%d  %d X %d, Color %d\n", bar_xstart,bar_yend,bar_width,bar_height,vc[i].pixval);
	    XFillRectangle(dpy,xid,gc,bar_xstart,bar_yend,bar_width,bar_height);
	    /* Decimate the number of labels */
	    if(decimate == 1 || i % decimate == 0) {
	     
	      if ((len = strlen(vc[i].label)) > 0) {
		    STRcopy(label,vc[i].label,LABEL_LENGTH);
		
		    y_pos = bar_yend  + (bar_height/2);
			XTextExtents(fst,label,strlen(label),&direct,&ascent,&descent,&overall);
		    if(*label == ' ' ) { // Labels that Begin with Space have no bakground blanking
		        XDrawString(dpy,xid,gc, x_pos - (overall.width /2), y_pos, label, len);
			} else {
		        XDrawImageString(dpy,xid,gc, x_pos - (overall.width /2), y_pos, label, len);
			}

	      } else {	    /* numeric label */
		
		    if(fabs(vc[i].max - vc[i].min) == 1.0) {	/* If range == 1; center label */
		        y_pos =  (bar_yend  + (bar_height/2));
		        sprintf(label,"%g",vc[i].min);
		    } else {
		        if(reverse_scale) {	/* Place label at top of bar */
		                y_pos = bar_yend + 1;
		        } else {	/* Place label at bottom of bar */
		                y_pos = bar_ystart - 1;
		        }
		        ival = (int) vc[i].min;   /* convert to an integer */
		        if(ival == vc[i].min) {
				    sprintf(label,"%d",ival);
			    } else {
				    sprintf(label,"%.3g",vc[i].min);
			    }
		    }
		
		    if(strncmp(label,"       ",strlen(label)) != 0)  {
			    XTextExtents(fst,label,strlen(label),&direct,&ascent,&descent,&overall);
		        XDrawImageString(dpy,xid,gc,
	  	            x_pos  - (overall.width /2), y_pos, label, strlen(label));
		    }
	      }
	    }
	    bar_ystart = bar_yend;
	} 

	if(reverse_scale == 0) {
	  /* place a max val at the top of the scale if appropriate */
	  if(strlen(vc[vc.size()-1].label) == 0 && (fabs(vc[vc.size()-1].max - vc[vc.size()-1].min) != 1.0)) {
		y_pos = bar_ystart - 1;
		ival =  (int) vc[vc.size()-1].max;   /* convert to an integer */
	    if(ival == vc[vc.size()-1].max) {
			sprintf(label,"%d",ival);
		} else {
			sprintf(label,"%.3g",vc[vc.size()-1].max);
		}
		XTextExtents(fst,label,strlen(label),&direct,&ascent,&descent,&overall);
		XDrawImageString(dpy,xid,gc, x_pos - (overall.width /2), y_pos, label, strlen(label));
	  }
	}
	
  break;
	
  case 1:  /* Horizontal orientation */
        decimate = vc.size()/ 16 + 1; /* don't put more than 16 labels on */
	if(decimate < 1) decimate = 1;

	bar_xstart = bar_width + x1;
	bar_ystart = y1  + 1 ;
	
	if(reverse_scale) {
	  /* place a minval at the left side of the scale if appropriate */
	  if(strlen(vc[0].label) == 0 && (fabs(vc[vc.size()-1].max - vc[vc.size()-1].min) != 1.0)) {
		x_pos = bar_xstart + 2;
		ival =  (int) vc[0].min;   /* convert to an integer */
	    if(ival == vc[0].min) {
			sprintf(label,"%d",ival);
		} else {
			sprintf(label,"%.3g",vc[0].min);
		}
		XDrawImageString(dpy,xid,gc, x_pos, y_pos, label, strlen(label));
	  }
	}

	for(i=0; i < vc.size(); i++ ) {
	    bar_xend =  (int) ((bar_width + ((i +1) * bar_width) + 0.5))+ x1;
	    bar_wd   = bar_xend - bar_xstart;
	    
	    /* fill in color rectangle */
	    XSetForeground(dpy,gc,vc[i].pixval);
	    XFillRectangle(dpy,xid,gc,bar_xstart,bar_ystart,bar_wd,bar_height);
	    

	    /* Decimate the number of labels */
	    if(decimate == 1 || i % decimate == 0) {
	    
	      if ((len = strlen(vc[i].label)) != 0) {
		    STRcopy(label,vc[i].label,LABEL_LENGTH);
		
		    x_pos = bar_xstart + 2;
		    y_pos = bar_yend  + (bar_height/2);
		    if(*label == ' ' ) { // Labels that Begin with Space have no bakground blanking
		        XDrawString(dpy,xid,gc, x_pos, y_pos, label, len);
			} else {
		        XDrawImageString(dpy,xid,gc, x_pos, y_pos, label, len);
			}
	      } else { /* numeric label */
		
		if(fabs(vc[i].max - vc[i].min) == 1.0) {	/* If range == 1; center label */
		    x_pos =  bar_xstart  + 2;
		} else {	
		    if(reverse_scale) {	/* Place label at right edge of bar */
		        x_pos = bar_xstart + 2;
		    } else {	/* Place label at left edge of bar */
		        x_pos = bar_xstart - 1;
		    }
			ival =  (int) vc[i].min;   /* convert to an integer */
		    if(ival == vc[i].min) {
				sprintf(label,"%d",ival);
			} else {
				sprintf(label,"%.3g",vc[i].min);
		    }

		    y_pos =  (bar_yend  + (bar_height/2));
		
		    XDrawImageString(dpy,xid,gc, x_pos, y_pos, label, strlen(label));
	      }
	    }
	  }
	  bar_xstart = bar_xend;
	}

	if(reverse_scale == 0) {
	  /* place a max val at the right side of the scale if appropriate */
	  if(strlen(vc[vc.size()-1].label) == 0 && (fabs(vc[vc.size()-1].max - vc[vc.size()-1].min) != 1.0)) {
		x_pos = bar_xstart - 1;
		ival =  (int) vc[vc.size()-1].max;   /* convert to an integer */
	    if(ival == vc[vc.size()-1].max) {
			sprintf(label,"%d",ival);
		} else {
			sprintf(label,"%.3g",vc[vc.size()-1].max);
		}
		XDrawImageString(dpy,xid,gc, x_pos, y_pos, label, strlen(label));
	  }
	}
	
  break;
	
  }
   return;
}
