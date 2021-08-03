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

#include <toolsa/LogStream.hh>


#ifndef FIELDRENDERERCONTROLLER_H
#define FIELDRENDERERCONTROLLER_H


#include <deque>

#include "FieldRendererView.hh"
#include "Beam.hh"
#include "PpiBeam.hh"
#include "RayLocationController.hh"
#include "DataModel.hh"
#include <QImage>

class FieldRendererController
{

public:

  FieldRendererController();
  virtual ~FieldRendererController();

  // NOTE: only index by fieldIndex which must match the button row in the field Panel
  void addField(string &fieldName);
  void addFieldRenderer(FieldRendererView *);
  void addBeam(Beam *beam);
  void addBeam(size_t fieldIndex, Beam *beam);
  //  void addBeam(string newFieldName, Beam *beam);
  void addBeamToBackgroundRenderedFields(Beam *beam);
  //void selectField(size_t fieldIndex);
  //void unselectField(size_t fieldIndex);
  FieldRendererView *get(size_t fieldIndex);
  FieldRendererView *get(string fieldName);
  QImage *getImage(string fieldName);
  //void activateArchiveRendering();
  //void activateRealtimeRendering(size_t selectedField);
  QImage *renderImage(int width, int height,
    string fieldName, QTransform zoomTransform, double sweepAngle,
    RayLocationController *rayLocationController,
    ColorMap &colorMap,
    QColor backgroundColor);
  
  //void performRendering(size_t selectedField);
  //bool isBackgroundRendered(size_t index);
  //void setBackgroundRenderingOn(size_t index) {_fieldRenderers[index]->setBackgroundRenderingOn();;};
  void refreshImages(int width, int height, QSize image_size,
					      QBrush backgroundColor, // QRgb background_brush_color_rgb,
					      QTransform zoomTransform);
					      //size_t selectedField);
		     //vector< PpiBeam* > &Beams);

/*
  void refreshImagesAsDeque(int width, int height, QSize image_size,
					      QRgb background_brush_color_rgb,
					      QTransform zoomTransform,
					      size_t selectedField,
			    std::deque< RhiBeam* > &Beams);
          */
  void takeCareOfMissingValues(vector<float> *rayData, float missingValue);

private:
 
  vector<FieldRendererView *> _fieldRenderers;
  //  vector<FieldRenderer *> _working;
 
  // size_t _findFieldIndex(string fieldName);

};

#endif
