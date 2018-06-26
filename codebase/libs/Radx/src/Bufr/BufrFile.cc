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
// BufrFile.cc
//
// BUFR file wrapper
//
// Mike Dixon and Brenda Javornik, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2017
//
///////////////////////////////////////////////////////////////

#include <Radx/BufrFile.hh>
#include <string>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <errno.h>
#include <cmath>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
#include <Radx/ByteOrder.hh>
#include <Radx/TableMapKey.hh>
#include <Radx/RadxStr.hh>
#include "BufrProductGeneric.hh"
#include "BufrProduct_204_31_X.hh"
#include "BufrProduct_gsi.hh"

using namespace std;

DNode::DNode() {
}

DNode::~DNode() {
}


//////////////
// Constructor

BufrFile::BufrFile()
  
{
  currentTemplate = NULL;
  _debug = false;
  _verbose = false;
  _very_verbose = false;
  _file = NULL;
  GTree = NULL;
  _tablePath = NULL;
  clear();
}


  
/////////////
// destructor

BufrFile::~BufrFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void BufrFile::clear()
  
{
  //clearErrStr();
  close();
  _pathInUse.clear();
  _firstBufferReplenish = true;
  _errString.clear();
  _file = NULL;
  freeTree(GTree);
  GTree = NULL;
  _descriptorsToProcess.clear();
  _numBytesRead = 0;
  _addBitsToDataWidth = 0;
  _addBitsToDataScale = 0;
  _multiplyFactorForReferenceValue = 1;
  if (currentTemplate != NULL) 
    delete currentTemplate;
  currentTemplate = NULL;

}

// clear and reset things for the next BUFR message
// in the same file.
void BufrFile::clearForNextMessage()
{
  _errString.clear();
  freeTree(GTree);
  GTree = NULL;
  _descriptorsToProcess.clear();
  _addBitsToDataWidth = 0;
  _addBitsToDataScale = 0;
  _multiplyFactorForReferenceValue = 1;
  if (currentTemplate != NULL) 
    delete currentTemplate;
  currentTemplate = NULL;
  _nBitsRead = 0;
 // set this value to be able to read initial BUFR + size (3 bytes) + ending (7777)
  _s0.nBytes = 12;
}

void BufrFile::setDebug(bool state) { 
  _debug = state; 
  //currentTemplate->setDebug(state);
}

void BufrFile::setVerbose(bool state) {
  _verbose = state;
  if (_verbose) _debug = true;
  //currentTemplate->setVerbose(state);
  // we only want debug information from the tables if the setting is verbose,
  // because the information is very detailed.
  tableMap.setDebug(state);
}

void BufrFile::setVeryVerbose(bool state) {
  _very_verbose = state;
  if (_very_verbose) {
    _debug = true;
    _verbose = true;
  }
  //currentTemplate->setVerbose(state);
  // we only want debug information from the tables if the setting is verbose,
  // because the information is very detailed.
  tableMap.setDebug(state);
}

void BufrFile::setTablePath(char *path) {
  _tablePath = path;  
}

// go ahead and read all the data from the file
// completely fill currentTemplate with data
void BufrFile::readThatField(string fileName,
             string filePath,
             time_t fileTime,
             string fieldName,
             string standardName,
             string longName,
             string units) {

  clear();
  _pathInUse = filePath;
  _fieldName = fieldName;

  try {
    openRead(_pathInUse); // path);
    readSection0();
    readSection1();
    readDataDescriptors();
    // TODO: determine which type of product to instaniate based
    // on the data descriptors of section 3
    //currentTemplate = new BufrProductGeneric();  // TODO: set this to 204_13_203
    //currentTemplate->setVerbose(_verbose);
    //currentTemplate->setDebug(_debug);
    //currentTemplate->reset(); 
    readData(); 
    readSection5();
    close();
  } catch (const char *msg) {
    throw _errString.c_str();
  }
}

/*
//////////////////////////////////////////////
/// open netcdf file for writing
/// create error object so we can handle errors
/// set the netcdf format, before a write
/// format options are:
///   Classic - classic format (i.e. version 1 format)
///   Offset64Bits - 64-bit offset format
///   Netcdf4 - netCDF-4 using HDF5 format
///   Netcdf4Classic - netCDF-4 using HDF5 but only netCDF-3 calls
/// Returns 0 on success, -1 on failure

int BufrFile::openWrite(const string &path,
                        Nc3File::FileFormat format) 

{
  
  close();
  _pathInUse = path;
  _ncFormat = format;
  _ncFile = new Nc3File(path.c_str(), Nc3File::Replace, NULL, 0, _ncFormat);
  
  if (!_ncFile || !_ncFile->is_valid()) {
    _addErrStr("ERROR - BufrFile::openWrite");
    _addErrStr("  Cannot open netCDF file for writing: ", path);
    close();
    return -1;
  }
  
  // Change the error behavior of the netCDF C++ API by creating an
  // Nc3Error object. Until it is destroyed, this Nc3Error object will
  // ensure that the netCDF C++ API silently returns error codes
  // on any failure, and leaves any other error handling to the
  // calling program.
  
  if (_err == NULL) {
    _err = new Nc3Error(Nc3Error::silent_nonfatal);
  }

  return 0;

}
*/


bool BufrFile::eof() {
  return feof(_file);
}



//////////////////////////////////////
// close netcdf file if open
// remove error object if it exists

void BufrFile::close()
  
{
  
  // close file if open, delete ncFile
  
  if (_file) {
    fclose(_file);
    //delete _ncFile;
    _file = NULL;
  }
  /*
  if (_err) {
    delete _err;
    _err = NULL;
  }
  */
}

//////////////////////////////////////
// open netcdf file for reading
// Returns 0 on success, -1 on failure

int BufrFile::openRead(const string &path)
  
{

  close();
  _bufrMessageCount = 0;

  _file = fopen(path.c_str(), "r");
  
  if (_file == NULL) {
    int errNum = errno;
    Radx::addErrStr(_errString, "", "ERROR - BufrFile::_openRead", true);
    Radx::addErrStr(_errString, "  Cannot open file for reading, path: ", path, true);
    Radx::addErrStr(_errString, "  ", strerror(errNum), true);
    throw _errString.c_str();
  }

  // prime the pump
  currentBufferLengthBytes = ReplenishBuffer();
  currentBufferLengthBits = currentBufferLengthBytes * 8;
  currentBufferIndexBits = 0;
  if (currentBufferLengthBits <= 0) {
    return -1;
  }

  //currentTemplate->reset(); 
  return 0;
}

///////////////////////////////////////////
// read Section 0
// which positively identifies this as a BUFR file
// Returns 0 on success, -1 on failure
// Side effect: 

int BufrFile::readSection0()
{
 
  clearForNextMessage();
  inSection5 = false;
  /******* decode section 0 */
  if (_verbose) fprintf (stderr, "Input file header:\n");
  //char temp[250];
  // read through file, finding data blocks

  // read ID
    
  //  char id[8];
  //memset(id, 0, 8);

  try {
    /*
    if (fread(id, 1, 4, _file) != 4) {
      int errNum = errno;
      sprintf(temp, "  Cannot read BUFR starting code: %s", strerror(errNum));
      throw temp;
    }
    */
    string value;
    do {
      value = ExtractText(8);
    } while (value.find("B") == string::npos);
    _nBitsRead = 8; // reset this count after finding "B"
    value = ExtractText(8*3);
    if (value.find("UFR") == string::npos)
      throw "Not a BUFR file"; // "  Cannot read BUFR starting code";
    //_numBytesRead += 4;
    /*
    string idStr(id);
    if ((id[0] != 'B') || (id[1] != 'U') || (id[2] != 'F') || (id[3] != 'R')) {
      throw "Not a BUFR file";
    }
    */

    // read length of message in octets
    // the length is 24 bits (3 bytes)
    Radx::ui32 nBytes;
    nBytes = ExtractIt(24);
    
    /*
    unsigned char id2[4];
    if (fread(id2, 1, 3, _file) != 3) {
      int errNum = errno;
      sprintf(temp, "  Cannot read length of section in octets: %s", strerror(errNum));
      throw temp;
    }

    _numBytesRead += 3;

    nBytes = 0;
    nBytes = nBytes | id2[2];
    nBytes = nBytes | (id2[1] << 8);
    nBytes = nBytes | (id2[0] << 16);
    */
    if (_debug) cerr << "nBytes in total message " << nBytes << endl;
    _s0.nBytes = nBytes;

    Radx::ui08 bufr_edition;
    bufr_edition = ExtractIt(8);
    /*
    if (fread(&bufr_edition, 1, 1, _file) != 1) {
      int errNum = errno;
      sprintf(temp, "  Cannot read BUFR edition: %s", strerror(errNum));
      throw temp;
      }*/

    _s0.edition = bufr_edition;   
    if (_verbose) printf("BUFR edition number %d\n", bufr_edition); 

    _bufrMessageCount += 1;
    if (_debug) {
      printf("Processing BUFR message %d at nBytes %d\n", _bufrMessageCount,
             _getCurrentBytePositionInFile());
    }
  } catch (const char *msg) {
    close();
    Radx::addErrStr(_errString, "ERROR - ", "BufrFile::_readSection0()", true);
    //    Radx::addErrStr(_errString, "  File path: ", _pathInUse, true);
    Radx::addErrStr(_errString, "", msg, true);
    throw _errString.c_str();
  }
  return 0;
}

void BufrFile::printSection0(ostream &out) {
  out << "  BUFR edition number: " << (int) _s0.edition << endl;
  out << "  number of bytes    : " << _s0.nBytes << endl; 
}

///////////////////////////////////////////
// read Section 1
// which gives time and table info
// Returns 0 on success, -1 on failure
// Side effect: 

int BufrFile::readSection1() {
  if (_s0.edition >= 4) 
    readSection1_edition4();
  else if (_s0.edition == 2) 
    readSection1_edition2();
  else if (_s0.edition == 3)
    readSection1_edition3();
  else {
    close();
    Radx::addErrStr(_errString, "ERROR - ", "BufrFile::_readSection1()", true);
    Radx::addErrInt(_errString, "  unrecognized BUFR edition: ", _s0.edition, true);
    throw _errString.c_str();
  }
  return 0;
}

