/* 
   exp_buff.c
   Implementation of the exp_buff class type
*/

#include "expbuff.h"
#include "rdr.h"
#include "freelist.h"
#include <time.h>
#include "log.h"

#ifdef USE_DEFRAG_FN_ORIDE
#define CLASSTYPE exp_buff_rd
#define DEFRAGSOURCE
#include "defrag.h"
#endif

bool useExpBuffVector = true;  // if true use an exp_buff_node_ptr vector
                               // to set read offset, 

int TotalExpBuffNodeAlloc = 0;

exp_buff_node::exp_buff_node(int buffsize)
{ 
  buff_size = buffsize;
  data_size = 0;
  next = NULL;
  if (buffsize)
    {
      data_buff = new char[buffsize];
      TotalExpBuffNodeAlloc += buff_size;
    }
  else
    data_buff = NULL;
}

exp_buff_node::~exp_buff_node() 
{
  if (data_buff) {
    delete[] data_buff;
    data_buff = 0;
    TotalExpBuffNodeAlloc -= buff_size;
  }
}

exp_buff::exp_buff(int buffsize, expbuff_freelist *FreeList) {   // constructor
  if (buffsize < MINBUFFSIZE)
    buffsize = MINBUFFSIZE;
  std_buff_size = buffsize;
  wr_node_count = rd_node_count = 0;
  wr_total_size = rd_total_size = 0;
  _first = _this = _next = _last=0;
  freelist = 0;
  if (FreeList) 
    freelist = FreeList;
  else if (FreeListMng) 
    freelist = FreeListMng->ExpBuffFreeList;
  lock = new spinlock("exp_buff->lock", 200); // 2 secs
  useLock = (lock != NULL);
  debug = FALSE;
}

exp_buff::~exp_buff() { // destructor
  char logstr[128];
  _this = _first;
  for (int x = 1; x <= wr_node_count; x++) {
    if (!_this) {
      sprintf(logstr,"exp_buff::~exp_buff - Error deleting, no more nodes"
	      " x(%d) < wr_node_count(%d)\n",x,wr_node_count);
      RapicLog(logstr, LOG_ERR);
      break;
    }
    else {
      _next = _this->next;    // keep link to next
      if (freelist)						// keep node in free list for re-use
	freelist->AddNode(_this);
      else {
	delete _this;       // de-allocate node
      }
      _this = _next;        // link to next
    };
  }
  DelLock();
}

int exp_buff::add_node() {
  exp_buff_node_ptr _temp;
  _temp = _last;
  _last = 0;
  if (freelist)				// try to get used node from free list
    _last = freelist->GetNode(std_buff_size);
  if (!_last) {	// no freelist, or no node avail in freelist
    _last = new exp_buff_node(std_buff_size);
  }
  if (_last == 0) {
    return(-1);	// error creating buffer
  }
  if (_first == 0) {			// this is first node
    _first = _last;	
  }
  wr_node_count++;			// initialise variables
  _last->next = 0;
  if (_temp != 0) 
    _temp->next = _last;	// link in new node
  nodePtrVector.push_back(_last);
  if (debug) {
    printf("ADDING NODE - COUNT = %d\n",wr_node_count);
  }
  return(0);
}

int exp_buff::append_data(char* in_data, int data_sz) { 
  int		bytes_copied = 0;
  int		buff_space;
  int		bytestocopy;
  if (_last == 0) 
    if (add_node() < 0) 
      return(bytes_copied);
  while (bytes_copied < data_sz) {
    buff_space = _last->buff_size - _last->data_size;
    if (buff_space == 0) {
      if (add_node() < 0)
	return(bytes_copied);		
      buff_space = _last->buff_size - _last->data_size;
    }
    bytestocopy = data_sz-bytes_copied;	// remaining data size
    if (buff_space < bytestocopy) 
      bytestocopy = buff_space;
    char  *input = (char*)in_data;
    memcpy(&_last->data_buff[_last->data_size],
	   &input[bytes_copied],
	   bytestocopy);
    _last->data_size += bytestocopy;	// update node size record
    bytes_copied += bytestocopy;		// update local record
    wr_total_size += bytestocopy;		// update total record
  }
  Lock();
  rd_node_count = wr_node_count;
  rd_total_size = wr_total_size;
  Unlock();
  return(bytes_copied);
}


/*
  append another exp_buff to this one,
  NOT YET IMPLEMENTED
*/
int   exp_buff::append_exp_buff(exp_buff* src_buff) {
  exp_buff_rd	*src_buff_rd = 0;
	
  if (src_buff || src_buff_rd);
  return(0);
}	



