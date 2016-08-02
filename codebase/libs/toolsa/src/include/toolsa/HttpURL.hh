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
////////////////////////////////////////////////////////////////////////////
// HTTPURL: Subclassed off of URL.hh
////////////////////////////////////////////////////////////////////////////

#ifndef HTTP_URL_INCLUDED
#define HTTP_URL_INCLUDED

#include <toolsa/URL.hh>
using namespace std;

class HttpURL : public URL {

public:

  // default constructor

  HttpURL();

  // constructor with URL supplied

  HttpURL(const string &urlStr);
  
  // NOTE: copy constructor and assignment are automatically
  // supplied by compiler since all members are OK for copy and
  // assignment
  
  // assignment with URL supplied
  inline void operator=(const string &urlStr)
  {
    setURLStr(urlStr);
  }

  // destructor - virtual
  
  virtual ~HttpURL();

  // Set the URL string, and decode it into its component parts.
  // sets valid flag appropriately
  // returns 0 on success, -1 on failure
  
  virtual int setURLStr(const string &urlStr);
  
  // Get the string representation of the URL.
  // Computes string from previously stored components
  // Returns:
  //   On success: a string representing the URL
  //   On error:
  //      An empty string if the URL has no protocol,
  //      or if the subclass cannot encode the protocol-specific part.

  virtual string getURLStr() const;

  // Check the URL for consistency.
  // If URL member is not valid, try to create a valid
  // one from the stored properties.
  // Sets the _isValid flag appropriately.

  virtual bool checkValid();
  
  // Check a URL string for consistency.
  // Static method. Can be called without object.

  static bool checkValid(string url);
  
  // print

  virtual void print(ostream &out) const;

protected:

};

#endif
