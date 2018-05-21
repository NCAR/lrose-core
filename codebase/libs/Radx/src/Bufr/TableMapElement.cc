#include <Radx/TableMapElement.hh>
#include <map>
#include <iostream>
#include <cassert>
#include <fstream>
#include <vector>
#include <iterator>
#include <sstream>
#include <stdio.h>

using namespace std;

/// Constructor
TableMapElement::TableMapElement() {
}

TableMapElement::TableMapElement(vector<unsigned short> keys) {
  _whichType = KEYS;
  _listOfKeys = keys;
}

TableMapElement::TableMapElement(string fieldName, int scale, string units, int referenceValue,
  int dataWidthBits) {
  _whichType = DESCRIPTOR;
  _descriptor.fieldName = fieldName;
  _descriptor.scale = scale;
  _descriptor.units = units;
  _descriptor.referenceValue = referenceValue;
  _descriptor.dataWidthBits = dataWidthBits;
}
  

bool TableMapElement::IsText() {
  return (_descriptor.units.find("CCITT") != string::npos);
}  
  /// Destructor
  
TableMapElement::~TableMapElement() {

}
