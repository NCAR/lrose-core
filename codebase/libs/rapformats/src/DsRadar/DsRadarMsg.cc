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
//////////////////////////////////////////////////////////////////////////////
//
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// April 1998
//
//////////////////////////////////////////////////////////////////////////////

#include <cstring>
#include <dataport/bigend.h>
#include <didss/DsMsgPart.hh>
#include <rapformats/DsRadarMsg.hh>
using namespace std;

DsRadarMsg::DsRadarMsg() : DsMessage()
{
  pad           = false;
  paramsSet     = false;
  accumContent  = 0;
  nGatesOut     = 1;
  nGatesIn      = 1;
}
   
DsRadarMsg::~DsRadarMsg()
{
  _clearFields();
}

DsRadarMsg&
DsRadarMsg::operator=( const DsRadarMsg &source ) 
{
  copy( source );
  return( *this );
}

void
DsRadarMsg::copy( const DsRadarMsg& source ) 
{
   if( this == &source )
      return;

   _clearFields();
   
   for (size_t ii = 0; ii < source.fieldParams.size(); ii++) {
     DsFieldParams *newField = new DsFieldParams(*source.fieldParams[ii]);
     fieldParams.push_back( newField );
   }

   radarParams  = source.radarParams;
   radarBeam    = source.radarBeam;
   platformGeoref = source.platformGeoref;
   radarFlags   = source.radarFlags;
   radarCalib   = source.radarCalib;
   statusXml    = source.statusXml;
   accumContent = source.accumContent;
   pad          = source.pad;
   nGatesOut    = source.nGatesOut;
   nGatesIn     = source.nGatesIn;

}

int
DsRadarMsg::disassemble( const void *in_msg, const int msg_len,
			 int *content /* = NULL*/ )
{

  //
  // Decode bigEndian message buffer and use it
  // to set the current state of radar info 
  //
  int               msgContent = 0;
  int               numFields;
  int               numFieldParams;
  int               numGates;
  DsFieldParams    *newFieldParams;
  
  // peek at the header to make sure we're looking at the
  // right type of message

  decodeHeader(in_msg);
  if ( _type != DS_MESSAGE_TYPE_DSRADAR ) {
    return( -1 );
  }

  // disassemble the message
  if (DsMessage::disassemble(in_msg, msg_len)) {
    return( -1 );
  }

  //
  // Interpret the message parts
  //

  if (partExists(RADAR_PARAMS)) {
    msgContent |= RADAR_PARAMS;
    DsRadarParams_t *rparams = (DsRadarParams_t *)
      getPartByType(RADAR_PARAMS)->getBuf();
    radarParams.decode(rparams);
    nGatesIn = radarParams.getNumGates();
    if( pad ) {
      radarParams.setNumGates(nGatesOut);
    }
  }

  numFields = radarParams.getNumFields();
  numFieldParams = partExists(FIELD_PARAMS);
  
  if (numFieldParams > 0 &&
      numFieldParams != numFields) {
    fprintf(stderr, "ERROR - wrong number of fields in DsRadarMsg\n");
    fprintf(stderr, "Number of fields in message: %d\n", numFields);
    fprintf(stderr, "Number of fields in header: %d\n",
	    radarParams.getNumFields());
    return (-1);
  }

  if (numFieldParams > 0) {
    msgContent |= FIELD_PARAMS;
    _clearFields();
    for( int i = 0; i < numFields; i++ ) {
      DsFieldParams_t *fieldPtr =
	(DsFieldParams_t *) getPartByType(FIELD_PARAMS, i)->getBuf();
      newFieldParams = new DsFieldParams;
      newFieldParams->decode( fieldPtr );
      fieldParams.push_back( newFieldParams );
    }
  }

  if (partExists(RADAR_BEAM)) {
    numFields = radarParams.getNumFields();
    numGates = radarParams.getNumGates();
    if (numFields > 0 && numGates > 0) {
      msgContent |= RADAR_BEAM;
      DsBeamHdr_t *beam =
	(DsBeamHdr_t *) getPartByType(RADAR_BEAM)->getBuf();
      radarBeam.decode(nGatesIn, beam, numFields, numGates );
    }
  }
   
  if (partExists(PLATFORM_GEOREF)) {
    msgContent |= PLATFORM_GEOREF;
    ds_iwrf_platform_georef_t *georef =
      (ds_iwrf_platform_georef_t *) getPartByType(PLATFORM_GEOREF)->getBuf();
    platformGeoref.decode(*georef);
  }
   
  if (partExists(RADAR_FLAGS)) {
    msgContent |= RADAR_FLAGS;
    DsRadarFlags_t *flags =
      (DsRadarFlags_t *) getPartByType(RADAR_FLAGS)->getBuf();
    radarFlags.decode(flags);
  }
   
  if (partExists(RADAR_CALIB)) {
    msgContent |= RADAR_CALIB;
    ds_radar_calib_t *calib =
      (ds_radar_calib_t *) getPartByType(RADAR_CALIB)->getBuf();
    radarCalib.decode(calib);
  }
   
  if (partExists(STATUS_XML)) {
    msgContent |= STATUS_XML;
    DsMsgPart *xmlPart = getPartByType(STATUS_XML);
    int len = xmlPart->getLength();
    char *buf = new char[len + 2];
    memcpy(buf, xmlPart->getBuf(), len);
    buf[len+1] = '\0'; // ensure NULL termination
    statusXml = buf;
    delete[] buf;
  }
   
  accumContent |= msgContent;
  if( !paramsSet ){
    if ((accumContent & DsRadarMsg::RADAR_PARAMS) &&
	(accumContent & DsRadarMsg::FIELD_PARAMS) &&
	(accumContent & DsRadarMsg::RADAR_BEAM)) {
      paramsSet = true;
    }
  }
   
  if( content != NULL ) {
    *content = msgContent;
  }
   
  return( 0 );

}