/*
int exp_buff::make_min() {				// truncates last node to 
  exp_buff_node	*temp;				// data_size
  if (_last == 0) 
    return(0);						// nothing to do
  temp = new exp_buff_node;			// new mode			
  if (temp ==0) return(-1);
  Lock();
  temp->data_buff = new char[_last->data_size]; // right size buffer
  TotalExpBuffNodeAlloc += _last->data_size;
  memcpy(temp->data_buff,
	 _last->data_buff,				// copy data to new node
	 _last->data_size);
  temp->data_size = _last->data_size;	// copy node details
  temp->buff_size = _last->data_size;	
  temp->next = 0;
  if (freelist)
    freelist->AddNode(_last);
  else {
    delete _last;						// remove old node
  }
  _last = temp;						// assign truncated to _last
  Unlock();
  return(0);
}
*/

int exp_buff::get_size() {
  int sz;
  Lock();
  sz = rd_total_size;
  Unlock();
  return sz;
}

int exp_buff::get_nodecount() {
  int nodes;
  Lock();
  nodes = rd_node_count;
  Unlock();
  return nodes;
}

void exp_buff::Lock() {
  if (lock && useLock) 
    lock->get_lock();
}

void exp_buff::Unlock() {
  if (lock && useLock) 
    lock->rel_lock();
}

void exp_buff::DelLock() {
	
  if (lock)
    {
      delete lock;
      lock = 0;
    }
}

void exp_buff::ReleaseOSLock() {
	
  if (!lock)
    return;
  lock->ReleaseOSLock();
  useLock = false;
}


// position read_node and read_ofs at absolute offset
int exp_buff_rd::set_read_ofs(int new_ofs) {
  int		total_ofs = 0;
  int		node_pos = 0;
  int		node_count;
  exp_buff_node	*local_node;
    
  read_node = 0;
  rd_ofs_nd = 0;
  rd_ofs_total = 0;
  if (!buffer) return -1;
  if ((buffer->get_nodecount() == 0)||(new_ofs > buffer->get_size())) {
    return(-1);
  }
  if (useExpBuffVector)
    {
      int nodeIndex = new_ofs/buffer->get_node_size();
      if (nodeIndex >= int(buffer->nodePtrVector.size()))
	return -1;
      total_ofs = nodeIndex * buffer->get_node_size();
      local_node = buffer->nodePtrVector[nodeIndex];
    }
  else
    {
      local_node = buffer->FirstNode();
      node_pos++;
      node_count = buffer->get_nodecount();
      while (((total_ofs + local_node->data_size) <= new_ofs)
	     &&(node_pos < node_count)) {
	total_ofs += local_node->data_size;	//total_ofs at start of node
	local_node = local_node->next;
	if (!local_node) {
	  RapicLog("exp_buff_rd::set_read_ofs ERROR - bad linked list\n", LOG_ERR);
	  return -1;
	}
      }
    }
  if ((total_ofs + local_node->data_size) > new_ofs) {
    rd_ofs_nd = new_ofs - total_ofs;	// offset into read_node
    rd_ofs_total = new_ofs;		// absolute byte offset
    read_node = local_node;
    return(0);
  }
  else {
    return(-1);						// EOBuff encountered
  }
}


// read_blk returns the number of bytes read, this is less than 
// blk_size if EOBuff is encountered 
int exp_buff_rd::read_blk(char* dest_buff, int blk_size, int retain_last_node) {
  int		bytes_copied = 0;	// sum of bytes copied
  int		bytes_to_copy;		// bytes to copy from this node
  int		bytes_avail;		// bytes avail in this node
  int		buffersize;
  int		done;				// TRUE when done
  exp_buff_node_ptr temp_node;
	
  if (!buffer) return -1;
  if (read_node == 0) 
    return(-1);				// set_read_ofs failed or EOBuff		

  // Clip blk_size to remaining buffer data size
  buffersize = buffer->get_size();
  if ((rd_ofs_total + blk_size) > buffersize)
    blk_size = buffersize - rd_ofs_total;
        
  // calc bytes left in read_node
  bytes_avail = read_node->data_size - rd_ofs_nd;
  if (bytes_avail == 0) {		// no data left in current read_node
    temp_node = read_node;
    read_node = read_node->next;	// try next
    if (read_node != 0) {		// if not EOBuff set bytes avail 
      bytes_avail = read_node->data_size;
      rd_ofs_nd = 0;		// clear offset for new node
    }
    else {
      if (retain_last_node) read_node = temp_node;
      else rd_ofs_nd = 0;
      return(-1);		// return -1 for no data to read
    }
  }
  done = ((bytes_avail == 0) || (blk_size == 0));
  // if no bytes avail, we're finished
  while (!done) {
    if ((bytes_copied + bytes_avail) >= blk_size) {	// this block to satisfy read
      bytes_to_copy = blk_size-bytes_copied;	// set # to copy
      done = TRUE;				// finished
    }
    else bytes_to_copy = bytes_avail;	// copy rest of this node
    memcpy(&dest_buff[bytes_copied],
	   &read_node->data_buff[rd_ofs_nd],
	   bytes_to_copy);
    rd_ofs_nd += bytes_to_copy;		// reposition rd_ofs_nd
    rd_ofs_total += bytes_to_copy;	// update rd_ofs_total
    bytes_copied += bytes_to_copy;
    if (!done) {					// step to next node
      temp_node = read_node;
      read_node = read_node->next;	
      if (read_node == 0) {
	done = TRUE;			// EOBuff encountered
	if (retain_last_node) read_node = temp_node;
      }
      else {
	bytes_avail = read_node->data_size;
	rd_ofs_nd = 0;
      }
    }
  }
  return(bytes_copied);
}

