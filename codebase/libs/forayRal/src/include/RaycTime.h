//
//
//
//
//

#ifndef RAYCTIME_H
#define RAYCTIME_H

#include <string>

#include "Fault.h"

namespace ForayUtility {
    class RaycTime {

    public:
	RaycTime();
	RaycTime(int seconds)                     throw (ForayUtility::Fault);
	RaycTime(double seconds)                  throw (ForayUtility::Fault);
	~RaycTime();
	RaycTime(const RaycTime &src);
	RaycTime &operator=(const RaycTime &src);

	int    seconds()     const;
	int    nanoSeconds() const;
	double value()       const;

	void set_current_time()                   throw (ForayUtility::Fault);

	void set(int seconds, int nanoSeconds)    throw (ForayUtility::Fault);
	void set(int seconds)                     throw (ForayUtility::Fault);
	void set(double seconds)                  throw (ForayUtility::Fault);
	void set(int year, int mounth, int day, 
		 int hour, int minute, int second,
		 int nanoseconds)                 throw (ForayUtility::Fault);

	std::string        list_entry();
	static std::string csv_full_head();
	std::string        csv_full_line();

	int get_year()        const;
	int get_month()       const;
	int get_day()         const;
	int get_hour()        const;
	int get_minute()      const;
	int get_second()      const;
	int get_nanosecond()  const;
	int get_julian_day()  const;

	RaycTime &operator+=(const int &)                throw(ForayUtility::Fault);
	RaycTime &operator+=(const double &)             throw(ForayUtility::Fault);

	RaycTime &operator-=(const int &)                throw(ForayUtility::Fault);
	RaycTime &operator-=(const double &)             throw(ForayUtility::Fault);

	bool operator<(const RaycTime &)  const;  
	bool operator>(const RaycTime &)  const;
	bool operator>=(const RaycTime &) const;
	bool operator==(const RaycTime &) const;

    private:

	double makeDouble() const;
	void   makeInt(double seconds);

	int seconds_;
	int nanoSeconds_;

    };
}


ForayUtility::RaycTime operator+(const ForayUtility::RaycTime &,const int &)      throw(ForayUtility::Fault);
ForayUtility::RaycTime operator+(const ForayUtility::RaycTime &,const double &)   throw(ForayUtility::Fault);

ForayUtility::RaycTime operator-(const ForayUtility::RaycTime &,const int &)      throw(ForayUtility::Fault);
ForayUtility::RaycTime operator-(const ForayUtility::RaycTime &,const double &)   throw(ForayUtility::Fault);
double   operator-(const ForayUtility::RaycTime &,const ForayUtility::RaycTime &);



#endif // RAYCTIME_H
