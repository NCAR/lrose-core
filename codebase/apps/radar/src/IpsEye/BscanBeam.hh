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
#ifndef BscanBeam_HH
#define BscanBeam_HH

#include "Beam.hh"

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/// Manage static data for one display beam that can
/// render multiple variables.
/// For the specified solid angle and number
/// of gates:
///
/// - a vector of vertices is created.
/// - nVars vectors, each of length matching the vertices vector,
///   is allocated.
/// 
/// The colors for each variable will be set dynamically
/// by the owner as data need to be rendered. At that time, 
/// a display list is also created (by the beam owner)
/// that uses the display list id. By having display lists
/// created for all variables, the display of beams can be
/// rapidly switched between variables just by executing
/// the correct display list.

class BscanBeam : public Beam

{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor
   *
   */
  
  BscanBeam(const Params &params,
            const RadxRay *ray,
            double instHtKm,     // height of instrument in km
            int n_fields,
            const RadxTime &plot_start_time,
            double plot_width_secs,
            const RadxTime &beam_start_time, 
            const RadxTime &beam_end_time);

  /**
   * @brief Destructor
   */

  virtual ~BscanBeam();

  // reset the plot start time
  
  void resetPlotStartTime(const RadxTime &plot_start_time);

  /**
   * @brief Paint the given field in the given painter.
   */

  virtual void paint(QImage *image,
                     const QTransform &transform,
                     size_t field,
                     bool useHeight = false,
                     bool drawInstHt = false);

  /**
   * @brief Print details of beam object
   */

  virtual void print(ostream &out);

  // get methods

  const RadxTime &getPlotStartTime() const { return _plotStartTime; }
  double getPlotWithSecs() const { return _plotWidthSecs; }
  const RadxTime &getBeamStartTime() const { return _beamStartTime; }
  const RadxTime &getBeamEndTime() const { return _beamEndTime; }

protected:

  /////////////////////
  // Protected types //
  /////////////////////

  typedef struct
  {
    double x;
    double y;
    double width;
    double height;
  } rect_t;
    
  /**
   * @brief The polygons that represent the gate in this beam.
   */
  
  vector<rect_t> _rangeRects;
  vector<rect_t> _heightRects;

  rect_t _instRect;

  ///////////////////////
  // Protected members //
  ///////////////////////

  double _instHtKm;
  RadxTime _plotStartTime;
  double _plotWidthSecs;
  RadxTime _beamStartTime;
  RadxTime _beamEndTime;
  
  void _paintRects(QPainter &painter,
                   const vector<rect_t> &rects,
                   size_t field);

};

#endif
