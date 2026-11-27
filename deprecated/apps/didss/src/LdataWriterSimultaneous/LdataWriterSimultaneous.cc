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
#include <string>
#include <cstdio>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <toolsa/pmu.h>
#include <dsserver/DsLdataInfo.hh>

#include "LdataWriterSimultaneous.hh"
#include "Params.hh"

using namespace std;

LdataWriterSimultaneous::LdataWriterSimultaneous(){
  return;
}

LdataWriterSimultaneous::~LdataWriterSimultaneous(){
  return;
}


int LdataWriterSimultaneous::Run(int argc, char *argv[]){

  vector <LdataWriterSimultaneous::name_t> alreadyProcessed;
  vector <LdataWriterSimultaneous::name_t> foundFiles;
  
  Params P;
  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }

  PMU_auto_init("LdataWriterSimultaneous", P.instance,
                PROCMAP_REGISTER_INTERVAL);

  
  while (1){
    //
    // Fill up the foundFiles vector with files
    // that are young enough to be considered.
    //
    foundFiles.clear();

    DIR *dirp = opendir( P.dir );
    if (dirp == NULL){
      sleep (1);
      continue;
    }

    struct dirent *dp;

    for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)){

       PMU_auto_register("Scanning");

      char fullName[1024];
      sprintf(fullName,"%s/%s", P.dir, dp->d_name);

      struct stat buf;
      if (stat(fullName, &buf)) continue;
      if (!(S_ISREG(buf.st_mode))) continue; // Only process files, not dirs or links.
      if (dp->d_name[0] == '_') continue; // Nothing that starts with an underscore.
      if (buf.st_size < P.minSize) continue; // Nothing too small

      long age = time(NULL) - buf.st_ctime;
      if (age <= P.maxAge){
	name_t f;
	f.fullName = string( fullName );
	f.baseName = string( dp->d_name );
	foundFiles.push_back( f );
      }
    }

    closedir(dirp);

    //
    // Loop through found files, if they have not already been
    // processed, process them.
    //
    for (unsigned i=0; i < foundFiles.size(); i++){

       PMU_auto_register("Working");

      int alreadyDone = 0;
      for (unsigned j=0; j < alreadyProcessed.size(); j++){
	if (alreadyProcessed[j].baseName == foundFiles[i].baseName){
	  alreadyDone = 1;
	  if (P.debug) cerr << "Already processed " << foundFiles[i].baseName << endl;
	  break;
	}
      }

      if (!(alreadyDone)){
	if (P.debug) cerr << "Need to process " << foundFiles[i].baseName << endl;
	alreadyProcessed.push_back(foundFiles[i]);
	_process(foundFiles[i], P.dir);
	for (int k=0; k < P.ldataDelay; k++){
	  sleep(1);
	  PMU_auto_register("Waiting");
	}
      }

    }

    //
    // Housework - so that the list of files we have processed
    // does not grow to infinity, go though it and cross off any files
    // older than maxAge that are out of contention anyway.
    //
    // In order to remain on the list a file must exist and have age
    // less than maxAge.
    //
    vector <name_t> keepers;
    keepers.clear();
    for (unsigned i=0; i < alreadyProcessed.size(); i++){

      PMU_auto_register("Cleaning");

      struct stat buf;
      if (0==stat(alreadyProcessed[i].fullName.c_str(), &buf)){
	long age = time(NULL) - buf.st_ctime;
	if (age <= P.maxAge){
	  keepers.push_back( alreadyProcessed[i] );
	}
      }
    }

    alreadyProcessed.clear();
    for (unsigned i=0; i < keepers.size(); i++){
      alreadyProcessed.push_back( keepers[i] );
    }
    keepers.clear();

    if (P.debug) cerr << endl;

    for (int k=0; k < P.checkDelay; k++){
      sleep(1);
      PMU_auto_register("Waiting");
    }

  }


  return 0;

}


void LdataWriterSimultaneous::_process(name_t n, char *dir){

   // create Ldata file object

  DsLdataInfo *ldata = new DsLdataInfo(dir, false);

  ldata->setRelDataPath(n.baseName.c_str());

  if (ldata->write( time(NULL) ) ) {
    cerr << "ERROR -  LdataWriterSimultaneous::Run" << endl;
    cerr << "  Cannot write ldata file to dir: " << dir << endl;
  }

  delete ldata;

  return;

}
