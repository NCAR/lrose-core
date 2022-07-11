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
#include "RayLocationController.hh"
#include <toolsa/LogStream.hh>
#include <QPainter>

using namespace std;

FieldRendererController::FieldRendererController()
{

}


FieldRendererController::~FieldRendererController()
{
  _fieldRenderers.clear();
}


void FieldRendererController::addField(string &fieldName) {
  FieldRendererView *fieldRenderer = new FieldRendererView(fieldName);
  addFieldRenderer(fieldRenderer);
}

// add a new FieldRenderer; one FieldRenderer for each field
void FieldRendererController::addFieldRenderer(FieldRendererView *fieldRenderer)
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
FieldRendererView *FieldRendererController::get(size_t fieldIndex) 
{
  LOG(DEBUG) << "enter: fieldIndex = " << fieldIndex 
  << " fieldRenderers.size = " << _fieldRenderers.size();
  //if (fieldIndex >= _fieldRenderers.size()) throw "fieldIndex exceed number of fieldRenderers";
  return _fieldRenderers.at(fieldIndex);
  LOG(DEBUG) << "exit";
}

// get the FieldRenderer by name ...
FieldRendererView *FieldRendererController::get(string fieldName) 
{
  LOG(DEBUG) << "enter: fieldName = " << fieldName 
  << " fieldRenderers.size = " << _fieldRenderers.size();
  //if (fieldIndex >= _fieldRenderers.size()) throw "fieldIndex exceed number of fieldRenderers";
  vector<FieldRendererView *>::iterator it;
  for (it = _fieldRenderers.begin(); it != _fieldRenderers.end(); ++it) {
    FieldRendererView *fr = *it;
    if (fr->getName().compare(fieldName) == 0) {
      return fr;
    }
  }
  return NULL;
  LOG(DEBUG) << "exit";
}


/*
QImage *FieldRendererController::getImage(string fieldName) {
  LOG(DEBUG) << "enter" << "fieldName = " << fieldName;
  QImage *image = NULL;
  FieldRendererView *fieldRenderer = get(fieldName);
  if (fieldRenderer != NULL) {
    image = fieldRenderer->getImage();
  }
  LOG(DEBUG) << "exit: ";

  return image;
}
*/

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

void FieldRendererController::takeCareOfMissingValues(vector<float> *rayData,
  float missingValue) {

  vector<float>::iterator it;
  for (it = rayData->begin(); it != rayData->end(); ++it) {
        float val = *it;
        if (fabs(val - missingValue) < 0.0001) {
          *it = -9999.0;
        } 
  }

  //for (int ifield=0; ifield < newFieldNames.size(); ++ifield) {
  //string fieldName = newFieldNames.at(ifield); // .toLocal8Bit().constData();
  // vector<double> &data = fieldData[ifield];
  //  data.resize(_nGates);
   // RadxField *rfld = ray->getField(fieldName);

    // at this point, we know the data values for the field AND the color map                                                                        
    //ColorMap *fieldColorMap = _displayFieldController->getColorMap(fieldName); 
    //bool haveColorMap = fieldColorMap != NULL;

/*TODO: fix this code, need missing value

    if (rfld == NULL) {
      // fill with missing
      for (int igate = 0; igate < _nGates; igate++) {
        data[igate] = -9999.0;
      }
    } else {
      rfld->convertToFl32();
      const Radx::fl32 *fdata = rfld->getDataFl32();
      // print first 15 data values
      //LOG(DEBUG) << "ray->nGates = " << ray->getNGates();
      //LOG(DEBUG) << "first 30 gates ...";
      //for (int ii = 0; ii< 15; ii++)
      //LOG(DEBUG) << fdata[ii];
      // end print first 15 data values
      const Radx::fl32 missingVal = rfld->getMissingFl32();
      // we can only look at the data available, so only go to nGates
      for (int igate = 0; igate < _nGates; igate++, fdata++) {  // was _nGates
        Radx::fl32 val = *fdata;
        if (fabs(val - missingVal) < 0.0001) {
          data[igate] = -9999.0;
        } else {
          data[igate] = val;
        
        } // end else not missing value
      } // end for each gate

    } // end else vector not NULL
    */
} 



