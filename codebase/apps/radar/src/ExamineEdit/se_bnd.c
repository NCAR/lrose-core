/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

# define OLD_XNSx

/*
 * return_se_bnd_struct
 * se_absorb_bnd
 * se_add_bnd_pt
 * se_append_bpm
 * se_ascii_bnd
 * se_bnd_pt_atts
 * 
 * se_ccw
 * se_clr_all_bnds
 * se_clr_bnd
 * se_clr_current_bnd
 * se_cycle_bnd
 * se_delete_bnd_pt
 * se_draw_bnd
 * se_erase_all_bnds
 * se_find_intxns
 * 
 * se_malloc_one_bnd
 * se_mid_insert
 * se_nab_segment
 * se_num_segments
 * 
 * se_pack_bnd_set
 * se_pop_bpms
 * se_prev_bnd_set
 * se_push_all_bpms
 * se_push_bpm
 * se_radar_inside_bnd
 * se_redraw_all_bnds
 * se_return_next_bnd
 * 
 * se_save_bnd
 * se_set_bnd
 * se_set_intxn
 * se_shift_bnd
 * se_sizeof_bnd_set
 * se_unpack_bnd_buf
 * se_use_bnd
 * se_x_insert
 * se_y_insert
 * se_zap_last_bpm
 * 
 * 
 */
# include <dorade_headers.h>
# include <solo_window_structs.h>
# include <solo_editor_structs.h>
# include <solo_list_widget_ids.h>
# include <ui.h>
# include <ui_error.h>
# include <dd_math.h>
# include <function_decl.h>
# include <dgi_func_decl.h>
# include <glib.h>

extern GString *gs_complaints;

extern int LittleEndian;
static struct boundary_stuff *sebs=NULL;
static struct bnd_point_mgmt *bpm_spairs=NULL;

/* c------------------------------------------------------------------------ */

/* Sherrie's routines */
void se_refresh_bnd_files_widget();
void se_clear_segments();
void se_erase_segments();
void se_draw_segments();
void se_dump_bnd_files_widget();

/* external routines */
void se_fix_comment();		/* se_utils.c */
double sp_meters_per_pixel();	/* sp_xyraster.c */
void sp_locate_this_point();	/* sp_clkd.c */
void solo_message();		/* solo.c */
int se_crack_ed_path_name();	/* se_utils.c */
void solo_return_radar_name();	/* sp_basics.c */

/* internal routines */
struct boundary_stuff *return_se_bnd_struct();
int xse_absorb_bnd();
void xse_add_bnd_pt();
void se_append_bpm();
void se_ascii_bnd();
void se_bnd_pt_atts();
int xse_ccw();
void se_clr_all_bnds();
void se_clr_bnd();
void se_clr_current_bnd();
void se_cycle_bnd();
void se_delete_bnd_pt();
void se_draw_bnd();
void se_erase_all_bnds();
void se_ts_find_intxns();
int se_merge_intxn();
int xse_find_intxns();
struct one_boundary *se_malloc_one_bnd();
void se_mid_insert();
void se_nab_segment();
int xse_num_segments();
int se_compare_bnds();
void se_pack_bnd_set();
struct bnd_point_mgmt *se_pop_bpms();
void se_prev_bnd_set();
void se_push_all_bpms();
void se_push_bpm();
int se_radar_inside_bnd();
void se_redraw_all_bnds();
struct one_boundary *se_return_next_bnd();
void xse_save_bnd();
int xse_set_intxn();
void se_shift_bnd();
int se_sizeof_bnd_set();
void se_unpack_bnd_buf();
void se_use_bnd();
void xse_x_insert();
void xse_y_insert();
struct bnd_point_mgmt *se_zap_last_bpm();


/* c------------------------------------------------------------------------ */

struct boundary_stuff *
return_se_bnd_struct()
{
    /* return the solo editor boundary struct
     */
    int ii, nn, mark;
    struct point_in_space *solo_malloc_pisp();

    if(!sebs) {
	sebs = (struct boundary_stuff *)malloc(sizeof(struct boundary_stuff));
	memset(sebs, 0, sizeof(struct boundary_stuff));
	strcpy(sebs->comment_text, "no_comment");

	sebs->bh = (struct boundary_header *)
	      malloc(sizeof(struct boundary_header));

	memset(sebs->bh, 0, sizeof(struct boundary_header));
	strncpy(sebs->bh->name_struct, "BDHD", 4);
	sebs->bh->sizeof_struct = sizeof(struct boundary_header);
	sebs->origin = solo_malloc_pisp();
	sebs->pisp = solo_malloc_pisp();
	for(ii=0; ii < SE_FRAME; ii++) {
	    sebs->linked_windows[ii] = YES;
	}
	sebs->first_boundary = sebs->current_boundary = se_malloc_one_bnd();
	sebs->first_boundary->last = sebs->first_boundary;
    }
    return(sebs);
}
/* c------------------------------------------------------------------------ */

int xse_absorb_bnd()
{
    int ii, nn, mark, len, lenx, count=0;
    FILE *stream;
    WW_PTR wwptr, solo_return_wwptr();
    struct boundary_stuff *sebs, *return_se_bnd_struct();
    char *a, *b, *c, *e, str[256], mess[256], *buf;
    double d;


    sebs = return_se_bnd_struct();

    slash_path(str, sebs->directory_text);
    strcat(str, sebs->file_name_text);
# ifdef obsolete
    if(*sebs->comment_text) {
	se_fix_comment(sebs->comment_text);
	strcat(str, ".");
	strcat(str, sebs->comment_text);
    }
    se_refresh_bnd_files_widget(sebs);
# endif

    if(!(stream = fopen(str, "r"))) {
	sprintf(mess, "Unable to open boundary file %s\n", str);
	sii_message (mess);
	return(-1);
    }
    ii = fseek(stream, 0L, (int)2); /* at end of file */
    len = ftell(stream);	/* how big is the file */
    rewind(stream);
        
    buf = (char *)malloc(len);
    memset (buf, 0, len);
    if((lenx = fread(buf, sizeof(char), len, stream)) < len) {
	sprintf(mess, "Unable to read file %s\n", str);
	sii_message (mess);
	free(buf);
	return(-1);
    }
    fclose(stream);

    se_unpack_bnd_buf(buf, len);

    free(buf);
    return(0);
}
/* c------------------------------------------------------------------------ */

void xse_add_bnd_pt(sci, ob)
  struct solo_click_info *sci;
  struct one_boundary *ob;
{
    /* add a point to the boundary based on the x & y coordinates
     * in the click struct
     */
    struct boundary_stuff *sebs, *return_se_bnd_struct();
    struct point_in_space *pisp, *solo_return_radar_pisp();
    struct solo_edit_stuff *seds, *return_sed_stuff();

    struct bnd_point_mgmt *bpm, *se_pop_bpms();
    double d, sqrt(), fabs();
    double x = sci->x, y = sci->y;

    seds = return_sed_stuff();
    sebs = return_se_bnd_struct();
    bpm = se_pop_bpms();	/* nab a fresh boundary point */
    se_append_bpm(&ob->top_bpm, bpm); /* the append happens first so
				       * the last pointers are fresh */
    bpm->x = sci->x;
    bpm->y = sci->y;

    if(++ob->num_points > 1) {	/* avoid duplicates */
	if((bpm->x == bpm->last->x) && (bpm->y == bpm->last->y)) {
	    ob->num_points--;
	    se_delete_bnd_pt(bpm);
	    return;
	}
    }
# ifdef obsolete
    bpm->r = .5 +sqrt((SQ((double)x)+SQ((double)y)));
# endif
    bpm->which_frame = sci->frame;

    if(!(sebs->view_bounds || sebs->absorbing_boundary)) {
	sp_locate_this_point(sci, bpm); /* this routine is in
					 * ../perusal/sp_clkd.c */
# ifdef obsolete	
	bpm->pisp->state =
	      PISP_PLOT_RELATIVE | PISP_AZELRG |  PISP_EARTH;
	strcpy(bpm->pisp->id, "BND_PT_V1");
# endif
    }
    else if(sebs->absorbing_boundary) {
	memcpy(bpm->pisp, sebs->pisp, sizeof(struct point_in_space));
	bpm->r = KM_TO_M(bpm->pisp->range);
    }
    if(bpm->pisp->state & PISP_TIME_SERIES) {
	x = bpm->pisp->time;
	y = bpm->pisp->range;
    }
    /* the rasterization code sets sebs->view_bounds to YES and also uses
     * this routine and others to bound the rasterization
     */
    if(ob->num_points > 1) {
	if(!sebs->view_bounds) {
	    se_draw_bnd(bpm, 2, NO);
	    seds->boundary_exists = YES;
	    sii_set_boundary_pt_count (sci->frame, ob->num_points);
	}
	if(x > ob->max_x)
	      ob->max_x = x;
	if(x < ob->min_x)
	      ob->min_x = x;
	if(y > ob->max_y)
	      ob->max_y = y;
	if(y < ob->min_y)
	      ob->min_y = y;
	/*
	 * calculate boundary point attributes
	 */
	se_bnd_pt_atts(bpm);
	xse_x_insert(bpm, ob);
	xse_y_insert(bpm, ob);

	/*
	 * now do it for the line between this point and the first point
	 */
	bpm = ob->top_bpm;
	se_bnd_pt_atts(bpm);
    }
    else {			/* first point */
	if(!(sebs->absorbing_boundary || sebs->view_bounds)) {
	    /* get the radar origin and radar name from the first point
	     */
	    solo_return_radar_name(sci->frame, ob->bh->radar_name);
	    memcpy(sebs->origin, bpm->pisp, sizeof(struct point_in_space));
	    strcpy(sebs->origin->id, "BND_ORIGIN");
	}
	ob->min_x = ob->max_x = x;
	ob->min_y = ob->max_y = y;
    }
}
/* c------------------------------------------------------------------------ */

void se_append_bpm(top_bpm, bpm)
  struct bnd_point_mgmt **top_bpm, *bpm;
{
    /* append this point to the list of boundary points
     * (*top_bpm)->last always points to the last point appended
     * to the boundary this all points are linked by the last pointer
     * except for the last point appended the next pointer points
     * to the next point and the NULL pointer in the last point
     * serves to terminate loops
     */
    if(!(*top_bpm)) {			/* no list yet */
	*top_bpm = bpm;
	(*top_bpm)->last = bpm;
    }
    else {
	(*top_bpm)->last->next = bpm; /* last bpm on list should point
				       * to this one */
	bpm->last = (*top_bpm)->last;
	(*top_bpm)->last = bpm;
    }
    bpm->next = NULL;
    return;
}
/* c------------------------------------------------------------------------ */

