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
#include "PpiBeamModel.hh"
//#include "PolarManager.hh"
//#include "ScriptEditorView.hh"
//#include "ScriptEditorController.hh"
//#include "SpreadSheetView.hh"
//#include "SpreadSheetController.hh"
//#include "ParameterColorView.hh"
//#include "FieldColorController.hh"
//#include "DisplayFieldModel.hh"
//#include "BoundaryPointEditor.hh"

#include <toolsa/toolsa_macros.h>
#include <toolsa/LogStream.hh>

using namespace std;

PpiBeamModel::PpiBeamModel(DisplayFieldController *displayFieldController,
                     bool haveFilteredFields)
        
{

  _aspectRatio = _params.ppi_aspect_ratio;
  _colorScaleWidth = _params.color_scale_width;

  // initialoze world view

  configureRange(_params.max_range_km);

  setGrids(_params.ppi_grids_on_at_startup);
  setRings(_params.ppi_range_rings_on_at_startup);
  setAngleLines(_params.ppi_azimuth_lines_on_at_startup);

  _isArchiveMode = false;
  _isStartOfSweep = true;

  _plotStartTime.set(0);
  _plotEndTime.set(0);
  _meanElev = -9999.0;
  _sumElev = 0.0;
  _nRays = 0.0;

  startTimer(50);  //used for boundary editor to detect shift key down (changes cursor)
}

/*************************************************************************
 * Destructor
 */

PpiBeamModel::~PpiBeamModel()
{

  // delete all of the dynamically created beams
  
  for (size_t i = 0; i < _ppiBeams.size(); ++i) {
    Beam::deleteIfUnused(_ppiBeams[i]);
  }
  LOG(DEBUG) << "_ppiBeams.clear()";
  _ppiBeams.clear();

}

/*************************************************************************
 * clear()
 */

void PpiBeamModel::clear()
{
  LOG(DEBUG) << "enter";
  // Clear out the beam array
  
  for (size_t i = 0; i < _ppiBeams.size(); i++) {
    Beam::deleteIfUnused(_ppiBeams[i]);
  }
  LOG(DEBUG) << "_ppiBeams.clear()";
  _ppiBeams.clear();
  
  // Now rerender the images
  
  _refreshImages();

  LOG(DEBUG) << "exit";
}



/*************************************************************************
 * addBeam()
 */

