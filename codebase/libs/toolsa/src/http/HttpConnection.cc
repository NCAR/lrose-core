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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/03 18:00:26 $
//   $Id: HttpConnection.cc,v 1.9 2016/03/03 18:00:26 dixon Exp $
//   $Revision: 1.9 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * HttpConnection.cc: HttpConnection methods.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2000
 *
 * Nancy Rehak
 *
 *********************************************************************/


#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>

#include <toolsa/HttpConnection.hh>
#include <toolsa/HttpURL.hh>
#include <toolsa/mem.h>
#include <toolsa/sockutil.h>
#include <toolsa/str.h>
#include <toolsa/udatetime.h>
using namespace std;


// Global constants

const int HttpConnection::HTTP_PORT = 80;
const int HttpConnection::HTTP_HEAD_BUF_SIZE = 2048;
const int HttpConnection::HTTP_COMM_BUF_SIZE = 1024;
const int HttpConnection::HTTP_DATA_BUF_SIZE = 1024;
const int HttpConnection::HTTP_HEAD_READ_LEN = 128;

const int HttpConnection::DATE_STRING_NUM_TOKENS = 6;
const int HttpConnection::DATE_STRING_YEAR_POS = 3;
const int HttpConnection::DATE_STRING_MONTH_POS = 2;
const int HttpConnection::DATE_STRING_DAY_POS = 1;
const int HttpConnection::DATE_STRING_TIME_POS = 4;

const int HttpConnection::TIME_STRING_NUM_TOKENS = 3;
const int HttpConnection::TIME_STRING_HOUR_POS = 0;
const int HttpConnection::TIME_STRING_MIN_POS = 1;
const int HttpConnection::TIME_STRING_SEC_POS = 2;

const int HttpConnection::MAX_TOKENS = 10;
const int HttpConnection::MAX_TOKEN_LEN = 80;


/*********************************************************************
 * Constructors
 */

HttpConnection::HttpConnection(const HttpURL url,
			       const HttpURL *proxy_url) :
  _url(url),
  _lastData(0),
  _lastDataLength(0)
{
  // Set the proxy URL

  if (proxy_url == 0)
    _proxyUrl = 0;
  else
    _proxyUrl = new HttpURL(*proxy_url);
  

  // Allocate space for the objects used for converting time strings
  // to time values.

  _dateTokens = new char*[MAX_TOKENS];
  for (int i = 0; i < MAX_TOKENS; ++i)
    _dateTokens[i] = new char[MAX_TOKEN_LEN];
  
  _timeTokens = new char*[MAX_TOKENS];
  for (int i = 0; i < MAX_TOKENS; ++i)
    _timeTokens[i] = new char[MAX_TOKEN_LEN];
  
  _months["Jan"] = 1;
  _months["Feb"] = 2;
  _months["Mar"] = 3;
  _months["Apr"] = 4;
  _months["May"] = 5;
  _months["Jun"] = 6;
  _months["Jul"] = 7;
  _months["Aug"] = 8;
  _months["Sep"] = 9;
  _months["Oct"] = 10;
  _months["Nov"] = 11;
  _months["Dec"] = 12;

  _months["January"] = 1;
  _months["February"] = 2;
  _months["March"] = 3;
  _months["April"] = 4;
  _months["June"] = 6;
  _months["July"] = 7;
  _months["August"] = 8;
  _months["September"] = 9;
  _months["October"] = 10;
  _months["November"] = 11;
  _months["December"] = 12;
}


/*********************************************************************
 * Destructor
 */

HttpConnection::~HttpConnection()
{
  // Delete contained objects

  delete _proxyUrl;

  delete _lastData;
}


/*********************************************************************
 * getData() - Retrieve the data at the given URL.  If a proxy URL was
 *             specified, go through the proxy.
 */