void se_ascii_bnd(stream, num, pisp)
  FILE *stream;
  int num;
  struct point_in_space *pisp;
{
    fprintf(stream
	    , "bnd:%3d; lon:%.4f; lat:%.4f; alt:%.4f; "
	    , num, pisp->longitude, pisp->latitude, pisp->altitude);
    fprintf(stream
	    , "az:%.4f; el:%.4f; rng:%.4f;"
	    , pisp->azimuth, pisp->elevation, pisp->range);
    fprintf(stream
	    , "hdg:%.4f; tlt:%.4f; dft:%.4f; "
	    , pisp->heading, pisp->tilt, pisp->drift);
    fprintf(stream, "\n");
}
/* c------------------------------------------------------------------------ */

void se_bnd_pt_atts(bpm)
  struct bnd_point_mgmt *bpm;
{

    if(bpm->pisp->state & PISP_TIME_SERIES) {
	bpm->dt = bpm->last->pisp->time - bpm->pisp->time;
	bpm->dr = bpm->last->pisp->range - bpm->pisp->range;
	if(bpm->dt) bpm->slope = bpm->dr/bpm->dt;
	if(bpm->dr) bpm->slope_90 = -1./bpm->slope; 
	bpm->len = SQRT(SQ(bpm->dt) + SQ(bpm->dr));
	bpm->t_mid = bpm->pisp->time + 0.5 * bpm->dt;
	bpm->r_mid = bpm->pisp->range + 0.5 * bpm->dr;
    }
    else {
	bpm->dy = bpm->last->y - bpm->y;
	bpm->dx = bpm->last->x - bpm->x;
	
	if(bpm->dx)
	      bpm->slope = (double)bpm->dy/bpm->dx;
	
	if(bpm->dy)
	      bpm->slope_90 = -1./bpm->slope; /* slope of the line
					       * perpendicular to this line */
	
	bpm->len = sqrt((SQ((double)bpm->dx) + SQ((double)bpm->dy)));
	bpm->x_mid = bpm->x + 0.5 * bpm->dx;
	bpm->y_mid = bpm->y + 0.5 * bpm->dy;
    }
}
/* c------------------------------------------------------------------------ */

int xse_ccw(x0, y0, x1, y1)
  double x0, y0, x1, y1;
{
    /* is point 0 left or right of a line between the origin and point 1
     * form a line between the origin and point 0 called line 0
     */
    if(y0 * x1 > y1 * x0)
	  return(1);
				/*
				 * this says the slope of line 0
				 * is greater than the slope of line 1
				 * and is counter-clockwise or left
				 * of the line.
				 */
    if(y0 * x1 < y1 * x0)
	  return(-1);
				/* cw or right */
    return(0);			/* on the line */
}
/* c------------------------------------------------------------------------ */

int se_ccw(x0, y0, x1, y1)
  long x0, y0, x1, y1;
{
    /* is point 0 left or right of a line between the origin and point 1
     * form a line between the origin and point 0 called line 0
     */
    if((float)y0 * x1 > (float)y1 * x0)
	  return(1);
				/*
				 * this says the slope of line 0
				 * is greater than the slope of line 1
				 * and is counter-clockwise or left
				 * of the line.
				 */
    if((float)y0 * x1 < (float)y1 * x0)
	  return(-1);
				/* cw or right */
    return(0);			/* on the line */
}
/* c------------------------------------------------------------------------ */

void se_clr_all_bnds()
{
    int ww;
    struct boundary_stuff *sebs, *return_se_bnd_struct();
    struct one_boundary *ob;
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct solo_perusal_info *spi, *solo_return_winfo_ptr();

    spi = solo_return_winfo_ptr();
    seds = return_sed_stuff();
    seds->boundary_exists = NO;
    sebs = return_se_bnd_struct();
    ob = sebs->current_boundary = sebs->first_boundary;

    for(; ob; ob = ob->next) {
	se_clr_bnd(ob);
    }
    for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	if(!spi->active_windows[ww] || !sebs->linked_windows[ww])
	      continue;
	se_clear_segments(ww);
    }
}
/* c------------------------------------------------------------------------ */

void se_clr_bnd(ob)
  struct one_boundary *ob;
{
    struct solo_edit_stuff *seds, *return_sed_stuff();

    se_push_all_bpms(&ob->top_bpm);
    ob->bh->force_inside_outside = 
	  ob->num_points = 0;
    ob->last_line = ob->last_point = ob->x_mids = ob->y_mids = NULL;
}
/* c------------------------------------------------------------------------ */

void se_clr_current_bnd()
{
    struct boundary_stuff *sebs, *return_se_bnd_struct();
    struct one_boundary *ob;
    int erase, ww;

    sebs = return_se_bnd_struct();

    if(!sebs->first_boundary)
	  return;
    ob = sebs->current_boundary;
    erase = ob->num_points > 1;

    se_clr_bnd(sebs->current_boundary);

    if(erase) {
      for (ww=0; ww < SOLO_MAX_WINDOWS; ww++)
	{ se_clear_segments (ww); }
    }
}
/* c------------------------------------------------------------------------ */

void se_cycle_bnd()
{
    /* the purpose of this routine is to move between boundaries
     * the first move from a boundary with two or more points in it
     * might be to an empty boundary in order to start a new boundary
     * so a move always clears the screen and may redraw a boundary if
     * the move is to an existing boundary otherwise the screen will
     * be blank and can be refilled with a redraw boundaries command
     * moves will always be in on direction towards an empty boundary
     * a move from an empty (the last) boundary will be to the first
     * boundary.
     */
    int ii, nn, ww, mark;
    struct boundary_stuff *sebs, *return_se_bnd_struct();
    struct one_boundary *ob, *ob_next;
    struct solo_perusal_info *spi, *solo_return_winfo_ptr();

    spi = solo_return_winfo_ptr();
    sebs = return_se_bnd_struct();
    ob = sebs->current_boundary;

    for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	if(!spi->active_windows[ww] || !sebs->linked_windows[ww])
	      continue;
	se_clear_segments(ww);
    }

    /* check to see if we are moving from a cleared boundary to a
     * boundary with points in it
     */
    if(!ob->num_points && ob->next && ob->next->num_points) {
	sebs->current_boundary = ob->next;
	/* if so put the cleared boundary at the end
	 */
	if(ob == sebs->first_boundary) {
	    sebs->first_boundary->next->last = sebs->first_boundary->last;
	    sebs->first_boundary = sebs->first_boundary->next;
	}
	else {
	    ob->last->next = ob->next;
	    ob->next->last = ob->last;
	}
	sebs->first_boundary->last->next = ob;
	ob->last = sebs->first_boundary->last;
	sebs->first_boundary->last = ob;
	ob->next = NULL;
    }
    else if(!ob->num_points) {
	/* shift back to first boundary
	 * does the right thing when the first boundary is empty
	 */
	sebs->current_boundary = sebs->first_boundary;
    }
    else {
	se_return_next_bnd();
    }
    ob = sebs->current_boundary;

    if(ob->num_points > 1) {
	se_draw_bnd(ob->top_bpm->last, ob->num_points, NO);
    }
}
/* c------------------------------------------------------------------------ */

void se_delete_bnd_pt(bpm)
  struct bnd_point_mgmt *bpm;
{
    /* remove this point from the boundary
     * remember to recalculate mins, maxs, slopes and deltas
     */
    struct one_boundary *ob;
    struct boundary_stuff *sebs, *return_se_bnd_struct();
    struct bnd_point_mgmt *bpmx;

    sebs = return_se_bnd_struct();
    ob = sebs->current_boundary;

    if(bpm == ob->top_bpm) {
	if(!bpm->next) {	/* this is the only point */
	    ob->top_bpm = NULL;
	}
	ob->top_bpm->next->last = ob->top_bpm->last;
	ob->top_bpm = ob->top_bpm->next;
    }
    else {
	bpm->last->next = bpm->next;
	if(bpm->next) bpm->next->last = bpm->last;
	if(bpm == ob->top_bpm->last) ob->top_bpm->last = bpm->last;
    }
    se_push_bpm(bpm);
    /*
     * now do it for the line between this point and the first point
     */
    bpm = ob->top_bpm;
    se_bnd_pt_atts(bpm);

    ob->max_x = ob->min_x = bpm->x;
    ob->max_y = ob->min_y = bpm->y;
    for(bpm=bpm->next;  bpm;  bpm=bpm->next) {
	if(bpm->x > ob->max_x)
	      ob->max_x = bpm->x;
	if(bpm->x < ob->min_x)
	      ob->min_x = bpm->x;
	if(bpm->y > ob->max_y)
	      ob->max_y = bpm->y;
	if(bpm->y < ob->min_y)
	      ob->min_y = bpm->y;
    }
}
/* c------------------------------------------------------------------------ */

