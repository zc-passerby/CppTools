/*!
 * \file	util/timeval.h
 * \brief	Time storage with micro-second resolution.
 */

#ifndef _TIMEVAL_H_
#define _TIMEVAL_H_

/*----------------------------- Dependencies -------------------------------*/

#include <sys/time.h>
#include <sstream>
#include <iomanip>
#include <string.h>
#include <stdlib.h>

/*--------------------------------------------------------------------------*/

namespace util
{

/*!
	 * \class Timeval
	 * \brief Time storage with micro-second resolution, similar to the
	 *		  standard C library 'struct timeval'
	 */
class Timeval
{
public:
    /*!
		  * ctors, using default copy ctor.
		*/
    Timeval() : sec_(0L), usec_(0L) {}
    Timeval(time_t sec, suseconds_t usec = 0L);

    /*!
		 * operators
		 */
    Timeval &operator=(const Timeval &t);

    bool operator==(const Timeval &t) const;
    bool operator!=(const Timeval &t) const;
    bool operator<(const Timeval &t) const;

    Timeval &operator+=(const Timeval &t);
    const Timeval operator+(const Timeval &t) const;

    Timeval &operator-=(const Timeval &t);
    const Timeval operator-(const Timeval &t) const;

    Timeval &operator*=(int t);
    const Timeval operator*(int n) const;

    /*!
		 * standard library equivalents.
		 */
    Timeval &getTimeOfDay();

    /*!
		 * printing
		 */
    friend std::ostream &operator<<(std::ostream &s, const util::Timeval &t);

    std::ostream &print(std::ostream &s) const;
    const std::string toString() const;

private:
    time_t sec_;       /* whole seconds */
    suseconds_t usec_; /* microseconds */
};

///////////////////////////////////////////////////////////////////////////

inline Timeval::Timeval(time_t sec, suseconds_t usec)
{
    long nsec = usec / 1000000L;
    sec_ = sec + nsec;
    usec_ = usec - nsec * 1000000L;
}

inline Timeval &Timeval::operator=(const Timeval &t)
{
    if (this == &t)
    {
        return *this;
    }
    sec_ = t.sec_;
    usec_ = t.usec_;
    return *this;
}

inline bool Timeval::operator==(const Timeval &t) const
{
    return ((sec_ == t.sec_) && (usec_ == t.usec_));
}

inline bool Timeval::operator!=(const Timeval &t) const
{
    return !(*this == t);
}

inline bool Timeval::operator<(const Timeval &t) const
{
    return (sec_ == t.sec_) ? usec_ < t.usec_ : sec_ < t.sec_;
}

inline Timeval &Timeval::operator+=(const Timeval &t)
{
    sec_ += t.sec_;
    usec_ += t.usec_;
    if (usec_ >= 1000000L)
    {
        usec_ -= 1000000L;
        sec_ += 1L;
    }
    return *this;
}

inline const Timeval Timeval::operator+(const Timeval &t) const
{
    return Timeval(*this) += t;
}

inline Timeval &Timeval::operator-=(const Timeval &t)
{
    sec_ -= t.sec_;
    if (usec_ < t.usec_)
    {
        usec_ += 1000000L;
        sec_ -= 1L;
    }
    usec_ -= t.usec_;

    return *this;
}

inline const Timeval Timeval::operator-(const Timeval &t) const
{
    return Timeval(*this) -= t;
}

inline Timeval &Timeval::operator*=(int t)
{
    usec_ *= t;
    long nsec = usec_ / 1000000L;
    sec_ = t * sec_ + nsec;
    usec_ -= nsec * 1000000L;

    return *this;
}

inline const Timeval Timeval::operator*(int n) const
{
    return Timeval(*this) *= n;
}

inline Timeval &Timeval::getTimeOfDay()
{
    timeval tv;
    gettimeofday(&tv, NULL);
    sec_ = tv.tv_sec;
    usec_ = tv.tv_usec;

    return *this;
}

inline const std::string Timeval::toString() const
{
    std::stringstream ss;
    ss << sec_ << '.' << std::setfill('0')
       << std::setw(6) << std::setprecision(6) << usec_;
    return ss.str();
}

inline std::ostream &Timeval::print(std::ostream &s) const
{
    s << toString();
    return s;
}

/* friend */
inline std::ostream &operator<<(std::ostream &s, const util::Timeval &t)
{
    return t.print(s);
}

//get current time stamp,such as 1371090607580958
static unsigned long get_current_time_stamp()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return (1000000 * now.tv_sec + now.tv_usec);
}
//convert timestamp to string
static std::string convert_timestamp_to_str(unsigned long timestamp)
{
    //get current date
    char return_time[30];
    struct tm _date;
    time_t seconds = timestamp / 1000000;
    localtime_r(&seconds, &_date);
    memset(return_time, 0, sizeof(return_time));
    strftime(return_time, sizeof(return_time) - 1, "%Y-%m-%d %H:%M:%S", &_date);
    return return_time;
}

static std::string convert_timestamp_to_str_by_format(unsigned long timestamp, const char *timeformat)
{
    //get current date
    char return_time[50];
    struct tm _date;
    time_t seconds = timestamp / 1000000;
    localtime_r(&seconds, &_date);
    memset(return_time, 0, sizeof(return_time));
    strftime(return_time, sizeof(return_time) - 1, timeformat, &_date);
    return return_time;
}

// convert string to timestamp
/*static unsigned long convert_str_to_timestamp(char *str_time)
{
    struct tm _date;
    strptime(str_time, "%Y-%m-%d %H:%M:%S", &_date);
    return mktime(&_date) * 1000000;
}*/
static unsigned long convert_str_to_timestamp_by_format(char *str_time, const char *timeformat = "%Y-%m-%d %H:%M:%S")
{
    struct tm _date;
    strptime(str_time, timeformat, &_date);
    return mktime(&_date) * 1000000;
}


static time_t convert_str_to_tm(char *str_time)
{
    struct tm tt;
    memset(&tt, 0, sizeof(tt));
    tt.tm_year = atoi(str_time) - 1900;
    tt.tm_mon = atoi(str_time + 5) - 1;
    tt.tm_mday = atoi(str_time + 8);
    tt.tm_hour = atoi(str_time + 11);
    tt.tm_min = atoi(str_time + 14);
    tt.tm_sec = atoi(str_time + 17);
    return mktime(&tt);
}

static std::string get_current_time_str()
{
    return convert_timestamp_to_str(get_current_time_stamp());
}

static std::string get_current_time_str_by_format(std::string timeformat)
{
    return convert_timestamp_to_str_by_format(get_current_time_stamp(), timeformat.c_str());
}
} // namespace util

#endif // _TIMEVAL_H_

/* vim:ts=4:set nu:
 * EOF
 */
