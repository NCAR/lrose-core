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


#include "Process.hh"

#include <iostream>
#include <Mdv/MdvxField.hh>
#include <math.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxPjg.hh>

using namespace std;

//
// Constructor
//
Process::Process(){
  OutputUrl = (char *)NULL;
}

////////////////////////////////////////////////////
//
// Main method - process data at a given time.
//
int Process::Derive(Params *P, time_t T){

  if (P->Debug)
    cerr << "*** Processing data for time: " << DateTime::str(T) << endl;
  
  OutputUrl = STRdup(P->OutUrl);

  string InputUrl = P->InUrl;
  
  if (InputUrl.length() == 0)
    InputUrl = P->TriggerUrl;
  
  if (P->Debug){
    cerr << "Data at " << utimstr(T) << endl;
  }


  //
  // Set up for the new data.
  //
  DsMdvx New;


//  New.setDebug( P->Debug);

  New.setReadTime(Mdvx::READ_FIRST_BEFORE, InputUrl, 0, T);
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  New.setReadScalingType(Mdvx::SCALING_NONE);
  

  if (P->RemapGrid){
    switch ( P->grid_projection){

    case Params::FLAT:
      New.setReadRemapFlat(P->grid_nx, P->grid_ny,
			 P->grid_minx, P->grid_miny,
			 P->grid_dx, P->grid_dy,
			 P->grid_origin_lat, P->grid_origin_lon,
			 P->grid_rotation);

      break;
                                   
    case Params::LATLON:
      New.setReadRemapLatlon(P->grid_nx, P->grid_ny,
			   P->grid_minx, P->grid_miny,
			   P->grid_dx, P->grid_dy);

      break;            

    case Params::LAMBERT:
      New.setReadRemapLc2(P->grid_nx, P->grid_ny,
			P->grid_minx, P->grid_miny,
			P->grid_dx, P->grid_dy,
			P->grid_origin_lat, 
			P->grid_origin_lon,
			P->grid_lat1,  P->grid_lat2);
      
      break;
      
    default:
      cerr << "Unsupported projection." << endl;
      return -1;
      break;
      
    }               
  }

  if (New.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    cerr << InputUrl  << endl;
    return -1;
  }     

  //
  // Set up the output.
  //
  Mdvx::master_header_t InMhdr = New.getMasterHeader();
  Mdvx::master_header_t OutMhdr = InMhdr;

  Out.setMasterHeader(OutMhdr);
  Out.clearFields();     


  // Get a pointer to the threshold field, if it is separate from the
  // input fields.

  MdvxField *ThreshField = 0;
  MdvxField *MinThreshField = 0;
  MdvxField *MaxThreshField = 0;
  
  if (!_getThreshField(New, P, T, &ThreshField, "thresh"))
    return -1;

  if(!P->OneValueMin)
  {
    if (!_getThreshField(New, P, T, &MinThreshField, "min"))
      return -1;
  }

  if(!P->OneValueMax)
  {
    if (!_getThreshField(New, P, T, &MaxThreshField, "max"))
      return -1;
  }
  
  //
  // Commence loop through fields.
  //
  for (int i=0; i< P->InFieldName_n; i++){
    //
    // Get the desired field.
    //
    MdvxField *InField;

    if (!(strncmp(P->_InFieldName[i],"#",1))){
      int Fnum = atoi(P->_InFieldName[i] + 1);
      InField = New.getFieldByNum( Fnum ); 
    } else {
      InField = New.getFieldByName( P->_InFieldName[i] );
    }

    if (InField == NULL){
      cerr << "New field " << P->_InFieldName[i] << " not found." << endl;
      return -1;
    }
    Mdvx::field_header_t InFhdr = InField->getFieldHeader();
    Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();

    MdvxProj Proj(InMhdr, InFhdr);

    fl32 *InData = (fl32 *) InField->getVol();
    fl32 *ThreshData = 0;
    Mdvx::field_header_t ThreshFhdr;
    bool threshIs2d = false;
    
    if (ThreshField == 0)
    {
      ThreshData = InData;
      ThreshFhdr = InFhdr;
    }
    else
    {
      ThreshData = (fl32 *)ThreshField->getVol();
      ThreshFhdr = ThreshField->getFieldHeader();
      
      MdvxPjg ThreshPjg(ThreshFhdr);
      MdvxPjg InPjg(InFhdr);
      
      if (ThreshPjg != InPjg)
      {
        // if thresh field is 2D but has same plane grid, apply 2D to each vert level
        if( (InFhdr.nx == ThreshFhdr.nx) &&
            (InFhdr.ny == ThreshFhdr.ny) &&
            (ThreshFhdr.nz == 1) )
	{
          threshIs2d = true;
          cout << "Applying 2D thresh field to each vertical level." << endl;
        }
        else
	{
	  cerr << "Threshold field has different projection than input field" << endl;
          cerr << "Cannot process field" << endl;
          return -1;
	}
      }
    }

    fl32 minThresh = (fl32)P->MinThresh;
    fl32 maxThresh = (fl32)P->MaxThresh;
    fl32 *minThreshData = 0;
    fl32 *maxThreshData = 0;

    if(!P->OneValueMin)
    {
      minThreshData = (fl32 *)MinThreshField->getVol();
    }

    if(!P->OneValueMax)
    {
      maxThreshData = (fl32 *)MaxThreshField->getVol();
    }
    
    int plane_size = InFhdr.nx*InFhdr.ny;
    int ndex_max = plane_size*InFhdr.nz;
    int tIndex;
    int index2d;
    for(int index = 0; index < ndex_max; index++){
      if(threshIs2d)
      {
        tIndex = index % plane_size;
      }
      else
      {
        tIndex = index;
      }

      if(!P->OneValueMin)
      {
        index2d = index % plane_size;
        minThresh = minThreshData[index2d];
      }

      if(!P->OneValueMax)
      {
        index2d = index % plane_size;
        maxThresh = maxThreshData[index2d];
      }
      //
      // Process bad or missing data in the threshold field
      //
      if (ThreshData[tIndex] == ThreshFhdr.missing_data_value ||
	  ThreshData[tIndex] == ThreshFhdr.bad_data_value)
      {
	if (P->ThreshMissing)
	  InData[index] = InFhdr.missing_data_value;
      }
      //
      // See if data passes threshold, if not, mark it missing.
      //
      else if (ThreshData[tIndex] < minThresh ||
	       ThreshData[tIndex] > maxThresh)
      {
        if (P->ReplaceFailValues)
        {
	  if (InData[index] != InFhdr.missing_data_value &&
	      InData[index] != InFhdr.bad_data_value)
	  {
	    InData[index]=P->FailValue;
	  }
	} else {
  	  InData[index] = InFhdr.missing_data_value;
	}
      }
      //
      // Replace passed values with a single value, if desired.
      //
      else if (P->ReplacePassValues)
      {
	if (InData[index] != InFhdr.missing_data_value &&
	    InData[index] != InFhdr.bad_data_value)
	{
	  InData[index]=P->PassValue;
	}
      }
    }
    //
    // Do a loop to get the min,max. Useful as a diagnostic,
    // and allows us to set the bad and missing values of the
    // output.
    //
    int first = 1;
    double min=0.0, max=0.0, mean, total=0.0;
    long NumGood = 0;
    for(int k=0; k < InFhdr.nx*InFhdr.ny*InFhdr.nz; k++){
      if (
	  (InData[k] == InFhdr.missing_data_value) ||
	  (InData[k] == InFhdr.bad_data_value)
	  ) {
	continue;
      } else {
	NumGood++;
	total = total + InData[k];
	if (first){
	  min = InData[k];
	  max = min;
	  first = 0;
	} else {
	  if (InData[k] < min) min = InData[k];
	  if (InData[k] > max) max = InData[k];
	}
      }
    }
    
    if (NumGood == 0){
      if (P->Debug){
	cerr << "All output data are missing." << endl;
      }
    } else {
      mean = total / double(NumGood);
      fl32 newBad = max + 1.0;
      for(int k=0; k < InFhdr.nx*InFhdr.ny*InFhdr.nz; k++){
	if (
	    (InData[k] == InFhdr.missing_data_value) ||
	    (InData[k] == InFhdr.bad_data_value)
	    ) {   
	  InData[k] = newBad;
	}
      }

      InFhdr.missing_data_value = newBad;
      InFhdr.bad_data_value = newBad;

      if (P->Debug){
	cerr << "Data range from " << min << " to " << max;
	cerr << " with mean " << mean << endl;
      }
    }

    MdvxField *fld = new MdvxField(InFhdr, InVhdr, (void *)InData);
    
    fld->setFieldName(P->_OutFieldName[i]);
    fld->setUnits(P->_Units[i]);
    fld->setFieldNameLong(P->_OutFieldName[i]);
    
    switch (P->output_encoding_type)
    {
    case Params::ENCODING_INT8 :
      if (fld->convertRounded(Mdvx::ENCODING_INT8,
			      Mdvx::COMPRESSION_ZLIB) != 0)
      {
	cerr << "convertRounded failed - I cannot go on." << endl;
	return -1;
      }  
      break;
    
    case Params::ENCODING_INT16 :
      if (fld->convertRounded(Mdvx::ENCODING_INT16,
			      Mdvx::COMPRESSION_ZLIB) != 0)
      {
	cerr << "convertRounded failed - I cannot go on." << endl;
	return -1;
      }
      break;
  
    case Params::ENCODING_FLOAT32 :
      if (fld->compress(Mdvx::COMPRESSION_ZLIB) != 0)
      {
	cerr << "compress failed - I cannot go on." << endl;
	return -1;
      }
      break;
    } /* endswitch - P->output_encoding_type */
    
    Out.addField(fld);
    
  } // End of loop through the fields.

  if (ThreshField != 0)
    delete ThreshField;
  
  if (MinThreshField != 0)
    delete MinThreshField;

  if (MaxThreshField != 0)
    delete MaxThreshField;
  
  if (P->Debug){
    cerr << "Finished data processing for this time." << endl << endl;
  }

  return 0;

}

