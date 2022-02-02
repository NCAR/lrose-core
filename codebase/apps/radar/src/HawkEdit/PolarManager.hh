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
//#include "DisplayManager.hh"
#include "RayLocationController.hh"
#include "ContextEditingView.hh"
#include "ClickableLabel.hh"
#include "ParameterColorView.hh"
#include "FieldColorController.hh"
#include "SweepView.hh"
#include "SweepController.hh"
#include "DisplayFieldView.hh"
#include "SpreadSheetController.hh"
#include "SpreadSheetView.hh"
#include "ScriptEditorController.hh"
#include "ScriptEditorView.hh"
#include "BoundaryPointEditor.hh"
#include "BoundaryPointEditorView.hh"
#include "TimeNavView.hh"
#include "TimeNavController.hh"
#include "UndoRedoController.hh"
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

class PolarManager : public QMainWindow { // public DisplayManager {
  
  Q_OBJECT

public:
  static PolarManager* Instance();

  // boundary editor dialog
  QDialog *_boundaryEditorDialog;
  QGridLayout *_boundaryEditorDialogLayout;

  // constructor
  
  PolarManager(DisplayFieldController *displayFieldController,
	       //               const vector<DisplayField *> &fields,
               bool haveFilteredFields);
  
  // destructor
  
  ~PolarManager();

  // run 

  int run(QApplication &app);
  //int run(QApplication &app, bool noFilename);  

  // enable the zoom button - called by PolarWidget

  void enableZoomButton() const;

  // override event handling

  //void timerEvent (QTimerEvent * event);
  void resizeEvent (QResizeEvent * event);
  void keyPressEvent(QKeyEvent* event);

  // check on archive mode
  
  bool checkArchiveMode() const { return _archiveMode; }

  // input file list for archive mode

  void setArchiveFileList(const vector<string> &list,
                          bool fromCommandLine = true);
  
  // load archive file list by searching for files
  // returns 0 on success, -1 on failure
  
  //int loadArchiveFileList();


  //  const RadxVol getDataVolume();

  double getSelectedSweepAngle();
  size_t getSelectedFieldIndex();

  vector<string> *getFieldsArchiveData(string fileName);
  vector<string> *userSelectFieldsForReading(string fileName);
  void getFileAndFields();

// from DisplayManager ...

  // get selected name and units

  const string &getSelectedFieldLabel() const { return _selectedLabel; }
  const string &getSelectedFieldName() const { return _selectedName; }
  const string &getSelectedFieldUnits() const { return _selectedUnits; }
  // const DisplayField &getSelectedField() const { return _displayFieldController->getField(_fieldNum); }
  // const vector<DisplayField *> &getDisplayFields() const { return _fields; }
  //  const DisplayField &getSelectedField() const { return *_fields[_fieldNum]; }
  //  const vector<DisplayField *> &getDisplayFields() const { return _fields; }

  // location

  double getRadarLat() const { return _radarLat; }
  double getRadarLon() const { return _radarLon; }
  double getRadarAltKm() const { return _radarAltKm; }
  const RadxPlatform &getPlatform() const { return _platform; }


  // enable the zoom button
  
  //virtual void enableZoomButton() const = 0;

  //virtual double getSelectedSweepAngle() {return 0.0;}
  //virtual size_t getSelectedFieldIndex() {return 0;}

// end from DisplayManager



//  void clearBoundaryEditorClick();
//  void helpBoundaryEditorClick();
//  void polygonBtnBoundaryEditorClick();
//  void circleBtnBoundaryEditorClick();
//  void brushBtnBoundaryEditorClick();
//  void onBoundaryEditorListItemClicked(QListWidgetItem* item);

  void selectBoundaryTool(BoundaryToolType tool);
  void drawBoundary(WorldPlot &_zoomWorld, QPainter &painter);
  void mouseMoveEvent(int worldX, int worldY);  
  bool evaluateCursor(bool isShiftKeyDown);
  void addDeleteBoundaryPoint(double mouseReleaseX, double mouseReleaseY, 
    bool isShiftKeyDown);
  bool isOverBoundaryPoint(double worldX, double worldY);
  bool moveBoundaryPoint(double worldPressX, double worldPressY,
  double worldReleaseX, double worldReleaseY);
  bool evaluateRange(double xRange);

