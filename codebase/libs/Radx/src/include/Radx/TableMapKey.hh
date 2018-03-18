#include <vector>
#include <sstream>
#include <stdio.h>

using namespace std;

class TableMapKey {

public:

  TableMapKey();
  TableMapKey(unsigned short key);
  ~TableMapKey();

  unsigned short EncodeKey(string fs, string xs, string ys);

  unsigned short EncodeKey(unsigned char f, unsigned char x, unsigned char y);

  void Decode(unsigned short key, unsigned char *f, unsigned char *x, unsigned char *y);

  bool isTableBEntry();
 
  bool isReplicator();

  bool isTableCEntry();

  bool isAnotherNode();

private:
  // unsigned char _f;
  // unsigned char _x;
  // unsigned char _y;
  unsigned short _key;

  bool _debug;

};

