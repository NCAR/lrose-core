
#include "Solo/OneBoundary.hh"
#include <cstdio>


OneBoundary::OneBoundary() {
  num_points = 0;
  x_mids = NULL;
  y_mids = NULL;
  next = NULL;
  last = NULL;
  top_bpm = NULL;
  first_intxn = NULL;
  next_segment = NULL;
  bh = NULL;
}

// OneBoundary::OneBoundary(int *xpoints, int *ypoints, int npoints) {}

OneBoundary::~OneBoundary() {}

void OneBoundary::print() {
  printf("OneBoundary:\n");
  printf("num_points %d\n", num_points);
  printf("num_intxns %d\n", num_intxns);
  printf("num_segments %d\n", num_segments);

  printf("top_bpm ...\n");
  if (top_bpm == NULL)
    printf("  is NULL\n");
  else {
    BoundaryPointManagement *bpm = top_bpm;
    while (bpm != NULL) {
      bpm->print();
      bpm = bpm->next;
    }
  }


  printf("next_segment ...\n");
  if (next_segment == NULL)
    printf("  is NULL\n");
  else {
    BoundaryPointManagement *bpm = next_segment;
    while (bpm != NULL) {
      bpm->print();
      bpm = bpm->next;
    }
  }

  printf("radar_inside_boundary ");
  if (radar_inside_boundary)
    printf("true\n");
  else
    printf("false\n");

  printf("first_intxn ...\n");
  if (first_intxn == NULL)
    printf("  is NULL\n");
  else {
    BoundaryPointManagement *bpm = first_intxn;
    bpm->print_intxns();
    //    while (bpm != NULL) {
    //  bpm->print();
    //  bpm = bpm->next;
    //}
  }


  printf("OneBoundary end\n");
}


/*
// TODO: x,y are in which coordinates?
void OneBoundary::AddBoundaryPoint(int x, int y, int frame) {
  // add a point to the boundary based on the x & y coordinates
  // in the click struct
  //
  //BoundaryStuff *sebs, *return_se_bnd_struct();
  PointInSpace *pisp, *solo_return_radar_pisp();
  // struct solo_edit_stuff *seds, *return_sed_stuff();

  BoundaryPointManagement *bpm, *se_pop_bpms();
  double d, sqrt(), fabs();
  //  double x = sci->x, y = sci->y;

  // seds = return_sed_stuff();
  sebs = return_se_bnd_struct();
  bpm = se_pop_bpms();        // nab a fresh boundary point 
  se_append_bpm(&top_bpm, bpm); // the append happens first so
                                //     * the last pointers are fresh 
  bpm->x = x;
  bpm->y = y;

  if(++num_points > 1) {  // avoid duplicates 
    if((bpm->x == bpm->last->x) && (bpm->y == bpm->last->y)) {
      num_points--;
      se_delete_bnd_pt(bpm);
      return;
    }
  }
  // TODO: move this frame stuff outside of this function;
  // it is specific to the user interface
  //bpm->which_frame = frame;

  if(!(sebs->view_bounds || sebs->absorbing_boundary)) {
    bpm->LocateThisPoint(x, y, frame); // this routine is in
                                       // * ../perusal/sp_clkd.c 
    // PointInSpace structure that is filled resides inside bpm
    // to access it, use bpm->pisp
  }
  else if(sebs->absorbing_boundary) {
    memcpy(bpm->pisp, sebs->pisp, sizeof(struct point_in_space));
    bpm->r = KM_TO_M(bpm->pisp->range);
  }
  // state can be PISP_TIME_SERIES, or PISP_AZELRG or PISP_EARTH
  if(bpm->pisp->state & PISP_TIME_SERIES) {
    x = bpm->pisp->time;
    y = bpm->pisp->range;
  }
  // the rasterization code sets sebs->view_bounds to YES and also uses
  // this routine and others to bound the rasterization   
  if(num_points > 1) {
    if(!sebs->view_bounds) {
      se_draw_bnd(bpm, 2, NO);
      // TODO: how to store/communicate this global info?
      //seds->boundary_exists = YES;
      //sii_set_boundary_pt_count (frame, num_points);
    }
    if(x > max_x)
      max_x = x;
    if(x < min_x)
      min_x = x;
    if(y > max_y)
      max_y = y;
    if(y < min_y)
      min_y = y;

    se_bnd_pt_atts(bpm);
    xse_x_insert(bpm, ob);
    xse_y_insert(bpm, ob);

    //
    // * now do it for the line between this point and the first point
     
    bpm = top_bpm;
    se_bnd_pt_atts(bpm);
  }
  else {                      // first point 
    if(!(sebs->absorbing_boundary || sebs->view_bounds)) {
      // get the radar origin and radar name from the first point                                       
       
      solo_return_radar_name(frame, bh->radar_name);
      memcpy(sebs->origin, bpm->pisp, sizeof(struct point_in_space));
      strcpy(sebs->origin->id, "BND_ORIGIN");
    }
    min_x = max_x = x;
    min_y = max_y = y;
  }

}
*/
