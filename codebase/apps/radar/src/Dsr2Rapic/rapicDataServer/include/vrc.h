#ifndef __VRC_H
#define __VRC_H

/*
 *  vrc.h
 * 
 */

#ifdef __cplusplus
extern "C" {
#endif
	void InitVRC(int argc,char **argv);
	void PollVRC();
	void CloseVRC();

extern int UIMXIFace;

#ifdef __cplusplus
	}
#endif

#endif		/* __VRC_H */
