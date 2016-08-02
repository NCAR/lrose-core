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
//////////////////////////////////////////////////
// DRS - SECTION 5 - DATA REPRESENTATION SECTION
//
//
/////////////////////////////////////////////////

#include <string.h>
#include <grib2/DRS.hh>
#include <grib2/GDS.hh>
#include <iostream>

using namespace std;


namespace Grib2 {


DRS::DRS(Grib2Record::Grib2Sections_t sectionsPtr) :
  GribSection()
{
  _sectionNum = 5;
  _sectionsPtr = sectionsPtr;
  _sectionsPtr.drs = this;
  _dataRepresentation = NULL;
  _numPackedDataPoints = -1;
}

DRS::DRS(Grib2Record::Grib2Sections_t sectionsPtr, si32 dataRepNum, 
	 DataRepTemp *dataRepTemplate)
{
  _sectionNum = 5;
  _sectionsPtr = sectionsPtr;
  _sectionsPtr.drs = this;
  _numPackedDataPoints = -1;
  _dataTemplateNum = dataRepNum;
   switch (_dataTemplateNum) {
     case 0:
     case 2:
     case 3:
     case 40:
     case 40000:
     case 41:
     case 40010:
       break;

     default:
       _dataRepresentation = NULL;
       cerr << "ERROR: DRS()" << endl;
       cerr << "Data Representation template  - " << _dataTemplateNum << " not implemented" << endl;
       return;
   }
   _dataRepresentation = dataRepTemplate;
   _dataRepresentation->setSectionsPtr(_sectionsPtr);
   _sectionLen = _dataRepresentation->getTemplateSize();
}

DRS::~DRS() 
{
  if(_dataRepresentation != NULL)
    delete _dataRepresentation;
}

int DRS::pack(ui08 *drsPtr)
{
  _pkUnsigned4(_sectionLen, &(drsPtr[0]));

  drsPtr[4] = (ui08) _sectionNum;

  _pkUnsigned4(_numPackedDataPoints, &(drsPtr[5]));

  _pkUnsigned2(_dataTemplateNum, &(drsPtr[9]));

   switch (_dataTemplateNum) {
     case 0:
       // simple packing
     case 2:
       // complex packing
     case 3:
       // complex packing with spatial differencing
     case 40:
     case 40000:
       // jpeg compression
     case 41:    // PNG packing 
     case 40010: // PNG packing 

       return _dataRepresentation->pack(&(drsPtr[11]));

     default:
       cerr << "ERROR: DRS::pack()" << endl;
       cerr << "Data Representation template  - " << _dataTemplateNum << " not implemented" << endl;
       return (GRIB_FAILURE);

   }
}

int DRS::unpack( ui08 *drsPtr ) 
{
   // 
   // Length in bytes of section 
   //
   _sectionLen = _upkUnsigned4 (drsPtr[0], drsPtr[1], drsPtr[2], drsPtr[3]);

   //
   // Number of section ("5")
   _sectionNum = (si32) drsPtr[4];


   if (_sectionNum != 5) {
     cerr << "ERROR: DRS::unpack()" << endl;
     cerr << "Detecting incorrect section number, should be 5 but found section "
           << _sectionNum << endl;
      return( GRIB_FAILURE );

   }

   //
   // Number of data points where one or more values are specified in Section 7 
   // when a bit map is present, total number of data points when a bit map is 
   // absent.
   //

   _numPackedDataPoints = _upkUnsigned4 (drsPtr[5], drsPtr[6], drsPtr[7], drsPtr[8]);

   //
   // Data Representation Template Number (see Code Table 5.0)
   //
   _dataTemplateNum = _upkUnsigned2 (drsPtr[9], drsPtr[10]);
   
   switch (_dataTemplateNum) {
     case 0:
       // simple packing
       _dataRepresentation = new Template5_pt_0 (_sectionsPtr);
       break;
     case 2:
       // complex packing
       _dataRepresentation = new Template5_pt_2 (_sectionsPtr);
       break;
     case 3:
       // complex packing with spatial differencing
       _dataRepresentation = new Template5_pt_3 (_sectionsPtr);
       break;
     case 40:
     case 40000:
       // jpeg compression
       _dataRepresentation = new Template5_pt_4000(_sectionsPtr);
       break;

    case 41:    // PNG packing 
    case 40010: // PNG packing 
       _dataRepresentation = new Template5_pt_41(_sectionsPtr);
       break;

    case 1:     // Matrix values at a grid point - simple packing
      cerr << "ERROR: DS()" << endl;
      cerr << "Data TemplateNum " << _dataTemplateNum << 
	"(compression technique Matrix values) not implemented" << endl;
      return (GRIB_FAILURE);

    case 4:	// Grid Point Data - IEEE Floating Point Data
      cerr << "ERROR: DS()" << endl;
      cerr << "Data TemplateNum " << _dataTemplateNum << 
	"(compression technique Grid Point IEEE Floating) not implemented" << endl;
      return (GRIB_FAILURE);

    case 50:    // Spectral Simple
      cerr << "ERROR: DS()" << endl;
      cerr << "Data TemplateNum " << _dataTemplateNum << 
	"(compression technique Spectral Simple) not implemented" << endl;
      return (GRIB_FAILURE);

    case 51:    // Spectral complex
      cerr << "ERROR: DS()" << endl;
      cerr << "Data TemplateNum " << _dataTemplateNum << 
	"(compression technique Spectral Complex) not implemented" << endl;
      return (GRIB_FAILURE);

    case 61:	// Grid Point Data - Simple Packing With Logarithm Pre-processing
      cerr << "ERROR: DS()" << endl;
      cerr << "Data TemplateNum " << _dataTemplateNum << 
	"(compression technique Grid Point Simple Packing " <<
	"with Logarithm Pre-processing) not implemented" << endl;
      return (GRIB_FAILURE);

     default:
       cerr << "ERROR: DRS::unpack()" << endl;
       cerr << "Data Representation template  - " << _dataTemplateNum << " not implemented" << endl;
       return (GRIB_FAILURE);

   }
   _dataRepresentation->unpack(&drsPtr[11]);


   return( GRIB_SUCCESS );
}

void DRS::print(FILE *stream) const
{
  fprintf(stream, "\n\n");
  fprintf(stream, "Data Representation Section:\n");
  fprintf(stream, "--------------------------------------------------\n");
  fprintf(stream, "DRS length %d\n", _sectionLen);
  fprintf(stream, "   Number of data points in Section 7 %d\n", _numPackedDataPoints);
  fprintf(stream, "   Data Representation Template Number %d\n", _dataTemplateNum);
  fprintf(stream, "\n");
  _dataRepresentation->print(stream);
} 

} // namespace Grib2

