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
// DsFile2Server.cc
//
// DsFile2Server object
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2004
//
///////////////////////////////////////////////////////////////

#include <toolsa/pmu.h>
#include <didss/LdataInfo.hh>
#include <didss/DsMessage.hh>

#include <dsserver/DsFileCopyMsg.hh>
#include <Spdb/DsSpdbMsg.hh>
#include <Spdb/DsSpdb.hh>
#include <dsserver/DsFileCopy.hh>
#include <didss/DsInputPath.hh>

#include <cstdio>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include "Args.hh"
#include "DsFile2Server.hh"

extern void heartbeat( const char *label);

// constructor

DsFile2Server::DsFile2Server(int argc, char *argv[]){

  isOK = 0;

  // get TDRP params

  _params = new Params();
  if (_params->loadFromArgs(argc, argv, NULL, NULL)) {
    cerr << "  Problem with TDRP parameters." << endl;
    return;
  }

  // process registration

  PMU_auto_init("DsFile2Server", _params->instance,
                PROCMAP_REGISTER_INTERVAL);


  // Go to archive mode, if -f specified.

  for (int i=0; i < argc; i++){
    if (!(strcmp(argv[i], "-f"))){
      _params->mode = Params::ARCHIVE;
    }
  }

  isOK = 1;

  return;

}

////////////////////////////////////////////////////////////////////////////
// HEARTBEAT
void  DsFile2Server::Heartbeat(const char *label)
{
	  static time_t last_update = 0;

	  FILE *ofile;
	  time_t now = time(0);
	  if (_params->debug >= Params::DEBUG_NORM) fprintf(stderr," %s",ctime(&now));

	  if( _params->write_status_file && now > (last_update + 59)) {
		 if((ofile = fopen(_params->IO_status_file,"w+")) != NULL) {
	         IOC.write_stats(ofile);
			 fclose(ofile);

			 last_update = now - (now % 60); // Make updates happen on the minute
			  if (_params->debug >= Params::DEBUG_NORM) {
                            fprintf(stderr,"---Remainder: %ld------Last Update; %s",(now % 60),ctime(&last_update));
			  }
		 }
	  }

	  PMU_auto_register(label);
}
  

// Main method - run.

int DsFile2Server::Run(int argc, char *argv[]){

  // Loop forever, reading files and acting on them.

  char *fname;

  string pname = "DsFile2Server";
  string idir = _params->input_directory;

  if (_params->mode == Params::REALTIME){

      DsInputPath *input = new DsInputPath( pname,
           false,
           idir,
           600,                        // Pick up all files 600 secs or newer.
           heartbeat,                  // Call out heartbeat function every sec
           _params->use_ldatainfo,      
           false);                     // Want latest only - false - We want everything.


    input->setFileQuiescence(2);          //  2 second for expected small files.
    while ( 1 ) {
      
    fname = input->next(); // Blocks here waiting for new files.
	
	if (_params->debug >= Params::DEBUG_NORM){
	  cerr << "Triggering on " << fname << endl;
	}
	
	_dealWithFile( fname );

	if (_params->debug >= Params::DEBUG_VERBOSE){
	  cerr << "Deleting " << fname << endl;
	}
	
    unlink(fname);
	
      }
  } else {
    //
    // Archive mode.
    //
    Args A(argc, argv, "DsFile2Server");
    //
    for (int i=0; i < A.nFiles; i++){

      if (_params->debug >= Params::DEBUG_VERBOSE){
	cerr << "Considering " << A.filePaths[i] << endl;
      }
      _dealWithFile( A.filePaths[i] );
    }
    if (_params->debug >= Params::DEBUG_VERBOSE){
      cerr << "Finished with archive mode." << endl;
    }
  }

  return 0;

}

