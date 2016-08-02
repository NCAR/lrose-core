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
/**************************************************************************
 * XVIEW_FILE_ACCESS.C: Routines to check for reasonable file access and
 *    to pop up a conformation window if anything is wrong, etc 
 *    Callable from within XView Applications only!
 *
 * F. Hage    NCAR/RAP 1991.
 *
 * KEYWORD: XView Notice, File Access Checking
 */

#define  XVIEW_FILE_ACCESS

#include "cidd.h"

extern    int    errno;
/**************************************************************************
 * OPEN_CHECK_WRITE: Open a file for writing and if there is an error,
 *          or the file exists,    Display a popup notice with the cause of
 *           the error and an OK button. Returns a FILE * or Null if error.
 */
FILE* open_check_write(const char *file_name, Frame    owner)
{
    int    result;
    char    message[80];
    struct stat filestat;

    if(stat(file_name,&filestat)) {
        switch(errno) {
            case ENOTDIR:
                sprintf(message,"No such directory");
            break;

            case EACCES:
                sprintf(message,"Sorry, Can not access named directory");
            break;

            case ELOOP:
                sprintf(message,"Too many symbolic links to resolve");
            break;
        }

        if(errno != ENOENT) {
             result = notice_prompt(owner,NULL,
                         NOTICE_MESSAGE_STRINGS,message,NULL,
                         NOTICE_BUTTON,  "OK", 101,
                         NULL);
            return NULL;
        }
    } else { 

        result = notice_prompt(owner,NULL,
                     NOTICE_MESSAGE_STRINGS,"File exists!    Overwrite?" ,NULL,
                     NOTICE_BUTTON_YES,    "Yes",
                     NOTICE_BUTTON_NO,  "No",
                     NULL);

        if(result == NOTICE_NO) return NULL;
    }
        
    return  fopen(file_name,"w");
}

/**************************************************************************
 * OPEN_CHECK_READ: Open a file for reading and if there is an error,
 *            Display a popup notice with the cause of the error and an OK
 *            button. Returns a FILE * or Null if error
 */

FILE * open_check_read( char    *file_name, Frame    owner)
{
    int    result;
    char    message[80];
    struct stat filestat;


    if(stat(file_name,&filestat)) {
        switch(errno) {
            case ENOTDIR:
                sprintf(message,"No such directory");
            break;

            case EACCES:
                sprintf(message,"Sorry, Can not access named directory");
            break;

            case ELOOP:
                sprintf(message,"Too many symbolic links to resolve");
            break;

            case ENOENT:
                sprintf(message,"File does not exist");
            break;
        }

         result = notice_prompt(owner,NULL,
                     NOTICE_MESSAGE_STRINGS,message,NULL,
                     NOTICE_BUTTON,  "OK", 101,
                     NULL);
        return NULL;
    }
     
    return fopen(file_name,"r"); 
}

/*****************************************************************************
 * CHDIR_CHECK: Change the current directory if possible. Popup error
 *        box if necessary. Returns 0 on success -1 on failure
 */

int chdir_check( char * path, Frame    owner)

{
    int    result;
    char    message[80];

    if(chdir(path)) {
        switch(errno) {
            case ENOENT:
            case ENOTDIR:
                sprintf(message,"No such directory");
            break;

            case EINVAL:
                sprintf(message,"Illegal character in name");
            break;

            case EACCES:
                sprintf(message,"Sorry, Access denied for named directory");
            break;

            case ELOOP:
                sprintf(message,"Too many symbolic links to resolve");
            break;

            case EIO:
                sprintf(message,"I/O error occured, try again");
            break;
        }

         result = notice_prompt(owner,NULL,
                     NOTICE_MESSAGE_STRINGS,message,NULL,
                     NOTICE_BUTTON,  "OK", 101,
                     NULL);
        return -1;
    }
     return 0;
}
