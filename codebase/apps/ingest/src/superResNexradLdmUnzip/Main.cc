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

#include <bzlib.h>
#include <cstdio>

#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <didss/LdataInfo.hh>
#include <dataport/swap.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <toolsa/ServerSocket.hh>
#include <toolsa/Socket.hh>

#include "fileFetch.hh"

#include "Args.hh"
#include "Params.hh"

static void tidy_and_exit (int sig);


void debug31(unsigned char *p){

  unsigned char *b = p + 16; // Skip 16 byte header to get to message 31

  int eNum = b[22];
  int aNum = 256*b[10]+b[11];
  
  float az,el;
  unsigned char *azp=(unsigned char *) &az;
  unsigned char *elp=(unsigned char *) &el;
  
  azp[3]=b[12];
  azp[2]=b[13];
  azp[1]=b[14];
  azp[0]=b[15];
  
  elp[3]=b[24];
  elp[2]=b[25];
  elp[1]=b[26];
  elp[0]=b[27];
  
  fprintf(stderr,"   Message 31 placement : Az %d (%g deg), el %d (%g deg)\n",
	  aNum, az, eNum, el);

  return;
}


unsigned char *checkMalloc(int size){
  unsigned char *b;
  b = (unsigned char *) malloc( size );
  if (b == NULL){
    fprintf(stderr,"Malloc failed!\n");
    exit(-1);
  }
  return b;
}

using namespace std;

int main(int argc, char *argv[])

