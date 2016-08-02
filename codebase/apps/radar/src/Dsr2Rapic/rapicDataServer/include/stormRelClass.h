#ifndef STORMRELCLASS_H
#define STORMRELCLASS_H

enum stormRelMode {sr_none, sr_manual, sr_cell, sr_cell_avg, 
		   sr_cur_rel_vel, sr_cur_rel_shear};

enum stormRelViewMode {sr_view_transient, sr_view_fixed};

class stormRelClass
{
 public:
  float dir,   // degrees from North
    speed;     // m/s
  stormRelMode mode;
  stormRelViewMode viewMode;
  stormRelClass()
    {
      dir = 0;
      speed = 0;
      mode = sr_none;
      viewMode = sr_view_transient;
    };
  void set(stormRelClass &stormrel)
    {
      dir = stormrel.dir;
      speed = stormrel.speed;
      mode = stormrel.mode;
      viewMode = stormrel.viewMode;
    };
  void get(stormRelClass &stormrel)
    {
      stormrel.dir = dir;
      stormrel.speed = speed;
      stormrel.mode = mode;
      stormrel.viewMode = viewMode;
    };
  void setVals(stormRelClass &stormrel)
    {
      dir = stormrel.dir;
      speed = stormrel.speed;
    };
  void clear()
    {
      dir = 0;
      speed = 0;
    };
  bool sameAs(stormRelClass &stormrel)
    {
      return
	(dir == stormrel.dir) &&
	(speed == stormrel.speed) &&
	(mode == stormrel.mode) &&
	(viewMode == stormrel.viewMode);
    };
  bool sameValsAs(stormRelClass &stormrel)
    {
      return
	(dir == stormrel.dir) &&
	(speed == stormrel.speed);
    };
  bool isZero()
    {
      return (dir == 0) && (speed == 0);
    };
};

#endif