  //void runForEachRayScript(QString script, bool useBoundary,
  //  bool useAllSweeps);

 // void runScriptBatchMode(QString script, bool useBoundary, 
 //   bool useAllSweeps, string saveDirectoryPath,
 //   string startDateTime, string endDateTime);

  //void runScriptBatchMode(QString script, bool useBoundary, 
  //  bool useAllSweeps, bool useTimeRange);
  /*
  void runScriptBatchMode(QString script, bool useBoundary, 
  bool useAllSweeps, string saveDirectoryPath, 
  int startYear, int startMonth, int startDay,
  int startHour, int startMinute, int startSecond,
  int endYear, int endMonth, int endDay,
  int endHour, int endMinute, int endSecond);
  */

  void closeEvent(QEvent *event);

public slots:
  void fieldsSelected(vector<string> *selectedFields);
  void closeFieldListDialog(bool clicked);

  void _openFile();
  void _saveFile();
  void _howto();   

  //void contextMenuParameterColors();
  void ShowParameterColorDialog(QString fieldName);

  //colorMapRedefineReceived(string, ColorMap)
  void colorMapRedefineReceived(string fieldName, ColorMap newColorMap,
				QColor gridColor,
				QColor emphasisColor,
				QColor annotationColor,
				QColor backgroundColor);
  void setVolume(); // const RadxVol &radarDataVolume);
  void setDataMissing(string fieldName, float missingValue);
  // TODO:
  // Good. Now go and read about Q_DECLARE_METATYPE(). Or better yet, use QStringList instead of std::vector<std::string>.
  void updateVolume(QStringList newFieldNames);
  void _volumeDataChanged(QStringList newFieldNames);
  void _addNewFields(QStringList newFieldNames);
  void _addNewFields(vector<string> *newFieldNames);
  void selectedFieldChanged(QString newFieldName);
  void selectedFieldChanged(string fieldName);
  //void _updateField(size_t fieldId);

  void selectedSweepChanged(double);
  void dataFileChanged();

  void spreadSheetClosed();
  void scriptEditorClosed();
  void boundaryEditorClosed();
  void showBoundaryEditor();
  void refreshBoundaries();
  void boundaryCircleRadiusChanged(int value);
  void boundaryBrushRadiusChanged(int value);
  void saveBoundaryEvent(int boundaryIndex);
  void loadBoundaryEvent(int boundaryIndex);  
  void _clearBoundaryEditorClick(); 
  //void boundaryColorChanged(QColor newColor);

  void setFieldToMissing(QString fieldName);
  void deleteFieldFromVolume(QString fieldName);   

  void spreadsheetDataChanged();

  void newTimeSelected(int value);
  void startEndTimeChanged(int startYear, int startMonth, int startDay,
                       int startHour, int startMinute, int startSecond,
                       int endYear, int endMonth, int endDay,
                       int endHour, int endMinute, int endSecond);
  void resetStartEndTime();

  void runForEachRayScript(QString script, bool useBoundary, bool useAllSweeps);
  void runScriptBatchMode(QString script, bool useBoundary, 
    bool useAllSweeps, bool useTimeRange);
  void undoScriptEdits(); // bool batchMode = false);
  void redoScriptEdits(); // bool batchMode = false);
  void cancelScriptRun();

  void errorMessage(string title, string message);
  int saveDiscardMessage(string text, string question);

  //void close();

signals:

// from DisplayManager ...
  ////////////////
  // Qt signals //
  ////////////////

  /**
   * @brief Signal emitted when the main frame is resized.
   *
   * @param[in] width    The new width of the frame.
   * @param[in] height   The new height of the frame.
   */
  
  void frameResized(const int width, const int height);
  //void setParamsFile();

  void addField(QString fieldName);

  void newDataFile();

// end from DisplayManager

private:

  static PolarManager* m_pInstance;

  string _boundaryDir;
  void setBoundaryDir();
  string getBoundaryFilePath(string boundaryFileName);

  // from DisplayManager ...
  ParamFile *_params;
  
  // reading data in
  
  Reader *_reader;
  vector<const RadxRay *> _rays;
  bool _initialRay;
  
  // instrument platform details 

  RadxPlatform _platform;
  
  // beam reading timer

  //static bool _firstTimerEvent;
  //int _beamTimerId;
  //bool _frozen;

