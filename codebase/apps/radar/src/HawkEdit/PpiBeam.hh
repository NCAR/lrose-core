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
#ifndef PpiBeam_HH
#define PpiBeam_HH

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

class PpiBeam : public Beam
{

public:

  ////////////////////
  // Public members //
  ////////////////////

  /**
   * @brief The start angel of the beam.
   */

  double startAngle;

  /**
   * @brief The stop angle of the beam.
   */

  double stopAngle;

  /**
   * @brief Left end point store of segment(dynamic beams only).
   */

  double leftEnd;

  /**
   * @brief Right end point store of segment(dynamic beams only).
   */

  double rightEnd;

  /**
   * @brief If beam is hidden (dynamic beams only).
   */

  bool hidden;


  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   *
   * @param[in] params      TDRP parameters
   * @param[in] n_gates     Number of gates in the beam.
   * @param[in] n_fields    Number of fields in the beam.
   * @param[in] start_angle Start angle of the beam.
   * @param[in] stop_angle  Stop angle of the beam.
   */
  
  PpiBeam( // const RadxRay *ray,
          double start_angle, double stop_angle,
          double startRangeKm, double gateSpacingKm,
          size_t nGates);
  
  /**
   * @brief Destructor
   */

  virtual ~PpiBeam();

  /**
   * @brief Paint the given field in the given painter.
   */

  virtual void paint(QPainter *painter, QImage *image,
                     const QTransform &transform,
                     bool useHeight = false,
                     bool drawInstHt = false);

  /**
   * @brief Print details of beam object
   */

  virtual void print(ostream &out);

protected:

  /////////////////////
  // Protected types //
  /////////////////////

  typedef struct
  {
    double x;
    double y;
  } point_t;
    
  typedef struct
  {
    point_t pts[4];
    bool doPaint;
  } polygon_t;
    
  /**
   * @brief The polygons that represent the gate in this beam.
   */
  
  std::vector< polygon_t > _polygons;

};

#endif
