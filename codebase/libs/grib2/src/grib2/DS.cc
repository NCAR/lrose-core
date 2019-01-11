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
/////////////////////////////////////////////
// Ds - Section 7 Data Section
//
////////////////////////////////////////////

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <grib2/DS.hh>
#include <grib2/DataRepTemp.hh>
#include <grib2/DRS.hh>


using namespace std;

namespace Grib2 {

DS::DS (Grib2Record::Grib2Sections_t sectionsPtr) :
  GribSection( )
{
  _sectionNum = 7;
  _sectionsPtr = sectionsPtr;
  _sectionsPtr.ds = this;
  _data_status = NONE;
  _dataTemp = NULL;
  _readDataPtr = NULL;
  _drsTemplateNum = _sectionsPtr.drs->getDrsConstants().templateNumber;

  switch (_drsTemplateNum) {
    case 1:     // Matrix values at a grid point - simple packing
      cerr << "ERROR: DS()" << endl;
      cerr << "Data TemplateNum " << _drsTemplateNum << 
	"(compression technique Matrix values) not implemented" << endl;
      break;

    case 4:	// Grid Point Data - IEEE Floating Point Data
      cerr << "ERROR: DS()" << endl;
      cerr << "Data TemplateNum " << _drsTemplateNum << 
	"(compression technique Grid Point IEEE Floating) not implemented" << endl;
      break;

    case 50:    // Spectral Simple
      cerr << "ERROR: DS()" << endl;
      cerr << "Data TemplateNum " << _drsTemplateNum << 
	"(compression technique Spectral Simple) not implemented" << endl;
      break;

    case 51:    // Spectral complex
      cerr << "ERROR: DS()" << endl;
      cerr << "Data TemplateNum " << _drsTemplateNum << 
	"(compression technique Spectral Complex) not implemented" << endl;
      break;

    case 61:	// Grid Point Data - Simple Packing With Logarithm Pre-processing
      cerr << "ERROR: DS()" << endl;
      cerr << "Data TemplateNum " << _drsTemplateNum << 
	"(compression technique Grid Point Simple Packing " <<
	"with Logarithm Pre-processing) not implemented" << endl;
      break;

    case 0:     // Grid point data - simple packing

      _dataTemp = new Template7_pt_0(_sectionsPtr);
      break;

    case 2:     // Grid point data - complex packing
    case 3:     // Grid point data - complex packing and spacial differencing 

      _dataTemp = new Template7_pt_2(_sectionsPtr);
      break;

    case 40:     // Jpeg 2000 packing
    case 4000:   // Jpeg 2000 packing

      _dataTemp = new Template7_pt_4000(_sectionsPtr);
       
      break;

    case 41:    // PNG packing 
    case 40010: // PNG packing 

      _dataTemp = new Template7_pt_41(_sectionsPtr);
       
      break;
    default:
      cerr << "ERROR: DS()" << endl;
      cerr << "Data TemplateNum (compression technique) - " << _drsTemplateNum << " not implemented" << endl;
      break;
  }

}

DS::~DS()
{
  if(_dataTemp != NULL)
    delete _dataTemp;
  if(_readDataPtr != NULL)
    delete[] _readDataPtr;
}

void DS::freeData()
{
  if(_dataTemp != NULL)
    _dataTemp->freeData();
}

int DS::unpack(ui08 *dsPtr)
{
  // Length of section in octets
  _sectionLen = _upkUnsigned4 (dsPtr[0], dsPtr[1], dsPtr[2], dsPtr[3]);

  // Number of section ("7")
  _sectionNum = (si32) dsPtr[4];

  if (_sectionNum != 7) {
     cerr << "ERROR: DS::unpack()" << endl;
     cerr << "Detecting incorrect section number, should be 6 but found section "
           << _sectionNum << endl;
    return GRIB_FAILURE;
  }

  if(_dataTemp == NULL)
    return GRIB_FAILURE;

  if(_readDataPtr != NULL)
    delete[] _readDataPtr;

  _readDataPtr = new ui08[_sectionLen - 4];
  memcpy(_readDataPtr, &(dsPtr[5]), _sectionLen -4);

  _data_status = READ;

  return GRIB_SUCCESS;
}

fl32 *DS::getData()
{
  ui08 *dataPtr = NULL;
  if(_data_status == READ) {
    dataPtr = _readDataPtr;
  } else if(_data_status == ENCODE || 
	    _data_status == PACK) {
    dataPtr = _dataTemp->getPackedData();
  } else if(_data_status == UNPACK) {
    return _dataTemp->getData();
  } else if(_data_status == NONE) {
    return NULL;
  }

  if(_dataTemp == NULL)
    return NULL;

  if(_dataTemp->unpack(dataPtr) == GRIB_FAILURE)
    return NULL;

  if(_data_status == READ) {
    delete[] _readDataPtr;
    _readDataPtr = NULL;
  }

  _data_status = UNPACK;

  return _dataTemp->getData();
}

int DS::encode(fl32 *dataPtr)
{
  if(_dataTemp == NULL)
    return GRIB_FAILURE;

  switch (_drsTemplateNum) {
    case 1:     // Matrix values at a grid point - simple packing
      cerr << "ERROR: DS()" << endl;
      cerr << "Data TemplateNum " << _drsTemplateNum << 
	"(compression technique Matrix values) not implemented" << endl;
      break;

    case 4:	// Grid Point Data - IEEE Floating Point Data
      cerr << "ERROR: DS()" << endl;
      cerr << "Data TemplateNum " << _drsTemplateNum << 
	"(compression technique Grid Point IEEE Floating) not implemented" << endl;
      break;

    case 50:    // Spectral Simple
      cerr << "ERROR: DS()" << endl;
      cerr << "Data TemplateNum " << _drsTemplateNum << 
	"(compression technique Spectral Simple) not implemented" << endl;
      break;

    case 51:    // Spectral complex
      cerr << "ERROR: DS()" << endl;
      cerr << "Data TemplateNum " << _drsTemplateNum << 
	"(compression technique Spectral Complex) not implemented" << endl;
      break;

    case 61:	// Grid Point Data - Simple Packing With Logarithm Pre-processing
      cerr << "ERROR: DS()" << endl;
      cerr << "Data TemplateNum " << _drsTemplateNum << 
	"(compression technique Grid Point Simple Packing " <<
	"with Logarithm Pre-processing) not implemented" << endl;
      break;


    case 0:     // Grid point data - simple packing
    case 2:     // Grid point data - complex packing
    case 3:     // Grid point data - complex packing and spacial differencing 
    case 40:    // Jpeg 2000 packing
    case 4000:  // Jpeg 2000 packing
    case 41:    // PNG packing 
    case 40010: // PNG packing 

       if(_dataTemp->pack(dataPtr) == GRIB_FAILURE)
	 return GRIB_FAILURE;
       _sectionLen = _dataTemp->getTemplateSize();
       _data_status = ENCODE;
       return GRIB_SUCCESS;

    default:
      cerr << "ERROR: DS()" << endl;
      cerr << "Data TemplateNum (compression technique) - " << _drsTemplateNum << " not implemented" << endl;
      return GRIB_FAILURE;
  }

  return GRIB_FAILURE;
}

int DS::pack(ui08 *dsPtr)
{
  if(_dataTemp == NULL)
    return GRIB_FAILURE;

  dsPtr[4] = (ui08) _sectionNum;

  ui08 *pdata = _dataTemp->getPackedData();
  int lcpack = _dataTemp->getPackedDataSize();
  if (pdata != 0) {
    for(int j = 0; j < lcpack; j++)
      dsPtr[5+j] = pdata[j];
  }

  _sectionLen = 5 + lcpack;

  _pkUnsigned4(_sectionLen, &(dsPtr[0]));
  _data_status = PACK;

  return GRIB_SUCCESS;

}


void DS::print(FILE *stream) const
{
  fprintf(stream, "\n\n");
  fprintf(stream, "Grib Data Section:\n");
  fprintf(stream, "----------------------------------------------------\n\n");
  if(_dataTemp == NULL)
    return;

  _dataTemp->print(stream);

}

/*          Get bits - unpack bits:  Extract arbitrary size values from a
/          packed bit string, right justifying each value in the unpacked
/          iout array.
/           *in    = pointer to character array input
/           *iout  = pointer to unpacked array output
/            iskip = initial number of bits to skip
/            bitsPerVal = number of bits to take
/            nskip = additional number of bits to skip on each iteration
/            n     = number of iterations
/ v1.1
*/
void DS::gbits (ui08 *in, si32 *iout, si32 iskip, si32 bitsPerVal, si32 nskip, si32 n)
{ 
  si32 i,tbit,bitcnt,ibit,itmp;
  si32 nbit,index;
  static si32 ones[]={1,3,7,15,31,63,127,255};
  
  //     nbit is the start position of the field in bits
  nbit = iskip;
  for (i=0;i<n;i++) {
    bitcnt = bitsPerVal;
    index=nbit/8;
    ibit=nbit%8;
    nbit = nbit + bitsPerVal + nskip;
    
    //        first byte
    tbit= ( bitcnt < (8-ibit) ) ? bitcnt : 8-ibit;  // find min
    itmp = (int)*(in+index) & ones[7-ibit];
    if (tbit != 8-ibit) itmp >>= (8-ibit-tbit);
    index++;
    bitcnt = bitcnt - tbit;
    
    //        now transfer whole bytes
    while (bitcnt >= 8) {
      itmp = itmp<<8 | (int)*(in+index);
      bitcnt = bitcnt - 8;
      index++;
    }
    
    //        get data from last byte
    if (bitcnt > 0) {
      itmp = ( itmp << bitcnt ) | ( ((int)*(in+index) >> (8-bitcnt)) & ones[bitcnt-1] );
    }
    
    *(iout+i) = itmp;
  }
}


void DS::sbits (ui08 *out, si32 *in, si32 iskip, si32 bitsPerVal, si32 nskip, si32 n)
{
//          Store bits - pack bits:  Put arbitrary size values into a
//          packed bit string, taking the low order bits from each value
//          in the unpacked array.
//            *out  = packed array output
//            *in   = unpacked array input
//            iskip = initial number of bits to skip
//            bitsPerVal = number of bits to pack
//            nskip = additional number of bits to skip on each iteration
//            n     = number of iterations
// v1.1
//
  
  si32 bitcnt, tbit, nbit, itmp, index;
  si32 ibit, imask, itmp2, itmp3;

  const si32 ones[]={1,3,7,15,31,63,127,255};

  //     number bits from zero to ...
  //     nbit is the last bit of the field to be filled
  nbit = iskip + bitsPerVal - 1;
  for(int i = 0; i < n; i++)
  {
    itmp = in[i];
    bitcnt = bitsPerVal;
    index = nbit/8;
    ibit = nbit%8;
    nbit = nbit + bitsPerVal + nskip;

    // make byte aligned 
    if (ibit != 7) 
    {
      tbit = ( bitcnt < (ibit+1) ) ? bitcnt : ibit+1;  // find min
      imask = ones[tbit-1] << (7-ibit);
      itmp2 = (itmp << (7-ibit)) & imask;
      itmp3 = (int)*(out+index) & (255-imask);
      out[index] = char(itmp2 | itmp3);
      bitcnt = bitcnt - tbit;
      itmp = itmp >> tbit;
      index--;
    }

    //        now byte aligned
    
    //        do by bytes
    while (bitcnt >= 8)
    {
      out[index] = char(itmp & 255);
      itmp = itmp >> 8;
      bitcnt = bitcnt - 8;
      index--;
    }

    //        do last byte
    
    if (bitcnt > 0) 
    {
      itmp2 = itmp & ones[bitcnt-1];
      itmp3 = (int)*(out+index) & (255-ones[bitcnt-1]);
      out[index] = char(itmp2 | itmp3);
    }
  }

}

} // namespace Grib2

