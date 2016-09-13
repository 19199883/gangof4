#include "util.h"
#include <string>

#include <boost/date_time.hpp>
#include <boost/date_time/gregorian_calendar.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

void ConvertTimval( long long loc_t, int &d, int &t )
{
	boost::posix_time::ptime pt(boost::gregorian::date(1970, 1, 1), boost::posix_time::time_duration(0, 0, 0));
	long long s_l = loc_t / 1000000;
	long long us_l = loc_t % 1000000;
	int ms_i = (int)(us_l / 1000);
	pt += boost::posix_time::seconds((long)s_l);

	// 时区校正
	pt += boost::posix_time::hours(8);

	d = t = 0;
	d = pt.date().year() * 10000 + pt.date().month() * 100 + pt.date().day();
	t = pt.time_of_day().hours() * 10000 + pt.time_of_day().minutes() * 100 + pt.time_of_day().seconds();
	t *= 1000;
	t += ms_i;
}
std::string ConvertNanoTime( long long loc_t )
{
	long long s_l = loc_t / 1000000000;
	int ns_l = (int)(loc_t % 1000000000);
	boost::posix_time::ptime pt(boost::gregorian::date(1970, 1, 1), boost::posix_time::time_duration(0, 0, 0));
	pt += boost::posix_time::seconds((long)s_l);

	// 时区校正
	pt += boost::posix_time::hours(8);

	char buf[64];
	sprintf(buf, "%02d%02d%02d%09d",
		pt.time_of_day().hours(), pt.time_of_day().minutes(), pt.time_of_day().seconds(), ns_l);

	return buf;
}


std::string ConvertToShortTimval( long long loc_t )
{
	if (loc_t > 10000000000000000) return ConvertNanoTime(loc_t);

	long long s_l = loc_t / 1000000;
	int us_l = (int)(loc_t % 1000000);
	int ms_l = us_l / 1000;
	boost::posix_time::ptime pt(boost::gregorian::date(1970, 1, 1), boost::posix_time::time_duration(0, 0, 0));
	pt += boost::posix_time::seconds((long)s_l);

	// 时区校正
	pt += boost::posix_time::hours(8);

	char buf[64];
	sprintf(buf, "%02d%02d%02d%06d",
		pt.time_of_day().hours(), pt.time_of_day().minutes(), pt.time_of_day().seconds(), us_l);

	return buf;
}

std::string TrimAndToString( const char * const p, int len )
{
	std::string t(p, len);
	boost::trim_right(t);
	return t;
}