void se_draw_bnd_for_frame(frame_num, bpm, num, erase)
     struct bnd_point_mgmt *bpm;
     int frame_num, num, erase;
{
  /* draw or erase the requisite number of points in the active windows
   * see "se_shift_bnd()" for similar logic
   */
  int ii, mm, nn, mark, right_left, width, height;
  struct bnd_point_mgmt *bpmx;
  struct boundary_stuff *sebs, *return_se_bnd_struct();
  struct solo_perusal_info *spi, *solo_return_winfo_ptr();
  struct point_in_space radar, *current_radar, *boundary_radar;
  WW_PTR wwptr, solo_return_wwptr();
  double x, y, z, costilt, untilt, tiltfactor, current_tilt;
  double d, v_scale, h_scale, ta, time_span, top_edge;
  double sp_meters_per_pixel();

  spi = solo_return_winfo_ptr();
  sebs = return_se_bnd_struct();
  if(sebs->view_bounds || frame_num >= SOLO_MAX_WINDOWS)
    return;

  current_radar = &radar;
  boundary_radar = sebs->origin;
  untilt = fabs(cos(RADIANS(boundary_radar->tilt)));

  wwptr = solo_return_wwptr(frame_num);
  dd_copy_pisp(wwptr->radar_location, current_radar);
  /*
	 * we now have the frame radar (current_radar)
	 * and the location of the boundary radar
	 * see also the routine "sp_locate_this_point()" in file
	 * "sp_clkd.c"
	 */
  dd_latlon_relative(boundary_radar, current_radar);
  /*
	 * x,y,z now contains the coordinates of the boundary radar
	 * relative to the current radar
	 */
  x = KM_TO_M(current_radar->x);
  y = KM_TO_M(current_radar->y);
  z = KM_TO_M(current_radar->z);

  if(fabs(costilt = cos(fabs(RADIANS(current_radar->tilt)))) > 1.e-6) {
    tiltfactor = fabs(1./costilt);
  }
  else {
    tiltfactor = 0;
  }
  mm = num;
  bpmx = bpm;

  if(wwptr->view->type_of_plot & SOLO_TIME_SERIES) {
    d = sp_meters_per_pixel();
    d /= (double)wwptr->view->ts_magnification;
    v_scale = M_TO_KM(d);
    height = wwptr->view->height_in_pix-1;
    top_edge = wwptr->view->ts_ctr_km + .5 * height * v_scale;

    ta = wwptr->sweep->start_time;
    time_span = wwptr->sweep->stop_time - ta;
    width = wwptr->view->width_in_pix-1;
    h_scale = (double)width/time_span;
    right_left = wwptr->view->type_of_plot & TS_PLOT_RIGHT_TO_LEFT;
    /*
     * we will be passing pixel coodinates and assuming that (0,0) is
     * the upper left corner of the screen
     */
    for(; mm--; bpmx = bpmx->last) {
      bpmx->_x = (bpmx->pisp->time -ta) * h_scale + .5;
      if(right_left) bpmx->_x = width - bpmx->_x;
      bpmx->_y = (top_edge - bpmx->pisp->z) / v_scale +.5;
    }
  }	
  else if(!(wwptr->lead_sweep->sweep->radar_type == GROUND ||
	    wwptr->lead_sweep->sweep->radar_type == SHIP)) {

    for(; mm--; bpmx = bpmx->last) {
# ifdef notyet
      /* we will need to project these points onto a plane
       * perpendicular to the heading
	 if(wwptr->lead_sweep->scan_mode == AIR) {
	 for(; mm--; bpmx = bpmx->last) {
	*/
# else
      bpmx->_x = bpmx->x;
      bpmx->_y = bpmx->y;
# endif
    }
  }
  else {			/* ground based */

    /* we need to project the original values
     * and then unproject the new values
     */
    for(; mm--; bpmx = bpmx->last) {
      switch (wwptr->lead_sweep->scan_mode) {
      case RHI:
	bpmx->_x = bpmx->x;
	bpmx->_y = bpmx->y;
	break;
      default:
	bpmx->_x = (bpmx->x * untilt + x) * tiltfactor;
	bpmx->_y = (bpmx->y * untilt + y) * tiltfactor;
	break;
      }
    }
  }

  if(!(wwptr->view->type_of_plot & SOLO_TIME_SERIES)) {
    mark = 0;
  }
  if(erase) {
    se_erase_segments(frame_num, num, bpm);
  }
  else {
    se_draw_segments(frame_num, num, bpm);
  }
}

/* c------------------------------------------------------------------------ */

void se_draw_bnd(bpm, num, erase)
  struct bnd_point_mgmt *bpm;
  int num, erase;
{
    /* draw or erase the requisite number of points in the active windows
     * see "se_shift_bnd()" for similar logic
     */
    int ii, mm, nn, ww, mark, right_left, width, height;
    struct bnd_point_mgmt *bpmx;
    struct boundary_stuff *sebs, *return_se_bnd_struct();
    struct solo_perusal_info *spi, *solo_return_winfo_ptr();
    struct point_in_space radar, *current_radar, *boundary_radar;
    WW_PTR wwptr, solo_return_wwptr();
    double x, y, z, costilt, untilt, tiltfactor, current_tilt;
    double d, v_scale, h_scale, ta, time_span, top_edge;
    double sp_meters_per_pixel();

    spi = solo_return_winfo_ptr();
    sebs = return_se_bnd_struct();
    if(sebs->view_bounds)
	  return;

    current_radar = &radar;
    boundary_radar = sebs->origin;
    untilt = fabs(cos(RADIANS(boundary_radar->tilt)));

    for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	if(!spi->active_windows[ww] || !sebs->linked_windows[ww])
	      continue;

	wwptr = solo_return_wwptr(ww);
	dd_copy_pisp(wwptr->radar_location, current_radar);
	/*
	 * we now have the frame radar (current_radar)
	 * and the location of the boundary radar
	 * see also the routine "sp_locate_this_point()" in file
	 * "sp_clkd.c"
	 */
	dd_latlon_relative(boundary_radar, current_radar);
	/*
	 * x,y,z now contains the coordinates of the boundary radar
	 * relative to the current radar
	 */
	x = KM_TO_M(current_radar->x);
	y = KM_TO_M(current_radar->y);
	z = KM_TO_M(current_radar->z);

	if(fabs(costilt = cos(fabs(RADIANS(current_radar->tilt)))) > 1.e-6) {
	    tiltfactor = fabs(1./costilt);
	}
	else {
	    tiltfactor = 0;
	}
	mm = num;
	bpmx = bpm;

	if(wwptr->view->type_of_plot & SOLO_TIME_SERIES) {
	    d = sp_meters_per_pixel();
	    d /= (double)wwptr->view->ts_magnification;
	    v_scale = M_TO_KM(d);
	    height = wwptr->view->height_in_pix-1;
	    top_edge = wwptr->view->ts_ctr_km + .5 * height * v_scale;

	    ta = wwptr->sweep->start_time;
	    time_span = wwptr->sweep->stop_time - ta;
	    width = wwptr->view->width_in_pix-1;
	    h_scale = (double)width/time_span;
	    right_left = wwptr->view->type_of_plot & TS_PLOT_RIGHT_TO_LEFT;
	    /*
	     * we will be passing pixel coodinates and assuming that (0,0) is
	     * the upper left corner of the screen
	     */
	    for(; mm--; bpmx = bpmx->last) {
		bpmx->_x = (bpmx->pisp->time -ta) * h_scale + .5;
		if(right_left) bpmx->_x = width - bpmx->_x;
		bpmx->_y = (top_edge - bpmx->pisp->z) / v_scale +.5;
	    }
	}	
	else if(!(wwptr->lead_sweep->sweep->radar_type == GROUND ||
	     wwptr->lead_sweep->sweep->radar_type == SHIP)) {

	    for(; mm--; bpmx = bpmx->last) {
# ifdef notyet
	if(wwptr->lead_sweep->scan_mode == AIR) {
	    for(; mm--; bpmx = bpmx->last) {
		/* we will need to project these points onto a plane
		 * perpendicular to the heading
		 */
# else
		bpmx->_x = bpmx->x;
		bpmx->_y = bpmx->y;
# endif
	    }
	}
	else {			/* ground based */

	    /* we need to project the original values
	     * and then unproject the new values
	     */
	    for(; mm--; bpmx = bpmx->last) {
		switch (wwptr->lead_sweep->scan_mode) {
		case RHI:
		    bpmx->_x = bpmx->x;
		    bpmx->_y = bpmx->y;
		    break;
		default:
		    bpmx->_x = (bpmx->x * untilt + x) * tiltfactor;
		    bpmx->_y = (bpmx->y * untilt + y) * tiltfactor;
		    break;
		}
	    }
	}

	if(!(wwptr->view->type_of_plot & SOLO_TIME_SERIES)) {
	    mark = 0;
	}
	if(erase) {
	   se_erase_segments(ww, num, bpm);
	}
	else {
	   se_draw_segments(ww, num, bpm);
	}
    }
}
/* c------------------------------------------------------------------------ */

void se_erase_all_bnds()
{
    int ii, nn, ww, mark;
    struct boundary_stuff *sebs, *return_se_bnd_struct();
    struct one_boundary *ob;
    struct solo_perusal_info *spi, *solo_return_winfo_ptr();

    spi = solo_return_winfo_ptr();
    sebs = return_se_bnd_struct();
    ob = sebs->first_boundary;

    for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	if(!spi->active_windows[ww] || !sebs->linked_windows[ww])
	      continue;
	se_clear_segments(ww);
    }
}
/* c------------------------------------------------------------------------ */

void se_ts_find_intxns(radar_alt, d_max_range, ob, d_time, d_pointing
		       , automatic, down, d_ctr)
  double radar_alt, d_max_range, d_time, d_pointing, d_ctr;
  struct one_boundary *ob;
  int down, automatic;
{
   int mm;
   struct bnd_point_mgmt *bpm, *bpmx, *bpma, *bpmb;
   double d, time_span, ta, tb, xx, zz;



   ob->num_intxns = 0;
   ob->first_intxn = NULL;
   mm = ob->num_points;
   bpm = ob->top_bpm;
   d_time += .005;
   d = RADIANS(CART_ANGLE(d_pointing));
   /*
    * we bump the time by 5 milliseconds so a ray and a vertex
    * are not coincident in time
    */
   if(automatic) {
      down = sin(d) < 0;
   }
   else {
      radar_alt = down ? 2. * d_ctr : 0;
   }
    
   for(; mm--; bpm = bpm->last) {
      if((bpm->last->pisp->time) < (bpm->pisp->time)) {
	 bpma = bpm->last;
	 bpmb = bpm;
      }
      else {
	 bpma = bpm;
	 bpmb = bpm->last;
      }
      ta = bpma->pisp->time;
      tb = bpmb->pisp->time;

      if(d_time >= ta && d_time < tb) {
	 /* possible intxn */
	 zz = ((d_time - ta)/(tb - ta)) * (bpmb->pisp->z - bpma->pisp->z)
	   + bpma->pisp->z;
	 if((down && zz < radar_alt) || (!down && zz > radar_alt)) {
	    /* intxn! */
	    bpm->rx = down
	      ? KM_TO_M(radar_alt -zz) : KM_TO_M(zz - radar_alt);
	    ob->num_intxns++;
	    bpm->next_intxn = NULL;
	    se_merge_intxn(bpm, ob);
	 }
      }
   }
   ob->radar_inside_boundary = ob->num_intxns & 1;
   /* i.e. an odd number of intxns implies radar inside */

   if(ob->bh->force_inside_outside) {
      if(ob->bh->force_inside_outside == BND_FIX_RADAR_INSIDE) {
	 ob->radar_inside_boundary = YES;
      }
      if(ob->bh->force_inside_outside == BND_FIX_RADAR_OUTSIDE) {
	 ob->radar_inside_boundary = NO;
      }
   }
}
/* c------------------------------------------------------------------------ */

int se_merge_intxn(bpm, ob)
  struct bnd_point_mgmt *bpm;
  struct one_boundary *ob;
{
    struct bnd_point_mgmt *bpmx;

    if(!(bpmx = ob->first_intxn)) { /* first intersection */
	ob->first_intxn = bpm->last_intxn = bpm;
	return(YES);
    }

    for(; bpmx; bpmx = bpmx->next_intxn) {
	if(bpm->rx < bpmx->rx) { 
				/* insert intxn here */
	    bpm->next_intxn = bpmx;
	    bpm->last_intxn = bpmx->last_intxn;
	    if(bpmx == ob->first_intxn) { /* new first intxn */
		ob->first_intxn = bpm;
	    }
	    else {
		bpmx->last_intxn->next_intxn = bpm;
	    }
	    bpmx->last_intxn = bpm;
	    break;
	}
    }
    if(!bpmx) {			/* furthest out...tack it onto the end */
	ob->first_intxn->last_intxn->next_intxn = bpm;
	bpm->last_intxn = ob->first_intxn->last_intxn;
	ob->first_intxn->last_intxn = bpm;
    }
    return(YES);
}
/* c------------------------------------------------------------------------ */

