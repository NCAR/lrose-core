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
//
//
#include "superResNexradRawUnzip.hh"

#include <bzlib.h>

#include <iostream>
#include <netcdf.h>
#include <stdlib.h>
#include <cstdio>

#include <dataport/swap.h>
#include <didss/LdataInfo.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>
#include <toolsa/pjg_flat.h>


unsigned char *superResNexradRawUnzip::_checkMalloc(int size){
  unsigned char *b;
  b = (unsigned char *) malloc( size );
  if (b == NULL){
    fprintf(stderr,"Malloc failed!\n");
    exit(-1);
  }
  return b;
}





//
// Constructor - makes a copy of a pointer to the TDRP parameters.
//
superResNexradRawUnzip::superResNexradRawUnzip(Params *P){
  _params = P;
  return;
}
//
// Destructor - does nothing but avoids default destructor.
//
superResNexradRawUnzip::~superResNexradRawUnzip(){
  return;
}
//
// Main method 
//
void superResNexradRawUnzip::superResNexradRawUnzipFile( char *FilePath ){
  //
  
  //
  // Construe output filename from input filename.
  // Input files are named like unto :
  //
  // KOUN20100405041727V06.raw
  //
  if (strlen(FilePath) < strlen("KOUN20100405041727V06.raw")){
    fprintf(stderr, "%s does not follow KOUN20100405041727V06.raw naming convention - skipping\n",
	    FilePath);
    return;
  }

  char *p = FilePath + strlen(FilePath) - strlen("KOUN20100405041727V06.raw");

  date_time_t dataTime;
  if (6 != sscanf(p+4,"%4d%2d%2d%2d%2d%2d",
		  &dataTime.year, &dataTime.month, &dataTime.day,
		  &dataTime.hour, &dataTime.min, &dataTime.sec)){
    fprintf(stderr, "Could not parse time from %s\n", FilePath);
    return;
  }

  uconvert_to_utime( &dataTime );

  char outFileName[1024];
  sprintf(outFileName,"%s/%s.unzipped", _params->OutDir, p);

  FILE *ifp = fopen(FilePath, "r");
  if (ifp == NULL){
    fprintf(stderr,"Failed to open %s\n", FilePath);
    return;
  }

  if (ta_makedir_recurse( _params->OutDir )){
    fprintf(stderr, "Failed to create directory %s\n", _params->OutDir);
    fclose(ifp);
    return;
  }

  FILE *ofp = fopen(outFileName, "w");
  if (ofp == NULL){
    fprintf(stderr,"Failed to create %s\n", outFileName);
    fclose(ifp);
    return;
  }

  if (_params->debug) fprintf(stderr,"Converting %s to %s\n", FilePath, outFileName);

  int numRec = 0;
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
    // 24 byte header. that we should read and write out.
    // If it is not, then this is a four-byte integer (network
    // byte order) length that we should read, decompress and write
    // out.
    //
    
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

      //
      // It is a header.
      //
      if (_params->debug) fprintf(stderr, "24 byte header record encountered.\n");

      if (fwrite(&len, 1, 4, ofp) != 4){
	fprintf(stderr,"ERROR writing header bytes.\n");
	exit(-1);
      }

      unsigned char *b = _checkMalloc(20);
      if (fread(b, 1, 20, ifp) != 20) {
	fprintf(stderr,"ERROR reading header bytes part 2.\n");
	exit(-1);
      }

      if (fwrite(b, 1, 20, ofp) != 20){
	fprintf(stderr,"ERROR writing header bytes part 2.\n");
	exit(-1);
      }

      free(b);
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
    //
    // Read from file.
    //
    unsigned char *inBuffer = _checkMalloc( length );
    if (fread(inBuffer, 1, length, ifp) != (unsigned)length) {
      fprintf(stderr,"Failed to read buffer from %s\n", FilePath);
      exit(-1);
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
      
      outBuffer = _checkMalloc( olength );

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
	      FilePath, length, length+4, error);

      free(inBuffer); free(outBuffer);
      continue;
    }

    int msgType = (int)outBuffer[15];

    if (_params->debug){
      int pi = 100 * length / olength;
      fprintf(stderr, "%d byte record unzipped to %d bytes (zipped was %d percent of unzipped) message type %d.\n", 
	      (int)length, olength, pi, msgType);
    }
    numRec++;


    // Write out the result for message types 2 and 31.

    if ((msgType == 31) || (msgType == 2)){ // Only message types we want.
      if (olength != fwrite(outBuffer, 1, olength, ofp)){
	fprintf(stderr,"ERROR writing buffer length %d\n",olength);
	exit(-1);
      }
    }

    free(inBuffer); free(outBuffer);

  }

  fclose(ifp);

    //
  // Time to close this output file and write ldatainfo
  //
  fclose(ofp);
  //
  // OK - now write an ldatainfo file to outDir.
  //
  char outfileName[1024];
  char outNameMinusExt[1024];

  sprintf(outfileName, "%s.unzipped", p);

  sprintf(outNameMinusExt, "%s", outfileName);
  int id = strlen(outfileName) - strlen(".raw.unzipped");
  outNameMinusExt[id]=char(0);

  LdataInfo L;
  L.setWriter( "superResNexradrawUnzip" );
  L.setDir(_params->OutDir);
  L.setDataFileExt( ".raw.unzipped" );
  L.setUserInfo1( outNameMinusExt );
  L.setUserInfo2( outfileName );
  L.setRelDataPath( outfileName );
  L.setDataType( ".raw.unzipped" );
  if ( L.write( dataTime.unix_time )){
    fprintf(stderr, "Writing of latest data info for %s failed.\n", outFileName);
  }

  if (_params->debug) fprintf(stderr,"%d records processed for %s\n", numRec, FilePath);

  return;

}

