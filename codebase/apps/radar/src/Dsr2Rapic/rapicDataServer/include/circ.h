#ifndef CIRC_H
#define CIRC_H


class circle {
 public:
  float vertsf[360][2]; // float x,y coords of 0-359 degrees
  double vertsd[360][2]; // double x,y coords of 0-359 degrees

  circle();       // initialise table
  void calcVerts();
  void draw(float ofsx, float ofsy, float rng, int angres = 10);
  float* vertf(float ang);
  double* vertd(double ang);
};
    
extern class circle globalCircle;

#endif
