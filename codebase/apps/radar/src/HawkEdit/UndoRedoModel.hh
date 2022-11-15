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
// Storage of archive file names indexed by date and time
//
// Brenda Javornik, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2021
//
///////////////////////////////////////////////////////////////
//
// Undo/Redo stack as temp directory
// keep undo/redo stack in temp directory, 
// one directory for each archive file in set
//
// basedir/.tmp/file_1/v1
//             /file_2
//             /file_100/v1
//                      /v2
//
//  where v1, v2 are temporary names for each version of file_N
//  The actual name of file_N is kept in the Time Nav.
// 
// Undo/Redo stack just keeps track of versions, and the base dir.
// 
///////////////////////////////////////////////////////////////

#ifndef UndoRedoModel_HH
#define UndoRedoModel_HH

#include <string>
#include <vector>

#include <Radx/RadxPath.hh>

#include "UndoRedoModel.hh"


using namespace std;

class UndoRedoModel {

public:

  UndoRedoModel();

  ~UndoRedoModel();

  void setBaseDir(string path, int nFiles);

  void clear();
  // remove .tmp dir // removes all temp versions; clears the stacks

  string undo(int fileNum);
  //find the previous version of the file in the temp dir; change to this file.

  string redo(int fileNum);
  //find the next version of the file in the temp dir; change to this file.

  //writeToVersion(currentFile, tempFile_N)
  //set currentVersion for the file.

  // can save to the file path returned
  string moveToNextVersion(int fileNum); //  or (path/file)
  //.tmp/file_1/vN

  string getCurrentVersion(int fileNum);
  int getCurrentVersionNum(int fileNum);

  //save(int fileNum)
  //fn = getCurrentVersion(file#) 
  // move (fn, savePath)

  //saveAll(bool overwrite, path)
  //saves current version of all files

  // move all the version numbers from the batch to the currentVersion 
  void batchUndo();
  void batchRedo();

  // add another batch to the batches store;
  // take the current version of every file, and increment by one.
  // these are the versions to use when writing batch edited files.
  // call at the end of a batch script edit to record 
  // the watermark, i.e. the current version of every file.
  // During batch edit, use moveToNewVersion for each file,
  // then call this function to save the watermark.
  void makeNewBatch();

  // get the  version for this file
  // can save to the file path returned
  //string batchGetVersion(int fileNum);
  // may not need this.  just use moveToNextVersion


private:

  void reset();
  //void _setBaseDirTempStack();
  //void _resetTempStack();
  //string _no_yyyymmdd(string s);

  //bool _isTempDir(string *path);

  //bool _isDifferentBaseDir(string nextTempDir);
  void _removeTempDirs();

  static const string _tmpDir;
  static const string _fileDir;

  bool _validFileNum(int fileNum);
  string _constructFullTempPath(int fileNum, int version);
  void _createDirectory(string path);
  void _printBatchStack();

  //vector<string> _tempDirStack;
  vector<int> currentVersion;
  //int _tempDirIndex;

  // contains watermark of versions for each batch edit
  vector< vector <int> *> batches;

  int _currentBatchIndex; 

  // Keep list (vector) of currentVersion of each file in Time Nav set. 
  // vector <int> currentVersion. 
  // This is one stack for each archive file in Time Nav. 
  // Moving up and down the stack (undo/redo) is moving up and down 
  // in version number in the basedir/.tmp/file_1 dir.
  
  RadxPath baseDir; // keep at model level
  // QDir basedir  // then must move this to View level???

};

#endif