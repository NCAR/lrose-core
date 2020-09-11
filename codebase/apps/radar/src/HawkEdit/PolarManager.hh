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
// PolarManager.hh
//
// PolarManager object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2014
//
///////////////////////////////////////////////////////////////
//
// PolarManager manages polar data gathering and dissemination
// for PPIs and RHIs
//
// Rendering is delegated to PpiWidget and RhiWidget
//
///////////////////////////////////////////////////////////////

#ifndef PolarManager_HH
#define PolarManager_HH

#include <string>
#include <vector>

#include "Args.hh"
#include "Params.hh"
#include "DisplayManager.hh"
#include "RayLoc.hh"
#include "ContextEditingView.hh"
#include <QMainWindow>
#include <QListWidgetItem>
#include <QStringList>
#include <euclid/SunPosn.hh>
#include <Radx/RadxRay.hh>

class QApplication;
class QActionGroup;
class QButtonGroup;
class QRadioButton;
class QPushButton;
class QListWidget;
class QFrame;
class QDialog;
class QLabel;
class QGroupBox;
class QGridLayout;
class QDateTime;
class QDateTimeEdit;
class QFileDialog;

class DisplayField;
class PpiWidget;
class RhiWidget;
class RhiWindow;
class Reader;
class RadxPlatform;
class TimeScaleWidget;

class PolarManager : public DisplayManager {
  
  Q_OBJECT

public:
  static PolarManager* Instance();

  // boundary editor dialog
  QDialog *_boundaryEditorDialog;
  QGridLayout *_boundaryEditorDialogLayout;

  // constructor
  
  PolarManager(const Params &params,
               Reader *reader,
	       DisplayFieldController *displayFieldController,
	       //               const vector<DisplayField *> &fields,
               bool haveFilteredFields);
  
  // destructor
  
  ~PolarManager();

  // run 

  int run(QApplication &app);

  // enable the zoom button - called by PolarWidget

  void enableZoomButton() const;

  // override event handling

  virtual void timerEvent (QTimerEvent * event);
  virtual void resizeEvent (QResizeEvent * event);
  virtual void keyPressEvent(QKeyEvent* event);

  // check on archive mode
  
  bool checkArchiveMode() const { return _archiveMode; }

  // input file list for archive mode

  void setArchiveFileList(const vector<string> &list,
                          bool fromCommandLine = true);
  
  // load archive file list by searching for files
  // returns 0 on success, -1 on failure
  
  int loadArchiveFileList();


  //  const RadxVol getDataVolume();

  double getSelectedSweepAngle();
  size_t getSelectedFieldIndex();


public slots:

  //colorMapRedefineReceived(string, ColorMap)
  void colorMapRedefineReceived(string fieldName, ColorMap newColorMap,
				QColor gridColor,
				QColor emphasisColor,
				QColor annotationColor,
				QColor backgroundColor);
  void setVolume(); // const RadxVol &radarDataVolume);
  // TODO:
  // Good. Now go and read about Q_DECLARE_METATYPE(). Or better yet, use QStringList instead of std::vector<std::string>.
  void updateVolume(QStringList newFieldNames);
  void _volumeDataChanged(QStringList newFieldNames);
  void _addNewFields(QStringList newFieldNames);
  //void _updateField(size_t fieldId);

signals:

private:

  static PolarManager* m_pInstance;


  bool _firstTime;
  bool _urlOK;

  // beam geometry
  
  int _nGates;
  double _maxRangeKm;

  RayLoc* _ppiRayLoc; // for use, allows negative indices at north line
  RayLoc* _ppiRays;   // for new and delete

  // input data
  
  RadxTime _readerRayTime;
  RadxVol _vol;

  // sweeps

  SweepManager _sweepManager;
  QVBoxLayout *_sweepVBoxLayout;
  QGroupBox *_sweepPanel;
  vector<QRadioButton *> *_sweepRButtons;

  // windows

  QFrame *_ppiFrame;
  PpiWidget *_ppi;

  RhiWindow *_rhiWindow;
  RhiWidget *_rhi;
  bool _rhiWindowDisplayed;
  bool _rhiMode;
  
  // azimuths for current ray

  double _prevAz;
  double _prevEl;
  double _startAz;
  double _endAz;

  // times for rays

  RadxTime _plotStartTime;
  RadxTime _plotEndTime;
  RadxTime _prevRayTime;
  
  // menus

  QMenu *_fileMenu;
  QMenu *_timeMenu;
  QMenu *_overlaysMenu;
  QMenu *_helpMenu;

  // actions

  QAction *_realtimeAct;
  QAction *_showTimeControlAct;
  QAction *_ringsAct;
  QAction *_gridsAct;
  QAction *_azLinesAct;
  QAction *_showRhiAct;
  
  QAction *_timeControllerAct;
  QAction *_openFileAct;
  QAction *_saveFileAct;
  QAction *_saveImageAct;

  // archive mode
  
  bool _archiveMode;
  bool _archiveRetrievalPending;

  QDateTimeEdit *_archiveStartTimeEdit;
  RadxTime _guiStartTime;
  RadxTime _archiveStartTime;
  
  QDateTimeEdit *_archiveEndTimeEdit;
  RadxTime _guiEndTime;
  RadxTime _archiveEndTime;

  QPushButton *_selectedTimeLabel;
  RadxTime _selectedTime;

