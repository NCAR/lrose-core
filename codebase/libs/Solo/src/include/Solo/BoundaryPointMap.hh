
#include "Solo/BoundaryPointManagement.hh"
#include "Solo/OneBoundary.hh"


class BoundaryPointMap {

public:

  int xse_ccw(double x0, double y0, double x1, double y1);

  void xse_set_intxn(double x, double y, double slope, 
                    BoundaryPointManagement *bpm,
                    OneBoundary *ob);

  void se_radar_inside_bnd(OneBoundary *ob);

  int xse_find_intxns(double angle, double range, OneBoundary *ob);

  void se_ts_find_intxns(double radar_alt, double d_max_range,
                         OneBoundary *ob, double d_time, double d_pointing,
                         int automatic, int down, double d_ctr);

  void se_merge_intxn(BoundaryPointManagement *bpm, 
                     OneBoundary *ob);

  double dd_earthr(double lat);

  int loop_ll2xy_v3(double *plat, double *plon, double *palt,
                    double *x, double *y, double *z, double olat,
                    double olon, double oalt, double  R_earth, int num_pts);

  void dd_latlon_relative(PointInSpace *p0, PointInSpace *p1);

  void se_nab_segment(int num, double *r0, double *r1, 
                      OneBoundary *ob);

  int dd_cell_num(int nGates, float gateSize,
                  float distanceToFirstGate,  float range);

  int xse_num_segments(OneBoundary *ob);

  void se_shift_bnd(
          OneBoundary *ob,
          PointInSpace *boundary_radar,
          PointInSpace *current_radar,
          int scan_mode,
          double current_tilt);

  short *get_boundary_mask(
          OneBoundary *boundaryList,
          PointInSpace *radar_origin,
          PointInSpace *boundary_origin,
          int nGates,
          float gateSize,
          float distanceToCellNInMeters,
          float azimuth,
          int radar_scan_mode,
          int radar_type,
          float tilt_angle,
          float rotation_angle);
  /*

  short *get_boundary_mask_time_series(
          OneBoundary *boundaryList,
          int time_series,
          bool new_sweep,
          bool operate_outside_bnd,
          bool shift_bnd,
          PointInSpace *radar,
          int nGates,
          float gateSize,
          float distanceToCellNInMeters,
          float azimuth);
  */
  //  int se_perform_cmds (struct ui_cmd_mgmt *the_ucm, int num_cmds);

};
