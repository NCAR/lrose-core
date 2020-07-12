// trying to figure out the Solo boundary point code

// There must be a list of points, that get translated from display coordinates into
// radar coordinates, then some list of points that are either side the boundary
// or outside the boundary.

  // determine if point is left or right of line 
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
  return(0);                  /* on the line */
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
  return(0);                  /* on the line */
}


int se_radar_inside_bnd(OneBoundary *ob)
{
  /* determine if the radar is inside or outside the boundary                                               
   */
  BoundaryPointManagement *bpm, *bpmx;
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


int main(int argc, char *argv[]) {


 

}
