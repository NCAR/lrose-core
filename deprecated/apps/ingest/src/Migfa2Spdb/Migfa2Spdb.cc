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
#include <toolsa/umisc.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/pmu.h>

#include "Migfa2Spdb.hh"
#include "SpdbOut.hh"
using namespace std;


//
// Constructor and destructor does nothing.
//
Migfa2Spdb::Migfa2Spdb(){

}

//
// Destructor closes reading source.
//

Migfa2Spdb::~Migfa2Spdb(){

}

void Migfa2Spdb::ProcFile(char *FilePath, Params *P){

  if (openReadSource(P, FilePath)){
    fprintf(stderr,"Data source - file or stream - unopenable.\n");
    return;
  }

  const int bufferSize = 4096;
  unsigned char buffer[bufferSize];
  unsigned id;
  int num, pnum;
  int leadMinutes=0;
  long detectionID=0, fdetectionID=0;
  float U=0,V=0;
  date_time_t dataTime;

  double x[MaxNumPoints];
  double y[MaxNumPoints];
  double lat[MaxNumPoints];
  double lon[MaxNumPoints];


  //
  // Get the key.
  //
  readFromSource(buffer, 2);
  PMU_auto_register("Got new ID");

  id = decode2bytesUnsigned(buffer,P->byteSwap);
  int seqNum=0;
  do {

    if (P->debug){
      fprintf(stderr,"Processing Record with ID %d\n",id);
    }

    //
    // Act appropriately. Set up output object first.
    //
    bool weWantThisOne;
    SpdbOut *S = new SpdbOut();
    switch( id ){

    case frameStartRecordID :
      readFromSource(buffer,frameStartRecordSize);
      break;

    case tiltHeaderRecordID :
      readFromSource(buffer, tiltHeaderRecordSize);
      break;
 
    case tiltTrailerRecordID :
      readFromSource(buffer, tiltTrailerRecordSize);
      break;

    case frameEndRecordID :
      readFromSource(buffer, frameEndRecordSize);
      break;

    case detectionRecordID :

      readFromSource(buffer, detectionRecordSize);
      num = decode2bytes(buffer+50,P->byteSwap);
      detectionID = decode4bytes(buffer+40,P->byteSwap);
      U = decode2bytes(buffer+18,P->byteSwap)/100.0;
      V = decode2bytes(buffer+20,P->byteSwap)/100.0;
      decodeTime( buffer+6, &dataTime, 0,P->byteSwap);

      if (P->debug){
	fprintf(stderr, 
		"Detection ID %ld at %s has %d points, moving at %g,%g m/s\n",
		detectionID,utimstr(dataTime.unix_time),num,U,V);
      }

      for (int k=0; k<num; k++){
	readFromSource(buffer, 2);
	x[k] = decode2bytes(buffer,P->byteSwap) / 100.0;
      }
      readFromSource(buffer, 2);
      pnum = decode2bytes(buffer,P->byteSwap);
      if (pnum != num) {
	fprintf(stderr, "Nx != Ny\n");
      }
      for (int k=0; k<num; k++){
	readFromSource(buffer, 2);
	y[k] =  decode2bytes(buffer,P->byteSwap)/ 100.0;
      }
      readFromSource(buffer, 2); // delta V - not needed.

      ConvertXY2LatLon(num,x,y,lat,lon,P->radar_lat,P->radar_lon);

      seqNum = 0;
      leadMinutes = 0;
      //
      // Write this out, if we want this lead time.
      //
      weWantThisOne = false;
      for (int k=0; k < P->desiredLeadTimes_n; k++){
	if (P->_desiredLeadTimes[k] == leadMinutes){
	  weWantThisOne = true;
	}
      }

      if (weWantThisOne){
	S->WriteOut(P,num,lat,lon,dataTime.unix_time,leadMinutes,
		    detectionID,seqNum,U,V);
      }
      break;

    case forecastRecordID :
      readFromSource(buffer, forecastRecordSize);
      num = decode2bytes(buffer+30,P->byteSwap);
      leadMinutes = decode2bytes(buffer+18,P->byteSwap);
      fdetectionID = decode4bytes(buffer+22,P->byteSwap);
      decodeTime( buffer+6, &dataTime, 60*leadMinutes,P->byteSwap );

      if (P->debug){
	fprintf(stderr, "%d minute forecast for %s has %d points (ID %ld).\n",
		leadMinutes, utimstr(dataTime.unix_time), num, fdetectionID);
      }

      for (int k=0; k<num; k++){
	readFromSource(buffer, 2);
	x[k] = decode2bytes(buffer,P->byteSwap) / 100.0;
      }
      readFromSource(buffer, 2);
      pnum = decode2bytes(buffer,P->byteSwap);
      if (pnum != num) {
	fprintf(stderr, "Nx != Ny\n");
      }
      for (int k=0; k<num; k++){
	readFromSource(buffer, 2);
	y[k] = decode2bytes(buffer,P->byteSwap) / 100.0;
      }
      
      ConvertXY2LatLon(num,x,y,lat,lon,P->radar_lat,P->radar_lon);
      seqNum++;

      //
      // Write this out, if we want this lead time.
      //
      weWantThisOne = false;
      for (int k=0; k < P->desiredLeadTimes_n; k++){
	if (P->_desiredLeadTimes[k] == leadMinutes){
	  weWantThisOne = true;
	}
      }
      if (weWantThisOne){
	if (fdetectionID == detectionID){
	  S->WriteOut(P,num,lat,lon,dataTime.unix_time,leadMinutes,
		      fdetectionID,seqNum,U,V);
	} else {
	  S->WriteOut(P,num,lat,lon,dataTime.unix_time,leadMinutes,
		      fdetectionID,seqNum,0.0,0.0);
	}
      }

      break;


    case areYouThereID :
      //
      // This is a message the MIGFA system puts out every few
      // minutes, apparently just to see if anyone is listening.
      // Take this as a cue to write out a zero-length boundary
      // just so that this program is seen to be functioning.
      //
      time_t dummyDataTime;
      dummyDataTime = time(NULL);
      if (P->debug){
	fprintf(stderr,"Got are you there message at %s - writing 0 length boundary ....\n", 
		utimstr(dummyDataTime));
      }
      S->WriteOut(P, 0, NULL, NULL, dummyDataTime, 0, 99, 99, 0.0, 0.0);
      //
      // Do not break at the end of this case - still
      // have to process the areYouThereID message, in the
      // same way as we have to skip all unidentified IDs.
      //
    default :
      //
      // Unknown ID - the next two bytes indicate the length in words.
      // Just read in the length, then read in that many words
      // (1 word == 2 bytes) to skip it. Note that we have already read
      // one word - the ID - so we have to adjust for that.
      //
      if (P->debug){
	if (id != (signed) areYouThereID){
	  time_t now = time(NULL);
	  fprintf(stderr,"Unrecognized ID at %s : %d - skipping ....\n", 
		  utimstr(now), id);
	}
      }
      readFromSource(buffer, 2); // Have now read two words - the ID and the length.
      long length = (long) decode2bytesUnsigned(buffer,P->byteSwap);
      length = length - 2; // Adjust for the two words we have already read.

      long byteLen = 2*length;

      if (byteLen < 0){
	fprintf(stderr,"ERROR : Negative length : %ld\n", byteLen);
	fprintf(stderr,"Exiting.\n");
	exit(-1);
      }

      if (P->debug){
	fprintf(stderr,"Byte length is : %ld\n",byteLen);
      }

      if (byteLen <= bufferSize){
	readFromSource(buffer, byteLen);
      } else {
	//
	// Rather a long message - read it in chunks.
	//
	int bytesToRead = byteLen;
	do {
	  PMU_auto_register("Reading skippable data ...");
	  if (bytesToRead >= bufferSize){
	    readFromSource(buffer, bufferSize);
	    bytesToRead = bytesToRead - bufferSize;
	  } else {
	    readFromSource(buffer, bytesToRead);
	    bytesToRead = 0;
	  }
	  if (P->debug){
	    time_t n = time(NULL);
	    fprintf(stderr,
		    "At %s bytes left to read in %ld byte skip message : %d\n",
		    utimstr(n), byteLen, bytesToRead);
	    fflush(stderr);
	  }
	} while (bytesToRead > 0);
      }
      break;
    }
    delete S;
    //
    // Get the next key.
    //
    pnum=readFromSource(buffer, 2);
    id = decode2bytesUnsigned(buffer, P->byteSwap);
    //
  } while(pnum==2);

  closeReadSource();
  return;


}

