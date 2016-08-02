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
// Template5_pt_2 Complex Packing Format
//
// Jason Craig,  May 2006
//
//////////////////////////////////////////////////

#include <grib2/Template5.2.hh>
#include <grib2/DRS.hh>
#include <grib2/GDS.hh>

using namespace std;

namespace Grib2 {

const si32 Template5_pt_2::TEMPLATE5_PT_2_SIZE = 47;

Template5_pt_2::Template5_pt_2(si32 decimalScaleFactor, si32 origFieldTypes)
: DataRepTemp()
{
  _dataRepresentation.templateNumber = 2;
  _dataRepresentation.decimalScaleFactor = decimalScaleFactor;
  _dataRepresentation.binaryScaleFactor = 0;
  _dataRepresentation.origFieldTypes = origFieldTypes;
}

Template5_pt_2::Template5_pt_2(Grib2Record::Grib2Sections_t sectionsPtr)
: DataRepTemp(sectionsPtr)
{
  _dataRepresentation.templateNumber = 2;
}


Template5_pt_2::~Template5_pt_2 () {

}


int Template5_pt_2::pack (ui08 *templatePtr) 
{

  si32 tmp = GribSection::mkIeee(_dataRepresentation.referenceValue);
  GribSection::_pkUnsigned4(tmp, &(templatePtr[0]));

  GribSection::_pkUnsigned2(_dataRepresentation.binaryScaleFactor, &(templatePtr[4]));

  GribSection::_pkUnsigned2(_dataRepresentation.decimalScaleFactor, &(templatePtr[6]));

  templatePtr[8] = (ui08) _dataRepresentation.numberOfBits;

  templatePtr[9] = (ui08) _dataRepresentation.origFieldTypes;

  templatePtr[10] = (ui08) _splittingMethod;

  templatePtr[11] = (ui08) _missingType;

  if(_dataRepresentation.origFieldTypes == 0)
    tmp = GribSection::mkIeee(_primaryMissingVal);
  else
    tmp = (int)_primaryMissingVal;
  GribSection::_pkUnsigned4(tmp, &(templatePtr[12]));

  if(_dataRepresentation.origFieldTypes == 0)
    tmp = GribSection::mkIeee(_secondaryMissingVal);
  else
    tmp = (int)_secondaryMissingVal;
  GribSection::_pkUnsigned4(tmp, &(templatePtr[16]));

  GribSection::_pkUnsigned4(_numberGroups, &(templatePtr[20]));

  templatePtr[24] = (ui08) _groupWidths;

  templatePtr[25] = (ui08) _groupWidthsBits;

  GribSection::_pkUnsigned4(_groupLength, &(templatePtr[26]));

  templatePtr[30] = (ui08) _lengthIncrement;

  GribSection::_pkUnsigned4(_lengthOfLastGroup, &(templatePtr[31]));

  templatePtr[35] = (ui08) _groupLengthsBits;

  return GRIB_SUCCESS;

}

int Template5_pt_2::unpack (ui08 *templatePtr) 
{
  si32 tmp;

  _dataRepresentation.templateNumber = 2;

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

  // Group splitting method used (see Code Table 5.4)
  _splittingMethod = (si32) templatePtr[10];

  // Missing value management used (see Code Table 5.5) 
  _missingType = (si32) templatePtr[11];

  // Primary missing value substitute
  _primaryMissingVal = GribSection::_upkUnsigned4 (templatePtr[12], templatePtr[13], templatePtr[14], templatePtr[15]);

  // Secondary missing value substitute
  _secondaryMissingVal = GribSection::_upkUnsigned4 (templatePtr[16], templatePtr[17], templatePtr[18], templatePtr[19]);

  // number of groups of data values into which field is split
  _numberGroups = GribSection::_upkUnsigned4 (templatePtr[20], templatePtr[21], templatePtr[22], templatePtr[23]);

  // Reference for group widths
  // The group width is the number of bits used for every value in a group.
  _groupWidths = (si32) templatePtr[24];

  // Number of bits used for the group widths (after the reference value (_groupWidths) has been removed)
  _groupWidthsBits = (si32) templatePtr[25];

  // Reference for group lengths
  // The group length (L) is the number of values in a group.
  _groupLength = GribSection::_upkUnsigned4 (templatePtr[26], templatePtr[27], templatePtr[28], templatePtr[29]);

  // Length increment for the group lengths
  _lengthIncrement = (si32) templatePtr[30];

  // True length of last group
  _lengthOfLastGroup = GribSection::_upkUnsigned4 (templatePtr[31], templatePtr[32], templatePtr[33], templatePtr[34]);

  // Number of bits used for the scaled group lengths 
  // (after subtraction of the reference value given in octets 38-41 
  //  and division by the length increment given in octet 42)
  _groupLengthsBits = (si32) templatePtr[35];

  return( GRIB_SUCCESS );
}


void Template5_pt_2::print(FILE *stream) const
{
  fprintf(stream, "Complex Packing:\n");

  fprintf(stream, "Reference value (R) (IEEE 32-bit floating point value) %f\n", 
                                                      _dataRepresentation.referenceValue);
  fprintf(stream, "Binary scale factor (E) %d\n", _dataRepresentation.binaryScaleFactor);
  fprintf(stream, "Decimal scale factor (D) %d\n", _dataRepresentation.decimalScaleFactor );
  fprintf(stream, "Number of bits used for each packed value %d\n", _dataRepresentation.numberOfBits);
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
       else if (_dataRepresentation.origFieldTypes >= 192 && _dataRepresentation.origFieldTypes <= 254)
         fprintf(stream, "Reserved for local use\n");
       else
         fprintf(stream, "Missing\n");
  }
  fprintf(stream, "Group splitting method is ");
  switch (_splittingMethod) {
     case 0:
      fprintf(stream, "Row by Row Splitting\n");
      break;
     case 1:
      fprintf(stream, "General Group Splitting\n");
      break;
     default:
       if (_dataRepresentation.origFieldTypes >= 2 && _dataRepresentation.origFieldTypes <= 191)
         fprintf(stream, "Reserved\n");
       else if (_dataRepresentation.origFieldTypes >= 192 && _dataRepresentation.origFieldTypes <= 254)
         fprintf(stream, "Reserved for local use\n");
       else
         fprintf(stream, "Missing\n");
  }

