//
//
//
//

#include <iostream>
#include <cstdio>
using namespace std;

#include "Fault.h"
using namespace ForayUtility;

#include "KeyAccess.h"


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
KeyAccess::KeyAccess(){

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
KeyAccess::~KeyAccess(){

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::clear(){

    integerValues_.clear();
    longlongValues_.clear();
    doubleValues_.clear();
    stringValues_.clear();
    raycTimeValues_.clear();
    raycAngleValues_.clear();
    shortValues_.clear();
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
short & KeyAccess::get_short(const string key) throw (Fault){

    if(shortValues_.count(key) == 0){
	char msg[2048];
	sprintf(msg,"KeyAccess::get_integer : key %s not defined.\n",
		key.c_str());
	throw Fault(msg);
    }
    
    return shortValues_[key];
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
short & KeyAccess::get_short(const string key, const int index) throw (Fault){
    
    string indexedKey = make_indexed_key(key,index);

    if(shortValues_.count(indexedKey) == 0){
	char msg[2048];
	sprintf(msg,"KeyAccess::get_integer : key %s not defined.\n",
		indexedKey.c_str());
	throw Fault(msg);
    }
    
    return shortValues_[indexedKey];
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::set_short(const string key, const short &value) throw (Fault){

    if(key_is_read_only(key)){
	char msg[2048];
	sprintf(msg,"KeyAccess::set_integer : value with key %s read only.\n",
		key.c_str());
	throw Fault(msg);
    }

    shortValues_[key] = value;

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::set_short(const string key, const int index, const short &value) throw (Fault){

    string indexedKey = make_indexed_key(key,index);

    set_short(indexedKey,value);
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int & KeyAccess::get_integer(const string key) throw (Fault){

    if(integerValues_.count(key) == 0){
	char msg[2048];
	sprintf(msg,"KeyAccess::get_integer : key %s not defined.\n",
		key.c_str());
	throw Fault(msg);
    }
    
    return integerValues_[key];
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int & KeyAccess::get_integer(const string key, const int index) throw (Fault){
    
    string indexedKey = make_indexed_key(key,index);

    if(integerValues_.count(indexedKey) == 0){
	char msg[2048];
	sprintf(msg,"KeyAccess::get_integer : key %s not defined.\n",
		indexedKey.c_str());
	throw Fault(msg);
    }
    
    return integerValues_[indexedKey];
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::set_integer(const string key, const int &value) throw (Fault){

    if(key_is_read_only(key)){
	char msg[2048];
	sprintf(msg,"KeyAccess::set_integer : value with key %s read only.\n",
		key.c_str());
	throw Fault(msg);
    }

    integerValues_[key] = value;

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::set_integer(const string key, const int index, const int &value) throw (Fault){

    string indexedKey = make_indexed_key(key,index);

    set_integer(indexedKey,value);
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
long long & KeyAccess::get_longlong(const string key) throw (Fault){

    if(longlongValues_.count(key) == 0){
	char msg[2048];
	sprintf(msg,"KeyAccess::get_longlong : key %s not defined.\n",
		key.c_str());
	throw Fault(msg);
    }
    
    return longlongValues_[key];
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::set_longlong(const string key, const long long &value) throw (Fault){

    if(key_is_read_only(key)){
	char msg[2048];
	sprintf(msg,"KeyAccess::set_longlong : value with key %s read only.\n",
		key.c_str());
	throw Fault(msg);
    }

    longlongValues_[key] = value;
}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
double & KeyAccess::get_double(const string key) throw (Fault){

    if(doubleValues_.count(key) == 0){
	char msg[2048];
	sprintf(msg,"KeyAccess::get_double : key %s not defined.\n",
		key.c_str());
	throw Fault(msg);
    }
    
    return doubleValues_[key];
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
double & KeyAccess::get_double(const string key, const int index) throw (Fault){

    string indexedKey = make_indexed_key(key,index);

    if(doubleValues_.count(indexedKey) == 0){
	char msg[2048];
	sprintf(msg,"KeyAccess::get_double : key %s not defined.\n",
		indexedKey.c_str());
	throw Fault(msg);
    }
    
    return doubleValues_[indexedKey];
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::set_double(const string key, const double &value) throw (Fault){

    if(key_is_read_only(key)){
	char msg[2048];
	sprintf(msg,"KeyAccess::set_double : value with key %s read only.\n",
		key.c_str());
	throw Fault(msg);
    }

    doubleValues_[key] = value;

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::set_double(const string key, const int index, const double &value) throw (Fault){

    string indexedKey = make_indexed_key(key,index);

    set_double(indexedKey,value);
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string & KeyAccess::get_string(const string key) throw (Fault) {

    if(stringValues_.count(key) == 0){
	char msg[2048];
	sprintf(msg,"KeyAccess::get_string : key %s not defined.\n",
		key.c_str());
	throw Fault(msg);
    }
    
    return stringValues_[key];
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string & KeyAccess::get_string(const string key,const int index) throw (Fault){

    string indexedKey = make_indexed_key(key,index);

    if(stringValues_.count(indexedKey) == 0){
	char msg[2048];
	sprintf(msg,"KeyAccess::get_string key %s not defined.\n",
		indexedKey.c_str());
	throw Fault(msg);
    }
    
    return stringValues_[indexedKey];


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::set_string(const string key, const string &value) throw (Fault){

    if(key_is_read_only(key)){
	char msg[2048];
	sprintf(msg,"KeyAccess::set_string : value with key %s read only.\n",
		key.c_str());
	throw Fault(msg);
    }

    stringValues_[key] = value;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::set_string(const string key, const int index, const string &value) throw (Fault){

    string indexedKey = make_indexed_key(key,index);

    set_string(indexedKey,value);
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool & KeyAccess::get_boolean(const string key) throw (Fault) {

    if(booleanValues_.count(key) == 0){
	char msg[2048];
	sprintf(msg,"KeyAccess::get_boolean : key %s not defined.\n",
		key.c_str());
	throw Fault(msg);
    }
    
    return booleanValues_[key];
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool & KeyAccess::get_boolean(const string key, const int index) throw (Fault) {

    string indexedKey = make_indexed_key(key,index);

    if(booleanValues_.count(key) == 0){
	char msg[2048];
	sprintf(msg,"KeyAccess::get_boolean : key %s not defined.\n",
		key.c_str());
	throw Fault(msg);
    }
    
    return booleanValues_[indexedKey];
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::set_boolean(const string key, const bool &value) throw (Fault){

    if(key_is_read_only(key)){
	char msg[2048];
	sprintf(msg,"KeyAccess::set_bool : value with key %s read only.\n",
		key.c_str());
	throw Fault(msg);
    }

    booleanValues_[key] = value;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::set_boolean(const string key, const int index, const bool &value) throw (Fault){

    string indexedKey = make_indexed_key(key,index);

    if(key_is_read_only(indexedKey)){
	char msg[2048];
	sprintf(msg,"KeyAccess::set_bool : value with key %s read only.\n",
		key.c_str());
	throw Fault(msg);
    }

    booleanValues_[indexedKey] = value;
}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycTime & KeyAccess::get_RaycTime(const string key) throw (Fault){

    return get_time(key);
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::set_RaycTime(const string key, const RaycTime &value) throw (Fault){

    set_time(key,value);
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycTime & KeyAccess::get_time(const string key) throw (Fault){

    if(raycTimeValues_.count(key) == 0){
	char msg[2048];
	sprintf(msg,"KeyAccess::get_time : key %s not defined.\n",
		key.c_str());
	throw Fault(msg);
    }
    
    return raycTimeValues_[key];
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::set_time(const string key, const RaycTime &value) throw (Fault){

    if(key_is_read_only(key)){
	char msg[2048];
	sprintf(msg,"KeyAccess::set_time : value with key %s read only.\n",
		key.c_str());
	throw Fault(msg);
    }

    raycTimeValues_[key] = value;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycAngle & KeyAccess::get_RaycAngle(const string key) throw (Fault){

    return get_angle(key);
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::set_RaycAngle(const string key, const RaycAngle &value) throw (Fault){

    set_angle(key,value);
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RaycAngle & KeyAccess::get_angle(const string key) throw (Fault){

    if(raycAngleValues_.count(key) == 0){
	char msg[2048];
	sprintf(msg,"KeyAccess::get_angle : key %s not defined.\n",
		key.c_str());
	throw Fault(msg);
    }
    
    return raycAngleValues_[key];
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::set_angle(const string key, const RaycAngle &value) throw (Fault){

    if(key_is_read_only(key)){
	char msg[2048];
	sprintf(msg,"KeyAccess::set_angle : value with key %s read only.\n",
		key.c_str());
	throw Fault(msg);
    }

    raycAngleValues_[key] = value;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::set_key_read_only(const string key) throw (Fault){

    if(key_is_used(key)){
	readOnlyKeys_.insert(key);
    }else{
	char msg[2048];
	sprintf(msg,"KeyAccess::set_key_read_only : key %s not defined.\n",
		key.c_str());
	throw Fault(msg);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::set_key_read_only(const string key, const int index) throw (Fault){

    string indexedKey = make_indexed_key(key,index);

    set_key_read_only(indexedKey);

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool KeyAccess::key_is_used(const string key){

    if(integerValues_.count(key) != 0){
	return true;
    }

    if(longlongValues_.count(key) != 0){
	return true;
    }
    
    if(doubleValues_.count(key) != 0){
	return true;
    }
    
    if(stringValues_.count(key) != 0){
	return true;
    }

    if(raycTimeValues_.count(key) != 0){
	return true;
    }

    if(raycAngleValues_.count(key) != 0){
	return true;
    }
    
    return false;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool KeyAccess::key_is_read_only(const string key){

    // Since this an private/protected function, not
    // testing to see if key exists.  If it did not
    // an exception should be thrown.

    if(readOnlyKeys_.count(key) == 0){
	return false;
    }

    return true;
}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string KeyAccess::make_indexed_key(const string key, const int index){

    char ckey[256];
    sprintf(ckey,"%s_%02d",key.c_str(),index);
    
    return string(ckey);
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::validate_double(const string className, const string keyName) throw (ForayUtility::Fault){

    if(doubleValues_.count(keyName) == 0){
	char msg[2048];
	sprintf(msg,"%s::validate_double : %s is not set.\n",className.c_str(),keyName.c_str());
	throw Fault(msg);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::validate_double(const string className, const string keyName, const int index) throw (ForayUtility::Fault){

    string indexedKeyName = make_indexed_key(keyName,index);

    if(doubleValues_.count(indexedKeyName) == 0){
	char msg[2048];
	sprintf(msg,"%s::validate_double : %s is not set.\n",className.c_str(),indexedKeyName.c_str());
	throw Fault(msg);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::validate_integer(const string className, const string keyName) throw (ForayUtility::Fault){

    if(integerValues_.count(keyName) == 0){
	char msg[2048];
	sprintf(msg,"%s::validate_integer : %s is not set.\n",className.c_str(),keyName.c_str());
	throw Fault(msg);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::validate_integer(const string className, const string keyName, const int index) throw (ForayUtility::Fault){

    string indexedKeyName = make_indexed_key(keyName,index);

    if(integerValues_.count(indexedKeyName) == 0){
	char msg[2048];
	sprintf(msg,"%s::validate_integer : %s is not set.\n",className.c_str(),indexedKeyName.c_str());
	throw Fault(msg);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::validate_longlong(const string className, const string keyName) throw (ForayUtility::Fault){

    if(longlongValues_.count(keyName) == 0){
	char msg[2048];
	sprintf(msg,"%s::validate_longlong : %s is not set.\n",className.c_str(),keyName.c_str());
	throw Fault(msg);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::validate_string(const string className, const string keyName) throw (ForayUtility::Fault){

    if(stringValues_.count(keyName) == 0){
	char msg[2048];
	sprintf(msg,"%s::validate_string : %s is not set.\n",className.c_str(),keyName.c_str());
	throw Fault(msg);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::validate_string(const string className, const string keyName, const int index) throw (ForayUtility::Fault){

    string indexedKeyName = make_indexed_key(keyName,index);

    if(stringValues_.count(indexedKeyName) == 0){
	char msg[2048];
	sprintf(msg,"%s::validate_string : %s is not set.\n",className.c_str(),indexedKeyName.c_str());
	throw Fault(msg);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::validate_RaycTime(const string className, const string keyName) throw (ForayUtility::Fault){

    if(raycTimeValues_.count(keyName) == 0){
	char msg[2048];
	sprintf(msg,"%s::validate_RaycTime : %s is not set.\n",className.c_str(),keyName.c_str());
	throw Fault(msg);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void KeyAccess::validate_RaycAngle(const string className, const string keyName) throw (ForayUtility::Fault){

    if(raycAngleValues_.count(keyName) == 0){
	char msg[2048];
	sprintf(msg,"%s::validate_RaycAngle : %s is not set.\n",className.c_str(),keyName.c_str());
	throw Fault(msg);
    }
}