int xse_find_intxns(angle, range, ob)
  double angle, range;
  struct one_boundary *ob;
{
    /* this routine creates a list of endpoints of lines that
     * intersect the current ray
     */
    struct bnd_point_mgmt *bpm, *bpmx;
    double theta = RADIANS(CART_ANGLE(angle)), cos(), sin();
    double slope=0, xx, yy;
    long x, y;
    int ii, mm, nn, nx=0, mark;


    ob->num_intxns = 0;
    ob->first_intxn = NULL;
    /*
     * compute the endpoints of the ray
     */
    xx = range * cos(theta);
    yy = range * sin(theta);
    x = xx < 0 ? xx -.5 : xx +.5;
    y = yy < 0 ? yy -.5 : yy +.5;
# ifdef OLD_XNS
    if(x) slope = yy/xx;
# else
    if(xx) slope = yy/xx;
# endif

    bpm = ob->top_bpm;		/* find the first point that is not
				 * on the line */
    /*
     * when we are doing intersections, we want to use the shifted
     * x,y values because the boundary may be for another radar
     */
    for(bpmx=NULL; bpm; bpm = bpm->next) {
# ifdef OLD_XNS
	bpm->which_side = se_ccw(bpm->_x, bpm->_y, x, y);
# else
	bpm->which_side = xse_ccw((double)bpm->_x, (double)bpm->_y
				  , xx, yy);
# endif
	if(bpm->which_side) {
	    bpmx = bpm;
	    break;
	}
    }
    if(!bpmx)
	  return(0);		/* no intersections! */
    mm = ob->num_points;
    /*
     * since "->last" links all points we loop through all the
     * points using the last pointer
     */
    for(; mm--; bpm = bpm->last) {
	/*
	 * a return of 0 from se_ccw says this point is
	 * colinear witht the ray. So we do nothing.
	 */
# ifdef OLD_XNS
	bpm->last->which_side = se_ccw(bpm->last->_x, bpm->last->_y
					  , x, y);
# else
	bpm->last->which_side = xse_ccw
	      ((double)bpm->last->_x, (double)bpm->last->_y, xx, yy);
# endif
	if(bpm->last->which_side) {
	    if(bpm->last->which_side != bpmx->which_side) {
		/*
		 * we may have an intersection between "bpm"
		 * and "bpm->last". See if it's real.
		 */
# ifdef OLD_XNS		
		xse_set_intxn(x, y, slope, bpm, ob);
# else
		xse_set_intxn(xx, yy, slope, bpm, ob);
# endif
	    }
	    bpmx = bpm->last;
	}
    }
# ifdef obsolete
    xse_num_segments(ob);
# endif
    return(ob->num_intxns);
}
/* c------------------------------------------------------------------------ */

struct one_boundary *se_malloc_one_bnd()
{
    struct one_boundary *ob;
    ob = (struct one_boundary *)malloc
	  (sizeof(struct one_boundary));
    memset(ob, 0, sizeof(struct one_boundary));
    ob->bh = (struct boundary_header *)
	  malloc(sizeof(struct boundary_header));
    
    memset(ob->bh, 0, sizeof(struct boundary_header));
    strncpy(ob->bh->name_struct, "BDHD", 4);
    ob->bh->sizeof_struct = sizeof(struct boundary_header);
    ob->bh->offset_to_origin_struct = sizeof(struct boundary_header);
    ob->bh->offset_to_first_point = ob->bh->offset_to_origin_struct 
	  + sizeof(struct point_in_space);
    ob->bh->sizeof_boundary_point = sizeof(struct point_in_space);

    return(ob);
}
/* c------------------------------------------------------------------------ */

void se_mid_insert(bpm, ob)
  struct bnd_point_mgmt *bpm;
  struct one_boundary *ob;
{
    int ii, nn, mark;
    struct bnd_point_mgmt *bpmx;

    if((nn=ob->num_points) < 2)
	  return;
    ob->x_mids = ob->y_mids = NULL;
    bpmx = ob->top_bpm;

    if(ob->num_points == 2) {
	xse_x_insert(bpmx, ob);
	return;
    }
    for(; nn--; bpmx = bpmx->next) {
	xse_x_insert(bpmx, ob);
    }
    return;
}
/* c------------------------------------------------------------------------ */

void se_nab_segment(num, r0, r1, ob)
  int num;
  double *r0, *r1;
  struct one_boundary *ob;
{
    /* this routine returns the start and stop range of the
     * requested segment
     */
    int ii, nn, mark;
    struct bnd_point_mgmt *bpm;

    if(num < 0 || num >= ob->num_segments) {
	*r0 = -999999.;
	*r1 = -999998.;
	return;
    }
    if(num) {
	*r0 = ob->next_segment->rx;
	if(ob->next_segment->next_intxn) {
	    *r1 = ob->next_segment->next_intxn->rx;
	    ob->next_segment = ob->next_segment->next_intxn->next_intxn;
	}
	else {
	    *r1 = 1.e9;
	}
    }
    else {			/* first segment */
	*r0 = ob->r0;
	*r1 = ob->r1;
    }
}
/* c------------------------------------------------------------------------ */

int xse_num_segments(ob)
  struct one_boundary *ob;
{
    /* calculate the number of segments and set up
     * the first segment
     */
    int ii, nn, nx, mark;

    nx = ob->num_intxns;

    if(ob->radar_inside_boundary) {
	if(!nx) {
	    /* the end of the ray is inside the boundary */
	    ob->num_segments = 1;
	    ob->r0 = 0;
	    ob->r1 = 1.e9;
	    return(1);
	}
	ob->r0 = 0;
	ob->r1 = ob->first_intxn->rx;
	ob->next_segment = ob->first_intxn->next_intxn;

	if(nx & 1) {		/* no funny stuff */
	    ob->num_segments = (nx+1)/2;
	}
	else {
	    /*
	     * even number of intersections
	     * assume the boundary is past the end of the ray
	     */
	    ob->num_segments = nx/2 +1;
	}
	return(ob->num_segments);
    }
    /* radar is outside the boundary
     */
    if(!nx) {
	ob->num_segments = 0;
	return(ob->num_segments);
    }
    ob->r0 = ob->first_intxn->rx;

    if(nx & 1) {		/* the boundary is past the end of the ray */
	if(nx == 1) {
	    ob->num_segments = 1;
	    ob->r1 = 1.e9;
	    return(1);
	}
	ob->num_segments = (nx+1)/2;
    }
    else {
	ob->num_segments = nx/2;
    }
    ob->r1 = ob->first_intxn->next_intxn->rx;
    ob->next_segment = ob->first_intxn->next_intxn->next_intxn;

    return(ob->num_segments);
}
/* c------------------------------------------------------------------------ */

int se_compare_bnds(aa, bb, size)
  char *aa, *bb;
  int size;
{
    int nn;
    struct boundary_header bh, *bha, *bhb;
    struct point_in_space *pispa, *pispb;
    struct generic_descriptor *gda, *gdb;
    char *ee = aa + size;
    
    for(; aa < ee; ) {
	gda = (struct generic_descriptor *)aa;
	gdb = (struct generic_descriptor *)bb;
	if(gda->sizeof_struct != gdb->sizeof_struct)
	      return(1);

	if(!strncmp(aa, "BDHD", 4)) {
	    /* copy aa to local and make the times the same before comparing
	     */
	    memcpy(&bh, aa, gda->sizeof_struct);
	    bha = (struct boundary_header *)aa;
	    bhb = (struct boundary_header *)bb;
	    bh.time_stamp = bhb->time_stamp;
	    if(nn = memcmp(&bh, bb, gda->sizeof_struct))
		  return(nn);	/* they don't compare */
	}
	else if(!strncmp(aa, "PISP", 4)) {
	    pispa = (struct point_in_space *)aa;
	    pispb = (struct point_in_space *)bb;

	    if(nn = memcmp(aa, bb, gda->sizeof_struct))
		  return(nn);	/* they don't compare */
	}
	aa += gda->sizeof_struct;
	bb += gda->sizeof_struct;
    }
    return((int)0);
}
/* c------------------------------------------------------------------------ */

void se_pack_bnd_set(buf)
  char *buf;
{
    int size=0;
    struct one_boundary *ob;
    struct boundary_stuff *sebs, *return_se_bnd_struct();
    struct boundary_header *bh;
    struct bnd_point_mgmt *bpm;
    long time_now();
    char *bb = buf;

    if(!(buf))
	  return;
    sebs = return_se_bnd_struct();
    ob = sebs->first_boundary;


    /*
     * for each boundary
     */
    for(; ob; ob = ob->next) {
	bh = ob->bh;
	bh->time_stamp = time_now();
	/* reset all this stuff in case we sucked in an old boundary
	 */
	ob->bh->sizeof_struct = sizeof(struct boundary_header);
	ob->bh->offset_to_origin_struct = sizeof(struct boundary_header);
	ob->bh->offset_to_first_point = ob->bh->offset_to_origin_struct 
	  + sizeof(struct point_in_space);
	ob->bh->sizeof_boundary_point = sizeof(struct point_in_space);

	if((bh->num_points = ob->num_points) > 1) {
	    /*
	     * the boundary header
	     */
	    memcpy(bb, bh, sizeof(struct boundary_header));
	    bb += sizeof(struct boundary_header);
	    /*
	     * the radar location
	     */
	    memcpy(bb, sebs->origin, sizeof(struct point_in_space)); 
	    bb += sizeof(struct point_in_space);
	    /*
	     * now the points in the boundary
	     */
	    bpm = ob->top_bpm;
	    
	    for(; bpm; bpm = bpm->next) {
		memcpy(bb, bpm->pisp, sizeof(struct point_in_space));
		bb += sizeof(struct point_in_space);
	    }
	}
    }
}
/* c------------------------------------------------------------------------ */

struct bnd_point_mgmt *se_pop_bpms()
{
    struct bnd_point_mgmt *bpm=bpm_spairs;
    struct point_in_space *pisp, *solo_malloc_pisp();

    if(!bpm_spairs) {
	bpm = (struct bnd_point_mgmt *)malloc(sizeof(struct bnd_point_mgmt));
	memset(bpm, 0, sizeof(struct bnd_point_mgmt)); /* clear it out */
	if(!bpm) {
	    uii_printf("Unable to pop bpms\n");
	    exit(1);
	}
	bpm->pisp = solo_malloc_pisp();
    }
    else {
	bpm_spairs = bpm->next;
    }
    pisp = bpm->pisp;
    memset(bpm, 0, sizeof(struct bnd_point_mgmt)); /* clear it out */
    bpm->pisp = pisp;		/* except for the pisp */
    return(bpm);
}
/* c------------------------------------------------------------------------ */