int Migfa2Spdb::decode2bytes(unsigned char *b, bool byteSwap){

  if (sizeof(short) !=2){
    fprintf(stderr,"short ints not 2 bytes - cannot cope.\n");
    exit(-1);
  }

  if (byteSwap){
    short s;
    unsigned char *q = (unsigned char *)&s;
    q[0]=b[1]; q[1]=b[0];
    return (int) s;
  }
  //
  // The following is untested.
  //
  short s;
  unsigned char *q = (unsigned char *)&s;
  q[0]=b[0]; q[1]=b[1];
  return (int) s;


}

/////////////////////////////////////////////////////////////////
//
// Same thing, but unsigned.
//

unsigned int Migfa2Spdb::decode2bytesUnsigned(unsigned char *b, bool byteSwap){

  if (sizeof(short) !=2){
    fprintf(stderr,"short ints not 2 bytes - cannot cope.\n");
    exit(-1);
  }

  if (byteSwap){
    unsigned short s;
    unsigned char *q = (unsigned char *)&s;
    q[0]=b[1]; q[1]=b[0];
    return (unsigned int) s;
  }
  //
  // The following is untested.
  //
  unsigned short s;
  unsigned char *q = (unsigned char *)&s;
  q[0]=b[0]; q[1]=b[1];
  return (unsigned int) s;


}


