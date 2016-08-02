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
//
//  Buffer.cpp
//
//

#include <stdio.h>
#include <string.h>

#include <iostream>
using namespace std;

#include "Buffer.h"
using namespace ForayUtility;

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
Buffer::Buffer( const int size ) : size_(0), data_(NULL) {

   if ( size > 0 ) {
      size_ = size;
      data_ = new unsigned char[size_];
   }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
Buffer::Buffer( const unsigned char *byteStream, int size ) :
   size_(0), data_(NULL), decoder_(), encoder_()
{
   if ( size > 0 ) {
      size_ = size;
      data_ = new unsigned char[size_];
      memcpy( data_, byteStream, size_ );
   }
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
Buffer::~Buffer(){

    if(data_ != NULL){
	delete [] data_;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
Buffer::Buffer(const Buffer &src) : size_(0), data_(NULL) {

    if(size_ != 0){
	delete [] data_;
    }
    
    if(src.size_ <= 0){
	size_ = 0;
	data_ = NULL;
	return;
    }
	
    size_ = src.size_;
    data_ = new unsigned char[size_];

    memcpy(data_,src.data_,size_);
    decoder_ = src.decoder_;

}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
Buffer & Buffer::operator=(const Buffer &src){

    if(size_ != 0){
	delete [] data_;
    }
    
    if(src.size_ <= 0){
	size_ = 0;
	data_ = NULL;
	return *this;
    }
	
    size_ = src.size_;
    data_ = new unsigned char[size_];

    memcpy(data_,src.data_,size_);
    decoder_ = src.decoder_;

    return *this;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
unsigned char *Buffer::new_data(const int size){

    if(size == size_){
	return data_;
    }

    if(data_ != NULL){
	delete [] data_;
    }

    data_ = new unsigned char[size];
    size_ = size;

    memset(data_,0,size_);

    return data_;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
const unsigned char *Buffer::data(const int loc) const throw(Fault){
    
    char msg[2048];
    
    if(loc >= size_){
	sprintf(msg,
		"Buffer::data: location(%d) >= buffer size(%d).\n",
		loc,size_);
	throw Fault(msg);
    }

    return &data_[loc];
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int Buffer::current_size() const {
    return size_;
}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void Buffer::is_big_endian(const bool bigEndian){
    decoder_.buffers_big_endian(bigEndian);
}

//////////////////////////////////////////////////////////////////////
//
//
//
/////////////////////////////////////////////////////////////////////
string Buffer::get_string_from_char(const int loc, const int size) throw (Fault){

    if ((loc + size) > size_){
	char msg[2048];
	sprintf(msg,"Buffer::get_string_from_char: Tried to access past end of buffer, loc %d, size: %d, Buffer size: %d.\n",loc,size,size_);
	throw Fault(msg);
    }

    return decoder_.char_string(&data_[loc],size);
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int Buffer::get_one_byte_integer(const int loc) throw(Fault){

    if(loc  > size_){
	char msg[1024];
	sprintf(msg,"Buffer::get_one_byte_integer: Tried to access past end of buffer, loc %d \n",loc);
	throw Fault(msg);
    }

    return decoder_.one_byte(&data_[loc]); 
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int Buffer::get_one_byte_unsigned_integer(const int loc) throw(Fault){

    if(loc  > size_){
	char msg[1024];
	sprintf(msg,"Buffer::get_one_byte_integer: Tried to access past end of buffer, loc %d \n",loc);
	throw Fault(msg);
    }

    return decoder_.one_byte_unsigned(&data_[loc]); 
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int Buffer::get_two_byte_integer(const int loc) throw(Fault){

    if((loc + 2) > size_){
	char msg[1024];
	sprintf(msg,"Buffer::get_two_byte_integer: Tried to access past end of buffer, loc %d \n",loc);
	throw Fault(msg);
    }

    return decoder_.two_byte(&data_[loc]); 
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int Buffer::get_two_byte_unsigned_integer(const int loc) throw(Fault){

    if((loc + 2) > size_){
	char msg[1024];
	sprintf(msg,"Buffer::get_two_byte_integer: Tried to access past end of buffer, loc %d \n",loc);
	throw Fault(msg);
    }

    return  decoder_.two_byte_unsigned(&data_[loc]); 
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int Buffer::get_four_byte_integer(const int loc) throw(Fault){

    if((loc + 4) > size_){
	char msg[1024];
	sprintf(msg,"Buffer::get_four_byte_integer: Tried to access past end of buffer, loc %d \n",loc);
	throw Fault(msg);
    }

    return decoder_.four_byte(&data_[loc]); 
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int Buffer::get_four_byte_unsigned_integer(const int loc) throw(Fault){

    if((loc + 4) > size_){
	char msg[1024];
	sprintf(msg,"Buffer::get_four_byte_unsigned_integer: Tried to access past end of buffer, loc %d \n",loc);
	throw Fault(msg);
    }

    return decoder_.four_byte_unsigned(&data_[loc]); 
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
double Buffer::get_four_byte_float(const int loc) throw(Fault){

    if((loc + 4) > size_){
	char msg[1024];
	sprintf(msg,"Buffer::get_four_byte_float: Tried to access past end of buffer, loc %d \n",loc);
	throw Fault(msg);
    }

    return decoder_.four_byte_float(&data_[loc]); 
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
double Buffer::get_eight_byte_float(const int loc) throw(Fault){

    if((loc + 8) > size_){
	char msg[1024];
	sprintf(msg,"Buffer::get_eight_byte_float: Tried to access past end of buffer, loc %d \n",loc);
	throw Fault(msg);
    }

    return decoder_.eight_byte_float(&data_[loc]); 
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
long long Buffer::get_eight_byte_integer(const int loc) throw(Fault){

    if((loc + 8) > size_){
	char msg[1024];
	sprintf(msg,"Buffer::get_eight_byte_integer: Tried to access past end of buffer, loc %d \n",loc);
	throw Fault(msg);
    }

    return decoder_.eight_byte(&data_[loc]); 
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int Buffer::set_four_byte_integer(const int loc,const int value) throw(Fault){

    if((loc + 4) > size_){
	char msg[1024];
	sprintf(msg,"Buffer::set_four_byte_interger: Tried to access past end of buffer, loc %d \n",loc);
	throw Fault(msg);
    }

    encoder_.four_byte_integer(&data_[loc],value); 
    return 4;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int Buffer::set_four_byte_unsigned_integer(const int loc,const unsigned int value) throw(Fault){

    if((loc + 4) > size_){
	char msg[1024];
	sprintf(msg,"Buffer::set_four_byte_interger: Tried to access past end of buffer, loc %d \n",loc);
	throw Fault(msg);
    }

    encoder_.four_byte_unsigned_integer(&data_[loc],value); 
    return 4;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int Buffer::set_two_byte_integer(const int loc,const int value) throw(Fault){

    if((loc + 2) > size_){
	char msg[1024];
	sprintf(msg,"Buffer::set_two_byte_interger: Tried to access past end of buffer, loc %d \n",loc);
	throw Fault(msg);
    }

    encoder_.two_byte_integer(&data_[loc],value); 
    return 2;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int Buffer::set_two_byte_unsigned_integer(const int loc,const unsigned int value) throw(Fault){

    if((loc + 2) > size_){
	char msg[1024];
	sprintf(msg,"Buffer::set_two_byte_interger: Tried to access past end of buffer, loc %d \n",loc);
	throw Fault(msg);
    }

    encoder_.two_byte_unsigned_integer(&data_[loc],value); 
    return 2;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int Buffer::set_one_byte_integer(const int loc,const char value) throw(Fault){
   if ((loc + 1) > size_){
      char msg[1024];
      sprintf(msg,"Buffer::set_one_byte_interger: Tried to access past end of buffer, loc %d \n",loc);
      throw Fault(msg);
   }

   encoder_.one_byte_integer(&(data_[loc]),value); 
   return 1;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int Buffer::set_one_byte_unsigned_integer(const int loc,const unsigned char value) throw(Fault){
   if ((loc + 1) > size_){
      char msg[1024];
      sprintf(msg,"Buffer::set_one_byte_interger: Tried to access past end of buffer, loc %d \n",loc);
      throw Fault(msg);
   }

   encoder_.one_byte_unsigned_integer(&(data_[loc]),value); 
   return 1;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int Buffer::set_four_byte_float(const int loc,const double value) throw(Fault){

    if((loc + 4) > size_){
	char msg[1024];
	sprintf(msg,"Buffer::set_four_byte_float: Tried to access past end of buffer, loc %d \n",loc);
	throw Fault(msg);
    }

    encoder_.four_byte_float(&data_[loc],value); 
    return 4;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int Buffer::set_eight_byte_float(const int loc,const double value) throw(Fault){

    if((loc + 8) > size_){
	char msg[1024];
	sprintf(msg,"Buffer::set_eight_byte_float: Tried to access past end of buffer, loc %d \n",loc);
	throw Fault(msg);
    }

    encoder_.eight_byte_float(&data_[loc],value); 
    return 8;
}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int Buffer::set_string(const int loc,const string &value, const int size) throw(Fault){

    if((loc + size) > size_){
	char msg[1024];
	sprintf(msg,"Buffer::set_string: Tried to access past end of buffer, loc %d \n",loc);
	throw Fault(msg);
    }

    const char * c_str = value.c_str();

    encoder_.string(&data_[loc],c_str,size); 
    return size;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int Buffer::set_string(const int loc,const unsigned char *value, const int size) throw(Fault){

    if((loc + size) > size_){
	char msg[1024];
	sprintf(msg,"Buffer::set_string: Tried to access past end of buffer, loc %d \n",loc);
	throw Fault(msg);
    }

    encoder_.string(&data_[loc],value,size); 
    return size;
}


