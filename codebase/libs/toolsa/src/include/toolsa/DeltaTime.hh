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

#ifndef DeltaTimeINCLUDED
#define DeltaTimeINCLUDED

#include <iostream>

using namespace std;

class DeltaTime {

  public:

    enum TimeUnit { Seconds, Minutes, Hours, Days, Weeks };

    DeltaTime();
    DeltaTime(long seconds);
    DeltaTime(const DeltaTime & orig);
    DeltaTime(long duration, TimeUnit units);
    ~DeltaTime();
    
    long getDuration() const         { return _duration;     }
    void setDuration(long duration)  { _duration = duration; }

    TimeUnit getUnits() const        { return _units;        }
    void setUnits(TimeUnit units)    { _units = units;       }

    long getDurationInSeconds() const;

    bool operator == (const DeltaTime &other) const;
    bool operator != (const DeltaTime &other) const;
    DeltaTime & operator = (const DeltaTime &other);
    
   friend ostream& operator<< (ostream&, const DeltaTime*);
   friend ostream& operator<< (ostream&, const DeltaTime&);

  protected:
    long     _duration;
    TimeUnit _units;
};

#endif
