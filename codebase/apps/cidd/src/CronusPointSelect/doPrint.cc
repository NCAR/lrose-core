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
//////////////////////////////////////////////////////////
// doPrint.cc
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// December 2003
//

#include "doPrint.hh"
#include <Mdv/MdvxField.hh>
#include <cmath>
using namespace std;

// constructor
doPrint::doPrint (Params *TDRP_Params, int refNum){
  _params = TDRP_Params;
  _firstTime = true;
  _dataOK = false;
  _lastTime = 0L;
  _refNum = refNum;

  _Proj = NULL;
  _Fhdr = NULL;
  _Vhdr = NULL;
  _cronusFhdr = NULL;
  _cronusVhdr = NULL;

}


// destructor.

doPrint::~doPrint(){
  delete _Proj;
  delete _Fhdr;
  delete _Vhdr;
  delete _cronusFhdr;
  delete _cronusVhdr;
}

  // Do a printout of data at the time/location specified.

void doPrint::printData(time_t dataTime,
			double lat,
			double lon,
			double *sum){
  //
  // Set the sum to 0 if we are the first object invoked.
  //
  if (_refNum == 0) *sum = 0.0;

  //
  // If this is our first go around, then of course we need 
  // to update the data.
  //
  if (_firstTime){
    _lastTime = dataTime; 
    _dataUpdate(dataTime);
    _firstTime = false;
  }
  //
  // If the data time has changed, then we need to
  // update the data.
  //
  if (dataTime != _lastTime){
    _lastTime = dataTime;
    _dataUpdate(dataTime);
  }
  //
  if (!(_dataOK)){
    fprintf(stderr,"%s :\tNO DATA\n", 
	    _params->_sources[_refNum].label);
    return;
  }

  //
  // Get the indicies.
  //
  int ixc,iyc;
  if (_Proj->latlon2xyIndex(lat, lon, ixc, iyc)){
    fprintf(stderr,"%s : point %g, %g outside grid.\n", 
	    _params->_sources[_refNum].label, 
	    lat, lon);
  return;
  }

  int ixcr,iycr;
  if (_cronusProj->latlon2xyIndex(lat, lon, ixcr, iycr)){
    fprintf(stderr,"%s : point %g, %g outside cronus grid.\n", 
	    _params->_sources[_refNum].label, 
	    lat, lon);
  return;
  }
  //
  if (_Fhdr->nz != 1){
    fprintf(stderr,"WARNING : Data are not two dimensional.\n");
  }
  //
  // Pick off the data values.
  //
  int index =  iyc * _Fhdr->nx + ixc;
  int cronusIndex = iycr * _cronusFhdr->nx + ixcr;

  double datavalue, cronusDataValue;

  if (
      (_dataPointer[index] != _Fhdr->bad_data_value) &&
      (_dataPointer[index] != _Fhdr->missing_data_value)
      ){
    datavalue = _dataPointer[index];
  } else {
    datavalue = 0.0; // Set to 0 for missing data.
  }

  if (
      (_cronusDataPointer[cronusIndex] != _cronusFhdr->bad_data_value) &&
      (_cronusDataPointer[cronusIndex] != _cronusFhdr->missing_data_value)
      ){
    cronusDataValue = _cronusDataPointer[cronusIndex];
  } else {
    cronusDataValue = 0.0; // Set to 0 for missing data.
  }
  //
  //
  //
  double cronusWeight = _params->_sources[_refNum].cronusWeight;
  //
  // If we are the first object to print, then print a few
  // blank lines followed by a header.
  //
  if (_refNum == 0){
    for (int i=0; i < _params->numHeaderBlanks; i++){
      fprintf(stderr,"\n");
    }
 
    fprintf(stderr,"Time : %s Lat : %g Lon : %g\n",
	    utimstr( dataTime ), lat, lon );
  }

  //
  // Print the data.
  //
  if (_params->printFieldTimes){
    fprintf(stderr,"Data time=%s, ", utimstr(_actualDataTime));
  }

  char printFormatString[1024];
  sprintf(printFormatString,
	  "%s :\tValue=%s\tWeight=%s\tInterest value=%s\tScaled value=%s\n",
	  _params->_sources[_refNum].label,
	  _params->formatString, _params->formatString, 
	  _params->formatString, _params->formatString);

  fprintf(stderr, printFormatString,
	  _roundVal(datavalue),
	  _roundVal(cronusWeight),
	  _roundVal(cronusDataValue),
	  _roundVal(cronusWeight*cronusDataValue));

  *sum = *sum + cronusWeight*cronusDataValue;

  //
  // If we are the last object invoked, print the sum.
  //


  if (_refNum ==  _params->sources_n-1){
    sprintf(printFormatString, "\nSum of scaled values : %s\n",
	    _params->formatString); 
    fprintf(stderr, printFormatString, _roundVal(*sum));
  }

  return;
}
 

