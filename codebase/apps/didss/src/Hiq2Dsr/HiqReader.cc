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
/*********************************************************************
 * HiqReader: Class for objects used to read HiQ beam data.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2006
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <rapformats/ds_radar.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/sockutil.h>
#include <toolsa/MsgLog.hh>

#if defined(__linux)
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#else
#include <sys/filio.h>
#endif


#include "MsgFactory.hh"
#include "HiqReader.hh"
using namespace std;


// Global variables

const int HiqReader::MAX_BUF_SIZE   = 50000;


/*********************************************************************
 * Constructor
 */

HiqReader::HiqReader(MsgFactory *msg_factory, const bool debug) :
  _debug(debug),
  _reader(0),
  _inputBuffer(0),
  _inputBufferLen(0),
  _msgFactory(msg_factory)
{
}


/*********************************************************************
 * Destructor
 */

HiqReader::~HiqReader() 
{
  delete _reader;
  delete _inputBuffer;
}


/*********************************************************************
 * init() - Initialize the reader.
 *
 * Returns true on success, false on failure.
 *
 * NOTE: After this method is called, the HiqReader object takes
 * control of the Reader pointer so this pointer shouldn't be
 * used or deleted by the calling object.
 */

bool HiqReader::init(Reader *reader) 
{
  // Save the reader pointer

  _reader = reader;

  // Allocate space for the input buffer

  _inputBuffer = new char[MAX_BUF_SIZE];
  _inputBufferLen = 0;

  return true;
}


/*********************************************************************
 * getNextMsg() - Get the next HiQ message.
 *
 * Returns a pointer to the latest message read on success; 0 otherwise.
 *
 * NOTE: The calling object takes control of the returned msg object and
 * is expected to delete this object when it is no longer needed.
 */

