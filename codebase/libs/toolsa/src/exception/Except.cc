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
// except.c 
// 
//  Original author: J.A. Horsmeier  May, 3 1992     
//  Modified to more closely resemble ANSI C++ Exception syntax - F. Hage
// 
// note: all jmp_bufs are stored on the stack, by pushing another
//       scope block. The jmp_bufs are linked as a singly linked list.
//       A longjmp is always done to the top jmp_buf. If no top element
//       is present, the program aborts.
//
// note: except.h contains macros to facilitate the use of this simple
//       exception handling mechanism. Typical structure of the usage
//       of these macros look like this:
//
//     //  Working code section
//     TRY {    //Critical code that might do a THROW(<code>)  }
//     // Exception handling section
//     CATCH(<code>) {
//            This is optional, code here catches exception <code>
//     } CATCH(<code>) {
//         Similar to above
//     ...
//     ...
//     } CATCH_ALL {
//         Optional code, this code catches all other exceptions
//     }
//     END_EXCEPT Mandatory macro, it must match the TRY macro
//     ...
//      Start of normal (non critical) code
//
//       The entire CATCH ... CATCH CATCH_ALL sequence is just
//       optional. The CATCH macro must precede the CATCH_ALL
//       macros though. The END_EXCEPT macro is always the last macro.
//
//##############################################################################

// Included files

#include <cstdio>
#include <cstdlib>
#include <toolsa/Except.hh>
using namespace std;

// Defined macros

// Typedefs

// Static prototypes

// Static data
static except_t*  ExceptHead = NULL; // Head of the excep list
static except_t*  ExceptCurr = NULL; // Current excep 
static int        ExceptError =  0;       // Last exception

// Static functions

//##############################################################################
// External linkage functions
// Set another exception level
//
// note: The CATCH macros implement another (hidden to the user)
//  nested scope wherein a jmp_buf is allocated on the stack.
//  All jmp_bufs are linked into a single linked list, with the
//  last ExceptIst'ed buffer at the head of the list. The most
//  recent raise of an exception uses the buffer at the head of
//  this list.
//
void ExceptIst( except_t*  Except,const char *fname, int lineno)
{

    Except->Next= ExceptHead;
    Except->try_file_name = fname;
    Except->try_file_line = lineno;
    ExceptHead  = Except;
    ExceptCurr  = Except;

} // ExceptIst

//##############################################################################
// Drop the last exception level
//
void   ExceptRls()
{

    if (ExceptHead) ExceptHead= ExceptHead->Next;

} // ExceptRls

//##############################################################################
// Raise an exception
//
// note: This routine simply unlinks the head of the list jmp_buf and
//  does a longjmp to it, effectively returning to the ExceptIst
//  function that will in turn return to the caller that set up
//  this exception level.
//
void ExceptRaise(int ErrCode, const char *reason, const char *fname, int line)
{
except_t*  Except;

    if (!(Except= ExceptHead))  {          // No exception instance present
        fprintf(stderr,"\n\nUncaught Exception: %d encountered\n  %s\n",ErrCode,reason);
        fprintf(stderr,"Thrown at line %d in file %s\n",line,fname);
	if(ExceptCurr) {
          fprintf(stderr,"Within Try block at line %d in file %s\n",ExceptCurr->try_file_line,ExceptCurr->try_file_name);
	} else {
          fprintf(stderr,"Exception thrown external to any Try block\n");
	}
        fflush(stderr);
        printf("Exiting\n\n");
        exit(ErrCode);
    }

    Except->code = ErrCode;
    Except->file = fname;
    Except->line = line;
    Except->reason = reason;

    ExceptHead= Except->Next;

    longjmp(Except->JmpBuf, ExceptError = ErrCode);             // Jump back

} // ExceptRaise

//##############################################################################
// Get the last exception code
//
const char * ExceptReason() {

    if(ExceptCurr) {
      return (ExceptCurr->reason);
    } else {
      fprintf(stderr, "ERROR - ExceptReason passed NULL pointer\n");
      exit(-1);
    }

    return (NULL);

} // ExceptReason


//##############################################################################
// Get the last exception code
//
int ExceptCode() {

    return (ExceptError);

} // ExceptCode

//##############################################################################
// Get the file name where the throw is
//
const char * ExceptTryFile() {

    if(ExceptCurr) {
      return (ExceptCurr->try_file_name);
    } else {
      fprintf(stderr, "ERROR - ExceptTryFile passed NULL pointer\n");
      exit(-1);
    }
    
    return (NULL);

} // ExceptTryFile

//##############################################################################
// Get the  file line where the throw is
//
int ExceptTryLine() {

    if(ExceptCurr) {
      return (ExceptCurr->try_file_line);
    } else {
      fprintf(stderr, "ERROR - ExceptTryLine passed NULL pointer\n");
      exit(-1);
    }

    return (0);

} // ExceptTryLine


//##############################################################################
// Get the file name where the TRY is
//
const char * ExceptFile() {

    if (ExceptCurr) {
      return (ExceptCurr->file);
    } else {
      fprintf(stderr, "ERROR - ExceptFile passed NULL pointer\n");
      exit(-1);
    }

    return (NULL);

} // ExceptFile

//##############################################################################
// Get the  file line where the TRY is
//
int ExceptLine() {

    if(ExceptCurr) {
      return (ExceptCurr->line);
    } else {
      fprintf(stderr, "ERROR - ExceptLine passed NULL pointer\n");
      exit(-1);
    }

    return (0);

} // ExceptLine
