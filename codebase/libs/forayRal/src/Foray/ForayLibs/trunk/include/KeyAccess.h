//
//
//
//
//
//



#ifndef KEYACCESS
#define KEYACCESS

#include <string>
#include <set>
#include <map>

#include "Fault.h"

#include "RaycTime.h"
#include "RaycAngle.h"
namespace ForayUtility {

    class KeyAccess {
    public:
	KeyAccess();
	~KeyAccess();

	void        clear();

	short     & get_short(const std::string key)                                        throw (ForayUtility::Fault); 
	short     & get_short(const std::string key, const int index)                       throw (ForayUtility::Fault); 
	void        set_short(const std::string key, const short &value)                    throw (ForayUtility::Fault);
	void        set_short(const std::string key, const int index, 
			      const short &value)   throw (ForayUtility::Fault);

	int       & get_integer(const std::string key)                                      throw (ForayUtility::Fault);
	int       & get_integer(const std::string key, const int index)                     throw (ForayUtility::Fault);
	void        set_integer(const std::string key, const int &value)                    throw (ForayUtility::Fault);
	void        set_integer(const std::string key, const int index, const int &value)   throw (ForayUtility::Fault);

	long long & get_longlong(const std::string key)                                     throw (ForayUtility::Fault);
	void        set_longlong(const std::string key, const long long &value)             throw (ForayUtility::Fault);
    
	double    & get_double(const std::string key)                                       throw (ForayUtility::Fault);
	double    & get_double(const std::string key, const int index)                      throw (ForayUtility::Fault);
	void        set_double(const std::string key, const double &value)                  throw (ForayUtility::Fault);
	void        set_double(const std::string key, const int index, const double &value) throw (ForayUtility::Fault);

	std::string & get_string(const std::string key)                                            throw (ForayUtility::Fault);
	std::string & get_string(const std::string key, const int index)                           throw (ForayUtility::Fault);
	void          set_string(const std::string key, const std::string &value)                  throw (ForayUtility::Fault);
	void          set_string(const std::string key, const int index, const std::string &value) throw (ForayUtility::Fault);

	bool      & get_boolean(const std::string key)                                      throw (ForayUtility::Fault);
	bool      & get_boolean(const std::string key, const int index)                     throw (ForayUtility::Fault);
	void        set_boolean(const std::string key, const bool &value)                   throw (ForayUtility::Fault);
	void        set_boolean(const std::string key, const int index, const bool &value)  throw (ForayUtility::Fault);

	RaycTime  & get_RaycTime(const std::string key)                                     throw (ForayUtility::Fault);
	void        set_RaycTime(const std::string key, const RaycTime &value)              throw (ForayUtility::Fault);

	RaycTime  & get_time(const std::string key)                                         throw (ForayUtility::Fault);
	void        set_time(const std::string key, const RaycTime &value)                  throw (ForayUtility::Fault);

	RaycAngle & get_RaycAngle(const std::string key)                                    throw (ForayUtility::Fault);
	void        set_RaycAngle(const std::string key, const RaycAngle &value)            throw (ForayUtility::Fault);

	RaycAngle & get_angle(const std::string key)                                        throw (ForayUtility::Fault);
	void        set_angle(const std::string key, const RaycAngle &value)                throw (ForayUtility::Fault);

	void        set_key_read_only(const std::string key)                                throw (ForayUtility::Fault);
	void        set_key_read_only(const std::string key, const int index)               throw (ForayUtility::Fault);
    
    protected:

	std::string    make_indexed_key(const std::string key,const int index);
	bool      key_is_used(const std::string key);
	bool      key_is_read_only(const std::string key);

	void      validate_double   (const std::string className, const std::string keyName)                  throw(ForayUtility::Fault);
	void      validate_double   (const std::string className, const std::string keyName, const int index) throw(ForayUtility::Fault);
	void      validate_integer  (const std::string className, const std::string keyName)                  throw(ForayUtility::Fault);
	void      validate_integer  (const std::string className, const std::string keyName, const int index) throw(ForayUtility::Fault);

	void      validate_short  (const std::string className, const std::string keyName)                    throw(ForayUtility::Fault);
	void      validate_short  (const std::string className, const std::string keyName, const int index)   throw(ForayUtility::Fault);

	void      validate_longlong (const std::string className, const std::string keyName)                  throw(ForayUtility::Fault);
	void      validate_string   (const std::string className, const std::string keyName)                  throw(ForayUtility::Fault);
	void      validate_string   (const std::string className, const std::string keyName, const int index) throw(ForayUtility::Fault);
	void      validate_RaycTime (const std::string className, const std::string keyName)                  throw(ForayUtility::Fault);
	void      validate_RaycAngle(const std::string className, const std::string keyName)                  throw(ForayUtility::Fault);


	std::map<std::string,int>         integerValues_;
	std::map<std::string,long long>   longlongValues_;
	std::map<std::string,double>      doubleValues_;
	std::map<std::string,std::string> stringValues_;
	std::map<std::string,short>       shortValues_;
	std::map<std::string,bool>        booleanValues_;

	std::map<std::string,RaycTime>    raycTimeValues_;
	std::map<std::string,RaycAngle>   raycAngleValues_;

	std::set<std::string>             readOnlyKeys_;


    };
}

#endif   // KEYACCESS