ui08 *
  DsRadarMsg::assemble( const int content 
		      /* = RADAR_PARAMS | FIELD_PARAMS | RADAR_BEAM
                       * | RADAR_FLAGS | RADAR_CALIB | STATUS_XML */ )
{

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_DSRADAR);
  
  // clear message parts

  clearParts();

  // radar params

  if ( content & RADAR_PARAMS ) {
    DsRadarParams_t rparams;
    radarParams.encode( &rparams );
    addPart(RADAR_PARAMS, sizeof(DsRadarParams_t), &rparams);
  }

  // field params

  if ( content & FIELD_PARAMS ) {
    // must get the fieldhave radar params
    if (radarParams.getNumFields() == 0) {
      cerr << "WARNING - DsRadarMsg::assemble" << endl;
      cerr << "  Number of fields is 0" << endl;
      cerr << "  Ensure that radar params are valid" << endl;
      cerr << "         when setting field params" << endl;
    }
    for (int i = 0; i < radarParams.getNumFields(); i++) {
      DsFieldParams_t fparams;
      fieldParams[i]->encode(&fparams);
      addPart(FIELD_PARAMS, sizeof(DsFieldParams_t), &fparams);
    }
  }

  // beam

  if ( content & RADAR_BEAM ) {

    radarBeam.encode();
    addPart(RADAR_BEAM, radarBeam.beamLen(), radarBeam.beam());
  }

  // platform georef

  if ( content & PLATFORM_GEOREF ) {
    ds_iwrf_platform_georef_t georef;
    platformGeoref.encode(georef);
    addPart(PLATFORM_GEOREF, sizeof(georef), &georef);
  }

  // flags

  if ( content & RADAR_FLAGS ) {
    DsRadarFlags_t flags;
    radarFlags.encode(&flags);
    addPart(RADAR_FLAGS, sizeof(DsRadarFlags_t), &flags);
  }

  // calib

  if ( content & RADAR_CALIB ) {
    ds_radar_calib_t calib;
    radarCalib.encode(&calib);
    addPart(RADAR_CALIB, sizeof(ds_radar_calib_t), &calib);
  }

  // status xml

  if ( content & STATUS_XML ) {
    addPart(STATUS_XML, statusXml.size() + 1, statusXml.c_str());
  }

  return (DsMessage::assemble());

}

void
DsRadarMsg::padBeams( bool padData, int numGates )
{
  pad       = padData;
  nGatesOut = numGates;
}
   
void
DsRadarMsg::_clearFields()
{
  vector< DsFieldParams* >::iterator i;

  //
  // Clear out the existing field parameters
  //
  for( i=fieldParams.begin(); i != fieldParams.end(); i++ ) {
    delete *i;
  }
  fieldParams.erase( fieldParams.begin(), fieldParams.end() );
}

/////////////////////
// set radar params

void DsRadarMsg::setRadarParams(const DsRadarParams &params)
{
  radarParams = params;
}

///////////////////////////////////////////
// set field params for a particular field

void DsRadarMsg::setFieldParams(const DsFieldParams &params, int fieldNum) { 
  *fieldParams[fieldNum] = params;
}

//////////////////////////
// set field params vector

void DsRadarMsg::setFieldParams(const vector<DsFieldParams *> params) { 
  fieldParams.clear();
  for (size_t ii = 0; ii < params.size(); ii++) {
    addFieldParams(*params[ii]);
  }
}

///////////////////
// set calibration

void DsRadarMsg::setRadarCalib(const DsRadarCalib &calib)
{
  radarCalib = calib;
}

//////////////
// set flags

void DsRadarMsg::setRadarFlags(const DsRadarFlags &flags)
{
  radarFlags = flags;
}

//////////////
// set georef

