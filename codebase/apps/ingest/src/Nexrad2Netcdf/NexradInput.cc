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
//////////////////////////////////////////////////////////
// NexradInput - handles decompression and buffering of ldm
//           files
//
// Jaimi Yee, RAP, NCAR, P.O.Box 3000, Boulder, CO, 
//   80307-3000, USA
//
// Note: Decompression code copied from NexradBzUncomp.
// See notes in that code to track history.
//
// May 2005
//
// $Id: NexradInput.cc,v 1.12 2016/03/07 01:23:03 dixon Exp $
//
///////////////////////////////////////////////////////////
#include <cerrno>
#include <assert.h>
#include <toolsa/os_config.h>
#include <bzlib.h>
#include <dataport/port_types.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <dsserver/DsLdataInfo.hh>
#include <netinet/in.h>
#include <rapformats/ridds.h>

#include "NexradInput.hh"
#include "Nexrad2Netcdf.hh"
#include "CompressedFile.hh"
using namespace std;

//
// Constants
//
const int NexradInput::MAX_FILE_LEN = 100 * 800 * NEX_PACKET_SIZE;

NexradInput::NexradInput()
{
   volumeFiles       = false;
   ppiFiles          = false;
   headerSize        = (int) sizeof( RIDDS_vol_title );
   bufferSize        = MAX_FILE_LEN;
   bytesInBuffer     = 0;
   header            = new unsigned char[headerSize];
   ldmBuffer         = NULL;
   bufferPtr         = NULL;
   cookieFound       = false;
}

NexradInput::~NexradInput()
{
   delete [] header;
   delete [] ldmBuffer;
}

void NexradInput::init( bool oneFilePerVolume, bool ppiFiles ) 
{
   volumeFiles = oneFilePerVolume;
   this->ppiFiles = ppiFiles;
   if( volumeFiles ) {
      bufferSize = MAX_FILE_LEN;
   }
   if( ppiFiles ) {
      bufferSize = MAX_FILE_LEN;
   }
   ldmBuffer = new unsigned char[bufferSize];
}

void NexradInput::clear() 
{
   cookieFound = false;

   memset( (void *) header, (int) 0, sizeof( unsigned char ) * headerSize );
   memset( (void *) ldmBuffer, (int) 0, sizeof( unsigned char ) * bufferSize );

   bufferPtr     = ldmBuffer;
   bytesInBuffer = 0;
}

int NexradInput::readFile( char* filePath, bool decompress ) 
{
  if( volumeFiles || ppiFiles ) {
    return( readVolumeFile( filePath, decompress ) );
  }
   
   return( readHundredBeamFile( filePath, decompress ) );
}

int NexradInput::readHundredBeamFile( char* filePath, bool decompress ) 
{
   //
   // Setup buffer
   //
   clear();

   //
   // Open file
   //
   FILE *inputFp;
   if ((inputFp = fopen( filePath, "rb") ) == NULL) {
      int errNum = errno;
      POSTMSG( ERROR, "Cannot open file %s, error = %d", 
               filePath, strerror(errNum) );
      return( -1 );
   }
   
   while( !feof(inputFp) ) {

      //
      // Read length
      //
      si32 len;
      if (fread(&len, 4, 1, inputFp) != 1) {
         if (feof(inputFp)) {
            break;
         }
         POSTMSG( ERROR, "Cannot read in length" );
         return -1;
      }

      //
      // Copy to cookie and check
      //
      char cookie[4];
      memcpy(cookie, &len, 4);
      
      if ( strncmp(cookie, "ARCH", 4) == 0 || 
           strncmp(cookie, "AR2V", 4) == 0 ) {

         cookieFound = true;

         POSTMSG( DEBUG, "Found cookie" );

         //
         // Write the cookie to the buffer
         //
         memcpy( (void *) header, (void *) cookie, 4 );

         //
         // Read in header
         //       
         unsigned char* headerPtr = header + 4;
         if((int) fread(headerPtr, 1, headerSize-4, inputFp) != headerSize-4) {
            if (feof(inputFp)) {
               break;
            }
            POSTMSG( ERROR, "Cannot find 24-byte header" );
            return -1;
         }
	 

         continue;
         
      } // found ARCH 

      //
      // Byte-swap len
      //
      int length = ntohl(len);
      if ( length < 0 ) {
         length = -length;
         POSTMSG( DEBUG, "Received negative value for length" );
      }
      if ( length > MAX_FILE_LEN ) {
         POSTMSG( ERROR, "Length too large: %d", length );
         return -1;
      }

      //POSTMSG( DEBUG, "Uncompressing file %s", filePath );

      //
      // Read in buffer
      //
      unsigned char *inBuf = new unsigned char[length];
      if ((int) fread(inBuf, 1, length, inputFp) != length) {
         if (feof(inputFp)) {
            return 0;
         }

         POSTMSG( ERROR, "Cannot read inBuf, nbytes: %d", length );
         delete[] inBuf;
         return -1;
      }
      
      if( decompress ) {

         //
         // uncompress
         //   Note that we uncompress into a temporary buffer. We do this
         //   so that if we get garbage from the decompression routines, or
         //   if we get too much data for the space left in the buffer, we 
         //   don't write goofy or incomplete stuff into our data buffer
         //
         int error;
         unsigned int   olength = bufferSize;
         unsigned char *outBuf  = new unsigned char[olength];
      
#ifdef BZ_CONFIG_ERROR
         error = BZ2_bzBuffToBuffDecompress((char *) outBuf, &olength,
                                            (char *) inBuf, length, 0, 0);
#else
         error = bzBuffToBuffDecompress((char *) outBuf, &olength,
                                        (char *) inBuf, length, 0, 0);
#endif

         delete[] inBuf;

         if (error) {
            POSTMSG( ERROR, "BZIP2 uncompress did not work" );
            if (error == BZ_OUTBUFF_FULL) {
               POSTMSG( ERROR, "  Probably max output file size exceeded" );
            }

            return -1;
         }

         if( (int) olength > (bufferSize - bytesInBuffer) ) {
            POSTMSG( ERROR, "Could not write data from file into buffer" );
            delete [] outBuf;
            return -1;
         }

         memcpy( (void *) bufferPtr, (void *) outBuf, olength );

         bufferPtr     += olength;
         bytesInBuffer += olength;

         delete [] outBuf;
      }
      else {
         memcpy( (void *) bufferPtr, (void *) inBuf, length );
         
         bufferPtr     += length;
         bytesInBuffer += length;

         delete [] inBuf;
      }
      
   }

   //
   // Close file
   //
   fclose( inputFp );

   return 0;
}

