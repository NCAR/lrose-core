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

#ifndef _LDM_FILE_STROBE_INC_
#define _LDM_FILE_STROBE_INC_

//
// Class to deal with files that grow in size over time, like
// those from an LDM. Niles Oien January 2007.
//
//
// Here is an example main.cc that shows how to use it :
//

/*------------------------------------------------------

#include <iostream>
#include <stdlib.h>

#include <toolsa/ldmFileStrobe.hh>

int main(int argc, char *argv[]){

  int debug = 0;

  if (argc > 1) debug = (int)atoi(argv[1]);

  ldmFileStrobe L("/raid/ext_obs/lightning",
		  ".ltg",
		  "./_tempFileList",
		  300);

  L.setDebug(debug);

  while(1){

    L.strobe();

    int numFound = L.getNfiles();

    cerr << numFound << " files found." << endl;

    char fileName[1024]; int oldSize; int newSize;

    for (int i=0; i < numFound; i++){
      L.getFile(i, fileName, &oldSize, &newSize);
      cerr << i << " : " << fileName << " : size went from " << oldSize << " to " << newSize << endl;
    }

    L.updateList();

    sleep(5);

  }

  return 0;

}

------------------------------------------------------*/

#include <vector>
#include <string>

using namespace std;

class ldmFileStrobe {

public:


  //
  // Constructor. You give it :
  //     * The directory to watch
  //     * A substring that must be in each input filename 
  //       (or an empty string if you don't care).
  //     * The name of a temporary file that will be used to
  //       record the directory status.
  //     * The maximum age, seconds, of valid input files (if
  //       the temp file is older than this on startup it is
  //       deleted).
  //     
  //
  ldmFileStrobe(char *directory,
		char *subString,
		char *tempFile,
		long maxAge);

  //
  // Strobe function. Compiles a list of files that have changed.
  //
  void strobe();

  //
  // Return number of updated files found by strobe().
  //
  int getNfiles();

  //
  // Get ith file. Returns the filename (caller must have allocated space),
  //               the old size (ie. the size of the file the last time
  //               it was strobed) and the new size. Sizes are in bytes.
  //
  void getFile(int fileNum, char *fileName, int *oldSize, int *newSize);

  //
  // Update list, ie accept the changes from the last strobe().
  // Call this after you have processed all the updated files.
  //
  int updateList();

  //
  // Set verbose output. 0 == No output, 1== some, 2 or more == lots
  //
  void setDebug(int debug);

  //
  // Destructor
  //
  ~ldmFileStrobe();

  private :

  char _directory[1024];
  char _subString[1024];
  char _tempFile[1024];
  long _maxAge;
  int _debug;

  vector <char *> _updatedNames;
  vector <int>    _updatedOldSize;
  vector <int>    _updatedNewSize;


  int _findTempFileEntry(char *fileName, int *tempFileSize);
  int _passesStatTest(char *fileName);
  int _alreadyInList(char *fileName);
  void _clearUpdatedList();

};




#endif
