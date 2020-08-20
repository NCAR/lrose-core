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

#include "PpiPlot.hh"
#include "PolarManager.hh"
#include "SpreadSheetView.hh"
#include "SpreadSheetController.hh"
#include "ParameterColorView.hh"
#include "FieldColorController.hh"
#include "DisplayFieldModel.hh"
#include "BoundaryPointEditor.hh"
#include "SpreadSheetView.hh"

#include <toolsa/toolsa_macros.h>
#include <toolsa/LogStream.hh>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QErrorMessage>
#include <QRect>

using namespace std;



PpiPlot::PpiPlot(PolarWidget* parent,
                 const PolarManager &manager,
                 const Params &params,
                 int id,
                 const RadxPlatform &platform,
                 const vector<DisplayField *> &fields,
                 bool haveFilteredFields) :
        PolarPlot(parent, manager, params, id,
                  platform, fields, haveFilteredFields)
        
{

  _aspectRatio = _params.ppi_aspect_ratio;
  _colorScaleWidth = _params.color_scale_width;

  // initialize world view

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

  // set up ray locators

  _rayLoc.resize(RayLoc::RAY_LOC_N);

}

/*************************************************************************
 * Destructor
 */

PpiPlot::~PpiPlot()
{

  // delete all of the dynamically created beams
  
  for (size_t i = 0; i < _ppiBeams.size(); ++i) {
    Beam::deleteIfUnused(_ppiBeams[i]);
  }
  _ppiBeams.clear();

}

/*************************************************************************
 * clear()
 */

void PpiPlot::clear()
{
  // Clear out the beam array
  
  for (size_t i = 0; i < _ppiBeams.size(); i++) {
    Beam::deleteIfUnused(_ppiBeams[i]);
  }
  _ppiBeams.clear();
  
  // Now rerender the images
  
  refreshImages();
  _parent->showOpeningFileMsg(false);
}

/*************************************************************************
 * selectVar()
 */

void PpiPlot::selectVar(const size_t index)
{

  // If the field index isn't actually changing, we don't need to do anything
  
  if (_fieldNum == index) {
    return;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=========>> PpiPlot::selectVar() for field index: " 
         << index << endl;
  }

  // If this field isn't being rendered in the background, render all of
  // the beams for it

  if (!_fieldRenderers[index]->isBackgroundRendered()) {
    std::vector< PpiBeam* >::iterator beam;
    for (beam = _ppiBeams.begin(); beam != _ppiBeams.end(); ++beam) {
      (*beam)->setBeingRendered(index, true);
      _fieldRenderers[index]->addBeam(*beam);
    }
  }
  _performRendering();

  // Do any needed housekeeping when the field selection is changed

  _fieldRenderers[_fieldNum]->unselectField();
  _fieldRenderers[index]->selectField();
  
  // Change the selected field index

  _fieldNum = index;

  // Update the display

  _parent->update();
}


/*************************************************************************
 * clearVar()
 */

void PpiPlot::clearVar(const size_t index)
{

  if (index >= _fields.size()) {
    return;
  }

  // Set the brush for every beam/gate for this field to use the background
  // color

  std::vector< PpiBeam* >::iterator beam;
  for (beam = _ppiBeams.begin(); beam != _ppiBeams.end(); ++beam) {
    (*beam)->resetFieldBrush(index, &_backgroundBrush);
  }
  
  if (index == _fieldNum) {
    _parent->update();
  }

}


/*************************************************************************
 * addBeam()
 */