void doPrint::_dataUpdate(time_t dataTime){

  if ((_refNum == 0) && (_params->debug)){
    time_t now = time(NULL);
    fprintf(stderr,"Data update requested at %s\n",
	    utimstr(now));
  }

  _dataOK = false;
  if (!(_firstTime)){
    delete _Proj;
    delete _Fhdr;
    delete _Vhdr;
    delete _cronusFhdr;
    delete _cronusVhdr;
  }

  //
  // First half - read the data.
  //
  _mdvxObject.setReadTime(Mdvx::READ_FIRST_BEFORE, 
			  _params->_sources[_refNum].url, 
			  _params->_sources[_refNum].lookBack, 
			  dataTime);
  _mdvxObject.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _mdvxObject.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  _mdvxObject.addReadField( _params->_sources[_refNum].fieldName );

  if (_mdvxObject.readVolume()){
    if (_params->debug){
      fprintf(stderr, "Read failed at %s from %s\n",
	      utimstr(dataTime),  _params->_sources[_refNum].url);
    }
    return;
  }

  MdvxField *InField;

  // Should be field 0 since that's all we requested with addField. 

  InField = _mdvxObject.getFieldByNum( 0 );

  if (InField == NULL){
    if (_params->debug){
      fprintf(stderr,"Unable to find field %s\n", 
	      _params->_sources[_refNum].fieldName );
    }
    return;
  }
  //
  // Get the master, field and vlevel headers.
  //
  Mdvx::master_header_t InMhdr = _mdvxObject.getMasterHeader();   
  Mdvx::field_header_t InFhdr = InField->getFieldHeader();
  Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();

  _Fhdr = new  Mdvx::field_header_t( InFhdr );
  _Vhdr = new  Mdvx::vlevel_header_t( InVhdr );

  _actualDataTime = InMhdr.time_centroid;

  // Set up the Proj, and return a pointer to the data.
  
  _Proj = new MdvxProj(InMhdr, InFhdr);
 
  _dataPointer = (fl32 *) InField->getVol();

  //
  // Second half of the method - same thing but
  // for the cronus data object.
  //
  _cronusMdvxObject.setReadTime(Mdvx::READ_FIRST_BEFORE, 
			  _params->_sources[_refNum].cronusUrl, 
			  _params->_sources[_refNum].lookBack, 
			  dataTime);
  _cronusMdvxObject.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _cronusMdvxObject.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  _cronusMdvxObject.addReadField( _params->_sources[_refNum].cronusFieldName );

  if (_cronusMdvxObject.readVolume()){
    if (_params->debug){
      fprintf(stderr, "Read failed at %s from %s\n",
	      utimstr(dataTime),  _params->_sources[_refNum].cronusUrl);
    }
    return;
  }

  MdvxField *cronusField;

  // Should be field 0 since that's all we requested with addField. 

  cronusField = _cronusMdvxObject.getFieldByNum( 0 );

  if (cronusField == NULL){
    if (_params->debug){
      fprintf(stderr,"Unable to find cronus field %s\n", 
	      _params->_sources[_refNum].cronusFieldName );
    }
    return;
  }
  //
  // Get the master, field and vlevel headers.
  //
  Mdvx::master_header_t cronusMhdr = _cronusMdvxObject.getMasterHeader();   
  Mdvx::field_header_t cronusFhdr = cronusField->getFieldHeader();
  Mdvx::vlevel_header_t cronusVhdr = cronusField->getVlevelHeader();

  _cronusFhdr = new  Mdvx::field_header_t( cronusFhdr );
  _cronusVhdr = new  Mdvx::vlevel_header_t( cronusVhdr );

  // Set up the Proj, and return a pointer to the data.
  
  _cronusProj = new MdvxProj(cronusMhdr, cronusFhdr);
  _cronusDataPointer = (fl32 *) cronusField->getVol();
 
  _dataOK = true;

}

//////////////////////////
//
// Routine to round things off.
//
double doPrint::_roundVal(double in){

  double tenPow = pow(10.0, double(_params->numDecimalPlaces));
  int multVal = (int)rint(in*tenPow);
  return double(multVal)/tenPow;

}
