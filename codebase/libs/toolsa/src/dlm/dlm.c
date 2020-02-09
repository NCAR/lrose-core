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
/* DLM.C    Doubly-linked List Manager                                  */
/*                                                                      */
/* Modification History :                                               */
/*   Creation        : 10/9/91
 *   Modifications
      2/2/92:  make data blocks optionally seperate from nodes
               change to getFirst, getLast, more like db
      12/30/92: use malloc, not MEM; rename traversal routines
 */

#include <stdio.h>  
#include <stdlib.h> 

#include <toolsa/globals.h>
#include <toolsa/err.h>
#include <toolsa/str.h>

typedef struct DLMnode_
   {
      struct   DLMlist_ *list;   /* ptr to list this node belongs to */
      struct   DLMnode_ *prev;   /* ptr to previous node  */
      struct   DLMnode_ *next;   /* ptr to next node      */
      void     *data;            /* pointer to the data */
   } DLMnode;

typedef struct DLMlist_
   {
      DLMnode  *first,       /* pointer to first node        */
               *last,        /* pointer to last node         */
               *current;     /* pointer to current node      */
      short    length;       /* number of nodes in list. */
      size_t   data_size;    /* size of data structures.
                                if 0, dont allocate, but store pointers.
                                if > 0, allocate and store data itself */
   } DLMlist;

#define THIS_IS_DLM
#include <toolsa/dlm.h>

   /* find the node with the given data pointer */
static DLMnode *FindNode( DLMlist *list, void *data)
   {
      DLMnode *node = list->first;
      while (NULL != node)
         {
         if (node->data == data)
            return node;
         node = node->next;
         }
      return NULL;
   }

   /* free the node, and data if appropriate */
static void FreeNode( DLMlist *list, DLMnode *this)
   {
      if (0 == list->data_size)
         free( this->data);
      free( this);
   }

static void LinkNode( DLMlist *list, DLMnode *this, DLMnode *previous)
   {
      /* forward link */
      if (previous == NULL)
         {
         this->next = list->first;
         list->first = this;
         }
      else
         {
         this->next = previous->next;
         previous->next = this;
         }

      /* backward link */
      this->prev = previous;
      if (this->next == NULL) 
         {
         list->last = this;
         }
      else
         {
         this->next->prev = this;
         }

      this->list = list;
      list->length++;
   }

static DLMnode *NewNode( DLMlist *list, void *data)
   {
      DLMnode *node;

      if (0 == list->data_size)
         {
         if (NULL == (node = (DLMnode *) calloc( sizeof(DLMnode), 1)))
            return NULL;
         node->data = data;
         }
      else
         {
         char *ptr;
         
         if (NULL == (ptr = calloc( list->data_size + sizeof(DLMnode), 1)))
               return NULL;
         node = (DLMnode *) ptr;
         node->data = ptr + sizeof( DLMnode);
         STRcopy( node->data, data, list->data_size);
         }

      return node;
   }

static int UnlinkNode(DLMlist *list, DLMnode *this)
   {
      /* error checking */
      if ( (this == NULL) || (this->list != list))
         {
         /* call err to report an error with the unlink */
         ERRprintf( ERR_PROGRAM, "Unlink DLM node %p error %p != %p",
            this, this->list, list);
         return(FALSE);
         }

      /* forward link */
      if (this->prev == NULL)
         {
         list->first = this->next;
         }
      else
         {
         this->prev->next = this->next;
         }

      /* backward link */
      if (this->next == NULL) 
         {
         list->last = this->prev;
         }
      else
         {
         this->next->prev = this->prev;
         }

      this->list = NULL;
      list->length--;
      return(TRUE);
   }

/*********************************************************************/
/* Exported functions                                                */
/*********************************************************************/

/* DLMaddAfter()
 *
 * Create a node and add it after the current node.
 * the new node is made current
 */

void *DLMaddAfter( DLMlist *list, void *data)
   {
      DLMnode  *node, *prev;

      if (NULL == (node = NewNode( list, data)))
         return NULL;

      prev = list->current;
      LinkNode(list, node, prev);
      list->current = node;

      return node->data; 
   }

/* DLMappend()
 *
 * Create a node and append it to the specified list.
 * Appending makes the last node the current node.
 */

void *DLMappend( DLMlist *list, void *data)
   {
      DLMnode  *node, *prev;

      if (NULL == (node = NewNode( list, data)))
         return NULL;

      prev = list->last;
      LinkNode(list, node, prev);
      list->current = node;

      return node->data; 
   }

/* DLMclone()
 *
 * Copy an existing List Header, and returns a pointer to the
 * different List Header
 * This allows seperate traversal of the same list
 * It is assumed that the list doesnt change during this traversal
 */

