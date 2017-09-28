#include <map>
#include <iostream>
#include <cassert>
#include <fstream>
#include <vector>
#include <iterator>
#include <sstream>
#include <stdio.h>
#include <Radx/TableMapElement.hh>
#include <Radx/Radx.hh>


class TableMap {

public: 

  TableMap();
  ~TableMap();
  int ImportTables();
  TableMapElement Retrieve(unsigned short key);

private:
  std::map<unsigned short, TableMapElement> table;  // TODO: should be unordered_map

  vector<string> split(const std::string &s, char delim);
  int ReadTableB(string fileName);
  int ReadTableD(string fileName);
  int ImportTablesOld();

  bool _debug = false;

};
