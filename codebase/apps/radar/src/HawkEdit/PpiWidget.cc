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
#include "PpiWidget.hh"
#include "PolarManager.hh"
#include "ScriptEditorView.hh"
#include "ScriptEditorController.hh"
#include "SpreadSheetView.hh"
#include "SpreadSheetController.hh"
#include "ParameterColorView.hh"
#include "FieldColorController.hh"
#include "DisplayFieldModel.hh"
#include "BoundaryPointEditor.hh"

#include <toolsa/toolsa_macros.h>
#include <toolsa/LogStream.hh>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QErrorMessage>
#include <QApplication>

using namespace std;



PpiWidget::PpiWidget(QWidget* parent,
                     const PolarManager &manager,
                     const Params &params,
                     const RadxPlatform &platform,
                     //const vector<DisplayField *> &fields,
		     DisplayFieldController *displayFieldController,
                     bool haveFilteredFields) :
        PolarWidget(parent, manager, params, platform,
                    displayFieldController, // fields, 
		    haveFilteredFields)
        
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

PpiWidget::~PpiWidget()
{

  LOG(DEBUG) << "enter";
  // delete all of the dynamically created beams
  
  for (size_t i = 0; i < _ppiBeams.size(); ++i) {
    Beam::deleteIfUnused(_ppiBeams[i]);
  }
  LOG(DEBUG) << "_ppiBeams.clear()";
  _ppiBeams.clear();
  LOG(DEBUG) << "exit";
  //  delete _ppiBeamController;
}

/*************************************************************************
 * clear()
 */

void PpiWidget::clear()
{
  LOG(DEBUG) << "enter";
  // Clear out the beam array
  LOG(DEBUG) << "NOT CLEARING BEAMS";
  //  for (size_t i = 0; i < _ppiBeams.size(); i++) {
  //    Beam::deleteIfUnused(_ppiBeams[i]);
  //  }
  //LOG(DEBUG) << "_ppiBeams.clear()";
  //  _ppiBeams.clear();
  
  //_ppiBeamController->clear();

  // Now rerender the images
  
  //_refreshImages();

  LOG(DEBUG) << "exit";
}

/*************************************************************************
 * selectVar()
 */

void PpiWidget::selectVar(const size_t index)
{
  LOG(DEBUG) << "enter, index = " << index;

  // If the field index isn't actually changing, we don't need to do anything
  size_t selectedField = displayFieldController->getSelectedFieldNum();
  //  if (selectedField == index) {
  //  return;
  //}
  
    string fieldName = displayFieldController->getField(index)->getName();
    LOG(DEBUG) << "=========>> PpiWidget::selectVar() for field index: " 
         << index << " for field " << fieldName;

  // If this field isn't being rendered in the background, render all of
  // the beams for it
  
  //if (!_fieldRendererController->isBackgroundRendered(index)) {
  //  LOG(DEBUG) << "isBackgroundRendered is FALSE";
    //LOG(DEBUG) << "number of beams in _ppiBeams = " << _ppiBeams.size();
    //     LOG(DEBUG) << "number of _fieldRenderers = " << _fieldRenderers.size();
    //LOG(DEBUG) << " and here is the 1st beam ...";
    //_ppiBeams[0]->print(cout);
    // 
    // go through each beam, set render flag to true, add it to the
    //    list of beam for this field

    // move this to 
    // give the list of beams to the fieldRendererController, along with
    // the field index, let the fieldRendererController set the status
    /*
    std::vector< PpiBeam* >::iterator beam;
    for (beam = _ppiBeams.begin(); beam != _ppiBeams.end(); ++beam) {
      (*beam)->setBeingRendered(index, true);
      _fieldRendererController->addBeam(index, (*beam));
      //      _fieldRenderers[index]->addBeam(*beam);
    }
    */
    //_fieldRendererController->setBackgroundRenderingOn(index);
    //}
  
  //HERE does performRendering interact with select and unselect 
  //  LOG(DEBUG) << "performRendering is next ...";


  // Do any needed housekeeping when the field selection is changed
  //_fieldRendererController->unselectField(selectedField);
  //_fieldRendererController->selectField(index);
  //  _fieldRenderers[_selectedField]->unselectField();
  //  _fieldRenderers[index]->selectField();
  
  // Change the selected field index
  // _selectedField = index;
  //displayFieldController->setSelectedField(index);

  _fieldRendererController->performRendering(index);

  // Update the display

  update();
  LOG(DEBUG) << "exit";
}


/*************************************************************************
 * updateVars()
 

// TODO: maybe just update the beam that changed?
void PpiWidget::updateVars()
{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=========>> PpiWidget::updateVars()" <<  endl;
  }

  // TODO: see, it would be nice to render only the beam ???
  // If this field isn't being rendered in the background, render all of
  // the beams for it

  // TODO: clear the Var first?

  // for each field in beam

  if (!_fieldRenderers[index]->isBackgroundRendered()) {
    std::vector< PpiBeam* >::iterator beam;
    for (beam = _ppiBeams.begin(); beam != _ppiBeams.end(); ++beam) {
      (*beam)->setBeingRendered(index, true);
      _fieldRenderers[index]->addBeam(*beam);
    }
  }
  _performRendering();

  // Update the display

  update();
}
*/

/*************************************************************************
 * clearVar()
 */

