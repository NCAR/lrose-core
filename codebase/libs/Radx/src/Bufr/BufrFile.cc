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
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2017
//
///////////////////////////////////////////////////////////////

#include <Radx/BufrFile.hh>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <errno.h>
#include <cmath>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
#include <Radx/TableMapKey.hh>
//#include <Radx/TableMap.hh>

using namespace std;

//////////////
// Constructor

BufrFile::BufrFile()
  
{
  _debug = false;
  _verbose = false;
  _file = NULL;
  GTree = NULL;
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
  //GTree = NULL;
  _descriptorsToProcess.clear();
  nOctetsRead = 0;
}


// go ahead and read all the data from the file
// completely fill currentProduct with data
void BufrFile::readThatField(string fileName,
             string filePath,
             time_t fileTime,
             string fieldName,
             string standardName,
             string longName,
             string units,
             bool debug,
		   bool verbose) {

  clear();
  _pathInUse = filePath;
  try {
    openRead(_pathInUse); // path);
    readSection0();
    readSection1();
    readDataDescriptors();
    readData(); 
    close();
  } catch (const char *msg) {
    //_addErrStr(msg);
    throw _errString.c_str();
    //return -1;
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
  _file = fopen(path.c_str(), "r");
  
  if (_file == NULL) {
    int errNum = errno;
    //string errStr;
    Radx::addErrStr(_errString, "", "ERROR - BufrFile::_openRead", true);
    Radx::addErrStr(_errString, "  Cannot open file for reading, path: ", path, true);
    Radx::addErrStr(_errString, "  ", strerror(errNum), true);
    throw _errString.c_str();
  }
  currentProduct.reset(); 
  return 0;
}

///////////////////////////////////////////
// read Section 0
// which positively identifies this as a BUFR file
// Returns 0 on success, -1 on failure
// Side effect: 

int BufrFile::readSection0()
{

  /******* decode section 0 */
  fprintf (stderr, "Input file header:\n");

  // read through file, finding data blocks

    // read ID
    
    char id[8];
    memset(id, 0, 8);
    
    if (fread(id, 1, 4, _file) != 4) {
      //if (feof(_file)) {
        // done
      //  break;
      //}
      /*
      int errNum = errno;
      _addErrStr("ERROR - BufrFile::_readSection0");
      _addErrStr("  Cannot read id string");
      _addErrStr("  File path: ", path);
      _addErrStr(strerror(errNum));
      */
      close();
      return -1;
    }

    _numBytesRead += 4;

    string idStr(id);
    if ((id[0] != 'B') || (id[1] != 'U') || (id[2] != 'F') || (id[3] != 'R')) {
      throw "Not a BUFR file";
    }
    
    // read length of message in octets
    // the length is 24 bits (3 bytes)
    Radx::ui32 nBytes;
    unsigned char id2[4];
    //id[0] = 0;
    if (fread(id2, 1, 3, _file) != 3) {
      //int errNum = errno;
      //      _addErrStr("ERROR - DoradeRadxFile::_readSection0()");
      //_addErrStr("  Cannot read data length");
      //_addErrStr("  ID: ", id);
      //_addErrStr("  File path: ", path);
      //_addErrStr(strerror(errNum));
      close();
      return -1;
    }

    _numBytesRead += 3;

    nBytes = 0;
    nBytes = nBytes | id2[2];
    nBytes = nBytes | (id2[1] << 8);
    nBytes = nBytes | (id2[0] << 16);

    cerr << "nBytes " << nBytes << endl;
    _s0.nBytes = nBytes;

    Radx::ui08 bufr_edition;

    if (fread(&bufr_edition, 1, 1, _file) != 1) {
      //int errNum = errno;
      //      _addErrStr("ERROR - DoradeRadxFile::_readSection0()");
      //_addErrStr("  Cannot read data length");
      //_addErrStr("  ID: ", id);
      //_addErrStr("  File path: ", path);
      //_addErrStr(strerror(errNum));
      close();
      return -1;
    }
    _s0.edition = bufr_edition;   
    printf("BUFR edition number %d\n", bufr_edition); 

    _numBytesRead += 1;
    // swap as needed

    //-------------------

    //}
  /*
  //dim = _ncFile->get_dim(name.c_str());
  if (!_file->bufr_decode_sections01(&s1, &msg)) {
    //fprintf (stderr, "FATAL: Unable to decode section 1\n");
    //exit (EXIT_FAILURE);
    _addErrStr("ERROR - BufrFile::readDim");
    _addErrStr("  Cannot read dimension, name: ", name);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  }
  header_dump(&s1);
  */
  return 0;
}
///////////////////////////////////////////
// read Section 1
// which gives time and table info
// Returns 0 on success, -1 on failure
// Side effect: 

int BufrFile::readSection1()
{

  /******* decode section 1 */
  fprintf (stderr, "Input file header:\n");


    // read ID
    // TODO: allocate this space based on the length of section read (octets 1 - 3)
    Radx::ui08 id[4];
    memset(id, 0, 4);
    
    // read length of message in octets
    // the length is 24 bits (3 bytes)
    Radx::ui32 nBytes;
    if (fread(id, 1, 3, _file) != 3) {
      //int errNum = errno;
      //      _addErrStr("ERROR - DoradeRadxFile::_readSection0()");
      //_addErrStr("  Cannot read data length");
      //_addErrStr("  ID: ", id);
      //_addErrStr("  File path: ", path);
      //_addErrStr(strerror(errNum));
      close();
      return -1;
    }
    nBytes = 0;
    nBytes = nBytes | id[2];
    nBytes = nBytes | (id[1] << 8);
    nBytes = nBytes | (id[0] << 16);
    Radx::ui32 sectionLen;
    sectionLen = nBytes;

    cerr << "sectionLen " << sectionLen << endl;
    Radx::ui08 *buffer;
    buffer = (Radx::ui08 *) calloc(sectionLen, sizeof(Radx::ui08));
    memset(buffer, 0, sectionLen);

    if (fread(buffer, 1, sectionLen-3, _file) != sectionLen-3) {
      //int errNum = errno;
      //      _addErrStr("ERROR - DoradeRadxFile::_readSection0()");
      //_addErrStr("  Cannot read data length");
      //_addErrStr("  ID: ", id);
      //_addErrStr("  File path: ", path);
      //_addErrStr(strerror(errNum));
      close();
      return -1;
    }


    //Radx::ui08 bufrMasterTable;
    //bufrMasterTable = buffer[0];  // not sure if this is needed??
    // check 1st bit of the octect for optional section 2
    bool section2 = buffer[6]; //  & 0x80; 
    _s1.hasSection2 = section2;
    cerr << "section 2? " ;
    if (section2) {
      cerr << "yes" << endl;
    } else {
      cerr << "no" << endl;
    }
    Radx::ui16 yearOfCentury;
    Radx::ui08 month;
    Radx::ui08 day;
    Radx::ui08 hour;
    Radx::ui08 minute;
    Radx::ui08 seconds;


    if (_s0.edition >= 4) {
      _s1.masterTable = buffer[0];
      _s1.generatingCenter = buffer[1] << 8 | (buffer[2]);
      _s1.originatingSubcenter = buffer[3] << 8 | (buffer[4]);
      _s1.updateSequenceNumber = buffer[5]; // original BUFR message
      _s1.dataCategoryType = buffer[7];
      _s1.localTableVersionNumber = buffer[11];
      _s1.masterTableVersionNumber = buffer[10];

      printf("master table: %d\n", _s1.masterTable);
      printf("generatingc center: %d\n", _s1.generatingCenter);
      printf("originating subcenter: %d\n", _s1.originatingSubcenter);
      printf("update sequence number: %d\n", _s1.updateSequenceNumber);
      printf("data category type: %d\n", _s1.dataCategoryType);
      printf("local table version: %d\n", _s1.localTableVersionNumber);
      printf("master table version: %d\n", _s1.masterTableVersionNumber);

      yearOfCentury = 0;
      yearOfCentury = yearOfCentury |  buffer[13];
      yearOfCentury = yearOfCentury | (buffer[12] << 8);
      month = buffer[14] | 0;
      day = buffer[15] | 0;
      hour = buffer[16] | 0;
      minute = buffer[17] | 0;
      seconds = buffer[18] | 0;

    }   
    // TODO: figure out _debug ... try RadxFile::copyReadDirectives .. if (_debug) {
      cerr << "year-month-day hour:minute:sec " <<
        yearOfCentury << "-" << 
        month  << "-" << 
        day  << " " << 
        hour  << ":" << 
        minute <<  ":" <<
        seconds << endl;
      //}

      hdr_year = yearOfCentury;
      hdr_month = month;
      hdr_day = day;
      printf("year-month-day hour:minute:sec\n%d-%d-%d %d:%d:%d\n",
	     yearOfCentury, month, day, hour, minute, seconds); 
    
      _numBytesRead += sectionLen;
  return 0;
}

// taken from bufr2hdf5.c -- read_tables(...)
int BufrFile::readDescriptorTables() {

  return 0;
}



int BufrFile::readDataDescriptors() {  // read section 3
  /******* decode section 3 */
  fprintf (stderr, "Reading section 3 ...\n");

    // read 
    // TODO: allocate this space based on the length of section read (octets 1 - 3)
    Radx::ui08 id[4];
    memset(id, 0, 4);
    
    // read length of message in octets
    // the length is 24 bits (3 bytes)
    Radx::ui32 nBytes;
    if (fread(id, 1, 3, _file) != 3) {
      //int errNum = errno;
      //      _addErrStr("ERROR - DoradeRadxFile::_readSection0()");
      //_addErrStr("  Cannot read data length");
      //_addErrStr("  ID: ", id);
      //_addErrStr("  File path: ", path);
      //_addErrStr(strerror(errNum));
      close();
      return -1;
    }
    nBytes = 0;
    nBytes = nBytes | id[2];
    nBytes = nBytes | (id[1] << 8);
    nBytes = nBytes | (id[0] << 16);
    Radx::ui32 sectionLen;
    sectionLen = nBytes;

    cerr << "sectionLen " << sectionLen << endl;

    Radx::ui08 *buffer;
    buffer = (Radx::ui08 *) calloc(sectionLen, sizeof(Radx::ui08));
    memset(buffer, 0, sectionLen);
    int nBytesToSectionEnd;
    nBytesToSectionEnd = sectionLen-3;
    if (fread(buffer, 1, nBytesToSectionEnd, _file) != sectionLen-3) {
      //int errNum = errno;
      //      _addErrStr("ERROR - DoradeRadxFile::_readSection0()");
      //_addErrStr("  Cannot read data length");
      //_addErrStr("  ID: ", id);
      //_addErrStr("  File path: ", path);
      //_addErrStr(strerror(errNum));
      close();
      return -1;
    }

    // octet 4 is reserved and set to zero
    // get the number of data subsets/descriptors
    Radx::ui16 nDataSubsets;
    nDataSubsets = 0;
    nDataSubsets = nDataSubsets | buffer[2];
    nDataSubsets = nDataSubsets | (buffer[1] << 8);
    printf("nDataSubsets = %d\n", nDataSubsets);

    // determine observed and compressed info
    //bool observedData = buffer[3] & 0x80;
    //bool compressedData = buffer[3] & 0x40;

    // read the descriptors and keep them in a list for
    // traversal later 

    //_descriptorsToProcess.clear();
    //int nDescriptors;
    //nDescriptors = sectionLen 
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
      printf("f x y: %d %d %d\n", d.f, d.x, d.y);
      unsigned short key;
      key = TableMapKey().EncodeKey(d.f, d.x, d.y);
      _descriptorsToProcess.insert(_descriptorsToProcess.begin(), key);
    }

    _numBytesRead += sectionLen;
  return 0;
}

