// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#ifndef BOUNDARYPOINTMAP_H
#define BOUNDARYPOINTMAP_H

#include "Solo/BoundaryPointManagement.hh"
#include "Solo/OneBoundary.hh"


// BoundaryPointMap is just an interface to the Solo boundary
// functions.  Remember, it holds no state!

class BoundaryPointMap {

public:

  void se_delete_bnd_pt(BoundaryPointManagement *bpm,
                        OneBoundary *ob);

  void xse_add_bnd_pt(long x, long y, OneBoundary *ob,
                      bool time_series = false);

  void xse_x_insert(BoundaryPointManagement *bpm,
                    OneBoundary *ob);
  void xse_y_insert(BoundaryPointManagement *bpm,
                    OneBoundary *ob);

  void se_bnd_pt_atts(BoundaryPointManagement *bpm);

  void se_append_bpm(BoundaryPointManagement **top_bpm,
                     BoundaryPointManagement *bpm);


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

  /*
  void dd_latlon_relative(double p0_tilt,
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
			  float *q_z);
  */

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
  /*
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
  */
void get_boundary_mask(
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
	      bool *boundary_mask);



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

#endif
