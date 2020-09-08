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
// PolarPlot.hh
//
// Plotting for power vs range in an ascope
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2019
//
///////////////////////////////////////////////////////////////
#ifndef PolarPlot_HH
#define PolarPlot_HH

#include <string>
#include <vector>

#include <QDialog>
#include <QWidget>
#include <QResizeEvent>
#include <QImage>
#include <QTimer>
#include <QRubberBand>
#include <QPoint>
#include <QTransform>

#include "Params.hh"
#include "ScaledLabel.hh"
#include "FieldRenderer.hh"
#include "WorldPlot.hh"
#include "DisplayField.hh"

class Beam;
class MomentsFields;
class PolarManager;
class PolarWidget;

/// AScope plotting

class PolarPlot
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * Constructor.
   */
  
  PolarPlot(PolarWidget *parent,
            const PolarManager &manager,
            const Params &params,
            int id,
            Params::plot_type_t plotType,
            string label,
            double minAz,
            double maxAz,
            double minEl,
            double maxEl,
            double minXKm,
            double maxXKm,
            double minYKm,
            double maxYKm,
            const RadxPlatform &platform,
            const vector<DisplayField *> &fields,
            bool haveFilteredFields);
  
  /**
   * @brief Destructor.
   */

  virtual ~PolarPlot();

  // set the window geometry
  
  void setWindowGeom(int width,
                     int height,
                     int xOffset,
                     int yOffset);

  // set the world limits
  
  void setWorldLimits(double xMinWorld,
                      double yMinWorld,
                      double xMaxWorld,
                      double yMaxWorld);

  // zooming - pixel space

  void zoom(int x1, int y1, int x2, int y2);
  void unzoom();

  // set current field

  void setFieldNum(int fieldNum) {
    _fieldNum = fieldNum;
  }

  // set archive mode

  void setArchiveMode(bool archive_mode);

  // set overlays

  void setRings(bool enabled);
  void setGrids(bool enabled);
  void setAngleLines(bool enabled);

  // turn on archive-style rendering - all fields

  void activateArchiveRendering();

  // turn on reatlime-style rendering - non-selected fields in background

  void activateRealtimeRendering();

  // set the background color

  void setBackgroundColor(const QColor &color);

  // set the color for the grid / rings

  void setGridRingsColor(const QColor &color);

  // id

  int getId() const { return _id; }
  
  // image for current field

  void setImageWidth(int val) { _imageWidth = val; }
  void setImageHeight(int val) { _imageHeight = val; }
  void setImageOffsetX(int val) { _imageOffsetX = val; }
  void setImageOffsetY(int val) { _imageOffsetY = val; }
  
  int getImageWidth() const { return _imageWidth; }
  int getImageHeight() const { return _imageHeight; }
  int getImageOffsetX() const { return _imageOffsetX; }
  int getImageOffsetY() const { return _imageOffsetY; }

  QImage *getCurrentImage();
  
  // get the world plot objects
  
  WorldPlot &getFullWorld() { return _fullWorld; }
  WorldPlot &getZoomWorld() { return _zoomWorld; }
  bool getIsZoomed() const { return _isZoomed; }

  // get the window geom

  int getWidth() const { return _fullWorld.getWidthPixels(); }
  int getHeight() const { return _fullWorld.getHeightPixels(); }
  int getXOffset() const { return _fullWorld.getXPixOffset(); }
  int getYOffset() const { return _fullWorld.getYPixOffset(); }
  
  // get grid lines state

  bool getRingsEnabled() const { return _ringsEnabled; }
  bool getGridsEnabled() const { return _gridsEnabled; }
  bool getAngleLinesEnabled() const { return _angleLinesEnabled; }

  // set the transform between world and image coords

  void setTransform00(const QTransform &transform);

  // add a beam
  
  virtual void addRay(const RadxRay *ray,
                      const std::vector< std::vector< double > > &beam_data,
                      const std::vector< DisplayField* > &fields) = 0;

  // Clear the field images
  
  void clearFieldImages();

  // Clear the plot
  
  virtual void clear() = 0;

  // refresh field images

  virtual void refreshFieldImages() = 0;

  // get ray closest to click point
  
  virtual const RadxRay *getClosestRay(int imageX, int imageY,
                                       double &xKm, double &yKm) = 0;
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  static const double SIN_45;
  static const double SIN_30;
  static const double COS_30;

  PolarWidget *_parent;
  const PolarManager &_manager;
  const Params &_params;
  int _id;

  // plot type and geometry
  
  Params::plot_type_t _plotType;
  string _label;
  double _minAz;
  double _maxAz;
  double _minEl;
  double _maxEl;
  double _minXKm;
  double _maxXKm;
  double _minYKm;
  double _maxYKm;
  double _maxRangeKm;
  
  // unzoomed world

  // QTransform _fullTransform;
  WorldPlot _fullWorld;

  // zoomed world'

  bool _isZoomed;
  // QTransform _zoomTransform;
  WorldPlot _zoomWorld;

  // instrument platform details 

  const RadxPlatform &_platform;
  
  // data fields
  
  const vector<DisplayField *> &_fields;
  bool _haveFilteredFields;
  size_t _fieldNum;

  // The renderer for each field.

  vector<FieldRenderer*> _fieldRenderers;
  
  // overlays

  bool _ringsEnabled;
  bool _gridsEnabled;
  bool _angleLinesEnabled;
  double _ringSpacing;

  // painting

  QBrush _backgroundBrush;
  QColor _gridRingsColor;
  ScaledLabel _scaledLabel;

  // archive mode

  bool _archiveMode;

  // Image used for background rendering of this field

  QImage *_image;

  int _imageWidth;
  int _imageHeight;
  int _imageOffsetX;
  int _imageOffsetY;

  ///////////////////////
  // Protected methods //
  ///////////////////////

  // Determine a ring spacing which will give even distances, and
  // fit a reasonable number of rings in the display.
  // return Returns the ring spacing in kilometers.

  virtual void _setGridSpacing() = 0;

  // draw the overlays
  
  virtual void _drawOverlays(QPainter &painter) = 0;
  
  // reset the world coords
  
  void _resetWorld(int width, int height);
  
  // create the image
  
  void _createImage(int width, int height);

  // rendering
  
  void _performRendering();
  
};

#endif