// necessary for entry into this function:
// the file pointer must be positioned at the start of 
// section 4.
int BufrFile::readData() {  // read section 4

  // prime the pump
  currentBufferLengthBytes = ReplenishBuffer();
  currentBufferLengthBits = currentBufferLengthBytes * 8;
  currentBufferIndexBits = 0;
  if (currentBufferLengthBits <= 0) {
    return -1;
  }

  try {
    if (!tableMap.filled())
      tableMap.ImportTables();
  } catch (const char *msg) {
    Radx::addErrStr(_errString, "ERROR - ", msg, true);
    throw _errString.c_str();
  }
  TraverseNew(_descriptorsToProcess);

  //TODO:  read the last section 
  //int nbytesRead;
  //nbytesRead = ReplenishBuffer();
  
  return 0;
}

// return the next nbits in the read buffer encased in a string
// there can be leading and trailing bits that are not needed
// returns 0 on success; -1 on failure
// int  BufrFile::NextNBitsEncased(int nbits, string ecasedBits, int *startIndex) {
//   string s; 
//   s = newString();

//   return 0;
// }

// need to buffer the binary data, so that we can read in a block
// of data and move a pointer within the block to retrieve bits
//char *BufrFile::NextNBitsAsChar(int nbits) {
  // remember, the character strings are encoded ins CCITT International Alphabet No. 5