void DsRadarMsg::setPlatformGeoref(const DsPlatformGeoref &val)
{
  platformGeoref = val;
}

///////////////
// set the beam

void DsRadarMsg::setRadarBeam(const DsRadarBeam &beam)
{
  radarBeam = beam;
}

/////////////////////
// set the status XML

void DsRadarMsg::setStatusXml(const string &xml)
{
  statusXml = xml;
}

void DsRadarMsg::clearStatusXml()
{
  statusXml.clear();
}

///////////////////
// clear the fields

void DsRadarMsg::clearFieldParams()
{
  _clearFields();
}

////////////////////////////////
// add a field parameter object

void DsRadarMsg::addFieldParams(const DsFieldParams &field)
{ 
  DsFieldParams *fparams = new DsFieldParams(field);
  fieldParams.push_back(fparams);
}

///////////////////////////////////////////////////////////////
// Censor beam data based on values in one field.
// The gate value for the censorFieldName field is tested for
// inclusion between minValue and maxValue.
// If not, all fields are set to the missing value.

void DsRadarMsg::censorBeamData(const string &censorFieldName,
                                double minValue,
                                double maxValue)

{



  if (!paramsSet) {
    // not fully filled out
    return;
  }

  // search for censor field
  
  int censorIndex = -1;
  DsFieldParams *censorParams = NULL;
  for (int ii = 0; ii < (int) fieldParams.size(); ii++) {
    if (fieldParams[ii]->name == censorFieldName) {
      censorParams = fieldParams[ii];
      censorIndex = ii;
      break;
    }
  }

  if (censorIndex < 0) {
    // censor field does not exist - do nothing
    return;
  }

  // make sure beam data is correct size

  int nGates = radarParams.numGates;
  int byteWidth = censorParams->byteWidth;
  int nFields = (int) fieldParams.size();
  int expectedNBytes = byteWidth * nGates * nFields;
  if (expectedNBytes !=  radarBeam.getDataNbytes()) {
    cerr << "ERROR - DsRadarMsg::censorBeamData" << endl;
    cerr << "expectedNBytes !=  radarBeam.getDataNbytes()" << endl;
    cerr << "expectedNBytes: " << expectedNBytes << endl;
    cerr << "radarBeam.getDataNbytes(): " << radarBeam.getDataNbytes() << endl;
    return;
  }
  
  // compute censor field values

  double missingDouble = -9999.0;
  int missingCensor = censorParams->missingDataValue;
  vector<double> censorVals;
  if (byteWidth == 4) {

    fl32 *data = (fl32 *) radarBeam.getData() + censorIndex;
    for (int ii = 0; ii < nGates; ii++, data += nFields) {
      if (*data == missingCensor) {
        censorVals.push_back(missingDouble);
      } else {
        double val = *data;
        censorVals.push_back(val);
      }
    }

  } else if (byteWidth == 2) {

    ui16 *data = (ui16 *) radarBeam.getData() + censorIndex;
    double scale = censorParams->scale;
    double bias = censorParams->bias;
    for (int ii = 0; ii < nGates; ii++, data += nFields) {
      if (*data == missingCensor) {
        censorVals.push_back(missingDouble);
      } else {
        double val = *data * scale + bias;
        censorVals.push_back(val);
      }
    }

  } else if (byteWidth == 1) {

    ui08 *data = (ui08 *) radarBeam.getData() + censorIndex;
    double scale = censorParams->scale;
    double bias = censorParams->bias;
    for (int ii = 0; ii < nGates; ii++, data += nFields) {
      if (*data == missingCensor) {
        censorVals.push_back(missingDouble);
      } else {
        double val = *data * scale + bias;
        censorVals.push_back(val);
      }
    }

  } else {

    return; // byte width not supported

  }

  // censor all fields based on censor values
  
  for (int ii = 0; ii < nGates; ii++) {

    // check for censoring condition
    
    double censorVal = censorVals[ii];
    if (censorVal == missingDouble) {
      // already censored
      continue;
    }
    if (censorVal >= minValue && censorVal <= maxValue) {
      // do not censor
      continue;
    }

    // apply censoring to this gate
    
    if (byteWidth == 4) {
      
      fl32 *data = (fl32 *) radarBeam.getData() + ii * nFields;
      for (int jj = 0; jj < nFields; jj++, data++) {
        *data = fieldParams[jj]->missingDataValue;
      }
      
    } else if (byteWidth == 2) {
      
      ui16 *data = (ui16 *) radarBeam.getData() + ii * nFields;
      for (int jj = 0; jj < nFields; jj++, data++) {
        *data = fieldParams[jj]->missingDataValue;
      }
      
    } else {
      
      ui08 *data = (ui08 *) radarBeam.getData() + ii * nFields;
      for (int jj = 0; jj < nFields; jj++, data++) {
        *data = fieldParams[jj]->missingDataValue;
      }
      
    }

  } // ii

}

