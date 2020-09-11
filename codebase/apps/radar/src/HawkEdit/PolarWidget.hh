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
#ifndef PolarWidget_HH
#define PolarWidget_HH

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

#include <QDialog>
#include <QWidget>
#include <QResizeEvent>
#include <QImage>
#include <QTimer>
#include <QRubberBand>
#include <QPoint>
#include <QTransform>
#include <QMessageBox>

#include <Radx/RadxPlatform.hh>
#include <Radx/RadxVol.hh>

#include "Params.hh"
#include "PpiBeamController.hh"
#include "PpiBeam.hh"
#include "RhiBeam.hh"
#include "FieldRendererController.hh"
#include "ScaledLabel.hh"
#include "WorldPlot.hh"
#include "DisplayField.hh"

class PolarManager;

/// Base class for widgets displaying PPI or RHI scans.
///
/// A beam is the basic building block for the scan. It has
/// a starting angle, ending angle, a fixed number of range 
/// gates, and a fixed number of fields that can be rendered 
/// on the beam.
///
/// The scan is given color maps for each of the fields. 
/// It is then simply called with the data for all fields
/// for a given beam. As each beam is received, the current field
/// and any fields in the beam that are being background rendered
/// are rendered into QImages.  The QImage for the current field is
/// then popped up on the display.  When the selected field is changed,
/// the image is cleared, and either the QImage for the new field is
/// popped up on the display or the new field is rendered. This
/// allows for quick switches between fields being rendered in the
/// background (which are the most recently viewed fields), without
/// the overhead of rendering all fields in the background.
///
/// The user specifies the number of gates along each beam, 
/// and the distance span of each beam. The latter is used to 
/// to create range rings in real world units.
///
/// The radar is located at 0, 0.
///
/// Zooming is accomplished by changing the limits of the Qt window.

class DLL_EXPORT PolarWidget : public QWidget
{

  // must include this if you use Qt signals/slots
  Q_OBJECT

 public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   *
   * @param[in] parent         Parent widget.
   * @param[in] params         TDRP parameters.
   */

  PolarWidget(QWidget* parent, 
              const PolarManager &manager,
              const Params &params,
              const RadxPlatform &platform,
              //const vector<DisplayField *> &fields,
	      DisplayFieldController *displayFieldController,
              bool haveFilteredFields);
  
  /**
   * @brief Destructor.
   */

  virtual ~PolarWidget();

  /**
   * @brief Configure the PolarWidget for range.
   */

  virtual void configureRange(double max_range) = 0;

  /**********************************************
   * turn on archive-style rendering - all fields
   */

  void activateArchiveRendering();

  /**********************************************************************
   * turn on reatlime-style rendering - non-selected fields in background
   */

  void activateRealtimeRendering();

  /**
   * @brief Add a new beam to the display. Data for all fields and all
   *        gates are provided, as well as color maps for all fields.
   *        addBeam() will map the field values to  the correct color, and
   *        render the beam for each field in the appropriate pixamp. The
   *        existing wedge for this beam will be discarded.
   *
   * @param[in] start_angle    The starting angle for the beam.
   * @param[in] stop_angle     The ending angle for the beam.
   * @param[in] gates          The number of gates (must match beam_data vector
   *                             sizes).
   * @param[in] beam_data      Vectors of data, one for each field.
   */

  // virtual void addBeam(const RadxRay *ray,
  //                      const float start_angle, const float stop_angle,
  //       	       const std::vector< std::vector< double > > &beam_data,
  //       	       const std::vector< DisplayField* > &fields) = 0;

  /**
   * @brief Specify the background color.
   *
   * @param[in] color     The background color.
   *
   * @notes This method is not currently called anywhere.
   */

  void backgroundColor(const QColor &color);

  /**
   * @brief Specify the grid and rings color.
   *
   * @params[in] color   The grid/rings color.
   *
   * @notes This method is not currently called anywhere.
   */

  void gridRingsColor(const QColor &color);

  void colorScaleLegend();

