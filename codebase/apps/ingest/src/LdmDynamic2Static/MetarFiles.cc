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

#include <cstdio>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <toolsa/pmu.h>
   
#include "MetarFiles.hh"
using namespace std;

// Constructor. Initialises local vars by finding first file.
// File is not read - offset is set to 0.

MetarFiles::MetarFiles(char *InDirPath, bool Debug,
		       int MaxGap, int MaxFails, long MinSize,
		       char *OutDirPath, bool writeLdataFile,
		       char *InExt, char *OutPrecursor) {

  // Make a local copy of the directory name
  // and the other local stuff.
  _InDirPath=STRdup(InDirPath);
  _OutDirPath=STRdup(OutDirPath);
  _OutPrecursor = STRdup(OutPrecursor);
  _InExt = STRdup( InExt );
  _debug = Debug;
  Max_Gap=MaxGap;
  Max_Fails=MaxFails;
  Fails=0;

  _ldataInfo = NULL;
  if (writeLdataFile) {
    _ldataInfo = new DsLdataInfo(_OutDirPath,true);
  }

  bool keepGoing = false;
  do {
    PMU_auto_register("Looking for input data");
    // Go looking for Metar files.
    ugmtime(&FileTime);
    
    // Set the minutes and seconds to 0
    
    FileTime.min=0; FileTime.sec=0;
    FileTime.unix_time=uconvert_to_utime(&FileTime);
    
    int i=0,Exists;
    long Size;
    
    do{
      get_filename(FileTime,_InDirPath,FileName,&Exists,&Size,InExt);
      if (Size < MinSize) Exists=0; // Doesn't count.
      
      if (Debug) fprintf(stderr,"%s : %d\n",FileName,Exists);
      
      if (Exists==0){
	i++;
	FileTime.unix_time=FileTime.unix_time-3600;
	uconvert_from_utime(&FileTime);
      }
    } while((Exists==0)&&(i < Max_Gap));
    
    if (Exists==0){
      fprintf(stderr,"Failed to find any current metars in %s\n",
	      _InDirPath);
      keepGoing = true;
    }
    
    ifp=fopen(FileName,"r");
    if (ifp==NULL){
      fprintf(stderr,"Failed to open %s\n",
	      FileName);
      keepGoing = true;
    }
    if (keepGoing) sleep(5);
  } while (keepGoing);

  Offset=0; // At start of file.

}

// Update
int MetarFiles::Update(bool Debug, long MinSize){


  if (Debug) fprintf(stderr,"Updating to %s\n",_OutDirPath);

  //
  // See if the file has grown.
  //
  struct stat S;

  if (stat(FileName,&S)){
    fprintf(stderr,"Failed to stat %s\n",FileName);
  } else {
    if (Offset < S.st_size){ // File has grown.

      if (Debug) fprintf(stderr,"%s : File has grown.\n",FileName);
      Fails=0;
      process_grown_file(&ifp,Offset,S.st_size,_OutDirPath, _OutPrecursor);
      Offset = S.st_size;

    } else {
      if (Debug) fprintf(stderr,"%s : File has not grown.\n",FileName);
      Fails++;
    }
  }

  //
  // See if a new file exists.
  // 
  date_time_t Future,Now;
  char NewName[MAX_PATH_LEN];
  Future=FileTime;

  Future.unix_time=Future.unix_time+3600;
  uconvert_from_utime(&Future);
 
  int Exists,i; long Size;

  i=0; Exists=0;
  do{

    i++;
    ugmtime(&Now);

    //
    // Do not allow filenames to go literally into the
    // future - ie, beyond now - this is a mistake.
    //
    if (Now.unix_time > Future.unix_time){
      get_filename(Future,_InDirPath,NewName,&Exists,&Size,_InExt);
      // 
      // Ensure that the file is big enough. This
      // avoids spurious metars causing valid Metars to be skipped.
      //
      if (Size < MinSize) Exists=0;

      if (Exists==0){
	Future.unix_time=Future.unix_time+3600;
	uconvert_from_utime(&Future);
      }

    }
  }while((Exists==0)&&(i < Max_Gap)&&(Now.unix_time > Future.unix_time));

  if (Exists){ 
    //
    // We have a new file. Point to it so
    // it will be processed on the next pass.
    //
    fclose(ifp); Offset=0;
    sprintf(FileName,"%s",NewName);
    FileTime=Future;

    ifp=fopen(FileName,"rb"); 
    if (ifp==NULL){
      fprintf(stderr,"Failed to open %s\n",
	      FileName);
      exit(-1);
    }
    
    if (Debug) fprintf(stderr,"Found new file %s\n",FileName);
    //
    // Process the new file.
    //
    if (stat(FileName,&S)){
      fprintf(stderr,"Failed to stat new file %s\n",FileName);
    } else {
      if (Offset < S.st_size){ // File has data since Offset=0 here.
	if (Debug) fprintf(stderr,"%s : New file has data.\n",FileName);
	Fails=0;
	process_grown_file(&ifp,Offset,S.st_size,_OutDirPath, _OutPrecursor);
	Offset = S.st_size;
      } else {
	if (Debug) fprintf(stderr,"%s : New file has zero size.\n",FileName);
      }
    }



  }

  if (Fails > Max_Fails){
    fprintf(stderr,"%d passes with no change - restarting.\n",Fails);
    return 1;
  }

  return 0;

}