//  return NULL;
//}

int BufrFile::ReplenishBuffer() {

  if (_firstBufferReplenish) { // (first) {
    fread(_dataBuffer, 1, 4, _file);
    _firstBufferReplenish = false;
  }
  int nBytesRead;
  nBytesRead = fread(_dataBuffer, 1, MAX_BUFFER_SIZE_BYTES, _file);

  nOctetsRead += nBytesRead;
  if (_debug) printf("nOctetsRead = %d\n", nOctetsRead);

  _numBytesRead += nBytesRead;
  if (_debug) printf("Read %d/%d bytes ", _numBytesRead, _s0.nBytes);

  if (_verbose) {
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


// on error, return -1; otherwise, a positive integer will be returned
string BufrFile::ExtractText(int nBits) {

  string val;
  // unsigned char mask;
  unsigned char character;

  val.clear();

  // if the number of bits is not a multiple of 8, then
  // we cannot extract as text
  if ((nBits % 8) != 0)  {
    cout << "ERROR: text width is not multiple of 8; cannot read in data" << endl;
    return val;
  }

  int i=0;
  bool endOfFile = false;
  //int bitPosition;
  // move one bit at a time
  while ((i<nBits) && (!endOfFile)) {
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
      val.push_back(character);
      character = 0;
    }
  }

  if ((endOfFile) && (i < nBits)) {
    val.clear();
    return val;  // ran out of data before completed the value
  }  else {
    //cout << "extracted " << val << endl;
    return val;
  }
}


// on error, return -1; otherwise, a positive integer will be returned
Radx::ui32 BufrFile::ExtractIt(int nBits) {

  Radx::ui32 val;
  //int idx;
  //unsigned char mask;

  if (_debug) {
    printf(" nBits=%d\n", nBits);
  }

  if (nBits > 32) {
    // TODO: for now ... just skip it
    //return -1;
    //SkipIt(nBits);
    //return 0;
    exit(1); // TODO throw exception
  }
  val = 0;
  int i=0;
  bool endOfFile = false;
  //int bitPosition;
  while ((i<nBits) && (!endOfFile)) {
    if (NextBit()) {
      // insert a 1
      val = val * 2 + 1;
    } else {
      val = val * 2;
    }
    i += 1;
  }

  if ((endOfFile) && (i < nBits)) {
    return -1;  // ran out of data before completed the value
  }  else {
    //cout << "extracted " << val << endl;
    return val;
  }
}

// TODO: not working ... need to fix it
void BufrFile::SkipIt(int nBits) {
  //int val;
  //int idx;
  //unsigned char mask;

  printf("entered non working code; exiting. \n");
  exit(1);
}

Radx::ui32 BufrFile::Apply(TableMapElement f) {

  if (f._whichType != TableMapElement::DESCRIPTOR) {
    return -1;
  }
  if (_debug) {
    cout << "Applying " << endl;
    cout << "  " << f._descriptor.fieldName << " ";

    for (unsigned int i=0; i<50-f._descriptor.fieldName.size(); i++)
      cout << "-";
    cout << " " << f._descriptor.dataWidthBits << endl;
    cout << " scale  " << f._descriptor.scale << endl;
    cout << " units  " << f._descriptor.units << endl;;
    cout << " reference value " << f._descriptor.referenceValue << endl;
  }
  if (f._descriptor.units.find("CCITT") != string::npos) {
    string  value;
    value = ExtractText(f._descriptor.dataWidthBits);
    cout << " " << f._descriptor.dataWidthBits << endl;
    cout << "extracted string = " << value << endl;
    string fieldName;
    fieldName = f._descriptor.fieldName;
    std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);

    if (fieldName.find("station identifier") != string::npos) {
      if (fieldName.find("type of") != string::npos) {
        typeOfStationId = value;
      } else {
        stationId = value;
      }
    }
    return 0;
  } else {
    Radx::ui32 value;
    value = ExtractIt(f._descriptor.dataWidthBits);
    return value;
  }
}