QImage *FieldRendererController::renderImage(QPainter &painter, int width, int height,
  string fieldName, QTransform zoomTransform, // double sweepAngle, 
  RayLocationController *rayLocationController,
  ColorMap &colorMap,
  QColor backgroundColor) {

  // TODO: need to keep sweepAngle with FieldRendererView
  // that way we can detect if the image is current for the
  // selected sweep

  LOG(DEBUG) << "enter: ";
  LOG(DEBUG) << "requested width = " << width << " height " << height;

  FieldRendererView *fieldRenderer = get(fieldName);
  if (fieldRenderer == NULL) {
    fieldRenderer = new FieldRendererView(fieldName);
    //fieldRenderer->setSweepAngle(sweepAngle); 
    //fieldRenderer->createImage(painter, width, height);
    _fieldRenderers.push_back(fieldRenderer);
  } else {
    //fieldRenderer->setSweepAngle(sweepAngle); 
    //fieldRenderer->createImage(painter, width, height);
  }
  //LOG(DEBUG) << "lock obtained";
  // fieldRenderView is locked, we can procede with changes ...

  //fieldRenderer->setTransform(zoomTransform);

        
  try {

  //if (!fieldRenderer->imageReady()) {
    LOG(DEBUG) << "image NOT READY, recreating beams";
    // create a beam for each ray  
    //ColorMap colorMap;
    QBrush *background_brush = new QBrush(backgroundColor); // QColor("orange"));
    // get the Data
    DataModel *dataModel = DataModel::Instance();
    float missingVal = dataModel->getMissingFl32(fieldName);
    fieldRenderer->clearBeams();
    size_t nRayLocations = rayLocationController->getNRayLocations();
    // get rays in sorted order from RayLocationController
    LOG(DEBUG) << "nRayLocations " << nRayLocations;
    size_t rayIdx=0;
    bool done = false;
    while ((rayIdx < nRayLocations) && (!done)) { // each field ray in sweep) {
      // LOG(DEBUG) << "  rayIdx = " << rayIdx;
      vector<float> *rayData;

        rayData = rayLocationController->getRayData(rayIdx, fieldName);

        if (rayData->size() > 0) {
          takeCareOfMissingValues(rayData, missingVal);
          //float rayFake[] = {0,1,2,3,4,5,6,7,8,9,10};
          size_t nData = rayData->size();
          size_t endIndex = rayLocationController->getEndIndex(rayIdx);
          double start_angle = rayLocationController->getStartAngle(rayIdx);
          double stop_angle = rayLocationController->getStopAngle(rayIdx);
          LOG(DEBUG) << "rayIdx " << rayIdx << "start_angle " << start_angle << " stop_angle " << stop_angle;
          double startRangeKm = rayLocationController->getStartRangeKm(rayIdx);
          double gateSpacingKm = rayLocationController->getGateSpacingKm(rayIdx);
          // create Beam for ray
          PpiBeam *beam = new PpiBeam(
                     start_angle,  stop_angle,
               startRangeKm,  gateSpacingKm, nData);
          float *data = &(*rayData)[0];
          beam->updateFillColors(data, nData, &colorMap, background_brush);  
          fieldRenderer->addBeam(beam);
          // rayIdx must be increasing
          if (endIndex < rayIdx) done = true;
          rayIdx = endIndex + 1;
        } else {
          rayIdx += 1;
        }
        delete rayData;
    }
    // add Beam to FieldRenderer
    LOG(DEBUG) << " added all beams to fieldRenderer; before fillBackground";
    //fieldRenderer->fillBackground(painter, background_brush);
    fieldRenderer->runIt(painter);  // calls paint method on each beam
    // fieldRendererView is unlocked
    //LOG(DEBUG) << "lock released";
    //delete background_brush;
 // }
  
  //painter->drawImage(0,0, fieldRenderer->_image);
  //QImage image = fieldRenderer->_image;
  //LOG(DEBUG) << "image width = " << image->width() << " height " << image->height();

  } catch (std::invalid_argument &ex) {
    cerr << "Exception: " << ex.what() << endl;
    //cerr << "  setting data to missing; rayIdx = " << rayIdx << endl;
    //throw ex;
  }
  //QImage image("/Users/brenda/Desktop/LROSE-Gateway-Banner.png");
  //painter->drawImage(0, 0, image);
  LOG(DEBUG) << "exit";  
  return NULL;
}


/*
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
*/

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

/*
void FieldRendererController::refreshImages(int width, int height, QSize image_size,
					    QBrush backgroundColor, 
					    QTransform zoomTransform)
{

  LOG(DEBUG) << "enter";
  LOG(DEBUG) << "there are " << _fieldRenderers.size() << " fieldRenderers";


  QBrush *background_brush = new QBrush("purple");
  *background_brush = backgroundColor;
  vector<FieldRendererView *>::iterator it;

  for (it = _fieldRenderers.begin(); it != _fieldRenderers.end(); ++it) {

    FieldRendererView *field = *it;

    field->fillBackground(background_brush); // background_brush_color_rgb);                                  
    field->setTransform(zoomTransform);
    field->runIt(); 

  } // ifield   

  delete background_brush;   

  LOG(DEBUG) << "exit";
}
*/

