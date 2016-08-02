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
//
//
//


#include <errno.h>
extern int errno;

#include <string.h>

#include <iostream>
using namespace std;

#include "FortranBinary.h"
using namespace ForayUtility;

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
FortranBinary::FortranBinary(){

    file_         = NULL;
    read_only_    = false;
    decoderHead_  = NULL;
    encoderHead_  = NULL;

}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
FortranBinary::~FortranBinary(){

    close_file();

    if(decoderHead_ != NULL){
	delete decoderHead_;
    }

    if(encoderHead_ != NULL){
	delete encoderHead_;
    }
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void FortranBinary::open_file(const char *filename) throw(Fault) {

    read_only_ = false;
    char msg[2048];

    if(file_ != NULL){
	close_file();
    }

    if((file_ = fopen(filename,"r")) == NULL){
	sprintf(msg,"FortranBinary::open_file failed for %s: %s \n",filename,strerror(errno));
	throw Fault(msg);
    }

    if(decoderHead_ == NULL){
	decoderHead_ = new Decoder();
	decoderHead_->buffers_big_endian(true);
    }

    unsigned char headBuffer[4];

    if(fread(headBuffer,4,1,file_) != 1){
	sprintf(msg,"FortranBinary::open_file: Can not read first header: %s \n",
		strerror(errno));
	throw Fault(msg);
    }

    int headValue = decoderHead_->four_byte(headBuffer);
    

    unsigned char *recordBuffer;
    recordBuffer = new unsigned char[headValue];

    if(fread(recordBuffer,headValue,1,file_) != 1){
	sprintf(msg,
		"FortranBinary::open_file: Reading of first record failed (size of %d): %s \n",
		headValue,strerror(errno));
	throw Fault(msg);
    }

    delete [] recordBuffer;

    if(fread(headBuffer,4,1,file_) != 1){
	sprintf(msg,
		"FortranBinary::open_file: Can not read first record tail: %s \n",
		strerror(errno));
	throw Fault(msg);
    }

    int tailValue = decoderHead_->four_byte(headBuffer);

    if(tailValue != headValue){
	throw Fault("FortranBinary::open_file: head and tail values do not agree.\n");
    }

    read_only_ = true;

    // Set file pointer back to zero.
    rewind(file_);
    
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void FortranBinary::create_file(const char *filename) throw(Fault) {

    read_only_ = false;
    char msg[2048];

    if(file_ != NULL){
	close_file();
    }

    if((file_ = fopen(filename,"w")) == NULL){
	sprintf(msg,"FortranBinary::create_file failed: %s \n",strerror(errno));
	throw Fault(msg);
    }

    if(encoderHead_ == NULL){
	encoderHead_ = new Encoder();
	encoderHead_->buffers_big_endian(true);
    }

    
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void FortranBinary::close_file(){

    if(file_ != NULL){
	fclose(file_);
    }
    
    file_ = NULL;

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool FortranBinary::read_record(Buffer &buffer) throw(Fault) {

    char msg[2048];
 
    if(file_ == NULL){
	throw Fault("FortranBinary::read_record: File not open.\n");
    }

    if(!read_only_){
	throw Fault("FortranBinary::read_record: File not read only.\n");
    }

    unsigned char headBuffer[4];

    if(fread(headBuffer,4,1,file_) != 1){
	// End of file ?
	if(feof(file_)){
	    // End of file reached.
	    return false;
	}
	// Error.
	sprintf(msg,"FortranBinary::read_record: Cound not read record head: %s \n",
		strerror(errno));
	throw Fault(msg);
    }

    int headValue = decoderHead_->four_byte(headBuffer);

    unsigned char *bufferData = buffer.new_data(headValue);

    if(fread(bufferData,headValue,1,file_) != 1){
	sprintf(msg,"FortranBinary::read_record: Could not read record: %s \n",
		strerror(errno));
	throw Fault(msg);
    }

    if(fread(headBuffer,4,1,file_) != 1){
	sprintf(msg,"FortranBinary::read_record: Could not read tail: %s \n",
		strerror(errno));
	throw Fault(msg);
    }
    
    int tailValue = decoderHead_->four_byte(headBuffer);

    if(tailValue != headValue){
	throw Fault("FortranBinary::read_record: head and tail values do not agree.\n");
    }
    
    return true;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void FortranBinary::write_record(Buffer &buffer) throw(Fault) {

    char msg[2048];
 
    if(file_ == NULL){
	throw Fault("FortranBinary::write_record: File not open.\n");
    }

    if(read_only_ ){
	throw Fault("FortranBinary::write_record: File is read only.\n");
    }

    unsigned char headBuffer[4];

    int bufferSize = buffer.current_size();

    encoderHead_->four_byte_integer(headBuffer,bufferSize);

    if(fwrite(headBuffer,4,1,file_) != 1){
	// Error.
	sprintf(msg,"FortranBinary::write_record: Cound not write record head: %s \n",
		strerror(errno));
	throw Fault(msg);
    }

    const unsigned char *bufferData = buffer.data(0);

    if(fwrite(bufferData,bufferSize,1,file_) != 1){
	sprintf(msg,"FortranBinary::write_record: Could not write record: %s \n",
		strerror(errno));
	throw Fault(msg);
    }

    if(fwrite(headBuffer,4,1,file_) != 1){
	// Error.
	sprintf(msg,"FortranBinary::write_record: Cound not write record tail: %s \n",
		strerror(errno));
	throw Fault(msg);
    }
    
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool FortranBinary::opened_for_writing() {

    if(file_ == NULL){
	return false;
    }

    if(read_only_ == true){
	return false;
    }

    return true;
}


