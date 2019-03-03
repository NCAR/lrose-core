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
/*********************************************************************
 * BeamWriter : Class of objects that write beam data in the Dsr format.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <toolsa/DateTime.hh>
#include <stdlib.h>

#include "BeamWriter.hh"

#include "DualPol1Products.hh"
#include "DualPol3Products.hh"
#include "DualPolFull1Products.hh"
#include "DualPrtProducts.hh"
#include "NewSimpleProducts.hh"
#include "PolyProducts.hh"
#include "SimpleProducts.hh"

using namespace std;


BeamWriter::BeamWriter() :
  _radarParamsInitialized(false),
  _fieldParamsInitialized(false),
  _debug(false),
  _printSummaryFlag(false),
  _summaryInterval(0),
  _prevVolNum(-1),
  _prevTiltNum(-1),
  _prevElevation(-1.0),
  _prevAzimuth(-1.0),
  _overrideLocation(false),
  _reflScale(1.0),
  _reflBias(0.0),
  _velScale(1.0),
  _velBias(0.0),
  _swScale(1.0),
  _swBias(0.0),
  _coherentReflScale(1.0),
  _coherentReflBias(0.0),
  _ncpScale(1.0),
  _ncpBias(0.0),
  _powerScale(1.0),
  _powerBias(1.0),
  _radarParams(&(_radarMsg.getRadarParams())),
  _radarBeam(&(_radarMsg.getRadarBeam())),
  _radarQueue(0),
  _heading_correction(0.0),
  _use_dynamic_heading(false)
{
}

BeamWriter::~BeamWriter() 
{
  delete _radarQueue;
}

bool BeamWriter::initRadarParams(const int radar_id,
				 const string scan_type_name,
				const string dynamic_origin_filename,
			const string dynamic_heading_filename,
			const bool use_dynamic_heading,
                        const bool use_dynamic_origin)
{
  _radarParams->radarType = DS_RADAR_GROUND_TYPE;
  _radarParams->numFields = NUM_OUTPUT_FIELDS;     // number of fields
  _radarParams->scanMode = DS_RADAR_SURVEILLANCE_MODE;

  _radarParams->radarId = radar_id;                // unique number
  _radarParams->scanTypeName = scan_type_name;     // name of scanType (string)

  _radarParamsInitialized = true;
 
  _dynamic_origin_filename = dynamic_origin_filename;
  _dynamic_heading_filename = dynamic_heading_filename;
  _use_dynamic_heading = use_dynamic_heading;
  _use_dynamic_origin = use_dynamic_origin;
  return true;
}


bool BeamWriter::initFieldParams(const double dbz_scale,
				 const double dbz_bias,
				 const double vel_scale,
				 const double vel_bias,
				 const double sw_scale,
				 const double sw_bias,
				 const double coherent_dbz_scale,
				 const double coherent_dbz_bias,
				 const double ncp_scale,
				 const double ncp_bias,
				 const double power_scale,
				 const double power_bias)
{
  // Initialize output field parameters.  The MUST match the
  // order in the output_field_offsets_t enumeration!!

  vector< DsFieldParams* > &field_params = _radarMsg.getFieldParams();
  DsFieldParams*  fparams;

  fparams = new DsFieldParams("DBZ", "dBZ", 
			      dbz_scale, 
			      dbz_bias );
  field_params.push_back(fparams);

  _reflScale = dbz_scale;
  _reflBias = dbz_bias;

  fparams = new DsFieldParams("VEL", "m/s", 
			      vel_scale, 
			      vel_bias );
  field_params.push_back(fparams);

  _velScale = vel_scale;
  _velBias = vel_bias;

  fparams = new DsFieldParams("SPW", "m/s", 
			      sw_scale,
			      sw_bias );
  field_params.push_back(fparams);

  _swScale = sw_scale;
  _swBias = sw_bias;

  fparams = new DsFieldParams("COH DBZ", "dBZ", 
			      coherent_dbz_scale,
			      coherent_dbz_bias );
  field_params.push_back(fparams);

  _coherentReflScale = coherent_dbz_scale;
  _coherentReflBias = coherent_dbz_bias;

  fparams = new DsFieldParams("NCP", "none",
			      ncp_scale,
			      ncp_bias );
  field_params.push_back(fparams);

  _ncpScale = ncp_scale;
  _ncpBias = ncp_bias;

  fparams = new DsFieldParams("POWER", "none",
			      power_scale,
			      power_bias );
  field_params.push_back(fparams);

  _powerScale = power_scale;
  _powerBias = power_bias;

  _fieldParamsInitialized = true;
  
  return true;
}


void BeamWriter::setLatLonOverride(const double latitude,
				   const double longitude,
				   const double altitude)
{
  _radarParams->latitude  = latitude;
  _radarParams->longitude = longitude;
  _radarParams->altitude  = altitude;

  _overrideLocation   = true;
}

void BeamWriter::setDiagnostics(const bool print_summary,
				const int summary_interval)
{
  _printSummaryFlag = print_summary;
  
  _summaryInterval = summary_interval;
}


// Read two simple files;
// 1. A location file - Contains lat, lon altitude 
// 2. A Heading Correction file
//  
void BeamWriter::loadOriginOverride(void)
{
  FILE *locfile;
  char buf[256];
  char buf2[256];
  double lat,lon,heading;
  char *ptr1,*ptr2;
  
  memset(buf,0,256);
  if(_dynamic_origin_filename.size() > 2) {
    if((locfile = fopen(_dynamic_origin_filename.c_str(),"r")) != NULL) {
         fseek(locfile,-64,SEEK_END);
         fread(buf,64,1,locfile);
	 ptr1 = rindex(buf,10); // find the last CR
         *ptr1 = '0';  //terminate it there
         ptr1 = rindex(buf,10); // Next to last CR
         ptr1++; // move to the first char of data string
         while(*ptr1 == ' ') ptr1++; // skip leading spaces
         memset(buf2,0,256);
         ptr2 = buf2;
         while(*ptr1 != ' ') *ptr2++ = *ptr1++;
         lat = atof(buf2);
         while(*ptr1 == ' ') ptr1++; // skip leading spaces
         memset(buf2,0,256);
         ptr2 = buf2;
         while(*ptr1 != ' ') *ptr2++ = *ptr1++;
         lon = atof(buf2);
         if(lat >= -90.0 && lat <= 90.0 && lon >= -360 && lon <=360)  {
            _radarParams->latitude  = lat;
	    _radarParams->longitude = lon;
         }
	 fclose(locfile);
    }
  }

  memset(buf,0,256);
  if(_dynamic_heading_filename.size() > 2) {
     if ((locfile = fopen(_dynamic_heading_filename.c_str(),"r")) != NULL) {
         fseek(locfile,-64,SEEK_END);
         fread(buf,64,1,locfile);
	 ptr1 = rindex(buf,10); // find the last CR
         *ptr1 = '0';  //terminate it there
         ptr1 = rindex(buf,10); // Next to last CR
         ptr1++; // move to the first char of data string
         while(*ptr1 == ' ') ptr1++; // skip leading spaces
         memset(buf2,0,256);
         ptr2 = buf2;
         while(*ptr1 != ' ') *ptr2++ = *ptr1++;
         heading = atof(buf2);
	 fclose(locfile);
         if (heading > -270 && heading < 360 ) _heading_correction = heading;
      }
  }
  _useDynamicLocation = true;
}


void BeamWriter::_printSummary()
{
  static int count = 0;

  if (count == 0)
  {
    fprintf(stderr,
	    " Vol Tilt El_tgt El_act     Az Ngat Gspac  PRF     Date     Time\n");

    //
    // Parse the time of the beam
    //
    DateTime  data_time(_radarBeam->dataTime);

    fprintf(stderr,
	    "%4ld %4ld %6.2f %6.2f %6.2f %4ld %5ld %4ld %.2ld/%.2ld/%.2ld %.2ld:%.2ld:%.2ld\n",
	    (long) _radarBeam->volumeNum,
	    (long) _radarBeam->tiltNum,
	    (double) _radarBeam->targetElev,
	    (double) _radarBeam->elevation,
	    (double) _radarBeam->azimuth,
	    (long) _radarParams->numGates,
	    (long) (_radarParams->gateSpacing * 1000),
	    (long) (_radarParams->pulseRepFreq + 0.5),
	    (long) data_time.getYear(),
	    (long) data_time.getMonth(),
	    (long) data_time.getDay(),
	    (long) data_time.getHour(),
	    (long) data_time.getMin(),
	    (long) data_time.getSec());
  }

  count++;
  if (count == _summaryInterval)
  {
    count = 0;
  }

  return;
}


bool BeamWriter::_putStartEndFlags(const int volume_num,
				   const int tilt_num,
				   const time_t data_time) const
{
  bool new_tilt = false;

  if (tilt_num != _prevTiltNum)
  {
    // Set the new_tilt flag so we know whether to send out the params
    // message down below.

    new_tilt = true;

    // Write the end of tilt flag if this isn't the first
    // beam we've processed.

    if (_prevTiltNum >= 0)
    {
      if (_debug || _printSummaryFlag)
      {
//	cerr << endl;
	cerr << "-------------------> end of tilt "
	     << _prevTiltNum << endl;
      }

      _radarQueue->putEndOfTilt(_prevTiltNum);
    }
  }

  // Check for this being the end of the volume

  if (volume_num != _prevVolNum)
  {
    if (_debug || _printSummaryFlag)
    {
//	cerr << endl;
      cerr << "-------------------> end of volume "
	   << _prevVolNum << endl;
    }

    _radarQueue->putEndOfVolume(_prevVolNum);

    // Output the beginning of volume flag.  We do this whenever the
    // volume number changes, even on the first volume read.

    if (_debug || _printSummaryFlag)
    {
//	cerr << endl;
      cerr << "-------------------> start of volume "
	   << volume_num << endl;
    }

    _radarQueue->putStartOfVolume(volume_num, data_time);
  }

  // Finally, output the beginning of tilt flag

  if (tilt_num != _prevTiltNum)
  {
    if (_debug || _printSummaryFlag)
    {
//      cerr << endl;
      cerr << "-------------------> start of tilt "
	   << tilt_num << endl;
    }

    _radarQueue->putStartOfTilt(tilt_num, data_time);
  }

  return new_tilt;
}


bool BeamWriter::_putStartEndFlags(const int volume_num,
				   const time_t data_time) const
{
  bool new_volume = false;

  // Check for this being the end of the volume

  if (volume_num != _prevVolNum)
  {
    new_volume = true;
    
    if (_debug || _printSummaryFlag)
    {
//	cerr << endl;
      cerr << "-------------------> end of volume "
	   << _prevVolNum << endl;
    }

    _radarQueue->putEndOfVolume(_prevVolNum);

    // Output the beginning of volume flag.  We do this whenever the
    // volume number changes, even on the first volume read.

    if (_debug || _printSummaryFlag)
    {
//	cerr << endl;
      cerr << "-------------------> start of volume "
	   << volume_num << endl;
    }

    _radarQueue->putStartOfVolume(volume_num, data_time);
  }

  return new_volume;
}
