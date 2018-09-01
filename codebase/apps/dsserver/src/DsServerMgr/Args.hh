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
/////////////////////////////////////////////////////////////
// Args.h: Command line object
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 1998
//   Paddy McCarthy Modified Sept 1998 to remove public data members.
//
/////////////////////////////////////////////////////////////

#ifndef ArgsINCLUDED
#define ArgsINCLUDED

#include <cstdio>
#include <toolsa/os_config.h>
#include <tdrp/tdrp.h>
using namespace std;

class Args {

protected:
    char *_appName;
    char *_paramsFilePath;
            
    bool _okay;
    bool _showUsage;
 
    tdrp_override_t _override;
  
public:

    Args (int argc, char **argv);
    inline bool isOkay() const;
    inline bool isShowUsage() const;
    inline char * getAppName() const;
    inline char * getParamsFilePath() const;
    inline time_t getStartTime() const;
    inline time_t getEndTime() const;
    inline tdrp_override_t * getOverride() const;

    void usage(FILE *out);

protected:
    time_t parseTime(char *time_str, char *label);

private:
    Args();                                            
    Args(const Args & orig);
    Args & operator=(const Args & orig);
};

inline bool Args::isOkay() const
{
    return _okay;
}

inline bool Args::isShowUsage() const
{
    return _showUsage;
}

inline char * Args::getAppName() const
{
    return _appName;
}

inline char * Args::getParamsFilePath() const
{
    return _paramsFilePath;
}

// Todo: Fix this so not so dangerous.
inline tdrp_override_t * Args::getOverride() const
{
    return (tdrp_override_t *) &_override;
}

#endif
