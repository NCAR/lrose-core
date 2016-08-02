/*
 * "@(#)psload.h	3.9\t11/4/91 Copyright 1989 Sun Microsystems" 
 */

/*
 * psload.h --  definitions for loading PostScript files
 *
 * Copyright (c) 1990 by Sun Microsystems
 *
 * Package Author:  Chris White
 * Creation:  3/21/90
 * Revision History: 
 *      11/01/91,       BD	--	Added typename to stringtype structure
 *      02/05/91,       BD	--	Added stringtype structure
 *      04/03/90,       EDS	--	Made versions for c and C++
 *      mm/dd/yy,       Name	--	Reason
 *
 */

#define NO		0
#define YES		1
#define ERROR_CLIENT	2
#define ERROR_SERVER	3
#define MAX_LOCALE	200


typedef struct stringtype {
    char *key;
    char *data;
} stringtype_struct;


#ifdef c_plusplus

extern "C" {
    int loadPostScript(int *,char *,char *,char *,void (*loadfile) (),int);
}

#endif