  // data fields
  //  vector <DisplayField *> _fields;
  DisplayFieldController *_displayFieldController;
  bool _haveFilteredFields;
  int _rowOffset;

  SpreadSheetController *spreadSheetControl;
  SpreadSheetView *sheetView;

  ScriptEditorController *scriptEditorControl;
  ScriptEditorView *scriptEditorView;

  BoundaryPointEditor *boundaryPointEditorControl;
  BoundaryPointEditorView *boundaryPointEditorView;
  BoundaryView *boundaryView;

  UndoRedoController *_undoRedoController;


  // windows

  QFrame *_main;

  // actions
  
  QAction *_exitAct;
  //QAction *_freezeAct;
  QAction *_clearAct;
  QAction *_unzoomAct;
  QAction *_refreshAct;
  QAction *_showClickAct;
  QAction *_showBoundaryEditorAct;
  QAction *_howtoAct;
  QAction *_aboutAct;
  QAction *_aboutQtAct;
  QAction *undoAct;
  QAction *redoAct;
  //QAction *_openFileAct;
  //QAction *_saveFileAct;

  // status panel

  QGroupBox *_statusPanel;
  QGridLayout *_statusLayout;

  QLabel *_radarName;
  QLabel *_dateVal;
  QLabel *_timeVal;

  QLabel *_volNumVal;
  QLabel *_sweepNumVal;

  QLabel *_fixedAngVal;
  QLabel *_elevVal;
  QLabel *_azVal;

  QLabel *_nSamplesVal;
  QLabel *_nGatesVal;
  QLabel *_gateSpacingVal;

  QLabel *_pulseWidthVal;
  QLabel *_prfVal;
  QLabel *_nyquistVal;
  QLabel *_maxRangeVal;
  QLabel *_unambigRangeVal;
  QLabel *_powerHVal;
  QLabel *_powerVVal;

  QLabel *_scanNameVal;
  QLabel *_sweepModeVal;
  QLabel *_polModeVal;
  QLabel *_prfModeVal;

  QLabel *_latVal;
  QLabel *_lonVal;

  QLabel *_altVal;
  QLabel *_altLabel;

  QLabel *_altRateVal;
  QLabel *_altRateLabel;
  double _prevAltKm;
  RadxTime _prevAltTime;
  double _altRateMps;

  QLabel *_speedVal;
  QLabel *_headingVal;
  QLabel *_trackVal;

  QLabel *_sunElVal;
  QLabel *_sunAzVal;

  bool _altitudeInFeet;

  vector<QLabel *> _valsRight;
  
  // field panel
  
  DisplayFieldView *_fieldPanel;
  QGridLayout *_fieldsLayout;
  QLabel *_selectedLabelWidget;
  QButtonGroup *_fieldGroup;
  vector<QRadioButton *> _fieldButtons;
  DisplayField *_selectedField;
  string _selectedName;
  string _selectedLabel;
  string _selectedUnits;
  QLabel *_valueLabel;
  //int _fieldNum;
  int _prevFieldNum;

  // click location report dialog
  QDialog *_clickReportDialog;
  QGridLayout *_clickReportDialogLayout;
  QLabel *_dateClicked;
  QLabel *_timeClicked;
  QLabel *_elevClicked;
  QLabel *_azClicked;
  QLabel *_gateNumClicked;
  QLabel *_rangeClicked;
  QLabel *_altitudeClicked;
  
  // sun position calculator
  double _radarLat, _radarLon, _radarAltKm;
  SunPosn _sunPosn;

  // set top bar

  //virtual void _setTitleBar(const string &radarName) = 0;

  // panels
  
  void _createStatusPanel();
  void _createFieldPanel();
  void _updateFieldPanel(string newFieldName);

  void _createClickReportDialog();
  void _updateStatusPanel(const RadxRay *ray);
  double _getInstHtKm(const RadxRay *ray);

  // setting text

  void _setText(char *text, const char *format, int val);
  void _setText(char *text, const char *format, double val);
  
  // adding vals / labels

  QLabel *_newLabelRight(const string &text);

  QLabel *_createStatusVal(const string &leftLabel,
                           const string &rightLabel,
                           int row, 
                           int fontSize,
                           QLabel **label = NULL);
  
