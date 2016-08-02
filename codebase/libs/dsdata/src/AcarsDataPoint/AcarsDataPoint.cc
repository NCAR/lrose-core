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

#include <ctype.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#ifndef AcarsDataPointINCLUDED
# include <dsdata/AcarsDataPoint.hh>
#endif

#ifndef BIGEND_H
# include <dataport/bigend.h>
#endif

#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>
using namespace std;

int AcarsDataPoint::_acars_data_uninitialized = AC_DATA_UNKNOWN_VALUE;

static double degsInRad = 360 / (3.1415926 * 2);
static double kmInFoot  = 0.0003048;
static double feetInKm  = 3280.8;

AcarsDataPoint::AcarsDataPoint()
{
    // Initialize and mark the data as invalid.
    initData();
    _isValid = false;
}

AcarsDataPoint::AcarsDataPoint(const date_time_t & timestamp,
                               const ac_data_t & data)
{
    // Initialize and mark the data as invalid.
    initData();
    _isValid = false;

    // Do bitwise copy of timestamp and data since it is all static data.
    _timestamp = timestamp;
    _data = data;

    // Verify the timestamp.
    int valid = uvalid_datetime(&_timestamp);
    if (valid == 0) {                                                  
        cerr << "Error: Data has bad timestamp in overridden constructor.";
        cerr << endl;
    }
    else {
        _isValid = true;
    }
}

AcarsDataPoint::~AcarsDataPoint()
{
    // No dynamically-allocated data.
}