void PpiBeamModel::addBeam(const RadxRay *ray,
                        const float start_angle,
                        const float stop_angle,
                        const std::vector< std::vector< double > > &beam_data,
			   //                        const std::vector< DisplayField* > &fields)
			  DisplayFieldController *displayFieldController
			  )
{

  LOG(DEBUG) << "enter";

  // add a new beam to the display. 
  // The steps are:
  // 1. preallocate mode: find the beam to be drawn, or dynamic mode:
  //    create the beam(s) to be drawn.
  // 2. fill the colors for all variables in the beams to be drawn
  // 3. make the display list for the selected variables in the beams
  //    to be drawn.
  // 4. call the new display list(s)

  std::vector< PpiBeam* > newBeams;

  // The start and stop angle MUST specify a clockwise fill for the sector.
  // Thus if start_angle > stop_angle, we know that we have crossed the 0
  // boundary, and must break it up into 2 beams.

  // Create the new beam(s), to keep track of the display information.
  // Beam start and stop angles are adjusted here so that they always 
  // increase clockwise. Likewise, if a beam crosses the 0 degree boundary,
  // it is split into two beams, each of them again obeying the clockwise
  // rule. Prescribing these rules makes the beam culling logic a lot simpler.

  // Normalize the start and stop angles.  I'm not convinced that this works
  // for negative angles, but leave it for now.

  double n_start_angle = start_angle - ((int)(start_angle/360.0))*360.0;
  double n_stop_angle = stop_angle - ((int)(stop_angle/360.0))*360.0;

  if (n_start_angle <= n_stop_angle) {

    // This beam does not cross the 0 degree angle.  Just add the beam to
    // the beam list.

    PpiBeam* b = new PpiBeam(_params, ray, _fields.size(), 
                             n_start_angle, n_stop_angle);
    b->addClient();
    _cullBeams(b);
    _ppiBeams.push_back(b);
    newBeams.push_back(b);

  } else {

    // The beam crosses the 0 degree angle.  First add the portion of the
    // beam to the left of the 0 degree point.

    PpiBeam* b1 = new PpiBeam(_params, ray, _fields.size(), n_start_angle, 360.0);
    b1->addClient();
    _cullBeams(b1);
    _ppiBeams.push_back(b1);
    newBeams.push_back(b1);

    // Now add the portion of the beam to the right of the 0 degree point.

    PpiBeam* b2 = new PpiBeam(_params, ray, _fields.size(), 0.0, n_stop_angle);
    b2->addClient();
    _cullBeams(b2);
    _ppiBeams.push_back(b2);
    newBeams.push_back(b2);

  }

  // compute angles and times in archive mode

  if (newBeams.size() > 0) {
    
    if (_isArchiveMode) {

      if (_isStartOfSweep) {
        _plotStartTime = ray->getRadxTime();
        _meanElev = -9999.0;
        _sumElev = 0.0;
        _nRays = 0.0;
        _isStartOfSweep = false;
      }
      _plotEndTime = ray->getRadxTime();
      _sumElev += ray->getElevationDeg();
      _nRays++;
      _meanElev = _sumElev / _nRays;
      LOG(DEBUG) << "isArchiveMode _nRays = " << _nRays;    
    } // if (_isArchiveMode) 
    
  } // if (newBeams.size() > 0) 


  if (_params.debug >= Params::DEBUG_VERBOSE &&
      _ppiBeams.size() % 10 == 0) {
    cerr << "==>> _ppiBeams.size(): " << _ppiBeams.size() << endl;
  }
  LOG(DEBUG) << "number of new Beams " << newBeams.size();

  // newBeams has pointers to all of the newly added beams.  Render the
  // beam data.

  for (size_t ii = 0; ii < newBeams.size(); ii++) {

    PpiBeam *beam = newBeams[ii];
    
    // Set up the brushes for all of the fields in this beam.  This can be
    // done independently of a Painter object.
    
    //beam->fillColors(beam_data, fields, &_backgroundBrush);
    //const ColorMap &map = colorMap;
    // TODO: need colormap for each field
      beam->fillColors(beam_data, displayFieldController, &_backgroundBrush);
    // Add the new beams to the render lists for each of the fields
    
    for (size_t field = 0; field < _fieldRenderers.size(); ++field) {
      if (field == _selectedField ||
          _fieldRenderers[field]->isBackgroundRendered()) {
        _fieldRenderers[field]->addBeam(beam);
      } else {
        beam->setBeingRendered(field, false);
      }
    }
    
  } /* endfor - beam */

  // Start the threads to render the new beams

  _performRendering();

  LOG(DEBUG) << "exit";
}


/*************************************************************************
 * configureRange()
 */

void PpiBeamModel::configureRange(double max_range)
{

  // Save the specified values

  _maxRangeKm = max_range;

  // Set the ring spacing.  This is dependent on the value of _maxRange.

  _setGridSpacing();
  
  // set world view

  int leftMargin = 0;
  int rightMargin = 0;
  int topMargin = 0;
  int bottomMargin = 0;
  int colorScaleWidth = _params.color_scale_width;
  int axisTickLen = 7;
  int nTicksIdeal = 7;
  int textMargin = 5;

  if (_params.ppi_display_type == Params::PPI_AIRBORNE) {

    _fullWorld.set(width(), height(),
                   leftMargin, rightMargin, topMargin, bottomMargin, colorScaleWidth,
                   -_maxRangeKm, 0.0,
                   _maxRangeKm, _maxRangeKm,
                   axisTickLen, nTicksIdeal, textMargin);
    
  } else {
    
    _fullWorld.set(width(), height(),
                   leftMargin, rightMargin, topMargin, bottomMargin, colorScaleWidth,
                   -_maxRangeKm, -_maxRangeKm,
                   _maxRangeKm, _maxRangeKm,
                   axisTickLen, nTicksIdeal, textMargin);

  }
  
  _zoomWorld = _fullWorld;
  _isZoomed = false;
  _setTransform(_zoomWorld.getTransform());
  _setGridSpacing();

  // Initialize the images used for double-buffering.  For some reason,
  // the window size is incorrect at this point, but that will be corrected
  // by the system with a call to resize().

  _refreshImages();
  
}



