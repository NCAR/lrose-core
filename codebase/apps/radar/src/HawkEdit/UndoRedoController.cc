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
// UndoRedoController.hh
//
// Coordinates undo and redo activities
//
// Brenda Javornik, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2022
//
///////////////////////////////////////////////////////////////
//
// coordinates undo and redo of edits to archive files
// 
///////////////////////////////////////////////////////////////

 #include <iostream>

#include "UndoRedoController.hh"

UndoRedoController::UndoRedoController() {

	_model = new UndoRedoModel();
  //_tempDirIndex = -1;

}

UndoRedoController::~UndoRedoController() {
  cerr << "UndoRedoController destructor called" << endl;

  if (_model != NULL) { 
    delete _model;
  }
}

void UndoRedoController::reset(string path, int nFiles) {
  _model->setBaseDir(path, nFiles);
}

string UndoRedoController::getPreviousVersion(int fileIndex) {
  return _model->undo(fileIndex);
}

string UndoRedoController::getNextVersion(int fileIndex) {
  return _model->redo(fileIndex);
}

string UndoRedoController::getCurrentVersion(int fileIndex) {
  return _model->getCurrentVersion(fileIndex);
}

string UndoRedoController::getNewVersion(int fileIndex) {
  return _model->moveToNextVersion(fileIndex);
}

void UndoRedoController::waterMarkVersion() {
  _model->makeNewBatch();
}

void UndoRedoController::undoBatchMode() {
  _model->batchUndo();
}

void UndoRedoController::redoBatchMode() {
  _model->batchRedo();
}

/*
string &UndoRedoController::getSelectedArchiveFile() {
  return _model->getSelectedArchiveFile();
}

string UndoRedoController::getSelectedPath() {
  return _model->getCurrentPath();
}

// compare nextTempDir base directory to the base directory
// in the temp stack
bool UndoRedoController::_isDifferentBaseDir(string nextTempDir) {
  bool different = true;
  if (_tempDirStack.size() > 0) {
    // then there is a previous base directory
    // TODO: compare up to /.tmp_N of nextTempDir
    // with _tempDirStack.at(0)
    string previousBaseDir = _tempDirStack.at(0);
    // base/Goshen_tornado_2009/DOW6  ==> previous base
    // base/Goshen_tornado_2009/DOW6/.tmp_N  ==> nextTempDir
    int result = nextTempDir.compare(0, previousBaseDir.size(),
      previousBaseDir);
    if (result == 0) {
      different = false;
    } else {
      different = true;
    } 
  }
  return different;
}

// The undo/redo stack ...
// 
// batch mode, the directories are:
//  base/       yyyymmdd/cfrad.*.nc
//  base/.tmp_N/yyyymmdd/cfrad.*.nc
// stack:
//  base
//  base/.tmp_N 
// the stack needs to contain the dir above the yyyymmdd dir

// individual scan mode, i.e. editing just one file, or no day dir
//  base/         cfrad.*.nc
//  base/.tmp_N/  cfrad.*.nc 

//  What to keep in the stack?
//  base
//  base/.tmp_N

// getArchiveFiles must handle both cases: with dayDir and no dayDir!


// push base dir on the stack, and clear the stack when
// setting a new base dir

string UndoRedoController::getTempDir() {
  string nextTempDir = _model->getTempDir();

  if (_tempDirStack.empty()) {
    _setBaseDirTempStack();
  } else {
    // check if the base dirs are different
    if (_isDifferentBaseDir(nextTempDir)) {
      _resetTempStack();
      _setBaseDirTempStack();
    }
  }

  // push temp dir WITHOUT day dir, to make save easier (just rename)
  // and make RadxFileList work because it looks for a day Dir
  string noDayDir = nextTempDir;
  if (_model->archiveFilesHaveDayDir()) {
    noDayDir.erase(noDayDir.size()-9, 9);
  }
  _tempDirStack.push_back(noDayDir); 
  _tempDirIndex += 1;
  // BUT, return path with day Dir if the archive files have it
  return nextTempDir;
}

string UndoRedoController::getSelectedArchiveFileName() {
  return _model->getSelectedArchiveFileName();
}

// return true if the current directory is a temp dir
// i.e. of the form .../.tmp_N/yyyymmdd
bool UndoRedoController::isSelectedFileInTempDir() {
  return (_tempDirIndex >= 0);
}

// Temp stack for undo/redo 
// maybe move to a separate class??
// The bottom of the stack (index = 0)
// is the base directory for the temp files

// ALWAYS point at current directory!!!
//
// return the previous temp directory, or the base
// directory if we are out of the temp dirs in the stack,
// i.e. idx < 0; or use a stack????
// return empty string, if there are no more directories
// in the stack
string UndoRedoController::getPreviousTempDir() {
  if (_tempDirIndex <= 0) {
    return "";
  } else {
    _tempDirIndex -= 1;
    string previousDir = _tempDirStack.at(_tempDirIndex);
    return previousDir;
  }
}

// return the next temp directory, or the
// empty string if out of the temp dirs in the stack,
// i.e. idx < 0; or use a stack????
// return empty string, if there are no more directories
// in the stack
string UndoRedoController::getNextTempDir() {
  if (_tempDirIndex >= _tempDirStack.size()-1) {
    return "";
  } else {
    _tempDirIndex += 1;
    string nextDir = _tempDirStack.at(_tempDirIndex);
    return nextDir;
  }
}

void UndoRedoController::_resetTempStack() {

  // delete all temporary directories in stack
  _removeTempDirs();
  // clear the undo/redo stack
  _tempDirStack.clear();
}

void UndoRedoController::_setBaseDirTempStack() {
  // set the base directory for any edits
  _tempDirIndex = 0;
  _tempDirStack.push_back(_model->getCurrentPath());
}

bool UndoRedoController::moreFiles() {
  return _model->moreFiles();
}

void UndoRedoController::getBounds(bool useTimeRange, int *firstArchiveFileIndex,
    int *lastArchiveFileIndex) {
  if (useTimeRange) {
    *firstArchiveFileIndex = 0;
    *lastArchiveFileIndex = _model->getNArchiveFiles() - 1;
  } else {
    *firstArchiveFileIndex = _model->getPositionOfSelection();
    *lastArchiveFileIndex = *firstArchiveFileIndex;
  }
}


bool UndoRedoController::_isTempDir(string *path) {
  bool found = false;
  if (path->find(".tmp_") != string::npos) {
    found = true;
  } 
  return found; 
}

// move/rename the current selected temp directory
// to the newName
void UndoRedoController::replaceSelectedTempPath(string newName) {
  _tempDirStack[_tempDirIndex] = newName;
}

// before changing to a new base directory, prompt to save any temp dirs?
// otherwise, the temp dirs will be deleted.
// new method ... movingToNewBaseDir ...
// if moving to new base dir, clear the stack
void UndoRedoController::_removeTempDirs() {
  //vector<string>::iterator it;
  if (_tempDirStack.size() > 1) {

    // clean up temp dirs
    string command = "rm -r ";
    command.append(_tempDirStack.at(0)); //  getSelectedPath());
    command.append("/.tmp_*");

    int result = std::system(command.c_str()); // execute the UNIX command "mv -f oldName newName"

    //  The rm utility exits 0 on success, and >0 if an error occurs.
    if (result >0) {
      // TODO: handle error condition ...
      //errorMessage("Error", "Save batch files failed.");
      // TODO: try "mv -f src dest > test.txt"
      //  std::cout << std::ifstream("test.txt").rdbuf();
      // to catch any error information.
    } else {

    }
  }
  //
}

*/