DLMlist  *DLMclone( DLMlist *original)
   {
      DLMlist  *clone = NULL;
   
      if ( (clone = (DLMlist *) calloc(sizeof(DLMlist), 1)) != NULL )
         {
         memcpy( clone, original, sizeof( DLMlist));
         return (clone);
         }
      else
         return (NULL);
   } 

/* DLMcreateList()
 *
 * Creates a list and returns a pointer to the list's header record.
 * The length of the list is set to zero. The first, last and current
 * nodes are all set to NULL.
 */
DLMlist *DLMcreateList( size_t data_size)
   {
      DLMlist  *list = NULL;
   
      if ( NULL != (list = (DLMlist *) calloc(sizeof(DLMlist), 1)))
         {
         list->length   = 0;
         list->first    = NULL;
         list->last     = NULL;
         list->current  = NULL;
         list->data_size = data_size;
         return (list);
         }
      else
         return (NULL);

   }  

/* DLMdestroyHead()
 *
 * Destroy a list header and release its memory
 */
void  DLMdestroyHead( DLMlist *list)
   {
      free( list);
   } 

/* DLMdestroyList()
 *
 * Unconditionally destroy a list and release ALL the resources
 * associated with it including the data pointed to by the link nodes.
 */

void DLMdestroyList( DLMlist *list)
   {
      DLMnode *next, *this;

      this = list->first;
      while (this != NULL)
         {
         if ( this->list != list)
            {
            ERRprintf( ERR_PROGRAM, "Destroy DLM node %p error %p != %p",
               this, this->list, list);
            return;
            }

         next = this->next;
         FreeNode(list, this);
         this = next;
         }

      free(list);
      return;
   }

/* DLMgetCurrent()
 *
 * Return a pointer to the datum associated with the current node.
 */

void *DLMgetCurrent( DLMlist *list)
   {
      if (NULL == list->current)
         return NULL;
      else
         return list->current->data;
   }

/* DLMfirst, DLMlast
 *
 * Initialize traversal
 */

void  *DLMfirst( DLMlist *list)
   {
      list->current = list->first;
      return ((list->current == NULL) ? NULL : list->current->data);
   }
void  *DLMlast( DLMlist *list)
   {
      list->current = list->last;
      return ((list->current == NULL) ? NULL : list->current->data);
   }

/* DLMnext(),  DLMprev()
 *
 * increment (decrement) the current node and return its data.
 * return NULL if at end (beginning) of list
 * dont move currency past the list
 */

void *DLMnext( DLMlist *list)
   {
      if ((list->current == NULL) || (list->current->next == NULL))
         return NULL;

      list->current = list->current->next;
      return list->current->data;
   }

void *DLMprev( DLMlist *list)
   {
      if ((list->current == NULL) || (list->current->prev == NULL))
         return NULL;

      list->current = list->current->prev;
      return list->current->data;
   }


/* DLMinsert()
 *
 *  Insert the specified node in the list, using the ordering
 *  function to indicate where in the list. 
 *  This function must define a fully ordered relation on the 
 *  data records; the new node will be inserted into this ordering; 
 *  the list must already be in the that order.
 */
void *DLMinsert(DLMlist *list,void *data,DLMlessthan_fn order_fn)
   {
      DLMnode  *node, *new_node, *prev_node = NULL;

      if (NULL == (new_node = NewNode( list, data)))
         return NULL;

      /* run through the list until we find where to insert it */
      node = list->first;
      while( NULL != node)
         {
         /* call the ordering function - returns true until we
          * get to the first element greater than the element to insert */
         if ((*order_fn)(data, node->data))
            break;

         prev_node = node;
         node = node->next;
         }

      LinkNode(list, new_node, prev_node);
      list->current = new_node;

      return new_node->data; 
   }


/* DLMpop()
 *
 * Delete the first node and free the memory associated with the data.
 * the following node becomes the current node, and is returned
 */

void *DLMpop( DLMlist *list)
   {
      DLMnode  *node;

      if (NULL == (node = list->first))
         return NULL;
      list->current = list->first;
      DLMremove( list, node->data);
      if (list->current == NULL)
         return NULL;
      else
         return list->current->data;
   }


/* DLMpromote()
 *
 * Find the node, and make it the first node
 * it also becomes the current node
 */

void DLMpromote( DLMlist *list, void *data)
   {
      DLMnode  *node = NULL;

      if (NULL == (node = FindNode( list, data)))
         return;
      if (UnlinkNode( list, node) == FALSE)
         return;

      LinkNode( list, node, NULL);
      list->current = node;
   } 

/* DLMpush(), same as DLMprepend()
 *
 * Create a node and prepend it to the specified list.
 * Prepending makes the first node the current node.
 */

