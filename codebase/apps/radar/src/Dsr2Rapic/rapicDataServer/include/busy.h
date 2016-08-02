/*
 * busy.h
 * 
 * Viewkit app busy cursor and dialog utilities
 * 
 */

#ifndef __BUSY_H__
#define __BUSY_H__

#ifdef __cplusplus
extern "C" {
#endif

/* void InitBusyCursor(Display display, Window window); */
void BusyCursor(int Switch);
void BusyDialog(char *busymessage, VkSimpleWindow *parent = NULL);
void BusyDialogOff();
void ProgressDialog(char *progressmessage, VkSimpleWindow *parent = NULL);
void ProgressDialogPercentDone(int pcdone, bool writepercent = TRUE);
  void ProgressDialogSetTitle(char *newtitle);
  void ProgressDialogUpdateTitle(char *newtitle, 
				 VkSimpleWindow *parent = NULL);
Boolean ProgressDialogInterrupted();
void ProgressDialogOff();
	
#ifdef __cplusplus
	}
#endif

#endif
