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
#ifndef KEYED_LIST_H
#define KEYED_LIST_H

#include <toolsa/mem.h>

template <class Key, class T>
class KeyedList {
 public:

   // Constructor

   KeyedList()
   {
     _size = 0;   // Initialize the array
     _first = NULL;
     _last = NULL;
     _current = NULL;
   }
  
   // Destructor

   ~KeyedList()
   {
     _data_node_t *next_list;
     
     while (_first != NULL)
     {
       next_list = _first->next;
       ufree((char *)_first->data);
       ufree((char *)_first);
       _first = next_list;
     }
   }
  
   // Add a node to the list.  If a node with this key already exists
   // in the list, it is overwritten with the new information.

   void add(Key key, T t)
   {
     // See if a node with this key already exists.  If it does, update
     // the data for that node.

     if (find(key) != NULL)
     {
       update(key, t);
       return;
     }
     
     _data_node_t *new_node = (_data_node_t *)umalloc(sizeof(_data_node_t));
     T *new_data = (T *)umalloc(sizeof(T));
     
     *new_data = t;
     
     // Always add at the back of the list.

     new_node->key = key;
     new_node->data = new_data;
     new_node->next = NULL;
     new_node->prev = _last;
     
     if (_first == NULL)
       _first = new_node;

     if (_last != NULL)
       _last->next = new_node;
     
     _last = new_node;
     
     _size++;
     
     return;
   }

   // Find the node with the given key.  Return a pointer to the data
   // for that key.  This returns a pointer to the data within the
   // list so the pointer should not be freed by the calling routine.
   // Note that because this returns a pointer to the actual data, the
   // calling routine can update the data for the node directly after
   // making this call.

   T *find(Key key)
   {
     _data_node_t *node;
     
     node = _findNode(key);
     
     if (node == NULL)
       return(NULL);
     
     return(node->data);
   }
   
   // Update the data for the given node.

   void update(Key key, T t)
   {
     T *node = find(key);
     
     if (node == NULL)
       add(key, t);
     else
     {
       remove(key);
       add(key, t);
     }
     
     return;
   }
   
   // Remove the given node from the list.  Returns a pointer to the
   // data for the node.  This pointer must be freed by the calling
   // routine.

   void remove(Key key)
   {
     _data_node_t *node;
     
     node = _findNode(key);
     
     if (node == NULL)
       return;
     
     _current = node;
     
     remove();
     
     return;
   }

   // Remove the "current" node in the list.  If there is no
   // current node, do nothing.

   void remove(void)
   {
     if (_current == (_data_node_t *)NULL)
       return;
     
     _data_node_t *node = _current;
     
     // Update the node pointers if they point to the node
     // we are removing

     if (_first == node)
       _first = node->next;
     
     if (_last == node)
       _last = node->prev;
     
     _current = _current->next;
     
     // Remove the node from the linked list

     if (node->next != NULL)
       node->next->prev = node->prev;

     if (node->prev != NULL)
       node->prev->next = node->next;
     
     // Free the node

     ufree(node->data);
     ufree(node);
     
     return;
   }
   
   // Returns a pointer to the data in the first node in the list.
   // Returns NULL if the list is empty.  This routine returns a
   // pointer to the actual data in the list so the pointer should
   // not be freed and the data can be updated directly, if desired.
   // The key value is returned in the argument list just in case
   // this value is not a part of the node and the calling routine
   // needs it.  The space for the key must be allocated by the
   // calling routine -- the internal key value may not be changed
   // by the calling routine.

   T *first(Key *key)
   {
     // Set the current pointer

     _current = _first;

     // Check for an empty list

     if (_first == NULL)
       return(NULL);
     
     // Return the data pointer.

     *key = _current->key;
     
     return(_current->data);
   }
   
   // Returns a pointer to the data in the next node in the list.
   // Returns NULL if the end of the list is reached.  If this
   // routine is called after the end of the list or before first()
   // is called, it returns the first node in the list.
   // Note that this routine returns a pointer to the actual data 
   // in the list so the pointer should not be freed and the data
   // can be updated directly, if desired.  The key value is returned
   // in the argument list just in case this value is not a part of
   // the node and the calling routine needs it.  The space for the
   // key must be allocated by the calling routine -- the internal key
   // value may not be changed by the calling routine.

   T *next(Key *key)
   {
     // Check for a NULL current pointer

     if (_current == NULL)
       return(first(key));
     
     // Update the current node pointer

     _current = _current->next;
     
     // Return the data in the current node

     if (_current == NULL)
       return(NULL);
     else
     {
       *key = _current->key;
       return(_current->data);
     }
   }
   
   // Returns the number of nodes in the list.

   int size() { return _size; };
  
 private:

   typedef struct _data_node_s
   {
     Key key;
     T* data;
     struct _data_node_s *next;
     struct _data_node_s *prev;
   } _data_node_t;
   
   _data_node_t *_first;
   _data_node_t *_last;
   _data_node_t *_current;
   
   int _size;

   _data_node_t *_findNode(Key key)
   {
     _data_node_t *node = _first;
     
     while (node != NULL &&
	    memcmp(&node->key, &key, sizeof(Key)) != 0)
       node = node->next;
     
     return(node);
   }
   
};

#endif
