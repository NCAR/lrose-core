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
//  Encoder.cpp
//
//

#include <stdio.h>
#include <string.h>

#include <iostream>
using namespace std;

#include "Encoder.h"

using namespace ForayUtility;


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
Encoder::Encoder(){

    machineBigEndian_ = machine_big_endian();
    buffersBigEndian_ = false;  // Default;
    set_swapBytes();
    
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
Encoder::~Encoder(){

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void Encoder::buffers_big_endian(bool bigEndian){

    buffersBigEndian_ = bigEndian;
    set_swapBytes();

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void Encoder::four_byte_integer(unsigned char * const buffer,const int encodeValue){

    union {
	unsigned char byte[4];
	int val;
    }word;

    word.val = encodeValue;

    if(swapBytes_){
	buffer[0] = word.byte[3];
	buffer[1] = word.byte[2];
	buffer[2] = word.byte[1];
	buffer[3] = word.byte[0];
    }else{
	buffer[0] = word.byte[0];
	buffer[1] = word.byte[1];
	buffer[2] = word.byte[2];
	buffer[3] = word.byte[3];
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void Encoder::four_byte_unsigned_integer(unsigned char * const buffer,const unsigned int encodeValue){

    union {
	unsigned char byte[4];
        unsigned int val;
    }word;

    word.val = encodeValue;

    if(swapBytes_){
	buffer[0] = word.byte[3];
	buffer[1] = word.byte[2];
	buffer[2] = word.byte[1];
	buffer[3] = word.byte[0];
    }else{
	buffer[0] = word.byte[0];
	buffer[1] = word.byte[1];
	buffer[2] = word.byte[2];
	buffer[3] = word.byte[3];
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void Encoder::two_byte_integer(unsigned char * const buffer, const int encodeValue){

    union {
	unsigned char byte[2];
	short val;
    }word;

    word.val = encodeValue;

    if(swapBytes_){
	buffer[0] = word.byte[1];
	buffer[1] = word.byte[0];
    }else{
	buffer[0] = word.byte[0];
	buffer[1] = word.byte[1];
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void Encoder::two_byte_unsigned_integer(unsigned char * const buffer, const int encodeValue){

    union {
	unsigned char byte[2];
	unsigned short val;
    }word;

    word.val = encodeValue;

    if(swapBytes_){
	buffer[0] = word.byte[1];
	buffer[1] = word.byte[0];
    }else{
	buffer[0] = word.byte[0];
	buffer[1] = word.byte[1];
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void Encoder::one_byte_integer( unsigned char * const buffer, 
                                const char encodeValue ) const
{
   *buffer = encodeValue;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void Encoder::one_byte_unsigned_integer( unsigned char * const buffer, 
                                         const unsigned char encodeValue ) const
{
   *buffer = encodeValue;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void Encoder::four_byte_float(unsigned char * const buffer, const double encodeValue){

    union {
	unsigned char byte[4];
	float val;
    }word;

    word.val = (float)encodeValue;

    if(swapBytes_){
	buffer[0] = word.byte[3];
	buffer[1] = word.byte[2];
	buffer[2] = word.byte[1];
	buffer[3] = word.byte[0];
    }else{
	buffer[0] = word.byte[0];
	buffer[1] = word.byte[1];
	buffer[2] = word.byte[2];
	buffer[3] = word.byte[3];
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void Encoder::eight_byte_float(unsigned char * const buffer, const double encodeValue){

    union {
	unsigned char byte[8];
	double val;
    }word;

    word.val = encodeValue;

    if(swapBytes_){
	buffer[0] = word.byte[7];
	buffer[1] = word.byte[6];
	buffer[2] = word.byte[5];
	buffer[3] = word.byte[4];
	buffer[4] = word.byte[3];
	buffer[5] = word.byte[2];
	buffer[6] = word.byte[1];
	buffer[7] = word.byte[0];
    }else{
	buffer[0] = word.byte[0];
	buffer[1] = word.byte[1];
	buffer[2] = word.byte[2];
	buffer[3] = word.byte[3];
	buffer[4] = word.byte[4];
	buffer[5] = word.byte[5];
	buffer[6] = word.byte[6];
	buffer[7] = word.byte[7];
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void Encoder::string(unsigned char * const buffer, const char *encodeValue,const int size){

    strncpy((char *)buffer,encodeValue,size);

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void Encoder::string(unsigned char * const buffer, const unsigned char *encodeValue,const int size){

    memcpy(buffer,encodeValue,size);

}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
long long Encoder::eight_byte(const unsigned char *buffer){

    union {
	unsigned char byte[8];
	long long val;
    }word;

    word.val = 0;

    if(swapBytes_){
	word.byte[0] = buffer[7];
	word.byte[1] = buffer[6];
	word.byte[2] = buffer[5];
	word.byte[3] = buffer[4];
	word.byte[4] = buffer[3];
	word.byte[5] = buffer[2];
	word.byte[6] = buffer[1];
	word.byte[7] = buffer[0];
	    
    }else{
	word.byte[0] = buffer[0];
	word.byte[1] = buffer[1];
	word.byte[2] = buffer[2];
	word.byte[3] = buffer[3];
	word.byte[4] = buffer[4];
	word.byte[5] = buffer[5];
	word.byte[6] = buffer[6];
	word.byte[7] = buffer[7];
    }

    return word.val;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int Encoder::one_byte(const unsigned char *buffer){

    signed char value = *buffer;

    return (int)value;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool Encoder::machine_big_endian(){

    union {
	unsigned char byte[4];
	int val;
    }word;

    word.val = 0;
    word.byte[3] = 0x01;
    
    return word.val == 1;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void Encoder::set_swapBytes(){
    
    swapBytes_ = true;

    if(machineBigEndian_ == buffersBigEndian_){
	swapBytes_ = false;
    }
    
}
