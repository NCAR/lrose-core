// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2021                                         
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
// UndoRedoModel.hh
//
// Storage of edited versions of archive files
//
// Brenda Javornik, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2021
//
///////////////////////////////////////////////////////////////
//
// Holds the temporary file versions
// The time slider provides an file index; the undo/redo 
// controller provides the current version of the file.
// The script editor also uses this class to get a list
// of files on which to run a script.  
// 
///////////////////////////////////////////////////////////////

#ifndef UndoRedoController_HH
#define UndoRedoController_HH

#include "UndoRedoModel.hh"


class UndoRedoController {

public:

  UndoRedoController();

  // use this constructor for utility functions, without GUI.
  //UndoRedoController();

  ~UndoRedoController();

  void clear();
  // remove .tmp dir // removes all temp versions; clears the stacks

  void reset(string path, int nFiles);
  // set up for a new data set with nFiles

  bool undo(int fileIdx);
  //find the previous version of the file in the temp dir; change to this file.
  // return true if previous version available

  bool redo(int fileIdx);
  //find the next version of the file in the temp dir; change to this file.
  // return true if next version available

  //writeToVersion(currentFile, tempFile_N) leave to DataModel
  //set currentVersion for the file.

  string getNewVersion(int fileIndex);
 //  or (path/file)
  //.tmp/file_1/vN

  string getNextVersion(int fileIdx);
  string getCurrentVersion(int fieldIdx);
  string getPreviousVersion(int fileIndex);

  void waterMarkVersion();
  void undoBatchMode();
  void redoBatchMode();

  //save(int fileIdx) leave to  DataModel
  //fn = getCurrentVersion(file#) 
  // move (fn, savePath)

  //saveAll(bool overwrite, path) leave to DataModel
  //saves current version of all files


  /* Events
Event, Undo/Redo Stack (aka. FileVersionMC no view?
File->Open,  Save All
             Clear
Script/Spreadsheet->Undo/Redo, undo/redo (currentFile)
Spreadsheet->Apply,   getNextVersion(currentFileIndex)
                      writeToVersion(...)
Spreadsheet->Save,    save(currentFile)
Script->Run (indiv),  getNextVersion(currentFile)
                      writeToVersion(...)
Script->Run (batch),  for each file in set
                        getNextVersion(currentFile)
                        writeToVersion(...)
TimeNav->selectFile,  getCurrentVersion(file#).

*/

/*

  void timeSliderValueChanged(int value);
  void timeSliderReleased(int value);
  void setTimeSliderPosition(int value);

  void fetchArchiveFiles(string seedFileName);
  void fetchArchiveFiles(string seedPath, string seedFileName,
    string fullUrl,
    bool keepTimeRange = false);

  string &getSelectedArchiveFile();
  string getSelectedArchiveFileName();
  string getTempDir();
  string getPreviousTempDir();
  string getNextTempDir();
  bool isSelectedFileInTempDir();

  void _preFileOpen();

  string getSelectedPath();


  void _setArchiveStartEndTimeFromGui(int startYear, int startMonth, int startDay,
                       int startHour, int startMinute, int startSecond,
                       int endYear, int endMonth, int endDay,
                       int endHour, int endMinute, int endSecond);


  bool moreFiles();
  void getBounds(bool useTimeRange, int *firstArchiveFileIndex,
    int *lastArchiveFileIndex);
  void replaceSelectedTempPath(string newName);
*/

private:


  void _setBaseDirTempStack();
  void _resetTempStack();
  string _no_yyyymmdd(string s);

  bool _isTempDir(string *path);

  bool _isDifferentBaseDir(string nextTempDir);
  void _removeTempDirs();

 // UndoRedoView *_view;
  UndoRedoModel *_model;

};

#endif