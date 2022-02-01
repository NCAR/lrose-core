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
#include <sstream>

#include "UndoRedoModel.hh"

const string UndoRedoModel::_tmpDir = ".tmp_hawkedit";
const string UndoRedoModel::_fileDir = "file_";

UndoRedoModel::UndoRedoModel() {



}

UndoRedoModel::~UndoRedoModel() {
  cerr << "UndoRedoModel destructor called" << endl;

  _removeTempDirs();
}


void UndoRedoModel::setBaseDir(string path, int nFiles) {
  baseDir.clear();
  baseDir.setDirectory(path);
  currentVersion.clear();
  currentVersion.resize(nFiles);

  for (int i=0; i<nFiles; i++) {
    currentVersion.at(i) = 0;
    stringstream ss;
    ss << path << "/" << _tmpDir << "/" << _fileDir << i;
    RadxPath tmpDir;
    tmpDir.setDirectory(ss.str());
    int result = tmpDir.makeDirRecurse();
    if (result != 0) {
      throw std::invalid_argument("cannot make temporary directory for undo and redo");
    }
  }
  _currentBatchIndex = -1;
  //makeNewBatch(); // record the intial watermark
}


void UndoRedoModel::clear() {
  // remove .tmp dir // removes all temp versions; clears the stacks
  _removeTempDirs();
  currentVersion.clear();
  batches.clear();
}

string UndoRedoModel::_constructFullTempPath(int fileNum, int version) {
  stringstream ss;
  ss << baseDir.getDirectory() << "/" << _tmpDir << "/" << _fileDir << fileNum << 
    "/v" << version;
  return ss.str();
}

// version 0 is the original file

string UndoRedoModel::undo(int fileNum) {
  //find the previous version of the file in the temp dir; change to this file.
  int previousN = currentVersion.at(fileNum) - 1;
  if (previousN <= 0) {
    currentVersion.at(fileNum) = 0;
    return ""; // indicate we are at the end of versions
  } else {
    currentVersion.at(fileNum) = previousN;
    return _constructFullTempPath(fileNum, previousN);
  }
}

string UndoRedoModel::redo(int fileNum) {
  //find the next version of the file in the temp dir; change to this file.
  int nextN = currentVersion.at(fileNum) + 1;
  string nextFullPath = _constructFullTempPath(fileNum, nextN);
  RadxPath nextPath;
  //nextPath.setDirectory(nextFullPath);
  if (!nextPath.exists(nextFullPath)) {
    return ""; // indicate we are at the end of versions
  } else {
    currentVersion.at(fileNum) = nextN;
    return nextFullPath;
  }
  //writeToVersion(currentFile, tempFile_N)
  //set currentVersion for the file.
}

//void UndoRedoModel::moveToNextVersion ???

string UndoRedoModel::moveToNextVersion(int fileNum) { //  or (path/file)
  //.tmp/file_1/vN
  stringstream ss;
  currentVersion.at(fileNum) += 1;
  int nextN = currentVersion.at(fileNum); 
  return _constructFullTempPath(fileNum, nextN);
}

bool UndoRedoModel::_validFileNum(int fileNum) {
  return ((fileNum >= 0) && (fileNum < currentVersion.size()));
} 

string UndoRedoModel::getCurrentVersion(int fileNum) {
  if (_validFileNum(fileNum)) {
    return _constructFullTempPath(fileNum, currentVersion.at(fileNum));
  } else {
    return "";
  }
}

int UndoRedoModel::getCurrentVersionNum(int fileNum) {
  if (_validFileNum(fileNum)) {
    return currentVersion.at(fileNum);
  } else {
    return -1;
  }
}
  //save(int fileNum)
  //fn = getCurrentVersion(file#) 
  // move (fn, savePath)

  //saveAll(bool overwrite, path)
  //saves current version of all files

/*
void UndoRedoModel::fetchArchiveFiles(string seedPath, string seedFileName,
    string fullUrl, bool keepTimeRange) {
  
  int nFilesFound = _model->findArchiveFileList(seedPath, keepTimeRange);

  if (nFilesFound < 1) {
    _model->recoverFromNoDatDirFormat(fullUrl);
  }

  _setGuiFromArchiveStartTime();  
  _setGuiFromArchiveEndTime();
  _model->setSelectedFile(seedFileName);
  _setGuiFromSelectedTime();

  _view->setNTicks(_model->getNArchiveFiles());
  //setSliderPosition(_model->getPositionOfSelection());

  _view->showTimeControl();
}

*/