  /**
   * @brief Capture an image of the display.
   *
   * @return Returns the image. The caller must delete it when finished
   *         with it.
   *
   * @notes This method is not currently called anywhere.
   */

  QImage *getImage();

  /**
   * @brief Capture a pixmap of the display.
   *
   * @return Returns the pixmap. The caller must delete it when finished
   *         with it.
   *
   * @notes This method is not currently called anywhere.
   */

  QPixmap *getPixmap();
		      
  /**
   * @brief Get the aspect ratio of the display.
   *
   * @return Returns the aspect ratio of the display.
   */

  double getAspectRatio() const
  {
    return _aspectRatio;
  }

  ////////////////
  // Qt signals //
  ////////////////

 signals:

  void locationClicked(double xkm, double ykm, const RadxRay *closestRay);

  //////////////
  // Qt slots //
  //////////////

 public slots:

  /**
   * @brief Slot called when a beam has finished rendering.
   *
   * @params[in] field_num   The index of the field that was rendered.  This
   *                         is used to check if this was the selected field.
   */

  void displayImage(const size_t field_num);

  /**
   * set archive mode
   */
  
  void setArchiveMode(bool archive_mode);

  /**
   * @brief Unzoom the view.
   */

  void unzoomView();

  /**
   * @brief Resize the window.
   *
   */

  void resize(const int width, const int height);

  /**
   * @brief Set ring visibility.
   *
   * @param[in] enabled    True to show them, false otherwise.
   */

  void setRings(const bool enabled);

  /**
   * @brief Set grids visibility.
   *
   * @param[in] enabled   True to show them, false otherwise.
   */

  void setGrids(const bool enabled);

  /**
   * @brief Set azimuth lines visibility.
   *
   * @param[in] enabled    True to show them, false otherwise.
   */

  void setAngleLines(const bool enabled);
  void addNewFields(vector<DisplayField *> &newFields);

 protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief The sine of 45 degrees.  Used for positioning the labels on the
   *        45 degree lines.
   */

  static const double SIN_45;
  
  /**
   * @brief The sine of 30 degrees.  Used for positioning the azimuth lines on
   *        the 30 degree lines.
   */

  static const double SIN_30;
  
  /**
   * @brief The cosine of 30 degrees.  Used for positioning the azimuth lines on
   *        the 30 degree lines.
   */

  static const double COS_30;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Parent widget.
   */

  QWidget *_parent;
  const PolarManager &_manager;

  /**
   * @brief TDRP params.
   */

  const Params &_params;

  // instrument platform details 

  const RadxPlatform &_platform;
  
  // data fields

  //  const vector<DisplayField *> &_fields;
  DisplayFieldController *displayFieldController;
  bool _haveFilteredFields;

  /**
   * @brief The renderer for each field.
   */

  //vector<FieldRenderer*> _fieldRenderers;
  FieldRendererController *_fieldRendererController;

  // overide refresh images

  virtual void _refreshImages() = 0;

  /**
   * @brief The index of the field selected for display.
   */
  // this is now kept by the displayFieldModel
  //  size_t _selectedField;

  /**
   * @brief The brush for the background.
   */

  QBrush _backgroundBrush;

  /**
   * @brief The color for the grid and rings.
   */

  QColor _gridRingsColor;

  /**
   * @brief True if the ring display is enabled.
   */

  bool _ringsEnabled;

  /**
   * @brief True if the grids display is enabled.
   */
  
  bool _gridsEnabled;

  /**
   * @brief True if the angle lines enabled.
   */
  bool _angleLinesEnabled;


  /**
   * @brief This will create labels wiith nicely scaled values and
   *        approriate units.
   */

  ScaledLabel _scaledLabel;

  /**
   * @brief The maximum range of the beams, in km.  It affects the
   *        labelling of the range rings
   */

  double _maxRangeKm;

  // archive mode

  bool _archiveMode;

  /**
   * @brief Last X,Y location of the mouse during mouse move events; used for
   *        panning.
   */

