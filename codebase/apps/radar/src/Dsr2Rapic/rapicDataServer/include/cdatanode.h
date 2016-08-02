#ifndef __CDATANODE_H__
#define __CDATANODE_H__

/*
 * cdatanode.h
 * 
 * CDataNode header file.
 * Class provides linked list of nodes 
 * which contain pointers to CData instances
 * 
 */

#include "cdata.h"
#include "linkedlisttemplate.h"

class CDataNode : public CVarLinkedList<CData> {
public:
    CDataNode(CData *newcdata = 0);    
    virtual	~CDataNode();
    virtual	void SetByNode(CDataNode *newcdatanode = 0);
    virtual	void Set(CData *newcdata = 0);
    virtual	void Dereference();	// remove reference to CData
    virtual	CDataNode* NextNode();
    virtual	CDataNode* PrevNode();
    virtual	CDataNode* FirstNode();
    virtual	CDataNode* LastNode();
    };

#endif