void PpiWidget::clearVar(const size_t index)
{

  if (index >= displayFieldController->getNFields()) { // _fields.size()) {
    return;
  }

  // Set the brush for every beam/gate for this field to use the background
  // color

  std::vector< PpiBeam* >::iterator beam;
  for (beam = _ppiBeams.begin(); beam != _ppiBeams.end(); ++beam) {
    (*beam)->resetFieldBrush(index, &_backgroundBrush);
  }
  
  size_t selectedField = displayFieldController->getSelectedFieldNum();
  if (index == selectedField) {
    update();
  }

}

// always return one or at most two beams. 
void PpiWidget::addCullTrackBeam(const RadxRay *ray,
                        const float start_angle,
                        const float stop_angle,
                        const std::vector< std::vector< double > > &beam_data,
			size_t nFields)
{

  LOG(DEBUG) << "enter";

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
    PpiBeam* b = new PpiBeam(_params, ray, nFields, 
                             n_start_angle, n_stop_angle);
    b->addClient(); // add tracking information
    _cullBeams(b);
    _ppiBeams.push_back(b);
    //_accumulateStats(b);
    //    newBeams.push_back(b);
    b->fillColors(beam_data, displayFieldController, nFields, &_backgroundBrush);
    // Add the new beams to the render lists for each of the fields
    _fieldRendererController->addBeam(b); // selectedField, beam);

  } else {

    // The beam crosses the 0 degree angle.  First add the portion of the
    // beam to the left of the 0 degree point.

    PpiBeam* b1 = new PpiBeam(_params, ray, nFields, n_start_angle, 360.0);
    b1->addClient();
    _cullBeams(b1);
    _ppiBeams.push_back(b1);
    // newBeams.push_back(b1);
    //_accumulateStats(b1);
    //    newBeams.push_back(b);
    b1->fillColors(beam_data, displayFieldController, nFields, &_backgroundBrush);
    // Add the new beams to the render lists for each of the fields
    _fieldRendererController->addBeam(b1); // selectedField, beam);

    // Now add the portion of the beam to the right of the 0 degree point.

    PpiBeam* b2 = new PpiBeam(_params, ray, nFields, 0.0, n_stop_angle);
    b2->addClient();
    _cullBeams(b2);
    _ppiBeams.push_back(b2);
    //newBeams.push_back(b2);
    //_accumulateStats(b2);
    //    newBeams.push_back(b);
    b2->fillColors(beam_data, displayFieldController, nFields, &_backgroundBrush);
    // Add the new beams to the render lists for each of the fields
    _fieldRendererController->addBeam(b2); // selectedField, beam);
  }
  _accumulateStats(ray);
    
}

void PpiWidget::_accumulateStats(const RadxRay *ray) {

  // compute angles and times in archive mode
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
    
}

/*************************************************************************
 * addBeam()
 */

void PpiWidget::addBeam(const RadxRay *ray,
                        const float start_angle,
                        const float stop_angle,
                        const std::vector< std::vector< double > > &beam_data,
			size_t nFields)
			//                        const std::vector< DisplayField* > &fields)
			//displayFieldController)
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


    // TODO: how to integrate this ....
    //  _beamController->addBeam(ray, start_angle, stop_angle, beam_data, displayFieldController);
    PpiBeam* b = new PpiBeam(_params, ray, nFields, 
                             n_start_angle, n_stop_angle);
	b->addClient(); // TODO: register a callback with the beamController 
    _cullBeams(b);
    _ppiBeams.push_back(b);
    newBeams.push_back(b);

  } else {

    // The beam crosses the 0 degree angle.  First add the portion of the
    // beam to the left of the 0 degree point.

    PpiBeam* b1 = new PpiBeam(_params, ray, nFields, n_start_angle, 360.0);
    b1->addClient();
    _cullBeams(b1);
    _ppiBeams.push_back(b1);
    newBeams.push_back(b1);

    // Now add the portion of the beam to the right of the 0 degree point.

    PpiBeam* b2 = new PpiBeam(_params, ray, nFields, 0.0, n_stop_angle);
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

  //size_t selectedField = displayFieldController->getSelectedFieldNum();

  for (size_t ii = 0; ii < newBeams.size(); ii++) {

    PpiBeam *beam = newBeams[ii];
    
    // Set up the brushes for all of the fields in this beam.  This can be
    // done independently of a Painter object.
    // Just send the number of new fields
    beam->fillColors(beam_data, displayFieldController, nFields, &_backgroundBrush);

    // Add the new beams to the render lists for each of the fields
    _fieldRendererController->addBeam(beam); // selectedField, beam);
    //_fieldRendererController->addBeamToBackgroundRenderedFields(beam);

    /*
    for (size_t field = 0; field < _fieldRenderers.size(); ++field) {
      if (field == _selectedField ||
          _fieldRenderers[field]->isBackgroundRendered()) {
        _fieldRenderers[field]->addBeam(beam);
      } else {
        beam->setBeingRendered(field, false);
      }
    }
    */
  } // endfor - beam 

  // Start the threads to render the new beams

  // _performRendering();

  LOG(DEBUG) << "exit";
}