  QPushButton *_back1;
  QPushButton *_fwd1;
  QPushButton *_backPeriod;
  QPushButton *_fwdPeriod;
  QPushButton *_boundaryEditorClearBtn;
  QPushButton *_boundaryEditorSaveBtn;
  QListWidget *_boundaryEditorList;

  // time controller settings dialog
  
  QDialog *_timeControl;
  bool _timeControlPlaced;

  int _nArchiveScans;
  vector<string> _archiveFileList;
  int _archiveScanIndex;
  bool _archiveFilesHaveDayDir;

  // time slider

  QFrame *_timePanel;
  QVBoxLayout *_timeLayout;

  QSlider *_timeSlider;

  RadxTime _archiveIntermediateTime;

  RadxTime _startDisplayTime;
  RadxTime _currentDisplayTime;  // is this needed??
  RadxTime _endDisplayTime;
  RadxTime _imagesArchiveStartTime;
  RadxTime _imagesArchiveEndTime;
  int _imagesScanIntervalSecs;

  // saving images in real time mode

  RadxTime _imagesScheduledTime;

  //////////////////////////////
  // private methods

  // open File 

  void _openFile();
  void _saveFile();
  void _moveUpDown();
  string _getOutputPath(bool interactive, string &outputDir, string fileExt);

  // set top bar

  virtual void _setTitleBar(const string &radarName);
  
  // local methods

  void _clear();
  void _setupWindows();
  void _createActions();
  void _createMenus();

  // data retrieval

  void _handleRealtimeData(QTimerEvent * event);
  void _handleArchiveData(QTimerEvent * event);
  int _getArchiveData();
  void _plotArchiveData();
  void _updateArchiveData(vector<string> &fieldNames);
  void _updateArchiveData(QStringList newFieldNames);
  void _setupVolRead(RadxFile &file);
  void _handleColorMapChangeOnRay(RadxPlatform &platform, // RadxRay *ray, 
				  string fieldName);
  void _updateColorMap(string fieldName);

  //  int _applyDataEdits(RadxVol _editedVol);  // & or * ??
  void _applyDataEdits(); // const RadxVol &editedVol);
  void _addNewFields(vector<DisplayField *> newFields);

  // draw beam

  void _handleRay(RadxPlatform &platform, RadxRay *ray);
  void _handleRayUpdate(RadxPlatform &platform, RadxRay *ray,
			vector<string> &newFieldNames);

  // ray handling for display

  void _storeRayLoc(const RadxRay *ray, const double az,
		    const double beam_width, RayLoc *ray_loc);
  void _clearRayOverlap(const int start_index, const int end_index,
			RayLoc *ray_loc);

  // modes

  void _setArchiveMode(bool state);
  void _activateRealtimeRendering();
  void _activateArchiveRendering();

  // archive mode

  void _setGuiFromArchiveStartTime();
  void _setGuiFromArchiveEndTime();
  void _setGuiFromSelectedTime();
  void _setSweepPanelVisibility();

  // time slider

  void _createTimeControl();

  // override howto and boundaryEditor

  void _howto();

private slots:

  //////////////
  // Qt slots //
  //////////////

  // override

  virtual void _freeze();
  virtual void _unzoom();
  virtual void _refresh();
  virtual void _changeField(int fieldId, bool guiMode = true);

  // sweeps

  void _createSweepPanel();
  void _createSweepRadioButtons();
  void _clearSweepRadioButtons();
  void _changeSweep(bool value);
  void _changeSweepRadioButton(int value);

  // local

  void _ppiLocationClicked(double xkm, double ykm,
                           const RadxRay *closestRay);
  void _rhiLocationClicked(double xkm, double ykm,
                           const RadxRay *closestRay);
  void _locationClicked(double xkm, double ykm,
                        RayLoc *ray_loc, const RadxRay *ray);

  // modes
  
  void _setRealtime(bool enabled);

  // archive mode
  
  void _setArchiveStartTime(const RadxTime &rtime);
  void _setArchiveEndTime(const RadxTime &rtime);
  void _setArchiveStartTimeFromGui(const QDateTime &qdt);
  void _setArchiveEndTimeFromGui(const QDateTime &qdt);
  void _acceptGuiTimes();
  void _cancelGuiTimes();

  void _goBack1();
  void _goFwd1();
  void _goBackPeriod();
  void _goFwdPeriod();

  void _setArchiveRetrievalPending();

  // time controller

  void _showTimeControl();
  void _placeTimeControl();

  // time slider

  void _timeSliderActionTriggered(int action);
  void _timeSliderValueChanged(int value);
  void _timeSliderReleased();
  void _timeSliderPressed();
  
  // images

  void _saveImageToFile(bool interactive = true);
  void _createRealtimeImageFiles();
  void _createArchiveImageFiles();
  void _createImageFilesAllSweeps();
  void _createImageFiles();

  // open file 

  void _createFileChooserDialog();
  void _refreshFileChooserDialog();
  void _showFileChooserDialog();

  // context editing (SOLO)
  void ShowContextMenu(const QPoint &pos);

  // boundary editor
  void _createBoundaryEditorDialog();
  void _showBoundaryEditor();
  void _clearBoundaryEditorClick();
  void onBoundaryEditorListItemClicked(QListWidgetItem* item);
  void _saveBoundaryEditorClick();

};

#endif