// return the next character without affecting the current read state
char* exp_buff_rd::peek_nextchar()
{
  int		node_bytes_avail;		// bytes avail in this node
  int		buffersize;
  exp_buff_node_ptr temp_node = read_node;
  int temp_ofs_nd = rd_ofs_nd;
  char *nextchar;
    
  if (!temp_node || !buffer)
    return 0;
	
  // ensure don't peek past end of buffer
  buffersize = buffer->get_size();
  if ((rd_ofs_total + 1) > buffersize)
    return 0;
        
  node_bytes_avail = temp_node->data_size - temp_ofs_nd;
  if (node_bytes_avail == 0) {	// no data left in current read_node
    temp_node = temp_node->next;	// try next
    temp_ofs_nd = 0;
    if (temp_node != 0) {		// if not EOBuff set bytes avail 
      node_bytes_avail = temp_node->data_size;
    }
  }
  if (node_bytes_avail > 0)
    {
      nextchar = &temp_node->data_buff[temp_ofs_nd];
    }
  else
    {
      nextchar = 0;
    }
  return nextchar;		
}

// read_term returns the number of bytes read before a term char 
// was encountered or max chars were read or EOBuff was encountered.
int exp_buff_rd::read_term(char* dest_buff, int max_size, char term_ch, int retain_last_node) {
  int		bytes_copied = 0;	// sum of bytes copied
  int		bytes_to_copy;		// bytes to copy from this node
  int		node_bytes_avail;		// bytes avail in this node
  int		buffer_bytes_avail;		// bytes avail in this node
  int		done,term_ch_read;	// flags 
  char		*src_char, *dest_char;	
	
  exp_buff_node_ptr temp_node;
  if (!buffer) return -1;
  if (read_node == 0) 
    return(-1);				// set_read_ofs failed or EOBuff
  
  // Clip blk_size to remaining buffer data size
  buffer_bytes_avail = buffer->get_size() - rd_ofs_total;

  // if no bytes to read, return -1
  if (buffer_bytes_avail == 0)
    return -1;

  if (buffer_bytes_avail < max_size)
    max_size = buffer_bytes_avail;	// clip o/p size to buffer len

  // calc bytes left in read_node
  node_bytes_avail = read_node->data_size - rd_ofs_nd;
  if (node_bytes_avail == 0) {		// no data left in current read_node
    temp_node = read_node;
    read_node = read_node->next;	// try next
    if (read_node != 0) {		// if not EOBuff set bytes avail 
      node_bytes_avail = read_node->data_size;
      rd_ofs_nd = 0;				// clear offset for new node
    }
    else {
      if (retain_last_node) read_node = temp_node;
      else rd_ofs_nd = 0;
      return(-1);		// no data, ie EOBuff
    }
  }
  term_ch_read = FALSE;
  done = ((buffer_bytes_avail == 0) || (max_size == 0));
  // if no bytes avail, we're finished
  while (!done) {
    if ((bytes_copied + node_bytes_avail) >= max_size) {	
      bytes_to_copy = max_size-bytes_copied;	// set # to copy
      done = TRUE;							// finished
    }
    else bytes_to_copy = node_bytes_avail;	// copy rest of this node
    dest_char = &dest_buff[bytes_copied];
    src_char = &read_node->data_buff[rd_ofs_nd];
    while ((bytes_to_copy > 0) && !term_ch_read) {
      *dest_char = *src_char;
      term_ch_read = (*src_char == term_ch);
      bytes_copied++;
      bytes_to_copy--;
      dest_char++;
      src_char++;
      rd_ofs_nd++;
      rd_ofs_total++;
    }
    if (term_ch_read) done = TRUE;
    if (!done) {					// step to next node
      temp_node = read_node;
      read_node = read_node->next;	
      if (read_node == 0) {
	if (retain_last_node) read_node = temp_node;
	done = TRUE;			// EOBuff encountered
      }
      else {
	node_bytes_avail = read_node->data_size;
	rd_ofs_nd = 0;
      }
    }
  }
  return(bytes_copied);
}