// Update this beam with the new fields
// Precondition: displayFieldController must already be updated with new fields
// Precondition: fieldRendererController must already be updated with new fields
// TODO: call from PolarManager::updateVolume ===> PolarManager::updateArchiveData
// nFields  total number of Fields (old + new)
// newFieldNames 
// beam_data is [nFields][nGates]
void PpiWidget::updateBeamII(const RadxRay *ray,
                        const float start_angle,
                        const float stop_angle,
                        const std::vector< std::vector< double > > &beam_data,
			     size_t nFields,
			const vector<string> &fieldNames)
{

  LOG(DEBUG) << "enter";
  
  LOG(DEBUG) << "beam_data size = " << beam_data.size();
  // << " by " << beam_data[0].size();
  LOG(DEBUG) << "start_angle = " << start_angle;
  LOG(DEBUG) << "stop_angle = " << stop_angle;

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
  
  // --------

  if (n_start_angle <= n_stop_angle) {

    // This beam does not cross the 0 degree angle.  Just add the beam to
    // the beam list.
    PpiBeam* b = new PpiBeam(_params, ray, nFields, 
                             n_start_angle, n_stop_angle);
    b->addClient(); // add tracking information
    _cullBeams(b);
    _ppiBeams.push_back(b);
    //    accumulateStats(b);
    movingDownTheLine(b, fieldNames, beam_data, nFields);
    //b->fillColors(beam_data, displayFieldController, nFields, &_backgroundBrush);
    // Add the new beams to the render lists for each of the fields
    //_fieldRendererController->addBeam(b, newFields); // selectedField, beam);

  } else {

    // The beam crosses the 0 degree angle.  First add the portion of the
    // beam to the left of the 0 degree point.

    PpiBeam* b1 = new PpiBeam(_params, ray, nFields, n_start_angle, 360.0);
    b1->addClient();
    _cullBeams(b1);
    _ppiBeams.push_back(b1);
    //accumulateStats(b1);
    movingDownTheLine(b1, fieldNames, beam_data, nFields);
    //b1->fillColors(beam_data, displayFieldController, nFields, &_backgroundBrush);
    // Add the new beams to the render lists for each of the fields
    //_fieldRendererController->addBeam(b1, newFields); // selectedField, beam);

    // Now add the portion of the beam to the right of the 0 degree point.

    PpiBeam* b2 = new PpiBeam(_params, ray, nFields, 0.0, n_stop_angle);
    b2->addClient();
    _cullBeams(b2);
    _ppiBeams.push_back(b2);
    //accumulateStats(b2);
    movingDownTheLine(b2, fieldNames, beam_data, nFields);
    //b2->FillColors(beam_data, displayFieldController, nFields, &_backgroundBrush);
    // Add the new beams to the render lists for each of the fields
    //_fieldRendererController->addBeam(b2, newFields); // selectedField, beam);
  }
}


// nFields = total number of Fields (old + new)
// fill brushes and queue this beam with the renderers
void PpiWidget::movingDownTheLine(PpiBeam *beam, vector<string> fieldNames,
				  const std::vector< std::vector< double > > &beam_data,
				  size_t nFields) {
  //---------
  //size_t nFields = beam_data.size();
    // Set up the brushes for all of the fields in this beam.  This can be
    // done independently of a Painter object.
    // TODO: Just send the number of fields
  // fill colors becomes a sparse array
  vector<string>::iterator it;
  size_t newFieldIndex = 0;
  for (it = fieldNames.begin(); it < fieldNames.end(); ++it) {
    size_t displayFieldIdx = displayFieldController->getFieldIndex(*it); // _lookup(*it);
    if (displayFieldIdx > nFields)
      throw "Error: fieldIdx is outside dimensions (movingDownTheLine)"; 
    beam->updateFillColorsSparse(beam_data[newFieldIndex], displayFieldController, nFields, &_backgroundBrush, displayFieldIdx);
    // just add beam to the new fields 
    _fieldRendererController->addBeam(displayFieldIdx, beam);
    newFieldIndex += 1;
  }
  // Start the threads to render the new beams
  // _performRendering();

  LOG(DEBUG) << "exit";
}


void PpiWidget::updateBeamColors(//const RadxRay *ray,
				 //const float start_angle,
				 //const float stop_angle,
				 //const std::vector< double > &beam_data,
				 size_t nFields,
				 const string fieldName,
				 size_t nGates)
{

  //LOG(DEBUG) << "enter";

  //LOG(DEBUG) << "_ppiBeams.size() = " << _ppiBeams.size();  

  size_t displayFieldIdx = displayFieldController->getFieldIndex(fieldName);
  if (displayFieldIdx > nFields)
    throw "Error: fieldIdx is outside dimensions (updateColorsOnFields)";
  const ColorMap *map = displayFieldController->getColorMap(displayFieldIdx);


  // WAIT A MINUTE! Hold everything.  A beam holds a pointer to the associated ray.
  vector<PpiBeam *>::iterator it;
  for (it = _ppiBeams.begin(); it != _ppiBeams.end(); ++it) {
    PpiBeam* b = *it;
    const RadxRay *ray = b->getRay();
    const RadxField *rfld = ray->getField(fieldName);             
    if (rfld == NULL)          
      throw "field data not found for ray";
    //rfld->convertToFl32();
    const Radx::fl32 *fdata = rfld->getDataFl32(); 

    // Q: What if the number of data values != the dimension of the brushes
    //    const std::vector<double> &beam_data = ray->getFieldFl32();
    updateColorsOnFields(b, fieldName, nFields, fdata, nGates, map, displayFieldIdx);
  }

}


