/*

  wdss_rec.h

*/

#ifndef __WDSS_REC_H
#define __WDSS_REC_H

class wdss_rec
{
 public:
  float lat,lng;
  int   cell_id;
  float az;
  float rng;
  char  circ[20];
  float svrh;
  float hailsize;
  float prob_hail;
  float vil;
  float maxz;
  float ht_maxz;
  float ht_base;
  float ht_top;
  float dir;
  float speed;
  float mass;
  float ltng_rate;
  float posltng;
  float pos_ltng_flshrate;
  float neg_ltng_flshrate;
  float pct_posltng;
  float strm_rel_hlcty;
  float cell_vol;
  float core_asp_ratio;
  float 3d_ltngrate;
  float centre_mass_ht;
  float refl_ratio;
  float max_conv;
  bool  valid;
  wdss_rec *next, *prev;
  wdss_rec(char *instr = 0);
  virtual wdss_rec();
  void init(char *instr);
};

#endif