int BufrFile::readSection1_edition4()
{

  /******* decode section 1 */
  if (_verbose) fprintf (stderr, "Input file header:\n");

  try {
  // read ID
  // allocate this space based on the length of section read (octets 1 - 3)
  //Radx::ui08 id[4];
  //memset(id, 0, 4);
    
  // read length of message in octets
  // the length is 24 bits (3 bytes)
  Radx::ui32 nBytes;
  nBytes = ExtractIt(24);

  /*
  if (fread(id, 1, 3, _file) != 3) {
    int errNum = errno;
    Radx::addErrStr(_errString, "ERROR - ", "BufrFile::_readSection1()", true);
    Radx::addErrStr(_errString, "  Cannot read length of message in octets", strerror(errNum), true);
    Radx::addErrStr(_errString, "  File path: ", _pathInUse, true);
    close();
    return -1;
  }
  nBytes = 0;
  nBytes = nBytes | id[2];
  nBytes = nBytes | (id[1] << 8);
  nBytes = nBytes | (id[0] << 16);
  */
  Radx::ui32 sectionLen;
  sectionLen = nBytes;

  if (_verbose) cerr << "sectionLen " << sectionLen << endl;
  /*
  Radx::ui08 *buffer;
  buffer = (Radx::ui08 *) calloc(sectionLen, sizeof(Radx::ui08));
  memset(buffer, 0, sectionLen);

  if (fread(buffer, 1, sectionLen-3, _file) != sectionLen-3) {
    int errNum = errno;
    Radx::addErrStr(_errString, "ERROR - ", "BufrFile::_readSection1()", true);
    Radx::addErrStr(_errString, "  Cannot read data", strerror(errNum), true);
    Radx::addErrStr(_errString, "  File path: ", _pathInUse, true);
    free(buffer);
    close();
    return -1;
  }
  */

  //Radx::ui08 bufrMasterTable;
  //bufrMasterTable = buffer[0];  // not sure if this is needed??
  // check 1st bit of the octect for optional section 2
  //bool section2 = buffer[6]; //  & 0x80; 
  //_s1.hasSection2 = section2;

  Radx::ui16 yearOfCentury;
  Radx::ui08 month;
  Radx::ui08 day;
  Radx::ui08 hour;
  Radx::ui08 minute;
  Radx::ui08 seconds;

  if (_s0.edition != 4)
    throw "ERROR - Wrong version of section1 called ";

    _s1.masterTable = ExtractIt(8); //  octet 4
    _s1.generatingCenter =  ExtractIt(16); // octet 5-6  originating/generating center
    _s1.originatingSubcenter =  ExtractIt(16); // octet 7-8 originating sub-center
    _s1.updateSequenceNumber =  ExtractIt(8); // octet 9
    _s1.hasSection2 = ExtractIt(1); ExtractIt(7); // octet 10 optional sections 2 flag
    _s1.dataCategoryType = ExtractIt(8); //  octet 11 data category (Table A)
    ExtractIt(8); //  octet 12 international data sub-category
    ExtractIt(8); //  octet 13 local data sub-category
    _s1.masterTableVersionNumber =  ExtractIt(8); // octet 14
    _s1.localTableVersionNumber =  ExtractIt(8); // octet 15

    if (_verbose) {
      cerr << "section 2? " ;
      if (_s1.hasSection2) {
        cerr << "yes" << endl;
      } else {
        cerr << "no" << endl;
      }
 
      printf("master table: %d\n", _s1.masterTable);
      printf("generating center: %d\n", _s1.generatingCenter);
      printf("originating subcenter: %d\n", _s1.originatingSubcenter);
      printf("update sequence number: %d\n", _s1.updateSequenceNumber);
      printf("data category type: %d\n", _s1.dataCategoryType);
      printf("local table version: %d\n", _s1.localTableVersionNumber);
      printf("master table version: %d\n", _s1.masterTableVersionNumber);
    }
    //    Radx::ui16 junk = ExtractIt(16); // don't know why, but, just need to toss 16 bits. 
    //cout << "junk = " << junk << endl;
    yearOfCentury =  ExtractIt(16); // octet 16-17
    month =  ExtractIt(8); // octet 18-22 month, day, hour, minute, second
    day =  ExtractIt(8); // 
    hour =  ExtractIt(8); // 
    minute =  ExtractIt(8); // 
    seconds =  ExtractIt(8); // 
    _s1.year = yearOfCentury;
    _s1.month = month;
    _s1.day = day;
    _s1.hour = hour;
    _s1.minute = minute;
    _s1.seconds = seconds;
     

  hdr_year = yearOfCentury;
  hdr_month = month;
  hdr_day = day;
  if (_verbose) printf("year-month-day hour:minute:sec\n%d-%d-%d %d:%d:%d\n",
		       yearOfCentury, month, day, hour, minute, seconds); 
    
  //_numBytesRead += sectionLen;
  //free(buffer);
  return 0;
  } catch (char *msg) {
    int errNum = errno;
    Radx::addErrStr(_errString, "ERROR - ", "BufrFile::_readSection1()", true);
    Radx::addErrStr(_errString, "  Cannot read data", strerror(errNum), true);
    Radx::addErrStr(_errString, "  File path: ", _pathInUse, true);
    // free(buffer);
    // close();
    throw _errString;
  }
}

int BufrFile::readSection1_edition2()
{

  /******* decode section 1 */
  if (_verbose) fprintf (stderr, "Input file header:\n");

  // read ID
  // allocate this space based on the length of section read (octets 1 - 3)
  //Radx::ui08 id[4];
  // memset(id, 0, 4);
    
  // read length of message in octets
  // the length is 24 bits (3 bytes)
/*
  Radx::ui32 nBytes;
  if (fread(id, 1, 3, _file) != 3) {
    int errNum = errno;
    Radx::addErrStr(_errString, "ERROR - ", "BufrFile::_readSection1()", true);
    Radx::addErrStr(_errString, "  Cannot read length of message in octets", strerror(errNum), true);
    Radx::addErrStr(_errString, "  File path: ", _pathInUse, true);
    close();
    return -1;
  }
  nBytes = 0;
  nBytes = nBytes | id[2];
  nBytes = nBytes | (id[1] << 8);
  nBytes = nBytes | (id[0] << 16);
  */
  Radx::ui32 sectionLen;
  sectionLen = ExtractIt(24);
  if (_verbose) cerr << "sectionLen " << sectionLen << endl;

  /*
  Radx::ui08 *buffer;
  buffer = (Radx::ui08 *) calloc(sectionLen, sizeof(Radx::ui08));
  memset(buffer, 0, sectionLen);

  if (fread(buffer, 1, sectionLen-3, _file) != sectionLen-3) {
    int errNum = errno;
    Radx::addErrStr(_errString, "ERROR - ", "BufrFile::_readSection1()", true);
    Radx::addErrStr(_errString, "  Cannot read data", strerror(errNum), true);
    Radx::addErrStr(_errString, "  File path: ", _pathInUse, true);
    free(buffer);
    close();
    return -1;
  }
  */
  // check 1st bit of the octect for optional section 2
  /*
  bool section2 = buffer[4]; //  & 0x80; 
  _s1.hasSection2 = section2;
  if (_verbose) {
    cerr << "section 2? " ;
    if (section2) {
      cerr << "yes" << endl;
    } else {
      cerr << "no" << endl;
    }
  }
  */
  Radx::ui16 yearOfCentury;
  Radx::ui08 month;
  Radx::ui08 day;
  Radx::ui08 hour;
  Radx::ui08 minute;
  Radx::ui08 seconds;


  if (_s0.edition != 2) 
    throw "ERROR - Wrong version of section1 called ";
  _s1.masterTable = ExtractIt(8); // Octet 4
  _s1.originatingSubcenter  =  0; 
  _s1.generatingCenter = ExtractIt(16); // octets 5,6
  _s1.updateSequenceNumber =  ExtractIt(8); // octet 7
  _s1.hasSection2 = ExtractIt(1); ExtractIt(7); // octet 8
  _s1.dataCategoryType =  ExtractIt(8); // octet 9 BUFR Table A
  ExtractIt(8);  // octet 10 data category sub-type
  _s1.masterTableVersionNumber =  ExtractIt(8); // octet 11
  _s1.localTableVersionNumber =  ExtractIt(8); // octet 12


  if (_verbose) {
    cerr << "section 2? " ;
    if (_s1.hasSection2) {
      cerr << "yes" << endl;
    } else {
      cerr << "no" << endl;
    }
    printf("master table: %d\n", _s1.masterTable);
    printf("generating center: %d\n", _s1.generatingCenter);
    printf("originating subcenter: %d\n", _s1.originatingSubcenter);
    printf("update sequence number: %d\n", _s1.updateSequenceNumber);
    printf("data category type: %d\n", _s1.dataCategoryType);
    printf("local table version: %d\n", _s1.localTableVersionNumber);
    printf("master table version: %d\n", _s1.masterTableVersionNumber);
  }
  yearOfCentury = ExtractIt(8); // octet 13
  month =  ExtractIt(8); // octet 14
  day =  ExtractIt(8); // octet 15
  hour =  ExtractIt(8); // octet 16
  minute =  ExtractIt(8); // octet 17
  //ExtractIt(8); // octet 18 reserved
  seconds = 0; //
  _s1.year = yearOfCentury;
  _s1.month = month;
  _s1.day = day;
  _s1.hour = hour;
  _s1.minute = minute;
  _s1.seconds = seconds;  

  hdr_year = yearOfCentury;
  hdr_month = month;
  hdr_day = day;
  if (_verbose) printf("year-month-day hour:minute:sec\n%d-%d-%d %d:%d:%d\n",
		       yearOfCentury, month, day, hour, minute, seconds); 

  // read the rest of the octets in this section ...
  Radx::ui32 i;
  for (i=18; i<=sectionLen; i++)
    ExtractIt(8);
    
  return 0;
}