void  *DLMpush( DLMlist *list, void *data)
   {
      DLMnode  *node, *prev;

      if (NULL == (node = NewNode( list, data)))
         return NULL;

      prev = NULL;
      LinkNode(list, node, prev);
      list->current = node;

      return node->data;
   } 

/* DLMremove()
   Unlink and delete the specified node and free the memory
   associated with the data.
        The deletion does NOT affect the currency of the list UNLESS 
   the node to be deleted is the current node, in which case the
   the next node becomes current (if null, previous node becomes current).
 */

void DLMremove( DLMlist *list, void *data)
   {
      DLMnode  *node;

      if (NULL == (node = FindNode( list, data)))
         {
         ERRprintf(ERR_PROGRAM,"DLMremove didnt find %p", data);
         return;
         }

      /* currency */
      if (list->current == node)
         {
         if (node->next != NULL)
            list->current = node->next;
         else
            list->current = node->prev;
         }

      if (!UnlinkNode( list, node))
         return;

      FreeNode(list, node);
   }


/* DLMsearchC
 *  Search through the list until you find the node, specified by the 
 *  search function. Found node becomes current.
 *  Return NULL in data pointer, NULL in node pointer if not found.
 */

extern  void *DLMsearchC(DLMlist *list,DLMequal_fn equal_fn, void *param)
   {
      void     *data;

      data = DLMfirst(list);
      while ( NULL != data)
         {
         if ((*equal_fn)(data, param))
            return(data);
         data = DLMnext(list);
         }

      /* didnt find it */
      return(NULL);
   }


/* DLMsearch
 *  same as searchC, but dont change currency
 */

extern  void *DLMsearch(DLMlist *list, DLMequal_fn equal_fn, void *param)
   {
      DLMnode  *save;
      void     *data;

      save = list->current;
      data = DLMsearchC( list, equal_fn, param);
      list->current = save;

      return(data);
   }

/* DLMsetCurrent()
 *
 * Set the current node.
 */

void DLMsetCurrent( DLMlist *list, void *data)
   {
      DLMnode *node;

      if (NULL != (node = FindNode( list, data)))
         list->current = node;
   }

      
void DLMdump( DLMlist *head)
   {
      DLMnode *node;

      printf("\nDLM dump: head %p first %p last %p current %p size %d\n",
             (void *) head, (void *) head->first,
             (void *) head->last, (void *) head->current,
             (int) head->data_size);
      
      node = head->first;
      while ( NULL != node) {
        printf("node %p list %p prev %p next %p\n",
               (void *) node, (void *) node->list,
               (void *) node->prev, (void *) node->next);
        node = node->next;
      }
   }

#ifdef TEST
typedef struct rec_ {
         int   flag;
         char  dumm[100];
         } rec;

int Search( rec *r1, int *flag)
{
   printf("%d ", r1->flag);
   return (r1->flag == *flag);
}
int LessTh( rec *r1, rec *r2)
{
   return (r1->flag <= r2->flag);
}
int main (int argc, char *argv[])
   {
      DLMlist  *list;
      rec r, *pr;
      int   i, flag;

      list = DLMcreateList( sizeof(rec), 0x1234);
      ERRcontrol("ON LOG ON STD");

      for (i=0; i<5;i++)
         {
         r.flag = i*5;
         DLMappend( list, &r);
         }
      DLMgetForward( list);
      while (NULL != (pr = DLMgetNext( list)))
         printf("%d ", pr->flag);
      printf("\n");
      DLMgetBackward( list);
      while (NULL != (pr = DLMgetPrev( list)))
         printf("%d ", pr->flag);
      printf("\n");
      flag = 15;
      pr = DLMsearchC( list, (DLMequal_fn) Search, &flag);
      printf("found %d: remove it\n", pr->flag);
      DLMremove( list, pr);
      pr = DLMgetCurrent( list);
      printf("current = %d\n", pr->flag);
      DLMgetForward( list);
      while (NULL != (pr = DLMgetNext( list)))
         printf("%d ", pr->flag);
      printf("\n");

      for (i=0; i<15;i++)
         {
         r.flag = rand();
         DLMinsert( list, &r, (DLMlessthan_fn) LessTh);
         }
      DLMgetForward( list);
      while (NULL != (pr = DLMgetNext( list)))
         printf("%d ", pr->flag);
      printf("\n");

      DLMpop(list);
      DLMgetForward( list);
      while (NULL != (pr = DLMgetNext( list)))
         printf("%d ", pr->flag);
      printf(" popoped\n");
      flag = 10;
      pr = DLMsearchC( list, (DLMequal_fn) Search, &flag);
      printf("found %d: promote it\n", pr->flag);
      DLMpromote( list, pr);
      DLMgetForward( list);
      while (NULL != (pr = DLMgetNext( list)))
         printf("%d ", pr->flag);
      printf(" \n");

      DLMdestroyList( list);
      return 0;
   }

#endif
