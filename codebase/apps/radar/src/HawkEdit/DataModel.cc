#include "DataModel.hh"  


using namespace std;


DataModel *DataModel::_instance = NULL;  


DataModel *DataModel::Instance() {
  	if (_instance == NULL) {
  		_instance = new DataModel();
  	}
  	return _instance;
}

DataModel::~DataModel() {}

void DataModel::setData(RadxVol *vol) {

}

void DataModel::readData(string path) {

}

void DataModel::writeData(string path) {

}

void DataModel::update() {

}

void DataModel::get() {

}

  
DataModel::DataModel() {
	init();
}

void DataModel::init() {

}



