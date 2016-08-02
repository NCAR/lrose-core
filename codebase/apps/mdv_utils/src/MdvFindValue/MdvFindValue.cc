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
//
//
// Program to print filename if MDV file has data
// meeting certain criteria
//
// Use it like this :
//
// MdvxHowBadField blah.mdv field_name min max
//
// This is a simplistic diagnostic so it's not coded all that neatly.
// It is a reasonable example of how MDV data are read, however.
//
// Niles Oien

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>  
#include <cstdio>
#include <cstdlib>
#include <math.h>
using namespace std;

int main(int argc, char *argv[])
{

  //
  // Get the input MDV filename from the command line.
  //
  char FileName[1024];
  char FieldName[1024];
  float min=0,max=0;
 

  // See if we got a filename on the input line.
  // if not, ask for one.
  // 



  if (argc < 5){
    fprintf(stderr, "USAGE : %s filename fieldname min max\n", argv[0]);
    fprintf(stderr, "EXAMPLE : %s ./20110405201634.mdv DBZ 34 1000\n", argv[0]);
    return -1;
  }

  sprintf(FileName,"%s", argv[1]);
  sprintf(FieldName,"%s", argv[2]);
  min =atof(argv[3]);
  max =atof(argv[4]);
 


  //
  // Instantiate an MDV object.
  //
  DsMdvx In;

  //
  // Set the read path. This is a rather unusual way to read
  // MDV data, more typically a client will specify a URL to
  // read the data from and the time for which we want to read
  // data, with a temporal tolerance, and read the data most
  // appropriate for that time. But that's not what we're doing
  // in this little diagnostic.
  //
  In.setReadPath(FileName);

  //
  // Only read the field we want.
  //
  In.addReadField(FieldName);

  //
  // Set the encoding type to be floating point numbers.
  // Other options include scaled bytes, in which case the
  // phsical value is opbtained by multiplying the bytes by a scale and
  // adding an offset. Scaled integers are also an option.
  //
  In.setReadEncodingType(Mdvx::ENCODING_FLOAT32);

  //
  // Make sure we get uncompressed data.
  //
  In.setReadCompressionType(Mdvx::COMPRESSION_NONE);
         
  //
  // OK - we're ready to do the read. Whatever data is in the
  // MDV file on disk, it will now be converted to floating point
  // uncompressed data in the DsMdvx object in memory.
  //
  if (In.readVolume()){
    fprintf(stderr,"Failed to open %s\n",FileName);
    exit(-1);
  }

  //
  // An MDV file consists of one master header followed by
  // several fields. Each field has a field header and a vlevel
  // header. The field header contains information pertinent to
  // that field and the vlevel header (which I don't think we
  // even consider here) has information about the vertical heights
  // of the planes. 
  //
  // Get the master header from the DsMdvx object so we can loop through
  // the fields.
  //
  Mdvx::master_header_t InMhdr = In.getMasterHeader();     

  MdvxField *InField;
  
  //
  // Get the field by its field number. 
  //
  InField = In.getFieldByNum( 0 );
  
  if (InField == NULL){
    fprintf(stderr,"Field %s not found.\n", FieldName);
    return -1;
  }

  //
  // Get the field header from this field object. The vlevel header
  // could be obtained in a similar way.
  //
  Mdvx::field_header_t InFhdr = InField->getFieldHeader();   
  float *data = (float *) InField->getVol();

  for (int j=0; j< InFhdr.nz*InFhdr.ny*InFhdr.nx; j++){
    
    if ((data[j] ==  InFhdr.missing_data_value ) ||
	(data[j] ==  InFhdr.bad_data_value )){
      continue;
    }
	
    if ((data[j] >= min) && (data[j] <= max)){
      fprintf(stderr,"%s\n", FileName);
      return -1;
    }

  }



  // All done. 

  return (0);

}








