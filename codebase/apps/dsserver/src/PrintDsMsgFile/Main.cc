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
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include <dsserver/DsFileCopyMsg.hh>
#include <Spdb/DsSpdbMsg.hh>
#include <didss/DsMessage.hh>

void _dealWithFilecopy( unsigned char *buffer, int file_size);
void _dealWithSpdb( unsigned char *buffer, int file_size);

int main(int argc, char *argv[]){

  //
  // Is this a cry for help?
  //
  for (int i=0; i < argc; i++){
    if (!(strcmp(argv[i], "-h"))){

      cerr << endl;
      cerr << "PrintDsMsgFile can be used to print the Ds Message" << endl;
      cerr << "files that DsFileDist writes to disk. The first" << endl;
      cerr << "argument is the filename to print the info for." << endl;
      cerr << "Niles Oien October 2004." << endl << endl;

      exit(0);
    }
  }


  char fileName[1024];
  if (argc < 2){
    fprintf(stdout,"Input file ? ");
    fscanf(stdin,"%s", fileName);
  } else {
    sprintf(fileName,"%s", argv[1]);
  }

  struct stat buf;

  if (stat(fileName, &buf)){
    cerr << "Failed to stat " << fileName << endl;
    return -1;
  }

  int file_size =   buf.st_size;

  unsigned char *buffer = (unsigned char *) malloc(file_size);
  if (buffer == NULL){
    cerr << "Malloc failed!" << endl;
    return -1;
  }

  FILE *fp = fopen(fileName,"r");
  if (fp == NULL){
    cerr << "Failed to open " << fileName << endl;
    return -1;
  }

  int numRead = fread(buffer, sizeof(unsigned char), file_size, fp);

  fclose(fp);

  if (numRead != file_size){
    cerr << "Read error." << endl;
    return -1;
  }
  

  DsMessage Q;

  int retVal = Q.disassemble(buffer, file_size);

  if (retVal != 0){
    cerr << "Dissassemble into DsMessage failed." << endl;
    exit(-1);
  }

  Q.printHeader( cerr, " " );


  switch ( Q.getType() ){

  case DsFileCopyMsg::DS_MESSAGE_TYPE_FILECOPY :
    _dealWithFilecopy( buffer, file_size);
    break;

  case DsSpdbMsg::DS_MESSAGE_TYPE_SPDB :
    _dealWithSpdb( buffer, file_size);
    break;

  default :
    cerr << "Unsupported DsMessage type " <<  Q.getType() << endl;
    break;

  }
  
  return 0;

}

void _dealWithFilecopy( unsigned char *buffer, int file_size){

  DsFileCopyMsg D;

  if (D.disassemble(buffer, file_size)){
    cerr << "Failed to disassemble." << endl;
    return;
  }

  D.print(cerr);

  return;

}

void _dealWithSpdb( unsigned char *buffer, int file_size){

  DsSpdbMsg D;

  if (D.disassemble(buffer, file_size)){
    cerr << "Failed to disassemble." << endl;
    return;
  }
  
  D.print(cerr);
  
  return;

}