void AcarsDataPoint::initData()
{
    // Initialize the data.
    strcpy(_data.callsign, "None");
    _year                  = AcarsDataPoint::_acars_data_uninitialized;
    _month                 = AcarsDataPoint::_acars_data_uninitialized;
    _day                   = AcarsDataPoint::_acars_data_uninitialized;
    _hour                  = AcarsDataPoint::_acars_data_uninitialized;
    _minute                = AcarsDataPoint::_acars_data_uninitialized;
    _second                = 0;
    _data.alt_type         = ALT_NORMAL;
    _data.client_data_type = AcarsDataPoint::_acars_data_uninitialized;
    _data.client_data_len  = AcarsDataPoint::_acars_data_uninitialized;
    _data.lat              = AcarsDataPoint::_acars_data_uninitialized;
    _data.lon              = AcarsDataPoint::_acars_data_uninitialized;
    _data.alt              = AcarsDataPoint::_acars_data_uninitialized;
    _data.ground_speed     = AcarsDataPoint::_acars_data_uninitialized;
    _data.heading          = AcarsDataPoint::_acars_data_uninitialized;
    _data.temperature      = AcarsDataPoint::_acars_data_uninitialized;
    _data.dew_point        = AcarsDataPoint::_acars_data_uninitialized;
    _data.wind_u           = AcarsDataPoint::_acars_data_uninitialized;
    _data.wind_v           = AcarsDataPoint::_acars_data_uninitialized;
    _data.max_turb         = AcarsDataPoint::_acars_data_uninitialized;
    _data.avg_turb         = AcarsDataPoint::_acars_data_uninitialized;
    _data.data_quality     = AC_DATA_DataOk;

}
bool AcarsDataPoint::readAscii(istream & is)
{
    // Todo: This routine should make an attempt to reset the istream
    //         pointer to the end of the line after failing to read
    //         a data point. At the moment, if a data point contains
    //         the wrong number of fields, the whole file will fail.
    //
    //       Alternatively, use regular expressions to parse each line.
    //

    // Cache the position where the read started.
    streampos start = is.tellg();

    is >> _data.origin;
    is >> _data.destination;
    is >> _data.callsign;
    is >> _year;
    is >> _month;
    is >> _day;
    is >> _hour;
    is >> _minute;
    is >> _data.lat;
    is >> _data.lon;
    is >> _data.avg_turb;
    is >> _data.max_turb;
    is >> _data.temperature;
    
    // Wind data comes in as direction and speed. Convert to vectors.
    double tmpAz, tmpMag;
    is >> tmpAz;
    is >> tmpMag;

    // Verify the wind data.
    if (tmpAz < 0.0 || tmpAz > 360.0 ||
        tmpMag < 0.0) {

        cerr << "Wind data is invalid: " << tmpAz << ":" << tmpMag << endl;

        // Add bit to the quality flag and clear values.
        _data.data_quality |= AC_DATA_WindBad;
        _data.wind_u = AcarsDataPoint::_acars_data_uninitialized;
        _data.wind_v = AcarsDataPoint::_acars_data_uninitialized;
    }
    else {
        setWindAsBarb(tmpAz, tmpMag);
    }

    // Data arrives in feet. Stored in km.
    double tmpAlt; // Altitude in feet.
    is >> tmpAlt;

    // Verify the altitude data (still in feet).
    if (tmpAlt < -1000.0 || tmpAlt > 50000.0) {

        cerr << "Altitude data is invalid: " << tmpAlt << endl;

        // Add bit to the quality flag and clear value.
        _data.data_quality |= AC_DATA_AltitudeBad;
        _data.alt = AcarsDataPoint::_acars_data_uninitialized;
    }
    else {
        _data.alt = tmpAlt * kmInFoot; // Convert to kilometers.
    }

    // Verify the lat/lon data.
    if (_data.lat < -90.0  ||
        _data.lat > 90.0   ||
        _data.lon < -180.0 ||
        _data.lon > 180.0) {

        cerr << "Lat/Lon data is invalid: " << _data.lat << "/"
             << _data.lon << endl;

        // Add bit to the quality flag and clear values.
        _data.data_quality |= AC_DATA_LatBad;
        _data.data_quality |= AC_DATA_LonBad;
        _data.lat = AcarsDataPoint::_acars_data_uninitialized;
        _data.lat = AcarsDataPoint::_acars_data_uninitialized;
        _data.lon = AcarsDataPoint::_acars_data_uninitialized;
        _data.lon = AcarsDataPoint::_acars_data_uninitialized;
    }                                                        

    // Verify the turbulence data.
    if (_data.avg_turb < 0.000 ||
        _data.avg_turb > 20.0 ||
        _data.max_turb < 0.000 ||
        _data.max_turb > 20.0) {

        cerr << "Turbulence data is invalid: Max: " << _data.max_turb
             << " Ave: " << _data.avg_turb << endl;

        // Add bit to the quality flag and clear values.
        _data.data_quality |= AC_DATA_TurbulenceBad;
        _data.avg_turb = AcarsDataPoint::_acars_data_uninitialized;
        _data.max_turb = AcarsDataPoint::_acars_data_uninitialized;
    }

    // Verify the temperature data.
    if (_data.temperature <= -100.0 || _data.temperature >= 100.0) {

        cerr << "Temperature data is invalid: " << _data.temperature << endl;

        // Add bit to the quality flag and clear value.
        _data.data_quality |= AC_DATA_TempBad;
        _data.temperature = AcarsDataPoint::_acars_data_uninitialized;
    }

    // Re-read the material and check for newline characters,
    //   which would indicate an incomplete data point.
    // 
    streampos end = is.tellg();
    is.seekg(start);
    bool stripped = false;
    for (int i = 0; i < (end - start); i++) {
        char ch;
        is.get(ch);

        // Check for leading whitespace (including newlines),
        //  or newlines not at beginning, which indicate bad data.
        // 
        if (!stripped) {
            if (!isspace(ch)) {
                stripped = true;
            }
        }
        else if (ch == '\n') {
            cerr << "Got extraneous newline when reading data point!" << endl;
            
            // Reset the input stream for the next point.
	    // Note: dixon - the following does not work under g++3.2.
            //   is.seekg(start + i);
	    // Therefore modified to following: first seek to start and
	    // then seek to offset i from start
	    is.seekg(start);
	    is.seekg(i, ios::cur);

            return false;
        }
    }

    is.seekg(end);

    // Check that seeking around caused no errors.
    if (is.eof()) {
        cerr << "Error: After newline check, got eof condition." << endl;
        return false;
    }
    if (is.fail()) {
        cerr << "Error: After newline check, got fail condition." << endl;
        return false;
    }

    // Validate the data's timestamp.
    //
    // date_time_t struct from /cvs/Reference/incs/rapmisc/date_time.h
    // 
    // typedef struct {
    //   int year, month, day, hour, min, sec;
    //   time_t unix_time;
    // } date_time_t;
    // 
    _timestamp.year  = _year;
    _timestamp.month = _month;
    _timestamp.day   = _day;
    _timestamp.hour  = _hour;
    _timestamp.min   = _minute;
    _timestamp.sec   = _second;
    int valid = uvalid_datetime(&_timestamp);
    if (valid == 0) {
        cerr << "Error: Data has bad timestamp." << endl;
        return false;
    }

    _isValid = true;
    return true;
}

