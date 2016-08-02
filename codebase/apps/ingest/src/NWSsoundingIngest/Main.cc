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
#include <cerrno>
#include <cstdio>
#include <ctype.h> 
#include <cstring>
#include <toolsa/pmu.h>
#include <toolsa/TaArray.hh>
#include <didss/DsInputPath.hh>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>       

#include "Params.hh"
#include "Args.hh"

using namespace std;
#include "Decoders.hh"

static void tidy_and_exit (int sig);
static int _readFile(char *FileName, Params *P);
 
int main(int argc, char *argv[])

{

  // Trap.
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  
  // Get the CLI arguments.

  Args *_args = new Args(argc, argv, "NWSsoundingIngest");
  if (!_args->OK) {
    fprintf(stderr, "ERROR: NWSsoundingIngest\n");
    fprintf(stderr, "Problem with command line args\n");
    exit(-1);
  }                 

  // Get the TDRP parameters.

  Params *P = new Params(); 
  char *paramsFilePath;

  if (P->loadFromArgs(argc, argv, _args->override.list,
                      &paramsFilePath)){
    fprintf(stderr,"Specify params file with -params option.\n");
    exit(-1);
  } 

  // Check in.

  PMU_auto_init("NWSsoundingIngest", P->instance, PROCMAP_REGISTER_INTERVAL);
  
  // Set up DsInputPath.

  DsInputPath *_input;

  if (P->mode == Params::ARCHIVE) {

    if (_args->nFiles <= 0){
      fprintf(stderr,"For ARCHIVE mode, specify file list a argument.\n");
      exit(-1);
    }

    fprintf(stdout,"%d files :\n",_args->nFiles);
    for (int i=0;i<_args->nFiles;i++){
      fprintf(stdout,"[ %d ] %s\n",i+1,_args->filePaths[i]);
    }

    _input = new DsInputPath("NWSsoundingIngest",
			     P->debug,
			     _args->nFiles,
			     _args->filePaths);
                                                            
  } else { // REALTIME mode.

   _input = new DsInputPath("NWSsoundingIngest",
                             P->debug,
                             P->InDir,
                             P->max_realtime_valid_age,
                             PMU_auto_register,
			     P->useLdataInfo, true);
   //
   // Set the file extension to search for, if specified.
   //
   if (strlen(P->fileExtension) > 0){
     _input->setSearchExt(P->fileExtension);
   }

  }


  // And use it.
  PMU_auto_register("Waiting for data");

  char *FilePath;
  _input->reset();
  int iret = 0;

  while ((FilePath = _input->next()) != NULL) {

    if (_readFile(FilePath,P)) {
      iret = -1;
    }
    PMU_auto_register("Waiting for data");

  } // while loop.

  return iret;

}

/////////////////////////////////////////////

static void tidy_and_exit (int sig){
  // PMU_auto_unregister(); // This seems to cause problems on cntrl-C
  exit(sig);
}

//////////////////////////////////////////////
// read in the file and process it
// returns 0 on success, -1 on error

static int _readFile(char *FileName, Params *P)

{

  //
  // Stat the file to get its time and size
  //
  struct stat fileStat;
  if (stat(FileName,&fileStat)){
    int errNum = errno;
    fprintf(stderr, "Could not stat %s\n",FileName);
    fprintf(stderr, "%s\n", strerror(errNum));
    return -1;
  }
  
  //
  // Open the file.
  //
  FILE *in = fopen(FileName,"rt");
  if (in == NULL){
    int errNum = errno;
    fprintf(stderr,"Could not open %s\n",FileName);
    fprintf(stderr, "%s\n", strerror(errNum));
    return -1;
  }

  //
  if (P->debug){
    fprintf(stderr,"Processing %s\n",FileName);
  }
  
  // read the file into a vector of strings, a line at a time

  vector<string> lines;

  while (!feof(in)) {
    char line[BUFSIZ];
    if (fgets(line, BUFSIZ, in) == NULL) {
      break;
    }
    // remove line feed
    line[strlen(line)-1] = '\0';
    // add to vector of lines
    lines.push_back(line);
  }
  fclose(in);

  if (P->debug) {
    cerr << "====================================" << endl;
    cerr << "Found data lines: " << endl;
    for (size_t ii = 0; ii < lines.size(); ii++) {
      cerr << lines[ii] << endl;
    }
    cerr << "====================================" << endl;
  }

  // look through the lines,
  // create a vector of independent messages

  vector<string> messages;
  string message;
  for (size_t ii = 0; ii < lines.size(); ii++) {
    
    message += lines[ii];
    
    // is this message done?
    
    bool done = false;
    if (ii == lines.size() - 1) {
      done = true;
    } else if (lines[ii].find("=") != string::npos) {
      done = true;
    } else if (lines[ii+1].find("TT") != string::npos) {
      done = true;
    } else if (lines[ii+1].find("PP") != string::npos) {
      done = true;
    }

    if (done) {
      messages.push_back(message);
      message.clear();
    }
    
  } // ii

  // handle the messages one at a time

  int messagesDecoded = 0;

  for (size_t ii = 0; ii < messages.size(); ii++) {

    string message = messages[ii];
    
    if (P->debug) {
      cerr << "------------------------------------" << endl;
      cerr << "Handling message: " << message << endl;
      cerr << "------------------------------------" << endl;
    }
    
    // create clean message, without control characters etc

    string clean;
    for (size_t jj = 0; jj < message.size(); jj++) {
      char c = message[jj];
      if (isalnum(c) || (c == '/') || (c == '=') || (c == ' ')) {
        clean += c;
      }
    }
    
    int decoded = 0;
    //
    // Set up the Decoders with the file date/time
    // and name.
    //
    Decoders D(P, fileStat.st_ctime, FileName);
    
    size_t start;

    if ((start = clean.find("TTAA")) != string::npos) {
      decoded = D.TTAA_decode("TTAA", clean);
      if (P->debug) D.print();
      if (D.gotData()) D.write();
    }
    
    if ((start = clean.find("TTBB")) != string::npos) {
      decoded = D.TTBB_decode("TTBB", clean);
      if (P->debug) D.print();
      if (D.gotData()) D.write();
    }
    
    if ((start = clean.find("TTCC")) != string::npos) {
      decoded = D.TTCC_decode("TTCC", clean);
      if (P->debug) D.print();
      if (D.gotData()) D.write();
    }
    
    if ((start = clean.find("TTDD")) != string::npos) {
      decoded = D.TTDD_decode("TTDD", clean);
      if (P->debug) D.print();
      if (D.gotData()) D.write();
    }
    
    if ((start = clean.find("PPAA")) != string::npos) {
      decoded = D.PPAA_decode("PPAA", clean);
      if (P->debug) D.print();
      if (D.gotData()) D.write();
    }
    
    if ((start = clean.find("PPBB")) != string::npos) {
      decoded = D.PPBB_decode("PPBB", clean);
      if (P->debug) D.print();
      if (D.gotData()) D.write();
    }
    
    if ((start = clean.find("PPCC")) != string::npos) {
      decoded = D.PPCC_decode("PPCC", clean);
      if (P->debug) D.print();
      if (D.gotData()) D.write();
    }
    
    if ((start = clean.find("PPDD")) != string::npos) {
      decoded = D.PPDD_decode("PPDD", clean);
      if (P->debug) D.print();
      if (D.gotData()) D.write();
    }
    
    if (decoded) messagesDecoded++;;
    
    if (P->debug){
      if (!(decoded)) 
        fprintf(stderr,"Message not decoded.\n");
      else
        fprintf(stderr,"Message decoded.\n");
    }
    
  } // ii
  
  return 0;

}



