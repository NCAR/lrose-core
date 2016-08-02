/*

radlpnt.c

Implementation of the radl_pnt and radl_pnt_freelist classes

*/

#include "rdr.h"
#include "rdrscan.h"
#include "freelist.h"

radl_pnt_freelist::radl_pnt_freelist() {
  lock = new spinlock("radl_pnt_freelist->lock", 1000);  // 10 secs
  FreeList = 0;
  freeSize = 0;
  freeCount = 0;
}

radl_pnt_freelist::~radl_pnt_freelist() {
  radl_pnt *temp;
	
  if (lock)
    lock->get_lock();
  while (FreeList) {
    temp = FreeList->next;
    freeCount--;
    freeSize -= FreeList->getSize();
    delete FreeList;
    FreeList = temp;
  }
  if (lock)
    {
      lock->rel_lock();
      delete lock;
    }
}

void	radl_pnt_freelist::StoreRadlPnt(radl_pnt *StorePnt) {
  if (!StorePnt) return;
  if (lock)
    lock->get_lock();
  if (FreeList) FreeList->prev = StorePnt;
  StorePnt->next = FreeList;
  StorePnt->prev = 0;
  FreeList = StorePnt;
  freeCount++;
  freeSize += StorePnt->getSize();
  if (lock)
    lock->rel_lock();
}

radl_pnt* radl_pnt_freelist::GetRadlPnt(int NumRadials) {
  radl_pnt *temp;
  if (lock)
    lock->get_lock();
  temp = FreeList;
  while (temp && (temp->numradials != NumRadials))
    temp = temp->next;
  if (temp) {
    if (temp == FreeList) FreeList = temp->next;
    if (temp->prev) temp->prev->next = temp->next;
    if (temp->next) temp->next->prev = temp->prev;
    temp->next = temp->prev = 0;
    freeCount--;
    freeSize -= temp->getSize();
  }
  if (!temp) temp = new radl_pnt(NumRadials, this);
  if (temp) {
    int *ipnt = temp->PntTbl; 
    for (int x = 0; x < temp->numradials; x++) *ipnt++ = -1;
  }
  if (lock)
    lock->rel_lock();
  return temp;	
}

radl_pnt::radl_pnt(int NumRadials, void *freelist) {
  FreeList = 0;
  if (freelist) FreeList = freelist;
  else if (FreeListMng) FreeList = FreeListMng->RadlPntFreeList;
  PntTbl = new int[NumRadials];
  numradials = NumRadials;
  int *ipnt = PntTbl; 
  for (int x = 0; x < numradials; x++) *ipnt++ = -1;
  radltype = SIMPLE;
  next = prev = 0;
}

radl_pnt::~radl_pnt() {
  if (PntTbl) delete[] PntTbl;
}
	
int radl_pnt::getSize()
{
  return sizeof(*this) + (numradials * sizeof(int));
}
