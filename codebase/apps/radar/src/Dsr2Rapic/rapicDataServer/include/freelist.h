#ifndef	__FREELIST_H
#define __FREELIST_H

#include "rdrscan.h"
#include "spinlock.h"

class free_list_mng {
public:
	expbuff_freelist *ExpBuffFreeList;
	radl_pnt_freelist *RadlPntFreeList;
	spinl_freelist *SpinLockFreeList;
// SCANFREELIST	rdrscan_freelist *RdrScanFreeList;
// SCANFREELIST	rdrscannode_freelist *RdrScanNodeFreeList;
	free_list_mng();
	~free_list_mng();
	void	PrintFreeListState(FILE *file, bool verbose = false);
	};

extern free_list_mng *FreeListMng;

#endif	/* __FREELIST_H */
