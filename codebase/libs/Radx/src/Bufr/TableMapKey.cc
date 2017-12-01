#include <map>
#include <iostream>
#include <cassert>
#include <fstream>
#include <vector>
#include <iterator>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <Radx/TableMapKey.hh>

using namespace std;

TableMapKey::TableMapKey() {
  _debug = false;
}

TableMapKey::TableMapKey(unsigned short key) {
  _key = key;
}

TableMapKey::~TableMapKey() {
 
}

unsigned short TableMapKey::EncodeKey(string fs, string xs, string ys) {

         unsigned char f,x,y;
 
         f = atoi(fs.c_str());  // try tokens[0].stoui
         x = atoi(xs.c_str());
         y = atoi(ys.c_str());

         unsigned short key;
         //f = 1; x = 8;
         key = f << 6;
         key = key | x;
         key = key << 8;
         key = key | y;
         if (_debug) printf("key = %d (x%x) for f;x;y %d;%d;%d \n", key, key, f,x,y);
   return key;
 }

unsigned short TableMapKey::EncodeKey(unsigned char f, unsigned char x, unsigned char y) {

        unsigned short key;
        key = f << 6;
        key = key | x;
        key = key << 8;
        key = key | y;
        if (_debug) printf("key = %d (x%x) for f;x;y %d;%d;%d \n", key, key, f,x,y);
  return key;
}

void TableMapKey::Decode(unsigned short key, unsigned char *f, unsigned char *x,
  unsigned char *y) {
 
  *f = (unsigned char) (key >> 14);
  *x = (unsigned char) ((key & 0x3f00) >> 8);
  *y = (unsigned char) key & 0x00ff;

  if (_debug) printf("key = %d (x%x) for f;x;y %d;%d;%d \n", key, key, *f,*x,*y);
  //return key;
}

 bool TableMapKey::isTableBEntry() {
  if (_key < 0x4000) { // 00 XX
    return true;
  } else { 
      return false;
  }
}

 bool TableMapKey::isReplicator() {
  if ((_key >= 0x4000) && (_key < 0x8000)) { // 01 XX
    return true;
  } else { 
      return false;
  }
}

 bool TableMapKey::isTableCEntry() {
  if ((_key >= 0x8000) && (_key < 0xC000)) { // 10 XX
    return true;
  } else { 
      return false;
  }
}

 bool TableMapKey::isAnotherNode() {
  if (_key >= 0xC000) { // 11 XX
    return true;
  } else { 
      return false;
  }
}
