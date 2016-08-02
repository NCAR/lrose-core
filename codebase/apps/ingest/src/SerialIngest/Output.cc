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
/////////////////////////////////////////////////////////////
// Output.cc
//
// Output file class
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 1999
//
/////////////////////////////////////////////////////////////

#include "Output.hh"
#include <cerrno>
#include <sys/stat.h>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <dsserver/DsLdataInfo.hh>
using namespace std;

Output::Output (const string &prog_name, const Params &params) :
  _progName(prog_name), _params(params)

{

  _isOK = true;
  _fp = NULL;
}

Output::~Output ()

{
  this->close();
}

int Output::open ()

{

  this->close();

  _tmpDirPath = _params.output_dir_path;
  _tmpDirPath += PATH_DELIM;
  _tmpDirPath += "tmp";

  if (ta_makedir_recurse(_tmpDirPath.c_str())) {
    cerr << "ERROR - " << _progName << ": Output::open()" << endl;
    cerr << "  Cannot make tmp dir: '" << _tmpDirPath << "'" << endl;
    return (-1);
  }

  _tmpFilePath = _tmpDirPath + PATH_DELIM;
  _tmpFilePath += "SerialIngest.tmp";

  if ((_fp = fopen(_tmpFilePath.c_str(), "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - " << _progName << ": Output::open()" << endl;
    cerr << "  Cannot open tmp file: '" << _tmpDirPath << "'" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return (-1);
  }

  if (_params.debug) {
    cerr << "Opened file: '" << _tmpFilePath << "'" << endl;
  }

  return (0);

}

void Output::close ()

{

  if (_fp) {
    fclose(_fp);
    _fp = NULL;
  }

}

int Output::save (time_t data_time)

{

  // check for 0-length files

  if (_params.discard_zero_length_files) {
    struct stat fileStat;
    if (stat(_tmpFilePath.c_str(), &fileStat)) {
      cerr << "ERROR - " << _progName << ": Output::save()" << endl;
      cerr << "  Cannot stat tmp file: '" << _tmpFilePath << "'" << endl;
      return (-1);
    }
    if (fileStat.st_size == 0) {
      return (0);
    }
  }

  date_time_t dtime;
  dtime.unix_time = data_time;
  uconvert_from_utime(&dtime);
  
  // ensure directory exists

  char dateDirPath[MAX_PATH_LEN];
  sprintf(dateDirPath, "%s%s%.4d%.2d%.2d",
	  _params.output_dir_path, PATH_DELIM,
	  dtime.year, dtime.month, dtime.day);

  if (ta_makedir_recurse(dateDirPath)) {
    cerr << "ERROR - " << _progName << ": Output::save()" << endl;
    cerr << "  Cannot make date dir: '" << dateDirPath << "'" << endl;
    return (-1);
  }

  // compute file path

  char relDataPath[MAX_PATH_LEN];
  sprintf(relDataPath, "%.4d%.2d%.2d%s%.2d%.2d%.2d.%s",
	  dtime.year, dtime.month, dtime.day,
	  PATH_DELIM,
	  dtime.hour, dtime.min, dtime.sec,
	  _params.output_file_ext);
  
  char outPath[MAX_PATH_LEN];
  sprintf(outPath, "%s%s%s",
	  _params.output_dir_path, PATH_DELIM, relDataPath);
  
  if (rename(_tmpFilePath.c_str(), outPath)) {
    int errNum = errno;
    cerr << "ERROR - " << _progName << ": Output::save()" << endl;
    cerr << "  Cannot rename tmp file: '" << _tmpFilePath << "'" << endl;
    cerr << "                to  file: '" << outPath << "'" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return (-1);
  }

  DsLdataInfo ldata(_params.output_dir_path, _params.debug);
  ldata.setDataFileExt(_params.output_file_ext);
  ldata.setRelDataPath(relDataPath);
  ldata.setWriter(_progName.c_str());
  if (ldata.write(data_time)) {
    cerr << "ERROR - " << _progName << ": Output::save()" << endl;
    cerr << "  Cannot write latest_data_info file";
    return (-1);
  }

  if (_params.debug) {
    cerr << "Saved file: '" << _tmpFilePath << "'" << endl;
    cerr << "   to file: '" << outPath << "'" << endl;
  }

  return (0);
    
}

/////////////////////////////		    
// putChar - put a character

int Output::putChar(char cc)

{

  if (fputc(cc, _fp) == EOF) {
    int errNum = errno;
    cerr << "ERROR - " << _progName << ": Output::putChar()" << endl;
    cerr << "  Cannot put char to file: '" << _tmpFilePath << "'" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return (-1);
  }
  
  return (0);

}

////////////////////////////////////
// putBuf - put contents of a membuf

int Output::putBuf(MemBuf &buf)

{

  if (ufwrite(buf.getBufPtr(), buf.getLen(), 1, _fp) != 1) {
    int errNum = errno;
    cerr << "ERROR - " << _progName << ": Output::putChar()" << endl;
    cerr << "  Cannot put membuf to file: '" << _tmpFilePath << "'" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return (-1);
  }
  
  return (0);

}