//
// 10^(-10) to 10^10  ==> need 21 elements
//  -3 -2 -1 0 1 2 3
double BufrFile::fastPow10(int n)
{
  static double pow10[21] = {
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

Radx::si32 BufrFile::ApplyNumeric(TableMapElement f) {

  if (f._whichType != TableMapElement::DESCRIPTOR) {
    return -1;
  } 
  if (_debug) {
    if (f._descriptor.fieldName.find("Byte element") == string::npos) {
    cout << "Applying " << endl;
    cout << "  " << f._descriptor.fieldName << " ";

    for (unsigned int i=0; i<50-f._descriptor.fieldName.size(); i++)
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
    value = ExtractIt(f._descriptor.dataWidthBits);
    Radx::si32 svalue;
    double temp;
    //svalue = (value+f._descriptor.referenceValue)/fastPow10(f._descriptor.scale);
    temp = f._descriptor.referenceValue;
    temp = value + temp;
    temp  = temp/fastPow10(f._descriptor.scale);

    svalue = (Radx::si32) temp;
    if (_debug) cout << "converted to " << svalue << endl;
    return svalue;
  }
}

Radx::fl32 BufrFile::ApplyNumericFloat(TableMapElement f) {

  if (f._whichType != TableMapElement::DESCRIPTOR) {
    return -1;
  } 
  if (_debug) {
    if (f._descriptor.fieldName.find("Byte element") == string::npos) {
    cout << "Applying " << endl;
    cout << "  " << f._descriptor.fieldName << " ";

    for (unsigned int i=0; i<50-f._descriptor.fieldName.size(); i++)
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
    value = ExtractIt(f._descriptor.dataWidthBits);
    Radx::fl32 svalue;
    double temp;
    //svalue = (value+f._descriptor.referenceValue)/fastPow10(f._descriptor.scale);
    temp = f._descriptor.referenceValue;
    temp = value + temp;
    temp  = temp/fastPow10(f._descriptor.scale);

    svalue = (Radx::fl32) temp;
    if (_debug) cout << "converted to " << svalue << endl;
    return svalue;
  }
}

// Put the info in the correct storage location
// and take care of any setup that needs to happen
bool BufrFile::StuffIt(string fieldName, double value) {
  bool ok = true;

  std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);
  if (fieldName.find("byte element") != string::npos) {
    // TODO: ugh!  It's not a double value, it's an unsigned char...
    currentProduct.addData((unsigned char) value);
  } else if (fieldName.find("latitude") != string::npos) {
    latitude = value;
  } else if (fieldName.find("longitude") != string::npos) {
    longitude = value;
  } else if (fieldName.find("height") != string::npos) {
    height = value;
  } else if (fieldName.find("antenna elevation") != string::npos) {
    currentProduct.antennaElevationDegrees = value;
  } else if (fieldName.find("number of bins along the radial") != string::npos) {
    currentProduct.nBinsAlongTheRadial = value;
  } else if (fieldName.find("range-bin size") != string::npos) {
    currentProduct.rangeBinSizeMeters = value;
  } else if (fieldName.find("range-bin offset") != string::npos) {
    currentProduct.rangeBinOffsetMeters = value;
  } else if (fieldName.find("number of azimuths") != string::npos) {
    currentProduct.nAzimuths = value;
  } else if (fieldName.find("antenna beam azimuth") != string::npos) {
    currentProduct.antennaBeamAzimuthDegrees = value;

  } else if (fieldName.find("year") != string::npos) {
    currentProduct.putYear(value);
  } else if (fieldName.find("month") != string::npos) {
    currentProduct.putMonth(value);
  } else if (fieldName.find("day") != string::npos) {
    currentProduct.putDay(value);
  } else if (fieldName.find("hour") != string::npos) {
    currentProduct.putHour(value);
  } else if (fieldName.find("minute") != string::npos) {
    currentProduct.putMinute(value);
  } else if (fieldName.find("second") != string::npos) {
    currentProduct.putSecond(value);
    // TODO: should probably use key to identify these fields...
  } else if (fieldName.find("wmo block") != string::npos) {
    WMOBlockNumber = value;
  } else if (fieldName.find("wmo station") != string::npos) {
    WMOStationNumber = value;

  
  } else if (fieldName.find("type of product") != string::npos) {
    int code = (int) value;
    // TODO: make sure the type of product agrees with the field name
    switch(code) {
    case 0:
      currentProduct.typeOfProduct = "DBZH";
      break;
    case 40:
      currentProduct.typeOfProduct = "VRAD";
      break;
    case 91: 
    case 230:
      currentProduct.typeOfProduct = "TH";
      break;
    case 92:
    case 60:
      currentProduct.typeOfProduct = "WRAD";
      break;	  
    case 243:
      currentProduct.typeOfProduct = "CM";
      break;
    case 240:
      currentProduct.typeOfProduct = "KDP";
      break;
    case 239:
      currentProduct.typeOfProduct = "PHIDP";
      break;
    case 241:
      currentProduct.typeOfProduct = "RHOHV";
      break;
    case 231:
      currentProduct.typeOfProduct = "TV";
      break; 
    default:
      currentProduct.typeOfProduct = "XXXX";
      ok = false;
    } 
  } else {
    ok = false;
  }
  return ok;
}

size_t BufrFile::getTimeDimension() {
  return currentProduct.nAzimuths;
}

size_t BufrFile::getRangeDimension() {
  return currentProduct.nBinsAlongTheRadial;
}

size_t BufrFile::getNumberOfSweeps() {
  return currentProduct.sweepData.size();
}

size_t BufrFile::getNBinsAlongTheRadial() {
  return currentProduct.nBinsAlongTheRadial;
}

double BufrFile::getRangeBinOffsetMeters() {
  return currentProduct.rangeBinOffsetMeters;
}

double BufrFile::getRangeBinSizeMeters() {
  return currentProduct.rangeBinSizeMeters;
}

double BufrFile::getElevationForSweep(int sweepNumber) {
  return currentProduct.sweepData.at(sweepNumber).antennaElevationDegrees;
}

int BufrFile::getNAzimuthsForSweep(int sweepNumber) {
  return currentProduct.sweepData.at(sweepNumber).nAzimuths;
}

double BufrFile::getStartingAzimuthForSweep(int sweepNumber) {
  return currentProduct.sweepData.at(sweepNumber).antennaBeamAzimuthDegrees;
}

double BufrFile::getStartTimeForSweep(int sweepNumber) {
  BufrProduct::TimeStamp t;
  t = currentProduct.sweepData.at(sweepNumber).startTime;
  RadxTime rTime(t.year, t.month, t.day, t.hour, t.minute, t.second); 
  return rTime.asDouble();
}

double BufrFile::getEndTimeForSweep(int sweepNumber) {
  BufrProduct::TimeStamp t;
  t = currentProduct.sweepData.at(sweepNumber).endTime;
  RadxTime rTime(t.year, t.month, t.day, t.hour, t.minute, t.second); 
  return rTime.asDouble();
}

double *BufrFile::getDataForSweep(int sweepNumber) { 
  return currentProduct.sweepData.at(sweepNumber).parameterData[0].data;
}

string BufrFile::getTypeOfProductForSweep(int sweepNumber) { 
  return currentProduct.sweepData.at(sweepNumber).parameterData[0].typeOfProduct;
}

/*
// traverse the trees from section 3.  These are the decoders
// of the data in section 4
// need knowledge of BUFR tables, pointer to data file ( _file )
int BufrFile::TraverseOriginal(vector<unsigned short> descriptors) {



  bool _debug = true;  // TODO: make this more global

  if (_debug) 
    printf("\nTraversing ... \n");

  unsigned short des;
  try {
  // for each descriptor in the list
  while (!descriptors.empty()) {  // NOTE: the descriptor list is changing

    unsigned char f, x, y;

    des = descriptors.back();
    TableMapKey key(des);
    descriptors.pop_back();
    
    // visit the node
    if (_debug) {
      TableMapKey().Decode(des, &f, &x, &y);
      printf("visiting f(x,y): %x(%x,%x) ", f, x, y);        
    }

    if (key.isTableBEntry()) {  // a leaf
      if (_debug) 
        printf(" leaf\n");
      // if the node is from table b, retrieve the data; apply any transformations;
      //   insert into temporary structure for saving
      TableMapElement val1;
      val1 = tableMap.Retrieve(des);
      if (val1._whichType == TableMapElement::DESCRIPTOR) {
        cout << "value for key " << des << ":" <<
          val1._descriptor.fieldName << "," << 
          val1._descriptor.scale << endl;
      }
      Radx::ui32 valueFromData; 
      valueFromData = Apply(val1);
      // cout << "value from Data = " << valueFromData << endl;

    } else if (key.isReplicator()) {
	if (_debug) 
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
          delayed_replication_descriptor = descriptors.back();
          descriptors.pop_back();
          // get the number of repeats from section 4 data
          unsigned char nrepeats; // actually read this from the data section
          Radx::ui32 valueFromData; 
          valueFromData = Apply(tableMap.Retrieve(delayed_replication_descriptor));
          cout << "nrepeats from Data = " << valueFromData << endl;
          nrepeats = (unsigned char) valueFromData; // TODO: check if value exceeds max for y values     
          if (nrepeats > 0) {
            // construct a new descriptor with the number of repeats
            unsigned short new_descriptor;
            new_descriptor = TableMapKey().EncodeKey(f, x, nrepeats);
            // push the new fixed repeater back on the stack
	    descriptors.push_back(new_descriptor);
	  } else {
	    // remove the items that would have been traversed, since there are no repeats
            for (int i=0; i<x; i++)
              descriptors.pop_back();
          }
	} else { 
          // must be a fixed repeater
          // pop the next x descriptors and apply them
	  vector<unsigned short> theNextList;
          for (int i=0; i<x; i++) {
            unsigned short embedded_des;
            embedded_des = descriptors.back();
            //TableMapKey key(des);
            descriptors.pop_back();
	    theNextList.insert(theNextList.begin(), embedded_des);
	  }
	  // if there are more repeats, decrement the count and push 
          // everything back on the stack with a decremented repeater
          // 1st attempt, use recursion
          for (int i=0; i<y; i++) {
            printf("%d out of %d repeats\n", i+1, y);
            printf("  working with list: "); 
	    for (vector<unsigned short>::iterator i = descriptors.begin(); i!= descriptors.end(); ++i)
	      cout << *i << ' ';
            TraverseOriginal(theNextList);
	  }
	}
        

    } else if (key.isAnotherNode()) {
	if (_debug) 
	  printf(" another node\n");

	// if the node is another node; recur
	// look up the expansions and insert them...
	//TableMapElement val1 = tableMap.Retrieve(des);
        TableMapElement val1;
        val1 = tableMap.Retrieve(des);
	 
      
	//descriptors.push_back();
	vector<unsigned short> theList;
	theList = val1._listOfKeys; 
	//cout << "value for key " << key << ": " << theList.size() << endl; 
	for (vector<unsigned short>::reverse_iterator i = theList.rbegin(); i!= theList.rend(); ++i)
	  descriptors.push_back(*i);
	//cout << *i << ' ';

    } else {
	if (_debug) 
	  printf(" unknown\n");
        return -1; // error occurred
    }
  }
  } catch (const std::out_of_range& e) {
    cout << "Error: unknown descriptor: " << des << endl;
    exit(1);
  }
  return 0;
}
*/

/*
// traverse the trees from section 3.  These are the decoders
// of the data in section 4
// need knowledge of BUFR tables, pointer to data file ( _file )
// returns a list of any modifications to the repeat factors
// we are going to access the global list of descriptors (_descriptorsToProcess)
int BufrFile::Traverse(int start, int length) {

  bool _debug = true;  // TODO: make this more global

  if (_debug) 
    printf("\nTraversing ... \n");

  unsigned short des;
  int pos, len;
  pos = start;
  len = length;
  try {
  // for each descriptor in the list
    while (!_descriptorsToProcess.empty() ) {  // NOTE: the descriptor list is changing

    unsigned char f, x, y;

    des = _descriptorsToProcess.at(pos); // descriptors.back();
    TableMapKey key(des);

    
    // visit the node
    if (_debug) {
      TableMapKey().Decode(des, &f, &x, &y);
      printf("visiting f(x,y): %x(%x,%x) ", f, x, y);        
    }

    if (key.isTableBEntry()) {  // a leaf
      if (_debug) 
        printf(" leaf\n");
      // if the node is from table b, retrieve the data; apply any transformations;
      //   insert into temporary structure for saving
      TableMapElement val1;
      val1 = tableMap.Retrieve(des);
      if (val1._whichType == TableMapElement::DESCRIPTOR) {
        cout << "value for key " << des << ":" <<
          val1._descriptor.fieldName << "," << 
          val1._descriptor.scale << endl;
      }
      Radx::ui32 valueFromData; 
      valueFromData = Apply(val1);
      // cout << "value from Data = " << valueFromData << endl;
      pos += 1;

    } else if (key.isReplicator()) {
	if (_debug) 
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
          delayed_replication_descriptor = _descriptorsToProcess.back();
          _descriptorsToProcess.pop_back();
          // get the number of repeats from section 4 data
          unsigned char nrepeats; // actually read this from the data section
          Radx::ui32 valueFromData; 
          valueFromData = Apply(tableMap.Retrieve(delayed_replication_descriptor));
          cout << "nrepeats from Data = " << valueFromData << endl;
          nrepeats = (unsigned char) valueFromData; // TODO: check if value exceeds max for y values     
          if (nrepeats > 0) {
            // construct a new descriptor with the number of repeats
            unsigned short new_descriptor;
            new_descriptor = TableMapKey().EncodeKey(f, x, nrepeats);
            // push the new fixed repeater back on the stack
	    _descriptorsToProcess.push_back(new_descriptor);
	  } else {
	    // remove the items that would have been traversed, since there are no repeats
            for (int i=0; i<x; i++)
              _descriptorsToProcess.pop_back();
          }
	} else { 
          // must be a fixed repeater
          // pop the next x descriptors and apply them -- NOPE!
          // just record the length and setup a loop the start and end point
	  //vector<unsigned short> theNextList;
          //for (int i=0; i<x; i++) {
          //  unsigned short embedded_des;
          //  embedded_des = _descriptorsToProcess.back();
          //  //TableMapKey key(des);
          //  //descriptors.pop_back();
	  //  //theNextList.insert(theNextList.begin(), embedded_des);
	  //}
	  // if there are more repeats, decrement the count and push 
          // everything back on the stack with a decremented repeater
          // 1st attempt, use recursion

          //vector<unsigned short> anyMods;
          //printf("1 out of %d repeats\n", y);
          //anyMods = Traverse(start, length); // theNextList);
          for (int i=0; i<y; i++) {
            printf("%d out of %d repeats\n", i+1, y);
            //printf("  anyMods: "); 
	    //for (vector<unsigned short>::iterator i = anyMods.begin(); i!= anyMods.end(); ++i)
	    //  cout << *i << ' ';
            Traverse(pos, x); // (anyMods);
	  }
	}
        

    } else if (key.isAnotherNode()) {
	if (_debug) 
	  printf(" another node\n");
	// pop/remove the element from the list; it is being replaced
        //_descriptorsToProcess.pop_back();
	// if the node is another node,
	// look up the expansions and insert them...
	//TableMapElement val1 = tableMap.Retrieve(des);
        TableMapElement val1;
        val1 = tableMap.Retrieve(des);
	 
      
	//descriptors.push_back();
	vector<unsigned short> theList;
	theList = val1._listOfKeys;
        int nProcessed;
        nProcessed = pos - start; 
        // insert after the pos
        _descriptorsToProcess.insert(pos+1, theList);
        // now, remove the node that was expanded
        _descriptorsToProcess.erase(pos);
	len += theList.size() - 1;
        pos = start + nProcessed;
	//cout << "value for key " << key << ": " << theList.size() << endl; 
	//for (vector<unsigned short>::reverse_iterator i = theList.rbegin(); i!= theList.rend(); ++i)
	// _descriptorsToProcess.push_back(*i);
	//cout << *i << ' ';

    } else {
	if (_debug) 
	  printf(" ERROR!!! unknown\n");  // TODO: this should be an exception thrown
        return -1; // error occurred
    }
  }
  } catch (const std::out_of_range& e) {
    cout << "Error: unknown descriptor: " << des << endl;
    exit(1);
  }
  return 0;
}
*/



void BufrFile::_deleteAfter(DNode *p) {
  DNode *q;
  if (p!=NULL) {
    q = p->next;
    p->next = q->next;
    free(q);
  }
}

//  Node *      _buildTree(vector<unsigned short> descriptors);
BufrFile::DNode* BufrFile::buildTree(vector<unsigned short> descriptors, bool reverse) {
  DNode *t;
  t = NULL;
  if (reverse) {
  for (vector<unsigned short>::reverse_iterator i = descriptors.rbegin(); i!= descriptors.rend(); ++i) {
    DNode *p = (DNode *) malloc(sizeof(DNode));
    p->des = *i;
    //p->nRepeats = 0;
    unsigned short new_descriptor;
    new_descriptor = TableMapKey().EncodeKey(0, 0, 0);
    p->delayed_repeater = new_descriptor;
    p->children = NULL;
    p->next = t;
    t = p;
  }
  } else {
  for (vector<unsigned short>::iterator i = descriptors.begin(); i!= descriptors.end(); ++i) {
    DNode *p = (DNode *) malloc(sizeof(DNode));
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
    for (int i=0; i<level; i++) printf(" ");
    printf("+(%d) delayed_rep %u\n", p->des, p->delayed_repeater);
    q=p->children;
    if (q != NULL) {
      printTree(q, level+1);
    }
    p=p->next;
  }
}

void BufrFile::freeTree(DNode *tree) {
  DNode *p,*q;
  p = tree;
  while (p!=NULL) {
    // free the children
    q=p->children;
    if (q != NULL) {
      freeTree(q);
    }
    DNode *temp;
    temp = p;
    p=p->next;
    if (_debug) printf("freeing %d\n", temp->des);
    free(temp);
  }
}

// traverse the trees from section 3.  These are the decoders
// of the data in section 4
// need knowledge of BUFR tables, pointer to data file ( _file )
// returns a list of any modifications to the repeat factors
// we are going to access the global list of descriptors (_descriptorsToProcess)
int BufrFile::TraverseNew(vector<unsigned short> descriptors) {

  GTree = buildTree(descriptors, false);

  int result =   _descend(GTree);
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

int BufrFile::_descend(DNode *tree) {

  //bool _debug = true;  // TODO: make this more global

  if (_debug) { 
    printf("\nTraversing ... \n");
    printTree(tree,0);
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
    if (_debug) {
      TableMapKey().Decode(des, &f, &x, &y);
      printf("visiting f(x,y): %x(%x,%x) ", f, x, y);        
    }

    if (key.isTableBEntry()) {  // a leaf
      if (_debug) 
        printf(" leaf\n");
      // if the node is from table b, retrieve the data; apply any transformations;
      //   insert into temporary structure for saving
      TableMapElement val1;
      val1 = tableMap.Retrieve(des);
      if (val1._whichType == TableMapElement::DESCRIPTOR) {
        //cout << "value for key " << des << ":" <<
        //  val1._descriptor.fieldName << "," << 
        //  val1._descriptor.scale << endl;
	;
      }
      Radx::fl32 valueFromData;
      if (val1.IsText()) {
	// THE NEXT TWO LINES ARE CRUCIAL!! DO NOT REMOVE IT!!!
	//	Radx::ui32 junk;
	// junk = Apply(val1); // TODO: integrate a string type here; or toss exception? 
	// we don't care about the return value when the descriptor is text
	Apply(val1); 
        //; //cerr << "Apply text value not implemented" << endl;
      } else {
        valueFromData = ApplyNumericFloat(val1);
        if (!StuffIt(val1._descriptor.fieldName, valueFromData)) {
          // throw "Cannot associate value with BufrField";
          cout << "Cannot associate value with BufrField\n";
	}
        if (val1._descriptor.fieldName.find("Compression method") != string::npos) {
          compressionStart = true;
        }
      }
      p = p->next;

    } else if (key.isReplicator()) {
	if (_debug) 
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
          printf("nrepeats from Data = %u\n", nRepeats);
          //repeaters.push(nRepeats);
          currentProduct.storeReplicator(nRepeats);
          if (p->children == NULL) {
            moveChildren(p, x);
	  }
          // transition state; set location levels
          // the state determines which counters to increment & decrement
          // It's up to the product to deal with the space allocation as needed
          for (unsigned int i=0; i<nRepeats; i++) {
            if ((i%1000)==0)  
              printf("%d out of %d repeats\n", i+1, nRepeats);
            _descend(p->children);
	  }
          printf("-- end repeat\n");
          // transition state; set location levels
          currentProduct.trashReplicator();
          p=p->next;
	} else {           // must be a fixed repeater
          if (p->children == NULL) {
            moveChildren(p, x);
	  }
          for (int i=0; i<y; i++) {
            if ((i%1000)==0)
              printf("%d out of %d repeats\n", i+1, y);
            _descend(p->children);
	  }
          printf("-- end repeat\n");
          p=p->next;
	} // end else fixed repeater
        printTree(tree, 0);
    } else if (key.isAnotherNode()) {
	if (_debug) 
	  printf(" another node\n");
	// pop/remove the element from the list; it is being replaced
	// if the node is another node,
	// look up the expansions and insert them...
        TableMapElement val1;
        val1 = tableMap.Retrieve(des);
	 
	vector<unsigned short> theList;
	theList = val1._listOfKeys;

        // replace the contents of this node with the first element of the list
        p->des = theList.front();  // TODO: this could be the wrong order
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
        printTree(tree, 0);
    } else {
	if (_debug) 
	  printf(" ERROR!!! unknown\n");  // TODO: this should be an exception thrown
        return -1; // error occurred
    }
    } // end while p!= NULL
    if (compressionStart) {
      //double *realData;
      // realData = currentProduct.decompressData();
      // push info to RadxVol
      //pushToRadxVol(realData, currentProduct);
      currentProduct.createSweep();
      compressionStart = false;
    }
  } catch (const std::out_of_range& e) {
    cout << "Error: unknown descriptor: " << des << endl;
    exit(1);
  }
  return 0;
}

// TODO: or are we going to pull the info after we are done?
// If I can figure out when a sweep starts, then we can pull
//BufrFile::pushToRadxVol(realData, currentProduct);

/*

int BufrFile::_descend(DNode *tree) {

  bool _debug = true;  // TODO: make this more global

  if (_debug) { 
    printf("\nTraversing ... \n");
    printTree(tree,0);
  }

  unsigned short des;
  DNode *p;
  p = tree;

  try {
  // for each descriptor in the list
    while (p != NULL ) {

    unsigned char f, x, y;

    des = p->des;
    TableMapKey key(des);

    // visit the node
    if (_debug) {
      TableMapKey().Decode(des, &f, &x, &y);
      printf("visiting f(x,y): %x(%x,%x) ", f, x, y);        
    }

    if (key.isTableBEntry()) {  // a leaf
      if (_debug) 
        printf(" leaf\n");
      // if the node is from table b, retrieve the data; apply any transformations;
      //   insert into temporary structure for saving
      TableMapElement val1;
      val1 = tableMap.Retrieve(des);
      if (val1._whichType == TableMapElement::DESCRIPTOR) {
        cout << "value for key " << des << ":" <<
          val1._descriptor.fieldName << "," << 
          val1._descriptor.scale << endl;
      }
      Radx::ui32 valueFromData; 
      valueFromData = Apply(val1);
      // cout << "value from Data = " << valueFromData << endl;
      p = p->next;

    } else if (key.isReplicator()) {
	if (_debug) 
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
          DNode *delayed_rep_node;
          delayed_rep_node = p->next;
          delayed_replication_descriptor = delayed_rep_node->des;
          // get the number of repeats from section 4 data
          unsigned char nrepeats; // actually read this from the data section
          Radx::ui32 valueFromData; 
          valueFromData = Apply(tableMap.Retrieve(delayed_replication_descriptor));
          printf("nrepeats from Data = %u\n", valueFromData);
	  //>>>>> bad conversion!!!      
          nrepeats = (unsigned char) valueFromData; // TODO: check if value exceeds max for y values
          p->nRepeats = valueFromData; 
          // replace the contents of the replicator node with the number of repeats
          // construct a new descriptor with the number of repeats
          unsigned short new_descriptor;
          new_descriptor = TableMapKey().EncodeKey(f, x, nrepeats);
	  p->des = new_descriptor;
          // remove the delayed replication descriptor node from the list
          _deleteAfter(p);    
	} else {           // must be a fixed repeater
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
	  Radx::ui32 y32;
          y32 = y;
          if (p->nRepeats > 0) {
            y32 = p->nRepeats;  // because some repeats are larger than 8 bits
	  }
          for (int i=0; i<y32; i++) {
            printf("%d out of %d repeats\n", i+1, y32);
            _descend(p->children);
	  }
          printf("-- end repeat\n");
          p=p->next;
	} // end else fixed repeater
        printTree(tree, 0);
    } else if (key.isAnotherNode()) {
	if (_debug) 
	  printf(" another node\n");
	// pop/remove the element from the list; it is being replaced
	// if the node is another node,
	// look up the expansions and insert them...
	//TableMapElement val1 = tableMap.Retrieve(des);
        TableMapElement val1;
        val1 = tableMap.Retrieve(des);
	 
	vector<unsigned short> theList;
	theList = val1._listOfKeys;

        // replace the contents of this node with the first element of the list
        p->des = theList.front();  // TODO: this could be the wrong order
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
        printTree(tree, 0);
    } else {
	if (_debug) 
	  printf(" ERROR!!! unknown\n");  // TODO: this should be an exception thrown
        return -1; // error occurred
    }
  }
  } catch (const std::out_of_range& e) {
    cout << "Error: unknown descriptor: " << des << endl;
    exit(1);
  }
  return 0;
}

*/

/////////////////////////////////////
// Print contents of Bufr file read
// Returns 0 on success, -1 on failure
int BufrFile::Print() {

  return 0;
}