int BufrFile::readSection1_edition3()
{

  /******* decode section 1 */
  if (_verbose) fprintf (stderr, "Input file header:\n");

  // read ID
  // allocate this space based on the length of section read (octets 1 - 3)
  /*
  Radx::ui08 id[4];
  memset(id, 0, 4);
    
  // read length of message in octets
  // the length is 24 bits (3 bytes)
  Radx::ui32 nBytes;
  if (fread(id, 1, 3, _file) != 3) {
    int errNum = errno;
    Radx::addErrStr(_errString, "ERROR - ", "BufrFile::_readSection1()", true);
    Radx::addErrStr(_errString, "  Cannot read length of message in octets", strerror(errNum), true);
    Radx::addErrStr(_errString, "  File path: ", _pathInUse, true);
    close();
    return -1;
  }
  nBytes = 0;
  nBytes = nBytes | id[2];
  nBytes = nBytes | (id[1] << 8);
  nBytes = nBytes | (id[0] << 16);
  */
  Radx::ui32 sectionLen;
  sectionLen = ExtractIt(24); //nBytes;

  if (_verbose) cerr << "sectionLen " << sectionLen << endl;
  /*
  Radx::ui08 *buffer;
  buffer = (Radx::ui08 *) calloc(sectionLen, sizeof(Radx::ui08));
  memset(buffer, 0, sectionLen);

  if (fread(buffer, 1, sectionLen-3, _file) != sectionLen-3) {
    int errNum = errno;
    Radx::addErrStr(_errString, "ERROR - ", "BufrFile::_readSection1()", true);
    Radx::addErrStr(_errString, "  Cannot read data", strerror(errNum), true);
    Radx::addErrStr(_errString, "  File path: ", _pathInUse, true);
    free(buffer);
    close();
    return -1;
  }
  */
  // check 1st bit of the octect for optional section 2
  /*
  bool section2 = buffer[4]; //  & 0x80; 
  _s1.hasSection2 = section2;
  if (_verbose) {
    cerr << "section 2? " ;
    if (section2) {
      cerr << "yes" << endl;
    } else {
      cerr << "no" << endl;
    }
  }
  */
  Radx::ui16 yearOfCentury;
  Radx::ui08 month;
  Radx::ui08 day;
  Radx::ui08 hour;
  Radx::ui08 minute;
  Radx::ui08 seconds;


  // if (_s0.edition == 2) {
    _s1.masterTable = ExtractIt(8); //buffer[0];
    _s1.originatingSubcenter = ExtractIt(8); // 0; // buffer[1] << 8 | (buffer
    _s1.generatingCenter = ExtractIt(8); // buffer[1] << 8 | (buffer[2]);
    _s1.updateSequenceNumber = ExtractIt(8); // buffer[3]; // original BUFR message
    _s1.hasSection2 = ExtractIt(1); ExtractIt(7); 
    _s1.dataCategoryType = ExtractIt(8); // buffer[5];
ExtractIt(8);  // data sub-category
    _s1.masterTableVersionNumber = ExtractIt(8); // buffer[7];
    _s1.localTableVersionNumber = ExtractIt(8); // buffer[8];


    if (_verbose) {
      cerr << "section 2? " ;
      if (_s1.hasSection2) {
        cerr << "yes" << endl;
      } else {
        cerr << "no" << endl;
      }
      printf("master table: %d\n", _s1.masterTable);
      printf("generating center: %d\n", _s1.generatingCenter);
      printf("originating subcenter: %d\n", _s1.originatingSubcenter);
      printf("update sequence number: %d\n", _s1.updateSequenceNumber);
      printf("data category type: %d\n", _s1.dataCategoryType);
      printf("local table version: %d\n", _s1.localTableVersionNumber);
      printf("master table version: %d\n", _s1.masterTableVersionNumber);
    }
    //yearOfCentury = 0;
    yearOfCentury = ExtractIt(8); // yearOfCentury |  buffer[9];
    //  yearOfCentury = yearOfCentury | (buffer[12] << 8);
    month = ExtractIt(8); // buffer[10] | 0;
    day = ExtractIt(8); // buffer[11] | 0;
    hour = ExtractIt(8); // buffer[12] | 0;
    minute = ExtractIt(8); // buffer[13] | 0;
    seconds = 0; // buffer[18] | 0;
    ExtractIt(8); // Reserved for local use by ADP centers

    _s1.year = yearOfCentury;
    _s1.month = month;
    _s1.day = day;
    _s1.hour = hour;
    _s1.minute = minute;
    _s1.seconds = seconds;
    //}   

  hdr_year = yearOfCentury;
  hdr_month = month;
  hdr_day = day;
  if (_verbose) printf("year-month-day hour:minute:sec\n%d-%d-%d %d:%d:%d\n",
		       yearOfCentury, month, day, hour, minute, seconds); 
    
  //_numBytesRead += sectionLen;
  //free(buffer);
  return 0;
}


int BufrFile::readSection5() {
  // read the last section 
  // look for ending "7777" coded in CCITT International Alphabet No. 5
  string value;
  inSection5 = true;
  bool foundEndMark = false;
  try {
    MoveToNextByteBoundary();
    if (_debug) printf("looking for 7777 at %d bytes\n", _getCurrentBytePositionInFile());

    /*
    do {
      value = ExtractText(8);
    } while (value.compare("7") != 0);
    // read the last 7's
    // skip the last 7, because we hit end of file reading it.
    value = ExtractText(16);
    if (value.compare("77") != 0) {
      throw " Did not find ending code ";
    }
    */
    int countSeven = 0;
    do {
      value = ExtractText(8);
      if (value.compare("7") == 0) {
        countSeven += 1;
        if (countSeven >= 3)
          foundEndMark = true;
      } else {
        countSeven = 0;
      }
    } while (!foundEndMark);

  } catch (const char *msg) {
    Radx::addErrStr(_errString, "", "ERROR - BufrFile::readSection5", true);
    Radx::addErrStr(_errString, " ", " Could not read ending code", true);
    throw _errString.c_str();
  }

  if (!foundEndMark) {
    throw " Did not find ending code ";
  }
  
  return 0;
}

//
// peek ahead at the next few bytes for the ending 7777
//
bool BufrFile::isEndInSight() {
  // peek ahead for the last section 
  // look for ending "7777" coded in CCITT International Alphabet No. 5
  string value;
  // copy the current index values into the buffer
  int peekIndexBits =  currentBufferIndexBits;
  int peekLengthBits = currentBufferLengthBits;
  bool endFound = false;
  bool endMarkFound = false;
  
  try {
    if (_debug) printf("peeking ahead for 7777 at %d bytes\n", _getCurrentBytePositionInFile());
    int i;
    i = peekIndexBits/8;
    int timesThroughLoop = 0;
    int lastByte;
    lastByte = peekLengthBits/8;
    if (_debug) printf("starting at byte %d/%d bytes in buffer\n", i, lastByte);
    bool end = false;
    int countSeven = 0;
    // just peek at the next 10 bytes, if we see four 7's then flag it
    while (!end) {
      if (_dataBuffer[i] == '7') {
        countSeven += 1;
        if (_debug) printf("  found 7 at %d\n", i);
        if (countSeven >= 3) endMarkFound = true;
      }
      else countSeven = 0;
      i += 1;
      timesThroughLoop += 1;
      // only look at the next 10 bytes
      if ((i >= lastByte) || timesThroughLoop > 10) end = true;
    }
    if (endMarkFound || (i >= lastByte)) endFound = true;
    if (_debug) {
      if (endMarkFound)  printf("  found >3 contiguous 7's\n");
      else  printf("  endMarkFound not found\n");
    }
  } catch (const char *msg) {
    Radx::addErrStr(_errString, "", "ERROR - BufrFile::isEndInSight", true);
    Radx::addErrStr(_errString, " ", " Could not find ending code before end of file", true);
    throw _errString.c_str();
  }
  
  // of course, this doesn't work if we are right on the end of the buffer
  // and the ending 7777 crosses the buffer edge. TODO: fix this problem.
  return endFound;
}

void BufrFile::printSection1(ostream &out) {
  out << "  master table          : " << (unsigned int) _s1.masterTable << endl;
  out << "  generating center     : " << (unsigned int) _s1.generatingCenter << endl; 
  out << "  originating subcenter : " << (unsigned int) _s1.originatingSubcenter << endl; 
  out << "  update sequence number: " << (unsigned int) _s1.updateSequenceNumber << endl; 
  out << "  data category type    : " << (unsigned int) _s1.dataCategoryType << endl; 
  out << "  data category subtype : " << (unsigned int) _s1.dataCategorySubtype << endl; 
  out << "  master table version  : " << (unsigned int) _s1.masterTableVersionNumber << endl; 
  out << "  local table version   : " << (unsigned int) _s1.localTableVersionNumber << endl; 
  char buffer[50];
  sprintf(buffer, "%4d-%02d-%02d %02d:%02d:%02d", _s1.year, _s1.month, _s1.day,
	  _s1.hour, _s1.minute, _s1.seconds);
  out << "  YY-MM-DD HH:MM:SS     : " << buffer << endl;  
}

void BufrFile::printSection2(ostream &out) {
  if (_s1.hasSection2) {
    out << "file has section 2, but it's not used" << endl;
  } else {
    out << " -- no section 2 --" << endl;
  }
}

void BufrFile::printSection3(ostream &out) {
  unsigned char f, x, y;
  for (vector<unsigned short>::reverse_iterator i = _descriptorsToProcess.rbegin(); 
       i!= _descriptorsToProcess.rend(); ++i) {
    TableMapKey().Decode(*i, &f, &x, &y);
    out << (unsigned int) f << ";" << (unsigned int) x << ";" << (unsigned int) y << endl;
  }
}

void BufrFile::printSection4(ostream &out) {
  prettyPrintTree(out, GTree, 0);
  currentTemplate->printSweepData(out);
}

/*
void BufrFile::printSweepData(ostream &out) {
  currentTemplate->printSweepData(out);
}
*/

// taken from bufr2hdf5.c -- read_tables(...)
int BufrFile::readDescriptorTables() {

  return 0;
}

