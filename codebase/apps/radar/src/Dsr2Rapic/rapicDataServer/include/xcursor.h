/*
	xcursor.h

*/

#ifndef __XCURSOR_H_
#define __XCURSOR_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <X11/Xlib.h>

void InitXBusyCursor(Display *display, Window window);
void XBusyCursor(int Switch);

#ifdef __cplusplus
	}
#endif


#endif /* __XCURSOR_H_ */