  QLabel *_addLabelRow(QWidget *widget,
                       QGridLayout *layout,
                       const string &leftLabel,
                       const string &rightLabel,
                       int row,
                       int fontSize = 0);
/*
  QLineEdit *_addInputRow(QWidget *widget,
                          QVBoxLayout *layout,
                          const string &leftLabel,
                          const string &rightContent,
                          int fontSize = 0,
                          QLabel **label = NULL);

  QLineEdit *_addInputRow(QWidget *parent,
                          QVBoxLayout *layout,
                          const string &leftLabel,
                          const string &rightContent,
                          int fontSize = 0,
                          QFrame **framePtr = NULL);
*/
  // end from DisplayManager


  bool _firstTime;
  bool _urlOK;

  // beam geometry
  
  int _nGates;
  double _maxRangeKm;

  //RayLoc* _ppiRayLoc; // for use, allows negative indices at north line
  //RayLoc* _ppiRays;   // for new and delete
  RayLocationController *_rayLocationController;
  // input data
  
  RadxTime _readerRayTime;
  //RadxVol _vol;

  // sweeps

  SweepController *_sweepController;
  //QVBoxLayout *_sweepVBoxLayout;
  SweepView *_sweepPanel;
  //vector<QRadioButton *> *_sweepRButtons;

  // windows

  QFrame *_ppiFrame;
  PolarWidget *_ppi;

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
  QMenu *_editMenu;
  QMenu *_boundaryMenu;

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

  QAction *_examineAct;
  QAction *_editAct;

  // archive mode
  
  bool _archiveMode;
  bool _archiveRetrievalPending;

  bool _cancelled;

/*
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
  */
  /*
  QPushButton *_boundaryEditorClearBtn;
  QPushButton *_boundaryEditorHelpBtn;
  QPushButton *_boundaryEditorSaveBtn;
  QPushButton *_boundaryEditorPolygonBtn;
  QPushButton *_boundaryEditorCircleBtn;
  QPushButton *_boundaryEditorBrushBtn;
  QListWidget *_boundaryEditorList;
  QLabel *_boundaryEditorInfoLabel;
  bool forceHide = true;
  QSlider *_circleRadiusSlider;
  QSlider *_brushRadiusSlider;

  QPushButton *_boundaryEditorClearBtn;
  QPushButton *_boundaryEditorSaveBtn;
  QListWidget *_boundaryEditorList;
  */

  // time controller settings dialog
  
  TimeNavView *_timeNavView;
  TimeNavController *_timeNavController;
  bool _timeControlPlaced;

  //int _nArchiveScans;
  //vector<string> _archiveFileList;
  //int _archiveScanIndex;
  //bool _archiveFilesHaveDayDir;

  // time slider

  //QFrame *_timePanel;
  //QVBoxLayout *_timeLayout;

  //QSlider *_timeSlider;
/*
  RadxTime _archiveIntermediateTime;

  RadxTime _startDisplayTime;
  RadxTime _currentDisplayTime;  // is this needed??
  RadxTime _endDisplayTime;
  */
  RadxTime _imagesArchiveStartTime;
  RadxTime _imagesArchiveEndTime;
  

  int _imagesScanIntervalSecs;

  // saving images in real time mode

  RadxTime _imagesScheduledTime;

  QDialog *fieldListDialog;

  //////////////////////////////
  // private methods

  // open File 


  void _moveUpDown();
  string _getOutputPath(bool interactive, string &outputDir, string fileExt);

  // set top bar

  void _setTitleBar(const string &radarName);
  
  // local methods

  //void _clear();
  void _setupWindows();
  void _createActions();
  void _createMenus();

  // data retrieval

  void _readDataFile(vector<string> *selectedFields);
  void _readDataFile2();
  void _readDataFile2(string &inputPath);

  // handleArchiveData calls:
  // getArchiveData
  // plotArchiveData
  // activateArchiveRendering

  // volumeDataChanged (from script or editor) calls:
  // addNewFields
  // fieldPanel->update()
  // updateArchiveData  --> calls _handleRayUpdate for each ray of selected sweep
  // activateArchiveRendering

  // plotArchiveData vs. activateArchiveRendering?
  // activateArchiveRendering calls _ppi->ActivateArchiveRendering ... FieldRenderers
  // plotArchiveData gets selected Sweep, 
  //   then calls handleRay for each ray of the sweep. ???
  //      handleRay and handleRayUpdate both replace missing data with fill value (TODO: move to displayFieldX)
  //          then call _storeRayLoc
  // ray_loc used to determine which ray is closest to click point. 
  // Q: What about _ppiRayLoc? this is where the index is used. Ah, _ppiRayLoc 
  //       an instance of RayLoc.


