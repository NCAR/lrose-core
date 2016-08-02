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
////////////////////////////////////////////////////////////////////////////////
// X_Resource.H : CLass to support X resource parameters 
//
// -F. Hage Nov 1994
//
// Keyword: Runtime default value database management, access
///Keyword: XResources, extraction, X11
//
// These class does not have to be used with an X application. The
// Xrm....() functions  are standalone. All one has to do is to link in
// libX11.a to the appication. 
//
//
// application_name_or_class.debug_flag:         1
// application_name_or_class.bias:       -104.761730
// application_name_or_class.fatal_error_message:    Panic! Can't continue, fatal error
//
// Applicatioon Example:
//    int  max_rows;
//    double  bias;
//    char  *label;
//    X_res_db db("File_name");
//    db.extract(debug_flag,20,"*.debug_flag");
//    db.extract(fatal_error_message,"Death and Destruction awaits us","*.fatal_error_message");
//    if(! db.extract(bias,0.0,"*.bias")) // Warning - Using default value;
//     


#include <cstdio>
#include <cstdlib>
#include <cerrno>

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>

#include <toolsa/Except.hh>
using namespace std;
class X_res_db {
    XrmDatabase db;

public:
 // constructors
    X_res_db(char *file_name);

 // Set functions
    // none
 // Access functions
    // Overloaded extract functions for setting a value based in the input string
    int extract(int& value, int def, char* match_string);  // returns 0 if using defualt, 1 if set from data base
    int extract(long& value, long def, char* match_string);  // returns 0 if using defualt, 1 if set from data base
    int extract(long& value, int def, char* match_string);  // returns 0 if using defualt, 1 if set from data base
    int extract(float& value, float def, char* match_string);  // returns 0 if using defualt, 1 if set from data base
    int extract(double& value, double def, char* match_string);  // returns 0 if using defualt, 1 if set from data base
    int extract(char*& value, char* def, char* match_string);  // returns 0 if using defualt, 1 if set from data base

 // destructors
    ~X_res_db();
         
};