///////////////////////////////////////////////////////////
//
long Migfa2Spdb::decode4bytes(unsigned char *b, bool byteSwap){


  if (sizeof(long) !=4){
    fprintf(stderr,"long ints not 4 bytes - cannot cope.\n");
    exit(-1);
  }


  if (byteSwap){
    long l;
    unsigned char *q = (unsigned char *)&l;
    q[0]=b[3]; q[1]=b[2]; q[2]=b[1]; q[3] = b[0];
    return  l;
  }
  //
  // The following is untested.
  //
  long l;
  unsigned char *q = (unsigned char *)&l;
  q[0]=b[2]; q[1]=b[3]; q[2]=b[0]; q[3] = b[1];
  return  l;



}

///////////////////////////////////////////////////////////
//
void Migfa2Spdb::decodeTime( unsigned char *b, date_time_t *dataTime, long leadTime, bool byteSwap ){

  dataTime->month = decode2bytes(b,byteSwap);
  dataTime->day = decode2bytes(b+2,byteSwap);
  dataTime->year = decode2bytes(b+4,byteSwap);
  dataTime->hour = decode2bytes(b+6,byteSwap);
  dataTime->min = decode2bytes(b+8,byteSwap);
  dataTime->sec = decode2bytes(b+10,byteSwap);

  if (dataTime->year < 1900) dataTime->year = dataTime->year + 1900;
  //
  // At the time of writing I have only one sample data
  // file from 1994. It codes the year as 94. I don't
  // know if 2002 will be coded as 02 or 102, hence
  // the following line.
  //
  if (dataTime->year < 1950) dataTime->year = dataTime->year + 100;

  uconvert_to_utime( dataTime );
  dataTime->unix_time = dataTime->unix_time + leadTime;
  uconvert_from_utime( dataTime );

}

///////////////////////////////////////////////////////////
//
void Migfa2Spdb::ConvertXY2LatLon(int num,
		      double *x,
		      double *y,
		      double *lat,
		      double *lon,
		      double radar_lat,
		      double radar_lon){

  for(int i=0; i < num; i++){
     PJGLatLonPlusDxDy(radar_lat, radar_lon, x[i], y[i], lat+i, lon+i);
  }

}