// read_term returns the number of bytes read before the 2 term chars
// was encountered or max chars were read or EOBuff was encountered.
int exp_buff_rd::read_term2(char* dest_buff, int max_size, char term_ch, char term_ch2, 
			    int ignore_1st_chars,	// char to start looking for term chars
			    int retain_last_node) {
  int		bytes_copied = 0;	// sum of bytes copied
  int		bytes_to_copy;		// bytes to copy from this node
  int		node_bytes_avail;		// bytes avail in this node
  int		buffer_bytes_avail;		// bytes avail in this node
  bool	done,
    term_ch1_read = false, 
    term_ch2_read = false;	// flags 
  char	c,*src_char, *dest_char;	
        
  exp_buff_node_ptr temp_node;
  if (!buffer) return -1;
  if (read_node == 0) 
    return(-1);				// set_read_ofs failed or EOBuff		

  // Clip blk_size to remaining buffer data size
  buffer_bytes_avail = buffer->get_size() - rd_ofs_total;

  // if no bytes to read, return -1
  if (buffer_bytes_avail == 0)
    return -1;

  if (buffer_bytes_avail < max_size)
    max_size = buffer_bytes_avail;	// clip o/p size to buffer len

  // calc bytes left in read_node
  node_bytes_avail = read_node->data_size - rd_ofs_nd;
  if (node_bytes_avail == 0) {		// no data left in current read_node
    temp_node = read_node;
    read_node = read_node->next;	// try next
    if (read_node != 0) {		// if not EOBuff set bytes avail 
      node_bytes_avail = read_node->data_size;
      rd_ofs_nd = 0;				// clear offset for new node
    }
    else {
      if (retain_last_node) read_node = temp_node;
      else rd_ofs_nd = 0;
      return(-1);		// no data, ie EOBuff
    }
  }
  done = ((buffer_bytes_avail == 0) || (max_size == 0));
  // if no bytes avail, we're finished
  while (!done) {
    if ((bytes_copied + node_bytes_avail) >= max_size) {	
      bytes_to_copy = max_size-bytes_copied;	// set # to copy
      done = TRUE;							// finished
    }
    else bytes_to_copy = node_bytes_avail;	// copy rest of this node
    dest_char = &dest_buff[bytes_copied];
    src_char = &read_node->data_buff[rd_ofs_nd];
    while ((bytes_to_copy > 0) && !term_ch2_read) {
      c = *dest_char = *src_char; // copy this char to dest
      bytes_copied++;	    // update counters & pntrs
      bytes_to_copy--;
      dest_char++;
      src_char++;
      rd_ofs_nd++;
      rd_ofs_total++;
      // check for 2 char termination
      if (term_ch1_read)	// we've gor first term char, check for 2nd
	{
	  if (!(term_ch2_read = (c == term_ch2)))
	    term_ch1_read = false;	// if not 2nd term, reset first
	}
      if ((!term_ch1_read) && 
	  (bytes_copied > ignore_1st_chars)) // don't start checking until specified no of chars passed
	{
	  term_ch1_read = (c == term_ch);
	}
    }
    if (term_ch2_read) done = TRUE;
    if (!done) {				// step to next node
      temp_node = read_node;
      read_node = read_node->next;	
      if (read_node == 0) {
	if (retain_last_node) read_node = temp_node;
	done = TRUE;			// EOBuff encountered
      }
      else {
	node_bytes_avail = read_node->data_size;
	rd_ofs_nd = 0;
      }
    }
  }
  return(bytes_copied);
}