void se_prev_bnd_set()
{
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct prev_bnd_sets *pbs;
    struct boundary_stuff *sebs, *return_se_bnd_struct();

    seds = return_sed_stuff();
    if(!seds->last_pbs)
	  return;
    sebs = return_se_bnd_struct();

    if(sebs->last_operation != PREV_BND_SET) {
	seds->pbs = seds->last_pbs;
    }
    else {
	seds->pbs = seds->pbs->last;
    }
    se_clr_all_bnds();
    if(seds->pbs)
	  se_unpack_bnd_buf(seds->pbs->at, seds->pbs->sizeof_set);
}
/* c------------------------------------------------------------------------ */

void se_push_all_bpms(bpmptr)
  struct bnd_point_mgmt **bpmptr;
{
    struct bnd_point_mgmt *bpm=(*bpmptr);

    if(!bpm)
	  return;
    bpm->last->next = bpm_spairs;
    bpm_spairs = bpm;
    *bpmptr = NULL;
    return;
}
/* c------------------------------------------------------------------------ */

void se_push_bpm(bpm)
  struct bnd_point_mgmt *bpm;
{
    bpm->next = bpm_spairs;
    bpm_spairs = bpm;
    return;
}
/* c------------------------------------------------------------------------ */

int se_radar_inside_bnd(ob)
  struct one_boundary *ob;
{
    /* determine if the radar is inside or outside the boundary
     */
    struct bnd_point_mgmt *bpm, *bpmx;
    double dd, sqrt(), atan2(), fmod();
    double r, x, y, theta;
    int ii, mm = ob->num_points-1, nn, mark, inside_count=0;


    if(ob->bh->force_inside_outside) {
	if(ob->bh->force_inside_outside == BND_FIX_RADAR_INSIDE) {
	    return(ob->radar_inside_boundary = YES);
	}
	if(ob->bh->force_inside_outside == BND_FIX_RADAR_OUTSIDE) {
	    return(ob->radar_inside_boundary = NO);
	}
    }

    bpm = ob->top_bpm;
    x = ABS(bpm->_x);
    y = ABS(bpm->_y);
    /*
     * we are using the shifted values of x and y in case this boundary
     * is for a different radar
     */
    for(bpm = bpm->next; mm--; bpm = bpm->next) {
	if(ABS(bpm->_x) > x) x = ABS(bpm->_x);
	if(ABS(bpm->_y) > y) y = ABS(bpm->_y);
    }
    x += 11;
    y += 11;

    for(ii=0; ii < 4; ii++) {
	switch(ii) {
	case 1:
	    x = -x;		/* x negative, y positive */
	    break;
	case 2:
	    y = -y;		/* both negative */
	    break;
	case 3:
	    x = -x;		/* x postive, y negative */
	    break;
	default:		/* case 0: both positive */
	    break;
	}
	r = sqrt(SQ(x)+SQ(y));
	theta = atan2(y, x);
	theta = DEGREES(theta);
	theta = CART_ANGLE(theta);
	theta = FMOD360(theta);
	nn = xse_find_intxns(theta, r, ob);
	inside_count += (int)(nn & 1); /* i.e. and odd number of intxns */
    }
    ob->radar_inside_boundary = inside_count > 2;
    return(ob->radar_inside_boundary);
}
/* c------------------------------------------------------------------------ */

void se_redraw_all_bnds()
{
    int ii, nn, ww, mark;
    struct boundary_stuff *sebs, *return_se_bnd_struct();
    struct one_boundary *ob;
    struct solo_perusal_info *spi, *solo_return_winfo_ptr();

    spi = solo_return_winfo_ptr();
    sebs = return_se_bnd_struct();
    ob = sebs->first_boundary;

# ifdef obsolete
    for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	if(!spi->active_windows[ww] || !sebs->linked_windows[ww])
	      continue;
	se_clear_segments(ww);
    }
# endif

    for(; ob; ob = ob->next) {
	if(ob->num_points > 1) {
	    se_draw_bnd(ob->top_bpm->last, ob->num_points, NO);
	}
    }
}
/* c------------------------------------------------------------------------ */

void se_redraw_all_bnds_for_frame(frame_num)
     int frame_num;
{
    int ii, nn, ww, mark;
    struct boundary_stuff *sebs, *return_se_bnd_struct();
    struct one_boundary *ob;
    struct solo_perusal_info *spi, *solo_return_winfo_ptr();

    spi = solo_return_winfo_ptr();
    sebs = return_se_bnd_struct();
    ob = sebs->first_boundary;

# ifdef obsolete
    for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	if(!spi->active_windows[ww] || !sebs->linked_windows[ww])
	      continue;
	se_clear_segments(ww);
    }
# endif

    for(; ob; ob = ob->next) {
      if(ob->num_points > 1) {
	se_draw_bnd_for_frame
	  (frame_num, ob->top_bpm->last, ob->num_points, NO);
      }
    }
}
/* c------------------------------------------------------------------------ */

struct one_boundary *se_return_next_bnd()
{
    struct boundary_stuff *sebs, *return_se_bnd_struct();
    struct one_boundary *ob;
    struct solo_edit_stuff *seds, *return_sed_stuff();

    sebs = return_se_bnd_struct();

    if(sebs->current_boundary->next) {
	ob = sebs->current_boundary->next;
    }
    else {
	ob = se_malloc_one_bnd();
	sebs->current_boundary->next = ob;
	ob->last = sebs->current_boundary;
	sebs->first_boundary->last = ob;
    }
    sebs->current_boundary = ob;
    return(ob);
}
/* c------------------------------------------------------------------------ */

void xse_save_bnd ()
{
    int ii, bn, nn, mark, total_points, size, bnd_loop;
    FILE *stream, *ASCIIstream;
    struct point_in_space *pisp;
    struct boundary_header *bh;
    struct bnd_point_mgmt *bpm;
    WW_PTR wwptr, solo_return_wwptr();
    struct one_boundary *ob;
    struct boundary_stuff *sebs, *return_se_bnd_struct();
    char *a, *bb, str[256], mess[256], **buf, *bbuf;

    sebs = return_se_bnd_struct();

    /* loop through the boundaries and make sure there is some data
     */
    for(bnd_loop=0; bnd_loop < 2; bnd_loop++) {
	ob = sebs->first_boundary;
	for(nn=0; ob; ob = ob->next) {
	    if(ob->num_points > 1)
		  nn += ob->num_points;
	}
	if(nn) break;
	/* they may be referring to the previous set of boundaries
	 */
	se_prev_bnd_set();
    }
    if(!nn) {
	sprintf(mess,
		"There are not enough points in the current boundary!\n");
	sii_message(mess);
	return;
    }
    bh = sebs->first_boundary->bh;

    /* buffer up the boundary info
     */
    size = se_sizeof_bnd_set();
    bbuf = (char *)malloc(size);
    memset(bbuf, 0, size);
    se_pack_bnd_set(bbuf);

    se_dump_bnd_files_widget(sebs);
    if(!strlen(sebs->directory_text)) {
	wwptr = solo_return_wwptr(sebs->first_boundary->top_bpm->which_frame);
	slash_path(sebs->directory_text, wwptr->sweep->directory_name);
    }
    slash_path(str, sebs->directory_text);
    a = str +strlen(str);
    dd_file_name("bnd", (long)bh->time_stamp, bh->radar_name, getuid(), a);
    strcpy(sebs->file_name_text, a);
    a = a +strlen(a);
    if(strlen(sebs->comment_text)) {
	se_fix_comment(sebs->comment_text);
	strcat(a, ".");
	strcat(a, sebs->comment_text);
    }
    se_refresh_bnd_files_widget(sebs);

    if(!(stream = fopen(str, "w"))) {
	sprintf(mess, "Problem opening %s for writing\n", str);
	solo_message(mess);
	return;
    }
    /* we are also going to write an ascii file of boundary info
     */
    a = strstr(str, "bnd.");
    strncpy(a, "asb", 3);	/* change the prefix */
    if(!(ASCIIstream = fopen(str, "w"))) {
	sprintf(mess, "Problem opening %s for writing\n", str);
	solo_message(mess);
	return;
    }
    /* print a header line in the ASCII file
     */
    fprintf(ASCIIstream, "# solo boundary for: %s  file: %s\n"
	    , bh->radar_name, str);

    /* dump out the ascii file
     */
    ob = sebs->first_boundary;
    for(bn=0; ob; bn++, ob = ob->next) {
	if(ob->num_points > 1) {
	    bpm = ob->top_bpm;
	    for(; bpm; bpm = bpm->next) { /* each boundary point */
		se_ascii_bnd(ASCIIstream, bn, bpm->pisp);
	    }
	}
    }
    /* write out bbuf
     */
    if((nn = fwrite(bbuf, sizeof(char), (size_t)size, stream)) < size) {
	sprintf(mess, "Problem writing boundary info: %d\n", nn);
	solo_message(mess);
    }
    if(bnd_loop > 0) {		/* nabbed and saved the prev bnd set
				 * so clear it out again */
	se_clr_all_bnds();
    }
    if(bbuf) free(bbuf);
    fclose(stream);
    fclose(ASCIIstream);
}
# ifdef OLD_XNS
/* c------------------------------------------------------------------------ */

xse_set_intxn(x, y, slope, bpm, ob)
  long x, y;
  double slope;
  struct bnd_point_mgmt *bpm;
  struct one_boundary *ob;
{
    /* compute the intersection of a ray and a boundary segment
     * x & y are the endpoints of the ray and "slope" is
     * the slope of the ray and "bpm" represents the boundary segment
     *
     * when we are doing intersections, we want to use the shifted
     * x,y values because the boundary may be for another radar
     */
    struct bnd_point_mgmt *bpmx;
    double dd, xx, yy, sqrt();

    /*
     * first compute the x coordinate of the intersection
     */
    if(!x) {			/* the ray is vertical */
	xx = 0;
    }
    if(!bpm->dx) {		/* the segment is vertical */
	xx = bpm->_x;
    }
    else {			/* remember the origin of the ray is (0,0) */
	xx = (-bpm->slope*bpm->_x +bpm->_y)/(slope -bpm->slope);
    }
    if(x < 0) {			/* check for an imaginary intersection */
	if(xx < x || xx > 0)
	      return(NO);
    }
    else if(xx < 0 || xx > x)
	  return(NO);

