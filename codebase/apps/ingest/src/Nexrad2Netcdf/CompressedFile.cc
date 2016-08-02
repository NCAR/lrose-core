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
#include "CompressedFile.hh"




CompressedFile::~CompressedFile() {
}




NotCompressedFile::~NotCompressedFile() {

	fclose( m_file );
}


bool NotCompressedFile::IsEOF() {

	return( feof( m_file ) != 0 );
}


int NotCompressedFile::Read( void* buffer, int length ) {

	length = fread( buffer, 1, length, m_file );
	if ( ferror( m_file ) ) {

		std::stringstream errorStream;
		errorStream << "NotCompressedFile::Read( " << buffer << ", " << length << " ) - Error reading from file";
		throw std::runtime_error( errorStream.str() );
	}

        return length;
}




GZCompressedFile::~GZCompressedFile() {

	gzclose( m_gzFile );
}


bool GZCompressedFile::IsEOF() {

	return( gzeof( m_gzFile ) != 0 );
}


int GZCompressedFile::Read( void* buffer, int length ) {

	length = gzread( m_gzFile, buffer, length );
	if ( length < 0 ) {

		int errorNumber;
		const char* errorMessage = gzerror( m_gzFile, &errorNumber );
		if ( errorNumber == Z_ERRNO )
			errorMessage = strerror( errno );

		std::stringstream errorStream;
		errorStream << "GZCompressedFile::Read( " << buffer << ", " << length << " ) - " << errorMessage;
		throw std::runtime_error( errorStream.str() );
	}

	return length;
}




BZ2CompressedFile::~BZ2CompressedFile() {

	// really not much to be done about a close error
	int bzError = BZ_OK;
	BZ2_bzReadClose( &bzError, m_bzFile );

	fclose( m_file );
}


bool BZ2CompressedFile::IsEOF() {

	return m_eof;
}


int BZ2CompressedFile::Read( void* buffer, int length ) {

	int bzError = BZ_OK;
	length = BZ2_bzRead( &bzError, m_bzFile, buffer, length );
	if ( ( bzError != BZ_OK ) && ( bzError != BZ_STREAM_END ) ) {

		std::stringstream errorStream;
		errorStream << "BZ2CompressedFile::Read( " << buffer << ", " << length << " ) - Unable to read from file (bzerror = " << bzError << ')';
		throw std::runtime_error( errorStream.str() );
	}
	m_eof |= ( bzError == BZ_STREAM_END );

	return length;
}