void exp_buff_rd::skip_white()
{
  int		node_bytes_avail;		// bytes avail in this node
  int		buffer_bytes_avail;		// bytes avail in this node
  bool	done;	// flags 
  char	*src_char;	
        
  if (!buffer) return;
  if (read_node == 0) 
    return;				// set_read_ofs failed or EOBuff		

  // Clip blk_size to remaining buffer data size
  buffer_bytes_avail = buffer->get_size() - rd_ofs_total;

  // calc bytes left in read_node
  node_bytes_avail = read_node->data_size - rd_ofs_nd;
  if (node_bytes_avail == 0) {		// no data left in current read_node
    read_node = read_node->next;	// try next
    if (read_node != 0) {		// if not EOBuff set bytes avail 
      node_bytes_avail = read_node->data_size;
      rd_ofs_nd = 0;				// clear offset for new node
    }
  }
  done = (node_bytes_avail == 0);
  // if no bytes avail, we're finished
  while (!done) {
    src_char = &read_node->data_buff[rd_ofs_nd];
    while ((buffer_bytes_avail > 0) && 
	   (node_bytes_avail > 0) && 
	   (*src_char < 33)) {
      src_char++;
      rd_ofs_nd++;
      rd_ofs_total++;
      buffer_bytes_avail--;
    }
    done = *src_char >= 33;
    if (!done) {				// step to next node
      read_node = read_node->next;	
      if ((buffer_bytes_avail <= 0) ||
	  (read_node == 0)) {
	done = TRUE;			// EOBuff encountered
      }
      else {
	node_bytes_avail = read_node->data_size;
	rd_ofs_nd = 0;
      }
    }
  }
}

expbuff_freelist::expbuff_freelist() {
  first_free = 0;
  freecount = 0;
  freeSize = 0;
  lock = new spinlock("expbuff_freelist->lock", 200); // 2 secs
}

expbuff_freelist::~expbuff_freelist() {
  ReleaseNodes();
  if (lock) delete lock;
}

void expbuff_freelist::AddNode(exp_buff_node *node) {
  char logstr[128];

  int debug = FALSE;
  if (!node) return;
  if (lock) lock->get_lock();
  node->next = first_free;
  first_free = node;
  freecount++;
  freeSize += node->buff_size;
  node->data_size = 0;
  if (lock) lock->rel_lock();
  if (debug) {
    sprintf(logstr,"expbuff_freelist::AddNode - freecount=%d\n",freecount);
    RapicLog(logstr, LOG_INFO);
  }
}

exp_buff_node *expbuff_freelist::GetNode(int sz) {
  char logstr[128];
  exp_buff_node	*temp,*prev;
  int	szmatch;
  int debug = FALSE;
	
  if (!first_free) return 0;	
  if (lock) 
    if (!lock->get_lock())   // if not able to get lock, return NULL
      {
	sprintf(logstr,"expbuff_freelist::GetNode - Failed to get lock - returning\n");
	RapicLog(logstr, LOG_INFO);
	return NULL;
      }
  if (debug) 
    RapicLog("expbuff_freelist::GetNode", LOG_INFO);
  if (!sz) {
    temp = first_free;
    if (first_free) {
      temp->data_size = 0;
      first_free = first_free->next;
      freecount--;
      freeSize -= temp->buff_size;
    }
    if (lock) lock->rel_lock();
    if (debug) {
      //			sprintf(logstr,"(sz=0) returned %x freecount=%d\n",temp,freecount);
      //	RapicLog(logstr, LOG_INFO);
    }
    return temp;
  }
  else {
    prev = 0;
    szmatch = FALSE;
    temp = first_free;
    while (temp && !szmatch)
      {
	szmatch = temp->buff_size == sz;
	if (!szmatch) {
	  prev = temp;
	  temp = temp->next;
	}
      } 
    if (szmatch && temp) {
      if (temp == first_free) 
	first_free = temp->next;		// if matched on 1st use next as 1st
      else 
	{
	  if (prev)
	    prev->next = temp->next;		// else link list past this
	}
      temp->data_size = 0;
      freecount--;
      freeSize -= temp->buff_size;
    }
    if (lock) lock->rel_lock();
    if (debug) {
      //sprintf(logstr,"(sz=%d) returned %x freecount=%d\n",sz,temp,freecount);
      //RapicLog(logstr, LOG_INFO);
    }
    return temp;
  }
}

void expbuff_freelist::ReleaseNodes() {
  exp_buff_node *temp;
  if (lock) lock->get_lock();
  while (first_free) {
    temp = first_free;
    first_free = temp->next;
    /* 
       if (temp->data_buff) {
       delete[] temp->data_buff;
       temp->data_buff = 0;
       }
    */
    freecount--;
    freeSize -= temp->buff_size;
    delete temp;
  }
  if (lock) lock->rel_lock();
}