bool AcarsDataPoint::insertSPDB(char * url, int validitySecs) const
{

    // Check if the data is valid.
    if (!isValid()) {
        cerr << "Error: Attempting to archive invalid data." << endl;
        return false;
    }

    // Convert the data to big-endian.
    // Make copy first, b/c this is const method.
    ac_data_t localData = _data;
    ac_data_to_BE(&localData);

    // put to SPDB url

    DsSpdb spdb;
    int data_type = ac_data_callsign_hash((char *) _data.callsign);
    time_t valid_time = uunix_time(&_timestamp);
    time_t expire_time = valid_time + validitySecs;
    if (spdb.put(url,
		 SPDB_AC_DATA_ID,
		 SPDB_AC_DATA_LABEL,
		 data_type, valid_time, expire_time,
		 sizeof(ac_data_t),
		 &localData)) {
      cerr << "Error: Could not archive data." << endl;
      cerr << "  " << spdb.getErrStr() << endl;
      return false;
    }

    return true;
}

bool AcarsDataPoint::sendToApplet(ostream & os) const
{
    os << uunix_time(&_timestamp) << " ";
    os << _data.lat               << " ";
    os << _data.lon               << " ";

    // Convert data from km to feet (needed by applet) if it is valid.
    if (_data.alt == AcarsDataPoint::_acars_data_uninitialized) {

        // Bad data. Send the data as it is.
        os << _data.alt;
    }
    else {
        double tmpAlt;
        tmpAlt = _data.alt * feetInKm;
        os << tmpAlt              << " ";
    }

    os << _data.temperature       << " ";

    // Convert the wind data from vector form if it is valid.
    if (_data.wind_u == AcarsDataPoint::_acars_data_uninitialized ||
        _data.wind_v == AcarsDataPoint::_acars_data_uninitialized) {

        // Bad data. Send the data as it is.
        os << _data.wind_u        << " ";
        os << _data.wind_v        << " ";
    }
    else {
        float az, vel;
        getWindAsBarb(&az, &vel);
        os << az                  << " ";
        os << vel                 << " ";
    }
    
    os << _data.avg_turb          << " ";
    os << _data.max_turb          << " ";

    // Data descriptor -- contains bits indicating which (if any) data is bad.
    os << _data.data_quality      << " ";

    return true;
}

bool AcarsDataPoint::sendToAppletBinary(ostream & os) const
{
    // 
    // Perform some unit conversions.
    // 

    // Convert data from km to feet (needed by applet) if it is valid.
    float tmpAlt;
    if (_data.alt == AcarsDataPoint::_acars_data_uninitialized) {

        // Bad data. Send the data as it is.
        tmpAlt = _data.alt;
    }
    else {
        tmpAlt = _data.alt * feetInKm;
    }
    BE_from_array_32(&tmpAlt, sizeof(tmpAlt));

    float tmpAz, tmpVel;
    // Convert the wind data from vector form if it is valid.
    if (_data.wind_u == AcarsDataPoint::_acars_data_uninitialized ||
        _data.wind_v == AcarsDataPoint::_acars_data_uninitialized) {

        tmpAz = _data.wind_u;
        tmpVel = _data.wind_v;
    }
    else {
        getWindAsBarb(&tmpAz, &tmpVel);
    }
    BE_from_array_32(&tmpAz, sizeof(tmpAz));
    BE_from_array_32(&tmpVel, sizeof(tmpVel));

    // Convert data to big-endian format.
    ac_data_t localData = _data;
    ac_data_to_BE(&localData);

    int ut = (int) uunix_time(&_timestamp);
    BE_from_array_32(&ut, sizeof(ut));
    // os << uunix_time(&_timestamp) << " ";
    os.write((const char *) &ut, (int) sizeof(ut));
    // os << localData.lat               << " ";
    os.write((const char *) &localData.lat, sizeof(localData.lat));
    // os << localData.lon               << " ";
    os.write((const char *) &localData.lon, sizeof(localData.lon));
    //     os << tmpAlt              << " ";
    os.write((const char *) &tmpAlt, sizeof(tmpAlt));
    // os << localData.temperature       << " ";
    os.write((const char *) &localData.temperature, sizeof(localData.temperature));
    //     os << tmpAz                  << " ";
    os.write((const char *) &tmpAz, sizeof(tmpAz));
    //     os << tmpVel                 << " ";
    os.write((const char *) &tmpVel, sizeof(tmpVel));
    // os << localData.avg_turb          << " ";
    os.write((const char *) &localData.avg_turb, sizeof(localData.avg_turb));
    // os << localData.max_turb          << " ";
    os.write((const char *) &localData.max_turb, sizeof(localData.max_turb));

    // Data descriptor -- contains bits indicating which (if any) data is bad.
    // os << localData.data_quality      << " ";
    os.write((const char *) &localData.data_quality, sizeof(localData.data_quality));

    return true;
}