////////////////////////////////////////////////////////////////
//
// Open read source, be it file or stream.
//
int Migfa2Spdb::openReadSource(Params *P, char *FileName){
  //
  // Decide if we are reading from a file.
  //
  if (P->mode == Params::REALTIME_STREAM){
    _readingFromFile = false;
  } else {
    _readingFromFile = true;
  }
  //
  // And open the source appropriately.
  //
  int retVal = 0;
  if ( _readingFromFile ){
    //
    // Set up for file reads.
    //
    _fp = fopen(FileName,"rb");
    if (_fp == NULL){
      retVal = -1;
    }
  } else {
    retVal = _S.open(P->hostname, P->port);
  }

  if (retVal != 0){
    if ( _readingFromFile ){
      fprintf(stderr,"Failed to read from file %s\n",
	      FileName);
    } else {
      fprintf(stderr,"Attempt to open port %d at %s returned %d\n",
	      P->port, P->hostname, retVal);

      fprintf(stderr,"Error string : %s\n",_S.getErrString().c_str());

      switch(_S.getErrNum()){

      case Socket::UNKNOWN_HOST :
	fprintf(stderr,"Unknown host.\n");
	break;

      case Socket::SOCKET_FAILED :
	fprintf(stderr,"Could not set up socked (maxed out decriptors?).\n");
	break;
      
      case Socket::CONNECT_FAILED :
	fprintf(stderr,"Connect failed.\n");
	break;
 
      case Socket::TIMED_OUT :
	fprintf(stderr,"Timed out..\n");
	break;

      case Socket::UNEXPECTED :
	fprintf(stderr,"Unexpected error.\n");
	break;

      default :
	fprintf(stderr,"Unknown error.\n");
	break;

      }


    }
  }
  return retVal;
}

////////////////////////////////////////////////////////////////
//
// Close read source, be it file or stream.
//
void Migfa2Spdb::closeReadSource(){
  if (_readingFromFile){
    fclose(_fp);
  } else {
    _S.close();
    _S.freeData();
  }
}

///////////////////////////////////////////////////
//
// Read N bytes from the input source into buffer.
// Size of buffer to read to not checked.
// Returns number of bytes read.
//
int Migfa2Spdb::readFromSource(unsigned char *buffer, int numbytes){

  int retVal;
  if (_readingFromFile){
    retVal = fread(buffer, sizeof(unsigned char), numbytes, _fp);
  } else {

    if (_S.readSelectPmu()){

      switch (_S.getErrNum()){

      case Socket::TIMED_OUT :
	fprintf(stderr,"Read select timed out.\n");
	exit(-1);
	break;

      case Socket::SELECT_FAILED :
	fprintf(stderr,"Read select failed.\n");
	exit(-1);
	break;

      case Socket::UNEXPECTED :
	fprintf(stderr,"Read select - unexpected error.\n");
	exit(-1);
	break;

      default :
	fprintf(stderr,"Unkown error with read select.\n");
	exit(-1);
	break;

      }
    }

    if (_S.readBufferHb(buffer,
			numbytes,
			numbytes,
			(Socket::heartbeat_t)PMU_auto_register) != 0 ){

      switch (_S.getErrNum()){

      case Socket::TIMED_OUT :
	fprintf(stderr,"Read buffer timed out.\n");
	exit(-1);
	break;

      case Socket::BAD_BYTE_COUNT :
	fprintf(stderr,"Read buffer gave bad byte count.\n");
	exit(-1);
	break;

      default :
	fprintf(stderr,"Unkown error with read buffer.\n");
	exit(-1);
	break;
      }

      return -1;
    }

    retVal = _S.getNumBytes();
  }
  
  // Debugging............................................


  if (retVal < numbytes){
    fprintf(stderr,"%d of %d bytes read from source :\n",
	    retVal, numbytes);
    for(int i=0; i < retVal/2; i=i+2){
      fprintf(stderr,"%d %d\n",i/2,decode2bytes(buffer+i,true));
    }
  }

  return retVal;
}