///////////////////////////////////////////
// Destructor
MetarFiles::~MetarFiles(){

 
  delete (_ldataInfo);
  STRfree(_InDirPath);
  STRfree(_OutDirPath);
  fclose(ifp);

}

//////////////////////////////////////
// Get an input file name and see if it exists.

void MetarFiles::get_filename(date_time_t t, char *Dir, char *Name,
			      int *Exists, long *Size, char *InExt)

{

  if (Dir[strlen(Dir)-1] == '/'){
   sprintf(Name,              
	   "%s%d%02d%02d/%d%02d%02d%02d.%s",
	   Dir,t.year,t.month,t.day,
	   t.year,t.month,t.day,t.hour,InExt);
  } else {
   sprintf(Name,              
	   "%s/%d%02d%02d/%d%02d%02d%02d.%s",
	   Dir,t.year,t.month,t.day,
	   t.year,t.month,t.day,t.hour,InExt);

  }

  struct stat S;

  if (stat(Name,&S)){
    *Exists=0;
    *Size=0;
  } else {
    *Exists=1;
    *Size=S.st_size;
  }

  return;

}

//////////////////////////////////////
// Process a file that has grown.
void MetarFiles::process_grown_file(FILE **fp, long StartOffset,
				    long FinOffset, char *OutDir,
				    char *OutPrecursor){

  //
  // Seek to the first byte we have not processed.
  //
  long Begin,Length;
  if (StartOffset==0){ 

    // In this case StartOffset points 
    //to an unprocessed byte.

    Begin=0;
    Length=FinOffset-StartOffset+1;
  } else {

    // The byte pointed at by StartOffset has already been processed.

    Begin=StartOffset+1;
    Length=FinOffset-StartOffset;
  }

  if (fseek(*fp, Begin, SEEK_SET)){
    perror(" Seek failed ");
    exit(-1);
  }
  //
  // Allocate memory for new file.
  //
  unsigned char *b;
  b=(unsigned char *)malloc(Length);
  if (b==NULL){
    fprintf(stderr,"Malloc failed.\n");
    exit(-1);
  }
  //
  // Read in data.
  //
  fread(b, sizeof(unsigned char), Length,*fp);

  //
  // Put together the output file name.
  //
  char Name[MAX_PATH_LEN];
  date_time_t t;

  ugmtime(&t);

  if (OutDir[strlen(OutDir)-1] == '/'){
   sprintf(Name,              
	   "%s%s.%d%02d%02d%02d%02d",
	   OutDir,OutPrecursor,t.year,t.month,t.day,
	   t.hour,t.min);
  } else {
   sprintf(Name,              
	   "%s/%s.%d%02d%02d%02d%02d",
	   OutDir,OutPrecursor,t.year,t.month,t.day,
	   t.hour,t.min);

  }

  FILE *ofp;

  ofp=fopen(Name,"ab"); // Open append in case the file already exists.
  if (ofp==NULL){
    fprintf(stderr,"Failed to create %s\n",Name);
    exit(-1);
  }

  if (_debug){
    fprintf(stderr,"Appending to file %s\n", Name);
  }

  fwrite(b, sizeof(unsigned char), Length,ofp);

  free(b); fclose(ofp);

  if (_ldataInfo) {

    char ext[16];
    sprintf(ext,              
	    "%d%02d%02d%02d%02d",
	    t.year,t.month,t.day,
	    t.hour,t.min);
    
    char nam[32];
    sprintf(nam,"%s.%s",_OutPrecursor, ext);

    _ldataInfo->setDataFileExt( ext );
    _ldataInfo->setWriter( "LdmDynamic2Static" );
    _ldataInfo->setRelDataPath( nam ); // File name plus extension.
    _ldataInfo->setUserInfo1( _OutPrecursor ); // File name.
    _ldataInfo->setUserInfo2( nam ); // File name plus extension.
    _ldataInfo->write(t.unix_time);
  }

}