// nFields = total number of Fields (old + new)
// fill brushes and queue this beam with the renderers
  void PpiWidget::updateColorsOnFields(PpiBeam *beam,
				       string fieldName, size_t nFields,
				       const Radx::fl32 *beam_data,
				       size_t nGates,
				       const ColorMap *map,
				       size_t displayFieldIdx) {
  //---------
  //size_t nFields = beam_data.size();
    // Set up the brushes for all of the fields in this beam.  This can be
    // done independently of a Painter object.
    // TODO: Just send the number of fields
  // fill colors becomes a sparse array
    //  size_t displayFieldIdx = displayFieldController->getFieldIndex(fieldName);
    //if (displayFieldIdx > nFields)
    //  throw "Error: fieldIdx is outside dimensions (updateColorsOnFields)";
    //const ColorMap *map = displayFieldController->getColorMap(displayFieldIdx);
  beam->updateFillColors(beam_data, nGates, // displayFieldController, 
			 displayFieldIdx,
			 nFields,
			 map, &_backgroundBrush);
  // just add beam to the new fields 
  _fieldRendererController->addBeam(displayFieldIdx, beam);

  //LOG(DEBUG) << "exit";
}


/*
// A field has a new color map; update the fill colors for the field in each beam.
// nFields = total number of Fields (old + new)
// fill brushes and queue this beam with the renderers
// Q: Who holds the beams? FieldRenderer
void PpiWidget::colorMapChanged(string fieldName) {

  //----
  //for each beam, b
  // get beam_data
  b->fillColors(beam_data, displayFieldController, fieldIdx, &_backgroundBrush);
  // Add the new beams to the render lists for each of the fields                                                             
  _fieldRendererController->addBeam(b); // selectedField, beam);
  //-----
  // call FieldRendererController to update the ColorMap for this field
  // TODO: send beam data? because it is vetted? 
  //_fieldRendererController->colorMapChanged(fieldName, beam_data, &_backgroundBrush); // or displayFieldIdx);
  // inside FieldRenderer, loop is ...
  // for each beam
  //    

  // fill colors becomes a sparse array
  //  size_t displayFieldIdx = displayFieldController->getFieldIndex(*it); // _lookup(*it);
  //  if (displayFieldIdx > nFields)
  //    throw "Error: fieldIdx is outside dimensions (movingDownTheLine)"; 
  // beam->updateFillColorsSparse(beam_data[newFieldIndex], displayFieldController, nFields, &_backgroundBrush, displayFieldIdx);
    // just add beam to the new fields 

  LOG(DEBUG) << "exit";
}
*/

// TODO: does this become addFieldsToEachBeam? YES
// Update this beam with the new fields
// UpdateBeam only works on new beams, which are to be added to the end
// of any existing lists.
// Precondition: displayFieldController must already be updated with new fields
// Precondition: fieldRendererController must already be updated with new fields
// TODO: call from PolarManager::updateVolume ===> PolarManager::updateArchiveData
// nFields  number of new fields 
void PpiWidget::updateBeam(const RadxRay *ray,
                        const float start_angle,
                        const float stop_angle,
                        const std::vector< std::vector< double > > &beam_data,
			   //		   vector<string> &newFieldNames)
			   size_t nFields)
			//                        const std::vector< DisplayField* > &fields)
			//displayFieldController)
{
  /*
  LOG(DEBUG) << "enter";

  LOG(DEBUG) << "beam_data size = " << beam_data.size() << " by " << beam_data[0].size();
  LOG(DEBUG) << "start_angle = " << start_angle;
  LOG(DEBUG) << "stop_angle = " << stop_angle;

  // find the associated beam
  // update it 

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
  

  size_t nFields_expected = 0;
  if (_ppiBeams.size() > 0)
    _ppiBeams.at(0)->getNFields();

  if (n_start_angle <= n_stop_angle) {

    // This beam does not cross the 0 degree angle.  Just add the beam to
    // the beam list.


    // TODO: how to integrate this ....
    //  _beamController->addBeam(ray, start_angle, stop_angle, beam_data, displayFieldController);
    // find an existing beam
    size_t index = _beamIndex(n_start_angle, n_stop_angle);
    // NOTE: We only want to add Beams, because finding the beamIndex finds the closest beam
    // in the current list, but the list may not exist or may not contain all the beams.
    LOG(DEBUG) << "found closest beam at index " << index;
    PpiBeam *b = _ppiBeams.at(index);
    try {
    b->addFields(ray, nFields, nFields_expected); // newFieldNames.size());
    //PpiBeam* b = new PpiBeam(_params, ray, nFields, 
    //                         n_start_angle, n_stop_angle);
    b->addClient();  
    //_cullBeams(b);
    //_ppiBeams.push_back(b); // handled by ppiBeamController?
    newBeams.push_back(b);
    } catch (const char *ex) {
      LOG(DEBUG) << "Exception: beam index=" << index << "start_angle, stop_angle = "
		 << start_angle << "," << stop_angle;
      //throw ex;
    }

  } else {

    // The beam crosses the 0 degree angle.  First add the portion of the
    // beam to the left of the 0 degree point.

    size_t index = _beamIndex(n_start_angle, n_stop_angle);
    PpiBeam *b1 = _ppiBeams.at(index);
    try {
    b1->addFields(ray, nFields, nFields_expected); // newFieldNames.size()); // nFields);
    //    PpiBeam* b1 = new PpiBeam(_params, ray, nFields, n_start_angle, 360.0);

    b1->addClient();
    //_cullBeams(b1);
    //_ppiBeams.push_back(b1);
    newBeams.push_back(b1);
    } catch (const char *ex) {
      LOG(DEBUG) << "Exception: b1: beam index=" << index << "start_angle, stop_angle = "
		 << start_angle << "," << stop_angle;
      //throw ex;
    }

    // Now add the portion of the beam to the right of the 0 degree point.

    index = _beamIndex(n_start_angle, n_stop_angle);
    PpiBeam *b2 = _ppiBeams.at(index);
    try {
    b2->addFields(ray, nFields, nFields_expected); // newFieldNames.size());
    //PpiBeam* b2 = new PpiBeam(_params, ray, nFields, 0.0, n_stop_angle);

    b2->addClient();
    //_cullBeams(b2);
    //_ppiBeams.push_back(b2);
    newBeams.push_back(b2);
    } catch (const char *ex) {
      LOG(DEBUG) << "Exception: b2: beam index=" << index << "start_angle, stop_angle = "
		 << start_angle << "," << stop_angle;
      //throw ex;
    }

  }
  
  // newBeams has pointers to the updated beams.  Mark the beams to be rendered.
  //   (There are at most 2 new beams).

  //size_t selectedFieldIndex = displayFieldController->getSelectedFieldNum();
  for (size_t ii = 0; ii < newBeams.size(); ii++) {

    PpiBeam *beam = newBeams[ii];
    
    // Set up the brushes for all of the fields in this beam.  This can be
    // done independently of a Painter object.
    // TODO: Just send the number of fields
    beam->updateFillColors(beam_data, displayFieldController, nFields, &_backgroundBrush);

    // for each newFieldName
    // queue beam for rendering this field

    // TODO: ask Mike about this ...
    // add the new beam to the selected field, and to all the
    // fields that are background rendered?  
    // TODO: just add beam to the new fields 
    _fieldRendererController->addBeam(beam); // selectedFieldIndex, beam);
    //_fieldRendererController->addBeamToBackgroundRenderedFields(beam);

  } // endfor - beam 

  // Start the threads to render the new beams

  // _performRendering();

  LOG(DEBUG) << "exit";
  */
}


