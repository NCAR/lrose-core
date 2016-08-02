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
////////////////////////////////////////////////////////////////////
// Linebuff
//
// unpacked radial array class
//
// RAPIC code (c) BOM Australia
//
// Phil Purdam
//
// Jan 2000
//
/////////////////////////////////////////////////////////////////////

#include "Linebuff.hh"
#include <cerrno>
#include <cstring>
#include <iostream>
using namespace std;

///////////////
// constructor

Linebuff::Linebuff(int buffsz)

{
  in = NULL;
  line_buff = NULL;
  lb_max = buffsz;
  line_buff = new char[buffsz];
  reset();
  new_scan_ofs = 0;
  _repeat = false;
}

/////////////
// destructor

Linebuff::~Linebuff()
{

  if (in) {
    fclose (in);
  }

  if (line_buff) {
    delete[] line_buff;
  }

}

///////////////////
// reset the buffer

void Linebuff::reset()
{
  lb_size = 0;
  isBinRadl = isRadl = isComment = EOL =
    termCh1 = termCh2 = Overflow = false;   
  lb_pnt = line_buff;
}

///////////////////////////
// ensure null termination

void Linebuff::ensureTerminated()
{
  if ((line_buff[lb_size-1] != 0) &&	// if ln not null terminated
      (lb_size < (lb_max-1))) // check there is space for another char
    {
      line_buff[lb_size] = 0;	// null terminate
      lb_size++;
    }
}

///////////////////////
// open the input file

int Linebuff::openFile(const char *file_path)
{

  if (in) {
    fclose (in);
    in = NULL;
  }

  if ((in = fopen(file_path, "rb")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Linebuff::openFile" << endl;
    cerr << "  Cannot open file: " << file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  return 0;

}

///////////////////////
// close the input file

void Linebuff::closeFile()
{
  if (in) {
    fclose (in);
    in = NULL;
  }
}

/////////////////////
// read the next line

int Linebuff::readNext()

{

  if (_repeat) {
    _repeat = false;
    return 0;
  }

  char cc;

  while (!feof(in)) {
    
    if (fread(&cc, 1, 1, in) != 1) {
      return -1;
    }
    
    addchar_parsed(cc);

    if (IsEOL()) {
      addchar_parsed('\0');
      return 0;
    }

  } // while

  return -1;

}
  
bool  Linebuff::endOfFile() {

  if (!in) {
    cerr << "ERROR - in is NULL" << endl;
    return true;
  }

  return feof(in);

}