////////////////////////////////////////////////////
//
// _getThreshField() - Get a pointer to the appropriate threshhold
//                     field.
//
// Returns true on success, false on failure
//
bool Process::_getThreshField(const Mdvx &New,
			      const Params *P,
			      const time_t data_time,
			      MdvxField **ThreshFieldPtr,
                              string type) const
{
  string url; 
  string name;

  // set variables based on type
  if(type == "thresh")
  {
    url = P->ThreshUrl;
    name = P->ThreshFieldName;
  }
  else if(type == "min")
  {
    url = P->MinThreshUrl;
    name = P->MinThreshFieldName;
  }
  else if(type == "max")
  {
    url = P->MaxThreshUrl;
    name = P->MaxThreshFieldName;
  }
  else
  {
    cout << "Invalid type for thresh field" << endl;
    return false;
  }

  // Get threshold field from the input file, if specified
  if (strlen(url.c_str()) == 0)
  {
    if (strlen(name.c_str()) == 0)
    {
    }
    else if (!(strncmp(name.c_str(),"#",1)))
    {
      int Fnum = atoi(name.c_str() + 1);
      *ThreshFieldPtr = new MdvxField(*(New.getFieldByNum(Fnum)));

      if (*ThreshFieldPtr == 0)
      {
	cerr << "Threshold field " << name
	     << " not found." << endl;
	return false;
      }
    }
    else
    {
      *ThreshFieldPtr =
	new MdvxField(*(New.getFieldByName( name )));

      if (*ThreshFieldPtr == 0)
      {
	cerr << "Threshold field " << name
	     << " not found." << endl;
	return false;
      }
    }
  }
  else  // Get threshold field from specified URL
  {
    DsMdvx thresh_mdvx;
    
    thresh_mdvx.setReadTime(Mdvx::READ_FIRST_BEFORE, url,
			    P->ThreshValidSecs, data_time);
    thresh_mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    thresh_mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
    thresh_mdvx.setReadScalingType(Mdvx::SCALING_NONE);
    
    if (!(strncmp(name.c_str(), "#", 1)))
    {
      int field_num = atoi(name.c_str() + 1);
      thresh_mdvx.addReadField(field_num);
    }
    else
    {
      thresh_mdvx.addReadField(name);
    }
    
    // Remap the threshold field to match the first input field

    MdvxPjg proj(New);

    thresh_mdvx.setReadRemap(proj);
    
    if (thresh_mdvx.readVolume())
    {
      cerr << "ERROR: Thresh URL read failed at " << utimstr(data_time)
	   << " from " << url << endl;
      return false;
    }
    
    *ThreshFieldPtr = new MdvxField(*(thresh_mdvx.getField(0)));
  }
      
  return true;
}

////////////////////////////////////////////////////
//
// Destructor
//
Process::~Process(){
  //
  // Only do the write if fields were added.
  //

  Mdvx::master_header_t Mhdr = Out.getMasterHeader();

  if (Mhdr.n_fields > 0){
    if (Out.writeToDir(OutputUrl)) {
      cerr << "Failed to wite to " << OutputUrl << endl;
      exit(-1);
    }      
  }
  free(OutputUrl);
}










