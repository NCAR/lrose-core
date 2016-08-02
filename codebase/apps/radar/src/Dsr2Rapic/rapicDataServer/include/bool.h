#ifndef	__BOOL_H
#define __BOOL_H
#ifdef NEED_BOOL_H
//#include <bool.h>  //moved outside ifndef below  SD 22/12/99
#include <stl_config.h>
#ifndef _BOOL 
/* bool not predefined */
//typedef unsigned char bool;
typedef unsigned char uchar;
//static const bool false = 0;
//static const bool true = 1;
#define _BOOL 1
#endif /* _BOOL */
#endif /* NEED_BOOL_H */

#endif	/* __BOOL_H */
