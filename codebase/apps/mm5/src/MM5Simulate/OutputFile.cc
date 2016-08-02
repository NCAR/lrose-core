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
// OutputFile.cc
//
// Handles output of MM5 simulated data.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Octiber 1998
//
///////////////////////////////////////////////////////////////

#include "OutputFile.hh"

#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaFile.hh>
#include <didss/LdataInfo.hh>
#include <cstdio>
using namespace std;

//////////////
// Constructor

OutputFile::OutputFile(const string &prog_name, const Params &params) :
  _progName(prog_name), _params(params)
  
{
}

/////////////
// destructor

OutputFile::~OutputFile()

{
}

////////////////////////
// write out the dataset

int OutputFile::writeDataset(const string &input_path,
			     void *dataset_buf,
			     int dataset_len,
			     time_t model_time,
			     int forecast_lead_time)

{

  if (_params.debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "Writing dataset at time %s\n",
	    utimstr(model_time));
  }
  
  // compute the output path, using the input path as a template and 
  // applying the model_time and forecast_lead_time

  Path inputPath(input_path);
  string inputFileName = inputPath.getFile();

  size_t firstDot = inputFileName.find_first_of('.', 0);
  if (firstDot == string::npos) {
    fprintf(stderr, "ERROR - %s:OutputFile::writeDataset\n", _progName.c_str());
    fprintf(stderr, "  Input file name invalid: %s\n", inputFileName.c_str());
    fprintf(stderr, "  Input path: %s\n", input_path.c_str());
    return (-1);
  }
  string prefix = inputFileName.substr(0, firstDot);

  DateTime dtime(model_time);
  int year, month, day, hour;
  dtime.getAll(&year, &month, &day, &hour);
  int leadHour = forecast_lead_time / 3600;
  int leadMin = (forecast_lead_time % 3600) / 60;
  
  char outName[MAX_PATH_LEN];
  sprintf(outName, "%s.%.4d%.2d%.2d%.2d.tm%.2d%.2d.%s",
	  prefix.c_str(), year, month, day, hour, leadHour, leadMin,
	  inputPath.getExt().c_str());

  Path outputPath(inputPath);
  outputPath.setDirectory(_params.output_dir);
  outputPath.setFile(outName);
  
  // open the output file

  if (ta_makedir_recurse(_params.output_dir)) {
    fprintf(stderr, "ERROR - %s:OutputFile::writeDataset\n", _progName.c_str());
    fprintf(stderr, "  Cannot create output dir: %s\n", _params.output_dir);
    perror(_params.output_dir);
    return (-1);
  }

  TaFile out;
  if (out.fopen(outputPath.getPath().c_str(), "wb") == NULL) {
    fprintf(stderr, "ERROR - %s:OutputFile::writeDataset\n", _progName.c_str());
    fprintf(stderr, "  Cannot open output file: %s\n", outputPath.getPath().c_str());
    perror(outputPath.getPath().c_str());
    return (-1);
  }
  
  if (out.fwrite(dataset_buf, 1, dataset_len) != dataset_len) {
    fprintf(stderr, "ERROR - %s:OutputFile::writeDataset\n", _progName.c_str());
    fprintf(stderr, "Cannot write dataset at time %s\n",
	    utimstr(model_time));
    perror(outputPath.getPath().c_str());
    return (-1);
  }
  out.fclose();

  // write the ldata info file

  LdataInfo ldata;
  ldata.setDir(_params.output_dir);
  ldata.setDataFileExt(outputPath.getExt().c_str());
  ldata.setWriter(_progName.c_str());
  ldata.setRelDataPath(outputPath.getFile().c_str());
  ldata.setIsFcast(true);
  ldata.setLeadTime(forecast_lead_time);
  ldata.setUseFmq(false);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    ldata.setDebug(true);
  }

  if (ldata.write(model_time)) {
    fprintf(stderr, "ERROR - %s:OutputFile::writeDataset\n",
	    _progName.c_str());
    fprintf(stderr, "Cannot write latest data info file in dir %s\n",
	    _params.output_dir);
    return (-1);
  }

  return (0);

}










