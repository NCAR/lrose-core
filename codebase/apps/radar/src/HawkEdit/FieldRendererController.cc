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

#include "FieldRendererController.hh"

using namespace std;

FieldRendererController::FieldRendererController()
{

}


FieldRendererController::~FieldRendererController()
{
  _fieldRenderers.clear();
}



// add a new FieldRenderer; one FieldRenderer for each field
void FieldRendererController::addFieldRenderer(FieldRenderer *fieldRenderer)
{
  LOG(DEBUG) << "enter";
  _fieldRenderers.push_back(fieldRenderer);
  LOG(DEBUG) << "exit";
}

/*
void FieldRendererController::setBeams(const vector<Beam *> &beams)
{
  LOG(DEBUG) << "enter";
  // for each field,
  //   add a pointer to the beams
  for (size_t field = 0; field < _fieldRenderers.size(); ++field) {
      _fieldRenderers[field]->setBeams(beams);
      // beam->setBeingRendered(field, true);
  }
  LOG(DEBUG) << "exit";
}
*/

 
// Queue the beams to be rendered for each field
// add a Beam to each FieldRenderer
//void FieldRendererController::addBeam(Beam *beam)
//{
  //LOG(DEBUG) << "enter";

  // for each field,
  //   add beam
//  for (size_t field = 0; field < _fieldRenderers.size(); ++field) {
//      _fieldRenderers[field]->addBeam(beam);
      //beam->setBeingRendered(field, true);
//  }

  /* original code ...
  // Add the new beams to the render lists for each of the fields                        

  for (size_t field = 0; field < _fieldRenderers.size(); ++field) {
    if (field == _selectedField ||
	_fieldRenderers[field]->isBackgroundRendered()) {
      _fieldRenderers[field]->addBeam(beam);
    } else {
      beam->setBeingRendered(field, false);
    }
  }
  */
  
  //LOG(DEBUG) << "exit";
//}
 


  
// addBeam = queueforRendering
// add a Beam to a single FieldRenderer
//void FieldRendererController::addBeam(size_t fieldIndex, Beam *beam)
//{
  //LOG(DEBUG) << "enter";
//  _fieldRenderers[fieldIndex]->addBeam(beam);
  //beam->setBeingRendered(fieldIndex, true);
  //LOG(DEBUG) << "exit";
//}
 

/*
void FieldRendererController::addBeam(string newFieldName, Beam *beam)
{
  
  LOG(DEBUG) << "enter";
  size_t fieldIndex = _findFieldIndex(newFieldName);
  addBeam(fieldIndex, beam);
  LOG(DEBUG) << "exit";
}
*/
 /*
void FieldRendererController::addBeamToBackgroundRenderedFields(Beam *beam)
{
  LOG(DEBUG) << "enter";

  // for each field,
  //   add beam
  for (size_t field = 0; field < _fieldRenderers.size(); ++field) {
    if (_fieldRenderers[field]->isBackgroundRendered()) {
      _fieldRenderers[field]->addBeam(beam);
      beam->setBeingRendered(field, true);
    } else {
      beam->setBeingRendered(field, false);
    }
  }
  LOG(DEBUG) << "exit";
}
 */

/*
size_t FieldRendererController::_findFieldIndex(string fieldName)
{
  LOG(DEBUG) << "enter";
  size_t idx = 0;
  size_t theIdx = -1;
  bool found = false;
  vector<FieldRenderer *>::iterator it;
  for (it=_fieldRenderers.begin(); it != _fieldRenderers.end(); ++it) {
    FieldRenderer *fr = *it;
    DisplayField displayField = fr->getField();
    LOG(DEBUG) << "comparing to " << displayField.getName();
    if (displayField.getName().compare(fieldName) == 0) {
      found = true;
      theIdx = idx;
    }
    idx += 1;
  }  // TODO: CRAZY! The displayField is destroyed here! Why?
  if (found) {
    LOG(DEBUG) << "exit; found " << fieldName << " at idx " << theIdx;
    return theIdx;
  } else
    throw std::invalid_argument(fieldName);
}
*/

