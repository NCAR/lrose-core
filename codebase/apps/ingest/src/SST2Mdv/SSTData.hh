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
// See http://podaac.jpl.nasa.gov/navoceano_mcsst
// for more information about the file format.
//
// Data is stored in a 1024x2048 global lat/lon grid.
//
//

#ifndef SSTDATA_H
#define SSTDATA_H

#include <stdio.h>
#include <dataport/port_types.h>

// Size of data fields
#define SST_ROWS 1024
#define SST_COLUMNS 2048

// Scaling factors for Temperature fields
#define SST_SLOPE .15
#define SST_INT -3.0

// Scaling factors for Anomaly fields
#define ANOMALY_SLOPE .15
#define ANOMALY_INT -20

// Missing values for raw and converted data
#define RAW_MISSING_VALUE 0
#define MISSING_VALUE -2000

class SSTData {

  public:

    /********
     * Loads SST data from the given file into internal arrays.
     * The file may not contain the interpolated data.  In this case,
     * interpAvailable() would return false.
     *
     * dataFile -- A file containing the data
     */
    SSTData(FILE *dataFile);

    /********
     * Returns true is the interpolated fields are available.  If they
     * are not, then calling getInterpSST() or getInterpSSTAnomaly()
     * would return NULL.
     */
    bool interpAvailable();

    /********
     * Returns a pointer to a SST_ROWS by SST_COLUMNS array containing 
     * Sea Surface Temperatures in degrees Celsius.  Missing values are
     * filled with MISSING_VALUE.
     */
    float *getSST();

    /********
     * Returns a pointer to a SST_ROWS by SST_COLUMNS byte array containing
     * the raw SST values.  To convert to degrees Celsius, use
     * raw_value * SST_SLOPE + SST_INT.
     */
    ui08 *getRawSST();

    /********
     * Returns a pointer to a SST_ROWS by SST_COLUMNS array containing
     * the number of points per bin ???
     */
    int *getNumPoints();

    /********
     * Returns a pointer to a SST_ROWS by SST_COLUMNS byte array containing
     * the raw NumPoints values.  These values do not need to be converted.
     * This method is just here for completeness.
     */
    ui08 *getRawNumPoints();

    /********
     * Returns a pointer to a SST_ROWS by SST_COLUMNS array containing
     * Sea Surface Temperature Anomaly values (degrees C).  Missing values
     * are filled with MISSING_VALUE.
     */
    float *getSSTAnomaly();

    /********
     * Returns a pointer to a SST_ROWS by SST_COLUMNS byte array containing
     * the raw SST anomaly values.  To convert to degrees Celsius, use
     * raw_value * ANOMALY_SLOPE + ANOMALY_INT.
     */
    ui08 *getRawSSTAnomaly();

    /********
     * Returns a pointer to a SST_ROWS by SST_COLUMNS array containing
     * Interpolated Sea Surface Temperature values (degrees C), or
     * NULL if that data is unavailable.  Availability can be tested with
     * interpAvailable().  Missing values are filled with MISSING_VALUE.
     */
    float *getInterpSST();

    /********
     * Returns a pointer to a SST_ROWS by SST_COLUMNS byte array containing
     * the raw interpolated SST values.  To convert to degrees Celsius, use
     * raw_value * SST_SLOPE + SST_INT.
     */
    ui08 *getRawInterpSST();

    /********
     * Return a pointer to a SST_ROWS by SST_COLUMNS array containing
     * Interpolated Sea Surface Temperature Anomaly values (degrees C),
     * or NULL if that data is unavailable.  Availability can be tested
     * with interpAvailable().   Missing values are filled with
     * MISSING_VALUE.
     */
    float *getInterpSSTAnomaly();

    /********
     * Returns a pointer to a SST_ROWS by SST_COLUMNS byte array containing
     * the raw interpolated SST anomaly values.  To convert to degrees Celsius,
     * use raw_value * ANOMALY_SLOPE + ANOMALY_INT.
     */
    ui08 *getRawInterpSSTAnomaly();

    /********
     * Deletes the internal arrays of values, thus invalidating any pointers
     * returned by the getXXX() methods.
     */
    ~SSTData();

  private:
    // converted values
    float *_sst;
    int *_numPoints;
    float *_anomaly;
    float *_interpSST;
    float *_interpAnomaly;

    // raw bytes values read from the file
    ui08 *_sst_raw;
    ui08 *_numPoints_raw;
    ui08 *_anomaly_raw;
    ui08 *_interpSST_raw;
    ui08 *_interpAnomaly_raw;

    bool _interp_available;

};

#endif
