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
// Class SSTData
//
// Author:  Curtis Caravone
// Date:    1/30/2003
//
// SSTData holds the Sea Surface Temperature data from a
// NAVOCEANO MCSST L3 data file. 
// See http://podaac.jpl.nasa.gov/navoceano_mcsst/navoceano_news.html
// for more information about the file format.
//
//

#include "SSTData.hh"
#include <stdio.h>
#include <stdexcept>

/********
 * Loads SST data from the given file into internal arrays.
 * The file may not contain the interpolated data.  In this case,
 * interpAvailable() would return false.
 *
 * dataFile -- A file containing the data
 */
SSTData::SSTData(FILE *dataFile) {
    _interp_available = true;

    _sst = NULL;
    _numPoints = NULL;
    _anomaly = NULL;
    _interpSST = NULL;
    _interpAnomaly = NULL;

    _sst_raw = NULL;
    _numPoints_raw = NULL;
    _anomaly_raw = NULL;
    _interpSST_raw = NULL;
    _interpAnomaly_raw = NULL;

    size_t chunk_size = SST_ROWS * SST_COLUMNS;

    _sst_raw = new ui08[chunk_size];
    _sst = new float[chunk_size];
    if (fread(_sst_raw, sizeof(ui08), chunk_size, dataFile) == chunk_size) {
        for (int i = 0; i < chunk_size; i++) {
            _sst[i] = (_sst_raw[i] == 0 ?
            MISSING_VALUE : _sst_raw[i] * SST_SLOPE + SST_INT);
        }
    } else {
        delete [] _sst_raw;
        delete [] _sst;
        throw runtime_error("Error reading SST file: unexpected end of file");
    }

    _numPoints_raw = new ui08[chunk_size];
    _numPoints = new int[chunk_size];
    if (fread(_numPoints_raw, sizeof(ui08), chunk_size, dataFile) == chunk_size) {
        for (int i = 0; i < chunk_size; i++) {
            _numPoints[i] = _numPoints_raw[i];
        }
    } else {
        delete [] _sst_raw;
        delete [] _sst;
        delete [] _numPoints_raw;
        delete [] _numPoints;
        throw runtime_error("Error reading SST file: unexpected end of file");
    }

    _anomaly_raw = new ui08[chunk_size];
    _anomaly = new float[chunk_size];
    if (fread(_anomaly_raw, sizeof(ui08), chunk_size, dataFile) == chunk_size) {
        for (int i = 0; i < chunk_size; i++) {
            _anomaly[i] = _anomaly_raw[i] == 0 ?
            MISSING_VALUE : _anomaly_raw[i] * ANOMALY_SLOPE + ANOMALY_INT;
        }
    } else {
        delete [] _sst_raw;
        delete [] _sst;
        delete [] _numPoints_raw;
        delete [] _numPoints;
        delete [] _anomaly_raw;
        delete [] _anomaly;
        throw runtime_error("Error reading SST file: unexpected end of file");
    }

    _interpSST_raw = new ui08[chunk_size];
    _interpSST = new float[chunk_size];
    if (fread(_interpSST_raw, sizeof(ui08), chunk_size, dataFile) == chunk_size) {
        for (int i = 0; i < chunk_size; i++) {
            _interpSST[i] = _interpSST_raw[i] == 0 ?
            MISSING_VALUE : _interpSST_raw[i] * SST_SLOPE + SST_INT;
        }
    } else {
        // This just means that the interpolated data was not available, which is ok.
        _interp_available = false;
        delete [] _interpSST_raw;
        delete [] _interpSST;
        _interpSST_raw = NULL;
        _interpSST = NULL;
        return;
    }

    _interpAnomaly_raw = new ui08[chunk_size];
    _interpAnomaly = new float[chunk_size];
    if (fread(_interpAnomaly_raw, sizeof(ui08), chunk_size, dataFile) == chunk_size) {
        for (int i = 0; i < chunk_size; i++) {
            _interpAnomaly[i] = _interpAnomaly_raw[i] == 0 ?
            MISSING_VALUE : _interpAnomaly_raw[i] * ANOMALY_SLOPE + ANOMALY_INT;
        }
    } else {
        delete [] _sst_raw;
        delete [] _sst;
        delete [] _numPoints_raw;
        delete [] _numPoints;
        delete [] _anomaly_raw;
        delete [] _anomaly;
        delete [] _interpSST_raw;
        delete [] _interpSST;
        delete [] _interpAnomaly_raw;
        delete [] _interpAnomaly;
        throw runtime_error("Error reading SST file: unexpected end of file");
    }
}

    
/********
 * Returns true if the interpolated fields are available.  If they
 * are not, then calling getInterpSST(), getInterpSSTAnomaly(),
 * getRawInterpSST(), or getRawInterpSSTAnomaly()
 * would return NULL.
 */
