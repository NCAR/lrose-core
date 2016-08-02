#ifndef _RDR_ANGLE_H_
#define _RDR_ANGLE_H_

class rdr_angle {
	void limit();
	short m_angle;
public:
	// constructors
	rdr_angle(rdr_angle &angle);
	rdr_angle(int angle = 0);
	rdr_angle(float angle);

	// operators
	rdr_angle& operator=(const int &rhs);
	rdr_angle& operator=(const float &rhs);
	rdr_angle& operator=(const rdr_angle &rhs);

	rdr_angle  operator+(const rdr_angle &rhs);
	rdr_angle  operator+(const int &rhs);
	rdr_angle  operator+(const float &rhs);

	rdr_angle& operator+=(const rdr_angle &rhs);
	rdr_angle& operator+=(const int &rhs);
	rdr_angle& operator+=(const float &rhs);

	rdr_angle  operator-(const rdr_angle &rhs);
	rdr_angle  operator-(const int &rhs);
	rdr_angle  operator-(const float &rhs);

	rdr_angle& operator-=(const rdr_angle &rhs);
	rdr_angle& operator-=(const int &rhs);
	rdr_angle& operator-=(const float &rhs);

	rdr_angle  operator/(const rdr_angle &rhs);
	rdr_angle  operator/(const int &rhs);
	rdr_angle  operator/(const float &rhs);

	rdr_angle& operator/=(const rdr_angle &rhs);
	rdr_angle& operator/=(const int &rhs);
	rdr_angle& operator/=(const float &rhs);

	rdr_angle  operator*(const rdr_angle &rhs);
	rdr_angle  operator*(const int &rhs);
	rdr_angle  operator*(const float &rhs);

	rdr_angle& operator*=(const rdr_angle &rhs);
	rdr_angle& operator*=(const int &rhs);
	rdr_angle& operator*=(const float &rhs);

	bool operator>(const rdr_angle &rhs);
	bool operator>(const int &rhs);
	bool operator>(const float &rhs);

	bool operator>=(const rdr_angle &rhs);
	bool operator>=(const int &rhs);
	bool operator>=(const float &rhs);

	bool operator<(const rdr_angle &rhs);
	bool operator<(const int &rhs);
	bool operator<(const float &rhs);

	bool operator<=(const rdr_angle &rhs);
	bool operator<=(const int &rhs);
	bool operator<=(const float &rhs);

	bool operator==(const rdr_angle &rhs);
	bool operator==(const int &rhs);
	bool operator==(const float &rhs);

	bool operator!=(const rdr_angle &rhs);
	bool operator!=(const int &rhs);
	bool operator!=(const float &rhs);

	// special functions
	rdr_angle normalise(int angres);
	rdr_angle normalise(float angres);
	rdr_angle normalise(rdr_angle &angres);
//	rdr_angle& normalise(int angres);
//	rdr_angle& normalise(float angres);
//	rdr_angle& normalise(rdr_angle &angres);

	int	index(int angres);
	int	index(float angres);
	int	index(rdr_angle &angres);

	float	getangle();
	int	numindexes(int res);	
	int	numindexes(float res);	
	int	numindexes(rdr_angle &res);
};

#endif
