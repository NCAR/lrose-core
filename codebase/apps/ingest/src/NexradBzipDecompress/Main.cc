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
#include <stdlib.h>
#include <dataport/swap.h>
#include <cstring>
#include <toolsa/file_io.h>

//
// Small program to decompress nexrad data from
// the LDM. Niles Oien July 2006.
//


unsigned char *checkMalloc(int size){
  unsigned char *b;
  b = (unsigned char *) malloc( size );
  if (b == NULL){
    fprintf(stderr,"Malloc failed!\n");
    exit(-1);
  }
  return b;
}

int main(int argc, char *argv[]){

  if (argc < 3){
    fprintf(stderr,"USAGE : NexradBzipDecompress <infile> <outfile> [radar]\n");
    return -1;
  }

  char *inFilename = argv[1];
  FILE *ifp = fopen(inFilename, "r");
  if (ifp == NULL){
    fprintf(stderr, "Failed to open %s\n", inFilename);
    return -1;
  }

  char *outFilename = argv[2];
  FILE *ofp = fopen(outFilename, "w");
  if (ofp == NULL){
    fprintf(stderr, "Failed to create %s\n", outFilename);
    return -1;
  }

  while( !feof(ifp) ) {

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

      // fprintf(stderr,"HEADER FOUND.\n");

      //
      // It is a header.
      //
      if (fwrite(&len, 1, 4, ofp) != 4){
	fprintf(stderr,"ERROR writing header bytes.\n");
	exit(-1);
      }

      unsigned char *b = checkMalloc(20);
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

    // fprintf(stderr,"LENGTH : %d\n", length);

    if (length <0){
      // fprintf(stderr, "INFO : negative length detected.\n");
      length = -length; // Seems to do this on end of volume.
    }
    //
    // Read from file.
    //
    unsigned char *inBuffer = checkMalloc( length );
    if (fread(inBuffer, 1, length, ifp) != (unsigned)length) {
	// fprintf(stderr,"ERROR reading %d input bytes.\n", length);
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
	      inFilename, length, length+4, error);
      if (error == BZ_OUTBUFF_FULL) {
	fprintf(stderr, "  Probably max output file size exceeded.\n" );
      }
      //
      // If the environment variable NEXRAD_DECOMPRESS_FAILED_DIR
      // is defined, make an attempt to copy the file into that directory.
      // Helps with debugging. Niles.
      //
      char *targetTopDir = getenv("NEXRAD_DECOMPRESS_FAILED_DIR");
      if (NULL != targetTopDir) {

	char targetDir[1024];

	if (argc > 3) {
	  sprintf(targetDir, "%s/%s", targetTopDir, argv[3]);
	} else {
	  sprintf(targetDir, "%s", targetTopDir);
	}

	ta_makedir_recurse(targetDir);
	char com[1024];
	sprintf(com, "/bin/cp %s %s", inFilename, targetDir);
	system(com);
      }
      exit(-1);
    }

    // fprintf(stderr, "DECOMPRESS WENT FROM %d TO %d BYTES (FACTOR OF %g)\n",
    //	    length, olength, double(olength)/double(length));

    if (olength != fwrite(outBuffer, 1, olength, ofp)){
      fprintf(stderr,"ERROR writing buffer length %d\n",olength);
      exit(-1);
    }
    
    free(inBuffer); free(outBuffer);

  }

  fclose(ifp); fclose(ofp);

  return 0;

}