/*
void FieldRendererController::unselectField(size_t fieldIndex)
{
  LOG(DEBUG) << "enter";
  _fieldRenderers[fieldIndex]->unselectField();
  LOG(DEBUG) << "exit";
}

void FieldRendererController::selectField(size_t fieldIndex)
{
  LOG(DEBUG) << "enter";
  _fieldRenderers[fieldIndex]->selectField();
  LOG(DEBUG) << "exit";
}
*/

// get the FieldRenderer at index ...
FieldRenderer *FieldRendererController::get(size_t fieldIndex) 
{
  LOG(DEBUG) << "enter: fieldIndex = " << fieldIndex 
  << " fieldRenderers.size = " << _fieldRenderers.size();
  //if (fieldIndex >= _fieldRenderers.size()) throw "fieldIndex exceed number of fieldRenderers";
  return _fieldRenderers.at(fieldIndex);
  LOG(DEBUG) << "exit";
}

// get the FieldRenderer by name ...
FieldRenderer *FieldRendererController::get(string fieldName) 
{
  LOG(DEBUG) << "enter: fieldName = " << fieldName 
  << " fieldRenderers.size = " << _fieldRenderers.size();
  //if (fieldIndex >= _fieldRenderers.size()) throw "fieldIndex exceed number of fieldRenderers";
  vector<FieldRenderer *>::iterator it;
  for (it = _fieldRenderers.begin(); it != _fieldRenderers.end(); ++it) {
    FieldRenderer *fr = *it;
    if (fr->getName().compare(fieldName) == 0) {
      return fr;
    }
  }
  return NULL;
  LOG(DEBUG) << "exit";
}

/*
void FieldRendererController::activateArchiveRendering()
{
  LOG(DEBUG) << "enter";
  LOG(DEBUG) << "_fieldRenderers.size()  = " << _fieldRenderers.size();
  for (size_t ii = 0; ii < _fieldRenderers.size(); ii++) {
    _fieldRenderers[ii]->activateBackgroundRendering();
  }
  LOG(DEBUG) << "exit";
}
*/
/*
void FieldRendererController::activateRealtimeRendering(size_t selectedField)
{

  for (size_t ii = 0; ii < _fieldRenderers.size(); ii++) {
    if (ii != selectedField) {
      _fieldRenderers[ii]->activateBackgroundRendering();
    }b
  }
}
*/

QImage *FieldRendererController::renderImage(int width, int height,
  string fieldName, double sweepAngle) {

  FieldRenderer *fieldRenderer = get(fieldName);
  if (fieldRenderer == NULL) {
    fieldRenderer = new FieldRenderer(fieldName);
    _fieldRenderers.push_back(fieldRenderer);
  }
  if (!fieldRenderer->imageReady()) {
    // TODO: somehow get these? 
    ColorMap colorMap;
    QBrush *background_brush = new QBrush(QColor("orange"));
    // get the Data
    //DataModel *dataModel = DataModel::Instance();
    for (int i=0; i<10; i++) { // each field ray in sweep) {
      //float *ray = dataModel->getRayData();
      vector<float> rayFake = {0,1,2,3,4,5,6,7,8,9,10};
      size_t nData = rayFake.size();
      double start_angle = 36.0 * i;
      double stop_angle = start_angle + 35.9;
      double startRangeKm = 10;
      double gateSpacingKm = 10;
      // create Beam for ray
      PpiBeam *beam = new PpiBeam(
                 start_angle,  stop_angle,
           startRangeKm,  gateSpacingKm);
      beam->updateFillColors(&rayFake[0], nData, &colorMap, background_brush);  
      fieldRenderer->addBeam(beam);
    }
    // add Beam to FieldRenderer
    fieldRenderer->createImage(width, height);
    fieldRenderer->runIt();  // calls paint method on each beam
  }
  return fieldRenderer->getImage();
}