/*************************************************************************
 * configureRange()
 */

void PpiWidget::configureRange(double max_range)
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

void PpiWidget::timerEvent(QTimerEvent *event)
{
	bool doUpdate = false;
	bool isBoundaryEditorVisible = _manager._boundaryEditorDialog->isVisible();
	if (isBoundaryEditorVisible)
	{
		double xRange = _zoomWorld.getXMaxWorld() - _zoomWorld.getXMinWorld();
		doUpdate = BoundaryPointEditor::Instance()->updateScale(xRange);
	}
  bool isBoundaryFinished = BoundaryPointEditor::Instance()->isPolygonFinished();
  bool isShiftKeyDown = (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) == true);
  if ((isBoundaryEditorVisible && !isBoundaryFinished) || (isBoundaryEditorVisible && isBoundaryFinished && isShiftKeyDown))
    this->setCursor(Qt::CrossCursor);
  else
    this->setCursor(Qt::ArrowCursor);

  if (doUpdate)
  	update();
}


/*************************************************************************
 * mouseReleaseEvent()
 */
void PpiWidget::mouseReleaseEvent(QMouseEvent *e)
{

  _pointClicked = false;

  QRect rgeom = _rubberBand->geometry();

  // If the mouse hasn't moved much, assume we are clicking rather than
  // zooming

  QPointF clickPos(e->pos());
  
  _mouseReleaseX = clickPos.x();
  _mouseReleaseY = clickPos.y();

  // get click location in world coords

  if (rgeom.width() <= 20)
  {

	// Emit a signal to indicate that the click location has changed
    _worldReleaseX = _zoomWorld.getXWorld(_mouseReleaseX);
    _worldReleaseY = _zoomWorld.getYWorld(_mouseReleaseY);

    if (_manager._boundaryEditorDialog->isVisible())
    {
    	if (!BoundaryPointEditor::Instance()->isPolygonFinished())
    		BoundaryPointEditor::Instance()->addPoint(_worldReleaseX, _worldReleaseY);
    	else  //polygon finished, user may want to insert/delete a point
    	{
    		bool isOverExistingPt = BoundaryPointEditor::Instance()->isOverAnyPoint(_worldReleaseX, _worldReleaseY);
    		bool isShiftKeyDown = (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) == true);
    		if (isShiftKeyDown)
    		{
    			if (isOverExistingPt)
        			BoundaryPointEditor::Instance()->delNearestPoint(_worldReleaseX, _worldReleaseY);
        		else
        			BoundaryPointEditor::Instance()->insertPoint(_worldReleaseX, _worldReleaseY);
    		}
    	}
    }

    double x_km = _worldReleaseX;
    double y_km = _worldReleaseY;
    _pointClicked = true;

    // get ray closest to click point

    const RadxRay *closestRay = _getClosestRay(x_km, y_km);

    // emit signal

    emit locationClicked(x_km, y_km, closestRay);
  
  }
  else
  {
	  cerr << "Jeff: Zoom occurred" << endl;

    // mouse moved more than 20 pixels, so a zoom occurred
    
    _worldPressX = _zoomWorld.getXWorld(_mousePressX);
    _worldPressY = _zoomWorld.getYWorld(_mousePressY);

    _worldReleaseX = _zoomWorld.getXWorld(_zoomCornerX);
    _worldReleaseY = _zoomWorld.getYWorld(_zoomCornerY);

    _zoomWorld.set(_worldPressX, _worldPressY, _worldReleaseX, _worldReleaseY);

    _setTransform(_zoomWorld.getTransform());

    _setGridSpacing();

    // enable unzoom button
    
    _manager.enableZoomButton();
    
    // Update the window in the renderers
    
    _refreshImages();

  }
    
  // hide the rubber band

  _rubberBand->hide();
  update();

}