void AcarsDataPoint::getWindAsBarb(float * az, float * vel) const
{
    // Todo: Use the following conversion instead?
    //        1) Determine length of vector S by pythagoras.
    //        2) Determine u and v by asin(u/S) = angle, etc.
    //        3) Determine the quadrant based on sign of u and v.
    double outAz,
           outVel,
           windU = _data.wind_u,
           windV = _data.wind_v;

    if (windV == 0 && windU == 0) {
        outAz = 0.0;
        outVel = 0.0;
    }
    else if (windV == 0) {
        outAz = (windU > 0) ? 270.0 : 90.0;
        outVel = fabs(windU);
    }
    else if (windU == 0) {
        outAz = (windV > 0) ? 180 : 0;
        outVel = fabs(windV);
    }
    else {
        outAz = atan(windU / windV);
        outVel = fabs(windU / sin(outAz));
        outAz = outAz * degsInRad;

        // Correct for negative azimuth values.
        if (outAz < 0) {
            outAz += 360;
        }

        // Add 180 if in quadrants 1 or 4.
        if ( (windU > 0 && windV > 0) ||
             (windU < 0 && windV > 0) ) {
            outAz = fmod((outAz + 180.0), 360.0);
        }
    }

    *az = outAz;
    *vel = outVel;
}

void AcarsDataPoint::setWindAsBarb(float az, float mag)
{
    // Todo: Make the vector have the correct units.
    //         Should be in meters/sec.

    // Convert wind direction and wind speed to vectors.
    _data.wind_u = -1 * mag * sin(az / degsInRad);
    _data.wind_v = -1 * mag * cos(az / degsInRad);

    // Convert the wind back to verify it.
    float azOut, magOut;
    getWindAsBarb(&azOut, &magOut);
    if (fabs(az - azOut) > .001 || fabs(mag - magOut) > .001) {
        cerr << "CONVERSION ERROR" << endl;
        cerr << "   Orig: " << az << ":" << mag << endl;
        cerr << "   Vect: " << _data.wind_u << ":" << _data.wind_v << endl;
        cerr << "   Back: " << azOut << ":" << magOut << endl;
    }

}

void AcarsDataPoint::dump(ostream & os /* == cerr */) const
{
    os << "AcarsDataPoint: " << _data.callsign 
       << " " << _data.origin << "->" << _data.destination << "." << endl;
    char buf[1000];
    sprintf(buf, "    Position: %f : %f\n"
                 "    Altitude: %f\n"
                 "    Speed/Dir: %f / %f\n"
                 "    Temp: %f\n"
                 "    Dew Point: %f\n"
                 "    Wind u/v: %f / %f\n"
                 "    Max/Avg Turb: %f / %f",
            _data.lat, _data.lon,
            _data.alt,
            _data.ground_speed, _data.heading,
            _data.temperature, 
            _data.dew_point, 
            _data.wind_u, _data.wind_v,
            _data.max_turb, _data.avg_turb);
    os << buf << endl;
}
