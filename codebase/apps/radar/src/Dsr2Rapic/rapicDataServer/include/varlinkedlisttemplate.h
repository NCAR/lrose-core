// varlinkedlisttemplate.h

#include "log.h"

#ifndef _VARLINKEDLIST_H_
#define _VARLINKEDLIST_H_

template <class T>
class CVarLinkedList {
	CVarLinkedList *next,*prev;
	T*	pThisClass;
public:
	CVarLinkedList(T*	ThisClass=0);					// attach to given class
	virtual ~CVarLinkedList();			
	void AttachClass(T*	ThisClass=0);				// attach to given class
	void DetachClass();										// detach from current class
	T*   ThisClass();										// detach from current class
	CVarLinkedList* Next();
	CVarLinkedList* Prev();
	CVarLinkedList* First();
	CVarLinkedList* Last();
	CVarLinkedList* NthLast(int n);		// return nth last node (n=0=last)
	CVarLinkedList* NthNode(int n);		// return nth node from start (n=0=first)
	void PreLink(CVarLinkedList<T>* basisnode);   // insert this node into list BEFORE supplied basis node
	void PostLink(CVarLinkedList<T>* basisnode);  // insert this node into list AFTER supplied basis node
	void AppendLink(CVarLinkedList<T>* basisnode);// insert at END of linked list
	void PrependLink(CVarLinkedList<T>* basisnode);// insert at BEGINNING of linked list
	void CollapseList();
};


template <class T>
CVarLinkedList<T>::CVarLinkedList(T* Class)
{
	next = prev = 0;
	pThisClass = Class;
}


template <class T>
CVarLinkedList<T>::~CVarLinkedList()
{
	CollapseList();
}

template <class T>
void 
CVarLinkedList<T>::AttachClass(T* Class) {
	if (pThisClass) {
		RapicLog("CVarLinkedList::AttachClass - Class already attached\n", LOG_ERR);
		return;
	}
	pThisClass = Class;
}    

template <class T>
void 
CVarLinkedList<T>::DetachClass() {
	if (!pThisClass) {
		RapicLog("CVarLinkedList::DetachClass - Class not defined\n", LOG_ERR);
		return;
	}
	pThisClass = 0;
}    

template <class T>
T* 
CVarLinkedList<T>::ThisClass() 
{
	return pThisClass;
}

template <class T>
CVarLinkedList<T>* 
CVarLinkedList<T>::Next() 
{
	if (this)
	    return next;
	else 
	    return 0;
}

template <class T>
CVarLinkedList<T>* 
CVarLinkedList<T>::Prev() 
{
	if (this)
	    return prev;
	else 
	    return 0;
}

template <class T>
CVarLinkedList<T>* 
CVarLinkedList<T>::First() 
{
CVarLinkedList<T>* thisnode = this;
	while (thisnode && thisnode->prev)
	    thisnode = thisnode->prev;
	return thisnode;
}

template <class T>
CVarLinkedList<T>* 
CVarLinkedList<T>::Last() 
{
CVarLinkedList<T>* thisnode = this;
	while (thisnode && thisnode->next)
	    thisnode = thisnode->next;
	return thisnode;
}

template <class T>
CVarLinkedList<T>* 
CVarLinkedList<T>::NthLast(int n)
{
CVarLinkedList<T>* thisnode = Last();
	while (thisnode && (n > 0) && thisnode->prev)
	{
	    thisnode = thisnode->prev;
		n--;
	}
	return thisnode;
}

template <class T>
CVarLinkedList<T>* 
CVarLinkedList<T>::NthNode(int n)
{
CVarLinkedList<T>* thisnode = First();
	
	while (thisnode && (n > 0) && thisnode->next)
	{
	    thisnode = thisnode->next;
		n--;
	}
	return thisnode;
}

template <class T>	 // insert this node into list BEFORE supplied basis node
void 
CVarLinkedList<T>::PreLink(CVarLinkedList<T>* basisnode)
{
	if(basisnode) {   // check we have a node to work against
		prev = basisnode->prev;
		if(prev)
			prev->next = this;
		basisnode->prev = this;			
		next = basisnode;
	}
}

template <class T>   // insert this node at the START of the list
void 
CVarLinkedList<T>::PrependLink(CVarLinkedList<T>* basisnode)
{
CVarLinkedList<T>* thisnode = basisnode;
	if(thisnode) {   // check we have a node to work against
		while (thisnode->prev)		// if there is a prev
		    thisnode = thisnode->prev;	// we are not at start yet
		prev = thisnode->prev;
		if(prev)
			prev->next = this;
		thisnode->prev = this;
		next = thisnode;
	}
}

template <class T>   // insert this node into list AFTER supplied basis node
void 
CVarLinkedList<T>::PostLink(CVarLinkedList<T>* basisnode)
{
	if(basisnode) {   // check we have a node to work against
		next = basisnode->next;
		if(next)
			next->prev = this;
		basisnode->next = this;
		prev = basisnode;
	}
}

template <class T>   // insert this node at the END of the list
void 
CVarLinkedList<T>::AppendLink(CVarLinkedList<T>* basisnode)
{
CVarLinkedList<T>* thisnode = basisnode;
	if(thisnode) {   // check we have a node to work against
		while (thisnode->next)		// if there is a next
		    thisnode = thisnode->next;	// we are not at end yet
		next = thisnode->next;
		if(next)
			next->prev = this;
		thisnode->next = this;
		prev = thisnode;
	}
}

template <class T>   // move this node into list AFTER supplied basis node
void 
CVarLinkedList<T>::CollapseList()
{
	if(next)
		next->prev = prev;
	if(prev)
		prev->next = next;
	next = 0;
	prev = 0;
}

#endif