////////////////////////////////////////////////////////////////////////////
// get ray closest to click point

const RadxRay *PpiWidget::_getClosestRay(double x_km, double y_km)

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
  // _ppiBeams may be empty at this point, so get the closest ray from the 
  // RadxVol itself.
  // 
  LOG(DEBUG) << "_ppiBeams.size() = " << _ppiBeams.size();
  for (size_t ii = 0; ii < _ppiBeams.size(); ii++) {
    const RadxRay *ray = _ppiBeams[ii]->getRay();
    double rayAz = ray->getAzimuthDeg();
    // LOG(DEBUG) << "rayAz = " << rayAz;
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

////////////////////////////////////////////////////////////////////////////
// get azimuth closest to click point

double PpiWidget::_getClosestAz(double x_km, double y_km)

{

  double clickAz = atan2(y_km, x_km) * RAD_TO_DEG;
  double radarDisplayAz = 90.0 - clickAz;
  if (radarDisplayAz < 0.0) radarDisplayAz += 360.0;
  LOG(DEBUG) << "clickAz = " << clickAz << " from x_km, y_km = " 
                          << x_km << "," << y_km; 
  LOG(DEBUG) << "radarDisplayAz = " << radarDisplayAz << " from x_km, y_km = "
             << x_km << y_km;

  return radarDisplayAz;

}


/*************************************************************************
 * _setGridSpacing()
 */

void PpiWidget::_setGridSpacing()
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


void PpiWidget::_drawOverlays(QPainter &painter)
{

  LOG(DEBUG) << "enter";

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

      _zoomWorld.drawLine(painter, ringRange, -_maxRangeKm, ringRange, _maxRangeKm);
      _zoomWorld.drawLine(painter, -ringRange, -_maxRangeKm, -ringRange, _maxRangeKm);
      _zoomWorld.drawLine(painter, -_maxRangeKm, ringRange, _maxRangeKm, ringRange);
      _zoomWorld.drawLine(painter, -_maxRangeKm, -ringRange, _maxRangeKm, -ringRange);
      
      maxRingRange = ringRange;
      ringRange += _ringSpacing;
    }
    painter.restore();

    _zoomWorld.setSpecifyTicks(true, -maxRingRange, _ringSpacing);

    _zoomWorld.drawAxisLeft(painter, "km", true, true, true);
    _zoomWorld.drawAxisRight(painter, "km", true, true, true);
    _zoomWorld.drawAxisTop(painter, "km", true, true, true);
    _zoomWorld.drawAxisBottom(painter, "km", true, true, true);
    
    _zoomWorld.setSpecifyTicks(false);

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
  
  if (_pointClicked) {

    int startX = _mouseReleaseX - _params.click_cross_size / 2;
    int endX = _mouseReleaseX + _params.click_cross_size / 2;
    int startY = _mouseReleaseY - _params.click_cross_size / 2;
    int endY = _mouseReleaseY + _params.click_cross_size / 2;

    painter.drawLine(startX, _mouseReleaseY, endX, _mouseReleaseY);
    painter.drawLine(_mouseReleaseX, startY, _mouseReleaseX, endY);

  }

  // reset painter state
  
  painter.restore();

  // draw the color scale

  DisplayField *field = displayFieldController->getSelectedField();
  _zoomWorld.drawColorScale(field->getColorMap(), painter,
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
    size_t selectedField = displayFieldController->getSelectedFieldNum();
    FieldRenderer *selectedFieldRenderer = _fieldRendererController->get(selectedField);
    string fieldName = selectedFieldRenderer->getField().getLabel();
    //string fieldName = _fieldRenderers[_selectedField]->getField().getLabel();
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

  LOG(DEBUG) << "exit";

}

void PpiWidget::drawColorScaleLegend() {

  QPainter painter(this);
  // draw the color scale

  DisplayField *field = displayFieldController->getSelectedField();
  _zoomWorld.drawColorScale(field->getColorMap(), painter,
                            _params.label_font_size);

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

void PpiWidget::_drawScreenText(QPainter &painter, const string &text,
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

size_t PpiWidget::getNumBeams() const
{
  return _ppiBeams.size();
}

/*************************************************************************
 * _beamIndex()
 */

int PpiWidget::_beamIndex(const double start_angle,
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

void PpiWidget::_cullBeams(const PpiBeam *beamAB)
{
  // This routine examines the collection of beams, and removes those that are 
  // completely occluded by other beams. The algorithm gives precedence to the 
  // most recent beams; i.e. beams at the end of the _ppiBeams vector.
  //
  // Remember that there won't be any beams that cross angles through zero; 
  // otherwise the beam culling logic would be a real pain, and PpiWidget has
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
 * _refreshImages()
 */

void PpiWidget::_refreshImages()
{
  /*
  for (size_t ifield = 0; ifield < _fieldRenderers.size(); ++ifield) {
    
    FieldRenderer *field = _fieldRenderers[ifield];
    
    // If needed, create new image for this field
    
    if (size() != field->getImage()->size()) {
      field->createImage(width(), height());
    }

    // clear image

    field->getImage()->fill(_backgroundBrush.color().rgb());
    
    // set up rendering details

    field->setTransform(_zoomTransform);
    
    // Add pointers to the beams to be rendered
    
    if (ifield == _selectedField || field->isBackgroundRendered()) {

      std::vector< PpiBeam* >::iterator beam;
      for (beam = _ppiBeams.begin(); beam != _ppiBeams.end(); ++beam) {
	(*beam)->setBeingRendered(ifield, true);
	field->addBeam(*beam);
      }
      
    }
    
  } // ifield
  */  
  // do the rendering
  size_t selectedField = displayFieldController->getSelectedFieldNum();

  _fieldRendererController->refreshImages(width(), height(), size(),
					  _backgroundBrush.color().rgb(),
					  _zoomTransform, 
					  selectedField, 
					  _ppiBeams);
  // _performRendering();

  update();
}

void PpiWidget::contextMenuParameterColors()
{
  
  LOG(DEBUG) << "enter";

  //DisplayField selectedField;                                                                             

  // const DisplayField &field = _manager.getSelectedField();
  // const ColorMap &colorMapForSelectedField = field.getColorMap();
  ParameterColorView *parameterColorView = new ParameterColorView(this);
  // vector<DisplayField *> displayFields = displayFieldController->getDisplayFields(); // TODO: I guess, implement this as a signal and a slot? // getDisplayFields();
  DisplayField *selectedField = displayFieldController->getSelectedField();
  string emphasis_color = "white";
  string annotation_color = "white";
  // This is already created ...
  /*
  DisplayFieldModel *displayFieldModel = 
    new DisplayFieldModel(displayFields, selectedField->getName(),
			  _params.grid_and_range_ring_color,
			  emphasis_color,
			  annotation_color,
			  _params.background_color);
  */
  DisplayFieldModel *displayFieldModel = displayFieldController->getModel();

  FieldColorController *fieldColorController = new FieldColorController(parameterColorView, displayFieldModel);
  // connect some signals and slots in order to retrieve information
  // and send changes back to display
                                                                         
  //  connect(parameterColorView, SIGNAL(retrieveInfo), &_manager, SLOT(InfoRetrieved()));
  connect(fieldColorController, SIGNAL(colorMapRedefineSent(string, ColorMap, QColor, QColor, QColor, QColor)),
  	  &_manager, SLOT(colorMapRedefineReceived(string, ColorMap, QColor, QColor, QColor, QColor))); // THIS IS NOT CALLED!!
  //  PolarManager::colorMapRedefineReceived(string, ColorMap)
  //connect(fieldColorController, SIGNAL(colorMapRedefined(string)),
  //	  this, SLOT(changeToDisplayField(string))); // THIS IS NOT CALLED!!

  /* TODO: combine with replot
  connect(fieldColorController, SIGNAL(backgroundColorSet(QColor)),
  	  this, SLOT(backgroundColor(QColor)));
  */

  fieldColorController->startUp(); 

  //connect(parameterColorView, SIGNAL(needFieldNames()), this, SLOT(getFieldNames()));
  //connect(this, SIGNAL(fieldNamesSupplied(vector<string>)), 
  //  parameterColorView, SLOT(fieldNamesSupplied(vector<string>));
  // TODO: move this call to the controller?                                                                
	  // parameterColorView.exec();

  //  if(parameterColorController.Changes()) {
    // TODO: what are changes?  new displayField(s)?                                                        
  //}
 
  LOG(DEBUG) << "exit ";
  
}

/*
void PpiWidget::sillyReceived() {
  LOG(DEBUG) << "enter";
  LOG(DEBUG) << "exit";
}
*/
/*
void PpiWidget::changeToDisplayField(string fieldName)  // , ColorMap newColorMap) {
{
  LOG(DEBUG) << "enter";
  // connect the new color map with the field                                                                    
  // find the fieldName in the list of FieldDisplays                                                             
  
  bool found = false;
  vector<DisplayField *>::iterator it;
  int fieldId = 0;

  it = _fields.begin();
  while ( it != _fields.end() && !found ) {
    DisplayField *field = *it;

    string name = field->getName();
    if (name.compare(fieldName) == 0) {
      found = true;
      field->replaceColorMap(newColorMap);
    }
    fieldId++;
    it++;
  }
  if (!found) {
    LOG(ERROR) << fieldName;
    LOG(ERROR) << "ERROR - field not found; no color map change";
    // TODO: present error message box                                                                           
  } else {
    // look up the fieldId from the fieldName                                                                    
    // change the field variable                                                                                 
    _changeField(fieldId, true);
  }
  
  LOG(DEBUG) << "exit";
}
*/


 
void PpiWidget::ExamineEdit(double azimuth, double elevation, size_t fieldIndex) {   

  // get an version of the ray that we can edit
  // we'll need the az, and sweep number to get a list from
  // the volume

  LOG(DEBUG) << "azimuth=" << azimuth << ", elevation=" << elevation << ", fieldIndex=" << fieldIndex;
  vector<RadxRay *> rays = _vol->getRays();
  // find that ray
  bool foundIt = false;
  double minDiff = 1.0e99;
  double delta = 0.01;
  RadxRay *closestRayToEdit = NULL;
  vector<RadxRay *>::iterator r;
  r=rays.begin();
  int idx = 0;
  while(r<rays.end()) {
    RadxRay *rayr = *r;
    double diff = fabs(azimuth - rayr->getAzimuthDeg());
    if (diff > 180.0) {
      diff = fabs(diff - 360.0);
    }
    if (diff < minDiff) {
      if (abs(elevation - rayr->getElevationDeg()) <= delta) {
        foundIt = true;
        closestRayToEdit = *r;
        minDiff = diff;
      }
    }
    r += 1;
    idx += 1;
  }  
  if (!foundIt || closestRayToEdit == NULL) {
    //throw "couldn't find closest ray";
    errorMessage("ExamineEdit Error", "couldn't find closest ray");
    return;
  }

  LOG(DEBUG) << "Found closest ray: index = " << idx << " pointer = " << closestRayToEdit;
  closestRayToEdit->print(cout); 


  // create the view
  SpreadSheetView *sheetView;
  sheetView = new SpreadSheetView(this, closestRayToEdit->getAzimuthDeg());

  // create the model

  // SpreadSheetModel *model = new SpreadSheetModel(closestRayCopy);
  SpreadSheetModel *model = new SpreadSheetModel(closestRayToEdit, _vol);
  //SpreadSheetModel *model = new SpreadSheetModel(closestRay, _vol);
  
  // create the controller
  SpreadSheetController *sheetControl = new SpreadSheetController(sheetView, model);

  // finish the other connections ..
  //sheetView->addController(sheetController);
  // model->setController(sheetController);

  // connect some signals and slots in order to retrieve information
  // and send changes back to display
                                                                         
  connect(sheetControl, SIGNAL(volumeChanged()),
  	  &_manager, SLOT(setVolume()));
  
  sheetView->init();
  sheetView->show();
  // sheetView->layout()->setSizeConstraint(QLayout::SetFixedSize);
  
}

void PpiWidget::contextMenuEditor()
{
  LOG(DEBUG) << "enter";

  // get click location in world coords
  // by using the location stored in class variables
  double x_km = _worldPressX;
  double y_km = _worldPressY;

  // get azimuth closest to click point
  double  closestAz = _getClosestAz(x_km, y_km);
  // TODO: make sure the point is in the valid area
  //if (closestRay == NULL) {
    // report error
  //  QMessageBox::information(this, QString::fromStdString(""), QString::fromStdString("No ray found at location clicked"));
    // TODO: move to this ...  errorMessage("", "No ray found at location clicked");
  //} else {
  try {
  PolarManager *polarManager = PolarManager::Instance();
  if (polarManager != NULL) {
    double elevation = polarManager->getSelectedSweepAngle();
    size_t fieldIdx = polarManager->getSelectedFieldIndex();
    LOG(DEBUG) << "elevation=" << elevation << ", fieldIdx=" << fieldIdx;
    ExamineEdit(closestAz, elevation, fieldIdx);
  } else {
    LOG(DEBUG) << "polarManager is NULL";
  }
  } catch (const string& ex) {
    errorMessage("ExamineEdit Error", ex);
  }
  LOG(DEBUG) << "exit";
}

void PpiWidget::EditRunScript() {
  
  // create the view
  ScriptEditorView *scriptEditorView;
  scriptEditorView = new ScriptEditorView(this);

  // create the model
  ScriptEditorModel *model = new ScriptEditorModel(_vol);
  
  // create the controller
  ScriptEditorController *scriptEditorControl = new ScriptEditorController(scriptEditorView, model);

  // connect some signals and slots in order to retrieve information
  // and send changes back to display
                                                                         
  connect(scriptEditorControl, SIGNAL(scriptChangedVolume(QStringList)),
  	  &_manager, SLOT(updateVolume(QStringList)));
  
  scriptEditorView->init();
  scriptEditorView->show();
  // scriptEditorView->layout()->setSizeConstraint(QLayout::SetFixedSize);
  
}


// This is the script editor and runner
void PpiWidget::contextMenuExamine()
{
  LOG(DEBUG) << "enter";

  EditRunScript();

  LOG(DEBUG) << "exit";
}

/* TODO add to PolarWidget class
void PolarWidget::errorMessage(string title, string message) {
  QMessageBox::information(this, QString::fromStdString(title), QString::fromStdString(message));
}
*/

void PpiWidget::ShowContextMenu(const QPoint &pos, RadxVol *vol)
{

  _vol = vol;

  QMenu contextMenu("Context menu", this);
  
  QAction action1("Cancel", this);
  connect(&action1, SIGNAL(triggered()), this, SLOT(contextMenuCancel()));
  contextMenu.addAction(&action1);

  QAction action3("Change Color Map", this);
  connect(&action3, SIGNAL(triggered()), this, SLOT(contextMenuParameterColors()));
  contextMenu.addAction(&action3);

  QAction action4("View", this);
  connect(&action4, SIGNAL(triggered()), this, SLOT(contextMenuView()));
  contextMenu.addAction(&action4);

  QAction action5("Edit Data", this);
  connect(&action5, SIGNAL(triggered()), this, SLOT(contextMenuEditor()));
  contextMenu.addAction(&action5);
  
  QAction action6("Use Script", this);
  connect(&action6, SIGNAL(triggered()), this, SLOT(contextMenuExamine()));
  contextMenu.addAction(&action6);

  QAction action7("Histogram", this);
  connect(&action7, SIGNAL(triggered()), this, SLOT(contextMenuHistogram()));
  contextMenu.addAction(&action7);

  /*
  QAction action7("Data Widget", this);
  connect(&action7, SIGNAL(triggered()), this, SLOT(contextMenuDataWidget()));
  contextMenu.addAction(&action7);
  */

  contextMenu.exec(this->mapToGlobal(pos));
}
