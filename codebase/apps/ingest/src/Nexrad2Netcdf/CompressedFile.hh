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
#ifndef __COMPRESSED_FILE_HH__
#define __COMPRESSED_FILE_HH__

#ifdef __cplusplus




#include <zlib.h>
#include <bzlib.h>

#include <string.h>
#include <sstream>
#include <stdexcept>

#include <errno.h>
#include <stdio.h>




struct CompressedFile {

	virtual ~CompressedFile() = 0;

	virtual bool IsEOF() = 0;
	virtual int Read( void* buffer, int length ) = 0;


protected:

	inline CompressedFile();
};


CompressedFile::CompressedFile() {
}




struct NotCompressedFile : public CompressedFile {

	inline NotCompressedFile( const std::string& path );
	virtual ~NotCompressedFile();

	virtual bool IsEOF();
	virtual int Read( void* buffer, int length );


private:

	FILE* m_file;
};


NotCompressedFile::NotCompressedFile( const std::string& path ) : m_file( fopen( path.c_str(), "rb" ) ) {

	if ( m_file == NULL ) {

		std::stringstream errorStream;
		errorStream << "NotCompressedFile::NotCompressedFile( \"" << path << "\" ) - Unable to open file for reading";
		throw std::runtime_error( errorStream.str() );
	}
}




struct GZCompressedFile : public CompressedFile {

	inline GZCompressedFile( const std::string& path );
	virtual ~GZCompressedFile();

	virtual bool IsEOF();
	virtual int Read( void* buffer, int length );


private:

	gzFile m_gzFile;
};


GZCompressedFile::GZCompressedFile( const std::string& path ) : m_gzFile( gzopen( path.c_str(), "rb" ) ) {

	if ( m_gzFile == NULL ) {

		std::stringstream errorStream;
		errorStream << "GZCompressedFile::GZCompressedFile( \"" << path << "\" ) - Unable to open file for reading";
		throw std::runtime_error( errorStream.str() );
	}
}




struct BZ2CompressedFile : public CompressedFile {

	inline BZ2CompressedFile( const std::string& path );
	virtual ~BZ2CompressedFile();

	virtual bool IsEOF();
	virtual int Read( void* buffer, int length );


private:

	FILE* m_file;
	BZFILE* m_bzFile;

	bool m_eof;
};


BZ2CompressedFile::BZ2CompressedFile( const std::string& path ) : m_file( fopen( path.c_str(), "rb" ) ), m_bzFile( NULL ), m_eof( false ) {

	if ( m_file == NULL ) {

		std::stringstream errorStream;
		errorStream << "BZ2CompressedFile::BZ2CompressedFile( \"" << path << "\" ) - Unable to open file for reading";
		throw std::runtime_error( errorStream.str() );
	}
	else {

		int bzError = BZ_OK;
		m_bzFile = BZ2_bzReadOpen( &bzError, m_file, 0, 0, NULL, 0 );
		if ( bzError != BZ_OK ) {

			fclose( m_file );
			m_file = NULL;

			std::stringstream errorStream;
			errorStream << "BZ2CompressedFile::BZ2CompressedFile( \"" << path << "\" ) - Unable to initialize decompression context (bzerror = " << bzError << ')';
			throw std::runtime_error( errorStream.str() );
		}
	}
}




#endif    /* __cplusplus */

#endif    /* __COMPRESSED_FILE_HH__ */
