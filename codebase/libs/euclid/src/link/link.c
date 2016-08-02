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
/*
 * NAME
 * 	link
 *
 * PURPOSE
 * 	Implement an elementary linked list.  Allocation and freeing
 * must occur outside the functions.
 *
 * NOTES
 * 	
 *
 * HISTORY
 *     wiener - Jun 16, 1994: Created.
 */
#include <stdio.h>
#include <euclid/link.h>
#include <euclid/alloc.h>

void EG_init_link_head(struct link *head)
{
  head->next = NULL;
}

struct link * EG_new_link()
{
  return((struct link *)EG_malloc(sizeof(struct link)));
}

void EG_delete_link(struct link * elt)
{
  EG_free((void *)elt);
}

/* add a new link to a linked list headed by head */
void EG_add_link(struct link *head, struct link *new)
{
  struct link *save;

  save = head->next;
  head->next = new;
  new->next = save;
}

/*
 * unlink a link from a linked list headed by head having been given its
 * previous link
 */
struct link * EG_unlink_link(struct link *previous_link)
{
  struct link *save;

  save = previous_link->next;
  previous_link->next = save->next;
  return(save);
}

/*
 * remove a link from a linked list headed by head having been given its
 * previous link
 */
void EG_remove_link(struct link *previous_link)
{
  struct link *save;

  save = previous_link->next;
  previous_link->next = save->next;
  EG_delete_link(save);
}

/*
 * purge all links from a linked list headed by head having been given its
 * previous link
 */
void EG_purge_link(struct link *previous_link)
{
  struct link *save;

  while ((save = previous_link->next) != NULL)
    {
      previous_link->next = save->next;
      EG_delete_link(save);
    }
}

/*
 * print elements in the list
 */
void EG_print_link(struct link *head)
{
  struct link *next;

  for (next = head->next; next != NULL; next = next->next)
    printf("data value is %d\n", next->data);
}

