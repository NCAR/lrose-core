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
///////////////////////////////////////////////////////////////
// nexradBzUncomp.cc
//
// nexradBzUncomp object
//
// Mike Dixon, Jaimi Yee,
// RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2004
//
///////////////////////////////////////////////////////////////
//
// nexradBzUncomp unzips nexrad BZIP2 files
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <dsserver/DsLdataInfo.hh>
#include <netinet/in.h>
#include <bzlib.h>
#include "nexradBzUncomp.hh"
using namespace std;

// Constructor

nexradBzUncomp::nexradBzUncomp(int argc, char **argv)

{

  isOK = true;
  _outBuf = NULL;

  // set programe name

  _progName = "nexradBzUncomp";

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }

  // Display ucopyright message

  ucopyright((char *) _progName.c_str());

  // create work buffer

  _outBuf = new unsigned char[_params.max_output_file_size];
  
  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  return;

}

// destructor

nexradBzUncomp::~nexradBzUncomp()

{

   if (_outBuf) {
      delete[] _outBuf;
   }
   
   // unregister process
   
   PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int nexradBzUncomp::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  if (_params.debug) {
     cerr << "Uncompresing input file: " << _params.input_path << endl;
     cerr << "         to output file: " << _params.output_path << endl;
  }
  

  // open input files

  FILE *in;
  if ((in = fopen(_params.input_path, "rb")) == NULL) {
     int errNum = errno;
     cerr << "ERROR - " << _progName << endl;
     cerr << "  Cannot open input file: " <<  _params.input_path << endl;
     cerr << "  " << strerror(errNum) << endl;
     return -1;
  }

  // make sure output dir exists

  Path outPath(_params.output_path);
  if (outPath.makeDirRecurse()) {
     cerr << "ERROR - " << _progName << endl;
     cerr << "  Cannot make output dir: " << outPath.getDirectory() << endl;
     return -1;
  }
  
  // open output file

  FILE *out;
  if ((out = fopen(_params.output_path, "wb")) == NULL) {
     int errNum = errno;
     cerr << "ERROR - " << _progName << endl;
     cerr << "  Cannot open output file: " <<  _params.output_path << endl;
     cerr << "  " << strerror(errNum) << endl;
     return -1;
  }

  int iret = _uncompress(in, out);

  // close file

  fclose(in);
  fclose(out);
  
  return iret;

}

/////////////////////////////////////////////////////////////
// uncompress file
//

int nexradBzUncomp::_uncompress(FILE *in, FILE *out)
   
{
   
   // get start of data "ARCH"
   
   while (!feof(in)) {

      // read length

      si32 len;
      if (fread(&len, 4, 1, in) != 1) {
         if (feof(in)) {
            break;
         }
         cerr << "ERROR - " << _progName << endl;
         cerr << "  Cannot read in length" << endl;
         return -1;
      }

      // copy to cookie and check

      char cookie[8];
      memcpy(cookie, &len, 4);
      
      if (strncmp(cookie, "ARCH", 4)==0 || strncmp(cookie, "AR2V", 4)==0) {

         if (_params.debug) {
            cerr << "Found cookie" << endl;
         }

         // write the cookie
         if (fwrite(cookie, 1, 4, out) != 4) {
            cerr << "ERROR - " << _progName << endl;
            cerr << "  Cannot write cookie" << endl;
            return -1;
         }

         // read in header
   
         unsigned char header[24];
         if (fread(header, 1, 20, in) != 20) {
            if (feof(in)) {
               break;
            }
            cerr << "ERROR - " << _progName << endl;
            cerr << "  Cannot find 20-byte header" << endl;
            return -1;
         }
         
         // write the header
         
         if (fwrite(header, 1, 20, out) != 20) {
            cerr << "ERROR - " << _progName << endl;
            cerr << "  Cannot write header" << endl;
            return -1;
         }

         continue;
         
      } // found ARCH + header

      // byte-swap len

      int length = ntohl(len);
      if (length < 0) {
         length = -length;
         cerr << "WARNING - " << _progName << endl;
         cerr << "  Received negative value for length" << endl;
      }
      if (length > 1.0e9) {
         cerr << "ERROR - " << _progName << endl;
         cerr << "  Length too large: " << length << endl;
         return -1;
      }
      if (_params.debug) {
         cerr << "read length: " << length << endl;
      }
      
      // read in buffer

      unsigned char *inbuf = new unsigned char[length];
      if ((int) fread(inbuf, 1, length, in) != length) {
         if (feof(in)) {
            break;
         }
         cerr << "ERROR - " << _progName << endl;
         cerr << "  Cannot read inbuf, nbytes: " << length << endl;
         delete[] inbuf;
         return -1;
      }

      // uncompress

      int error;
      unsigned int olength = _params.max_output_file_size;
      
#ifdef BZ_CONFIG_ERROR
      error = BZ2_bzBuffToBuffDecompress((char *) _outBuf, &olength,
                                         (char *) inbuf, length, 0, 0);
#else
      error = bzBuffToBuffDecompress((char *) _outBuf, &olength,
                                     (char *) inbuf, length, 0, 0);
#endif

      delete[] inbuf;

      if (error) {
         cerr << "ERROR - " << _progName << endl;
         cerr << "  BZIP2 uncompress did not work" << endl;
         if (error == BZ_OUTBUFF_FULL) {
            cerr << "  Probably max output file size exceeded: "
                 << _params.max_output_file_size << endl;
         }
         return -1;
      }
      
      // write out buf
      
      if (fwrite(_outBuf, 1, olength, out) != olength) {
         cerr << "ERROR - " << _progName << endl;
         cerr << "  Cannot write output buffer, len: " << olength << endl;
         return -1;
      }
      
   }
   
   return 0;

}