  switch (_missingType) {
     case 0:
       fprintf(stream, "No explicit missing values included within the data values\n");
       break;
     case 1:
       fprintf(stream, "Primary Missing Value is %f\n", _primaryMissingVal);
       break;
     case 2:
       fprintf(stream, "Primary Missing Value is %f\n", _primaryMissingVal);
       fprintf(stream, "Secondary Missing Value is %f\n", _secondaryMissingVal);
       break;
     default:
       if (_dataRepresentation.origFieldTypes >= 2 && _dataRepresentation.origFieldTypes <= 191)
         fprintf(stream, "Missing Value Type Reserved\n");
       else if (_dataRepresentation.origFieldTypes >= 192 && _dataRepresentation.origFieldTypes <= 254)
         fprintf(stream, "Missing Value Type Reserved for local use\n");
       else
         fprintf(stream, "Missing Value Type is Missing\n");
  }
  fprintf(stream, "Number of Groups %d\n", _numberGroups);
  fprintf(stream, "Group Widths %d\n", _groupWidths);
  fprintf(stream, "Number of bits used for the group widths %d\n", _groupWidthsBits);
  fprintf(stream, "Group Lengths %d\n", _groupLength);
  fprintf(stream, "Length increment for group lengths %d\n", _lengthIncrement);
  fprintf(stream, "True length of last group %d\n", _lengthOfLastGroup);
  fprintf(stream, "Number of bits used for the scaled group lengths %d\n", _groupLengthsBits);
}

} // namespace Grib2