    if(!y) {			/* the ray is horizontal */
	yy = 0;
    }
    else if(!bpm->dy) {		/* the segment is horizontal */
	yy = bpm->_y;
    }    
    else {
	yy = slope*xx;
    }
    if(y < 0) {			/* check for an imaginary intersection */
	if(yy < y || yy > 0)
	      return(NO);
    }
    else if(yy < 0 || yy > y)
	  return(NO);

    ob->num_intxns++;
    bpm->rx = sqrt(((double)SQ(xx)+(double)SQ(yy)));

    bpm->next_intxn = NULL;

    if(!(bpmx = ob->first_intxn)) {	/* first intersection */
	ob->first_intxn = bpm->last_intxn = bpm;
				/* first intxn always points to
				 * the last intxn tacked on */
	return(YES);
    }
    /*
     * insert this intxn and preserve the order
     */
    for(; bpmx; bpmx = bpmx->next_intxn) {
	if(bpm->rx < bpmx->rx) { 
				/* insert intxn here */
	    bpm->next_intxn = bpmx;
	    bpm->last_intxn = bpmx->last_intxn;
	    if(bpmx == ob->first_intxn) { /* new first intxn */
		ob->first_intxn = bpm;
	    }
	    else {
		bpmx->last_intxn->next_intxn = bpm;
	    }
	    bpmx->last_intxn = bpm;
	    break;
	}
    }
    if(!bpmx) {			/* furthest out...tack it onto the end */
	ob->first_intxn->last_intxn->next_intxn = bpm;
	bpm->last_intxn = ob->first_intxn->last_intxn;
	ob->first_intxn->last_intxn = bpm;
    }
    return(YES);
}
# else
/* c------------------------------------------------------------------------ */

int xse_set_intxn(x, y, slope, bpm, ob)
  double x, y;
  double slope;
  struct bnd_point_mgmt *bpm;
  struct one_boundary *ob;
{
    /* compute the intersection of a ray and a boundary segment
     * x & y are the endpoints of the ray and "slope" is
     * the slope of the ray and "bpm" represents the boundary segment
     *
     * when we are doing intersections, we want to use the shifted
     * x,y values because the boundary may be for another radar
     */
    struct bnd_point_mgmt *bpmx;
    double dd, xx, yy, sqrt();

    /*
     * first compute the x coordinate of the intersection
     */
    if(!x) {			/* the ray is vertical */
	xx = 0;
    }
    if(!bpm->dx) {		/* the segment is vertical */
	xx = bpm->_x;
    }
    else {			/* remember the origin of the ray is (0,0) */
	xx = (-bpm->slope*bpm->_x +bpm->_y)/(slope -bpm->slope);
    }
    if(x < 0) {			/* check for an imaginary intersection */
	if(xx < x || xx > 0)
	      return(NO);
    }
    else if(xx < 0 || xx > x)
	  return(NO);

    if(!y) {			/* the ray is horizontal */
	yy = 0;
    }
    else if(!bpm->dy) {		/* the segment is horizontal */
	yy = bpm->_y;
    }    
    else {
	yy = slope*xx;
    }
    if(y < 0) {			/* check for an imaginary intersection */
	if(yy < y || yy > 0)
	      return(NO);
    }
    else if(yy < 0 || yy > y)
	  return(NO);

    ob->num_intxns++;
    bpm->rx = sqrt(((double)SQ(xx)+(double)SQ(yy)));

    bpm->next_intxn = NULL;

    if(!(bpmx = ob->first_intxn)) {	/* first intersection */
	ob->first_intxn = bpm->last_intxn = bpm;
				/* first intxn always points to
				 * the last intxn tacked on */
	return(YES);
    }
    /*
     * insert this intxn and preserve the order
     */
    for(; bpmx; bpmx = bpmx->next_intxn) {
	if(bpm->rx < bpmx->rx) { 
				/* insert intxn here */
	    bpm->next_intxn = bpmx;
	    bpm->last_intxn = bpmx->last_intxn;
	    if(bpmx == ob->first_intxn) { /* new first intxn */
		ob->first_intxn = bpm;
	    }
	    else {
		bpmx->last_intxn->next_intxn = bpm;
	    }
	    bpmx->last_intxn = bpm;
	    break;
	}
    }
    if(!bpmx) {			/* furthest out...tack it onto the end */
	ob->first_intxn->last_intxn->next_intxn = bpm;
	bpm->last_intxn = ob->first_intxn->last_intxn;
	ob->first_intxn->last_intxn = bpm;
    }
    return(YES);
}
# endif
/* c------------------------------------------------------------------------ */

void se_shift_bnd(ob, boundary_radar, current_radar, scan_mode, current_tilt)
  struct one_boundary *ob;
  struct point_in_space *boundary_radar, *current_radar;
  double current_tilt;
{
    /* shift this boundary's points so as to be relative to
     * the current radar represented by "usi"
     */
    int mm;
    struct bnd_point_mgmt *bpm;
    struct point_in_space radar;
    double x, y, z, costilt, untilt, tiltfactor;

    /*
     * calculate the offset between the radar that is the origin
     * of the boundary and the current radar
     */
    dd_latlon_relative(boundary_radar, current_radar);
    x = KM_TO_M(current_radar->x);
    y = KM_TO_M(current_radar->y);
    z = KM_TO_M(current_radar->z);

    untilt = fabs(cos(RADIANS(boundary_radar->tilt)));

    if(fabs(costilt = cos(fabs(RADIANS(current_tilt)))) > 1.e-6) {
	tiltfactor = 1./costilt;
    }
    else {
	tiltfactor = 0;
    }
    bpm = ob->top_bpm;
    mm = ob->num_points;

    if(scan_mode == AIR) {
	for(; mm--; bpm = bpm->last) {
# ifdef notyet
	    /* we will need to project these points onto a plane
	     * perpendicular to the heading + plus the tilt of the beam
	     */

	    /* don't forget == RHI scan_mode!
	     */
# endif
	}
    }

    else {			/* ground based */
	/* you need to project the original values
	 * and then unproject the new values
	 */
	for(; mm--; bpm = bpm->last) {
	    bpm->_x = (bpm->x * untilt + x) * tiltfactor;
	    bpm->_y = (bpm->y * untilt + y) * tiltfactor;
	}
    }
}
/* c------------------------------------------------------------------------ */

int se_sizeof_bnd_set()
{
    /* determine you much space this boundary will occupy
     */
    int size=0;
    struct one_boundary *ob;
    struct boundary_stuff *sebs, *return_se_bnd_struct();
    struct bnd_point_mgmt *bpm;

    sebs = return_se_bnd_struct();
    ob = sebs->first_boundary;

    for(; ob; ob = ob->next) {
	if(ob->num_points > 1) {
	    size += sizeof(struct boundary_header);
	    size += sizeof(struct point_in_space); /* origin */
	    bpm = ob->top_bpm;
	    for(; bpm; bpm = bpm->next) { /* each boundary point */
		size += sizeof(struct point_in_space);
	    }
	}
    }
    return(size);
}
/* c------------------------------------------------------------------------ */

void se_unpack_bnd_buf(buf, size)
  char *buf;
  int size;
{
    int len, gottaSwap=NO;
    unsigned int gdsos;
    struct solo_click_info *sci, *clear_click_info();
    struct point_in_space *pisp;
    struct boundary_header *bh;
    struct bnd_point_mgmt *bpm;
    struct one_boundary *ob, *obnx, *ob_last;
    struct one_boundary *se_malloc_one_bnd();
    struct boundary_stuff *sebs, *return_se_bnd_struct();
    struct generic_descriptor *gd;
    char *cc=buf, *ee=buf+size;
    static char *pisp_swap=NULL;


    sebs = return_se_bnd_struct();
    obnx = sebs->first_boundary;
    sebs->num_boundaries = 0;
    sebs->absorbing_boundary = YES;

    for(;;) {
	gd = (struct generic_descriptor *)cc;
	gdsos = gd->sizeof_struct;
	if(gdsos > MAX_REC_SIZE) {
	   gottaSwap = YES;
	   swack4(&gd->sizeof_struct, &gdsos);
	}

	if(!strncmp(cc, "BDHD", 4)) {
	    if(!sebs->num_boundaries) {
		ob = sebs->current_boundary = sebs->first_boundary;
	    }
	    else {
		ob = se_return_next_bnd();
	    }
	    se_clr_current_bnd();

	    if(gottaSwap) {
	       se_crack_bdhd (cc, ob->bh, (int)0);
	    }
	    else {
	       memcpy(ob->bh, cc, gdsos);
	    }
	    if(!sebs->num_boundaries++) {
		memcpy(sebs->bh, ob->bh, gdsos);
	    }
	}
	else if(!strncmp(cc, "PISP", 4)) {
	   if(gottaSwap) {
	      if(!pisp_swap) {
		 pisp_swap = (char *)malloc(sizeof(struct point_in_space));
		 memset(pisp_swap, 0, sizeof(struct point_in_space));
	      }
	      se_crack_pisp(cc, pisp_swap, (int)0);
	      sebs->pisp = (struct point_in_space *)pisp_swap;
	   }
	   else {
	      sebs->pisp = (struct point_in_space *)cc;
	   }	    
	    if(strstr(sebs->pisp->id, "BND_ORIGIN")) {
		memcpy(sebs->origin, cc, gdsos);
	    }
	    else {
		sci = clear_click_info();
		if(sebs->pisp->state & PISP_TIME_SERIES) {
		    sci->x = sebs->pisp->x;
		    sci->y = sebs->pisp->y;
		}
		else {
		    sci->x = KM_TO_M(sebs->pisp->x);
		    sci->y = KM_TO_M(sebs->pisp->y);
		}
		xse_add_bnd_pt(sci, sebs->current_boundary);
	    }
	}
	cc += gdsos;
	if(cc >= ee) {
	    break;
	}
    }
    sebs->absorbing_boundary = NO;
}
/* c------------------------------------------------------------------------ */

void se_use_bnd(arg, cmds)
  int arg;
  struct ui_command *cmds;	
{
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();

    seds = return_sed_stuff();
    if(seds->finish_up)
	  return;
    if(strncmp(cmds->uc_text, "dont", 4) == 0) {
	seds->use_boundary = NO;
	seds->boundary_mask = seds->all_ones_array;
    }
    else {
	seds->boundary_mask = seds->boundary_mask_array;
	seds->use_boundary = YES;
    }
    return;
}  
/* c------------------------------------------------------------------------ */

void xse_x_insert(bpm, ob)
  struct bnd_point_mgmt *bpm;
  struct one_boundary *ob;
{
    /* insert sort of x coorinates of the midpoints of the line
     */
    int ii=0, nn;
    struct bnd_point_mgmt *bpmx;

    if(ob->num_points < 1)
	  return;
    bpm->x_left = bpm->x_right = NULL;

