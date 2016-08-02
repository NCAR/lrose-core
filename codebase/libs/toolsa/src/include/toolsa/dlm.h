/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
#ifdef __cplusplus
 extern "C" {
#endif
#ifndef DLM_WAS_INCLUDED
#define DLM_WAS_INCLUDED

/* Summary of functions:
      Create, destroy:
         DLMcreateList, DLMdestroyList, DLMdestroyHead

      Add nodes:
         DLMaddAfter, DLMappend, DLMinsert, DLMprepend

      Remove nodes:
         DLMremove, DLMpop

      Traverse list:
         DLMfirst, DLMnext
         DLMlast, DLMprev
         DLMclone

      Search for specified node:
         DLMsearch, DLMsearchC

      Currency:
         DLMgetCurrent, DLMsetCurrent, DLMpromote

      Stack Operations:
         DLMpush, DLMpop, DLMpromote
 */


/************* typedefs **************/

typedef  int (*DLMlessthan_fn)(void *data1, void *data2);
   /* the order_fn must define a fully ordered relation such that
      it returns TRUE when data1 < data2.
      used in DLMinsert() */

typedef  int (*DLMequal_fn)(void *data, void *param);
   /* the equal_fn must return TRUE when data equals the desired node.
      param is an arbitrary pointer used by equal_fn to determine the
      "desired" node.
      Used in DLMsearch() */

#ifndef THIS_IS_DLM
typedef  void *    DLMlist;  /* DLMlist is hidded from end-users */
#endif


extern  void *DLMaddAfter(DLMlist *list,void *data);
   /*  Create a node, copy the specified data into it, and insert it 
       after the current node. Return a pointer to where the new data
       is stored.
       This makes the new node the current node.
    */

extern  void *DLMappend(DLMlist *list, void *data);
   /*  Create a node, copy the specified data in it, and append it to the
       specified list. Return a pointer to where the new data
       is stored.
       This makes the new node the current node.
    */

extern  DLMlist *DLMclone(DLMlist *original);
  /*
    Copy an existing List Header, and return a pointer to the
    new List Header
    This allows independent traversal of the same list
    It is assumed that the list doesnt change during this traversal
    Use DLMdestroyHead when you're done

   e.g:
         read through the list
         clone = DLMclone( Flag_list);

         data = DLMfirst( clone);
         while ( NULL != data)
            {
            process data
	    data = DLMnext( clone); 
            }

         DLMdestroyHead( clone);

  */

extern  DLMlist *DLMcreateList( size_t data_size);
/* Initialize an empty list. Return NULL on error.

   data_size is the size in bytes of the data structures.
   If data_size = 0, calling routine will malloc() the data, and DLM will
	store pointers. (must use malloc because destroy call free()).
   If data_size > 0, DLM will allocate storage and makle a copy of the data 
	when a new node is added.
 */


extern  void DLMdestroyHead(DLMlist *list);
  /*  Destroy the list header only */


extern  void DLMdestroyList(DLMlist *list);
/*
   Unconditionally destroy a list and release ALL the resources
   associated with it including the data pointed to by the link nodes.
   If data_size = 0, data must have been malloced, because this routine will
   call free().
 */


extern  void    *DLMfirst(DLMlist *list);
extern  void    *DLMnext(DLMlist *list);

extern  void    *DLMlast(DLMlist *list);
extern  void    *DLMprev(DLMlist *list);

  /*
   These functions are for traversing the linked list. 
   Returns a pointer to the current node's data
   NULL is returned if the list is at the end. 

   Example:

        forward traversal 
        data = DLMfirst(list);
        while (NULL != data)
           {
           manipulate data record 
           data = DLMnext(list);
           }
  */


extern  void *DLMgetCurrent(DLMlist *list);
   /*  Fetch the current node's data without changing the currency.
       if no current node, return NULL */

extern  void *DLMinsert(DLMlist *list,void *data,DLMlessthan_fn order_fn);
  /*   Create a node, copy the specified data in it, and insert it in the 
       list determined by the ordering function order_fn 
       (see above DLMlessthan_fn).
       The list must already be in that order.
       Return pointer to where the data is stored.
       This makes the new node the current node.

   e.g.:
      int LessTh( Flagstruct *data1, Flagstruct *data2)
         {
         return (data1->flag < data2->flag);
         }

      Flagstruct this;

      this.flag = 12;
      DLMinsert(Flag_list, &this, (DLMlessthan_fn) LessTh);
   */

/*** the next three functions implement a stack */

extern  void *DLMpop(DLMlist *list);
/* Delete the first node and free the memory associated with the data.
   the following node (if any) becomes the current node, and is returned
 */

#define DLMprepend DLMpush

extern  void DLMpromote(DLMlist *list, void *this_data);
/* Find the node whose data is pointed to by "this_data", 
 * relink it to the start of the stack, and make it current  
 */

extern  void *DLMpush(DLMlist *list, void *data);
/* Create a node and prepend it to the specified list.
   Copy the specified data into it, return a pointer to the
   new data.
   Make the new node the current node.
*/

extern  void DLMremove(DLMlist *list, void *data);
/*
   Unlink and delete the specified node and free the memory
   associated with the data.
        The deletion does NOT affect the currency of the list UNLESS 
   the node to be deleted is the current node, in which case the
   the next node becomes current (if null, previous node becomes current).
 */

extern  void *DLMsearch(DLMlist *list, DLMequal_fn equal_fn, void *param);
extern  void *DLMsearchC(DLMlist *list,DLMequal_fn equal_fn, void *param);

   /*   Search through the list until you find the node, specified by the 
   search function.  Return NULL if not found.
      For DLMsearchC, the found node becomes current; for DLMsearch, the 
   currency of the list is not affected.
      The search function is passed the data and the param, and it must 
   return true when the node is found (see above def of DLMequal_fn).

   e.g.:
      int ChkEqFlag( Flagstruct *data, int *want)
         {
         return (data->flag == *want);
         }

      Flagstruct *this;
      want_flag = 12;
      this = DLMsearch(Flag_list, (DLMequal_fn) ChkEqFlag, &want_flag);

   */


extern  void DLMsetCurrent(DLMlist *list, void *data);
   /*  set the current node */


void DLMdump( DLMlist *head);
/* debugging dump of head and nodes addresses */

#endif
#ifdef __cplusplus
}
#endif