////////////////////////////////////////////////////////////////////////////
// get ray closest to click point

const RadxRay *PpiBeamModel::_getClosestRay(double x_km, double y_km)

{

  double clickAz = atan2(y_km, x_km) * RAD_TO_DEG;
  double radarDisplayAz = 90.0 - clickAz;
  if (radarDisplayAz < 0.0) radarDisplayAz += 360.0;
  LOG(DEBUG) << "clickAz = " << clickAz << " from x_km, y_km = " 
                          << x_km << "," << y_km; 
  LOG(DEBUG) << "radarDisplayAz = " << radarDisplayAz << " from x_km, y_km = "
             << x_km << y_km;

  double minDiff = 1.0e99;
  const RadxRay *closestRay = NULL;
  for (size_t ii = 0; ii < _ppiBeams.size(); ii++) {
    const RadxRay *ray = _ppiBeams[ii]->getRay();
    double rayAz = ray->getAzimuthDeg();
    double diff = fabs(radarDisplayAz - rayAz);
    if (diff > 180.0) {
      diff = fabs(diff - 360.0);
    }
    if (diff < minDiff) {
      closestRay = ray;
      minDiff = diff;
    }
  }

  if (closestRay != NULL)
    LOG(DEBUG) << "closestRay has azimuth " << closestRay->getAzimuthDeg();
  else
    LOG(DEBUG) << "Error: No ray found";
  return closestRay;

}


/*************************************************************************
 * numBeams()
 */

size_t PpiBeamModel::getNumBeams() const
{
  return _ppiBeams.size();
}

/*************************************************************************
 * _beamIndex()
 */

int PpiBeamModel::_beamIndex(const double start_angle,
                          const double stop_angle)
{

  // Find where the center angle of the beam will fall within the beam array
  
  int ii = (int)
    (_ppiBeams.size()*(start_angle + (stop_angle-start_angle)/2)/360.0);

  // Take care of the cases at the ends of the beam list
  
  if (ii < 0)
    ii = 0;
  if (ii > (int)_ppiBeams.size() - 1)
    ii = _ppiBeams.size() - 1;

  return ii;

}


/*************************************************************************
 * _cullBeams()
 */