    if(!(bpmx = ob->x_mids)) {
	bpm->x_parent = NULL;
	ob->x_mids = bpm;
	return;
    }
    /*
     * the top node is an x value
     */
    if(bpm->pisp->state & PISP_TIME_SERIES) {
	for(;;) {
	    if(bpm->t_mid < bpmx->t_mid) {
		if(!bpmx->x_left) {
		    bpm->x_parent = bpmx;
		    bpmx->x_left = bpm;
		    break;
		}
		bpmx = bpmx->x_left;
	    }
	    else {
		if(!bpmx->x_right) {
		    bpm->x_parent = bpmx;
		    bpmx->x_right = bpm;
		    break;
		}
		bpmx = bpmx->x_right;
	    }
	}
    }
    else {
	for(;;) {
	    if(bpm->x_mid < bpmx->x_mid) {
		if(!bpmx->x_left) {
		    bpm->x_parent = bpmx;
		    bpmx->x_left = bpm;
		    break;
		}
		bpmx = bpmx->x_left;
	    }
	    else {
		if(!bpmx->x_right) {
		    bpm->x_parent = bpmx;
		    bpmx->x_right = bpm;
		    break;
		}
		bpmx = bpmx->x_right;
	    }
	}
    }
}
/* c------------------------------------------------------------------------ */

void xse_y_insert(bpm, ob)
  struct bnd_point_mgmt *bpm;
  struct one_boundary *ob;
{
    /* insert sort of x coorinates of the midpoints of the line
     */
    int ii=0, nn;
    struct bnd_point_mgmt *bpmx;

    if(ob->num_points < 1)
	  return;
    bpm->y_left = bpm->y_right = NULL;

    if(!(bpmx = ob->y_mids)) {
	bpm->y_parent = NULL;
	ob->y_mids = bpm;
	return;
    }
    /*
     * the top node is an x value
     */
    if(bpm->pisp->state & PISP_TIME_SERIES) {
	for(;;) {
	    if(bpm->r_mid < bpmx->r_mid) {
		if(!bpmx->y_left) {
		    bpm->y_parent = bpmx;
		    bpmx->y_left = bpm;
		    break;
		}
		bpmx = bpmx->y_left;
	    }
	    else {
		if(!bpmx->y_right) {
		    bpm->y_parent = bpmx;
		    bpmx->y_right = bpm;
		    break;
		}
		bpmx = bpmx->y_right;
	    }
	}
    }
    else {
	for(;;) {
	    if(bpm->y_mid < bpmx->y_mid) {
		if(!bpmx->y_left) {
		    bpm->y_parent = bpmx;
		    bpmx->y_left = bpm;
		    break;
		}
		bpmx = bpmx->y_left;
	    }
	    else {
		if(!bpmx->y_right) {
		    bpm->y_parent = bpmx;
		    bpmx->y_right = bpm;
		    break;
		}
		bpmx = bpmx->y_right;
	    }
	}
    }
}
/* c------------------------------------------------------------------------ */

struct bnd_point_mgmt *se_zap_last_bpm(top_bpm)
  struct bnd_point_mgmt **top_bpm;
{
    /* remove the last points from the list of boundary points
     */
    struct bnd_point_mgmt *last, *bpm = (*top_bpm);

    if(!bpm) {			/* no list yet */
	return(NULL);
    }
    if(bpm->last == bpm) {	/* list of one */
	*top_bpm = NULL;
    }
    else {
	/* point to next to last point
	 */
	bpm = bpm->last;
	(*top_bpm)->last = bpm->last;
	bpm->last->next = NULL;	/* last point always points to NULL */
	se_bnd_pt_atts(*top_bpm);
    }
    return(bpm);
}
/* c------------------------------------------------------------------------ */

char * absorb_zmap_bnd( ifile, skip, nbytes )
    char * ifile;
    int skip, *nbytes;
{
    /* routine to read in a zmap file and convert it to a boundary
     */
    FILE *fs;
    char str[256], *aa, *cc, *ee, line[256], *str_ptrs[32], mess[256], *bbuf;
    int ii, jj, len, lenx, mark, nt, trip=0;
    int max_pts = 0;
    int point_count = 1;
    int size, offs = 0;
    struct boundary_header *bh;
    struct point_in_space origin, pisp, p1;
    int spisp = sizeof( struct point_in_space );
    struct dd_general_info *dgi, *dd_window_dgi();
    WW_PTR wwptr, solo_return_wwptr();

    wwptr = solo_return_wwptr( 0 );
    memcpy( &origin, wwptr->radar_location
	    , sizeof( struct point_in_space ) );
    memcpy( &p1, wwptr->radar_location
	    , sizeof( struct point_in_space ) );
    memcpy( &pisp, wwptr->radar_location
	    , sizeof( struct point_in_space ) );
    dgi = dd_window_dgi( 0, "" );
    strcpy( str, ifile );

    if(!(fs = fopen(str, "r"))) {
	sprintf(mess, "Unable to open zmap boundary file %s\n", str);
	solo_message(mess);
	return(NULL);
    }

    max_pts = 100;
    offs = size = sizeof( struct boundary_header );
    size += ( max_pts +1 ) * sizeof( struct point_in_space );
    bbuf = (char *)malloc( size );
    memset( bbuf, 0, size );
    /*
     * boundary header
     */
    bh = (struct boundary_header *)bbuf;
    strncpy( bh->name_struct, "BDHD", 4);
    bh->sizeof_struct = sizeof(struct boundary_header);
    bh->offset_to_origin_struct = sizeof(struct boundary_header);
    bh->offset_to_first_point = bh->offset_to_origin_struct + spisp;
    bh->sizeof_boundary_point = sizeof(struct point_in_space);
    solo_return_radar_name(0, bh->radar_name);
    /*
     * boundary origin
     */
    strcpy( origin.id, "BND_ORIGIN");
    memcpy( bbuf + offs, &origin, spisp );
    offs += spisp;

    while( fgets( line, sizeof(line), fs ) != NULL ) {

	nt = dd_tokens( line, str_ptrs );
	if (nt & 1)
	  { nt--; }
	/*
	 * make sure we aren't going to overrun our space
	 */
	if( offs + nt/2 * spisp > size ) { /* need to realloc space */
	    size = sizeof( struct boundary_header );
	    max_pts += 100;
	    size += ( max_pts +1 ) * spisp;
	    if( !( bbuf = (char *)realloc( bbuf, size ) )) {
		sprintf(mess, "Unable to malloc space for zmap boundary: %d\n"
		    , point_count );
		solo_message(mess);
		return(NULL);
	    }
	}

	for(ii = 0; ii < nt; ) {
	    p1.latitude = atof( str_ptrs[ii++] );
	    p1.longitude = atof( str_ptrs[ii++] );

	    /* the (x,y,z) for p1 relative to the origin
	     * are going to come back in pisp
	     */
	    dd_latlon_relative( &p1, &pisp );
	    pisp.range = sqrt((SQ((double)pisp.x)+SQ((double)pisp.y)));
	    if( skip && ( trip++ % skip ) != 0 )
		{ continue; }
	    memcpy( bbuf + offs, &pisp, spisp );
	    offs += spisp;
	    *nbytes = offs;
	    point_count++;
	}
    }
    return( bbuf );
}
/* c------------------------------------------------------------------------ */

int absorb_zmap_pts( top, ifile )
    struct zmap_points **top;
    char * ifile;
{
  /* This version nabs the Florida Precip98 gauges */
  /* This routine is called in se_histog_setup() in se_histog.c */

    FILE *fs;
    char str[256], *aa, *bb, line[256], mess[256];
    char *sptrs[32];
    int mark, nt, nn, point_count = 0;
    double d;
    struct zmap_points * zmp, *zmpt, *zmpl = NULL;
    int lon, lon_mm, lat, lat_mm;
    float lon_ss, lat_ss;


    strcpy( str, ifile );

    if(!(fs = fopen(str, "r"))) {
	sprintf(mess, "Unable to open zmap points file %s\n", str);
	solo_message(mess);
	return(-1);
    }

    while( fgets( line, sizeof(line), fs ) != NULL ) {
	if( *line == '!' )	/* comment */
	    { continue; }

	if( aa = strchr( line, '!' )) {
	  *aa = '\0';
	}

	strcpy( str, line );
	nt = dd_tokens( str, sptrs );

	if( !strcmp( sptrs[0], "type" ))
	  { continue; }

	if( !point_count++ ) {	/* start list */
	    zmp = *top;
	    if( !zmp ) {
		zmp = ( struct zmap_points *)
		    malloc( sizeof( struct zmap_points ));
		memset( zmp, 0, sizeof( struct zmap_points ));
		*top = zmp;
	    }
	}	
	else if( !zmpl->next ) { /* extend list */
	    zmpl->next = zmp = ( struct zmap_points *)
		malloc( sizeof( struct zmap_points ));
	    memset( zmp, 0, sizeof( struct zmap_points ));
	}
	else {
	    zmp = zmpl->next;
	}
	zmpl = zmp;

	nn = sscanf( line, "%s%s%d%d%f%d%d%f", zmp->list_id, zmp->id
		     , &lon, &lon_mm, &lon_ss, &lat, &lat_mm, &lat_ss );

	if( nn == 8 ) {		/* two styles of input */
	  d = lon_mm/60. + lon_ss/3600.;
	  zmp->lon = lon < 0 ? (double)lon -d : (double)lon +d;
	  d = lat_mm/60. + lat_ss/3600.;
	  zmp->lat = lat < 0 ? (double)lat -d : (double)lat +d;
	}
	else {
	  zmp->lat = atof( sptrs[2] );
	  zmp->lon = atof( sptrs[3] );
	}
	strcpy( zmp->list_id, sptrs[0] ); /* nasa id */
	strcpy( zmp->id, sptrs[1] ); /* network id */
    }

    return( point_count );
}

/* c------------------------------------------------------------------------ */