int BufrFile::readDataDescriptors() {  // read section 3
  /******* decode section 3 */
  if (_verbose) cerr << "Reading section 3 ...\n";

    // allocate this space based on the length of section read (octets 1 - 3)
  /*
    Radx::ui08 id[4];
    memset(id, 0, 4);
    
    // read length of message in octets
    // the length is 24 bits (3 bytes)
    Radx::ui32 nBytes;
    if (fread(id, 1, 3, _file) != 3) {
      int errNum = errno;
      Radx::addErrStr(_errString, "", "ERROR - BufrFile::readDataDescriptors", true);
      Radx::addErrStr(_errString, " ", " Cannot read length of section 3 in octets", true);
      Radx::addErrStr(_errString, "  ", strerror(errNum), true);
      close();
      throw _errString.c_str();
    }
    nBytes = 0;
    nBytes = nBytes | id[2];
    nBytes = nBytes | (id[1] << 8);
    nBytes = nBytes | (id[0] << 16);
  */
    Radx::ui32 sectionLenOctets;
    sectionLenOctets = ExtractIt(24); // nBytes;

    if (_debug) cerr << "sectionLen in octets " << sectionLenOctets << endl;
    /*
    Radx::ui08 *buffer;
    buffer = (Radx::ui08 *) calloc(sectionLen, sizeof(Radx::ui08));
    memset(buffer, 0, sectionLen);
    int nBytesToSectionEnd;
    nBytesToSectionEnd = sectionLen-3;
    if (fread(buffer, 1, nBytesToSectionEnd, _file) != sectionLen-3) {
      int errNum = errno;
      Radx::addErrStr(_errString, "", "ERROR - BufrFile::readDataDescriptors", true);
      Radx::addErrStr(_errString, " ", " Cannot read section 3", true);
      Radx::addErrStr(_errString, "  ", strerror(errNum), true);
      close();
      throw _errString.c_str();
    }
    */

    // octet 4 is reserved and set to zero
    //Radx::ui32 dummy;
    //dummy = 
    ExtractIt(8);

    // octets 5-6 contain the number of data subsets
    // get the number of data subsets/descriptors
    Radx::ui16 nDataSubsets;
    nDataSubsets = ExtractIt(16);
    /*
    nDataSubsets = 0;
    nDataSubsets = nDataSubsets | buffer[2];
    nDataSubsets = nDataSubsets | (buffer[1] << 8);
    */
    // sometimes the number of data subsets does not agree
    // with the length of the section, so go with the
    // length of the section over the number of data subsets
    // TODO: commenting this because it may be needed only for gsi, not cordoba??
    Radx::ui16 nDataSubsetsCalc = (sectionLenOctets - 7) /2;
    if (_verbose) printf("nDataSubsets = %d\n", nDataSubsetsCalc);
    if (nDataSubsets != nDataSubsetsCalc) {
      if (_verbose) {
        printf("WARNING - The number of data subsets: %u\n", (unsigned int) nDataSubsets);
        printf(" does not agree with the number calculated from the section length: %u\n", (unsigned int) nDataSubsetsCalc);
        printf("  Using the number calculated from the section length.\n");
      }
      nDataSubsets = nDataSubsetsCalc;
    }
    
    // octet 7: determine observed and compressed info
    //bool observedData = buffer[3] & 0x80;
    //bool compressedData = buffer[3] & 0x40;
    ExtractIt(8);
    // read the descriptors and keep them in a list for
    // traversal later 

    unsigned int nOctetsRead;
    nOctetsRead = 7;

    int i;
    //int startOffset = 4;
    // remember, we need to have two bytes remaining
    // in the buffer in order to grab another descriptor

    for (i=0; i<nDataSubsets; i++) {
      // get and decode the descriptors
      DescriptorKey d;
      // f is the first 2 bits
      Radx::ui08 f;
      f =  ExtractIt(2); //buffer[i] >> 6;  // 1100 0000
      d.f = f;
      d.x = ExtractIt(6); // buffer[i] & 0x3f; // 0011 1111
      d.y = ExtractIt(8); //  buffer[i + 1];
      if (_debug) printf("f x y: %d %d %d\n", d.f, d.x, d.y);
      unsigned short key;
      key = TableMapKey().EncodeKey(d.f, d.x, d.y);
      _descriptorsToProcess.insert(_descriptorsToProcess.begin(), key);
      nOctetsRead += 2;
    }
   
    // needed for Allison's data; but not for Cordoba or ODIM
    // read any extra octets
    //unsigned int extraOctets;
    //extraOctets = sectionLenOctets - (nDataSubsets*2 + 7);
    //cerr << "extraOctets " << extraOctets << endl;
    // ExtractIt(8*extraOctets);
    // end ... needed for Allison's data

    // the extra filler octets are optional for edition 4,
    // so don't bother reading them. Only read them for
    // older editions
    if (_s0.edition < 4) {
      // get to the end of this section ... 
      unsigned int  n;
      n = sectionLenOctets - nOctetsRead;
      cerr << "n octets to the end " << n << endl;
      ExtractIt(8*n);   
    }
    /*
    int i;
    int startOffset = 4;
    // remember, we need to have two bytes remaining
    // in the buffer in order to grab another descriptor
    for (i=startOffset; i<nBytesToSectionEnd-1; i+=2) {
      // get and decode the descriptors
      DescriptorKey d;
      // f is the first 2 bits
      Radx::ui08 f;
      f =  buffer[i] >> 6;  // 1100 0000
      d.f = f;
      d.x = buffer[i] & 0x3f; // 0011 1111
      d.y = buffer[i + 1];
      if (_verbose) printf("f x y: %d %d %d\n", d.f, d.x, d.y);
      unsigned short key;
      key = TableMapKey().EncodeKey(d.f, d.x, d.y);
      _descriptorsToProcess.insert(_descriptorsToProcess.begin(), key);
    }
    free(buffer);
    _numBytesRead += sectionLen;
    */

    // TODO: determine which type of product to instaniate based
    // on the data descriptors of section 3
    _isGsi = false;
    if (matches_204_31_X(_descriptorsToProcess)) {
      ExtractIt(8); // WHOA!  TODO:  skipping 8 bits???
      currentTemplate = new BufrProduct_204_31_X();
    } else if (matches_gsi(_descriptorsToProcess)) {
      currentTemplate = new BufrProduct_gsi();
      currentTemplate->haveTheTable(&tableMap);
    } else { 
      currentTemplate = new BufrProductGeneric();
    }
    currentTemplate->setDebug(_debug);
    currentTemplate->setVerbose(_verbose);
    currentTemplate->reset(); 
    // currentTemplate->setFieldName(_fieldName);
  return 0;
}


bool BufrFile::matches_204_31_X(vector<unsigned short> &descriptors) {
  // UGH! the descriptors are in reverse order.  Blah!  Need to
  // access the last ones.
  int n = descriptors.size();
  if (n >= 3) { 
    if ((descriptors[n-1] == 54732) && (descriptors[n-2] == 49439) &&
	((descriptors[n-3] == 54731) || (descriptors[n-3] == 54735))) {
      // 3;21;204 3;1;31 (3;21;203 or 3;21;207)
      return true; 
    } else {
      return false;
    }
  } else {
    return false;
  }
}