bool SSTData::interpAvailable() {
    return _interp_available;
}

/********
 * Returns a pointer to a SST_ROWS by SST_COLUMNS array containing 
 * Sea Surface Temperatures in degrees Celsius.  Missing values are
 * filled with MISSING_VALUE.
 */
float *SSTData::getSST() {
    return _sst;
}

/********
 * Returns a pointer to a SST_ROWS by SST_COLUMNS byte array containing
 * the raw SST values.  To convert to degrees Celsius, use
 * raw_value * SST_SLOPE + SST_INT.
 */
ui08 *SSTData::getRawSST() {
    return _sst_raw;
}

/********
 * Returns a pointer to a SST_ROWS by SST_COLUMNS array containing
 * the number of points per bin ???
 */
int *SSTData::getNumPoints() {
    return _numPoints;
}

/********
 * Returns a pointer to a SST_ROWS by SST_COLUMNS byte array containing
 * the raw NumPoints values.  These values do not need to be converted.
 * This method is just here for completeness.
 */
ui08 *SSTData::getRawNumPoints() {
    return _numPoints_raw;
}

/********
 * Returns a pointer to a SST_ROWS by SST_COLUMNS array containing
 * Sea Surface Temperature Anomaly values (degrees C).  Missing values
 * are filled with MISSING_VALUE.
 */ 
float *SSTData::getSSTAnomaly() {
    return _anomaly;
}

/********
 * Returns a pointer to a SST_ROWS by SST_COLUMNS byte array containing
 * the raw SST anomaly values.  To convert to degrees Celsius, use
 * raw_value * ANOMALY_SLOPE + ANOMALY_INT.
 */
ui08 *SSTData::getRawSSTAnomaly() {
    return _anomaly_raw;
}

/********
 * Returns a pointer to a SST_ROWS by SST_COLUMNS array containing
 * Interpolated Sea Surface Temperature values (degrees C), or
 * NULL if that data is unavailable.  Availability can be tested with
 * interpAvailable().  Missing values are filled with MISSING_VALUE.
 */
float *SSTData::getInterpSST() {
    return _interpSST;
}

/********
 * Returns a pointer to a SST_ROWS by SST_COLUMNS byte array containing
 * the raw interpolated SST values.  To convert to degrees Celsius, use
 * raw_value * SST_SLOPE + SST_INT.
 */
ui08 *SSTData::getRawInterpSST() {
    return _interpSST_raw;
}

/********
 * Returns a pointer to a SST_ROWS by SST_COLUMNS array containing
 * Interpolated Sea Surface Temperature Anomaly values (degrees C),
 * or NULL if that data is unavailable.  Availability can be tested
 * with interpAvailable().   Missing values are filled with MISSING_VALUE.
 */
float *SSTData::getInterpSSTAnomaly() {
    return _interpAnomaly;
}

/********
 * Returns a pointer to a SST_ROWS by SST_COLUMNS byte array containing
 * the raw interpolated SST anomaly values.  To convert to degrees Celsius,
 * use raw_value * ANOMALY_SLOPE + ANOMALY_INT.
 */
ui08 *SSTData::getRawInterpSSTAnomaly() {
    return _interpAnomaly_raw;
}

/********
 * Deletes the internal arrays of values, thus invalidating any pointers
 * returned by the getXXX() methods.
 */
SSTData::~SSTData() {
    delete [] _sst;
    delete [] _numPoints;  
    delete [] _anomaly;
    delete [] _interpSST;
    delete [] _interpAnomaly;

    delete [] _sst_raw;
    delete [] _numPoints_raw;
    delete [] _anomaly_raw;
    delete [] _interpSST_raw;
    delete [] _interpAnomaly_raw;
}