  //void _handleArchiveData();
  int _getArchiveData();
  int _getArchiveData(string &inputPath);
  int _getArchiveDataPlainVanilla(string &inputPath);
  void _plotArchiveData();
  //void _updateArchiveData(vector<string> &fieldNames);
  //void _updateArchiveData(QStringList newFieldNames);
  void _setupVolRead(RadxFile &file);
  void _handleColorMapChangeOnRay(RadxPlatform &platform, // RadxRay *ray, 
				  string fieldName);
  //void _updateColorMap(string fieldName);

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

  void _setupRayLocation();
  void _setMaxRangeKm();

  // modes

  //void _setArchiveMode(bool state);
  //void _activateRealtimeRendering();
  //void _activateArchiveRendering();

  // archive mode

  void _setGuiFromArchiveStartTime();
  void _setGuiFromArchiveEndTime();
  void _setGuiFromSelectedTime();
  //void _setSweepPanelVisibility();

  bool _checkForUnsavedBatchEdits();
  //void _saveTempDir();

  // time slider

  void _createTimeControl();
  void _createUndoRedoStack();

  bool _unSavedEdits = false;

  string _getSelectedFile();
  string _getFileNewVersion(int archiveFileIndex);

private slots:

  //////////////
  // Qt slots //
  //////////////

 // from DisplayManager ...
  //virtual void _howto();
  void _about();
  void _showClick();
  //virtual void _freeze() = 0;
  //virtual void _unzoom() = 0;
  //virtual void _refresh() = 0;
  //virtual void _changeField(int fieldId, bool guiMode) = 0;
  //virtual void _openFile();
  //virtual void _saveFile();


  //void _changeFieldVariable(bool value);
  int _updateDisplayFields(vector<string> *fieldNames);
  // end from DisplayManager


  //void _freeze();
  void _unzoom();
  void _refresh();
  //void _changeField(int fieldId, bool guiMode = true);
  void changeToField(QString newFieldName);
  // sweeps

  //void _createSweepPanel();
  //void _createSweepRadioButtons();
  //void _clearSweepRadioButtons();
  //void _changeSweep(bool value);
  void _changeSweepRadioButton(int value);

  // local

  void _ppiLocationClicked(double xkm, double ykm,
                           const RadxRay *closestRay);
  void _rhiLocationClicked(double xkm, double ykm,
                           const RadxRay *closestRay);
  void _locationClicked(double xkm, double ykm,
                        //RayLoc *ray_loc, 
                        const RadxRay *ray);

  // modes
  
  //void _setRealtime(bool enabled);

  // archive mode
  
  //void _setArchiveStartTime(const RadxTime &rtime);
  //void _setArchiveEndTime(const RadxTime &rtime);
  //void _setArchiveStartTimeFromGui(const QDateTime &qdt);
  //void _setArchiveEndTimeFromGui(const QDateTime &qdt);
  //void _acceptGuiTimes();
  //void _cancelGuiTimes();

  //void _goBack1();
  //void _goFwd1();
  //void _goBackPeriod();
  //void _goFwdPeriod();

  //void _setArchiveRetrievalPending();

  // time controller

  void _showTimeControl();
  //void _placeTimeControl();

  // time slider

  //void _timeSliderActionTriggered(int action);
  //void _timeSliderValueChanged(int value);
  //void _timeSliderReleased();
  //void _timeSliderPressed();

  //circle radius slider for BoundaryPointEditor
  //void _circleRadiusSliderValueChanged(int value);
  //void _brushRadiusSliderValueChanged(int value);
  
  // images

  void _saveImageToFile(bool interactive = true);
  //void _createRealtimeImageFiles();
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
  //void _createBoundaryEditorDialog();



  //void onBoundaryEditorListItemClicked(QListWidgetItem* item);
  //void _saveBoundaryEditorClick();

  void _examineSpreadSheetSetup(double  closestAz = 30.0, double range = 0.0);
  void ExamineEdit(double azimuth, double elevation, size_t fieldIndex,
    double range);

  void _scriptEditorSetup();
  void EditRunScript();

};

#endif

