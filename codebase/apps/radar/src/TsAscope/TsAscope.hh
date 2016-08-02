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
// TsAscope.h
//
// TsAscope object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2011
//
///////////////////////////////////////////////////////////////

#ifndef TsAscope_HH
#define TsAscope_HH

#include <string>
#include <vector>

#include "Args.hh"
#include "Params.hh"
#include <QMainWindow>

class QApplication;
class QActionGroup;
class QButtonGroup;
class QRadioButton;
class QFrame;
class QDialog;
class QLabel;
class QGroupBox;
class QGridLayout;
class ColorMap;
class ColorBar;
class PpiWidget;
class RhiWidget;
class RhiWindow;
class Reader;
class RadxVol;
class RadxRay;

class TsAscope : public QMainWindow {
  
  Q_OBJECT

public:

  class Field {
  public:
    string name;
    string units;
    ColorMap *colorMap;
    double selectValue;
    QLabel *dialogEntry;
    Field() {
      colorMap = NULL;
      selectValue = -9999;
      dialogEntry = NULL;
    }
  };

  class RayLoc {
  public:
    int startIndex;
    int endIndex;
    bool master;
    bool active;
    const RadxRay *ray;
    RayLoc() {
      master = false;
      active = false;
      ray = NULL;
    }
  };

  // constructor

  TsAscope (int argc, char **argv, QWidget *parent);

  // destructor
  
  ~TsAscope();

  // run 

  int Run(QApplication &app);

  /**
   * @brief Set the radar name.  The name is included as part of the window
   *        title.
   */

  void setRadarName(const string &radar_name)
  {
    string window_title = "HAWK_EYE -- " + radar_name;
    setWindowTitle(tr(window_title.c_str()));
  }
  

  // data members

  int OK;

signals:

  ////////////////
  // Qt signals //
  ////////////////

  /**
   * @brief Signal emitted when the PPI frame is resized.
   *
   * @param[in] width    The new width of the frame.
   * @param[in] height   The new height of the frame.
   */

  void frameResized(const int width, const int height);
  
protected:
  
  // void _contextMenuEvent(QContextMenuEvent *event);
						 
private slots:

  //////////////
  // Qt slots //
  //////////////

  void _changeField(int fieldId);
  void _ppiLocationClicked(double xkm, double ykm);
  void _rhiLocationClicked(double xkm, double ykm);
  void _locationClicked(double xkm, double ykm, RayLoc *ray_loc);
  void _freeze();
  void _showFields();
  void _unzoom();
  // void _print();
  void _howto();
  void _about();
  void _aboutQt();
  
private:

  // basic

  string _progName;
  Params _params;
  Args _args;
  int _argc;
  char **_argv;
  char *_paramsPath;

  // reading data in

  Reader *_reader;

  // beam reading timer

  static bool _firstTimerEvent;
  int _beamTimerId;
  bool _frozen;

  // geometry

  int _nGates;
  double _maxRange;

  // data fields

  vector<Field> _fields;
  int _nFields;

  // colors

  vector<ColorMap *> _colorMaps;
  ColorBar *_colorBar;

  // ray locator
  // we use a locator with data every 1/100th degree
  // around the 360 degree circle

  static const int RAY_LOC_RES = 10;
  static const int RAY_LOC_N = 4000;
  static const int RAY_LOC_OFFSET = 300;
 
  RayLoc* _ppiRayLoc;  // for use, allows negative indices at north line
  RayLoc* _ppiRays;    // for new and delete

  RayLoc* _rhiRayLoc;  // for use, allows negative indices at north line
  RayLoc* _rhiRays;    // for new and delete

  // windows

  QFrame *_main;

  QFrame *_ppiParent;
  PpiWidget *_ppi;

  RhiWindow *_rhiWindow;
  RhiWidget *_rhi;
  bool _rhiWindowDisplayed;
  
  // azimuths for current ray

  double _prevAz;
  double _prevEl;
  double _startAz;
  double _endAz;

  // menus

  QMenu *_mainMenu;
  QMenu *_fileMenu;
  QMenu *_viewMenu;
  QMenu *_helpMenu;

  // actions

  QActionGroup *_alignmentGroup;
  QAction *_freezeAct;
  QAction *_showFieldsAct;
  QAction *_unzoomAct;
  QAction *_clearAct;
  // QAction *_printAct;
  QAction *_exitAct;
  QAction *_ringsAct;
  QAction *_gridsAct;
  QAction *_azLinesAct;
  QAction *_showRhiAct;
  QAction *_howtoAct;
  QAction *_aboutAct;
  QAction *_aboutQtAct;
  // QLabel *_infoLabel;

  // status panel

  QGroupBox *_statusPanel;
  QGridLayout *_statusLayout;

  QLabel *_radarName;
  QLabel *_dateLabel;
  QLabel *_timeLabel;

  QLabel *_volNumLabel;
  QLabel *_sweepNumLabel;

  QLabel *_fixedAngLabel;
  QLabel *_elevLabel;
  QLabel *_azLabel;

  QLabel *_nSamplesLabel;
  QLabel *_nGatesLabel;
  QLabel *_gateSpacingLabel;

  QLabel *_pulseWidthLabel;
  QLabel *_prfLabel;
  QLabel *_nyquistLabel;
  QLabel *_maxRangeLabel;
  QLabel *_powerHLabel;
  QLabel *_powerVLabel;

  QLabel *_sweepModeLabel;
  QLabel *_polModeLabel;
  QLabel *_prfModeLabel;

  QLabel *_latLabel;
  QLabel *_lonLabel;
  QLabel *_altLabel;

  // field panel
  
  QGroupBox *_fieldPanel;
  QGridLayout *_fieldsLayout;
  QLabel *_selectedLabel;
  QButtonGroup *_fieldGroup;
  vector<QRadioButton *> _fieldButtons;
  string _selectedName;
  QLabel *_valueLabel;
  int _fieldNum;
  int _prevFieldNum;

  // field status dialog - for click location

  QDialog *_fieldStatusDialog;
  QGridLayout *_fieldDialogLayout;
  QLabel *_dateClicked;
  QLabel *_timeClicked;
  QLabel *_elevClicked;
  QLabel *_azClicked;
  QLabel *_gateNumClicked;
  QLabel *_rangeClicked;

  // methods

  int _setupColorMaps();
  void _setupWindows();
  void _createStatusPanel();
  void _createFieldPanel();
  void _createFieldStatusDialog();
  void _createRhiWindow();
  QLabel *_newLabelRight(const string &text);
  QLabel *_createStatusLabel(const string &leftLabel,
			     const string &rightLabel,
			     int row,
                             int fontSize = 0);
  QLabel *_createDialogLabel(const string &leftLabel,
			     const string &rightLabel,
			     int row,
                             int fontSize = 0);
  int _setupReader();
  void _drawBeam(RadxVol &vol, RadxRay *ray);
  void _createActions();
  void _createMenus();


  // override event handling

  virtual void timerEvent (QTimerEvent * event);
  virtual void resizeEvent (QResizeEvent * event);

  void _storeRayLoc(const RadxRay *ray, const double az,
		    const double beam_width, RayLoc *ray_loc);
  void _clearRayOverlap(const int start_index, const int end_index,
			RayLoc *ray_loc);

  virtual void keyPressEvent(QKeyEvent* event);

};

#endif