void HttpConnection::getData(const time_t last_data_time)
{
  if (_proxyUrl == 0)
    _getData(last_data_time);
//  else
//    _getDataViaProxy(last_data_time);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _getData() - Retrieve the data at the given URL without using a proxy.
 */

void HttpConnection::_getData(const time_t last_data_time)
{
  // Initialize the returned values.

  _lastDataLength = 0;
  _lastDataTime = 0;
  
  // Initialize the error values

  _isError = false;
  
  // Check for a valid URL

  if (!_url.isValid())
  {
    _isError = true;
    _errorCode = INVALID_URL;
    
    delete _lastData;
    _lastData = 0;
    
    return;
  }
  
  // Open a connection to the HTTP server

  int infd;
  int port = _url.getPort();
  
  if (port == 0)
    port = HTTP_PORT;
  
  if ((infd = SKU_open_client((char *)(_url.getHost().c_str()), port)) < 0)
  {
    _isError = true;
    _errorCode = CONNECTION_FAILURE;
    
    delete _lastData;
    _lastData = 0;
    
    return;
  }

  // Build up the request message

  string request = "GET /" + _url.getFile() + " HTTP/1.0\r\n";
  request += "User-Agent: NCAR/RAP HttpConnection::getData()/1.0\r\n";
  request += "\r\n";
  
  // Send the request to the HTTP server

  if (SKU_write(infd, request.c_str(), request.size(), -1) !=
      (int)request.size())
  {
    SKU_close(infd);
    
    _isError = true;
    _errorCode = CONNECTION_FAILURE;
    
    delete _lastData;
    _lastData = 0;
    
    return;
  }
  
  // Create the read buffer

  char header_buffer[HTTP_HEAD_BUF_SIZE];
  memset(header_buffer, 0, HTTP_HEAD_BUF_SIZE);

  // Read data until the header is complete or we are out of buffer space

  int len_read;
  int total_read = 0;
  bool header_complete = false;
  
  char *header_end_pos;
    
  do
  {
    len_read = SKU_read_timed(infd, (header_buffer + total_read),
			      HTTP_HEAD_READ_LEN, -1, 2000);
    total_read += len_read;

    if ((header_end_pos = strstr(header_buffer, "\r\n\r\n")) != NULL)
      header_complete = true;

  } while (!header_complete &&
	   (total_read + HTTP_HEAD_READ_LEN < HTTP_HEAD_BUF_SIZE));
  
  if (total_read + HTTP_HEAD_READ_LEN >= HTTP_HEAD_BUF_SIZE)
  {
    SKU_close(infd);

    _isError = true;
    _errorCode = READ_REPLY_FAILURE;
    
    delete _lastData;
    _lastData = 0;
    
    return;
  }

  //  Calculate the length of the header + 4 bytes for "\r\n\r\n"

  int header_len = header_end_pos - header_buffer + 4;

  // Pick out the Status value

  char * pos = strtok(header_buffer, " ");  // prime strtok
  pos = strtok(NULL, " ");                  // get the second token

  int status;
  int iret = sscanf(pos, "%d", &status);
  if (iret != 1 || status >= 300) {
    
    SKU_close(infd);

    _isError = true;
    _errorCode = HTTP_ERROR;
    _httpStatus = status;
    
    delete _lastData;
    _lastData = 0;
    
    return;
  }

  // Skip over the status tokens - past the nulls that strtok
  // wrote in our buffer

  pos += strlen(pos) + 1;

  // Get the modification time from the header

  char *mod_time_string;
  
  if ((mod_time_string = _getHeaderString(pos, "Last-Modified:")) == 0)
  {
    if ((mod_time_string = _getHeaderString(pos, "Last-modified:")) == 0)
    {
      SKU_close(infd);

      _isError = true;
      _errorCode = READ_RESOURCE_DATA_ERROR;

      delete _lastData;
      _lastData = 0;
	
      return;
    }
  }

  _lastDataTime = _stringToTime(mod_time_string);
    
  if (last_data_time > 0)
  {
    if (_lastDataTime <= last_data_time)
    {
      SKU_close(infd);
      
      if (_lastDataTime <= 0)
      {
	_isError = true;
	_errorCode = READ_RESOURCE_DATA_ERROR;
      }
      
      delete _lastData;
      _lastData = 0;
      
      return;
    }
    
  }
  
  // Get the content length from the header

  char *length_string;
  
  if ((length_string = _getHeaderString(pos, "Content-Length:")) == 0)
  {
    if ((length_string = _getHeaderString(pos, "Content-length:")) == 0)
    {
      SKU_close(infd);

      _isError = true;
      _errorCode = READ_RESOURCE_DATA_ERROR;

      delete _lastData;
      _lastData = 0;
      
      return;
    }
  }

  if (sscanf(length_string, "%d", &_lastDataLength) != 1) {

    // Bad Length Indicated in HTTP reply

    SKU_close(infd);

    _isError = true;
    _errorCode = READ_RESOURCE_DATA_ERROR;
    
    delete _lastData;
    _lastData = 0;
    
    return;
  }

  // Create the returned data buffer

  _lastData = new char[_lastDataLength + 1];
  memset(_lastData, 0, _lastDataLength + 1);
  
  // Read the data into the data buffer

  char *cur_pos = _lastData;

  // Copy that part of the data we read while scanning for the header

  int target_size = total_read - header_len;

  if (target_size > 0)
  {
    memcpy(cur_pos, (header_buffer + header_len), target_size);
    cur_pos += target_size;
    target_size = _lastDataLength - target_size;
  }
  else
  {
    target_size = _lastDataLength;
  }

  // Now read the rest of the resource data from the socket

  do
  {
    len_read = SKU_read_timed(infd, cur_pos, target_size, -1, 2000);
    target_size -= len_read;
    cur_pos += len_read;
  } while (target_size > 0 && len_read >= 0);

  if (target_size > 0)
  {
    _lastDataLength = _lastDataLength - target_size;
    _lastData[_lastDataLength] = '\0'; /* Null terminate */

    SKU_close(infd);

    _isError = true;
    _errorCode = READ_RESOURCE_DATA_ERROR;
    
    delete _lastData;
    _lastData = 0;
    
    return;
  }

  _lastData[_lastDataLength] = '\0'; /* Null terminate */

  SKU_close(infd);
}


/*********************************************************************
 * _getHeaderString() - Retrieve the string containing the indicated
 *                      header information.
 *
 * Returns a pointer to the string containing the header information,
 * or 0 if that string wasn't found in the header.
 */

char *HttpConnection::_getHeaderString(char *header,
				       const char *label)
{
  char *label_pos;
  
  if ((label_pos = strstr(header, label)) == NULL)
      return 0;

  label_pos += strlen(label);
  label_pos = strtok(label_pos, "\r\n");
  
  return label_pos;
}


/*********************************************************************
 * _stringToTime() - Convert the given string to a time value.  The
 *                   string is expected to be something like
 *                   "Thu, 10 Aug 2000 14:05:01 GMT".
 *
 * Returns the time value for the string if successful, or 0 if
 * unsuccessful.
 */

time_t HttpConnection::_stringToTime(const char *time_string)
{
  const string method_name = "_stringToTime()";
  
  char *local_time_string = STRdup(time_string);
  
  // Break the string up into tokens

  int num_tokens;
  
  if ((num_tokens = STRparse(local_time_string, _dateTokens,
			     strlen(local_time_string),
			     MAX_TOKENS, MAX_TOKEN_LEN))
      != DATE_STRING_NUM_TOKENS)
  {
    cerr << "ERROR: " << _className() << "::" << method_name << endl;
    cerr << "Error parsing date string into tokens" << endl;
    cerr << "Date string: " << time_string << endl;
    cerr << "Expected " << DATE_STRING_NUM_TOKENS << " tokens, found " <<
      num_tokens << " tokens" << endl;
    
    STRfree(local_time_string);
    
    return 0;
  }
  
  STRfree(local_time_string);
  
  // Now break the time string up into tokens

  if ((num_tokens = STRparse_delim(_dateTokens[DATE_STRING_TIME_POS],
				   _timeTokens, MAX_TOKEN_LEN,
				   ":", MAX_TOKENS, MAX_TOKEN_LEN))
      != TIME_STRING_NUM_TOKENS)
  {
    cerr << "ERROR: " << _className() << "::" << method_name << endl;
    cerr << "Error parsing time string into tokens" << endl;
    cerr << "Time string: " << _dateTokens[DATE_STRING_TIME_POS] << endl;
    cerr << "Expected " << TIME_STRING_NUM_TOKENS << " tokens, found " <<
      num_tokens << " tokens" << endl;
    
    return 0;
  }
  
  // Get the time values

  date_time_t time_struct;
  
  time_struct.year = atoi(_dateTokens[DATE_STRING_YEAR_POS]);
  time_struct.month = _months[_dateTokens[DATE_STRING_MONTH_POS]];
  time_struct.day = atoi(_dateTokens[DATE_STRING_DAY_POS]);
  time_struct.hour = atoi(_timeTokens[TIME_STRING_HOUR_POS]);
  time_struct.min = atoi(_timeTokens[TIME_STRING_MIN_POS]);
  time_struct.sec = atoi(_timeTokens[TIME_STRING_SEC_POS]);
  
  // Convert to unix time

  return uunix_time(&time_struct);
}
