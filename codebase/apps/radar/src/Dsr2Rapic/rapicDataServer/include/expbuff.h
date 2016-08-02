#ifndef	__EXPBUFF_H
#define __EXPBUFF_H

#ifdef sgi
#include <sys/bsd_types.h>
#endif

#include <string.h>
#include "spinlock.h"

#ifdef USE_DEFRAG_FN_ORIDE
#include "defrag.h"
#endif

#include <vector>

/*
  exp_buff is a class which provides expanding memory storage facilities
  using a linked list of set length nodes.
  This provides a simple, space efficient storage with little overhead.
  It also suits ths storage of large data sets in environments where
  the size of individual elements is limited (e.g. DOS)
  Any size and type of data may be added to an exp_buff.
  exp_buff maintains a linked list of exp_buff_node's for storing	data.
  Separate exp_buff_rd objects are used for retrieval of blocks or 
  strings (read to terminating char). 
  This allows multiple random access readers to be set up on the one
  exp_buff.
  To read data the read position should first be set via set_read_ofs
  As the data is not stored in a contiguous fashion, it is best to
  use the exp_buff_rd calls to access data.
  make_cont may be used to make the buffer a minimally sized 
  contiguous buffer, but it is not required to acces data if the
  read calls provided are used.
  The _last node should be the only partially filled node

  NOTE*** 	AFTER A CALL TO MAKE_CONT, ALL EXP_BUFF_RD'S WILL
  HAVE INCORRECT NODE POINTERS AND OFFSET VALUES.
  IF THE exp_buff WERE AWARE OF ALL ATTACHED exp_buff_rd
  IT WOULD BE A SIMPLE MATTER TO HAVE THIS DONE 
  AUTOMATICALLY (I JUST HAVEN'T GOT AROUND TO IT YET)

*/

const	int	MINBUFFSIZE = 256;
extern int TotalExpBuffNodeAlloc;
extern bool useExpBuffVector;


class exp_buff;	// fwd declaration

class	exp_buff_node {
  friend	class	exp_buff;
  friend 	class expbuff_freelist;
  friend	class	exp_buff_rd;

  int		buff_size;		// size of this node's data_buff
  int		data_size;	// data bytes used in this node
  exp_buff_node*	next;	// next buffer
  char*	data_buff;		// data location

  int	node_full() {
    return(buff_size==data_size);
  };
  exp_buff_node(int buffsize = 0);
  ~exp_buff_node();
};

typedef	exp_buff_node*	exp_buff_node_ptr;

class expbuff_freelist {
  friend class    exp_buff;
  exp_buff_node	  *first_free;
  spinlock *lock;
  int		  freecount;
  long long       freeSize;
 public:
  expbuff_freelist();
  ~expbuff_freelist();
  void	          AddNode(exp_buff_node *node);
  exp_buff_node   *GetNode(int sz = 0); // if sz = 0, get any size
  void	          ReleaseNodes();
  long long       getSize() { return freeSize; };  // return total bytes count
  int             getCount() { return freecount; };
};


class	exp_buff {
  //    friend class exp_buff_rd;	// give exp_buff_rd access to all
  int		wr_node_count, rd_node_count;	// number of buffer nodes
  int		wr_total_size, rd_total_size;	// total data size
  int		std_buff_size;	// size of nodes to create
  exp_buff_node_ptr 	_first;
  exp_buff_node_ptr 	_this;
  exp_buff_node_ptr 	_next;
  exp_buff_node_ptr 	_last; 	//node pointers
  int		add_node();	// append new node to buffer
  expbuff_freelist *freelist;
  spinlock *lock;
  bool    useLock;
 public:
  int		debug;
  exp_buff(int buffsize = 1024,expbuff_freelist *FreeList = 0);
  std::vector<exp_buff_node_ptr> nodePtrVector;
  // if FreeList undefine, try to use global FreeListMng->ExpBuffFreeList
  // else delete nodes on destruction
  ~exp_buff();
  int		append_data(char*, int);	// point to data, pass size
  // return bytes written
  int		append_exp_buff(exp_buff*);	// append an exp_buff to this 
  int		get_size(); 	// retn total data size
  int		get_nodecount(); // retn total node count
  int           get_node_size() { return std_buff_size;};
  exp_buff_node_ptr 	FirstNode() { return _first; };
  //		int		make_cont();				// make min. contig buffer
  // int		make_min();					// make min. size buffer
  inline void Lock();
  inline void	Unlock();
  void 		DelLock();
  void 	ReleaseOSLock();
};

/*
  exp_buff_rd provides read access to the exp_buff data
  set_read_ofs should be called to set offset into total buffer
  Then read_blk or read_term should be called to return data from the exp_buff
*/

class	exp_buff_rd {
  exp_buff	*buffer;
  exp_buff_node_ptr	read_node; // current read node
  int		rd_ofs_nd;			// read offset in read_node
 public:
  int   radl_pos;       // added by RJ for M/T safe get radial
  int		rd_ofs_total;		// absolute read offset
  // set read offset to given offset
  // return -1 for no more data
  int		set_read_ofs(int new_ofs);	// set read_node, read_ofs
  int	    get_read_ofs() { return(rd_ofs_total); }
  // return a pointer to the next char, or 0 if none
  char* peek_nextchar();	// doesn't advance pos in buffer
  char* read_nextchar();	// does advance pos in buffer
  // read until term char: returns number of bytes read 
  // return -1 for no more data
  int		read_term(char* dest_buff, int max_size,char term_ch=0, 
			  int retain_last_node = 0);
  // read until term chars: returns number of bytes read 
  // return -1 for no more data
  int	read_term2(char* dest_buff, int max_size,char term_ch=0, char term_ch2=0,  
		   int ignore_1st_chars = 0,	// char to start looking for term chars
		   int retain_last_node = 0);
  // read block from buff returns number no. of bytes read 
  // return -1 for no more data
  int		read_blk(char* dest_buff, int blk_size, int retain_last_node = 0);	

  void	skip_white();	// skips white space characters

  void 	open(exp_buff *parent_buff) {
    buffer = parent_buff;	// pointer to exp_buff
    set_read_ofs(0);
  }
  int ReadNodeValid() {
    return (read_node != NULL);
  }
  exp_buff_rd(exp_buff *parent_buff = NULL) { 
#ifdef USE_DEFRAG_FN_ORIDE
    if(!DeFragList.IDString[0])
      strcpy(DeFragList.IDString, "exp_buff_rd");
#endif
    buffer = NULL;
    read_node = NULL; // current read node
    rd_ofs_nd = 0;			// read offset in read_node
    radl_pos = 0;       // added by RJ for M/T safe get radial
    rd_ofs_total = 0;		// absolute read offset
    if (parent_buff)	// if buffer passed, open it
      open(parent_buff);	
  };
#ifdef USE_DEFRAG_FN_ORIDE
#define DEFRAGDECLARATION
#include "defrag.h"
#endif
};

#endif	/* __EXPBUFF_H */
