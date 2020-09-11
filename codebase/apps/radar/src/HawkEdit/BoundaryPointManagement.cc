#include "BoundaryPointManagement.hh"


void BoundaryPointManagement::BoundaryPointAtts() {

  // Remember, we are working on a BoundaryPointManagement object (bpm)
  if(pisp->state & PISP_TIME_SERIES) {
    dt = last->pisp->time - pisp->time;
    dr = last->pisp->range - pisp->range;
    if(dt) slope = dr/dt;
    if(dr) slope_90 = -1./slope;
    len = SQRT(SQ(dt) + SQ(dr));
    t_mid = pisp->time + 0.5 * dt;
    r_mid = pisp->range + 0.5 * dr;
  }
  else {
    dy = last->y - y;
    dx = last->x - x;

    if(dx)
      slope = (double)dy/dx;

    if(dy)
      slope_90 = -1./slope; /* slope of the line                                            
                                       * perpendicular to this line */

    len = sqrt((SQ((double)dx) + SQ((double)dy)));
    x_mid = x + 0.5 * dx;
    y_mid = y + 0.5 * dy;
  }

}