HiqMsg *HiqReader::getNextMsg()
{
  static const string method_name = "HiqReader::getNextMsg()";

  // Read raw data until we have a complete message

  int message_len = -1;

  while (true)
  {
    PMU_auto_register("Reading raw beam data");

    // Read the next incoming buffer

    if ((_inputBufferLen = _reader->readBuffer(_inputBuffer,
					       MAX_BUF_SIZE)) <= 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error reading next data buffer" << endl;

      return 0;
    }

    // If this is the first buffer, get the size of the message
    // from the header.

    if (message_len < 0)
    {
      // Get a pointer to the beginning of the message.  We are
      // assuming that if there are any bytes left in the message
      // buffer, then there will be at least 6 bytes so we can get
      // the message length from the buffer.

      char *message_ptr;

      if (_messageBuffer.getBufLen() > 0)
	message_ptr = (char *)_messageBuffer.getBufPtr();
      else
	message_ptr = _inputBuffer;

      // Make sure this is a message that we can handle.  We need to
      // do this here because we get some messages that we don't understand
      // and we don't want to get a message_len that is invalid since we
      // might then skip a bunch of needed data.

      if (!_isHeader(message_ptr))
      {
	cerr << "WARNING: " << method_name << endl;
	cerr << "Invalid message type: " << (int)message_ptr[0];
	if (isprint(message_ptr[0]))
	  cerr << " (" << message_ptr[0] << ")";
	cerr << endl;
	
	// Clear out the message buffer because we're in a funny
	// state and just need to recover.  The message buffer should
	// be empty here because we're looking for a header, but if
	// too many bytes were read before processing the last message,
	// these bytes will still be in the message buffer.  Print out
	// a warning message so we know if we are losing bytes in this
	// way.

	if (_messageBuffer.getBufLen() > 0)
	{
	  cerr << "WARNING: " << method_name << endl;
	  cerr << "Clearing out non-empty message buffer used for reading UDP data" << endl;
	  cerr << "Message buffer contained " << _messageBuffer.getBufLen()
	       << " bytes" << endl;

	  _messageBuffer.reset();
	}

	continue;
      }

      message_len = _getMessageLen(message_ptr);
      
      if (message_len <= 0)
      {
	// We are obviously in a bad state here.  Clear out the message
	// buffer and try to get back to where we can read the data

	cerr << "WARNING: " << method_name << endl;
	cerr << "Received message that says it has " << message_len << "bytes" << endl;
	cerr << "Since this is obviously wrong, clearing out message buffer and waiting for something useful" << endl;
	
	_messageBuffer.reset();
	continue;
      }
      
      if (_debug)
	cerr << "---> Waiting for " << message_len
	     << " bytes for new message" << endl;
    }
    else
    {
      // If we get here, we are in the middle of constructing a message
      // from received packets.  We will look at each packet to see if it
      // looks like a packet with a header in it.  If it does, we assume
      // that we lost a packet when constucting the message and so throw
      // away the current message and start collecting a new one.

      // Make sure this is a message that we can handle.  We need to
      // do this here because we get some messages that we don't understand
      // and we don't want to get a message_len that is invalid since we
      // might then skip a bunch of needed data.

      if (_isHeader(_inputBuffer))
      {
	cerr << "WARNING: " << method_name << endl;
	cerr << "Looks like header in the middle of a message" << endl;
	cerr << "Skipping current message and starting over again" << endl;
	cerr << endl;
	
	// Clear out the message buffer because we're in a funny
	// state and just need to recover.  The message buffer should
	// be empty here because we're looking for a header, but if
	// too many bytes were read before processing the last message,
	// these bytes will still be in the message buffer.  Print out
	// a warning message so we know if we are losing bytes in this
	// way.

	if (_messageBuffer.getBufLen() > 0)
	{
	  cerr << "WARNING: " << method_name << endl;
	  cerr << "Clearing out non-empty message buffer used for reading UDP data" << endl;
	  cerr << "Message buffer contained " << _messageBuffer.getBufLen()
	       << " bytes" << endl;

	  _messageBuffer.reset();
	}

	_messageBuffer.add(_inputBuffer, _inputBufferLen);
	
	continue;
      }

    }
    
    // Append the buffer to our message buffer

    _messageBuffer.add(_inputBuffer, _inputBufferLen);

    // See if we've read the entire message

    if (message_len <= _messageBuffer.getBufLen())
    {
      if (_debug || message_len < _messageBuffer.getBufLen())
      {
	cerr << "**** Finished reading message bytes" << endl;
	cerr << "     Expected " << message_len 
	     << " bytes in message" << endl;
	cerr << "     Have " << _messageBuffer.getBufLen()
	     << " bytes in buffer" << endl;
      }

      break;
    }
    else if (_debug)
    {
      int bytes_left = message_len - _messageBuffer.getBufLen();
      cerr << "     Buffer has " << _messageBuffer.getBufLen()
	   << " bytes" << endl;
      cerr << "     Waiting for " << bytes_left
	   << " more bytes to complete message" << endl;
    }
  }

  // Create the HiqMsg object to return

  HiqMsg *msg =
    _msgFactory->createMessage((char *)_messageBuffer.getBufPtr(),
			       _messageBuffer.getBufLen());

  
  // Remove the processed message from the front of the message buffer.
  // If the MsgFactory wasn't able to create a HiqMsg object
  // from the current buffer, clear out the whole buffer so we have a
  // clean slate when waiting for the next message.  We are periodically
  // getting buffers that we can't interpret and this is the only way to
  // recover from this problem.

  if (_messageBuffer.getBufLen() == message_len ||
      msg == 0)
  {
    _messageBuffer.reset();
  }
  else
  {
    int bytes_left = _messageBuffer.getBufLen() - message_len;
    char *bytes_left_buffer = new char[bytes_left];
    memcpy(bytes_left_buffer, (char *)_messageBuffer.getBufPtr() + message_len,
	   bytes_left);

    _messageBuffer.load(bytes_left_buffer, bytes_left);

    delete []bytes_left_buffer;
  }

  return msg;
}