int se_rain_gauge_info( dlat, dlon )
     double dlat, dlon;
{
  /* This routine lists all the rain gauges within a given radius
   * of the input lat/lon
   */
  char mess[256];
  int ii, jj, kk, mark, num_pts = 0, num_hits = 0;
  struct point_in_space p0, p1;
  struct zmap_points * zmpc, *zzmpc[200], *ztmp;
  struct solo_edit_stuff *seds, *return_sed_stuff();
  double min_radius = 15;	/* km. */
  double min_val;

  seds = return_sed_stuff();

  if( !seds->top_zmap_list )
    { return(0); }

  zmpc = seds->top_zmap_list;
  p0.altitude = p1.altitude = 0;

  for(; zmpc ; zmpc = zmpc->next ) {
    num_pts++;
    p0.latitude = dlat;
    p0.longitude = dlon;
    p1.latitude = zmpc->lat;
    p1.longitude = zmpc->lon;
    /*
     * the (x,y,z) for p1 relative to p0
     * are going to come back in p0
     */
    dd_latlon_relative( &p1, &p0 );
    zmpc->x = p0.x;
    zmpc->y = p0.y;
    zmpc->rng = sqrt(SQ(p0.x) + SQ(p0.y));
    zmpc->number = zmpc->rng < min_radius;
    if( zmpc->rng < min_radius ) {
      zzmpc[num_hits++] = zmpc;
    }
  }
  if( !num_hits ) {
    sprintf( mess, "No gauges within %.3f of lat: %.4f lon: %.4f\n"
	     , min_radius, dlat, dlon );
    solo_message(mess);
    return( 0 );
  }
  
  for(ii = 0; ii < num_hits -1; ii++ ) {
    for( jj = ii+1; jj < num_hits; jj++ ) {
      if( zzmpc[jj]->rng < zzmpc[ii]->rng ) {
	ztmp = zzmpc[ii];
	zzmpc[ii] = zzmpc[jj];
	zzmpc[jj] = ztmp;
      }
    }
  }
  mark = 0;
  solo_message( "\n" );

  for(ii = 0; ii < num_hits; ii++ ) {
    zmpc = zzmpc[ii];
    sprintf( mess, "%10s rng:%7.2f x:%7.2f y:%7.2f\n"
	     , zmpc->list_id, zmpc->rng, zmpc->x, zmpc->y );
    solo_message(mess);
  }
  return(num_hits);
}

# ifdef obsolete
/* c------------------------------------------------------------------------ */

int xabsorb_zmap_pts( top, ifile )
    struct zmap_points **top;
    char * ifile;
{
# define STATION_MODE 1
# define DATA_MODE 2
# define STATION_OFFSET 9	/* column 10 */
# define ID_OFFSET 24		/* column 25 */
# define LAT_OFFSET 31		/* column 32 */

    FILE *fs;
    char str[256], *aa, *bb, line[256], *str_ptrs[32], mess[256];
    char *chunks[32], id[32], ns[32], ew[32];
    char lname[16], *name = NULL;
    int ii, jj, nt, mark, point_count = 0, nc, mode = DATA_MODE, number, nn;
    float val, lat, lon, azm, rng, x, y;
    static int list_count = 0;
    struct zmap_points * zmp, *zmpt, *zmpl = NULL;


    strcpy( str, ifile );
    lname[0] = '\0';

    if(!(fs = fopen(str, "r"))) {
	sprintf(mess, "Unable to open zmap points file %s\n", str);
	solo_message(mess);
	return(-1);
    }

    while( fgets( line, sizeof(line), fs ) != NULL ) {
	if( *line == '#' )	/* comment */
	    { continue; }

	if( ! point_count && !strncmp( line, "number", 6 )) {
	    /* station ids and locations */
	    mode = STATION_MODE;
	    continue;
	}

	if( mode == DATA_MODE ) {
	    if( ! point_count ) {
		zmp = *top;
		for(; zmp; zmp->src_val = -1, zmp = zmp->next );
	    }
	    zmp = *top;

	    nt = dd_tokens( line, str_ptrs );
	    if( sscanf( str_ptrs[1], "%d", &nn ) != 1 ) {
		continue;
	    }
	    if( sscanf( str_ptrs[2], "%f", &val ) != 1 ) {
		continue;
	    }
	    /* find the station number */
	    for(; zmp ; zmp = zmp->next ) {
		if( nn == zmp->number )
		    { break; }
	    }
	    if(!zmp )
		{ continue; }
	    zmp->src_val = val;
	    point_count++;

	    continue;
	}

	/* STATION MODE
	 */

	str_terminate( id, line, 5 ); /* station number */

	if( !strlen( id ))
	    { nn = 0; }
	else if( sscanf( id, "%d", &nn ) != 1 ) /* no station number */
	    { nn = 0; }

	str_terminate( id, line +ID_OFFSET, 6 ); /* get station the id */
	if( !strlen( id )) {	/* no id */
	    str_terminate( id, line +STATION_OFFSET, 5 ); /* substitute */
	}

	if( sscanf( line + LAT_OFFSET, "%f%s%f%s%f%f%f%f"
		    , &lat, ns, &lon, ew
		    , &azm, &rng, &x, &y ) != 8 )
	    { continue; }


	if( !point_count++ ) {	/* start list */
	    zmp = *top;
	    if( !zmp ) {
		zmp = ( struct zmap_points *)
		    malloc( sizeof( struct zmap_points ));
		memset( zmp, 0, sizeof( struct zmap_points ));
		*top = zmp;
	    }
	}	
	else if( !zmpl->next ) { /* extend list */
	    zmpl->next = zmp = ( struct zmap_points *)
		malloc( sizeof( struct zmap_points ));
	    memset( zmp, 0, sizeof( struct zmap_points ));
	}
	else {
	    zmp = zmpl->next;
	}
	zmpl = zmp;

	zmp->number = nn;
	strcpy( zmp->id, id );
	zmp->lat = *ns == 'N' ? lat : -lat;
	zmp->lon = *ew == 'E' ? lon : -lon;
	zmp->azm = azm;
	zmp->rng = rng;
	zmp->x = x;
	zmp->y = y;
    }

    return( point_count );
}

# endif
/* c------------------------------------------------------------------------ */

int list_zmap_values( fname, frame )
    char * fname;
    int frame;
{
    int ii, jj, kk, nn, pn, gate, one=1, point_count = 0;
    int numRatios = 0;
    char mess[256], *aa;
    float rng, cell_val, val, bad;
    struct point_in_space origin, pisp, p1;
    struct dd_general_info *dgi, *dd_window_dgi();
    WW_PTR wwptr, solo_return_wwptr();
    double x, y, theta, correlation, ratio;
    double sumx=0, sumxx=0, sumy=0, sumyy=0, sumxy=0, dnum, dnom;
    double sumRatios = 0, ratio2;
    struct rot_table_entry *entry1, *dd_return_rotang1();
    struct rot_ang_table *rat;
    struct zmap_points * zmpc;
    struct solo_str_mgmt *ssm, *se_pop_spair_string();
    struct solo_edit_stuff *seds, *return_sed_stuff();
    DD_TIME dts;


    seds = return_sed_stuff();
    se_push_all_ssms(&seds->h_output);

    wwptr = solo_return_wwptr( frame )->lead_sweep;
    dgi = dd_window_dgi( wwptr->window_num, "" );
    rat = dgi->source_rat;	/* rotation angle table */
    entry1 = dd_return_rotang1( rat );
    memcpy( &p1, wwptr->radar_location
	    , sizeof( struct point_in_space ) );
    memcpy( &pisp, wwptr->radar_location
	    , sizeof( struct point_in_space ) );
    zmpc = seds->curr_zmap_list;

    for(; zmpc ; zmpc = zmpc->next ) {
	if( zmpc->src_val < 0 )
	    { continue; }

	/* loop through the points in this list
	 */
	p1.latitude = zmpc->lat;
	p1.longitude = zmpc->lon;

	/* the (x,y,z) for p1 relative to the origin
	 * are going to come back in pisp
	 */
	dd_latlon_relative( &p1, &pisp );
	theta = zmpc->azm;
	theta = atan2( pisp.y, pisp.x );
	theta = FMOD360(CART_ANGLE(DEGREES(theta)));
	dd_absorb_header_info(dgi);
	dgi_buf_rewind(dgi);
	kk = dd_rotang_seek(rat, (float)theta);
	nn = lseek(dgi->in_swp_fid, (long)(entry1+kk)->offset, 0L);
	dd_absorb_ray_info(dgi);
	pn = dd_find_field( dgi, fname );
	if( pn < 0 ) {
	    sprintf( mess, "Field: %s cannot be accesses for %s\n"
		     , fname, zmpc->id );
	    uii_printf( mess );
	    continue;
	}
	rng = KM_TO_M( zmpc->rng );
	rng = KM_TO_M( sqrt((SQ(pisp.x)+SQ(pisp.y))) )/
	    COS( RADIANS( dd_elevation_angle(dgi)) );
	dd_range_gate( dgi, &rng, &gate, &cell_val );
	nn = dd_givfld( dgi, &pn, &gate, &one, &val, &bad );
	zmpc->curr_val = val;
	sumx += zmpc->src_val;
	sumxx += SQ( zmpc->src_val );
	sumy += zmpc->curr_val;
	sumyy += SQ( zmpc->curr_val );
	sumxy += zmpc->src_val * zmpc->curr_val;
	ratio = zmpc->curr_val ? zmpc->src_val/zmpc->curr_val : 0;
	if( ratio ) {
	    ++numRatios;
	    sumRatios += ratio;
	}
	if( !point_count++ ) {	/* top line */
	   ssm = se_pop_spair_string();
	   dts.time_stamp = dgi->time;
	   dd_file_name("rgc", (long)dgi->time
		     , dgi->radar_name, 0, seds->histo_filename);
	   sprintf( ssm->at, "\nSite values for %s for %s for"
		   , dts_print(d_unstamp_time(&dts))
		   , str_terminate( mess, dgi->dds->radd->radar_name, 8 )
		   );
	   sprintf( ssm->at + strlen(ssm->at), " %s in frame %d\n"
		   , str_terminate( mess, dgi->dds->parm[pn]->parameter_name, 8 )
		   , frame+1 );

	   se_append_string(&seds->h_output, ssm);
	}

	ssm = se_pop_spair_string();

	sprintf( ssm->at
 , "%5s %7.3f %8.3f   rg:%5.1f rd:%5.1f d:%5.1f   da:%5.1f dr:%6.3f\n"
		 , zmpc->id, zmpc->lat, zmpc->lon, zmpc->src_val
		 , zmpc->curr_val, zmpc->src_val - zmpc->curr_val
		 , zmpc->azm-theta, zmpc->rng - M_TO_KM(rng)
	    );

	se_append_string(&seds->h_output, ssm);
    }
    nn = point_count;
    dnum = nn * sumxy - sumx * sumy;
    dnom = sqrt((double)(( nn*sumxx - SQ(sumx) ) *
			 ( nn*sumyy - SQ(sumy))));
    correlation = nn > 0 && dnom > 0 ? dnum/dnom : 0;
    ratio = sumy ? sumx/sumy : 0;
    ratio2 = numRatios ? sumRatios/(float)numRatios : 0;
    ssm = se_pop_spair_string();
    sprintf( ssm->at
	     , "\nCorrelation:%6.3f g/r:%6.3f %6.3f for %s\n"
	     , correlation
	     , ratio
	     , ratio2
	     , fname
	);
    se_append_string(&seds->h_output, ssm);
    se_histo_output();
}
/* c------------------------------------------------------------------------ */


/* c------------------------------------------------------------------------ */


/* c------------------------------------------------------------------------ */





