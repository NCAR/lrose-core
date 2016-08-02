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
///////////////////////////////////////////////////
// Template5_pt_4000 JPEG 2000 Code Stream Format
//
// Gary Blackburn,  Nov 2004
//
//////////////////////////////////////////////////

#include <grib2/Template5.4000.hh>
#include <grib2/DRS.hh>
#include <grib2/GDS.hh>

using namespace std;

namespace Grib2 {

const si32 Template5_pt_4000::TEMPLATE5_PT_4000_SIZE = 23;

Template5_pt_4000::Template5_pt_4000(si32 decimalScaleFactor, si32 origFieldTypes,
				     si32 compressionType, si32 targetCompressionRatio)
  : DataRepTemp()
{
  _dataRepresentation.templateNumber = 40;
  _dataRepresentation.decimalScaleFactor = decimalScaleFactor;
  _dataRepresentation.binaryScaleFactor = 0;
  _dataRepresentation.origFieldTypes = origFieldTypes;
  _compressionType = compressionType;
  _targetCompressionRatio = targetCompressionRatio;
}

Template5_pt_4000::Template5_pt_4000(Grib2Record::Grib2Sections_t sectionsPtr)
: DataRepTemp(sectionsPtr)
{
  _dataRepresentation.templateNumber = 40;
}


Template5_pt_4000::~Template5_pt_4000 () {

}


int Template5_pt_4000::pack (ui08 *templatePtr) 
{
  si32 tmp = GribSection::mkIeee(_dataRepresentation.referenceValue);
  GribSection::_pkUnsigned4(tmp, &(templatePtr[0]));

  GribSection::_pkUnsigned2(_dataRepresentation.binaryScaleFactor, &(templatePtr[4]));

  GribSection::_pkUnsigned2(_dataRepresentation.decimalScaleFactor, &(templatePtr[6]));

  templatePtr[8] = (ui08) _dataRepresentation.numberOfBits;

  templatePtr[9] = (ui08) _dataRepresentation.origFieldTypes;

  templatePtr[10] = (ui08) _compressionType;

  templatePtr[11] = (ui08) _targetCompressionRatio;

  return GRIB_SUCCESS;

}

int Template5_pt_4000::unpack (ui08 *templatePtr) 
{
  si32 tmp;

  _dataRepresentation.templateNumber = 4000;

  // Reference value (R) (IEEE 32-bit floating-point value) 
  tmp = GribSection::_upkUnsigned4 (templatePtr[0], templatePtr[1], templatePtr[2], templatePtr[3]);
  _dataRepresentation.referenceValue = GribSection::rdIeee(tmp);

  // Binary scale factor (E) 
  _dataRepresentation.binaryScaleFactor =
            GribSection::_upkSigned2 (templatePtr[4], templatePtr[5]);

  // Decimal scale factor (D) 
  _dataRepresentation.decimalScaleFactor =
            GribSection::_upkSigned2 (templatePtr[6], templatePtr[7]);

  // Number of bits holding scaled and referenced data values.  (i.e. greyscale image depth.) 
  _dataRepresentation.numberOfBits = (si32) templatePtr[8]; 

  // Type of original field values (see Code Table 5.1) 
  _dataRepresentation.origFieldTypes = (si32) templatePtr[9]; 

  // Type of Compression used. (see Code Table 5.40000) 
  _compressionType = (si32) templatePtr[10]; 

  // Target compression ratio, M:1 (with respect to the bit-depth specified in octet 20), 
  //  when octet 22 indicates Lossy Compression.   Otherwise, set to missing. (see Note 3)
  _targetCompressionRatio = (si32) templatePtr[11]; 

  return( GRIB_SUCCESS );
}


void Template5_pt_4000::print(FILE *stream) const
{
  fprintf(stream, "JPEG 2000 Code Stream Format:\n");

  fprintf(stream, "Reference value (R) (IEEE 32-bit floating point value) %f\n", 
                                                              _dataRepresentation.referenceValue);
  fprintf(stream, "Binary scale factor (E) %d\n", _dataRepresentation.binaryScaleFactor);
  fprintf(stream, "Decimal scale factor (D) %d\n", _dataRepresentation.decimalScaleFactor );
  fprintf(stream, "Number of bits required to hold scaled/referenced values - greyscale depth%d\n", 
                                                             _dataRepresentation.numberOfBits);
  fprintf(stream, "Type of original field values is ");
  switch (_dataRepresentation.origFieldTypes) {
     case 0:
 	fprintf(stream, "Floating point\n");
        break;
     case 1:
 	fprintf(stream, "Integer\n");
        break;
     case 255:
        fprintf(stream, "Missing\n");
        break;
     default:
       if (_dataRepresentation.origFieldTypes >= 2 && _dataRepresentation.origFieldTypes <= 191)
         fprintf(stream, "Reserved\n");
       else 
         fprintf(stream, "Reserved for local use\n");
  }
  fprintf(stream, "Type of compression used is ");
  switch (_compressionType) {
     case 0:
	fprintf(stream, "Lossless\n");
        break;
     case 1:
	fprintf(stream, "Lossyn");
        break;
     case 255:
        fprintf(stream, "Missing\n");
        break;
     default:
       if (_compressionType >= 2 && _compressionType <= 254)
         fprintf(stream, "Reserved\n");
  }
  fprintf(stream, "M:1 with respect to the bit-depth specified in _grayscalImageDepth, Lossy only %d\n", _targetCompressionRatio);


}

} // namespace Grib2

