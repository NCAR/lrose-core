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

#ifndef AcarsDataPointINCLUDED
#define AcarsDataPointINCLUDED

// 
// C++ wrapper for the ac_data format. This class includes functionality
//     to load data from the ldm ascii files, and send the data to the
//     acars java applet.
// 

// Have to include these here b/c rap headers don't have
//   includes to satisfy their needs.
//
#include <cstdio>
#include <iostream>
using namespace std;

#ifndef ac_data_h
# include <rapformats/ac_data.h>
#endif

#ifndef umisc_h
# include <toolsa/umisc.h>
#endif

class AcarsDataPoint {

protected:
    int _year,
        _month,
        _day,
        _hour,
        _minute,
        _second;
    bool _isValid;
    ac_data_t _data;
    mutable date_time_t _timestamp;
    static int _acars_data_uninitialized;

    void getWindAsBarb(float * az, float * vel) const;
    void setWindAsBarb(float az, float vel);

public:
    AcarsDataPoint();
    AcarsDataPoint(const date_time_t & timestamp, const ac_data_t & data);
    ~AcarsDataPoint();

    void initData();

    inline bool isValid() const;

    bool readAscii(istream & is);
    bool insertSPDB(char * dataDir, int validitySecs) const;
    bool sendToApplet(ostream & os) const;
    bool sendToAppletBinary(ostream & os) const;

    inline const ac_data_t & getData() const;
    inline const date_time_t & getTimestamp() const;

    void dump(ostream & os = cerr) const;

};

inline bool AcarsDataPoint::isValid() const
{
    return _isValid;
}

inline const ac_data_t & AcarsDataPoint::getData() const
{
    return _data;
}

inline const date_time_t & AcarsDataPoint::getTimestamp() const
{
    return _timestamp;
}

#endif