  bool _pointClicked;
  int _mousePressX, _mousePressY;
  int _mouseReleaseX, _mouseReleaseY;
  int _zoomCornerX, _zoomCornerY;
  
  /**
   * @brief Location world of the latest click point.
   */
  
  double _worldPressX, _worldPressY;
  double _worldReleaseX, _worldReleaseY;

  /**
   * @brief Rubber band for zooming.
   */

  QRubberBand *_rubberBand;

  /**
   * @brief The rubber band origin.
   */

  QPoint _rubberBandOrigin;

  /**
   * @brief The current ring spacing in km.  This value is changed when we
   *        zoom.
   */

  double _ringSpacing;
  
  /**
   * @brief The aspect ratio of the display area.
   */

  double _aspectRatio;
  
  /**
   * @brief The width of the color scale
   */

  int _colorScaleWidth;
  
  /**
   * @brief The full window rendering dimensions.  These are different for
   *        PPI windows and RHI windows.
   */

  QTransform _fullTransform;
  WorldPlot _fullWorld;
  
  /**
   * @brief The window to use for rendering.  This is where the zoom is
   *        implemented.
   */

  bool _isZoomed;
  QTransform _zoomTransform;
  WorldPlot _zoomWorld;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Render the rings and grid. The current value of _ringsGridColor
   *        will be used for the color.
   *
   * @param[in] painter    Painter to use for rendering.
   */

  virtual void _drawOverlays(QPainter &painter) = 0;

  /**
   * @brief Determine a ring spacing which will give even distances, and
   *        fit a reasonable number of rings in the display.
   *
   * @return Returns the ring spacing in kilometers.
   */

  virtual void _setGridSpacing() = 0;

  /**
   * @brief Initialize the full window transform to use for the widget.
   *
   * @param[in] window    The full window to use for the widget.
   */

  void _setTransform(const QTransform &transform);

  /////////////////////////////////
  // Overridden QtWidget methods //
  /////////////////////////////////

  /**
   * @brief Capture mouse move event for panning/zooming.
   *
   * @param[in] event   The mouse event.
   */

  virtual void mouseMoveEvent(QMouseEvent* event);

  /**
   * @brief Capture mouse press event which signals the start of
   *        panning/zooming.
   *
   * @param[in] event    The mouse press event.
   */

  virtual void mousePressEvent(QMouseEvent* event);

  /**
   * @brief Capture mouse release event which signals the start of
   * panning/zooming.
   *
   * @param[in] event    The mouse event.
   */

  virtual void mouseReleaseEvent(QMouseEvent* event);

  /**
   * @brief The method that is called when a repaint event is triggered.
   *
   * @param[in] event   The repaint event.
   */

  void paintEvent(QPaintEvent *event);

  /**
   * @brief Handle a resize event. A timer is used to prevent refreshes until
   *        the resize is finished.
   *
   * @brief event   The resize event.
   */

  void smartBrush(int xPixel, int yPixel);

  virtual void resizeEvent(QResizeEvent * event);

  // reset the world coords

  void _resetWorld(int width, int height);

  // rendering

  void _performRendering();

  // get ray closest to click point

  virtual const RadxRay *_getClosestRay(double x_km, double y_km) = 0;

 public:

  
  //  virtual void ShowContextMenu(const QPoint &pos);
  virtual void ShowContextMenu(const QPoint &pos, RadxVol *vol);
  void setFont();
  virtual void ExamineEdit(const RadxRay *closestRay);
  void notImplemented();
  virtual void informationMessage();
  void errorMessage(string title, string message) {                                                    
    QMessageBox::information(this, QString::fromStdString(title), QString::fromStdString(message));                 
  }  

 public slots:

  virtual void contextMenuCancel();
  virtual void contextMenuParameterColors();
  virtual void contextMenuView();
  virtual void contextMenuEditor();
  virtual void contextMenuExamine(); // const QPoint &pos);
  virtual void contextMenuDataWidget();
  virtual void contextMenuHistogram();
  

};

#endif
