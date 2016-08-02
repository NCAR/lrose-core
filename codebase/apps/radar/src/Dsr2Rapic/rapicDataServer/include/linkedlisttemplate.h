/*
// CLinkedList - define a template class to perform the common linked list tasks

//   C A U T I O N 
//
//      - THIS CLASS IS ONLY INTENDED TO BE DERIVED FROM TO PROVIDE A
//        FUNDAMENTAL CHILD LINKAGE MECHANISM
//
//	- IF A LINKED LIST VARIABLE IS REQUIRED, CLinkedListVar MUST BE USED INSTEAD
//
*/

#ifndef _LINKEDLISTTEMPLATE_H_
#define _LINKEDLISTTEMPLATE_H_

#ifdef WIN
#include "stdafx.h"
#endif

template <class T> 
class CLinkedList {
	CLinkedList* next;
	CLinkedList* prev;
public:
	T* Next();
	T* Prev();
	T* First();
	T* Last();
	T* NthLast(int n);		// return nth last node (n=0=last)
	T* NthNode(int n);		// return nth node from start (n=0=first)
	CLinkedList();
	virtual ~CLinkedList();
	void  operator+=(T* a);		// append to linked list operator
	void	operator^=(T* a);		// linked list insertion operator
	void PreLink(CLinkedList<T>* basisnode);   // insert this node into list BEFORE supplied basis node
	void PostLink(CLinkedList<T>* basisnode);  // insert this node into list AFTER supplied basis node
	void AppendLink(CLinkedList<T>* basisnode);// insert at END of linked list
	void PrependLink(CLinkedList<T>* basisnode);// insert at BEGINNING of linked list
	void CollapseList();
/*	T*		operator--();
	T*		operator++();*/
};


template <class T>
CLinkedList<T>::CLinkedList()
{
	next = 0;
	prev = 0;
}

template <class T>
CLinkedList<T>::~CLinkedList()
{
	CollapseList();
}

// insert a new entry
template <class T>
void CLinkedList<T>::operator^=(T* newentry)
{
/*	newentry->next = (T*)this;
	newentry->prev = prev;
	if(prev)
		prev->next = newentry;
	prev = newentry;*/
	newentry->PostLink(this);
}

// append a new entry
template <class T>
void CLinkedList<T>::operator+=(T* newentry)
{
	T* end = (T*)this;
	while(end->Next())
		end = end->Next();

	newentry->PostLink(end);
	/*end->next = newentry;
	newentry->prev = end;*/
}

/*// return next entry 
template <class T>
T* CLinkedList<T>::operator++()
{
	return next;
}

// return prev entry 
template <class T>
T* CLinkedList<T>::operator--()
{
	return prev;
}
	*/

template <class T>
T* 
CLinkedList<T>::Next() 
{
	return (T*)next;
}

template <class T>
T* 
CLinkedList<T>::Prev() 
{
	return (T*)prev;
}

template <class T>
T* 
CLinkedList<T>::First() 
{
T*  thisnode = (T*)this;
	while ((T*)(thisnode->prev))
	    thisnode = (T*)(thisnode->prev);
	return thisnode;
}

template <class T>
T* 
CLinkedList<T>::Last() 
{
T*  thisnode = (T*)this;
	while ((T*)(thisnode->next))
	    thisnode = (T*)(thisnode->next);
	return thisnode;
}

template <class T>
T* 
CLinkedList<T>::NthLast(int n)
{
T* thisnode = Last();
	while (thisnode && (n > 0) && thisnode->prev)
	{
	    thisnode = (T*)(thisnode->prev);
	    n--;
	}
	return thisnode;
}

template <class T>
T* 
CLinkedList<T>::NthNode(int n)
{
T* thisnode = First();
	
	while (thisnode && (n > 0) && thisnode->next)
	{
	    thisnode = (T*)(thisnode->next);
	    n--;
	}
	return thisnode;
}

template <class T>	 // insert this node into list BEFORE supplied basis node
void 
CLinkedList<T>::PreLink(CLinkedList<T>* basisnode)
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
CLinkedList<T>::PrependLink(CLinkedList<T>* basisnode)
{
CLinkedList<T>* thisnode = basisnode;
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
CLinkedList<T>::PostLink(CLinkedList<T>* basisnode)
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
CLinkedList<T>::AppendLink(CLinkedList<T>* basisnode)
{
CLinkedList<T>* thisnode = basisnode;
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

template <class T> 
void
CLinkedList<T>::CollapseList()
{
	// collapse linked list
	if(next)
		next->prev = prev;
	if(prev)
		prev->next = next;
	next = 0;
	prev = 0;
}


#endif
