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

#ifndef FieldRendererView_HH
#define FieldRendererView_HH

#ifndef DLL_EXPORT
#ifdef WIN32
#ifdef QT_PLUGIN
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif
#else
#define DLL_EXPORT
#endif
#endif

#include <string>
#include <vector>

#include <QImage>
#include <QRect>
#include <QTimer>
#include <QTransform>

#include <toolsa/TaThread.hh>
//#include "Params.hh"
#include "Beam.hh"

// Really, this is a View 

class DLL_EXPORT FieldRendererView : public QObject  // , public TaThread
{

  Q_OBJECT

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */
  
  FieldRendererView(string fieldName);
  // const Params &params,
                //const size_t field_index,
                // const DisplayField &field);

  /**
   * @brief Destructor
   */

  virtual ~FieldRendererView();

  // parameters for this field

  //const DisplayField &getField() { return _field; }
  
  // setting state
  

  void setTransform(const QTransform &transform) { _transform = transform; }

  // setting state - bscan only

  void setUseHeight(bool useHeight) { _useHeight = useHeight; }
  void setDrawInstHt(bool drawInstHt) { _drawInstHt = drawInstHt; }
  
  /**
   * @brief Add the given beam to the beams to be rendered.
   */
 


  /**
   * @brief Perform the housekeeping needed when this field is newly selected.
   */

  //void selectField();

  /**
   * @brief Perform the housekeeping needed when this field is newly unselected.
   */

  //void unselectField();

  // Activate background rendering - turn on until time resets
  
  //void activateBackgroundRendering();

  // Set background rendering on - until set off
  
  //void setBackgroundRenderingOn();

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Get the current background rendering image for this field.
   */

  //inline QImage *getImage() const { return _image; }

  bool imageReady() { return _imageReady; }
  
  /**
   * @brief Check whether the field is currently being rendered in the
   *        background.
   */
  
  //inline bool isBackgroundRendered() const { return _backgroundRender; }
  
  //////////////
  // Qt slots //
  //////////////

//public slots:
  
  /**
   * @brief Slot called when the background rendering timer expires.
   */

//  void setBackgroundRenderOff()
//  {
//    _backgroundRender = false;
//  }
  
//  void setBackgroundRenderOn()
//  {
//    _backgroundRender = true;
//  }
  
  ////////////////
  // Qt signals //
  ////////////////
  
//signals:

  //void imageReady(QImage *image);
  
  //////////////
  // Qt slots //
  //////////////

//protected:

  ///////////////////////
  // Protected members //
  ///////////////////////


  
  //const Params &_params;

  /**
   * @brief The index for this field.
   */

  //size_t _fieldIndex;
  
  // parameters for this field
  
  //const DisplayField &_field;

  //void renderImage(int width, int height, double sweepAngle);
  
  void addBeam(Beam *beam);

  void createImage(int width, int height);  

  void fillBackground(QBrush *backgroundBrush);
  /**
   * @brief Image used for background rendering of this field.
   */

  string getName() { return _name; };
  double getSweepAngle() { return _sweepAngle; };
  //void setSweepAngle(double sweepAngle);

  QImage _image;
  bool _imageReady;
  string _name;
  double _sweepAngle;
  /**
   * @brief Flag indicating whether this field should be rendered in the
   *        background.
   */

  //bool _backgroundRender;

  /**
   * @brief Timer used for turning off the background rendering after the
   *        specified period of time.
   */

  //QTimer *_backgroundRenderTimer;

  /**
   * @brief The transform to use for rendering.
   */

  QTransform _transform;
  
  /**
   * @brief Array of beams to be rendered
   */
  // Let PpiWidget  manage the list of beams
  // only fill this vector, from refreshImages
  vector< Beam*> _beams;
    
  /**
   * @brief Render in height instead of range?
   */

  bool _useHeight;

  // draw in instrument height

  bool _drawInstHt;
  const ColorMap &getColorMap() const { return _colorMap; }

  bool haveColorMap() const { return _haveColorMap; }
  void setNoColorMap() { _haveColorMap = false; }
  void changeColorMap() { _haveColorMap = true; }
  void replaceColorMap(ColorMap newColorMap);

  void clearBeams();
    
  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Thread run method.
   * This is where the rendering actually gets done
   */

  void runIt(QPainter *painter);

private:
  ColorMap _colorMap;
  bool _haveColorMap;
  bool _rendering; 
  bool _changed;
  
};

#endif
    
