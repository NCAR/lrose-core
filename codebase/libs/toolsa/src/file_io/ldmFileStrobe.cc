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

#include <vector>

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#include <iostream>

#include <toolsa/ldmFileStrobe.hh>
#include <toolsa/file_io.h>

using namespace std;


//
// Constructor.
//
//
ldmFileStrobe::ldmFileStrobe(char *directory,
			     char *subString,
			     char *tempFile,
			     long maxAge){

  sprintf(_directory, "%s", directory);
  sprintf(_subString, "%s", subString);
  sprintf(_tempFile,  "%s", tempFile);
  _maxAge = maxAge;
  _debug = 0;

  //
  // If the temp file exists, and it it too old, delete it.
  //
  struct stat buf;
  if (ta_stat(_tempFile, &buf)) return; // Does not exist.

  long tempFileAge = time(NULL) - buf.st_ctime;

  if (tempFileAge > _maxAge){
    if (unlink(_tempFile)){
      cerr << "WARNING : Unable to delete aged temp file " << _tempFile << endl; // unlikely.
    }
  }

  return;

}

void ldmFileStrobe::setDebug(int debug){
  _debug = debug;
}

//
// Strobe function. Gets list of files that have changed.
//
void ldmFileStrobe::strobe(){

  if (_debug){
    cerr << endl << "In strobe()" << endl;
  }

  _clearUpdatedList();
  
  DIR *dirp;
  dirp = opendir(_directory);

  if (dirp == NULL){
    cerr << "Cannot open directory '" << _directory << "'" << endl;
    perror(_directory); 
    return;
  }

  struct dirent *dp;
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)){

    //
    // Skip hidden/underscore files.
    //
    if (strcmp(dp->d_name, ".") == 0 ||
        strcmp(dp->d_name, "..") == 0 ||
	strcmp(dp->d_name, "_") == 0) {
      continue;
    }

    //
    // If the sub string has length > 0, then skip
    // file names that do not contain the substring.
    //
    if (strlen(_subString) > 0){
      if (NULL == strstr(dp->d_name, _subString)) {
	continue;
      }
    }

    //
    // Put together the total filename and stat it.
    //
    char fileName[1024];
    sprintf(fileName,"%s/%s", _directory, dp->d_name);
    
    int fileSize = _passesStatTest(fileName);
    if ( fileSize == 0 ) continue;

    //
    // See if we can find it in the temp file.
    //
    int tempFileSize;
    if (_findTempFileEntry(fileName, &tempFileSize)){
      //
      // We can't find it, it is new, push it back.
      //
      if (_debug){
	cerr << "Adding new file " << fileName << " to update list, size " << fileSize << endl;
      }
      char *p = strdup(fileName);
      _updatedNames.push_back( p );
      _updatedOldSize.push_back( 0 );
      _updatedNewSize.push_back( fileSize );
    } else {
      //
      // We did find it, see if the size has changed, if
      // it has, push it back.
      //
      if (tempFileSize !=  fileSize){
	if (_debug){
	  cerr << "Adding updated file " << fileName << " to update list, size went from " << tempFileSize;
	  cerr << " to " << fileSize << endl;
	}
	char *p = strdup(fileName);
	_updatedNames.push_back( p );
	_updatedOldSize.push_back( tempFileSize );
	_updatedNewSize.push_back( fileSize );
      }
    }
  }

  if (_debug){
    cerr << "strobe() found " << _updatedNames.size() << " files." << endl << endl;
  }

  closedir(dirp);

  return;
}

//
// Return number of changed files found by strobe().
//
int ldmFileStrobe::getNfiles(){
  return _updatedNames.size();
}
  
//
// Get ith  changed file.
//
void ldmFileStrobe::getFile(int fileNum, char *fileName, int *oldSize, int *newSize){

  sprintf(fileName,"%s", _updatedNames[fileNum]);
  *oldSize = _updatedOldSize[fileNum];
  *newSize = _updatedNewSize[fileNum];

  return;
}