void PpiBeamModel::_cullBeams(const PpiBeam *beamAB)
{
  // This routine examines the collection of beams, and removes those that are 
  // completely occluded by other beams. The algorithm gives precedence to the 
  // most recent beams; i.e. beams at the end of the _ppiBeams vector.
  //
  // Remember that there won't be any beams that cross angles through zero; 
  // otherwise the beam culling logic would be a real pain, and PpiBeamModel has
  // already split incoming beams into two, if it received a beam of this type.
  //
  // The logic is as follows. First of all, just consider the start and stop angle 
  // of a beam to be a linear region. We can diagram the angle interval of beam(AB) as:
  //         a---------b
  // 
  // The culling logic will compare all other beams (XY) to AB, looking for an overlap.
  // An example overlap might be:
  //         a---------b
  //    x---------y
  // 
  // If an overlap on beam XY is detected, the occluded region is recorded
  //   as the interval (CD):        
  //         a---------b
  //    x---------y
  //         c----d
  // 
  // The culling algorithm starts with the last beam in the list, and compares it with all
  // preceeding beams, setting their overlap regions appropriately.
  // Then the next to the last beam is compared with all preceeding beams.
  // Previously found occluded regions will be expanded as they are detected.
  // 
  // Once the occluded region spans the entire beam, then the beam is known 
  // to be hidden, and it doesn't need to be tested any more, nor is it it used as a 
  // test on other beams.
  //
  // After the list has been completly processed in this manner, the completely occluded 
  // beams are removed.
  // .
  // Note now that if the list is rendered from beginning to end, the more recent beams will
  // overwrite the portions of previous beams that they share.
  //

  // NOTE - This algorithm doesn't handle beams that are occluded in different
  // subsections.  For example, the following would be handled as a hidden
  // beam even though the middle of the beam is still visible:
  //         a---------b    c--------d
  //              x-------------y

  // Do nothing if we don't have any beams in the list

  if (_ppiBeams.size() < 1)
    return;

  // Look through all of the beams in the list and record any place where
  // this beam occludes any other beam.

  bool need_to_cull = false;
  
  // Save the angle information for easier processing.
  
  double a = beamAB->startAngle;
  double b = beamAB->stopAngle;

  // Look at all of the beams in the list to see if any are occluded by this
  // new beam

  for (size_t j = 0; j < _ppiBeams.size(); ++j)
  {
    // Pull the beam from the list for ease of coding

    PpiBeam *beamXY = _ppiBeams[j];

    // If this beam has alread been marked hidden, we don't need to 
    // look at it.

    if (beamXY->hidden)
      continue;
      
    // Again, save the angles for easier coding

    double x = beamXY->startAngle;
    double y = beamXY->stopAngle;

    if (b <= x || a >= y)
    {
      //  handles these cases:
      //  a-----b                a-----b
      //           x-----------y
      //  
      // they don't overlap at all so do nothing
    }
    else if (a <= x && b >= y)
    {
      //     a------------------b
      //        x-----------y
      // completely covered

      beamXY->hidden = true;
      need_to_cull = true;
    }
    else if (a <= x && b <= y)
    {
      //   a-----------b
      //        x-----------y
      //
      // We know that b > x because otherwise this would have been handled
      // in the first case above.

      // If the right part of this beam is already occluded, we can just
      // mark the beam as hidden at this point.  Otherwise, we update the
      // c and d values.

      if (beamXY->rightEnd == y)
      {
	beamXY->hidden = true;
	need_to_cull = true;
      }
      else
      {
	beamXY->leftEnd = x;
	if (beamXY->rightEnd < b)
	  beamXY->rightEnd = b;
      }
    }
    else if (a >= x && b >= y)
    {
      //       a-----------b
      //   x-----------y
      //
      // We know that a < y because otherwise this would have been handled
      // in the first case above.
      
      // If the left part of this beam is already occluded, we can just
      // mark the beam as hidden at this point.  Otherwise, we update the
      // c and d values.

      if (beamXY->leftEnd == x)
      {
	beamXY->hidden = true;
	need_to_cull = true;
      }
      else
      {
	beamXY->rightEnd = y;
	if (a < beamXY->leftEnd)
	  beamXY->leftEnd = a;
      }
    }
    else
    {
      // all that is left is this pathological case:
      //     a-------b
      //   x-----------y
      //
      // We need to extend c and d, if the are inside of a and b.  We know
      // that a != x and b != y because otherwise this would have been
      // handled in the third case above.

      if (beamXY->leftEnd > a)
	beamXY->leftEnd = a;
      if (beamXY->rightEnd < b)
	beamXY->rightEnd = b;
	      
    } /* endif */
  } /* endfor - j */

  // Now actually cull the list

  if (need_to_cull)
  {
    // Note that i has to be an int rather than a size_t since we are going
    // backwards through the list and will end when i < 0.

    for (int i = _ppiBeams.size()-1; i >= 0; i--)
    {
      // Delete beams who are hidden but aren't currently being rendered.
      // We can get the case where we have hidden beams that are being
      // rendered when we do something (like resizing) that causes us to 
      // have to rerender all of the current beams.  During the rerendering,
      // new beams continue to come in and will obscure some of the beams
      // that are still in the rendering queue.  These beams will be deleted
      // during a later pass through this loop.

      if (_ppiBeams[i]->hidden && !_ppiBeams[i]->isBeingRendered())
      {
        Beam::deleteIfUnused(_ppiBeams[i]);
	_ppiBeams.erase(_ppiBeams.begin()+i);
      }
    }

  } /* endif - need_to_cull */
  
}