// HERE is where the action happens
void FieldRendererController::performRendering(size_t selectedField) {
                                                                                         
  // start the rendering                                                                   
  LOG(DEBUG) << " selectedField = " << selectedField;                                    
  LOG(DEBUG) << "_fieldRenderers.size() = " << _fieldRenderers.size();                     
  //for (size_t ifield = 0; ifield < _fieldRenderers.size(); ++ifield) {                     
    //LOG(DEBUG) << "ifield " << ifield << " isBackgroundRendered() = "                      
    //   << _fieldRenderers[ifield]->isBackgroundRendered();                         
    //if (ifield == selectedField || _fieldRenderers[ifield]->isBackgroundRendered()) {
	//LOG(DEBUG) << "signaling field " << ifield << " to start";                           
	//_fieldRenderers[ifield]->signalRunToStart();                                         
    //  }                                                                                    
  //} // ifield                                                                              
                                                                                           
  // wait for rendering to complete                                                        
                                                                                           
  //for (size_t ifield = 0; ifield < _fieldRenderers.size(); ++ifield) {                     
    //if (ifield == selectedField ||  _fieldRenderers[ifield]->isBackgroundRendered()) {
      //_fieldRenderers[ifield]->waitForRunToComplete();                                     
     // }                                                                                    
  //} // ifield                                                                              
  LOG(DEBUG) << "exit";
}

/*
bool FieldRendererController::isBackgroundRendered(size_t index) {

  FieldRenderer *fieldRenderer = _fieldRenderers.at(index);
  return fieldRenderer->isBackgroundRendered();
}
*/
/*
void FieldRendererController::colorMapChanged(size_t ifield)
{
    FieldRenderer *fieldRenderer = _fieldRenderers[ifield];

    fieldRenderer->colorMapChanged();
}
*/


void FieldRendererController::refreshImages(int width, int height, QSize image_size,
					    QRgb background_brush_color_rgb,
					    QTransform zoomTransform,
					    size_t selectedField,
					    vector< PpiBeam* > &Beams)
{
  // 
  // for each field
  //    for each beam
  //       add beam to field
  //
  LOG(DEBUG) << "enter";

  for (size_t ifield = 0; ifield < _fieldRenderers.size(); ++ifield) {

    FieldRenderer *field = _fieldRenderers[ifield];

    // If needed, create new image for this field                                          

    if (image_size != field->getImage()->size()) {
      field->createImage(width, height);
    }

    // clear image                                                                         

    field->getImage()->fill(background_brush_color_rgb);

    // set up rendering details                                                            

    field->setTransform(zoomTransform);

    // Add pointers to the beams to be rendered                                            

    //if (ifield == selectedField || field->isBackgroundRendered()) {
    //  std::vector< PpiBeam* >::iterator beam;
    //  for (beam = Beams.begin(); beam != Beams.end(); ++beam) {
	  //    //(*beam)->setBeingRendered(ifield, true);
	  //    field->addBeam(*beam);
    //  }
    //}

  } // ifield                                                                              

  // do the rendering                                                                      

  performRendering(selectedField);
  
  LOG(DEBUG) << "exit";
}


/*
void FieldRendererController::refreshImagesAsDeque(int width, int height, QSize image_size,
					    QRgb background_brush_color_rgb,
					    QTransform zoomTransform,
					    size_t selectedField,
					    deque< RhiBeam* > &Beams)
{
  // 
  // for each field
  //    for each beam
  //       add beam to field
  //
  for (size_t ifield = 0; ifield < _fieldRenderers.size(); ++ifield) {

    FieldRenderer *field = _fieldRenderers[ifield];

    // If needed, create new image for this field                                          

    if (image_size != field->getImage()->size()) {
      field->createImage(width, height);
    }

    // clear image                                                                         

    field->getImage()->fill(background_brush_color_rgb);

    // set up rendering details                                                            

    field->setTransform(zoomTransform);

    // Add pointers to the beams to be rendered                                            

    if (ifield == selectedField || field->isBackgroundRendered()) {
      std::deque< RhiBeam* >::iterator beam;
      for (beam = Beams.begin(); beam != Beams.end(); ++beam) {
	(*beam)->setBeingRendered(ifield, true);
	field->addBeam(*beam);
      }
    }

  } // ifield                                                                              

  // do the rendering                                                                      

  performRendering(selectedField);

}
*/