bool BufrFile::matches_gsi(vector<unsigned short> &descriptors) {
  // UGH! the descriptors are in reverse order.  Blah!  Need to
  // access the last ones.
  int n = descriptors.size();
  if (n >= 3) { 
    // $1 = std::vector of length 15, capacity 16 = {30, 7937, 16640, 34112, 49155, 7937, 17664, 49156, 7937, 16640, 3, 2, 1, 
    //  7937, 17152}

    if ((descriptors[n-1] == 17152) && (descriptors[n-2] == 7937) &&
	((descriptors[n-3] == 1) || (descriptors[n-3] == 2))) {
      return true; 
    } else if ((descriptors[n-3] == 16896) &&
               (descriptors[n-2] == 64021) && (descriptors[n-1] == 16128)) {
      // vector of length 6 {16383, 34305, 7937,   16896, 64021,   16128}
      //                  0;63;255, 2;6;1, 0;31;1, 1;2;0, 3;10;61, 0;63;0  
      _isGsi = true;
      return true;
      //} else if ((descriptors[n-3] == 16896) &&
      //         (descriptors[n-2] == ) && (descriptors[n-1] == 16128)) {
      // vector of length 6 {16383, 34305, 7937,   16896, ,   16128}
      //                  0;63;255, 2;6;1, 0;31;1, 1;2;0, 3;58;021, 0;63;0  
      // 
      //return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

// necessary for entry into this function:
// the file pointer must be positioned at the start of 
// section 4.
int BufrFile::readData() {  // read section 4

  // read and discard the 4 bytes of data
  // length of section in octets (octets 1-3)
  // set to zero (reserved) (octet 4)
  ExtractIt(8*4);

  try {
    if (!tableMap.filled())
      tableMap.ImportTables(_s1.masterTableVersionNumber,
			    _s1.generatingCenter, _s1.localTableVersionNumber,
			    _tablePath);
  } catch (const char *msg) {
    Radx::addErrStr(_errString, "", msg, true);
    throw _errString.c_str();
  }
  try {
    TraverseNew(_descriptorsToProcess);
  } catch (const char *msg) {
    // try to recover by continuing to next message
    cerr << "Attempting recovery ... \n";
  }
  
  return 0;
}

int BufrFile::ReplenishBuffer() {
  /*
  if (_firstBufferReplenish) { // (first) {
    fread(_dataBuffer, 1, 4, _file);
    _firstBufferReplenish = false;
  }
  */
  int nBytesRead;
  nBytesRead = fread(_dataBuffer, 1, MAX_BUFFER_SIZE_BYTES, _file);

  _numBytesRead += nBytesRead;
  if (_debug) printf("Read %d/%d bytes ", _numBytesRead, _s0.nBytes);

  if (_very_verbose) {
    printf("buffer: ");
    for (int i=0; i<MAX_BUFFER_SIZE_BYTES; i++) 
      printf("%0x ", _dataBuffer[i]);
    printf(" current position %d \n", currentBufferIndexBits);
  }

  return nBytesRead;
 
}

bool BufrFile::NextBit() {
  int idx;
  unsigned char mask;
  int bitPosition;
  bool bitValue;

    bitPosition = 7 - currentBufferIndexBits % 8;
    mask = 1 << bitPosition;
    idx = currentBufferIndexBits / 8;
    if (mask & _dataBuffer[idx]) {
      // insert a 1
      bitValue = true;
    } else {
      bitValue = false;
    }
   
    // advance the index
    currentBufferIndexBits += 1;
    if (currentBufferIndexBits >= currentBufferLengthBits) {
      // replenish the buffer
      currentBufferLengthBytes = ReplenishBuffer();
      currentBufferLengthBits = currentBufferLengthBytes * 8;
      currentBufferIndexBits = 0;
      if (currentBufferLengthBits <= 0) {
        throw "ERROR - End of file reached before end of descriptors.";
      }
    }
    return bitValue;
}


void BufrFile::MoveToNextByteBoundary() {

  while ((currentBufferIndexBits % 8) != 0) {   
    // advance the index
    currentBufferIndexBits += 1;
    if (currentBufferIndexBits >= currentBufferLengthBits) {
      // replenish the buffer
      currentBufferLengthBytes = ReplenishBuffer();
      currentBufferLengthBits = currentBufferLengthBytes * 8;
      currentBufferIndexBits = 0;
      if (currentBufferLengthBits <= 0) {
        throw "ERROR - End of file reached before end of descriptors.";
      }
    }
  }
}

int BufrFile::_getCurrentBytePositionInFile() {
  int currentBytePositionInFile;
  currentBytePositionInFile = _numBytesRead/MAX_BUFFER_SIZE_BYTES - 1 +
    currentBufferIndexBits/8;
  return currentBytePositionInFile;
}

// on error, throw exception; otherwise, return a string
string BufrFile::ExtractText(unsigned int nBits) {

  string val;
  unsigned char character = 0;

  val.clear();

  // if the number of bits is not a multiple of 8, then
  // we cannot extract as text
  if ((nBits % 8) != 0)  {
    Radx::addErrStr(_errString, "", "ERROR - BufrFile::ExtractText", true);
    Radx::addErrStr(_errString, "  ",
		    "Text width is not multiple of 8; cannot read in data.", true);
    throw _errString.c_str();
  }

  unsigned int i=0;
  bool endOfMessage = false;
  if ((_nBitsRead + nBits > (_s0.nBytes-4)*8) && !inSection5)
    endOfMessage = true;
  // move one bit at a time
  while ((i<nBits) && (!endOfMessage)) {
    if (NextBit()) {
      // insert a 1
      character = character * 2 + 1;
    } else {
      character = character * 2;
    }
   
    // // advance the index
    i += 1;
    // if we've completed a character, move it into string
    // and clear the value
    if ((i % 8) == 0) {
      if (isprint(character))
        val+=character;
      character = 0;
    }
  }

  if ((endOfMessage) && (i < nBits)) {
    val.clear();
    Radx::addErrStr(_errString, "", "ERROR - BufrFile::ExtractText", true);
    Radx::addErrStr(_errString, "  ", 
		    "Ran out of data before completing the value.", true);
    throw _errString.c_str(); 
  }  else {
    _nBitsRead += nBits;
    return _trim(val);
  }
}

string BufrFile::_trim(const std::string& str,
		       const std::string& whitespace) //  = " \t")
{
    size_t strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    size_t strEnd = str.find_last_not_of(whitespace);
    size_t strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

// on error, throw exception; otherwise, a positive integer will be returned
Radx::ui32 BufrFile::ExtractIt(unsigned int nBits) {

  Radx::ui32 val;

  //if (_verbose) {
  //  printf(" nBits=%d\n", nBits);
  //}

  if (nBits > 32) {
    Radx::addErrStr(_errString, "", "ERROR - BufrFile::ExtractIt", true);
    Radx::addErrInt(_errString, "  Request to extract > 32 bits: ", 
		    nBits, true);
    throw _errString.c_str();
  }

  val = 0;
  unsigned int i=0;
  bool endOfMessage = false;
  if ((_nBitsRead + nBits > (_s0.nBytes-4)*8) && !inSection5)
    endOfMessage = true;
  while ((i<nBits) && (!endOfMessage)) {
    if (NextBit()) {
      // insert a 1
      val = val * 2 + 1;
    } else {
      val = val * 2;
    }
    i += 1;
  }

  if ((endOfMessage) && (i < nBits)) {
    if (_debug) printf("%d/(%d including 7777) bits read; needed %d bits\n",
                       _nBitsRead, _s0.nBytes*8, nBits);
    Radx::addErrStr(_errString, "", "ERROR - BufrFile::ExtractIt", true);
    Radx::addErrStr(_errString, "  ", 
		    "Ran out of data before completing the value.", true);
    throw _errString.c_str(); 
  }  else {
    _nBitsRead += nBits;
    return val;
  }
}

Radx::ui32 BufrFile::Apply(TableMapElement f) {

  if (f._whichType != TableMapElement::DESCRIPTOR) {
    return -1;
  }
  if (_verbose) {
    cout << "Applying " << endl;
    //    prev = std::cout.fill ('x');
    //std::cout.width (50);
    std::cout << "  " << f._descriptor.fieldName << " ";
    //std::cout.fill(prev);
    unsigned int count = 50-f._descriptor.fieldName.size();
    if (count > 50) count = 0;
    for (unsigned int i=0; i<count; i++)
      cout << "-";
    cout << " " << f._descriptor.dataWidthBits << endl;
    cout << " scale  " << f._descriptor.scale << endl;
    cout << " units  " << f._descriptor.units << endl;;
    cout << " reference value " << f._descriptor.referenceValue << endl;
  }
  if (f._descriptor.units.find("CCITT") != string::npos) {
    string  value;
    value = ExtractText(f._descriptor.dataWidthBits);
    if (_verbose) {
      cout << " " << f._descriptor.dataWidthBits << endl;
      cout << "extracted string = " << value << endl;
    }
    _tempStringValue = value;
    string fieldName;
    fieldName = f._descriptor.fieldName;
    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);

    if (fieldName.find("station identifier") != string::npos) {
      if (fieldName.find("type of") != string::npos) {
        typeOfStationId = value;
      } else {
        stationId = value;
      }
    } else if (fieldName.find("odim quantity") != string::npos) {
      currentTemplate->typeOfProduct = value;
    }
    return 0;
  } else {
    Radx::ui32 value;
    value = ExtractIt(f._descriptor.dataWidthBits + _addBitsToDataWidth);
    if (_debug) cout << "returning unmodified " << value << endl;
    return value;
  }
}


//
// 10^(-10) to 10^10  ==> need 21 elements
//  -3 -2 -1 0 1 2 3
double BufrFile::fastPow10(int n)
{
  double pow10[21] = {
    .0000000001, .000000001, .00000001, .0000001, .000001,
         .00001, .0001, .001, .01, .1,
        1, 10, 100, 1000, 10000, 
    100000, 1000000, 10000000, 100000000, 1000000000,
                                         10000000000
    };

  if ((n > 10) || (n < -10))
    throw "Scale outside of bounds; must be between -10 and 10";
  else
    return pow10[n+10]; 
}
/*
Radx::si32 BufrFile::ApplyNumeric(TableMapElement f) {

  if (f._whichType != TableMapElement::DESCRIPTOR) {
    return -1;
  } 
  if (_verbose) {
    if (f._descriptor.fieldName.find("Byte element") == string::npos) {
    cout << "Applying " << endl;
    cout << "  " << f._descriptor.fieldName << " ";
    unsigned int count = 50-f._descriptor.fieldName.size();
    if (count > 50) count = 0;
    for (unsigned int i=0; i<count; i++)
      cout << "-";
    cout << " " << f._descriptor.dataWidthBits << endl;
    cout << " scale  " << f._descriptor.scale << endl;
    cout << " units  " << f._descriptor.units << endl;;
    cout << " reference value " << f._descriptor.referenceValue << endl;
    }
  }
  if (f._descriptor.units.find("CCITT") != string::npos) {
    string  value;
    value = ExtractText(f._descriptor.dataWidthBits);
    cout << "extracted string = " << value << endl;
    return 0;
  } else {
    Radx::ui32 value;
    value = ExtractIt(f._descriptor.dataWidthBits + _addBitsToDataWidth);
    Radx::si32 svalue;
    double temp;
    //svalue = (value+f._descriptor.referenceValue)/fastPow10(f._descriptor.scale);
    temp = f._descriptor.referenceValue * _multiplyFactorForReferenceValue;
    temp = value + temp;
    temp  = temp/fastPow10(f._descriptor.scale + _addBitsToDataScale);

    svalue = (Radx::si32) temp;
    //if ((_verbose) && (des != 7878)) 
    //  cout << "converted to " << svalue << endl;
    return svalue;
  }
}
*/

Radx::fl32 BufrFile::ApplyNumericFloat(TableMapElement f) {

  if (f._whichType != TableMapElement::DESCRIPTOR) {
    return -1;
  } 
  if (_verbose) {
    if (f._descriptor.fieldName.find("Byte element") == string::npos) {
    cout << "Applying " << endl;
    cout << "  " << f._descriptor.fieldName << " ";
    unsigned int count = 50-f._descriptor.fieldName.size();
    if (count > 50) count = 0;
    for (unsigned int i=0; i<count; i++)
      cout << "-";
    cout << " " << f._descriptor.dataWidthBits << endl;
    cout << " scale  " << f._descriptor.scale << endl;
    cout << " units  " << f._descriptor.units << endl;;
    cout << " reference value " << f._descriptor.referenceValue << endl;
    }
  }
  if (f._descriptor.units.find("CCITT") != string::npos) {
    string  value;
    value = ExtractText(f._descriptor.dataWidthBits);
    cout << "extracted string = " << value << endl;
    return 0;
  } else {
    Radx::ui32 value;
    value = ExtractIt(f._descriptor.dataWidthBits + _addBitsToDataWidth);
    Radx::fl32 svalue;
    double temp;
    //svalue = (value+f._descriptor.referenceValue)/fastPow10(f._descriptor.scale);
    temp = f._descriptor.referenceValue * _multiplyFactorForReferenceValue;
    temp = value + temp;
    temp  = temp/fastPow10(f._descriptor.scale + _addBitsToDataScale);

    svalue = (Radx::fl32) temp;
    if (_verbose) cout << "returning " << svalue << endl;
    return svalue;
  }
}

/*
// Put the info in the correct storage location
// and take care of any setup that needs to happen
bool BufrFile::StuffIt(string name, double value) {
  bool ok = true;

  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  if ((name.find("byte element") != string::npos) ||
      (name.find("pixel value") != string::npos)) {
    // ugh!  It's not a double value, it's an unsigned char...
    currentTemplate->addData((unsigned char) value);
  } else if (name.find("latitude") != string::npos) {
    latitude = value;
    // ******* TEST  *******
    //double latitudeTest;
    //latitudeTest = value;
    //ByteOrder::swap64(&latitudeTest, sizeof(double), true);
    //cerr << "latitude test = " << latitudeTest << endl;
  } else if (name.find("longitude") != string::npos) {
    longitude = value;
  } else if (name.find("height") != string::npos) {
    height = value;
  } else if (name.find("antenna elevation") != string::npos) {
    currentTemplate->setAntennaElevationDegrees(value);
  } else if ((name.find("number of bins along the radial") != string::npos)
	     || (name.find("number of pixels per row") != string::npos)) {
    currentTemplate->setNBinsAlongTheRadial((size_t) value);
  } else if ((name.find("range-bin size") != string::npos) ||
    (name.find("range bin size") != string::npos) || 
    (name.find("range-bin size") != string::npos)) {
    currentTemplate->setRangeBinSizeMeters(value);
  } else if ((name.find("range-bin offset") != string::npos) ||
    (name.find("range bin offset") != string::npos)) {
    currentTemplate->setRangeBinOffsetMeters(value);
  } else if (name.find("number of azimuths") != string::npos) {
    currentTemplate->setNAzimuths((size_t) value);
  } else if (name.find("antenna beam azimuth") != string::npos) {
    currentTemplate->setAntennaBeamAzimuthDegrees(value);

  } else if (name.find("year") != string::npos) {
    currentTemplate->putYear((int) value);
  } else if (name.find("month") != string::npos) {
    currentTemplate->putMonth((int) value);
  } else if (name.find("day") != string::npos) {
    currentTemplate->putDay((int) value);
  } else if (name.find("hour") != string::npos) {
    currentTemplate->putHour((int) value);
  } else if (name.find("minute") != string::npos) {
    currentTemplate->putMinute((int) value);
  } else if (name.find("second") != string::npos) {
    currentTemplate->putSecond((int) value);
    // could probably use key to identify these fields...
    // instead of a string search
  } else if (name.find("wmo block") != string::npos) {
    WMOBlockNumber = value;
  } else if (name.find("wmo station") != string::npos) {
    WMOStationNumber = value;
  } else if (name.find("compression method") != string::npos) {
    ; // ignore it
  } else if (name.find("type of station") != string::npos) {
    ; // ignore it
  } else if (name.find("type of product") != string::npos) {
    int code = (int) value;
    // make sure the type of product agrees with the field name
    switch(code) {
    case 0:
      currentTemplate->typeOfProduct = "DBZH";
      break;
    case 40:
      currentTemplate->typeOfProduct = "VRAD";
      break;
    case 80:
      currentTemplate->typeOfProduct = "ZDR";
      break;
    case 90:
      break;
    case 91: 
    case 230:
      currentTemplate->typeOfProduct = "TH";
      break;
    case 92:
    case 60:
      currentTemplate->typeOfProduct = "WRAD";
      break;	  
    case 243:
      currentTemplate->typeOfProduct = "CM";
      break;
    case 240:
      currentTemplate->typeOfProduct = "KDP";
      break;
    case 239:
      currentTemplate->typeOfProduct = "PHIDP";
      break;
    case 241:
      currentTemplate->typeOfProduct = "RHOHV";
      break;
    case 242:
      // ugh! allow for different name for this variable
      // fieldName == "TDR"  ==> use "TDR"
      // fieldName == ""     ==> use "ZDR"
      // fieldName == "ZDR"  ==> use "ZDR"
      if (_fieldName.size() < 0) {
          currentTemplate->typeOfProduct = "ZDR";
      } else {
        string temp = _fieldName;
        std::transform(temp.begin(), temp.end(), temp.begin(), ::toupper);
        if (temp.compare("ZDR") == 0) {
          currentTemplate->typeOfProduct = "ZDR";
	} else {
	  currentTemplate->typeOfProduct = "TDR";
	}
      }
      break;
    case 231:
      currentTemplate->typeOfProduct = "TV";
      break; 
    default:
      currentTemplate->typeOfProduct = "UNKNOWN";
      ok = false;
    } 
    // make sure the field name we are looking for agrees with
    // the code in the data file
    // skip this check if the type of product has code 90; 
    // I don't know what product code 90 means.
    if (code != 90) {
      if (_fieldName.size() > 0) {
        string temp = _fieldName;
        std::transform(temp.begin(), temp.end(), temp.begin(), ::toupper);
        if (currentTemplate->typeOfProduct.compare(temp) != 0) {
	  Radx::addErrStr(_errString, "", "ERROR - BufrFile::StuffIt", true);
	  Radx::addErrStr(_errString, "  Expected Type of Product code for ", 
			  temp.c_str(), true);
	  Radx::addErrInt(_errString, "  Found code ", code, true);
	  throw _errString.c_str();
	}
      } else {
        //  TODO: we'll need to use the currentTemplate->typeOfProduct as the field name
	;
      }
    }
  } else {
    ok = false;
  }
  return ok;
}
*/


size_t BufrFile::getTimeDimension() {
  return currentTemplate->getNAzimuths();
}

//size_t BufrFile::getMaxRangeDimension() {
//  return currentTemplate->getMaxBinsAlongTheRadial();
//}

size_t BufrFile::getNumberOfSweeps() {
  return currentTemplate->sweepData.size();
}

size_t BufrFile::getNBinsAlongTheRadial(int sweepNumber) {
  return currentTemplate->getNBinsAlongTheRadial(sweepNumber);
}

double BufrFile::getRangeBinOffsetMeters() {
  return currentTemplate->getRangeBinOffsetMeters();
}

double BufrFile::getRangeBinSizeMeters(int sweepNumber) {
  return currentTemplate->getRangeBinSizeMeters(sweepNumber);
}

double BufrFile::getElevationForSweep(int sweepNumber) {
  return currentTemplate->sweepData.at(sweepNumber).antennaElevationDegrees;
}

int BufrFile::getNAzimuthsForSweep(int sweepNumber) {
  return currentTemplate->sweepData.at(sweepNumber).nAzimuths;
}

double BufrFile::getStartingAzimuthForSweep(int sweepNumber) {
  return currentTemplate->sweepData.at(sweepNumber).antennaBeamAzimuthDegrees;
}

double BufrFile::getStartTimeForSweep(int sweepNumber) {
  // BufrProduct::TimeStamp t;
  // t = currentTemplate->sweepData.at(sweepNumber).startTime;
  // RadxTime rTime(t.year, t.month, t.day, t.hour, t.minute, t.second); 
  // return rTime.asDouble();
  //return currentTemplate->sweepData.at(sweepNumber).startTime.asDouble();
  RadxTime *time;
  time = currentTemplate->sweepData.at(sweepNumber).startTime;
  return time->asDouble();
}

double BufrFile::getEndTimeForSweep(int sweepNumber) {
  // BufrProduct::TimeStamp t;
  // t = currentTemplate->sweepData.at(sweepNumber).endTime;
  // RadxTime rTime(t.year, t.month, t.day, t.hour, t.minute, t.second); 
  // return rTime.asDouble();
  //return currentTemplate->sweepData.at(sweepNumber).endTime.asDouble();
  // TODO: shouldn't this be a time_t value?? instead of a double??
  RadxTime *time;
  time = currentTemplate->sweepData.at(sweepNumber).endTime;
  return time->asDouble();
}

time_t BufrFile::getStartUTime(int sweepNumber) {
  RadxTime *time;
  time = currentTemplate->sweepData.at(sweepNumber).startTime;
  return time->utime();
}

time_t BufrFile::getEndUTime(int sweepNumber) {
  RadxTime *time;
  time = currentTemplate->sweepData.at(sweepNumber).endTime;
  return time->utime();
}

float *BufrFile::getDataForSweepFl32(int sweepNumber) { 
  return currentTemplate->sweepData.at(sweepNumber).parameterData[0].data;
}

double *BufrFile::getDataForSweepFl64(int sweepNumber) { 
  return currentTemplate->sweepData.at(sweepNumber).parameterDataFl64[0].data;
}

string BufrFile::getTypeOfProductForSweep(int sweepNumber) { 
  if (_fieldName.size() > 0) 
    return _fieldName;
  else 
    return currentTemplate->sweepData.at(sweepNumber).parameterData[0].typeOfProduct;
}

void BufrFile::_deleteAfter(DNode *p) {
  DNode *q;
  if (p!=NULL) {
    q = p->next;
    p->next = q->next;
    free(q);
  }
}

//  Node *      _buildTree(vector<unsigned short> descriptors);
//BufrFile::DNode* BufrFile::buildTree(vector<unsigned short> descriptors, bool reverse) {
DNode* BufrFile::buildTree(vector<unsigned short> descriptors, bool reverse) {
  DNode *t;
  t = NULL;
  if (reverse) {
  for (vector<unsigned short>::reverse_iterator i = descriptors.rbegin(); i!= descriptors.rend(); ++i) {

    DNode *p = new DNode();
    p->des = *i;
    //p->nRepeats = 0;
    unsigned short new_descriptor;
    new_descriptor = TableMapKey().EncodeKey(0, 0, 0);
    p->delayed_repeater = new_descriptor;
    p->children = NULL;
    p->next = t;
    p->somejunksvalue = "";
    t = p;
  }
  } else {
  for (vector<unsigned short>::iterator i = descriptors.begin(); i!= descriptors.end(); ++i) {

    DNode *p = new DNode();
    p->des = *i;
    unsigned short new_descriptor;
    new_descriptor = TableMapKey().EncodeKey(0, 0, 0);
    p->delayed_repeater = new_descriptor;
    p->children = NULL;
    p->next = t;
    t = p;
  }
  }
  return t;
}

void BufrFile::printTree(DNode *tree, int level) {
  DNode *p,*q;
  p = tree;
  if (level == 0) printf("tree: \n");
  while (p!=NULL) {
    for (int i=0; i<level; i++) printf("  "); 
    // printf("+(%d) delayed_rep %u\n", p->des, p->delayed_repeater);
    prettyPrintNode(cout, p, level);
    q=p->children;
    if (q != NULL) {
      printTree(q, level+1);
    }
    p=p->next;
  }
}

void BufrFile::printHeader() {
  cout << "Tree: \n";
  cout.width(55);
  cout << " Descriptor ";
  cout.width(18);
  cout << " width(bits) ";
  cout.width(7);
  cout << "scale";
  cout.width(12);
  cout << " units ";
  cout.width(11);
  cout << " reference ";
  cout.width(7);
  cout << " value " << endl;
}

void BufrFile::prettyPrintReplicator(ostream &out, DNode *p, int level) {
  unsigned char f, x, y;
  //unsigned short des;

  for (int i=0; i<level; i++) printf(" ");
  TableMapKey().Decode(p->des, &f, &x, &y);
  printf("+(%1d %02d %03d) ", (unsigned int) f, (unsigned int) x, (unsigned int) y);
  if (y == 0) { // variable repeater ....
    printf(" repeats %d", p->ivalue);
  }
  printf("\n");
}

// TableMapElement gives us information about the data
// des is the encoded key to find the TableMapElement
void BufrFile::prettyPrintNode(ostream &out, DNode *p, int level) {
  unsigned char f, x, y;
  //unsigned short des;

  for (int i=0; i<level; i++) printf(" ");
  TableMapKey().Decode(p->des, &f, &x, &y);
  printf("+(%1d %02d %03d) ", (unsigned int) f, (unsigned int) x, (unsigned int) y);
  printf(" self=%zu ",(size_t) p);
  if (p!=NULL) printf(" next->%zu children->%zu \n", (size_t) p->next, (size_t) p->children); 
}

// TableMapElement gives us information about the data
// des is the encoded key to find the TableMapElement
void BufrFile::prettyPrintLeaf(ostream &out, DNode *p,
			       TableMapElement &element, int level) {
  //string svalue;
  //TableMapElement element;
  unsigned char f, x, y;
  // unsigned short des;

  for (int i=0; i<level; i++) printf(" ");
  //element = tableMap.Retrieve(p->des);
  TableMapKey().Decode(p->des, &f, &x, &y);
  printf("+(%1d %02d %03d) ", (unsigned int) f, (unsigned int) x, (unsigned int) y);
  cout << "  " << element._descriptor.fieldName << " ";
  unsigned int count;
  count = 50-level - element._descriptor.fieldName.size();
  if (count > 50) {
    count = 0;
  }
  for (unsigned int i=0; i<count; i++)
    cout << "-";
  cout << " ";
  cout.width(5);
  cout << element._descriptor.dataWidthBits;
  cout.width(5);
  cout << element._descriptor.scale;
  cout.width(15);
  cout << element._descriptor.units;
  cout.width(15);
  cout << element._descriptor.referenceValue << " ";
  //need to print the value ...
  switch (p->dataType) {
  case DNode::INT:
    cout << p->ivalue << endl;
    break;
  case DNode::FLOAT:
    cout << p->fvalue << endl;
    break;
  case DNode::STRING:
    cout << p->somejunksvalue << endl;
    break;
  default:
    cout << "undefined value" << endl;
  }
}

void BufrFile::prettyPrint(ostream &out, DNode *p, int level) {
  TableMapElement element;
  unsigned short des;

  des = p->des;
  TableMapKey key(p->des);

  if (key.isTableBEntry()) {  // a leaf
    element = tableMap.Retrieve(des);
    prettyPrintLeaf(out, p, element, level);
  } else if (key.isReplicator()) {
    prettyPrintReplicator(out, p, level);
  } else {
    prettyPrintNode(out, p, level);
  }
}

void BufrFile::prettyPrintTree(ostream &out, DNode *tree, int level) {
  //TableMapElement f;
  string svalue;
  //double fvalue;
  //int ivalue;
  DNode *p,*q;
  TableMapElement element;
  //unsigned char f, x, y;

  if (level == 0) {
    printHeader();
  }
  p = tree;
  while (p != NULL) {
    prettyPrint(out, p, level);
    // print the children
    q=p->children;
    if (q != NULL) {
      prettyPrintTree(out, q, level+1);
    }
    p = p->next;
  }
}

void BufrFile::freeTree(DNode *tree) {
  DNode *p,*q;
  p = tree;
  while (p!=NULL) {
    // free the children
    q=p->children;
    if (q != NULL) {
      //freeTree(q);
      delete q;
    }
    DNode *temp;
    temp = p;
    p=p->next;
    if (_verbose) printf("freeing %d\n", temp->des);
    delete temp;
  }
}

// traverse the trees from section 3.  These are the decoders
// of the data in section 4
// need knowledge of BUFR tables, pointer to data file ( _file )
// returns a list of any modifications to the repeat factors
// we are going to access the global list of descriptors (_descriptorsToProcess)
int BufrFile::TraverseNew(vector<unsigned short> descriptors) {

  GTree = buildTree(descriptors, false);
  // bool continuousRepeat = false;
  //if (_isGsi) continuousRepeat = true;
  int result = -1;
  //do {
    result = _descend(GTree);
    // What remains in the buffer at this point?
    //} while (!isEndInSight()); // (continuousRepeat);
  return result;
}

int BufrFile::moveChildren(DNode *parent, int howManySiblings) {
  DNode *p;
  int x;
  p = parent;
  x = howManySiblings;

          // move the children if they aren't already there
          if (p->children == NULL) {
            // remove the next x descriptors and make them children
            DNode *h, *t;
            h = p->next;
            t = h->next;
            int i = 1;
            while ((t!= NULL) && (i<x)) {
              h = t;
              t = t->next;
              i += 1;
            }
            p->children = p->next;
            // reconnect the list
            p->next = t;
	    h->next = NULL;
	  }  // end moving children
	  return 0;
}

/*
// Print the tree while we are traversing, since this is 
// the best way to display the values along with the the descriptors.
// The values are stored in a separate structure.
int BufrFile::_descend(DNode *tree) {

  if (_verbose) {
    if ((tree->des != 7878) && (tree->des !=7681)) { 
      // don't print Byte element of compressed array
      // don't print Pixel value (4 bits)
      printf("\nTraversing ... \n");
      printTree(tree,0);
    }
  }

  unsigned short des;
  DNode *p;
  p = tree;
  bool compressionStart = false;

  try {
    // for each descriptor in the list
    while (p != NULL ) {

      unsigned char f, x, y;

      des = p->des;
      TableMapKey key(des);

      // visit the node
      if (_verbose) {
        if ((des != 7878)  && (tree->des !=7681)) {
	  TableMapKey().Decode(des, &f, &x, &y);
	  printf("visiting f(x,y): %1d(%02d,%03d) ", (unsigned int) f,
		 (unsigned int) x, (unsigned int) y);  
	}     
      }

      if (key.isTableBEntry()) {  // a leaf
	if (_verbose) { 
          if ((des != 7878) && (tree->des !=7681))
  	    printf(" leaf\n");
	}
	// if the node is from table b, retrieve the data; apply any transformations;
	//   insert into temporary structure for saving
	TableMapElement val1;
	val1 = tableMap.Retrieve(des);
	if (val1._whichType == TableMapElement::DESCRIPTOR) {
	  //if (_debug) prettyPrintLeaf(cout, des, val1, prettyPrintLevel);
	  //cout << "value for key " << des << ":" <<
	  //  val1._descriptor.fieldName << "," << 
	  //  val1._descriptor.scale << endl;
	  ;
	}
	Radx::fl32 valueFromData;
	if (val1.IsText()) {
	  // THE NEXT TWO LINES ARE CRUCIAL!! DO NOT REMOVE IT!!!
	  // we don't care about the return value when the descriptor is text
	  Apply(val1); 
	  // grab the text value
	  p->dataType = DNode::STRING;
	  p->somejunksvalue = _tempStringValue;
	  if (!currentTemplate->StuffIt(des, val1._descriptor.fieldName,
                                        _tempStringValue )) {
	      Radx::addErrStr(_errString, "", "WARNING - BufrFile::_descend", true);
	      Radx::addErrStr(_errString, "Unrecognized descriptor: ",
			      val1._descriptor.fieldName, true);
	      // since this is just a warning, just print the message, no need
	      // to exit
	      cerr << _errString << endl;
          }
	} else {
	  valueFromData = ApplyNumericFloat(val1);
	  if (!currentTemplate->StuffIt(des, val1._descriptor.fieldName, valueFromData)) {
            if ((des != 7681) && 0) {
	      Radx::addErrStr(_errString, "", "WARNING - BufrFile::_descend", true);
	      Radx::addErrStr(_errString, "Unrecognized descriptor: ",
			      val1._descriptor.fieldName, true);
	      // since this is just a warning, just print the message, no need
	      // to exit
	      cerr << _errString << endl;
	    }
	  }
	  if (val1._descriptor.fieldName.find("Compression method") != string::npos) {
	    compressionStart = true;
	  }
	  // store the value
	  p->dataType = DNode::FLOAT;
	  p->fvalue = valueFromData;
	}
	//if (_debug) cout << endl;
	p = p->next;
      } else if (key.isTableCEntry()) {  // a leaf
	if (_verbose) { 
  	    printf(" leaf\n");
	}
	// if the node is from table c, 
	// we are only handling one descriptor from table c ...
	// decode the key into f(x,y) and check y for different action
        unsigned char f,x,y;
        TableMapKey().Decode(des, &f, &x, &y);
        switch(x) {
          case 1:
            // apply the modifications
            _addBitsToDataWidth = max(0, y - 128);
            if (_verbose) printf(" increasing dataWith by %d\n", _addBitsToDataWidth);
            break;
          case 5:
            // YYY characters (CCITT International Alphabet No. 5) are
            // inserted as a data field of YYY x 8 bits in length.
            // read the bytes ... 
            //string whatIsThis;
            //whatIsThis = ExtractIt(y*8);
            TableMapElement *dummy;
            int dataWidth;
            dataWidth = y * 8;
            dummy = new TableMapElement("DUMMY", 0, "CCITT IA5", 0, dataWidth);
            Apply(*dummy);
            delete dummy;
            if (!currentTemplate->StuffIt(des, "", _tempStringValue )) {
	      Radx::addErrStr(_errString, "", "WARNING - BufrFile::_descend", true);
	      Radx::addErrStr(_errString, "Unrecognized descriptor: ", "DUMMY", true);
	      // since this is just a warning, just print the message, no need
	      // to exit
	      cerr << _errString << endl;
            }
            break;
	  default:
	  // TODO: fix up this error message
          cerr << "Table C descriptor is not implemented " << endl;
	}
	p = p->next;

      } else if (key.isReplicator()) {
	if (_verbose) 
	  printf(" replicator\n");

	// if the node is a replicator, e.g. 1;2;0 or 1;3;5
	// decode the key into f(x,y) and check y for different action
        unsigned char f,x,y;
        TableMapKey().Decode(des, &f, &x, &y);
	bool variable_repeater = false;
        if (y == 0) variable_repeater = true;
      
        if (variable_repeater) {
	  // there will be a special "delayed replication", y=0
          unsigned short delayed_replication_descriptor;
          if (p->children == NULL) { // if we haven't been here before ...
            DNode *delayed_rep_node;
            delayed_rep_node = p->next;
            delayed_replication_descriptor = delayed_rep_node->des;
            // remove the delayed replication descriptor node from the list
            _deleteAfter(p);   
            // and save it in the node itself
            p->delayed_repeater = delayed_replication_descriptor;
          } else {
            delayed_replication_descriptor = p->delayed_repeater;
	  }
          // get the number of repeats from section 4 data
	  Radx::ui32 nRepeats; // actually read this from the data section
          nRepeats = Apply(tableMap.Retrieve(delayed_replication_descriptor));
          //if (_verbose) 
            printf("nrepeats from Data = %u\n", nRepeats);
          currentTemplate->storeReplicator(nRepeats);
          p->ivalue = nRepeats;
          if (p->children == NULL) {
            moveChildren(p, x);
	  }
          // transition state; set location levels
          // the state determines which counters to increment & decrement
          // It's up to the product to deal with the space allocation as needed
          for (unsigned int i=0; i<nRepeats; i++) {
            //if (((i%1000)==0) && (_verbose)) 
              printf("%d out of %d repeats\n", i+1, nRepeats);
            _descend(p->children);
	  }
          if (_verbose) printf("-- end repeat\n");
          // transition state; set location levels
          currentTemplate->trashReplicator();
          p=p->next;
	} else {           // must be a fixed repeater
          if (p->children == NULL) {
            moveChildren(p, x);
	  }
          for (int i=0; i<y; i++) {
            if (((i%1000)==0) && (_verbose))
              printf("%d out of %d repeats\n", i+1, y);
            _descend(p->children);
	  }
          if (_verbose) printf("-- end repeat\n");
          p=p->next;
	} // end else fixed repeater
        if (_verbose) printTree(tree, 0);
      } else if (key.isAnotherNode()) {
	if (_verbose) 
	  printf(" another node\n");
	// pop/remove the element from the list; it is being replaced
	// if the node is another node,
	// look up the expansions and insert them...
        TableMapElement val1;
        val1 = tableMap.Retrieve(des);
	 
	vector<unsigned short> theList;
	theList = val1._listOfKeys;

        // replace the contents of this node with the first element of the list
        p->des = theList.front();
        // insert the remaining elements after this node

        theList.erase(theList.begin());
	if (theList.size() > 0) {
	  DNode *newList = buildTree(theList, true);
          DNode *save;
          save = p->next;
          p->next = newList;
  	  // find the end of the new list;
          DNode *h, *t;
          h = newList;
          t = newList->next;
          while (t != NULL) {
            h = t;
            t = t->next;
          }
          h->next = save;
	}
        if (_verbose) printTree(tree, 0);
      } else {
        Radx::addErrStr(_errString, "", "ERROR - BufrFile::_descend", true);
	Radx::addErrInt(_errString, "   unrecognized table map key: ", des, true);
	throw _errString.c_str();
      }
    } // end while p!= NULL
    if (compressionStart) {
      currentTemplate->createSweep();
      compressionStart = false;
    }
  } catch (const std::out_of_range& e) {
    Radx::addErrStr(_errString, "", "ERROR - BufrFile::_descend", true);
    Radx::addErrInt(_errString, "   unknown descriptor: ", des, true);
    cerr << _errString;
    throw _errString.c_str();
  } catch (const char *msg) {
    Radx::addErrStr(_errString, "", "ERROR - BufrFile::_descend", true);
    Radx::addErrStr(_errString, "  ", msg, true);
    cerr << _errString;
    throw _errString.c_str();
  }
  return 0;
}
*/

void BufrFile::_verbosePrintTree(DNode *tree) {
  if (_verbose) {
    if ((tree->des != 7878) && (tree->des !=7681)) { 
      // don't print Byte element of compressed array
      // don't print Pixel value (4 bits)
      printf("\nTraversing ... \n");
      printTree(tree,0);
    }
  }
}

void BufrFile::_visitTableBNode(DNode *p, bool *compressionStart) {

  // if the node is from table b, retrieve the data; apply any transformations;
  //   insert into temporary structure for saving

  //unsigned char f, x, y;
  unsigned short des;
  des = p->des;

  TableMapElement val1;
  val1 = tableMap.Retrieve(des);

  Radx::fl32 valueFromData;
  if (val1.IsText()) {
    // THE NEXT TWO LINES ARE CRUCIAL!! DO NOT REMOVE IT!!!
    // we don't care about the return value when the descriptor is text
    Apply(val1); 
    // grab the text value
    p->dataType = DNode::STRING;
    p->somejunksvalue = _tempStringValue;
    if (!currentTemplate->StuffIt(des, val1._descriptor.fieldName,
                                  _tempStringValue )) {
      Radx::addErrStr(_errString, "", "WARNING - BufrFile::_descend", true);
      Radx::addErrStr(_errString, "Unrecognized descriptor: ",
                      val1._descriptor.fieldName, true);
      // since this is just a warning, just print the message, no need
      // to exit
      cerr << _errString << endl;
    }
  } else {
    valueFromData = ApplyNumericFloat(val1);
    if (_verbose) printf(" valueFromData = %f\n", valueFromData);
    if (!currentTemplate->StuffIt(des, val1._descriptor.fieldName, valueFromData)) {
      if ((des != 7681) && 0) {
        Radx::addErrStr(_errString, "", "WARNING - BufrFile::_descend", true);
        Radx::addErrStr(_errString, "Unrecognized descriptor: ",
                        val1._descriptor.fieldName, true);
        // since this is just a warning, just print the message, no need
        // to exit
        cerr << _errString << endl;
      }
    }
    if (val1._descriptor.fieldName.find("Compression method") != string::npos) {
      *compressionStart = true;
    }
    // store the value
    p->dataType = DNode::FLOAT;
    p->fvalue = valueFromData;
  }
}

void BufrFile::_visitTableCNode(DNode *p) {

  // if the node is from table c, 
  // we are only handling one descriptor from table c ...
  // decode the key into f(x,y) and check y for different action
  unsigned char f,x,y;
  unsigned short des;
  des = p->des;
  TableMapKey().Decode(des, &f, &x, &y);
  switch(x) {
    case 1:
      // apply the modifications
      if (y == 0) _addBitsToDataWidth = 0;
      else _addBitsToDataWidth = (unsigned int) y - 128;
      if (_debug) printf(" changing dataWith by %d\n", _addBitsToDataWidth);
      break;
    case 2:
      // apply the modifications
      if (y == 0) _addBitsToDataScale = 0;
      else _addBitsToDataScale = (unsigned int) y - 128;
      if (_debug) printf(" changing Scale by %d\n", _addBitsToDataScale);
      break;
    case 5:
      // YYY characters (CCITT International Alphabet No. 5) are
      // inserted as a data field of YYY x 8 bits in length.
      // read the bytes ... 
      //string whatIsThis;
      //whatIsThis = ExtractIt(y*8);
      TableMapElement *dummy;
      int dataWidth;
      dataWidth = y * 8;
      dummy = new TableMapElement("DUMMY", 0, "CCITT IA5", 0, dataWidth);
      Apply(*dummy);
      delete dummy;
      if (!currentTemplate->StuffIt(des, "", _tempStringValue )) {
        Radx::addErrStr(_errString, "", "WARNING - BufrFile::_descend", true);
        Radx::addErrStr(_errString, "Unrecognized descriptor: ", "DUMMY", true);
        // since this is just a warning, just print the message, no need
        // to exit
        cerr << _errString << endl;
      }
      break;
    case 6: 
      // YYY bits of data are described by the immediately following descriptor
      //Radx::ui32 value;
      //value = ExtractIt((unsigned) y);
      //printf("HERE 2 6 YYY = %d _nBitsRead=%d\n", value, _nBitsRead);
      printf("HERE 2 6 YYY =  _nBitsRead=%d\n",_nBitsRead);
      break;
    case 7:
      // Add YYY to the existing scale factor
      // multiply the existing reference value by 10^(YYY)
      // calculate ((10 * YYY) + 2) / 3,  disregard any fractional remainder
      // and add the result to the existing bit width.
      if (_debug) printf(" table C: 2 7 YYY where YYY = %u\n", y);
      unsigned int yValue;
      yValue = y;
      _addBitsToDataWidth = ((10 * yValue) + 2) / 3;
      if (_debug) printf(" changing dataWith by %d\n", _addBitsToDataWidth);
      _addBitsToDataScale = yValue;
      if (_debug) printf(" changing Scale by %d\n", _addBitsToDataScale);
                             _multiplyFactorForReferenceValue = fastPow10(yValue);
      if (_debug) printf(" changing Reference by factor of %d\n",
                          _multiplyFactorForReferenceValue);
      break;
    default:
      // report an error message
      cerr << "Table C descriptor is not implemented " 
           << (unsigned) f << ";" << (unsigned) x << ";" << (unsigned) y << ";"        
           << endl;
  }
}

void BufrFile::_visitVariableRepeater(DNode *p, unsigned char x) {

  // there will be a special "delayed replication", y=0
  unsigned short delayed_replication_descriptor;
  if (p->children == NULL) { // if we haven't been here before ...
    DNode *delayed_rep_node;
    delayed_rep_node = p->next;
    delayed_replication_descriptor = delayed_rep_node->des;
    // remove the delayed replication descriptor node from the list
    _deleteAfter(p);   
    // and save it in the node itself
    p->delayed_repeater = delayed_replication_descriptor;
  } else {
    delayed_replication_descriptor = p->delayed_repeater;
  }
  // get the number of repeats from section 4 data
  Radx::ui32 nRepeats; // actually read this from the data section
  nRepeats = Apply(tableMap.Retrieve(delayed_replication_descriptor));
  if (_verbose) 
    printf("nrepeats from Data = %u\n", nRepeats);
  //if (nRepeats == 0)
  //  printf("HERE<<<<< \n");
  currentTemplate->storeReplicator(nRepeats);
  p->ivalue = nRepeats;
  if (p->children == NULL) {
    moveChildren(p, x);
  }
  if (0) {
        // for debugging
    printf(" after moving children: current state of tree; p = %zu\n", (size_t) p);
        printTree(p,0);
        printf(" after moving children: end current state of tree\n");
        // end for debugging
  }

  // transition state; set location levels
  // the state determines which counters to increment & decrement
  // It's up to the product to deal with the space allocation as needed
  for (unsigned int i=0; i<nRepeats; i++) {
    if (((i%1000)==0) && (_verbose)) 
      printf("%d out of %d repeats\n", i+1, nRepeats);
    _descend(p->children);
  }
  if (_verbose) 
    printf("-- end repeat %d\n", nRepeats);
  // transition state; set location levels
  currentTemplate->trashReplicator();

}


void BufrFile::_visitFixedRepeater(DNode *p, unsigned char x, unsigned char y) {
  if (p->children == NULL) {
    moveChildren(p, x);
  }
  for (int i=0; i<y; i++) {
    if (((i%1000)==0) && (_verbose))
      printf("%d out of %d repeats\n", i+1, y);
    _descend(p->children);
  }
  if (_verbose) printf("-- end repeat\n");

}

void BufrFile::_visitTableDNode(DNode *p) {

  unsigned short des;
  des = p->des;

  if (_verbose) 
    printf(" another node\n");
  // pop/remove the element from the list; it is being replaced
  // if the node is another node,
  // look up the expansions and insert them...
  TableMapElement val1;
  val1 = tableMap.Retrieve(des);
	 
  vector<unsigned short> theList;
  theList = val1._listOfKeys;

  // replace the contents of this node with the first element of the list
  p->des = theList.front();
  // insert the remaining elements after this node

  theList.erase(theList.begin());
  if (theList.size() > 0) {
    DNode *newList = buildTree(theList, true);
    DNode *save;
    save = p->next;
    p->next = newList;
    // find the end of the new list;
    DNode *h, *t;
    h = newList;
    t = newList->next;
    while (t != NULL) {
      h = t;
      t = t->next;
    }
    h->next = save;
  }
  if (_verbose) printTree(p, 0);
}


void BufrFile::_visitReplicatorNode(DNode *p) {

  unsigned short des;
  des = p->des;

  if (_verbose) 
    printf(" replicator\n");

  // if the node is a replicator, e.g. 1;2;0 or 1;3;5
  // decode the key into f(x,y) and check y for different action
  unsigned char f,x,y;
  TableMapKey().Decode(des, &f, &x, &y);
  bool variable_repeater = false;
  if (y == 0) variable_repeater = true;
      
  if (variable_repeater) {
    _visitVariableRepeater(p, x);
  } else {           // must be a fixed repeater
    _visitFixedRepeater(p, x, y);
  } // end else fixed repeater
  if (_verbose) printTree(p, 0);
}

void BufrFile::_verbosePrintNode(unsigned short des) {
  unsigned char f, x, y;
  if (_verbose) {
    if ((des != 7878)  && (des !=7681)) {
      TableMapKey().Decode(des, &f, &x, &y);
      printf("visiting f(x,y): %1d(%02d,%03d) ", (unsigned int) f,
             (unsigned int) x, (unsigned int) y);  
    }     
  }
}

// Print the tree while we are traversing, since this is 
// the best way to display the values along with the the descriptors.
// The values are stored in a separate structure.
int BufrFile::_descend(DNode *tree) {

  if (_very_verbose) _verbosePrintTree(tree);

  unsigned short des = 0;
  DNode *p;
  p = tree;
  bool compressionStart = false;

  unsigned char f,x,y;

  try {
    // for each descriptor in the list
    while (p != NULL ) {

      if (_very_verbose) { 
        printf("\np != NULL ... \n");
        printTree(p,0);
      }

      des = p->des;
      TableMapKey key(des);
      TableMapKey().Decode(des, &f, &x, &y);
      // visit the node

      //_verbosePrintNode(des);

      if (key.isTableBEntry()) {  // a leaf
        _visitTableBNode(p, &compressionStart);
	p = p->next;
      } else if (key.isTableCEntry()) {  // a leaf
        _visitTableCNode(p);
	p = p->next;
      } else if (key.isReplicator()) {
        _visitReplicatorNode(p);
        if (0) {
        // for debugging
          printf(" current state of tree; p = %zu\n", (size_t) p);
        printTree(p,0);
        printf(" end current state of tree\n");
        // end for debugging
        }
        p=p->next;
      } else if (key.isAnotherNode()) {
        _visitTableDNode(p);
      } else {
        Radx::addErrStr(_errString, "", "ERROR - BufrFile::_descend", true);
	Radx::addErrInt(_errString, "   unrecognized table map key: ", des, true);
	throw _errString.c_str();
      }
    } // end while p!= NULL
    if (compressionStart) {
      currentTemplate->createSweep();
      compressionStart = false;
    }
  } catch (const std::out_of_range& e) {
    Radx::addErrStr(_errString, "", "ERROR - BufrFile::_descend", true);
    char desString[200];
    sprintf(desString, "unknown descriptor: (%u;%u;%u) ", f,x,y);
    Radx::addErrInt(_errString, desString, des, true);
    cerr << _errString;
    throw _errString.c_str();
  } catch (const char *msg) {
    Radx::addErrStr(_errString, "", "ERROR - BufrFile::_descend", true);
    Radx::addErrStr(_errString, "  ", msg, true);
    cerr << _errString;
    throw _errString.c_str();
  }
  // printf("Leaving _descend\n");
  return 0;
}


/////////////////////////////////////
// Print contents of Bufr file read
// Returns 0 on success, -1 on failure
int BufrFile::print(ostream &out, bool printRays, bool printData) {

  out << "=============== BufrFile ===============" << endl;
  out << "Section 0: " << endl;
  printSection0(out);
  out << "Section 1: " << endl;
  printSection1(out);
  out << "Section 2: " << endl;
  printSection2(out);
  out << "Section 3: " << endl;
  printSection3(out);
  //printTree();
  out << "Section 4: " << endl;
  if (printData) 
    printSection4(out);
  out << "=============== end of BufrFile =========" << endl;
  return 0;
}