// move all the version numbers from the batch to the currentVersion 
void UndoRedoModel::batchUndo() {

  if (_currentBatchIndex > 0) {
    _currentBatchIndex -= 1;
    vector<int> *previousBatch = batches.at(_currentBatchIndex);
    for (int i=0; i<currentVersion.size(); i++) {
      currentVersion.at(i) = previousBatch->at(i);
    }
  } else {
    throw std::invalid_argument("no more batch undo");
  }
}

void UndoRedoModel::batchRedo() {
  if (_currentBatchIndex < batches.size() -1) {
    _currentBatchIndex += 1;
    vector<int> *previousBatch = batches.at(_currentBatchIndex);
    for (int i=0; i<currentVersion.size(); i++) {
      currentVersion.at(i) = previousBatch->at(i);
    }
  } else {
    throw std::invalid_argument("no more batch undo");
  }
}

// add another batch to the batches store;
// take the current version of every file, and increment by one.
// these are the versions to use when writing batch edited files.
void UndoRedoModel::makeNewBatch() {
  size_t nFiles = currentVersion.size();
  vector<int> *newBatch = new vector<int>;
  newBatch->resize(nFiles);
  // push previous versions onto stack
  // these will always be -1 from the current versions
  for (int i=0; i<nFiles; i++) {
    newBatch->at(i) = currentVersion.at(i) -1;
  }
  batches.push_back(newBatch);
  _currentBatchIndex += 1;

  // push the current versions onto the stack
  newBatch = new vector<int>;
  newBatch->resize(nFiles);
  for (int i=0; i<nFiles; i++) {
    newBatch->at(i) = currentVersion.at(i);
  }
  batches.push_back(newBatch);
  _currentBatchIndex += 1;
}

// get the  version for this file
// can save to the file path returned
//string batchGetVersion(int fileNum) {
//  return 
//}

/*
string &UndoRedoModel::getSelectedArchiveFile() {
  return _model->getSelectedArchiveFile();
}

string UndoRedoModel::getSelectedPath() {
  return _model->getCurrentPath();
}

// compare nextTempDir base directory to the base directory
// in the temp stack
bool UndoRedoModel::_isDifferentBaseDir(string nextTempDir) {
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

string UndoRedoModel::getTempDir() {
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

string UndoRedoModel::getSelectedArchiveFileName() {
  return _model->getSelectedArchiveFileName();
}

// return true if the current directory is a temp dir
// i.e. of the form .../.tmp_N/yyyymmdd
bool UndoRedoModel::isSelectedFileInTempDir() {
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
string UndoRedoModel::getPreviousTempDir() {
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
string UndoRedoModel::getNextTempDir() {
  if (_tempDirIndex >= _tempDirStack.size()-1) {
    return "";
  } else {
    _tempDirIndex += 1;
    string nextDir = _tempDirStack.at(_tempDirIndex);
    return nextDir;
  }
}

void UndoRedoModel::_resetTempStack() {

  // delete all temporary directories in stack
  _removeTempDirs();
  // clear the undo/redo stack
  _tempDirStack.clear();
}

void UndoRedoModel::_setBaseDirTempStack() {
  // set the base directory for any edits
  _tempDirIndex = 0;
  _tempDirStack.push_back(_model->getCurrentPath());
}

bool UndoRedoModel::moreFiles() {
  return _model->moreFiles();
}

void UndoRedoModel::getBounds(bool useTimeRange, int *firstArchiveFileIndex,
    int *lastArchiveFileIndex) {
  if (useTimeRange) {
    *firstArchiveFileIndex = 0;
    *lastArchiveFileIndex = _model->getNArchiveFiles() - 1;
  } else {
    *firstArchiveFileIndex = _model->getPositionOfSelection();
    *lastArchiveFileIndex = *firstArchiveFileIndex;
  }
}


bool UndoRedoModel::_isTempDir(string *path) {
  bool found = false;
  if (path->find(".tmp_") != string::npos) {
    found = true;
  } 
  return found; 
}

// move/rename the current selected temp directory
// to the newName
void UndoRedoModel::replaceSelectedTempPath(string newName) {
  _tempDirStack[_tempDirIndex] = newName;
}
*/

// before changing to a new base directory, prompt to save any temp dirs?
// otherwise, the temp dirs will be deleted.
// new method ... movingToNewBaseDir ...
// if moving to new base dir, clear the stack
void UndoRedoModel::_removeTempDirs() {

  //vector<string>::iterator it;
  //if (_tempDirStack.size() > 1) {

    // clean up temp dirs
    string command = "rm -r ";
    command.append(baseDir.getDirectory()); //  getSelectedPath());
    command.append("/");
    command.append(_tmpDir);

    //cout << command << endl;
    //int result = 0;
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
  //}
  //
}

