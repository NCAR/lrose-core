/*
	freelist.c

	IMplementation of the free_list_mng class.

*/

#include "freelist.h"
#include "rdr.h"

free_list_mng *FreeListMng = 0;

free_list_mng::free_list_mng() {
  ExpBuffFreeList = new expbuff_freelist;
  RadlPntFreeList = new radl_pnt_freelist;
  SpinLockFreeList = new spinl_freelist;
  // SCANFREELIST	RdrScanFreeList = new rdrscan_freelist;
  //SCANFREELIST	RdrScanNodeFreeList = new rdrscannode_freelist;
}

free_list_mng::~free_list_mng() {
  if (ExpBuffFreeList) delete ExpBuffFreeList;
  if (RadlPntFreeList) delete RadlPntFreeList;
  if (SpinLockFreeList) delete SpinLockFreeList;
  // SCANFREELIST	if (RdrScanFreeList) delete RdrScanFreeList;
  // SCANFREELIST	if (RdrScanNodeFreeList) delete RdrScanNodeFreeList;
}

void free_list_mng::PrintFreeListState(FILE *file, bool verbose)
{
  if (ExpBuffFreeList && file) fprintf(file, "ExpBuffFreeList nodes=%d size=%1.0fMB\n", 
					ExpBuffFreeList->getCount(),
					float(ExpBuffFreeList->getSize()/1000000.0));
  if (RadlPntFreeList && file) fprintf(file, "RadlPntFreeList nodes=%d size=%1.0fMB\n", 
					RadlPntFreeList->getCount(),
					float(RadlPntFreeList->getSize()/1000000.0));
}
