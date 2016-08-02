/* memory fragmentation avoidance new/delete operators

	 The basic concept is that as objects are deleted, the memory is not given 
	 back to the operating system but a reference is added to a free list
	 When new is called in the future, this discarded memory is then re-used
	 if no memory is available for re-use, a fresh allocation is made


	 Typically this header file will need to be included 3 times for a particular class

		1.	The header must be initially included to define the CDefragList class

		2.  The header must then be included again within a public section of the class 
				declaration with DEFRAGDECLARATION defined eg.

					#define DEFRAGDECLARATION
					#include "defrag.h"

		3.	The header must then be included within the source file which typically defines 
				the class methods with DEFRAGSOURCE defined and	CLASSTYPE defined to be the 
				namespace of the class eg.
				
					#define CLASSTYPE txdevice
					#define DEFRAGSOURCE
					#include "defrag.h"
*/


// *************************************************************************************
// defrag.h must be included at least once before a class requiring defrag protection
// is required.
// the default action of the sentries will prevent CDeFragList being re-defined.

#ifndef _DEFRAGPROTECTION_H_
#define _DEFRAGPROTECTION_H_

#include "linkedlisttemplate.h"
#include "varlinkedlisttemplate.h"

#include <string.h>

class CDeFragList : public CLinkedList<CDeFragList> {
public:
	char IDString[80];
	int  nInUseCount;
	int  nFreeListCount;
	CVarLinkedList<void> *pInUseList;
	CVarLinkedList<void> *pFreeList;
public:
	CDeFragList() { 
		pInUseList = NULL; 
		pFreeList = NULL;
		nInUseCount = 0;
		nFreeListCount = 0;
		IDString[0] = 0;
	};
	~CDeFragList() { 
		if(pInUseList) {
			while(pInUseList->Next()) {
/*				if(pInUseList->Next()->ThisClass())
					delete pInUseList->Next()->ThisClass();*/
				delete pInUseList->Next();
			}
			delete pInUseList;
		}
		if(pFreeList) {
			while(pFreeList->Next()) {
/*				if(pFreeList->Next()->ThisClass())
					delete pFreeList->Next()->ThisClass();*/
				delete pFreeList->Next();
			}
			delete pFreeList;
		}
		pInUseList = NULL;
		pFreeList = NULL;
	}
};

#endif


#ifdef DEFRAGDECLARATION
// *************************************************************************************
// these entries need to appear in the class declaration header file
//
// do this by including defrag.h at the point required within the header with 
// DEFRAGDECLARATION defined eg.
//
//				#define DEFRAGDECLARATION
//				#include "defrag.h"
//

	static  CDeFragList DeFragList;			// static ensures there is only one instance for all instances of required class
	static  spinlock DeFragLock;
	void*		operator new(size_t nSize);
	void		operator delete(void* pObj);
	CVarLinkedList<void> *ReUseNode;

#undef DEFRAGDECLARATION					// prevent accidental re-use in source file
#endif



#ifdef DEFRAGSOURCE
// *************************************************************************************
// these entries need to appear in the class definition file
//
// do this by including defrag.h within the source file with DEFRAGSOURCE defined
// and CLASSTYPE defined to the class namespace eg.
//
//				#define CLASSTYPE txdevice
//				#define DEFRAGSOURCE
//				#include "defrag.h"
//

// declare static variables for the required class 
CDeFragList CLASSTYPE::DeFragList;
spinlock    CLASSTYPE::DeFragLock("DefragLock");

#ifndef ASSERT
extern void ASSERT(void* ptr);
#endif

void*
CLASSTYPE::operator new(size_t nSize)
{
	CVarLinkedList<void> *re_use_node;				// pointer to node which will be re-used
	void* retval = NULL;

	DeFragLock.get_lock();			// serialise calls to freelist

	if(DeFragList.pFreeList) {  // check if previously used memory is available

		retval = DeFragList.pFreeList->ThisClass();	// extract pointer to previously allocated block

		re_use_node = DeFragList.pFreeList;								// point to first available node to be re-used
		ASSERT(re_use_node);

		DeFragList.pFreeList = DeFragList.pFreeList->Next();		// update freelist head pointer BEFORE we collapse 

		re_use_node->CollapseList();											// detach linkage from freelist (next=prev=NULL)
		DeFragList.nFreeListCount--;
	}
	else {						// no free memory - allocate a new block and freelist node
		retval = ::new char[nSize];
		re_use_node = new CVarLinkedList<void>(retval);		// new node which can be re-used
		ASSERT(re_use_node);
	}

	re_use_node->PreLink(DeFragList.pInUseList);			// insert new node to head of InUse list
	DeFragList.pInUseList = re_use_node;							// set head pointer			
	DeFragList.nInUseCount++;

	((CLASSTYPE*)retval)->ReUseNode = re_use_node;				// link back to node for efficient delete

	DeFragLock.rel_lock();
	return retval;
}


// over-ridden delete actually does not free up memory but places this memory 
// block onto a freelist
void
CLASSTYPE::operator delete(void *pObj)
{
	ASSERT(pObj);
	CVarLinkedList<void> *re_use_node;				// pointer to node which is to be re-used

	DeFragLock.get_lock();

	re_use_node = ((CLASSTYPE*)pObj)->ReUseNode;
	ASSERT(re_use_node);
	if(DeFragList.pInUseList == re_use_node)							// safeguard from culling list head
		DeFragList.pInUseList = DeFragList.pInUseList->Next();
	re_use_node->CollapseList();							// remove node from InUse list
	DeFragList.nInUseCount--;
	re_use_node->PreLink(DeFragList.pFreeList);					// and insert onto Free list
	DeFragList.pFreeList = re_use_node;
	DeFragList.nFreeListCount++;

	DeFragLock.rel_lock();
}

#undef DEFRAGSOURCE
#undef CLASSTYPE
#endif // DEFRAGSOURCE
