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
// SplitV3.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2002
//
///////////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include <mm5/MM5DataV3.hh>
#include <dsserver/DsLdataInfo.hh>
#include <cstdio>
#include <cerrno>
#include <sys/stat.h>

#include "MM5Split.hh"
#include "SplitV3.hh"
using namespace std;

// Constructor

SplitV3::SplitV3(const string &prog_name,
		 const Params &params,
		 const string &input_path) :
  _progName(prog_name),
  _params(params),
  _inputPath(input_path)

{

}

// destructor

SplitV3::~SplitV3()

{
  
}

//////////////////////////////////////////////////
// doSplit

int SplitV3::doSplit()

{

  PMU_auto_register("SplitV3::doSplit");
  
  MM5DataV3 inData(_progName, _inputPath, _params.debug);
  
  if (!inData.OK) {
    cerr << "ERROR - SplitV3::doSplit" << endl;
    cerr << "  Cannot construct MM5DataV3 object for file: "
	 << _inputPath << endl;
    return -1;
  }

  if (!inData.more()) {
    cerr << "Cannot read file: " << _inputPath << endl;
    return -1;
  }
  
  while(inData.more()) {

    PMU_auto_register("SplitV3::doSplit");

    if (inData.read()) {
      cerr << "ERROR - SplitV3::doSplit" << endl;
      cerr << "  Cannot read MM5DataV3 object from file: "
	   << _inputPath << endl;
      return -1;
    }
    
    // compute the output name
    
    int leadTimeHr = inData.forecastLeadTime / 3600;
    int leadTimeMin = (inData.forecastLeadTime % 3600) / 60;
    
    Path ipath(_inputPath);
    char outputName[MAX_PATH_LEN];
    char outputPath[MAX_PATH_LEN];
    sprintf(outputName, "%s.tm%.2d%.2d.mm5",
	    ipath.getFile().c_str(), leadTimeHr, leadTimeMin);
    sprintf(outputPath, "%s%s%s",
	    _params.output_dir, PATH_DELIM, outputName);
    ta_makedir_recurse(_params.output_dir);
    
    // open input and output files
    
    FILE *in;
    if ((in = fopen(_inputPath.c_str(), "rb")) == NULL) {
      int errNum = errno;
      cerr << "ERROR - MM5Split::SplitV3::doSplit" << endl;
      cerr << " Cannot open input file: " << _inputPath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
    
    FILE *out;
    if ((out = fopen(outputPath, "wb")) == NULL) {
      int errNum = errno;
      cerr << "ERROR - MM5Split::SplitV3::doSplit" << endl;
      cerr << " Cannot open output file: " << outputPath << endl;
      cerr << "  " << strerror(errNum) << endl;
      fclose(in);
      return -1;
    }
    
    // copy over the headers

    if (MM5Split::copyBuffer(in, out,
			     inData.getHeadersOffset(),
			     inData.getHeadersLen())) {
      fclose(in);
      fclose(out);
      return -1;
    }
    
    // copy over the data set
    
    if (MM5Split::copyBuffer(in, out,
			     inData.getDataSetOffset(),
			     inData.getDataSetLen())) {
      fclose(in);
      fclose(out);
      return -1;
    }

    // close files

    fclose(in);
    fclose(out);

    if (_params.debug) {
      cerr << "Wrote file: " << outputPath << endl;
    }
    
    // write latest data info file
    
    DsLdataInfo ldata;
    ldata.setDir(_params.output_dir);
    ldata.setIsFcast(true);
    ldata.setLeadTime(inData.forecastLeadTime);
    ldata.setDataFileExt("mm5");
    ldata.setWriter(_progName.c_str());
    ldata.setRelDataPath(outputName);
    if (ldata.write(inData.modelTime, "raw")) {
      cerr << "ERROR - MM5Split::SplitV3::doSplit" << endl;
      cerr << "  Cannot write _latest_data_info file" << endl;
      cerr << "  Dir: " << _params.output_dir << endl;
    }
  
  } // while

  return 0;
  
}