void PpiPlot::addBeam(const RadxRay *ray,
                      const std::vector< std::vector< double > > &beam_data,
                      const std::vector< DisplayField* > &fields)
{

  LOG(DEBUG) << "enter";

  double az = ray->getAzimuthDeg();
  _storeRayLoc(ray, az, _platform.getRadarBeamWidthDegH());

  double start_angle = _startAz;
  double stop_angle = _endAz;

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
    
    beam->fillColors(beam_data, fields, &_backgroundBrush);

    // Add the new beams to the render lists for each of the fields
    
    for (size_t field = 0; field < _fieldRenderers.size(); ++field) {
      if (field == _fieldNum ||
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


///////////////////////////////////////////////////////////
// store ray location

void PpiPlot::_storeRayLoc(const RadxRay *ray, 
                           const double az,
                           const double beam_width)
{

  LOG(DEBUG) << "az = " << az << " beam_width = " << beam_width;

  // Determine the extent of this ray

  if (_params.ppi_override_rendering_beam_width) {
    double half_angle = _params.ppi_rendering_beam_width / 2.0;
    _startAz = az - half_angle - 0.1;
    _endAz = az + half_angle + 0.1;
  } else if (ray->getIsIndexed()) {
    double half_angle = ray->getAngleResDeg() / 2.0;
    _startAz = az - half_angle - 0.1;
    _endAz = az + half_angle + 0.1;
  } else {
    double beam_width_min = beam_width;
    if (beam_width_min < 0) 
      beam_width_min = 10.0;

    double max_half_angle = beam_width_min / 2.0;
    double prev_offset = max_half_angle;
    if (_prevAz > 0.0) { // >= 0.0) {
      double az_diff = az - _prevAz;
      if (az_diff < 0.0)
	az_diff += 360.0;
      double half_az_diff = az_diff / 2.0;
	
      if (prev_offset > half_az_diff)
	prev_offset = half_az_diff;
    }
    _startAz = az - prev_offset - 0.1;
    _endAz = az + max_half_angle + 0.1;
  }
    
  // store
  // HERE !!! fix up negative values here or in clearRayOverlap??
  if (_startAz < 0) _startAz += 360.0;
  if (_endAz < 0) _endAz += 360.0;
  if (_startAz >= 360) _startAz -= 360.0;
  if (_endAz >= 360) _endAz -= 360.0;
    
  LOG(DEBUG) << " startAz = " << _startAz << " endAz = " << _endAz;

  // compute start and end indices, using modulus to keep with array bounds

  int startIndex = ((int) (_startAz * RayLoc::RAY_LOC_RES)) % RayLoc::RAY_LOC_N;
  int endIndex = ((int) (_endAz * RayLoc::RAY_LOC_RES + 1)) % RayLoc::RAY_LOC_N;

  // Clear out any rays in the locations list that are overlapped by the
  // new ray
    
  if (startIndex > endIndex) {

    // area crosses the 360; 0 boundary; must break into two sections

    // first from start index to 360
    
    _clearRayOverlap(startIndex, RayLoc::RAY_LOC_N - 1);

    for (int ii = startIndex; ii < RayLoc::RAY_LOC_N; ii++) { // RayLoc::RAY_LOC_N; ii++) {
      _rayLoc[ii].ray = ray;
      _rayLoc[ii].active = true;
      _rayLoc[ii].startIndex = startIndex;
      _rayLoc[ii].endIndex = RayLoc::RAY_LOC_N - 1; // RayLoc::RAY_LOC_N;
    }

    // then from 0 to end index
    
    _clearRayOverlap(0, endIndex);

    // Set the locations associated with this ray
    
    for (int ii = 0; ii <= endIndex; ii++) {
      _rayLoc[ii].ray = ray;
      _rayLoc[ii].active = true;
      _rayLoc[ii].startIndex = 0;
      _rayLoc[ii].endIndex = endIndex;
    }

  } else { // if (startIndex > endIndex) 

    _clearRayOverlap(startIndex, endIndex);
    
    // Set the locations associated with this ray

    for (int ii = startIndex; ii <= endIndex; ii++) {
      _rayLoc[ii].ray = ray;
      _rayLoc[ii].active = true;
      _rayLoc[ii].startIndex = startIndex;
      _rayLoc[ii].endIndex = endIndex;
    }

  } // if (startIndex > endIndex) 

}

 
///////////////////////////////////////////////////////////
// clear any locations that are overlapped by the given ray

void PpiPlot::_clearRayOverlap(const int start_index, const int end_index)
{

  LOG(DEBUG) << "enter" << " start_index=" << start_index <<
    " end_index = " << end_index;

  if ((start_index < 0) || (start_index > RayLoc::RAY_LOC_N)) {
    cout << "ERROR: _clearRayOverlap start_index out of bounds " << start_index << endl;
    return;
  }
  if ((end_index < 0) || (end_index > RayLoc::RAY_LOC_N)) {
    cout << "ERROR: _clearRayOverlap end_index out of bounds " << end_index << endl;
    return;
  }

  // Loop through the ray locations, clearing out old information

  int i = start_index;
  
  while (i <= end_index) {

    RayLoc &loc = _rayLoc[i];
    
    // If this location isn't active, we can skip it

    if (!loc.active) {
      // LOG(DEBUG) << "loc NOT active";
      ++i;
      continue;
    }
    
    int loc_start_index = loc.startIndex;
    int loc_end_index = loc.endIndex;

    if ((loc_start_index < 0) || (loc_start_index > RayLoc::RAY_LOC_N)) {
      cout << "ERROR: _clearRayOverlap loc_start_index out of bounds " << loc_start_index << endl;
      ++i;
      continue;
    }
    if ((loc_end_index < 0) || (loc_end_index > RayLoc::RAY_LOC_N)) {
      cout << "ERROR: _clearRayOverlap loc_end_index out of bounds " << loc_end_index << endl;
      ++i;
      continue;
    }

    if (loc_end_index < i) {
      cout << " OH NO! We are HERE" << endl;
      ++i;
      continue;
    }
    // If we get here, this location is active.  We now have 4 possible
    // situations:

    if (loc.startIndex < start_index && loc.endIndex <= end_index) {

      // The overlap area covers the end of the current beam.  Reduce the
      // current beam down to just cover the area before the overlap area.
      LOG(DEBUG) << "Case 1a:";
      LOG(DEBUG) << " i = " << i;
      LOG(DEBUG) << "clearing from start_index=" << start_index <<
	  " to loc_end_index=" << loc_end_index;
      
      for (int j = start_index; j <= loc_end_index; ++j) {

	_rayLoc[j].ray = NULL;
	_rayLoc[j].active = false;

      }

      // Update the end indices for the remaining locations in the current
      // beam
      LOG(DEBUG) << "Case 1b:";
      LOG(DEBUG) << "setting endIndex to " << start_index - 1 << " from loc_start_index=" << loc_start_index <<
	  " to start_index=" << start_index;
      
      for (int j = loc_start_index; j < start_index; ++j)
	_rayLoc[j].endIndex = start_index - 1;

    } else if (loc.startIndex < start_index && loc.endIndex > end_index) {
      
      // The current beam is bigger than the overlap area.  This should never
      // happen, so go ahead and just clear out the locations for the current
      // beam.
      LOG(DEBUG) << "Case 2:";
      LOG(DEBUG) << " i = " << i;
      LOG(DEBUG) << "clearing from loc_start_index=" << loc_start_index <<
	  " to loc_end_index=" << loc_end_index;
      
      for (int j = loc_start_index; j <= loc_end_index; ++j) {
        _rayLoc[j].clear();
      }

    } else if (loc.endIndex > end_index) {
      
      // The overlap area covers the beginning of the current beam.  Reduce the
      // current beam down to just cover the area after the overlap area.

	LOG(DEBUG) << "Case 3a:";
	LOG(DEBUG) << " i = " << i;
	LOG(DEBUG) << "clearing from loc_start_index=" << loc_start_index <<
	  " to end_index=" << end_index;

      for (int j = loc_start_index; j <= end_index; ++j) {
	_rayLoc[j].ray = NULL;
	_rayLoc[j].active = false;
      }

      // Update the start indices for the remaining locations in the current
      // beam

      LOG(DEBUG) << "Case 3b:";
      LOG(DEBUG) << "setting startIndex to " << end_index + 1 << " from end_index=" << end_index <<
	  " to loc_end_index=" << loc_end_index;
      
      for (int j = end_index + 1; j <= loc_end_index; ++j) {
	_rayLoc[j].startIndex = end_index + 1;
      }

    } else {
      
      // The current beam is completely covered by the overlap area.  Clear
      // out all of the locations for the current beam.
      LOG(DEBUG) << "Case 4:";
      LOG(DEBUG) << " i = " << i;
      LOG(DEBUG) << "clearing from loc_start_index=" << loc_start_index <<
	  " to loc_end_index=" << loc_end_index;
      
      for (int j = loc_start_index; j <= loc_end_index; ++j) {
        _rayLoc[j].clear();
      }

    }
    
    i = loc_end_index + 1;

  } /* endwhile - i */
  
  LOG(DEBUG) << "exit ";
  
}


/*************************************************************************
 * configureRange()
 */

void PpiPlot::configureRange(double max_range)
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
  int colorScaleWidth = 0;
  int axisTickLen = 7;
  int nTicksIdeal = 7;
  int textMargin = 5;

  if (_params.ppi_display_type == Params::PPI_AIRBORNE) {

    // _fullWorld.setWindowGeom(_parent->width(), _parent->height(), 0, 0);
    _fullWorld.setLeftMargin(leftMargin);
    _fullWorld.setRightMargin(rightMargin);
    _fullWorld.setTopMargin(topMargin);
    _fullWorld.setBottomMargin(bottomMargin);
    _fullWorld.setColorScaleWidth(colorScaleWidth);
    _fullWorld.setWorldLimits(-_maxRangeKm, 0.0, _maxRangeKm, _maxRangeKm);
    _fullWorld.setXAxisTickLen(axisTickLen);
    _fullWorld.setXNTicksIdeal(nTicksIdeal);
    _fullWorld.setAxisTextMargin(textMargin);

  } else {
    
    cerr << "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP _maxRangeKm: " << _maxRangeKm << endl;

    // _fullWorld.setWindowGeom(_parent->width(), _parent->height(), 0, 0);
    // _fullWorld.setWindowGeom(400, 400, 0, 0);
    _fullWorld.setLeftMargin(leftMargin);
    _fullWorld.setRightMargin(rightMargin);
    _fullWorld.setTopMargin(topMargin);
    _fullWorld.setBottomMargin(bottomMargin);
    _fullWorld.setColorScaleWidth(colorScaleWidth);
    _fullWorld.setWorldLimits(-_maxRangeKm, -_maxRangeKm, _maxRangeKm, _maxRangeKm);
    _fullWorld.setXAxisTickLen(axisTickLen);
    _fullWorld.setXNTicksIdeal(nTicksIdeal);
    _fullWorld.setAxisTextMargin(textMargin);

  }
  
  _zoomWorld = _fullWorld;
  _isZoomed = false;
  setTransform(_zoomWorld.getTransform());
  _setGridSpacing();

  // Initialize the images used for double-buffering.  For some reason,
  // the window size is incorrect at this point, but that will be corrected
  // by the system with a call to resize().

  refreshImages();
  
}

// Used to notify BoundaryPointEditor if the user has zoomed in/out or is pressing the Shift key
// Todo: investigate implementing a listener pattern instead
// void PpiPlot::timerEvent(QTimerEvent *event)
// {
//   bool doUpdate = false;
//   bool isBoundaryEditorVisible = _manager._boundaryEditorDialog->isVisible();
//   if (isBoundaryEditorVisible)
//   {
//     double xRange = _zoomWorld.getXMaxWorld() - _zoomWorld.getXMinWorld();
//     doUpdate = BoundaryPointEditor::Instance()->updateScale(xRange);   //user may have zoomed in or out, so update the polygon point boxes so they are the right size on screen
//   }
//   bool isBoundaryFinished = BoundaryPointEditor::Instance()->isAClosedPolygon();
//   bool isShiftKeyDown = (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) == true);
//   if ((isBoundaryEditorVisible && !isBoundaryFinished) || (isBoundaryEditorVisible && isBoundaryFinished && isShiftKeyDown))
//     this->setCursor(Qt::CrossCursor);
//   else
//     this->setCursor(Qt::ArrowCursor);

//   if (doUpdate)  //only update if something has changed
//     _parent->update();
// }


/*************************************************************************
 * mouseReleaseEvent()
 */
// void PpiPlot::mouseReleaseEvent(QMouseEvent *e)
// {

//   _pointClicked = false;

//   QRect rgeom = _rubberBand->geometry();

//   // If the mouse hasn't moved much, assume we are clicking rather than
//   // zooming

//   QPointF clickPos(e->pos());
  
//   _mouseReleaseX = clickPos.x();
//   _mouseReleaseY = clickPos.y();

//   // get click location in world coords

//   if (rgeom.width() <= 20)
//   {

//     // Emit a signal to indicate that the click location has changed
//     _worldReleaseX = _zoomWorld.getXWorld(_mouseReleaseX);
//     _worldReleaseY = _zoomWorld.getYWorld(_mouseReleaseY);

//     // If boundary editor active, then interpret boundary mouse release event
//     if (_manager._boundaryEditorDialog->isVisible())
//     {
//       if (BoundaryPointEditor::Instance()->getCurrentTool() == BoundaryToolType::polygon)
//       {
//         if (!BoundaryPointEditor::Instance()->isAClosedPolygon())
//           BoundaryPointEditor::Instance()->addPoint(_worldReleaseX, _worldReleaseY);
//         else  //polygon finished, user may want to insert/delete a point
//           BoundaryPointEditor::Instance()->checkToAddOrDelPoint(_worldReleaseX, _worldReleaseY);
//       }
//       else if (BoundaryPointEditor::Instance()->getCurrentTool() == BoundaryToolType::circle)
//       {
//         if (BoundaryPointEditor::Instance()->isAClosedPolygon())
//           BoundaryPointEditor::Instance()->checkToAddOrDelPoint(_worldReleaseX, _worldReleaseY);
//         else
//           BoundaryPointEditor::Instance()->makeCircle(_worldReleaseX, _worldReleaseY, BoundaryPointEditor::Instance()->getCircleRadius());
//       }
//     }

//     double x_km = _worldReleaseX;
//     double y_km = _worldReleaseY;
//     _pointClicked = true;

//     // get ray closest to click point

//     const RadxRay *closestRay = _getClosestRay(x_km, y_km);

//     // emit signal

//     emit locationClicked(x_km, y_km, closestRay);
  
//   }
//   else
//   {

//     // mouse moved more than 20 pixels, so a zoom occurred
    
//     _worldPressX = _zoomWorld.getXWorld(_mousePressX);
//     _worldPressY = _zoomWorld.getYWorld(_mousePressY);

//     _worldReleaseX = _zoomWorld.getXWorld(_zoomCornerX);
//     _worldReleaseY = _zoomWorld.getYWorld(_zoomCornerY);

//     _zoomWorld.setWorldLimits(_worldPressX, _worldPressY, _worldReleaseX, _worldReleaseY);

//     _setTransform(_zoomWorld.getTransform());

//     _setGridSpacing();

//     // enable unzoom button
    
//     _manager.enableZoomButton();
    
//     // Update the window in the renderers
    
//     refreshImages();

//   }
    
//   // hide the rubber band

//   _rubberBand->hide();
//   _parent->update();

// }


////////////////////////////////////////////////////////////////////////////
// get ray closest to click point

const RadxRay *PpiPlot::_getClosestRay(double x_km, double y_km)

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
 * _setGridSpacing()
 */

void PpiPlot::_setGridSpacing()
{

  double xRange = _zoomWorld.getXMaxWorld() - _zoomWorld.getXMinWorld();
  double yRange = _zoomWorld.getYMaxWorld() - _zoomWorld.getYMinWorld();
  double diagonal = sqrt(xRange * xRange + yRange * yRange);

  if (diagonal <= 1.0) {
    _ringSpacing = 0.05;
  } else if (diagonal <= 2.0) {
    _ringSpacing = 0.1;
  } else if (diagonal <= 5.0) {
    _ringSpacing = 0.2;
  } else if (diagonal <= 10.0) {
    _ringSpacing = 0.5;
  } else if (diagonal <= 20.0) {
    _ringSpacing = 1.0;
  } else if (diagonal <= 50.0) {
    _ringSpacing = 2.0;
  } else if (diagonal <= 100.0) {
    _ringSpacing = 5.0;
  } else if (diagonal <= 200.0) {
    _ringSpacing = 10.0;
  } else if (diagonal <= 300.0) {
    _ringSpacing = 20.0;
  } else if (diagonal <= 400.0) {
    _ringSpacing = 25.0;
  } else if (diagonal <= 500.0) {
    _ringSpacing = 50.0;
  } else {
    _ringSpacing = 50.0;
  }

}


/*************************************************************************
 * _drawOverlays()
 */

void PpiPlot::_drawOverlays(QPainter &painter)
{

  // Don't try to draw rings if we haven't been configured yet or if the
  // rings or grids aren't enabled.
  
  if (!_ringsEnabled && !_gridsEnabled && !_angleLinesEnabled) {
    return;
  }

  // save painter state

  painter.save();

  // store font
  
  QFont origFont = painter.font();
  
  // Draw rings

  if (_ringSpacing > 0.0 && _ringsEnabled) {

    // Set up the painter
    
    painter.save();
    painter.setTransform(_zoomTransform);
    painter.setPen(_gridRingsColor);
  
    // set narrow line width
    QPen pen = painter.pen();
    pen.setWidth(0);
    painter.setPen(pen);
    painter.setPen(_gridRingsColor);

    double ringRange = _ringSpacing;
    while (ringRange <= _maxRangeKm) {
      QRectF rect(-ringRange, -ringRange, ringRange * 2.0, ringRange * 2.0);
      painter.drawEllipse(rect);
      ringRange += _ringSpacing;
    }
    painter.restore();

    // Draw the labels
    
    QFont font = painter.font();
    font.setPointSizeF(_params.range_ring_label_font_size);
    painter.setFont(font);
    painter.setPen(_gridRingsColor);
    // painter.setWindow(0, 0, width(), height());
    
    ringRange = _ringSpacing;
    while (ringRange <= _maxRangeKm) {
      double labelPos = ringRange * SIN_45;
      const string &labelStr = _scaledLabel.scale(ringRange);
      _zoomWorld.drawText(painter, labelStr, labelPos, labelPos, Qt::AlignCenter);
      _zoomWorld.drawText(painter, labelStr, -labelPos, labelPos, Qt::AlignCenter);
      _zoomWorld.drawText(painter, labelStr, labelPos, -labelPos, Qt::AlignCenter);
      _zoomWorld.drawText(painter, labelStr, -labelPos, -labelPos, Qt::AlignCenter);
      ringRange += _ringSpacing;
    }

  } /* endif - draw rings */
  
  // Draw the grid

  if (_ringSpacing > 0.0 && _gridsEnabled)  {

    // Set up the painter
    
    painter.save();
    painter.setTransform(_zoomTransform);
    painter.setPen(_gridRingsColor);
  
    double ringRange = _ringSpacing;
    double maxRingRange = ringRange;
    while (ringRange <= _maxRangeKm) {

      cerr << "1111111 ringRange: " << ringRange << endl;
      cerr << "1111111 maxRangeKm: " << _maxRangeKm << endl;
      cerr << "1111111 minX, minY, maxX, maxY: "
           << ringRange << ", " << -_maxRangeKm << ", "
           << ringRange << ", " << _maxRangeKm << endl;
      cerr << "1111111 minX, minY, maxX, maxY: "
           << -ringRange << ", " << -_maxRangeKm << ", "
           << -ringRange << ", " << _maxRangeKm << endl;
      cerr << "1111111 minX, minY, maxX, maxY: "
           << -_maxRangeKm << ", " << ringRange << ", "
           << _maxRangeKm << ", " << ringRange << endl;
      cerr << "1111111 minX, minY, maxX, maxY: "
           << -_maxRangeKm << ", " << -ringRange << ", "
           << _maxRangeKm << ", " << -ringRange << endl;

      _zoomWorld.drawLine(painter, ringRange-50, -_maxRangeKm-50, ringRange-50, _maxRangeKm-50);
      _zoomWorld.drawLine(painter, -ringRange-50, -_maxRangeKm-50, -ringRange-50, _maxRangeKm-50);
      _zoomWorld.drawLine(painter, -_maxRangeKm-50, ringRange-50, _maxRangeKm-50, ringRange-50);
      _zoomWorld.drawLine(painter, -_maxRangeKm-50, -ringRange-50, _maxRangeKm-50, -ringRange-50);
      
      maxRingRange = ringRange;
      ringRange += _ringSpacing;
    }
    painter.restore();

    _zoomWorld.specifyXTicks(-maxRingRange, _ringSpacing);
    _zoomWorld.specifyYTicks(-maxRingRange, _ringSpacing);

    _zoomWorld.drawAxisLeft(painter, "km", true, true, true, false);
    _zoomWorld.drawAxisRight(painter, "km", true, true, true, false);
    _zoomWorld.drawAxisTop(painter, "km", true, true, true, false);
    _zoomWorld.drawAxisBottom(painter, "km", true, true, true, false);
    
    _zoomWorld.unspecifyXTicks();
    _zoomWorld.unspecifyYTicks();

  }
  
  // Draw the azimuth lines

  if (_angleLinesEnabled) {

    // Set up the painter
    
    painter.save();
    painter.setPen(_gridRingsColor);
  
    // Draw the lines along the X and Y axes

    _zoomWorld.drawLine(painter, 0, -_maxRangeKm, 0, _maxRangeKm);
    _zoomWorld.drawLine(painter, -_maxRangeKm, 0, _maxRangeKm, 0);

    // Draw the lines along the 30 degree lines

    double end_pos1 = SIN_30 * _maxRangeKm;
    double end_pos2 = COS_30 * _maxRangeKm;
    
    _zoomWorld.drawLine(painter, end_pos1, end_pos2, -end_pos1, -end_pos2);
    _zoomWorld.drawLine(painter, end_pos2, end_pos1, -end_pos2, -end_pos1);
    _zoomWorld.drawLine(painter, -end_pos1, end_pos2, end_pos1, -end_pos2);
    _zoomWorld.drawLine(painter, end_pos2, -end_pos1, -end_pos2, end_pos1);

    painter.restore();

  }
  
  // click point cross hairs
  
  if (_parent->getPointClicked()) {

    // int startX = _mouseReleaseX - _params.click_cross_size / 2;
    // int endX = _mouseReleaseX + _params.click_cross_size / 2;
    // int startY = _mouseReleaseY - _params.click_cross_size / 2;
    // int endY = _mouseReleaseY + _params.click_cross_size / 2;

    // painter.drawLine(startX, _mouseReleaseY, endX, _mouseReleaseY);
    // painter.drawLine(_mouseReleaseX, startY, _mouseReleaseX, endY);

    /****** testing ******
     // do smart brush ...
     QImage qImage;
     qImage = *(_fieldRenderers[_fieldNum]->getImage());
     // qImage.load("/h/eol/brenda/octopus.jpg");
     // get the Image from somewhere ...   
     // qImage.invertPixels();
     qImage.convertToFormat(QImage::Format_RGB32);

     // get the color of the selected pixel
     QRgb colorToMatch = qImage.pixel(_mouseReleaseX, _mouseReleaseY);
     // walk to all adjacent pixels of the same color and make them white

     vector<QPoint> pixelsToConsider;
     vector<QPoint> neighbors = {QPoint(-1, 1), QPoint(0, 1), QPoint(1, 1),
     QPoint(-1, 0),               QPoint(1, 0),
     QPoint(-1,-1), QPoint(0,-1), QPoint(1,-1)};

     pixelsToConsider.push_back(QPoint(_mouseReleaseX, _mouseReleaseY));
     while (!pixelsToConsider.empty()) {
     QPoint currentPix = pixelsToConsider.back();
     pixelsToConsider.pop_back();
     if (qImage.pixel(currentPix) ==  colorToMatch) {
     // set currentPix to white
     qImage.setPixelColor(currentPix, QColor("white"));
     // cout << "setting pixel " << currentPix.x() << ", " << currentPix.y() << " to white" << endl;
     // add the eight adjacent neighbors
     for (vector<QPoint>::iterator noffset = neighbors.begin(); 
     noffset != neighbors.end(); ++noffset) {
     QPoint neighbor;
     neighbor = currentPix + *noffset; // QPoint(-1,1);
     if (qImage.valid(neighbor)) {
     pixelsToConsider.push_back(neighbor);
     }
     } // end for neighbors iterator
     }
     }

     pixelsToConsider.clear();
     QPainter painter(this);
     painter.drawImage(0, 0, qImage);
     ****** end testing *****/

  }

  // reset painter state
  
  painter.restore();

  // draw the color scale

  const DisplayField &field = _manager.getSelectedField();
  _zoomWorld.drawColorScale(field.getColorMap(), painter,
                            _params.label_font_size);

  if (_archiveMode) {

    // add legends with time, field name and elevation angle

    vector<string> legends;
    char text[1024];

    // time legend

    sprintf(text, "Start time: %s", _plotStartTime.asString(0).c_str());
    legends.push_back(text);
    
    // radar and site name legend

    string radarName(_platform.getInstrumentName());
    if (_params.override_radar_name) {
      radarName = _params.radar_name;
    }
    string siteName(_platform.getInstrumentName());
    if (_params.override_site_name) {
      siteName = _params.site_name;
    }
    string radarSiteLabel = radarName;
    if (siteName.size() > 0 && siteName != radarName) {
      radarSiteLabel += "/";
      radarSiteLabel += siteName;
    }
    legends.push_back(radarSiteLabel);

    // field name legend

    string fieldName = _fieldRenderers[_fieldNum]->getField().getLabel();
    sprintf(text, "Field: %s", fieldName.c_str());
    legends.push_back(text);

    // elevation legend

    sprintf(text, "Elevation(deg): %.2f", _meanElev);
    legends.push_back(text);

    // nrays legend

    sprintf(text, "NRays: %g", _nRays);
    legends.push_back(text);
    
    painter.save();
    painter.setPen(Qt::yellow);
    painter.setBrush(Qt::black);
    painter.setBackgroundMode(Qt::OpaqueMode);

    switch (_params.ppi_main_legend_pos) {
      case Params::LEGEND_TOP_LEFT:
        _zoomWorld.drawLegendsTopLeft(painter, legends);
        break;
      case Params::LEGEND_TOP_RIGHT:
        _zoomWorld.drawLegendsTopRight(painter, legends);
        break;
      case Params::LEGEND_BOTTOM_LEFT:
        _zoomWorld.drawLegendsBottomLeft(painter, legends);
        break;
      case Params::LEGEND_BOTTOM_RIGHT:
        _zoomWorld.drawLegendsBottomRight(painter, legends);
        break;
      default: {}
    }

    // painter.setBrush(Qt::white);
    // painter.setBackgroundMode(Qt::TransparentMode);
    painter.restore();

  } // if (_archiveMode) {

}

///////////////////////////////////////////////////////////////////////////
// Draw text, with (X, Y) in screen space
//
// Flags give the justification in Qt, and are or'd from the following:
//    Qt::AlignLeft aligns to the left border.
//    Qt::AlignRight aligns to the right border.
//    Qt::AlignJustify produces justified text.
//    Qt::AlignHCenter aligns horizontally centered.
//    Qt::AlignTop aligns to the top border.
//    Qt::AlignBottom aligns to the bottom border.
//    Qt::AlignVCenter aligns vertically centered
//    Qt::AlignCenter (== Qt::AlignHCenter | Qt::AlignVCenter)
//    Qt::TextSingleLine ignores newline characters in the text.
//    Qt::TextExpandTabs expands tabs (see below)
//    Qt::TextShowMnemonic interprets "&x" as x; i.e., underlined.
//    Qt::TextWordWrap breaks the text to fit the rectangle.

// draw text in world coords

void PpiPlot::_drawScreenText(QPainter &painter, const string &text,
                              int text_x, int text_y,
                              int flags)
  
{

  int ixx = text_x;
  int iyy = text_y;
	
  QRect tRect(painter.fontMetrics().tightBoundingRect(text.c_str()));
  QRect bRect(painter.fontMetrics().
              boundingRect(ixx, iyy,
                           tRect.width() + 2, tRect.height() + 2,
                           flags, text.c_str()));
    
  painter.drawText(bRect, flags, text.c_str());
    
}

/*************************************************************************
 * numBeams()
 */

size_t PpiPlot::getNumBeams() const
{
  return _ppiBeams.size();
}

/*************************************************************************
 * _beamIndex()
 */

int PpiPlot::_beamIndex(const double start_angle,
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

void PpiPlot::_cullBeams(const PpiBeam *beamAB)
{
  // This routine examines the collection of beams, and removes those that are 
  // completely occluded by other beams. The algorithm gives precedence to the 
  // most recent beams; i.e. beams at the end of the _ppiBeams vector.
  //
  // Remember that there won't be any beams that cross angles through zero; 
  // otherwise the beam culling logic would be a real pain, and PpiPlot has
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

/*************************************************************************
 * refreshImages()
 */

void PpiPlot::refreshImages()
{

  cerr << "XXXXXXXXXXXXXXXXXXXXXX width, height: "
       << _fullWorld.getWidthPixels() << ", " 
       << _fullWorld.getHeightPixels() << endl;
  
  for (size_t ifield = 0; ifield < _fieldRenderers.size(); ++ifield) {
    
    FieldRenderer *field = _fieldRenderers[ifield];
    
    // If needed, create new image for this field

    QSize imageSize = field->getImage()->size();
    if (imageSize.width() != _fullWorld.getWidthPixels() ||
        imageSize.height() != _fullWorld.getHeightPixels()) {
      field->createImage(_fullWorld.getWidthPixels(), _fullWorld.getHeightPixels());
      cerr << "XXXXXXXXXXXXXXXXXXXXXX width, height: "
           << _fullWorld.getWidthPixels() << ", " 
           << _fullWorld.getHeightPixels() << endl;
    }

    // clear image

    field->getImage()->fill(_backgroundBrush.color().rgb());
    
    // set up rendering details

    field->setTransform(_zoomTransform);
    
    // Add pointers to the beams to be rendered
    
    if (ifield == _fieldNum || field->isBackgroundRendered()) {

      std::vector< PpiBeam* >::iterator beam;
      for (beam = _ppiBeams.begin(); beam != _ppiBeams.end(); ++beam) {
	(*beam)->setBeingRendered(ifield, true);
	field->addBeam(*beam);
      }
      
    }
    
  } // ifield
  
  // do the rendering

  _performRendering();

  _parent->update();


}

