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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 19:26:12 $
 *   $Id: HttpConnection.hh,v 1.7 2016/03/03 19:26:12 dixon Exp $
 *   $Revision: 1.7 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * HttpConnection.hh : HttpConnection methods.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2000
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef HttpConnection_HH
#define HttpConnection_HH

/*
 **************************** includes **********************************
 */


#include <map>
#include <ctime>

#include <toolsa/HttpURL.hh>
using namespace std;


/*
 ******************************* defines ********************************
 */

/*
 ******************************* structures *****************************
 */

/*
 ************************* global variables *****************************
 */

/*
 ***************************** function prototypes **********************
 */

/*
 ************************* class definitions ****************************
 */


class HttpConnection
{      
public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    INVALID_URL,
    CONNECTION_FAILURE,
    READ_REPLY_FAILURE,
    MEMORY_ALLOCATION_ERROR,
    READ_RESOURCE_DATA_ERROR,
    HTTP_ERROR
  } error_code_t;
  
  
  ////////////////////
  // Public methods //
  ////////////////////

  // Constructors

  HttpConnection(const HttpURL url,
		 const HttpURL *proxy_url = 0);

  // Destructors

  ~HttpConnection();


  ////////////////////////////
  // Data retrieval methods //
  ////////////////////////////

  // Retrieve the data at the given URL.  If a proxy URL was specified,
  // go through the proxy.

  void getData(const time_t last_data_time = 0);
  

  ////////////////////
  // Access methods //
  ////////////////////

  inline char *getLastData(void) const
  {
    return _lastData;
  }
  

  inline int getLastDataLength(void) const
  {
    return _lastDataLength;
  }
  

  inline time_t getLastDataTime(void) const
  {
    return _lastDataTime;
  }
  

  inline bool isError(void) const
  {
    return _isError;
  }
  

  inline error_code_t getErrorCode(void) const
  {
    return _errorCode;
  }
  

  inline int getHttpStatus(void) const
  {
    return _httpStatus;
  }
  

private:

  ///////////////////////
  // Private constants //
  ///////////////////////

  static const int HTTP_PORT;
  static const int HTTP_HEAD_BUF_SIZE;
  static const int HTTP_COMM_BUF_SIZE;
  static const int HTTP_DATA_BUF_SIZE;
  static const int HTTP_HEAD_READ_LEN;
  

  /////////////////////
  // Private members //
  /////////////////////

  // The URL(s) for the connection.  If _proxyUrl is "", don't use
  // a proxy in the communications.

  HttpURL _url;
  HttpURL *_proxyUrl;
  
  // Data retrieved from the server

  char *_lastData;
  int _lastDataLength;
  time_t _lastDataTime;
  
  // Error information

  bool _isError;
  error_code_t _errorCode;    // Value valid only if _isError is true
  int _httpStatus;            // Value valid only if _isError is true and
                              //   error_code is HTTP_ERROR
  

  // Objects used for converting time strings to time values

  static const int DATE_STRING_NUM_TOKENS;
  static const int DATE_STRING_YEAR_POS;
  static const int DATE_STRING_MONTH_POS;
  static const int DATE_STRING_DAY_POS;
  static const int DATE_STRING_TIME_POS;
  
  static const int TIME_STRING_NUM_TOKENS;
  static const int TIME_STRING_HOUR_POS;
  static const int TIME_STRING_MIN_POS;
  static const int TIME_STRING_SEC_POS;
  
  map< string, int, less<string> > _months;
  
  static const int MAX_TOKENS;
  static const int MAX_TOKEN_LEN;
  
  char **_dateTokens;
  char **_timeTokens;
  

  /////////////////////
  // Private methods //
  /////////////////////

  // Retrieve the data at the given URL without using a proxy.

  void _getData(const time_t last_data_time = 0);
  
  // Retrieve the string containing the indicated header information.
  //
  // Returns a pointer to the string containing the header information,
  // or 0 if that string wasn't found in the header.

  static char *_getHeaderString(char *header,
				const char *label);
  
  // Convert the given string to a time value.  The string is expected
  // to be something like "Thu, 10 Aug 2000 14:05:01 GMT".
  //
  // Returns the time value for the string if successful, or 0 if
  // unsuccessful.

  time_t _stringToTime(const char *time_string);
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("HttpConnection");
  }
  
};

# endif     /* HttpConnection_HH */
