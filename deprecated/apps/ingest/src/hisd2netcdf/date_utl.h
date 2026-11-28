/* ----------------------------------------------------------------------------
	Sample source code for Himawari Satandard Data

	Copyright (C) 2015 MSC (Meteorological Satellite Center) of JMA

	Disclaimer:
		MSC does not guarantee regarding the correctness, accuracy, reliability,
		or any other aspect regarding use of these sample codes.

	Detail of Himawari Standard Format:
		For data structure of Himawari Standard Format, prelese refer to MSC
		Website and Himawari Standard Data User's Guide.

		MSC Website
		http://www.jma-net.go.jp/msc/en/

		Himawari Standard Data User's Guide
		http://www.data.jma.go.jp/mscweb/en/himawari89/space_segment/hsd_sample/HS_D_users_guide_en.pdf

	History
		March,   2015  First release

---------------------------------------------------------------------------- */

void mjd_to_date(double mjd, int date[7]);
void DateGetNowInts(int  date[7] );
double DateIntsToMjd(const int date[7] );
