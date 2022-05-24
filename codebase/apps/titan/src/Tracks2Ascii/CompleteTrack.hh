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
/////////////////////////////////////////////////////////////
// CompleteTrack.h
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 1998
//
///////////////////////////////////////////////////////////////

#ifndef CompleteTrack_H
#define CompleteTrack_H

#include "Entity.hh"
#include <titan/SeedCaseTracks.hh>
using namespace std;

class CompleteTrack : public Entity {
  
public:

  // constructor

  CompleteTrack(const string &prog_name, const Params &params);

  // destructor
  
  ~CompleteTrack();

  // compute

  int comps_init(storm_file_handle_t *s_handle,
		 track_file_handle_t *t_handle);

  int compute(storm_file_handle_t *s_handle,
	      track_file_handle_t *t_handle);

  int comps_done(storm_file_handle_t *s_handle,
		 track_file_handle_t *t_handle);

protected:
  
private:

  ui08 **_runsSwathGrid;
  ui08 **_ellipseSwathGrid;
  ui08 **_ellipseGrid;
  double **_precipGrid;
  
  double _rerv;
  double _totEntries;
  double _nstormsInBox;
  double _sumVolume, _maxVolume;
  double _sumPrecipArea, _maxPrecipArea;
  double _sumProjArea, _maxProj_area;
  double _sumMass, _maxMass;
  double _sumPrecipFlux, _maxPrecipFlux;
  double _sumDbz, _maxDbz;
  double _sumBase, _maxBase;
  double _sumTop, _maxTop;
  double _sumU, _sumV;
  double _sumRange, _minRange, _maxRange;
  double _remDuration;
  titan_grid_t _grid;

  SeedCaseTracks _caseTracks;
  const vector<SeedCaseTracks::CaseTrack> *_cases;

  void compute_verification (const track_file_contingency_data_t &cont,
                             double &pod, double &far, double &csi);

  void update_ellipse_precip(storm_file_handle_t *s_handle,
			     storm_file_params_t *sparams,
			     storm_file_global_props_t *gprops);

  void set_ellipse_grid(double ellipse_x,
			double ellipse_y,
			double major_radius,
			double minor_radius,
			double axis_rotation,
			si32 *start_ix_p,
			si32 *start_iy_p,
			si32 *end_ix_p,
			si32 *end_iy_p,
			ui08 **grid);

  void print_grid(const char *label1,
		  const char *label2,
		  ui08 **ellipse_grid,
		  ui08 **runs_grid);

};

#endif