//
// Update list, ie accept the changes from the last strobe().
//
int ldmFileStrobe::updateList(){

  if (_debug){
    cerr << endl << "In updateList()" << endl;
  }

  //
  // Read the temporary file list into vectors.
  //
  vector <char *> tempNames;
  vector <int> tempSizes;

  FILE *tfp = fopen(_tempFile,"r");
  if (tfp == NULL){
    if (_debug){
      cerr << _tempFile << "does not exist." << endl;
    }
  } else {
    char Line[1024];
    while (NULL != fgets(Line, 1024, tfp)){
	
      int tempSize;
      char tempName[1024];

      if ( 2 == sscanf(Line, "%s %d", tempName, &tempSize)){

	char *p = strdup(tempName);
	tempNames.push_back( p );
	tempSizes.push_back( tempSize );

	if (_debug){
	  cerr << "Read entry from " << _tempFile << " : " << tempName << " (" << tempSize << ")" << endl;
	}

      } else {
	cerr << "Failed to decode line from temp file : " << Line;
      }
    }
    fclose(tfp);
  }

  if (_debug){
    cerr << tempNames.size() << " entries read from " <<  _tempFile << endl;
  }

  //
  // OK - now, between the list of updates and the stuff from the temp
  // file, form a list of upated files. Take the stuff from the
  // _updatedNames vector first, it's more current.
  //
  vector <char *> newTempNames;
  vector <int>    newTempSizes;

  for (unsigned i=0; i < _updatedNames.size(); i++){
    if (_passesStatTest(_updatedNames[i])){
      char *p = strdup( _updatedNames[i] );
      newTempNames.push_back( p );

      int k = _updatedNewSize[i];
      newTempSizes.push_back( k );
      
      if (_debug){
	cerr << "Adding new entry to temp file : " << _updatedNames[i];
	cerr << " (" << _updatedNewSize[i] << ")" << endl;
      }
    }
  }

  //
  // Then take the stuff from the temp list - the stuff that has not changed.
  //
  for (unsigned i=0; i < tempNames.size(); i++){
    if ((!(_alreadyInList(tempNames[i]))) && (_passesStatTest(tempNames[i]))){
      char *p = strdup( tempNames[i] );
      newTempNames.push_back( p );

      int l = tempSizes[i];
      newTempSizes.push_back( l );

      if (_debug){
	cerr << "Keeping existing new entry in temp file : " << tempNames[i];
	cerr << " (" << tempSizes[i] << ")" << endl;
      }

    }
  }

  //
  // Write the output file.
  //
  tfp = fopen(_tempFile,"w");
  if (tfp == NULL){
    cerr << "Failed to create " << _tempFile << endl;
    return -1;
  }

  for (unsigned i=0; i < newTempNames.size(); i++){

    if (_debug){
      cerr << "Adding entry to " << _tempFile;
      cerr << " : " <<  newTempNames[i] << " (" << newTempSizes[i] << ")" << endl;
    }

    fprintf(tfp,"%s %d\n", newTempNames[i], newTempSizes[i]);
  }

  fclose(tfp);
  
  if (_debug){
    cerr << newTempNames.size() << " entries now in " << _tempFile << endl; 
  }
  //
  // Clear out the vectors we have used.
  //
  tempSizes.clear();
  newTempSizes.clear();

  for (unsigned i=0; i < tempNames.size(); i++){
    free(tempNames[i]);
  }
  tempNames.clear();

  for (unsigned i=0; i < newTempNames.size(); i++){
    free(newTempNames[i]);
  }
  newTempNames.clear();

  _clearUpdatedList();
  if (_debug){
    cerr << "end of updateList" << endl << endl;
  }

  return 0;

}

//
// Destructor
//
ldmFileStrobe::~ldmFileStrobe(){
  return;
}
 
//
// Find an entry for a file name in the temp file.
//
int ldmFileStrobe::_findTempFileEntry(char *fileName, int *tempFileSize){

  FILE *tfp = fopen(_tempFile,"r");
  if (tfp == NULL) return -1;

  char Line[1024];
  while (NULL != fgets(Line, 1024, tfp)){

    if (NULL != strstr(Line, fileName)){
      fclose(tfp);
      if ( 1 != sscanf(Line + strlen(fileName), " %d", tempFileSize)){
	return -1;
      } else {
	return 0;
      }
    }
  }

  fclose(tfp);

  return -1;

}

//
// See if a file passes our test - it exists, and it is new enough.
// Returns 0 if not, otherwise file size.
//
int ldmFileStrobe::_passesStatTest(char *fileName){

  if (_debug > 1){
    cerr << fileName << " : stat test : ";
  }

  struct stat buf;
  if (ta_stat(fileName, &buf)){
    if (_debug > 1){
      cerr << "does not exist." << endl;
    }
    return 0; // Does not exist.
  }

  long age = time(NULL) - buf.st_ctime;
  if (age > _maxAge){
    if (_debug > 1){
      cerr << "too old (age " << age << " seconds)" << endl;
    }
    return 0; // Too old.
  }

  if (_debug > 1){
    cerr << "passed (age " << age << " seconds)" << endl;
  }

  return buf.st_size; // Fine.

}

//
// See if a filename is already in the list.
//
int ldmFileStrobe::_alreadyInList(char *fileName){

  if (_debug > 1){
    cerr << fileName << " : in list already : ";
  }

  for (unsigned i=0; i < _updatedNames.size(); i++){
    if (0==strcmp(fileName, _updatedNames[i])){
      if (_debug > 1){
	cerr << "true" << endl;
      }
      return 1;
    }
  }

  if (_debug > 1){
    cerr << "false" << endl;
  }
  return 0;
}

//
// Empty out the internal lists.
//
void  ldmFileStrobe::_clearUpdatedList(){

  if (_debug) cerr << "Internal list cleared." << endl;

  _updatedOldSize.clear();
  _updatedNewSize.clear();

  for (unsigned i=0; i < _updatedNames.size(); i++){
    free( _updatedNames[i] );
  }
  
  _updatedNames.clear();

  return;

}