int NexradInput::readVolumeFile( char* filePath, bool decompress )
{
   // the decompress flag is now ignored--we decompress the file (or don't) based on its extension (or lack thereof)

   int result = 0;

   //
   // Setup buffer
   //
   clear();

   //
   // Open file
   //
   CompressedFile* pFile = NULL;
   try {    // many of the CopressedFile methods will throw

      int filePathLength = strlen( filePath );
      if ( ( filePathLength > 3 ) && ( strcasecmp( filePath + ( filePathLength - 3 ), ".gz" ) == 0 ) )
         pFile = new GZCompressedFile( filePath );
      else if ( ( filePathLength > 4 ) && ( strcasecmp( filePath + ( filePathLength - 4 ), ".bz2" ) == 0 ) )
         pFile = new BZ2CompressedFile( filePath );
      else if ( ( filePathLength > 2 ) && ( strcasecmp( filePath + ( filePathLength - 2 ), ".Z" ) == 0 ) )
	throw std::runtime_error( "Please uncompress .Z files by hand." );
      else
         pFile = new NotCompressedFile( filePath );
      assert( pFile != NULL );

      //
      // Copy to cookie and check
      //
      char cookie[ 4 ];
      if ( pFile->Read( &cookie, sizeof( cookie ) ) != sizeof( cookie ) )
         throw std::runtime_error( "Cannot read in length" );


      unsigned char* headerPtr = header;
      if ( ( strncmp(cookie, "ARCH", 4 ) == 0 ) || ( strncmp( cookie, "AR2V", 4 ) == 0 ) ) {

         cookieFound = true;

         //
         // Write the cookie to the header
         //
         memcpy( ( void* ) header, ( void* ) cookie, 4 );

         //
         // Read in header
         //       
         headerPtr = header + 4;

         if ( pFile->Read( headerPtr, headerSize - 4 ) != headerSize - 4 )
            throw std::runtime_error( "Cannot find 24-byte header" );
      } else {
	if(!ppiFiles)
	  throw std::runtime_error( "Didn't find cookie in file" );

	memcpy( ( void* ) bufferPtr, ( void* ) cookie, 4 );
	bufferPtr     += 4;
	bytesInBuffer += 4;
      }
   
      while ( ! pFile->IsEOF() ) {
         if ( pFile->Read( bufferPtr, NEX_PACKET_SIZE ) != NEX_PACKET_SIZE ) {
            if ( pFile->IsEOF() )
               break;
            throw std::runtime_error( "Cannot read file" );
         }
      
         bufferPtr     += NEX_PACKET_SIZE;
         bytesInBuffer += NEX_PACKET_SIZE;

	 if(bytesInBuffer + NEX_PACKET_SIZE > bufferSize) {
	   throw std::runtime_error( "File larger than max file size." );
	 }
      }
   }
   catch( std::runtime_error& exception ) {
      POSTMSG( ERROR, exception.what() );
      result = -1;
   }
   if ( pFile != NULL )
      delete pFile;

   return result;
}

