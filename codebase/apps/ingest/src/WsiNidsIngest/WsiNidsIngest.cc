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
// WsiNidsIngest.cc
//
// WsiNidsIngest object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <rapformats/nids_file.h>
#include <didss/LdataInfo.hh>
#include "WsiNidsIngest.hh"
using namespace std;

// Constructor

WsiNidsIngest::WsiNidsIngest(int argc, char **argv) 

{

  isOK = true;
  _input = NULL;

  // set programe name

  _progName = "WsiNidsIngest";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // set up input object

  if (_params.read_stdin) {
    _input = new StdInput(_params.input_buffer_size);
  } else {
    _input = new SockInput(_params.wsi_host, _params.wsi_port,
			   _params.input_buffer_size);
  }

  // load up product map

  for (int i = 0; i < _params.output_products_n; i++) {

    // check the radar name has 3 chars

    if (strlen(_params._output_products[i].radar_name) != 3) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Invalid radar name: '"
	   << _params._output_products[i].radar_name << "'" << endl;
      cerr << "All radar sites must have 3 character name.";
      isOK = FALSE;
      return;
    }

    // Copy radar name, force to lower case.
    // Incoming data has radars in lower case.
    // Output file name is in upper case.

    char name[4];

    strcpy(name, _params._output_products[i].radar_name);
    name[0] = tolower(name[0]);
    name[1] = tolower(name[1]);
    name[2] = tolower(name[2]);

    radar_product_t prod;
    prod.radar_name = name;
    prod.message_code = _params._output_products[i].message_code;
    prod.prod_name = _params._output_products[i].prod_name;

    ui32 u1 = name[0];
    ui32 u2 = name[1];
    ui32 u3 = name[2];
    ui32 u4 = prod.message_code;
    ui32 prodKey = (u1 << 24) + (u2 << 16) + (u3 << 8) + u4;

    _prodMap[prodKey] = prod;
    
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

WsiNidsIngest::~WsiNidsIngest()

{

  // unregister process

  PMU_auto_unregister();

  // free up

  if (_input) {
    delete _input;
  }

}

//////////////////////////////////////////////////
// Run

int WsiNidsIngest::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  ui08 cc;
  ui32 cc1 = 0, cc2 = 0, cc3 = 0, cc4 = 0, cc5 = 0, cc6 = 0, cc7 = 0;

  while (_input->read(&cc, 1) == 0) {

    // we are looking for 3 consecutive bytes, followed by a single byte
    // 4 bytes later. So we cascade the bytes.

    cc1 = cc2; cc2 = cc3; cc3 = cc4; cc4 = cc5;
    cc5 = cc6; cc6 = cc7; cc7 = cc;

#ifdef NOT_NOW
    // look for sync pattern
    if (cc4 == 0x00 && cc5 == 0xff && cc6 == 0x00 && cc7 == 0xff) {
      fprintf(stderr, "---> Sync pattern found\n");
      fprintf(stderr, "---> bytes read: %d\n", _input->getTotalRead());
    }
#endif

    // compute the product key

    ui32 prodKey = (cc1 << 24) + (cc2 << 16) + (cc3 << 8) + cc7;

    // check if key is in map - if so we are at the
    // start of a volume

    product_map_t::iterator ii = _prodMap.find(prodKey);

    if (ii != _prodMap.end()) {
      
      if (_params.debug) {
	fprintf(stderr, "----------> %c%c%c %d %d %d %d\n",
		cc1, cc2, cc3, cc4, cc5, cc6, cc7);
	fprintf(stderr, "---> bytes read: %d\n", _input->getTotalRead());
      }

      // read in whole message header, starting 2 bytes into
      // the header because we have already read the message code
      
      NIDS_header_t mhead;
      if (_input->read((char *) &mhead + 2, sizeof(NIDS_header_t) - 2)) {
	return (-1);
      }
      NIDS_BE_to_mess_header(&mhead);
      mhead.mcode = (*ii).second.message_code;

      if (_params.debug >= Params::DEBUG_VERBOSE) {
	fprintf(stderr, "\n=============================\n");
	NIDS_print_mess_hdr(stderr, "  ", &mhead);
      }
      
      // read in graphic

      size_t graphicLen = mhead.lendat;
      ui08 *graphic = new ui08[graphicLen];
      if (_input->read(graphic, mhead.lendat)) {
	return (-1);
      }

      // compute directory
      
      char num[4];
      sprintf(num, "%d", mhead.elevnum);
      string prodX = (*ii).second.prod_name;
      prodX += num;

      string dirPath = _params.output_dir_path;
      dirPath += PATH_DELIM;
      dirPath += (*ii).second.radar_name;
      dirPath += PATH_DELIM;
      dirPath += prodX;

      // compute date subdirectory path

      date_time_t scantime;
      scantime.unix_time =
	(mhead.vsdate - 1) * 86400 + mhead.vstime * 65536 + mhead.vstim2;
      uconvert_from_utime(&scantime);

      char dateStr[32];
      sprintf(dateStr, "%.4d%.2d%.2d",
	      scantime.year, scantime.month, scantime.day);

      string subDirPath = dirPath;
      subDirPath += PATH_DELIM;
      subDirPath += dateStr;

      // make sure it exists

      if (ta_makedir_recurse(subDirPath.c_str())) {
	cerr << "ERROR - " << _progName << endl;
	cerr << "Cannot make dir " << dirPath << endl;
	delete[] graphic;
	continue;
      }

      // compute file path

      char timeStr[32];
      sprintf(timeStr, "%.4d%.2d%.2d.%.2d%.2d%.2d",
	      scantime.year, scantime.month, scantime.day,
	      scantime.hour, scantime.min, scantime.sec);

      string fileName = timeStr;
      fileName += ".nids";

      string filePath = subDirPath;
      filePath += PATH_DELIM;
      filePath += fileName;
      
      if (_params.debug) {
	fprintf(stderr, "Writing file: %s\n", filePath.c_str());
      }

      FILE *out;

      if ((out = fopen(filePath.c_str(), "w")) == NULL) {
	int errNum = errno;
	cerr << "ERROR - " << _progName << endl;
	cerr << "Cannot open file " << filePath << " for writing." << endl;
	cerr << strerror(errNum);
	delete[] graphic;
	continue;
      }

      // swap header back and write out
      NIDS_BE_from_mess_header(&mhead);
      if (fwrite(&mhead, sizeof(NIDS_header_t), 1, out) != 1) {
	int errNum = errno;
	cerr << "ERROR - " << _progName << endl;
	cerr << "Cannot write header to file " << filePath << endl;
	cerr << strerror(errNum);
	delete[] graphic;
	continue;
      }

      // write out graphic
      NIDS_BE_from_mess_header(&mhead);
      if (fwrite(graphic, 1, graphicLen, out) != graphicLen) {
	int errNum = errno;
	cerr << "ERROR - " << _progName << endl;
	cerr << "Cannot write graphic to file " << filePath << endl;
	cerr << strerror(errNum);
	delete[] graphic;
	continue;
      }

      fclose(out);
      delete[] graphic;

      // write out the index file

      LdataInfo ldata(dirPath);

      string userInfo1 = dateStr;
      userInfo1 += PATH_DELIM;
      userInfo1 += fileName;

      string relPath = dateStr;
      relPath += PATH_DELIM;
      relPath += fileName;
      relPath += ".nids";
      
      ldata.setDataFileExt("nids");
      ldata.setRelDataPath(relPath.c_str());
      ldata.setWriter("WsiNidsIngest");

      ldata.setUserInfo1(userInfo1.c_str());

      ldata.write(scantime.unix_time);

    } // if (ii != _prodMap.end()) {
    
  } // while
  
  return (0);

}

