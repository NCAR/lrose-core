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
/**********************************************************************
 * CIDD_COLORS.C : Routines to read color table files and establish
 *         colormaps
 *
 *
 * For the Cartesian Radar Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#include <unistd.h>

#include <X11/Xlib.h>

#define    GET_COLORS    1
#include "cidd.h"

#include <toolsa/os_config.h>
#include <toolsa/str.h>
#include <rapplot/xrs.h>


#define NUM_PARSE_FIELDS      8
#define PARSE_FIELD_SIZE     32
#define INPUT_LINE_LEN      128

/**********************************************************************
 * COMBINE_COLOR_MAPS: Allocates colors and cells (in pseudocolor mode )
 * for color mapping based on   total number of unique colors used in
 * all the color scale files, plus  a    short list of colors used
 * for overlay drawing.  Fills in the global color array: gd.color[] 
 */

int combine_color_maps(dpy,cmap)
    Display    *dpy;
    Colormap    cmap;
{
    int    i,j,k;
    int    status;
    int    flags;
    int    found;
    int    nentries;        /* number of color entries in a color scale */
    int    v_class;
    unsigned long    mask; /*  */
    unsigned long    pix_vals[MAX_COLORS];
    double    mult;
    char    *cname;
    char    *color_file_subdir;
    char    *color_name;
    char    buf[256];
    char    *ptr;

    XColor    rgb_def;
    XColor    rgb_exact;
    XGCValues gcvalues;    
    FILE    *color_file;


    color_file_subdir = XRSgetString(gd.cidd_db, "cidd.color_file_subdir", "colorscales");
     

    gd.num_colors = 0;
    gd.num_draw_colors = 0;

   /* Start with colors for Background, Foreground */
   /* NOTE: IT is Assumed later that color 0 is the background! */
   color_name = XRSgetString(gd.cidd_db, "cidd.background_color", "Black");
   STRcopy(gd.color[gd.num_colors].name,color_name,NAME_LENGTH);
   gd.extras.background_color = &(gd.color[gd.num_colors]);
   gd.num_colors++;

   color_name = XRSgetString(gd.cidd_db, "cidd.foreground_color", "White");
   STRcopy(gd.color[gd.num_colors].name,color_name,NAME_LENGTH);
   gd.extras.foreground_color = &(gd.color[gd.num_colors]);
   gd.num_colors++;


    /* Now Add Colors Necessary for PRDS */
    STRcopy(gd.color[2].name,PRDS_COLOR0,NAME_LENGTH);
    STRcopy(gd.color[3].name,PRDS_COLOR1,NAME_LENGTH);
    STRcopy(gd.color[4].name,PRDS_COLOR2,NAME_LENGTH);
    STRcopy(gd.color[5].name,PRDS_COLOR3,NAME_LENGTH);
    STRcopy(gd.color[6].name,PRDS_COLOR4,NAME_LENGTH);
    STRcopy(gd.color[7].name,PRDS_COLOR5,NAME_LENGTH);
    STRcopy(gd.color[8].name,PRDS_COLOR6,NAME_LENGTH);
    STRcopy(gd.color[9].name,PRDS_COLOR7,NAME_LENGTH);

    /* Attach the product color pointers */
    for(i=0; i < PRDS_NUM_COLORS; i++) {
	 gd.extras.prod.prds_color[i] = &gd.color[i+2];
    }
    gd.num_colors += PRDS_NUM_COLORS;

   /* Add Reference Lines color */
   color_name = XRSgetString(gd.cidd_db, "cidd.range_ring_color", "grey");
   STRcopy(gd.color[gd.num_colors].name,color_name,NAME_LENGTH);
   gd.extras.range_color = &(gd.color[gd.num_colors]);
   gd.num_colors++;

    /* Add Winds colors */
    for(i=0; i <   gd.extras.num_wind_sets; i++) {
	 STRcopy(gd.color[gd.num_colors].name,
	     gd.extras.wind[i].color_name,NAME_LENGTH);
	 gd.extras.wind[i].color = &gd.color[gd.num_colors];
	 gd.num_colors++;
    }

    /* Add Contour colors */
    for(i=0; i <   NUM_CONT_LAYERS; i++) {
	 STRcopy(gd.color[gd.num_colors].name,
	     gd.extras.cont[i].color_name,NAME_LENGTH);
	 gd.extras.cont[i].color = &gd.color[gd.num_colors];
	 gd.num_colors++;
    }

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
    for(i=0; i < gd.num_datafields ; i++) {        /* each color map file */
        nentries = get_color_mapping(color_file_subdir,gd.mrec[i]->color_file,gd.mrec[i]->h_vcm.vc);
        gd.mrec[i]->h_vcm.nentries = nentries;
        nentries = get_color_mapping(color_file_subdir,gd.mrec[i]->color_file,gd.mrec[i]->v_vcm.vc);
        gd.mrec[i]->v_vcm.nentries = nentries;

		/* Set overlay and colorscale thresholds to currnet min,max of defined colorscale */
		gd.mrec[i]->overlay_min = gd.mrec[i]->h_vcm.vc[0]->min;
		gd.mrec[i]->overlay_max = gd.mrec[i]->h_vcm.vc[gd.mrec[i]->h_vcm.nentries-1]->max;
		gd.mrec[i]->cscale_min = gd.mrec[i]->h_vcm.vc[0]->min;
		gd.mrec[i]->cscale_delta = (gd.mrec[i]->h_vcm.vc[gd.mrec[i]->h_vcm.nentries-1]->max - gd.mrec[i]->cscale_min) / gd.mrec[i]->h_vcm.nentries;

        if(gd.debug) fprintf(stderr,"Loaded color file %s - %d entries\n",gd.mrec[i]->color_file,nentries);

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
		 fprintf(stderr,"set of colors in your colorscales. MAX_COLORS currently set to : %d\n",MAX_COLORS);
		 fprintf(stderr,"Turn on debug parameter to see a report on the number of colors in each scale.\n");
		 exit(-3);
	    }

        }
	if(gd.debug) fprintf(stderr,"Total Unique colors used so far: %d\n",gd.num_colors);
    }
            
    fprintf(stderr,"Need %d colors for everything\n",gd.num_colors);

    switch(v_class = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_VISUAL_CLASS)) {
	 default:
		fprintf(stderr,"Visual Class %d of Display not supported - Sorry \n",v_class);
		exit(-1);
	 break;

	 case PseudoColor:
           /* Allocate the color cells needed -  Can Modify colors */
           status = XAllocColorCells(dpy, cmap, False,&mask, 0, pix_vals, gd.num_colors);
           if(status == 0) return -1;
	 break;

	 case TrueColor: /* Colors are read only */
	 case DirectColor: /* Colors are read only */
	     gd.inten_levels = 0;
	     /* Disable any controls which allow a color cell modification */
	     xv_set(gd.extras_pu->dim_sl,PANEL_INACTIVE,TRUE,NULL);
	     xv_set(gd.extras_pu->dim_msg,PANEL_INACTIVE,TRUE,NULL);
	 break;
    }

    /* Get the Pixel value for the background */
    XParseColor(dpy,cmap,gd.extras.background_color->name,&rgb_def);
    gcvalues.background = rgb_def.pixel;

    /* Put the correct color into its color cell (or get the closest cell) and create a GC for it */
    for(i=0; i< gd.num_colors; i++) {
        mult = (i < gd.num_draw_colors)? 1.0 : gd.image_inten;
	if(gd.inten_levels == 0) mult = 1.0;

        XParseColor(dpy,cmap,gd.color[i].name,&rgb_def);

        gd.color[i].r = rgb_def.red;    /* save the color values & pixel */
        gd.color[i].g = rgb_def.green;
        gd.color[i].b = rgb_def.blue;

        rgb_def.red = gd.color[i].r * mult;    /* Start with the default intensity */
        rgb_def.green = gd.color[i].g * mult;
        rgb_def.blue = gd.color[i].b * mult;
        rgb_def.flags = DoRed | DoGreen | DoBlue;

        if(gd.inten_levels > 0) {
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
    	color_name = XRSgetString(gd.cidd_db, "cidd.overlay_colorname", OVERLAY_COLOR);
    	flags = DoRed | DoGreen | DoBlue;
        XAllocNamedColor(dpy,cmap,color_name,&rgb_def,&rgb_exact);

    	/* Set up  GC's to use when drawing in the overlay plane - */
    	gcvalues.foreground =  rgb_def.pixel; 
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

    return 0;
}

/**********************************************************************
 * GET_COLOR_MAPPING: A routine to read RADAR color table files.
 *        This routine allocates memory for the entries.
 *        Returns number of entries found.
 */

int    get_color_mapping(color_file_subdir,fname,cval)
    char    *color_file_subdir;
    char    *fname;        /* file name */
    Val_color_t    *cval[];        /* RETURN -  pointer to array of structs */
{
    FILE    *cfile;
    char    buf[1024];
    char    string[INPUT_LINE_LEN];
    char    *cfield[NUM_PARSE_FIELDS];
    int    i,j;
    int    size;
    int    nstrings;
    int    nentries;
    char   *ptr;
    char   *lptr;

    if((cfile = fopen(fname,"r")) == NULL) {
	sprintf(buf,"%s/%s",color_file_subdir,fname);
        if((cfile = fopen(buf,"r")) == NULL) {
            fprintf(stderr,"Couldn't find %s/%s or %s/%s\n",
				 getcwd(NULL,1024),fname,color_file_subdir,fname);
            fprintf(stderr,"Please install %s and try again\n",fname);
            exit(-1);
        }
    }

    /* Get temp storage for character strings */
    for(i=0;i < NUM_PARSE_FIELDS; i++) {
        cfield[i] = (char *)  ucalloc(1,PARSE_FIELD_SIZE);
    }

    size = sizeof(Val_color_t);
    nentries = 0;

    /* loop thru file looking for valid entries */
    while ((fgets(string, INPUT_LINE_LEN, cfile) != NULL) && (nentries < MAX_COLORS)) {    

        if(string[0] != '#') {
         ptr = strpbrk(string,"!\n");    /* look for trailing exclamation point */
	 if(ptr != NULL) {   /* replace trailing exclamation point with null  */
	     *ptr = '\0';
             lptr = ptr + 1; /* Set the label pointer */
             /* replace offensive newline in label with null */
             ptr = strpbrk(lptr,"\n");
             if(ptr) *ptr = '\0';
	 } else {
	    lptr = NULL;
	 }

         if((nstrings = STRparse(string, cfield, INPUT_LINE_LEN, NUM_PARSE_FIELDS, PARSE_FIELD_SIZE)) >= 3) {
            /* Is (hopefully)  a valid entry */
            cval[nentries] = (Val_color_t *) ucalloc(1,size);    /* Get space for this entry */

            /* Set the label for this color scale element */
            if((lptr != NULL) && strlen(lptr) > 1) {
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
            nentries++;
          }
        }
    }
    fclose(cfile);

    /* free temp storage for character strings */
    for (i = 0; i < NUM_PARSE_FIELDS; i++) {
        ufree(cfield[i]);
    }

    if(nentries <= 0) {
        fprintf(stderr,"No color map entries found in %s",fname);
        return 0;
    }

    return(nentries);
}

/*****************************************************************
 * SETUP_COLORSCALES: Establish X value to pixel color value arrays
 *    for each data field. Also find an unused pixel value to use for
 *    run-length encoding key.
 */
   
void setup_colorscales(dpy)
    Display *dpy;
{
    int    i,k,l;
    int status;
    int    found_color;
    char    hist[256];
    Visual *visual;
    Visual *visual_list;
 
    status = 0;
     
    gd.cmap = DefaultColormap(dpy,DefaultScreen(dpy));
    status = combine_color_maps(dpy,gd.cmap);
    if(status < 0) {
        fprintf(stderr,"Warning: Cannot allocate enough colors from standard colormap\n");
        fprintf(stderr,"Creating one - Other windows may be affected\n");
        visual = DefaultVisual(gd.dpy,0);
        gd.cmap = XCreateColormap(gd.dpy,RootWindow(gd.dpy,0),visual,AllocNone);
        status = combine_color_maps(dpy,gd.cmap);
        if(status < 0) {
            fprintf(stderr,"Error allocating colors from new colormap - \n");
            exit(-1);
        }
        /* XInstallColormap(gd.dpy,gd.cmap); /* - Not required after R3  */
    }

    /* Find RLE Key */
    for(i=0; i < 256 ;i++) hist[i] = 0; /* clear hist */
    /* Mark used values */
    for(i=0; i < gd.num_colors; i++) {
	if(gd.color[i].pixval >=0 && gd.color[i].pixval < 256)
	    hist[gd.color[i].pixval] = 1;
    }

    for(i=255; i >= 0;i--) {    /* Search for first unused value */
        if(hist[i] == 0) {
            gd.movie.key = i;
            i = -1;
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
}

/*****************************************************************
 * SETUP_COLOR_MAPPING: Determine the mapping between data values and
 *        color to use
 *
 */

setup_color_mapping(mr,vcm,scale,bias)
    met_record_t    *mr;
    Valcolormap_t *vcm;
    double scale,bias;
{
    int    j,k,l;
    int    found_range;
    int    found_color;
    double    value;
 
    if(vcm->nentries == 0 || vcm->nentries > MAX_COLORS ) return;
     
    for(j=0; j <MAX_DATA_VALS; j++) {        /* calculate all possible data values that we represent*/
        value = (double) j * scale + bias;
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
         
        if(found_range == 0) {    /* this value doesn't map into any range specified in color scale */
            vcm->val_gc[j] = gd.extras.background_color->gc;
            vcm->val_pix[j] = gd.extras.background_color->pixval;
        }

    }
}