/////////////////////////////////////////////////////////
//
// Do the actual work of dealing with a file.
//
void DsFile2Server::_dealWithFile( string inFilename ){

  // Read the file in.
  struct stat buf;

  if (stat(inFilename.c_str(), &buf)){
    cerr << "Failed to stat " << inFilename << endl;
	IOC.count_error();
    return;
  }

  int file_size =   buf.st_size;

  unsigned char *buffer = (unsigned char *) malloc(file_size);
  if (buffer == NULL){
    cerr << "Malloc failed!" << endl;
	IOC.count_error();
    return;
  }

  FILE *fp = fopen(inFilename.c_str(),"r");
  if (fp == NULL){
    cerr << "Failed to open " << inFilename << endl;
	IOC.count_error();
    return;
  }

  int numRead = fread(buffer, sizeof(unsigned char), file_size, fp);

  fclose(fp);

  if (numRead != file_size){
    cerr << "Read error." << endl;
	IOC.count_error();
    return;
  }

  DsMessage D;

  int retVal = D.disassemble(buffer, file_size);

  if (retVal != 0){
    free(buffer);
	IOC.count_error();
    return;
  }

  if (_params->debug >= Params::DEBUG_VERBOSE){
    D.printHeader(cerr, "   ");
  }

  int mtype = D.getType();

  IOC.count(mtype,file_size);

  switch ( mtype){

  case DsFileCopyMsg::DS_MESSAGE_TYPE_FILECOPY :
    _dealWithFilecopy( buffer, file_size);
    break;

  case DsSpdbMsg::DS_MESSAGE_TYPE_SPDB :
    _dealWithSpdb( buffer, file_size);
    break;

  default :
    cerr << "Unsupported DsMessage type " <<  D.getType() << endl;
    break;

  }


  free(buffer);
  return;


}

///////////////////////////////////////////////////////
//
// Deal with file copy messages.
//
void DsFile2Server::_dealWithFilecopy( unsigned char *buffer, int file_size){

  DsFileCopyMsg D;
  if (D.disassemble(buffer, file_size)) return;

  if (_params->debug >= Params::DEBUG_VERBOSE){
    D.print(cerr);
  }

  DsFileCopy copy( DsMessage::PointToMem ); // Use default memory management.

  DsURL url(D.getDataUrl());
  //
  // If the host is 'localhost', we have to set the port
  // to 0 to ensure that we go through the server.
  //
  if (!(strcmp("localhost", url.getHost().c_str()))){
    url.setPort(0);
  }

  if (copy.sendBufferToSocket(url, buffer, file_size)){

    cerr << "ERROR - file not sent to server - failed." << endl;
    cerr << "        URL in use was as follows : " << endl;
    url.print(cerr);
    cerr << copy.getErrorStr() << endl;
    cerr << "   Is DsFCopyServer or DsServerMgr running on the target host?" << endl;
    return;
  }

  return;

}

///////////////////////////////////////////////////////
//
// Deal with SPDB messages.
//
void DsFile2Server::_dealWithSpdb( unsigned char *buffer, int file_size){

  DsSpdbMsg D;

  if (D.disassemble( buffer, file_size)) return;

  if (_params->debug >= Params::DEBUG_VERBOSE){
    D.print(cerr);
  }

  DsSpdb spdb;

  DsSpdbMsg::info_t spdbInfo = D.getInfo();

  string urlString = D.getUrlStr();

  spdb.addPutChunks(D.getNChunks(), D.getChunkRefs(), D.getChunkData() );

  int putMode = D.getMode();

  switch ( putMode ) {
    
  case DsSpdbMsg::DS_SPDB_PUT_MODE_OVER:
    spdb.setPutMode(Spdb::putModeOver);
    if (spdb.put(urlString,
		 spdbInfo.prod_id,
		 spdbInfo.prod_label)) {
      cerr << "ERROR in put over mode" << endl;
    } 
    break;
    
  case DsSpdbMsg::DS_SPDB_PUT_MODE_ADD:
    spdb.setPutMode(Spdb::putModeAdd);
    if (spdb.put(urlString,
		 spdbInfo.prod_id,
		 spdbInfo.prod_label)) {
      cerr << "ERROR in put add mode" << endl;
    }
     break;

  case DsSpdbMsg::DS_SPDB_PUT_MODE_ONCE:
    spdb.setPutMode(Spdb::putModeOnce);
    if (spdb.put(urlString,
		 spdbInfo.prod_id,
		 spdbInfo.prod_label)) {
      cerr << "ERROR in put once mode" << endl;
    }
    break;
    
  case DsSpdbMsg::DS_SPDB_PUT_MODE_ERASE:
    
    if (spdb.erase(urlString)) {
       cerr << "ERROR in erase mode" << endl;
    }
    break;
    
  default:
    break;
    
  } // switch (put_mode)

  return;

}




// Destructor.

DsFile2Server::~DsFile2Server(){

}

