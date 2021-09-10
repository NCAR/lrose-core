// Contains the operations for boundaries

// code extracted from soloii with these settings:
// # define OLD_XNSx  <== OLD_XNS is NOT defined.
// #define obsolete   <== obsolete is NOT defined.

// trying to figure out the Solo boundary point code

// There must be a list of points, that get translated from display coordinates into
// radar coordinates, then some list of points that are either inside the boundary
// or outside the boundary.
// TODO: how do we know if a point in a RadxVol is inside or outside a boundary?

// The data structures are changed from Soloii.  
// No globals
// No sed structure
//
//

#include <cmath>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#include "Solo/BoundaryPointMap.hh"
#include "Solo/dd_math.h"

/* c------------------------------------------------------------------------ */

void BoundaryPointMap::se_delete_bnd_pt(BoundaryPointManagement *bpm,
                                        OneBoundary *ob)
{
  /* remove this point from the boundary                                                 
   * remember to recalculate mins, maxs, slopes and deltas                               
   */
  //  struct boundary_stuff *sebs, *return_se_bnd_struct();
  //BoundaryPointManagement *bpmx;

  //sebs = return_se_bnd_struct();
  //ob = sebs->current_boundary;

  if(bpm == ob->top_bpm) {
    if(!bpm->next) {        /* this is the only point */
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
  // memory management; bpms are maintained in a stack called bpm_spairs
  //se_push_bpm(bpm);
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

// add a BoundaryPointManagement block to the double linked list
// kept by OneBoundary
void BoundaryPointMap::xse_add_bnd_pt(long x, long y, OneBoundary *ob, 
                                      bool time_series) {

  BoundaryPointManagement *bpm = new BoundaryPointManagement();

  se_append_bpm(&ob->top_bpm, bpm); /* the append happens first so                       
                                     * the last pointers are fresh */

  bpm->x = x;
  bpm->y = y;
  PointInSpace *pisp = new PointInSpace();
  bpm->pisp = pisp;
  pisp->state = PointInSpace::PISP_AZELRG | PointInSpace::PISP_EARTH;
  if (time_series) {
    pisp->state |= PointInSpace::PISP_TIME_SERIES;
  }

  if(++ob->num_points > 1) {  /* avoid duplicates */
    if((bpm->x == bpm->last->x) && (bpm->y == bpm->last->y)) {
      ob->num_points--;
      se_delete_bnd_pt(bpm, ob);
      return;
    }
  }
  //# ifdef obsolete
  //  bpm->r = .5 +sqrt((SQ((double)x)+SQ((double)y)));
  //# endif
  //  bpm->which_frame = sci->frame;

  //if(!(sebs->view_bounds || sebs->absorbing_boundary)) {
  // loads the pisp struct with all the position info 
  // TODO: put this coordinate transform from world to local in a controller?
  //  sp_locate_this_point(sci, bpm); // this routine is in ../perusal/sp_clkd.c 
    //# ifdef obsolete
    //    bpm->pisp->state =
    //      PISP_PLOT_RELATIVE | PISP_AZELRG |  PISP_EARTH;
    //    strcpy(bpm->pisp->id, "BND_PT_V1");
    //# endif
   
      //}
  // TODO: I don't understand this ... calculating the range?
  // What is "absorbing_boundary" mean?  
  //else if(sebs->absorbing_boundary) {
  //  memcpy(bpm->pisp, sebs->pisp, sizeof(PointInSpace));
  //  bpm->r = KM_TO_M(bpm->pisp->range);
  //}
  // TODO: I don't understand this ...
  //if(bpm->pisp->state & PISP_TIME_SERIES) {
  //  x = bpm->pisp->time;
  //  y = bpm->pisp->range;
  //}
  /* the rasterization code sets sebs->view_bounds to YES and also uses                  
   * this routine and others to bound the rasterization                                  
   */
  if(ob->num_points > 1) {
    //if(!sebs->view_bounds) {  // this is UI stuff
      //se_draw_bnd(bpm, 2, NO);
      //seds->boundary_exists = YES;
      //sii_set_boundary_pt_count (sci->frame, ob->num_points);
    //}

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
  else {                      /* first point */
    //if(!(sebs->absorbing_boundary || sebs->view_bounds)) {
      /* get the radar origin and radar name from the first point                    
       */
      //solo_return_radar_name(sci->frame, ob->bh->radar_name);
      //memcpy(sebs->origin, bpm->pisp, sizeof(struct point_in_space));
      //strcpy(sebs->origin->id, "BND_ORIGIN");
    //}

    ob->min_x = ob->max_x = x;
    ob->min_y = ob->max_y = y;
  }
  //cout << "xse_add_bnd_pt finished  ... " << endl;
  //ob->print();
}

/* c------------------------------------------------------------------------ */

void BoundaryPointMap::xse_x_insert(BoundaryPointManagement *bpm, 
                                    OneBoundary *ob)
//  struct bnd_point_mgmt *bpm;
//struct one_boundary *ob;
{
  /* insert sort of x coorinates of the midpoints of the line                            
   */
  //int ii=0, nn;
  BoundaryPointManagement *bpmx;

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
   * TODO: the code is the same, only the comparison is different;
   *   make a separate function???? for the comparison
   */
  if(bpm->pisp->state & PointInSpace::PISP_TIME_SERIES) {
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

void BoundaryPointMap::xse_y_insert(BoundaryPointManagement *bpm,
                               OneBoundary *ob)
{
  /* insert sort of x coorinates of the midpoints of the line                            
   */
  //int ii=0, nn;
  BoundaryPointManagement *bpmx;

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
  if(bpm->pisp->state & PointInSpace::PISP_TIME_SERIES) {
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

void BoundaryPointMap::se_bnd_pt_atts(BoundaryPointManagement *bpm)
//  struct bnd_point_mgmt *bpm;
{

  if(bpm->pisp->state & PointInSpace::PISP_TIME_SERIES) {
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

void BoundaryPointMap::se_append_bpm(BoundaryPointManagement **top_bpm, 
BoundaryPointManagement *bpm)
//  struct bnd_point_mgmt **top_bpm, *bpm;
{
  /* append this point to the list of boundary points                                    
   * (*top_bpm)->last always points to the last point appended                           
   * to the boundary this all points are linked by the last pointer                      
   * except for the last point appended the next pointer points                          
   * to the next point and the NULL pointer in the last point                            
   * serves to terminate loops                                                           
   */
  if(!(*top_bpm)) {                   /* no list yet */
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


  // determine if point is left or right of line 
int BoundaryPointMap::xse_ccw(double x0, double y0, double x1, double y1)
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
  return(0);                  /* on the line */
}

/* c------------------------------------------------------------------------ */

// The return value is not used; The function produces
// side effects only; it add a new intersection point,
// if there is one.
void BoundaryPointMap::xse_set_intxn(double x, double y, double slope,
                                     BoundaryPointManagement *bpm,
                                     OneBoundary *ob)
{
    /* compute the intersection of a ray and a boundary segment
     * x & y are the endpoints of the ray and "slope" is
     * the slope of the ray and "bpm" represents the boundary segment
     *
     * when we are doing intersections, we want to use the shifted
     * x,y values because the boundary may be for another radar
     */
    BoundaryPointManagement *bpmx;
    double xx, yy; //  sqrt();

    /*
     * first compute the x coordinate of the intersection
     */
    if(!x) {                    /* the ray is vertical */
        xx = 0;
    }
    if(!bpm->dx) {              /* the segment is vertical */
	xx = bpm->_x;
    }
    else {                      /* remember the origin of the ray is (0,0) */
        xx = (-bpm->slope*bpm->_x +bpm->_y)/(slope -bpm->slope);
    }
    if(x < 0) {                 /* check for an imaginary intersection */
        if(xx < x || xx > 0)
          return; // (NO);
    }
    else if(xx < 0 || xx > x)
      return; // (NO);

    if(!y) {                    /* the ray is horizontal */
        yy = 0;
    }
    else if(!bpm->dy) {         /* the segment is horizontal */
        yy = bpm->_y;
    }
    else {
        yy = slope*xx;
    }
    if(y < 0) {                 /* check for an imaginary intersection */
        if(yy < y || yy > 0)
          return; // (NO);
    }
    else if(yy < 0 || yy > y)
      return; // (NO);

    ob->num_intxns++;
    bpm->rx = sqrt(((double)(xx*xx)+(double)(yy*yy)));

    bpm->next_intxn = NULL;

    if(!(bpmx = ob->first_intxn)) {     /* first intersection */
        ob->first_intxn = bpm->last_intxn = bpm;
                                /* first intxn always points to
                                 * the last intxn tacked on */
        return; // (YES);
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
    if(!bpmx) {                 /* furthest out...tack it onto the end */
        ob->first_intxn->last_intxn->next_intxn = bpm;
        bpm->last_intxn = ob->first_intxn->last_intxn;
        ob->first_intxn->last_intxn = bpm;
    }
    return; // (YES);
}



/* c------------------------------------------------------------------------ */
//
// return value is not used; function used for side effect only
// sets ob->radar_inside_boundary to true or false
// 
void BoundaryPointMap::se_radar_inside_bnd(OneBoundary *ob)
{
  // determine if the radar is inside or outside the boundary

  BoundaryPointManagement *bpm; //  *bpmx;
  //double dd  , sqrt(), atan2(), fmod();
  double r, x, y, theta;
  int ii, nBoundaryPoints = ob->num_points-1, nn, inside_count=0;


  // TODO: ignoring the boundary header for now 
  if (0) {
  if(ob->bh->force_inside_outside) {
    if(ob->bh->force_inside_outside == BND_FIX_RADAR_INSIDE) {
      ob->radar_inside_boundary = true;
    }
    if(ob->bh->force_inside_outside == BND_FIX_RADAR_OUTSIDE) {
      ob->radar_inside_boundary = false;
    }
  }
  }
  // 

  bpm = ob->top_bpm;
  x = abs(bpm->_x);
  y = abs(bpm->_y);
  /*
   * we are using the shifted values of x and y in case this boundary
   * is for a different radar
   */
  for(bpm = bpm->next; nBoundaryPoints--; bpm = bpm->next) {
    if(abs(bpm->_x) > x) x = abs(bpm->_x);
    if(abs(bpm->_y) > y) y = abs(bpm->_y);
  }
  x += 11;
  y += 11;

  for(ii=0; ii < 4; ii++) {
    switch(ii) {
      case 1:
        x = -x;             /* x negative, y positive */
        break;
      case 2:
        y = -y;             /* both negative */
        break;
      case 3:
        x = -x;             /* x postive, y negative */
        break;
      default:                /* case 0: both positive */
        break;
    }
    r = sqrt((x*x)+(y*y));
    theta = atan2(y, x);
    theta = DEGREES(theta);
    theta = CART_ANGLE(theta);
    theta = FMOD360(theta);
    nn = xse_find_intxns(theta, r, ob);
    // there must be side effects to find_intxns; what are they?

    inside_count += (int)(nn & 1); /* i.e. and odd number of intxns */
  }
  ob->radar_inside_boundary = inside_count > 2;
  // return(ob->radar_inside_boundary);
}


/* c------------------------------------------------------------------------ */

int BoundaryPointMap::xse_find_intxns(double angle, double range,
                                      OneBoundary *ob)
{
    /* this routine creates a list of endpoints of lines that 
     * intersect the current ray
     */   
    BoundaryPointManagement *bpm, *bpmx;
    double theta = RADIANS(CART_ANGLE(angle)); //, cos(), sin();
    double slope=0, xx, yy;
    long x, y;
    int  mm; // ii, nn, nx=0;


    ob->num_intxns = 0; 
    ob->first_intxn = NULL;
    /*   
     * compute the endpoints of the ray
     */   
    xx = range * cos(theta);
    yy = range * sin(theta);
    x = xx < 0 ? xx -.5 : xx +.5; 
    y = yy < 0 ? yy -.5 : yy +.5; 
    if(xx) slope = yy/xx;

    bpm = ob->top_bpm;          /* find the first point that is not
                                 * on the line */
    /*   
     * when we are doing intersections, we want to use the shifted
     * x,y values because the boundary may be for another radar
     */   
    for(bpmx=NULL; bpm; bpm = bpm->next) {
        bpm->which_side = xse_ccw((double)bpm->_x, (double)bpm->_y, xx, yy); 
        if(bpm->which_side) {
            bpmx = bpm; 
            break;
        }    
    }    
    if(!bpmx)
          return(0);            /* no intersections! */
    mm = ob->num_points;
    /*   
     * since "->last" links all points we loop through all the
     * points using the last pointer
     */   
    for(; mm--; bpm = bpm->last) {
        /*   
         * a return of 0 from se_ccw says this point is
         * colinear with the ray. So we do nothing.
         */   
        bpm->last->which_side = xse_ccw
              ((double)bpm->last->_x, (double)bpm->last->_y, xx, yy); 
        if(bpm->last->which_side) {
            if(bpm->last->which_side != bpmx->which_side) {
                /*   
                 * we may have an intersection between "bpm"
                 * and "bpm->last". See if it's real.
                 */   
                xse_set_intxn(xx, yy, slope, bpm, ob); 
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

void BoundaryPointMap::se_ts_find_intxns(double radar_alt, double d_max_range,
                                         OneBoundary *ob, double d_time, double d_pointing,
                                         int automatic, int down, double d_ctr)
{
  int mm;
  BoundaryPointManagement *bpm,  *bpma, *bpmb;
  double d, ta, tb, zz;



  ob->num_intxns = 0;
  ob->first_intxn = NULL;
  mm = ob->num_points;
  bpm = ob->top_bpm;
  d_time += .005;
  d = RADIANS(CART_ANGLE(d_pointing));
  // we bump the time by 5 milliseconds so a ray and a vertex are not coincident in time
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
      // possible intxn 
      zz = ((d_time - ta)/(tb - ta)) * (bpmb->pisp->z - bpma->pisp->z)
        + bpma->pisp->z;
      if((down && zz < radar_alt) || (!down && zz > radar_alt)) {
        // intxn! 
            bpm->rx = down
              ? KM_TO_M(radar_alt -zz) : KM_TO_M(zz - radar_alt);
            ob->num_intxns++;
            bpm->next_intxn = NULL;
            se_merge_intxn(bpm, ob);
      }
    }
  }
  ob->radar_inside_boundary = ob->num_intxns & 1;
  // i.e. an odd number of intxns implies radar inside

  if(ob->bh->force_inside_outside) {
    if(ob->bh->force_inside_outside == BND_FIX_RADAR_INSIDE) {
      ob->radar_inside_boundary = true;
    }
    if(ob->bh->force_inside_outside == BND_FIX_RADAR_OUTSIDE) {
      ob->radar_inside_boundary = false;
    }
  }
}

/* c------------------------------------------------------------------------ */
//
// function used for side effect only
void BoundaryPointMap::se_merge_intxn(BoundaryPointManagement *bpm, OneBoundary *ob)
{
  BoundaryPointManagement *bpmx;

  if(!(bpmx = ob->first_intxn)) { // first intersection
    ob->first_intxn = bpm->last_intxn = bpm;
    return; // (YES);
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
  if(!bpmx) {                 /* furthest out...tack it onto the end */
    ob->first_intxn->last_intxn->next_intxn = bpm;
    bpm->last_intxn = ob->first_intxn->last_intxn;
    ob->first_intxn->last_intxn = bpm;
  }
  return; // (YES);
}

/*c------------------------------------------------------------------------ */


int BoundaryPointMap::loop_ll2xy_v3(double *plat, double *plon, double *palt,
                                    double *x, double *y, double *z, double olat,
                                    double olon, double oalt, double  R_earth, int num_pts)
//  double *plat, *plon, *palt, olat, olon, oalt, R_earth;
//double *x, *y, *z;
//int num_pts;
{
  /* calculate (x,y,z) of (plat,plon) relative to (olat,olon) */
  /* all dimensions in km. */

  /* transform to earth coordinates and then to lat/lon/alt */

  /* These calculations are from the book                                                
   * "Aerospace Coordinate Systems and Transformations"                                  
   * by G. Minkler/J. Minkler                                                            
   * these are the ECEF/ENU point transformations                                        
   */

  int nn = num_pts;
  double delta_o, lambda_o, R_p, R_p_pr, delta_p, lambda_p;
  //double R, rr, abscissa, dlat,
  // double dlon;
  double xe, ye, ze, sinLambda, cosLambda, sinDelta, cosDelta;
  double h, a, b, c;


  h = R_earth + oalt;
  delta_o = RADIANS( olat );  /* read delta sub oh */
  lambda_o = RADIANS( olon );

  sinLambda = sin( lambda_o );
  cosLambda = cos( lambda_o );
  sinDelta = sin( delta_o );
  cosDelta = cos( delta_o );
  /*                                                                                     
    printf( "\n" );                                                                        
  */

  for(; nn--; plat++, plon++, palt++, x++, y++, z++ ) {

    R_p = R_earth + (*palt);
    delta_p = RADIANS( *plat );
    lambda_p = RADIANS( *plon );
    R_p_pr = R_p * cos( delta_p );

    xe = R_p * sin( delta_p );
    ye = -R_p_pr * sin( lambda_p );
    ze = R_p_pr * cos( lambda_p );

    /* transform to ENU coordinates */

    a = -h * sinDelta + xe;
    b =  h * cosDelta * sinLambda + ye;
    c = -h * cosDelta * cosLambda + ze;

    *x = -cosLambda * b  -sinLambda * c;
       *y = cosDelta * a  +  sinLambda * sinDelta * b
         -cosLambda * sinDelta * c;
       *z = sinDelta * a  -sinLambda * cosDelta * b
         +cosLambda * cosDelta * c;
       /*                                                                                  
       printf( "%f %f %f      %f %f %f\n", *plat, *plon, *palt, *x, *y, *z );              
       */
  }
  return num_pts;
}

/* c------------------------------------------------------------------------ */


double BoundaryPointMap::dd_earthr(double lat)
{
  static double *earth_r=NULL;

  double major=6378388;       /* radius in meters */
  double minor=6356911.946;
  double tt;
  double d, theta=0, x, y;
  int ii, nn;

  if(!earth_r) {
    earth_r = (double *)malloc(20*sizeof(double));

    for(ii=0; theta < 90.; ii++, theta += 5.) {
      /* create an entry every 5 degrees                                             
       */
      tt = tan(RADIANS(theta));
      d = sqrt(1.+SQ(tt*major/minor));
      x = major/d;
      y = x*tt;
      *(earth_r +ii) = sqrt(SQ(x) + SQ(y));
    }
  }
  nn = fabs(lat*.2);
  d = nn < 18 ? *(earth_r +nn) : minor;
  d *= .001;                  /* km.! */
  return(d);
}


/* c------------------------------------------------------------------------ */

// out params:
//    p1->x,y,z
//
void BoundaryPointMap::dd_latlon_relative(PointInSpace *p0, PointInSpace *p1)
{
    /* routine to calculate (x,y,z) for p1 so as to line up                                                   
     * with (0,0,0) for p0                                                                                    
     */
  //double del_lat;
 //del_lon,
 //double lat=RADIANS(p1->latitude);
    double xx, yy, zz, R_earth;

    R_earth = dd_earthr(p1->latitude);
    loop_ll2xy_v3( &p0->latitude, &p0->longitude, &p0->altitude
                   , &xx, &yy, &zz
                   , p1->latitude, p1->longitude, p1->altitude
                   , R_earth, 1 );
    p1->x = (float)xx;
    p1->y = (float)yy;
    p1->z = (float)zz;
}


/*
// out params:
//    q_x, q_y, q_zz
//
void BoundaryPointMap::dd_latlon_relative(double p0_tilt,
					  double p0_latitude,
					  double p0_longitude,
					  double p0_altitude,
					  double p1_x,
					  double p1_y,
					  double p1_z,
					  double p1_latitude,
					  double p1_longitude,
					  double p1_altitude,
					  float *q_x,
					  float *q_y,
					  float *q_z)
{
  // routine to calculate (x,y,z) for p1 so as to line up                                                   
  // with (0,0,0) for p0                                                                                    
  //
  //double del_lat;
 //del_lon,
 //double lat=RADIANS(p1->latitude);
    double xx, yy, zz, R_earth;

    R_earth = dd_earthr(p1_latitude);
    // TODO: note! the p0 lat, long, altitude are changed?
    loop_ll2xy_v3( &p0_latitude, &p0_longitude, &p0_altitude
                   , &xx, &yy, &zz
                   , p1_latitude, p1_longitude, p1_altitude
                   , R_earth, 1 );
    q_x = (float)xx;
    q_y = (float)yy;
    q_z = (float)zz;
}
*/

/* c------------------------------------------------------------------------ */

//
// this assigns boundary true/false to each data point
// i.e. to each gate of a ray?
//
//void dd_edd(time_series, automatic, down, d_ctr)
//  int time_series, automatic, down;
//  double d_ctr;
/*
short *BoundaryPointMap::dd_edd(commands, data, boundaryList) 
//  int time_series, 
//  int automatic,
//  int down,
//  double d_ctr,
//  BoundaryStuff *sebs //  *return_se_bnd_struct();
) {

   int ii, jj, mm, nn, nc,  mark, nx, g1, g2, num_segments=0, bflag;
   int not_aligned, shift_bnd, old_usi_swp_count=-1;
   int new_bnd, size, airborne, stopped = NO;
   BoundaryPointManagement *bpm;
   long time_now();
   static int count=0;
   static char *cptrs[32], *deep6=0;
   char *aa, *bbuf, *return_dd_open_info(), *getenv();
   float r1, r2, rr, ff;
   static float anga=58., angb=63., angc=118., angd=123.;
   double range1, range2, distanceToCellNInMeters, dalt;
   double d, rota, rotl;
   unsigned short *bnd;
   
   struct one_boundary *ob, *se_return_current_bnd();
   struct solo_str_mgmt *ssm;
   struct dd_input_sweepfiles_v3 *dis, *dd_return_sweepfile_structs_v3();
   //   BoundaryStuff *sebs, *return_se_bnd_struct();
   
   //struct unique_sweepfile_info_v3 *usi;
   //struct dd_general_info *dgi, *dd_window_dgi();
   struct cell_d *celv;   // this is the data
   static PointInSpace *radar=NULL;
   //PointInSpace *solo_malloc_pisp();
   //struct solo_edit_stuff *seds, *return_sed_stuff();
   
   if(!radar) radar = new PointInSpace(); // solo_malloc_pisp();
   // sebs = return_se_bnd_struct();  // boundary_stuff
   // seds = return_sed_stuff();
   // dis = dd_return_sweepfile_structs_v3();
   // dis->editing = YES; 
   usi = dis->usi_top;
   dd_output_flag(YES);
   dgi = dd_window_dgi(usi->radar_num, "UNK");
   celv = dgi->dds->celvc;
   dgi->compression_scheme = HRD_COMPRESSION;
   dgi->disk_output = YES; 
   // slash_path(dgi->directory_name, usi->directory);
   seds->process_ray_count = seds->volume_count = seds->sweep_count = 0; 
   seds->setup_input_ndx = 0; 
   seds->modified = seds->finish_up = NO;
   ob = sebs->first_boundary;
   for(nn=0; ob; ob = ob->next) {
      if(ob->num_points > 2) 
        nn += ob->num_points;
   }
   bool boundary_exists = nn ? true : false;
   
   for(;;) {                    // for each ray 
          
      count++;

      if(ddswp_next_ray_v3(usi) == END_OF_TIME_SPAN) {
         break;
      }    
      if(!seds->process_ray_count) { // first time through
                                     // tack on the seds cmds 
         se_tack_on_cmds(&dgi->seds, &dgi->sizeof_seds);
     
         if(boundary_exists) {
            radar->latitude = dd_latitude(dgi);
            radar->longitude = dd_longitude(dgi);
            radar->altitude = dd_altitude(dgi);
            radar->earth_radius = dd_earthr(radar->latitude);
            radar->tilt = dd_tilt_angle(dgi);
            radar->tilt = dgi->dds->swib->fixed_angle;

            // calculate x,y,z for radar to line up with (0,0,0) of boundary origin
            // calculated values are returned in radar->x,y,z
            dd_latlon_relative(sebs->origin, radar);
             //
             // radar->x is now the x coordinate of the boundary origin
             // relative to the radar
             // check to see if the boundary origin and the radar origin
             // are within 100m of each other
             //
            not_aligned = (SQ(radar->x) + SQ(radar->y)) > .1;
            airborne = dgi->dds->radd->scan_mode == RHI ||
              !(dgi->dds->radd->radar_type == GROUND ||
                dgi->dds->radd->radar_type == SHIP);

            for(ob = sebs->first_boundary; ob ; ob = ob->next) {
               if(ob->num_points > 2) {
                  bpm = ob->top_bpm;
                  mm = ob->num_points;

                  for(; mm--; bpm = bpm->next) {
                     bpm->_x = bpm->x;
                     bpm->_y = bpm->y;
                  }
                  se_radar_inside_bnd(ob);
// there must be some side effects of this. What are they?
// sets ob->radar_inside_boundary = T/F 
               }
            }
         } // boundary exists? 
} // first time through 

      if(dgi->new_vol) {
         dd_new_vol(dgi);
         seds->volume_count++;
         seds->volume_ray_count = 0;
         dgi->new_sweep = YES;
      }
      if(dgi->new_sweep) {
         seds->sweep_count++;
         seds->sweep_ray_count = 0;
      }
      seds->process_ray_count++;
      seds->sweep_ray_count++;
      seds->volume_ray_count++;
      nc = dgi->clip_gate+1;
      seds->num_segments = 0;

      if(boundary_exists) {
         seds->use_boundary = YES;
         ob = sebs->current_boundary = sebs->first_boundary;
         rota = d = dd_rotation_angle(dgi);
         // distanceToCellNInMeters is the distance from the radar to cell n in meters
         distanceToCellNInMeters = celv->dist_cells[celv->number_cells-1];
         seds->boundary_mask =
           bnd = seds->boundary_mask_array;
         nn = nc +1;
         if(sebs->operate_outside_bnd) {
            bflag = 0;
            for(ii=0; ii < nn; *(bnd+ii++) = 1);
         }
         else {
            bflag = 1;
            memset(bnd, 0, nn * sizeof(*seds->boundary_mask));
         }
         if(dgi->new_sweep) {
            radar->latitude = dd_latitude(dgi);
            radar->longitude = dd_longitude(dgi);
            radar->altitude = dd_altitude(dgi);
            radar->earth_radius = dd_earthr(radar->latitude);

            radar->tilt = dd_tilt_angle(dgi);
            radar->tilt = dgi->dds->swib->fixed_angle;
            radar->elevation = dd_elevation_angle(dgi);
            radar->azimuth = dd_azimuth_angle(dgi);
            radar->heading = dd_heading(dgi);

            if(airborne) {
               shift_bnd = NO;
            }
            else if(not_aligned) {
               shift_bnd = YES;
            }
            else if(fabs(radar->tilt - sebs->origin->tilt) > .2) {
               // boundary and radar origin are the same but
               // not the same tilt
               //
               shift_bnd = YES;
            }
            else {
               shift_bnd = NO;
            }
         }
         // for each boundary, set up the mask
          
         for(ob = sebs->first_boundary; ob ; ob = ob->next) {
            if(ob->num_points < 3)
              continue;

            if(dgi->new_sweep) {
               if(shift_bnd && !time_series) {
                  // shift the boundary's points to be relative to current radar represented by "usi"
                  se_shift_bnd(ob, sebs_origin, radar
                               , radd_scan_mode
                               , tilt_angle);
               }
               if(time_series || (not_aligned && !airborne)) {
                  // see if radar inside this boundary
                  //
                  se_radar_inside_bnd(ob);
// there must be some side effects of this. What are they?
// sets ob->radar_inside_boundary = T/F 
               }
            }
            if(time_series) {
               se_ts_find_intxns(dd_altitude(dgi), distanceToCellNInMeters, ob
                                 , dgi->time, ts_pointing_angle(dgi)
                                 , automatic, down, d_ctr);
            }
            else {
               xse_find_intxns(d, distanceToCellNInMeters, ob);
            }
            xse_num_segments(ob);
            seds->num_segments += ob->num_segments;

            for(ii=0; ii < ob->num_segments; ii++) {
               se_nab_segment(ii, &range1, &range2, ob);
               if(range2 <= 0)
                 continue;
               r1 = range1; r2 = range2;
               g1 = dd_cell_num(dgi->dds, 0, r1);
               g2 = dd_cell_num(dgi->dds, 0, r2) +1;

               for(; g1 < g2; g1++) { // set boundary flags 
                  *(bnd + g1) = bflag;
               }
            } // end segments loop 

         } // end boundary for loop 

      } // boundary exists 

      else {                    // no boundary
         seds->use_boundary = NO;
         seds->boundary_mask = seds->all_ones_array;
      }
   } // end of loop through data 


   if(boundary_exists) {  // pack up the current boundary 
      size = se_sizeof_bnd_set();
      bbuf = (char *)malloc(size);
      memset(bbuf, 0, size);
      se_pack_bnd_set(bbuf);

      if(seds->last_pbs && size == seds->last_pbs->sizeof_set) {
         // see if this boundary is different from the last boundary
          
         new_bnd = se_compare_bnds(seds->last_pbs->at, bbuf, size);
      }
      else {
         new_bnd = YES;
      }

      if(new_bnd) {             // put this boundary in the queue 
         if(seds->num_prev_bnd_sets < 7) {
            // grow the circular queue till it reaches 7 boundaries
             
            seds->num_prev_bnd_sets++;
            seds->pbs = (struct prev_bnd_sets *)
              malloc(sizeof(struct prev_bnd_sets));
            memset(seds->pbs, 0, sizeof(struct prev_bnd_sets));
            if(!seds->last_pbs) {
               seds->pbs->last = seds->pbs->next = seds->pbs;
            }
            else {
               seds->pbs->last = seds->last_pbs;
               seds->pbs->next = seds->last_pbs->next;
               seds->last_pbs->next->last = seds->pbs;
               seds->last_pbs->next = seds->pbs;
            }
         }
         else {
            seds->pbs = seds->last_pbs->next;
         }
         seds->last_pbs = seds->pbs;
         if(seds->pbs->at) free(seds->pbs->at);

         seds->pbs->at = bbuf;
         seds->pbs->sizeof_set = size;
      }
      if(!getenv("SOLO_DONT_CLEAR_BOUNDARIES"))
        se_clr_all_bnds();

      // we should now be ready to draw the next boundary or to
      // retreive  the last boundary we just put in the queue
      //

   } // end packing up current boundary 

   printf ("Finished!\n");
   return bnd;
}
*/

/* c------------------------------------------------------------------------ */

// Q: what is the difference between dd_edd ...for each  and se_process_data?
// dd_edd          processes the "for-each" commands
// se_process_data processes the "one-time" commands
/*
void dd_edd_process_for_each_cmd(
  int time_series, 
  int automatic,
  int down,
  double d_ctr,
  radar_type,
  scan_mode,
  PointInSpace radar, // radar is used to align origin of radar & boundary
  float *rays;
  BoundaryStuff *sebs)
{
   int ii, jj, mm, nn, nc,  mark, nx, g1, g2, num_segments=0, bflag;
   int not_aligned, shift_bnd, old_usi_swp_count=-1;
   int new_bnd, size, airborne, stopped = NO;
   BoundaryPointManagement *bpm;
   long time_now();
   //static int count=0;
   static char *cptrs[32], *deep6=0;
   char *aa, *bbuf, *return_dd_open_info(), *getenv();
   float r1, r2, rr, ff;
   static float anga=58., angb=63., angc=118., angd=123.;
   double range1, range2, distanceToCellNInMeters, dalt;
   double d, rota, rotl;
   unsigned short *bnd;
   
   OneBoundary *ob, *se_return_current_bnd();
   struct solo_str_mgmt *ssm;
   //struct dd_input_sweepfiles_v3 *dis, *dd_return_sweepfile_structs_v3();
   //;  *return_se_bnd_struct();
   
   //struct unique_sweepfile_info_v3 *usi;
   //struct dd_general_info *dgi, *dd_window_dgi();
   struct cell_d *celv;
   //static PointInSpace *radar=NULL;
   //PointInSpace *solo_malloc_pisp();
   //struct solo_edit_stuff *seds, *return_sed_stuff();
   
   //if(!radar) radar = new PointInSpace(); // solo_malloc_pisp();
   // sebs = return_se_bnd_struct();  // boundary_stuff
   // seds = return_sed_stuff();
   // dis = dd_return_sweepfile_structs_v3();
   // dis->editing = YES; 
   usi = dis->usi_top;
   // dd_output_flag(YES);
   //dgi = dd_window_dgi(usi->radar_num, "UNK");
   celv = dgi->dds->celvc;
   //dgi->compression_scheme = HRD_COMPRESSION;
   //dgi->disk_output = YES; 
   // slash_path(dgi->directory_name, usi->directory);
   seds->process_ray_count = seds->volume_count = seds->sweep_count = 0; 
   seds->setup_input_ndx = 0; 
   seds->modified = seds->finish_up = NO;
   ob = sebs->first_boundary;
   for(nn=0; ob; ob = ob->next) {
      if(ob->num_points > 2) 
        nn += ob->num_points;
   }
   bool boundary_exists = nn ? true : false;

   //if(!seds->process_ray_count) { 
   // first time through
   // tack on the seds cmds 
  //se_tack_on_cmds(&dgi->seds, &dgi->sizeof_seds);
     
  // if there are boundaries, translate the the origins to align
  // boundary with radar; also determine if radar is inside boundary
     if(boundary_exists) {

       // align the origins of the radar data to the boundary
       // calculate x,y,z for radar to line up with (0,0,0) of boundary origin
       // calculated values are returned in radar->x,y,z
       dd_latlon_relative(sebs->origin, radar);
       //
       // radar->x is now the x coordinate of the boundary origin
       // relative to the radar
       // check to see if the boundary origin and the radar origin
       // are within 100m of each other
       //
       not_aligned = (SQ(radar->x) + SQ(radar->y)) > .1;
       airborne = scan_mode == RHI ||
         !(radar_type == GROUND ||
           radar_type == SHIP);
       
       // for each boundary ...
       for(ob = sebs->first_boundary; ob ; ob = ob->next) {
         if(ob->num_points > 2) {
           bpm = ob->top_bpm;
           mm = ob->num_points;

           // Q: What is the difference between _x and x
           for(; mm--; bpm = bpm->next) {
             bpm->_x = bpm->x;
             bpm->_y = bpm->y;
           }
           se_radar_inside_bnd(ob);
           // there must be some side effects of this. What are they?
           // sets ob->radar_inside_boundary = T/F 
         }
       }
       } // boundary exists? 
     //} // first time through 

     /// for each ray 
   bool done = false;
   //----
     // TODO: this is the loop advance
     // get_next_ray();  // the rays may come in as a stream
     //if(ddswp_next_ray_v3(usi) == END_OF_TIME_SPAN) {
     //  done = true;
     //}    
   //---------
   
     while (!(seds->punt) && !done && get_next_ray() != END_OF_TIME_SPAN) {    

     nc = dgi->clip_gate+1;
     seds->num_segments = 0;

     if(boundary_exists) {
       bnd = get_boundary_mask();
     }
     else {
       //seds->use_boundary = NO;
       //seds->boundary_mask = seds->all_ones_array;
       bnd = all_ones_array;
     }
     ssm = seds->fer_cmds;
     nn = seds->num_fer_cmds;

     //
     // loop through the for-each-ray operations
     //
     se_perform_cmds (seds->first_fer_cmd, seds->num_fer_cmds,
                      data, bnd);

     if(seds->modified) {
       // Q: Are results of commands written to a file?
       dd_dump_ray(dgi);

       if (dgi->sweep_fid < 0) { // unable to open the new sweepfile
         seds->punt = YES;
         break;
         // TODO: report error; throw exception
       }

       if(ddswp_last_ray(usi)) { // end of current sweep 
         dd_flush(usi->radar_num);
       }
       if(usi->ray_num <= 1) {
         sp_sweep_file_check(usi->directory, usi->filename);
       }
       if(dgi->new_sweep) {
         g_string_sprintfa (gs_complaints,"%s", return_dd_open_info());
       }
     }
     dgi->new_sweep = dgi->new_vol = NO;
     if(usi->swp_count > usi->num_swps) {
       done = true;
     }
     } // end of loop through data 

   dis->editing = NO;

   //if(seds->punt)
   //  return;  // throw exception?

   //seds->finish_up = YES;
   ssm = seds->fer_cmds;
   nn = seds->num_fer_cmds;
   //
   // make a final pass through all operations in case they
   // need to finish up
   ///
   se_perform_cmds (seds->first_fer_cmd, seds->num_fer_cmds, data);
   //se_free_raqs();              // free running average queues
   // if any where set up 
   // TODO: signal GUI update?
   //   if(seds->modified) {
   //      seds->time_modified = time_now();
   //      ddir_rescan_urgent(seds->se_frame);
   //   }

   //if(boundary_exists) { 
   //  pack_up_current_boundary(); 

// we should now be ready to draw the next boundary or to
// retreive  the last boundary we just put in the queue
      

} // end packing up current boundary 

   printf ("Finished!\n");
   return;
}
*/

/* c------------------------------------------------------------------------ */
// out params:
// r0, r1, the start and stop range of the segment
// ob 
void BoundaryPointMap::se_nab_segment(int num, double *r0, double *r1,
                                      OneBoundary *ob)
{
  // this routine returns the start and stop range of the
  // requested segment
  //   
  //int ii, nn, mark;
  //  struct bnd_point_mgmt *bpm;

  if(num < 0 || num >= ob->num_segments) {
    *r0 = -999999.;
    *r1 = -999998.;
    return;
  }    
  // what are ob->r0 & ob->r1 & next_intxn->rx?
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
  else {                      // first segment 
    *r0 = ob->r0;
    *r1 = ob->r1;
  }    
}

/* c------------------------------------------------------------------------ */

// TODO: what to do about this function? We aren't using a "uniform spacing
// lookup table"
// let's assume the gates are uniformly spaced
int BoundaryPointMap::dd_cell_num(int nGates, float gateSize,
                                  float distanceToFirstGate,  float range)
//int parameter_num;
//float range;
{
  // find the cell corresponding to this range
  //
  int ii;

  ii = ((range-distanceToFirstGate)/gateSize) + .5;
  if(ii < 0)
    ii = 0;
  else if(ii >= nGates)
    ii = nGates-1;

  return(ii);
}

/* c------------------------------------------------------------------------ */

int BoundaryPointMap::xse_num_segments(OneBoundary *ob)
{
  // calculate the number of segments and set up                                         
  // the first segment                                                                   
  //
  //int ii, nn,
  int  nx; // , mark;

  nx = ob->num_intxns;

  if(ob->radar_inside_boundary) {
    if (nx == 0) { // if(!nx) {
      // the end of the ray is inside the boundary 
      ob->num_segments = 1;
      ob->r0 = 0;
      ob->r1 = 1.e9;
      // cout << "xse_num_segments: return point 1" << endl;
      return(1);
    }
    ob->r0 = 0;
    ob->r1 = ob->first_intxn->rx;
    ob->next_segment = ob->first_intxn->next_intxn;

    if(nx & 1) {            // no funny stuff
      ob->num_segments = (nx+1)/2;
    }
    else {
      //                                                       
      // even number of intersections                                                
      // assume the boundary is past the end of the ray                              
      //
      ob->num_segments = nx/2 +1;
    }
      //cout << "xse_num_segments: return point 2" << endl;

    return(ob->num_segments);
  }
  // radar is outside the boundary                                                       
   
  if(!nx) {
    ob->num_segments = 0;
      //cout << "xse_num_segments: return point 3" << endl;

    return(ob->num_segments);
  }
  ob->r0 = ob->first_intxn->rx;

  if(nx & 1) {                // the boundary is past the end of the ray 
    if(nx == 1) {
      ob->num_segments = 1;
      ob->r1 = 1.e9;
      //cout << "xse_num_segments: return point 4" << endl;

      return(1);
    }
    ob->num_segments = (nx+1)/2;
  }
  else {
    ob->num_segments = nx/2;
  }
  ob->r1 = ob->first_intxn->next_intxn->rx;
  ob->next_segment = ob->first_intxn->next_intxn->next_intxn;
      //cout << "xse_num_segments: return point 5" << endl;

  return(ob->num_segments);
}

/*
int BoundaryPointMap::xse_num_segments_original(OneBoundary *ob)
{
  // calculate the number of segments and set up                                         
  // the first segment                                                                   
  //
  //int ii, nn,
  int  nx; // , mark;

  nx = ob->num_intxns;

  if(ob->radar_inside_boundary) {
    if(!nx) {
      // the end of the ray is inside the boundary 
      ob->num_segments = 1;
      ob->r0 = 0;
      ob->r1 = 1.e9;
      return(1);
    }
    ob->r0 = 0;
    ob->r1 = ob->first_intxn->rx;
    ob->next_segment = ob->first_intxn->next_intxn;

    if(nx & 1) {            // no funny stuff
      ob->num_segments = (nx+1)/2;
    }
    else {
      //                                                       
      // even number of intersections                                                
      // assume the boundary is past the end of the ray                              
      //
      ob->num_segments = nx/2 +1;
    }
    return(ob->num_segments);
  }
  // radar is outside the boundary                                                       
   
  if(!nx) {
    ob->num_segments = 0;
    return(ob->num_segments);
  }
  ob->r0 = ob->first_intxn->rx;

  if(nx & 1) {                // the boundary is past the end of the ray 
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
*/
/* c------------------------------------------------------------------------ */

void BoundaryPointMap::se_shift_bnd( // ob, boundary_radar, current_radar, scan_mode, current_tilt)
  OneBoundary *ob,
  PointInSpace *boundary_radar, 
  PointInSpace *current_radar,
  int scan_mode,
  double current_tilt)
{
  /* shift this boundary's points so as to be relative to                                
   * the current radar represented by "usi"                                              
   */
  int mm;
  BoundaryPointManagement *bpm;
  //PointInSpace radar;
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
    //for(; mm--; bpm = bpm->last) {
    //}
    // TODO: log warning ... not implemented.
  }

  else {                      /* ground based */
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

/*
// returns boundary mask
// NOTE: This is a Frankenstein method ... it has been pieced together
//       from different sections of the original Soloii code. 
//
short *BoundaryPointMap::get_boundary_mask(
        OneBoundary *boundaryList,
        // bool new_sweep,  // assume new_sweep
        //        bool operate_outside_bnd,
        //bool shift_bnd,  // always shift
        PointInSpace *radar_origin,
        PointInSpace *boundary_origin,
        int nGates,
        float gateSize,
        float distanceToCellNInMeters,
        float azimuth,    // TODO: are azimuth & rotation_angle the same thing? YES
        int radar_scan_mode,
        int radar_type,
        float tilt_angle, 
        float rotation_angle) {

  OneBoundary *ob;
  //rota = d = dd_rotation_angle(dgi);
  short bflag;

  short *bnd = new short[nGates];
  int nn = nGates;
    bflag = 1;
    memset(bnd, 0, nn * sizeof(short));
  // for each boundary, add to the mask; the mask is the intersection
  // of all boundaries?
  //
  for(ob = boundaryList; ob ; ob = ob->next) {
    if(ob->num_points < 3)
      continue;

        // shift the boundary's points to be relative to current radar represented by "usi"
        se_shift_bnd(ob, boundary_origin, radar_origin,
                     radar_scan_mode,
                      tilt_angle);

        //--------
        //  NOTE: this should be filled by the calling function
        //radar->latitude = dd_latitude(dgi);
        //radar->longitude = dd_longitude(dgi);
        //radar->altitude = dd_altitude(dgi);
        //radar->earth_radius = dd_earthr(radar->latitude);
        //radar->tilt = dd_tilt_angle(dgi);
        //radar->tilt = dgi->dds->swib->fixed_angle;
       
        //dd_latlon_relative(boundary_origin, radar_origin);
        // end NOTE:
        //

        //                                                                             
         // radar->x is now the x coordinate of the boundary origin                     
         // relative to the radar                                                       
         // check to see if the boundary origin and the radar origin                    
         // are within 100m of each other                                               
         ///
        bool not_aligned = (SQ(radar_origin->x) + SQ(radar_origin->y)) > .1;
        bool airborne = radar_scan_mode == RHI ||
              !(radar_type == GROUND ||
                radar_type == SHIP);

            //---------------

      if(not_aligned && !airborne) {
        // see if radar is inside this boundary
        se_radar_inside_bnd(ob);
        // there must be some side effects of this. What are they?
        // sets ob->radar_inside_boundary = T/F 
      }
      double range = distanceToCellNInMeters + nGates*gateSize;
    xse_find_intxns(azimuth, range, ob);
    xse_num_segments(ob);
    
    double range1;
    double range2;

    for(int ii=0; ii < ob->num_segments; ii++) {
      se_nab_segment(ii, &range1, &range2, ob);
      if(range2 <= 0)
        continue;
      double r1 = range1; 
      double r2 = range2;
      int g1 = dd_cell_num(nGates, gateSize, distanceToCellNInMeters, r1);
      int g2 = dd_cell_num(nGates, gateSize, distanceToCellNInMeters, r2) +1;

      for(; g1 < g2; g1++) { // set boundary flags 
        *(bnd + g1) = bflag;
      }
    } // end segments loop //

  } // end boundary for loop //
  return bnd;
}
*/


/* c------------------------------------------------------------------------ */

// pass in allocated boundary mask array
// NOTE: This is a Frankenstein method ... it has been pieced together
//       from different sections of the original Soloii code. 
//
void BoundaryPointMap::get_boundary_mask(
        OneBoundary *boundaryList,
        // bool new_sweep,  // assume new_sweep
        //        bool operate_outside_bnd,
        //bool shift_bnd,  // always shift
        PointInSpace *radar_origin,
        PointInSpace *boundary_origin,
        int nGates,
        float gateSize,
        float distanceToCellNInMeters,
        float azimuth,    // TODO: are azimuth & rotation_angle the same thing? YES
        int radar_scan_mode,
        int radar_type,
        float tilt_angle, 
        float rotation_angle,
	bool *boundary_mask) {

  OneBoundary *ob;
  //rota = d = dd_rotation_angle(dgi);
  bool bflag;

  //  short *bnd = new short[nGates];
  bool *bnd = boundary_mask;

  int nn = nGates;
  bflag = true;
  memset(bnd, 0, nn * sizeof(bool));
  // for each boundary, add to the mask; the mask is the intersection
  // of all boundaries?
  //
  for(ob = boundaryList; ob ; ob = ob->next) {
    if(ob->num_points < 3)
      continue;

    // print the boundary for debug 
    // ob->print();
        // shift the boundary's points to be relative to current radar represented by "usi"
        se_shift_bnd(ob, boundary_origin, radar_origin,
                     radar_scan_mode,
                      tilt_angle);

        //--------
        /*  NOTE: this should be filled by the calling function
        radar->latitude = dd_latitude(dgi);
        radar->longitude = dd_longitude(dgi);
        radar->altitude = dd_altitude(dgi);
        radar->earth_radius = dd_earthr(radar->latitude);
        radar->tilt = dd_tilt_angle(dgi);
        radar->tilt = dgi->dds->swib->fixed_angle;
       
        dd_latlon_relative(boundary_origin, radar_origin);
        */

        /*                                                                             
         * radar->x is now the x coordinate of the boundary origin                     
         * relative to the radar                                                       
         * check to see if the boundary origin and the radar origin                    
         * are within 100m of each other                                               
         */
        bool not_aligned = (SQ(radar_origin->x) + SQ(radar_origin->y)) > .1;
        bool airborne = radar_scan_mode == RHI ||
              !(radar_type == GROUND ||
                radar_type == SHIP);

            //---------------

	//if(not_aligned && !airborne) {
        // see if radar is inside this boundary
        se_radar_inside_bnd(ob);
        // there must be some side effects of this. What are they?
        // sets ob->radar_inside_boundary = T/F 
	//}
      // TODO: must set ob->radar_inside_boundary <==== HERE
      double range = distanceToCellNInMeters + nGates*gateSize;
    xse_find_intxns(azimuth, range, ob);
    xse_num_segments(ob);
    
    cout << "azimuth=" << azimuth << " r0 = " << ob->r0 << " r1 = " << ob->r1 << endl;
    double range1;
    double range2;

    for(int ii=0; ii < ob->num_segments; ii++) {
      se_nab_segment(ii, &range1, &range2, ob);
      if(range2 <= 0)
        continue;
      double r1 = range1; 
      double r2 = range2;
      int g1 = dd_cell_num(nGates, gateSize, distanceToCellNInMeters, r1);
      int g2 = dd_cell_num(nGates, gateSize, distanceToCellNInMeters, r2) +1;

      for(; g1 < g2; g1++) { /* set boundary flags */
        *(bnd + g1) = bflag;
      }
    } /* end segments loop */

  } /* end boundary for loop */

}

/*
// returns boundary mask
short *BoundaryPointMap::get_boundary_mask_time_series(
        OneBoundary *boundaryList,
        bool new_sweep,
        bool operate_outside_bnd,
        bool shift_bnd,
        PointInSpace *radar_origin,
        PointInSpace *boundary_origin,
        int nGates,
        float gateSize,
        float distanceToCellNInMeters,
        float azimuth) {

  OneBoundary *ob;
  //rota = d = dd_rotation_angle(dgi);
  short bflag;

  short *bnd = new short[nGates];
  int nn = nGates;
  if(operate_outside_bnd) {
    bflag = 0;
    for(int ii=0; ii < nn; *(bnd+ii++) = 1);
  }
  else {
    bflag = 1;
    memset(bnd, 0, nn * sizeof(short));
  }
  // for each boundary, add to the mask; the mask is the intersection
  // of all boundaries?
  //
  for(ob = boundaryList; ob ; ob = ob->next) {
    if(ob->num_points < 3)
      continue;

    if(new_sweep) {
      if(not_aligned && !airborne) {
        // see if radar is inside this boundary
        se_radar_inside_bnd(ob);
        // there must be some side effects of this. What are they?
        // sets ob->radar_inside_boundary = T/F 
      }
    }
    se_ts_find_intxns(altitude, distanceToCellNInMeters, ob,
                        time, ts_pointing_angle,
                        automatic, down, d_ctr);
    xse_num_segments(ob);
    seds->num_segments += ob->num_segments;

    for(ii=0; ii < ob->num_segments; ii++) {
      se_nab_segment(ii, &range1, &range2, ob);
      if(range2 <= 0)
        continue;
      r1 = range1; r2 = range2;
      g1 = dd_cell_num(nGates, gateSize, distanceToCellNInMeters, r1);
      g2 = dd_cell_num(nGates, gateSize, distanceToCellNInMeters, r2) +1;

      for(; g1 < g2; g1++) { // set boundary flags 
        *(bnd + g1) = bflag;
      }
} // end segments loop 

} // end boundary for loop 
  return bnd;
}
*/

/* TODO: probably move this to controller or view?
void BoundaryPointMap::pack_up_current_boundary() {

      size = se_sizeof_bnd_set();
      bbuf = (char *)malloc(size);
      memset(bbuf, 0, size);
      se_pack_bnd_set(bbuf);

      if(seds->last_pbs && size == seds->last_pbs->sizeof_set) {
         // see if this boundary is different from the last boundary
         new_bnd = se_compare_bnds(seds->last_pbs->at, bbuf, size);
      }
      else {
         new_bnd = YES;
      }

      if(new_bnd) {             // put this boundary in the queue 
         if(seds->num_prev_bnd_sets < 7) {
            // grow the circular queue till it reaches 7 boundaries
             
            seds->num_prev_bnd_sets++;
            seds->pbs = (struct prev_bnd_sets *)
              malloc(sizeof(struct prev_bnd_sets));
            memset(seds->pbs, 0, sizeof(struct prev_bnd_sets));
            if(!seds->last_pbs) {
               seds->pbs->last = seds->pbs->next = seds->pbs;
            }
            else {
               seds->pbs->last = seds->last_pbs;
               seds->pbs->next = seds->last_pbs->next;
               seds->last_pbs->next->last = seds->pbs;
               seds->last_pbs->next = seds->pbs;
            }
         }
         else {
            seds->pbs = seds->last_pbs->next;
         }
         seds->last_pbs = seds->pbs;
         if(seds->pbs->at) free(seds->pbs->at);

         seds->pbs->at = bbuf;
         seds->pbs->sizeof_set = size;
      }
      if(!getenv("SOLO_DONT_CLEAR_BOUNDARIES"))
        se_clr_all_bnds();
}
*/

/* c------------------------------------------------------------------------ */

/*
int BoundaryPointMap::se_perform_cmds (struct ui_cmd_mgmt *the_ucm, int num_cmds)
{
  typedef int (*UI_FUNC)(int, struct ui_command *);

  struct ui_cmd_mgmt *ucm = the_ucm;
  int nn = num_cmds;

  if (num_cmds < 1)
  { return 1; }

  for (; nn--; ucm = ucm->next) {
    (*ucm->cmd_proc)(ucm->num_tokens, ucm->cmds);
  }
  return 1;
}


int main(int argc, char *argv[]) {


// Q: So, where is radar_inside_bnd called? and how is it used?
// See dd_edd(time_series, automatic, down, d_ctr)
// dd_edd goes through data and sets boundary_mask to true or false 
// se_nab_segment(..)  <== returns the start and stop range of the requested segment
//    the start and stop are intersection points

}
*/