{
  
  // Trap signals with reference to the 'tidy_and_exit' routine.
  
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);

  // set program name

  string progName = "superResNexradLdmUnzip";
  ucopyright((char *) progName.c_str());
  
  // get command line args

  Args args;
  if (args.parse(argc, argv, progName)) {
    cerr << "ERROR: " << progName << endl;
    cerr << "Problem with command line args" << endl;
    return -1;
  }

  // get TDRP params
  
  char *paramsPath = (char *)"unknown";
  Params P;
  if (P.loadFromArgs(argc, argv, args.override.list,
                     &paramsPath)) {
    cerr << "ERROR: " << progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    return -1;
  }

  // initialize procmap registration

  PMU_auto_init("superResNexradLdmUnzip", P.instance,
                PROCMAP_REGISTER_INTERVAL);  

  //
  // If we are writing to a socket, wait for a client
  //
  ServerSocket server;
  Socket *socket = NULL;

  if (P.socketOutput.writeToSocket){
    int status = server.openServer(P.socketOutput.socketNum);
    if (status != 0) {
      fprintf(stderr,"Failed to open server.\n");
      fprintf(stderr, "%s\n", server.getErrString().c_str() ); 
      return -1; // Maybe we want to loop and retry here? Not sure.
    }

    //
    // Wait until client calls.
    //
    do {
      PMU_auto_register("Waiting for client");
      socket = server.getClient(1000);
    } while (socket == NULL);
  }

  fileFetch F(P.inDir, P.maxFileAgeSec, P.deleteFiles, (char *)".BZIP2", 
	      P.debug, P.timeoutSecs, P.skipSecs,
	      P.checkFileTime.checkFilenameTime, 
	      P.checkFileTime.maxTimeDiffSecs );

  FILE *ofp = NULL;
  char fullOutName[1024];
  
  unsigned long numWrittenOut = 0L;

  while (1){
    char inFileName[1024];
    date_time_t filenameTime;
    int seqNum;
    bool isEOV;

    char outfileName[1024];
    char outNameMinusExt[1024];

    F.getNextFile( inFileName, &filenameTime, &seqNum, &isEOV );

    struct stat fileDetails;
    if (stat(inFileName, &fileDetails)){
      fprintf(stderr, "Could not stat %s\n", inFileName);
      continue;
    }

    if (P.debug){
      cerr << "Have file : " << inFileName << endl;
    }

    //
    // If this is the start of a volume, close the output file.
    // This can happen if we time out looking for a file. In this
    // case we do NOT want to write an ldatainfo file, we only do
    // that if we made it to the end of the volume. In fact in this case
    // we delete the output.
    //
    if (seqNum == 1){
      if (ofp != NULL){
	fclose(ofp);
	ofp = NULL;
	unlink(fullOutName);
      }
    }

    if ((ofp == NULL) && (P.outputFiles)){
      //
      // Time to open a new output file.
      //
      sprintf(outfileName,"%d%02d%02d_%02d%02d%02d.nexDat",
	      filenameTime.year, filenameTime.month, filenameTime.day,
	      filenameTime.hour, filenameTime.min, filenameTime.sec);

      sprintf(outNameMinusExt,"%d%02d%02d_%02d%02d%02d",
	      filenameTime.year, filenameTime.month, filenameTime.day,
	      filenameTime.hour, filenameTime.min, filenameTime.sec);
      
      sprintf(fullOutName,"%s/%s", P.outDir, outfileName);

      if (ta_makedir_recurse(P.outDir)){
	fprintf(stderr,"Failed to create %s\n", P.outDir);
	exit(-1);
      }

      ofp = fopen(fullOutName, "w");
      if (ofp == NULL){
	fprintf(stderr,"Failed to create %s\n", outfileName);
	exit(-1);
	continue;
      }
    }



    FILE *ifp = fopen(inFileName, "r");
    if (ifp == NULL){
      fprintf(stderr,"Failed to open %s\n", inFileName);
      continue;
    }


    while( !feof(ifp) ) {
      //
      // Sometimes there seems to be a header at the
      // start of a message that looks like it needs to be skipped.
      // The length varies.
      //
      // We know that we are at a valid place to start if either :
      //
      // * The first and second bytes are "AR", or 
      //
      // * The fifth and sixth bytes are "BZ", BUT they are not
      // part of a string spelling "BZIP". Check if this is the case, if not skip
      // forward a byte at a time until it is.
      //
      bool ok2start = false;
      int numBytesSkipped = 0;
      do {
	unsigned char b[9];
	if (9 != fread(b, sizeof(unsigned char), 9, ifp)){
	  break; // Probably EOF
	}
	
	fseek(ifp, -9L, SEEK_CUR); // Rewind back past the bytes we read.
	
	if ((b[0] == 'A') && (b[1] == 'R')) ok2start = true;
	if ((b[4] == 'B') && (b[5] == 'Z')){
	  if (strncmp((char *) &b[4], (char *)"BZIP2", strlen((char *)"BZIP2"))){
	    ok2start = true;
	  }       
	}
	if (!(ok2start)){
	  fseek(ifp, 1L, SEEK_CUR); // Skip to the next byte if we failed here.
	  numBytesSkipped++;
	}
      } while( !(ok2start));
      
      if (feof(ifp)) break;

      if (numBytesSkipped) fprintf(stderr,"WARNING : Had to skip %d bytes to re-sync\n", numBytesSkipped);

      ///////// AS READY TO READ AS WE CAN BE ///////
    
      
      //
      // Read the first four bytes. If this is the string "ARCH" or
      // the string "AR2V" then this is the first four bytes of a
      // 24 byte header that we should read and write out.
      // If it is not, then this is a four-byte integer (network
      // byte order) length that we should read, decompress and write
      // out.
      //
      
      if (P.debug){
	double pd = 100.0*rint(double(ftell(ifp))/double(fileDetails.st_size));
	fprintf(stderr,"Reading at offset %ld bytes, %g percent of file\n",
		ftell(ifp), pd);
      }

      si32 len;
      if (fread(&len, 1, 4, ifp) != 4) {
	//
	// Could just be end of input.
	//
	if (feof(ifp)){
	  break;
	} else {
	  fprintf(stderr,"ERROR reading length.\n");
	  exit(-1);
	}
      }

      if ( strncmp((char *) &len, "ARCH", 4) == 0 || 
	   strncmp((char *) &len, "AR2V", 4) == 0 ) {

	if (P.debug) fprintf(stderr, "Header detected.\n");
	
	//
	// It is a header.
	//
	if (P.outputFiles){
	  if (fwrite(&len, 1, 4, ofp) != 4){
	    fprintf(stderr,"ERROR writing header bytes.\n");
	    exit(-1);
	  }
	}

	if (P.socketOutput.writeToSocket){
	  int iWrite = socket->writeBuffer(&len,4);
	  if (iWrite!=0){
	    fprintf(stderr, "Error writing to socket! Error %d\n", iWrite);
	    exit(-1);
	  }
	}

	unsigned char *b = checkMalloc(20);
	if (fread(b, 1, 20, ifp) != 20) {
	  fprintf(stderr,"ERROR reading header bytes part 2.\n");
	  exit(-1);
	}

	if (P.outputFiles){
	  if (fwrite(b, 1, 20, ofp) != 20){
	    fprintf(stderr,"ERROR writing header bytes part 2.\n");
	    exit(-1);
	  }
	}

	if (P.socketOutput.writeToSocket){
	  int iWrite = socket->writeBuffer(b, 20);
	  if (iWrite != 0){
	    fprintf(stderr, "error writing to socket : %d\n", iWrite);
	    exit(-1);
	  }
	}

	free(b);

	if ((P.debug) && (numWrittenOut))
	  fprintf(stderr,"New volume, %ld bytes were written for the last volume\n", numWrittenOut);

	numWrittenOut = 24;

	//
	// End of processing header - continue to next record.
	//
	continue;
      }

      //
      // If we got here - it was not a header - byte swap length.
      //
      si32 length = SWAP_si32( len );

      if (length <0){
	length = -length; // Seems to do this on end of volume.
      }

      if (P.debug) fprintf(stderr,"Length of message is %d bytes.\n", length);

      //
      // Read from file.
      //
      unsigned char *inBuffer = checkMalloc( length );
      if (fread(inBuffer, 1, length, ifp) != (unsigned)length) {
	exit(-1);
      }
    
      if (P.debug){
	double pd = 100.0*rint(double(ftell(ifp))/double(fileDetails.st_size));
	fprintf(stderr,"Read until offset %ld bytes, %g percent of file\n",
		ftell(ifp), pd);
      }


      //
      // First two bytes should be "BZ" if this is a Bzip archive.
      //
      if (strncmp("BZ", (char *)inBuffer, 2) != 0){
	fprintf(stderr, "WARNING : MAGIC BZ CHARACTERS NOT FOUND.\n");
      }
    

      int sizeFactor = 1;
      int error = 0;
      unsigned char *outBuffer;
      unsigned int olength;
    
      do {

	olength = 200*length*sizeFactor;
	if (olength < 250000) olength = 250000; // Err on safe side.
      
	outBuffer = checkMalloc( olength );
      
#ifdef BZ_CONFIG_ERROR
	error = BZ2_bzBuffToBuffDecompress((char *) outBuffer, &olength,
					   (char *) inBuffer, length, 0, 0);
#else
	error = bzBuffToBuffDecompress((char *) outBuffer, &olength,
				       (char *) inBuffer, length, 0, 0);
#endif

	if (error == BZ_OUTBUFF_FULL){
	  free(outBuffer);
	  sizeFactor *= 2;
	  if (sizeFactor > 4097){
	    fprintf(stderr, "Crazy allocation size attemped, exiting\n");
	    exit(-1);
	  }
	}

      } while (error == BZ_OUTBUFF_FULL); // Keep doubling the size of the output buffer until it's enough.
    

      if (error) {
	fprintf(stderr, "BZIP2 uncompress did not work for file %s length was %d (suggested file size is %d) error code %d\n",
		inFileName, length, length+4, error);

	free(inBuffer); free(outBuffer);
	continue;
      }

      //
      // Loop through messages in the data stream.
      //
      int bOffset = 0;

      //
      // Sometimes - usually - at this point the RDA inserts 12 bytes, all 0.
      // If that's the case, skip them.
      //
      int numZeroes = 0;
      for (int q=0; q < 12; q++){
	if ((int)outBuffer[q] == 0) numZeroes++;
      }

      if (numZeroes == 12) bOffset = 12;

      do{
	int msgSize = 256*outBuffer[bOffset]+outBuffer[1+bOffset];
	msgSize = msgSize*2; // Size is given in halfwords but we need bytes.

	int segNum = 256*outBuffer[14+bOffset]+outBuffer[15+bOffset];
	int msgType = (int)outBuffer[3+bOffset];


	//
	// Do message 31 debugging, if requested
	//
	if ((msgType == 31) && (P.debug31)){
	  debug31(outBuffer + bOffset);
	}


	//
	// Amount to move forward in stream depends on message type.
	//
	int numBytes=0;
	if (msgType == 31)
	  numBytes = msgSize+12;
	else
	  numBytes = msgSize+16;

	if ((P.debug) && (msgType != 0)){
	  int startPD = (int)rint(100.0*double(bOffset)/double(olength));
	  int endPD = (int)rint(100.0*double(bOffset+numBytes)/double(olength)); if (endPD > 100) endPD = 100; // Avoid confusion for erroneous run lengths
	  fprintf(stderr,"Message type %d encountered, length %d, segment %d at offset %d of run %d (%d to %d percent)\n", 
		  msgType, numBytes, segNum, bOffset, olength, startPD, endPD);
	}

	bool sendThis = false;
	if (!(P.checkMsgType)){
	  sendThis = true;
	} else {
	  if (msgType == 31) sendThis = true;
	}

	if ((msgType == 0) && (seqNum == 0) && (msgType == 16)) sendThis = false; // All zeroes - always wrong - the RDA seems to do this
	
	if (sendThis){
	
	  if (P.outputFiles){
	    if (P.debug) fprintf(stderr,"Sending message type %d to file\n", msgType);
	    if (numBytes != (int)fwrite(outBuffer+bOffset, 1, numBytes, ofp)){
	      fprintf(stderr,"ERROR writing buffer length %d\n",olength);
	      exit(-1);
	    }
	  }
	
	  if (P.socketOutput.writeToSocket){
	    if (P.debug) fprintf(stderr,"Sending message type %d to socket\n", msgType);
	    int iWrite = socket->writeBuffer(outBuffer+bOffset, numBytes);
	    if (iWrite!=0){
	      fprintf(stderr, "Error %d writing to socket\n", iWrite);
	      exit(-1);
	    }
	  }

	  numWrittenOut += numBytes;

	} else {
	  if ((P.debug) && (msgType != 0))
	    fprintf(stderr,"Skipping message type %d\n", msgType);
	}

	bOffset += numBytes;
      } while ((unsigned)bOffset < olength);

      free(inBuffer); free(outBuffer);

    }

    fclose(ifp);

    if ((P.debug) && (P.outputFiles)){
      fprintf(stderr,"Making %s from %s\n",
	      fullOutName, inFileName);
    }

    if (isEOV){
     //
      // Last thing - send a message of our own devising to indicate EOV.
      // Message type 51, sent 4 times so there is no mistake.

      //
      // Set up message - 2416 byte message plus 16 byte header = 2432 bytes
      unsigned char *msg51 = (unsigned char *)calloc(2432, sizeof(unsigned char));
      if (NULL == msg51){
	fprintf(stderr, "Message 51 allocation failed.\n");
	exit(-1);
      }

      msg51[3]=51; // Message type
  
      msg51[0]=4;
      msg51[1]=184; // Message size in halfwords without 16 byte header is 1208 which is 4*256+184


      if (P.debug)
	fprintf(stderr,"Sending UCAR contrived message 51 four times\n");
      
      for (int fcount=0; fcount < 4; fcount++){

	numWrittenOut += 2432;
	
	if (P.outputFiles){ // Send to file	  
	  if (2432 != fwrite(msg51,sizeof(unsigned char), 2432, ofp)){
	    fprintf(stderr,"Failed to send message type 51 to file\n");
	    exit(-1);
	  }
	}

	if (P.socketOutput.writeToSocket){
	  if (socket->writeBuffer(msg51, 2432)!=0){
	    fprintf(stderr, "Error writing message 51 to socket\n");
	    exit(-1);
	  }
	}
	
      }
      free(msg51);
    }

    if ((P.outputFiles) && (isEOV)){
 
      //
      // Time to close this output file and write ldatainfo
      //
      fclose(ofp); ofp = NULL;
      //
      // OK - now write an ldatainfo file to outDir.
      //
      LdataInfo L;
      L.setWriter( "superResNexradLdmUnzip" );
      L.setDir(P.outDir);
      L.setDataFileExt( "nexDat" );
      L.setUserInfo1( outNameMinusExt );
      L.setUserInfo2( outfileName );
      L.setRelDataPath( outfileName );
      L.setDataType( "nexDat" );
      if ( L.write( filenameTime.unix_time )){
	fprintf(stderr, "Writing of latest data info for %s failed.\n", fullOutName);
      }

      for (int i=0; i < P.sleepSec; i++){
	sleep(1);
	PMU_auto_register("Sleeping after file process");
      }
    }
  }
  
  //
  // In fact never get here.
  //
  return 0;

}

/////////////////////////////////////////////
//
// Small routine called in the event of an interrupt signal.
//
static void tidy_and_exit (int sig){
  PMU_auto_unregister();
  exit(sig);
}


