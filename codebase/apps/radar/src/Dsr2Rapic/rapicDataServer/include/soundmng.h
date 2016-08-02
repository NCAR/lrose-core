#ifndef __SOUNDMNG_H
#define __SOUNDMNG_H

/*
 * Sound manager class
 * Usually run in its own thread
 */

#include "spinlock.h"

#define	SOUNDQ_DEPTH

int playaiff(char *filename);

class soundentry {
friend class soundmng;
    char	filename[128];	    // AIFF filename
    soundentry	*next;
public:
    soundentry(char *filename = 0);	    // makes a copy of filename
    void setfname(char *filename);
};

class soundmng : public ThreadObj {
    spinlock	*lock;
    soundentry	*soundqhead, *soundqtail, *freeq;
    int		soundq_size, freeq_size;
    int		is_duplicate(char *newfname);
    void	workProc();
public:
    void StartThread();
    void StopThread();
    soundmng();
    ~soundmng();
    /*
     * called by other threads
     */
    void AddSound(char *filename); // checks for duplicates.
};

extern soundmng *SoundManager;

#endif /*__SOUNDMNG_H */

