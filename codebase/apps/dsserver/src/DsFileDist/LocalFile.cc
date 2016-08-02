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
// localFile.cc
//
// Class to deal with cases in which the hostname
// is "localfile", indicating that the data are to
// be turned into a Ds Message, and that this message is to
// be written to file.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2004
//
/////////////////////////////////////////////////////////////


#include "LocalFile.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdlib.h>
#include <cstdio>
#include <toolsa/file_io.h>
#include <didss/RapDataDir.hh>

#include <dsserver/DsFileCopyMsg.hh>
#include <didss/LdataInfo.hh>

using namespace std;

// constructor

LocalFile::LocalFile(const string &prog_name,
		     const Params &params) :
  _progName(prog_name),
  _params(params)

{

}

// destructor

LocalFile::~LocalFile(){

}


// copy the file to local directory
//
// Returns 0 on success, -1 on failure

int LocalFile::doCopy(const PutArgs *putArgs)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << endl;
    cerr << "PUT DSMSG TO DISK : " << putArgs->destUrl.getFile() << endl;
  }

  // Stat the file to get the size.
  
  string fileName = putArgs->filePath;
  struct stat buf;
  if (stat(fileName.c_str(), &buf)){
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Could not stat " << fileName << endl;
    }
    return -1;
  }

  int fileSize = buf.st_size;
  time_t modTime = buf.st_mtime;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "File size : " << fileSize
	 << " mod time : " << utimstr(modTime) << endl;
  }

  // Read the file into memory.

  unsigned char *buffer = (unsigned char *) malloc( fileSize );
  if (buffer == NULL){
    cerr << "Malloc failed!" << endl;
    exit(-1);
  }

  FILE *fp = fopen(fileName.c_str(), "r");
  if (fp == NULL){
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Could not open " << fileName << endl;
    }
    free(buffer);
    return -1;
  }

  int numRead = fread(buffer, sizeof(unsigned char), fileSize, fp);
  fclose(fp);
  
  if (numRead != fileSize){
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Read error for " << fileName << endl;
    }
    free(buffer);
    return -1;
  }

  //
  // OK - now have the file read in to memory. Use what we
  // have to make a Ds Message.
  //

  void *DsBuffer;
  DsFileCopyMsg fileMsg;
  
  LdataInfo ld(putArgs->dirPath.c_str());
  ld.read();

  // Modify the URL somewhat.

  DsURL U(putArgs->destUrl);
  U.setHost( _params.local_file_host );
  string URLstr = U.getURLStr();

  if (_params.debug >= Params::DEBUG_VERBOSE) { 
    cerr << "Substituted host name " <<  _params.local_file_host << endl;
    cerr << "URL is now " << URLstr << endl;
  } 

  string strippedFileName;
  RapDataDir.stripPath(fileName, strippedFileName);

  DsBuffer = fileMsg.assemblePutForced(U,
				       ld,
				       strippedFileName,
				       modTime,
				       fileSize,
				       buffer,
				       fileSize);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << endl << "FILE OBJECT IS AS FOLLOWS :" << endl;
    fileMsg.print(cerr);
    cerr << "ASSEMBLED BUFFER LEN : ";
    cerr << fileMsg.lengthAssembled() << endl;
  }

  // Write this to a file in _params.local_file_dir

  string localPath;
  RapDataDir.fillPath(_params.local_file_dir, localPath);

  if (ta_makedir_recurse(localPath.c_str())){
    cerr << "Failed to create directory " << localPath << endl;
    free(buffer);
    return -1;
  }

  // Use the PID to create a unique filename since we are in child process.

  int pid = getpid();

  char outFilename[MAX_PATH_LEN];
  char fname1[MAX_PATH_LEN];
  char fname2[MAX_PATH_LEN];


  time_t now = time(NULL);

  sprintf(outFilename,"%s/ds_fcopyp_%ld_%d.msg", localPath.c_str(), 
	  now, pid);
  sprintf(fname1,"ds_fcopyp_%ld_%d", now, pid);
  sprintf(fname2,"ds_fcopyp_%ld_%d.msg", now, pid);


  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "TARGET DIR : " << _params.local_file_dir << endl;
    cerr << "STRIPPED TARGET DIR : " << localPath << endl;
    cerr << "WRITING TO FILE " << outFilename << endl;
  }

  FILE *ofp = fopen(outFilename, "w");
  if (ofp == NULL){
    cerr << "Failed to create DsMsg file " << outFilename << endl;
    free(buffer);
    return -1;
  }

  int numWritten = fwrite(DsBuffer, sizeof (unsigned char), 
			  fileMsg.lengthAssembled(), ofp);
  fclose(ofp);

  if (numWritten != fileMsg.lengthAssembled()){
    cerr << "Write failed to " << outFilename << endl;
    free(buffer);
    return -1;
  }

  free(buffer);

  // Write out a latest data info message.

  LdataInfo myLdata(localPath);
  myLdata.setLatestTime( now );
  myLdata.setDataFileExt( "msg");
  myLdata.setWriter( "DsFileDist" );
  myLdata.setRelDataPath( fname2 );
  myLdata.setIsFcast( false );
  myLdata.write( now );

  // Execute the script with the output filename as an argument.

  if (_params.do_post_file_write_script){
    char com[1024];
    sprintf(com, "%s %s", 
	    _params.post_file_write_script,
	    outFilename );
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "EXECUTING " << com << endl;
    }
    system(com);
  }
  
  return 0;

}
