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
//##############################################################################
// Except.h 
//
// Original Author: J.A. Horsmeier May 3, 1992
// Modified to more closely resemble C++ Exception syntaax. - F. Hage 10/94

#ifndef _EXCEPT_H
#define _EXCEPT_H


#include <cerrno>
#include <setjmp.h>
using namespace std;

enum gen_errors { GEN_ERR_BASE = 10000,INDEX_RANGE,NULL_POINTER,LOGIC_ERROR,PORT_UNAVAILIBLE};

//##############################################################################
// Exception handling list element
//

typedef struct _except_t {
	int code; 		     // Enumerated Exception code
	int try_file_line;           // location of try block
	int line; 		     // location of throw
	const char *try_file_name;   // location of try block
	const char *file;	     // location of throw
	const char *reason;          // readable cause of exception
	struct _except_t* Next;
	jmp_buf           JmpBuf;
} except_t;

//##############################################################################
// except.c Function prototypes
//

extern void		ExceptIst(except_t*,const char *, int);
extern void		ExceptRls(void);
extern void		ExceptRaise(int,const char *,const char *, int);
extern int		ExceptCode(void);
extern int		ExceptLine(void);
extern int		ExceptTryLine(void);
extern const char*	ExceptFile(void);
extern const char*	ExceptTryFile(void);
extern const char*	ExceptReason(void);

//##############################################################################
// except macros
//
#define TRY	{except_t __E_buf__; ExceptIst(&__E_buf__,__FILE__,__LINE__);if (!setjmp(__E_buf__.JmpBuf))

#define END_EXCEPT	ExceptRls(); }

#define CATCH(x)	else if (ExceptCode() == (x))

#define CATCH_ALL 	else

#define THROW(x,reason)	ExceptRaise(x,(reason),__FILE__,__LINE__)
#define RETHROW  	ExceptRaise( ExceptCode(), ExceptReason(), ExceptFile(), ExceptLine());

#define EXCEPTTRYFILE   ExceptTryFile()  // TRY block location
#define EXCEPTTRYLINE   ExceptTryLine()
#define EXCEPTFILE      ExceptFile()     // THROW statement location
#define EXCEPTLINE      ExceptLine()
#define EXCEPTREASON    ExceptReason()   // THROW reason
#define EXCEPTCODE      ExceptCode()     // THROW code

#endif // _EXCEPT_H 

