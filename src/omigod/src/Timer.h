#ifndef __TIMER_
#define __TIMER_


#include <chrono>

#ifdef _OPENMP
#include <omp.h>
#endif


namespace omigod {

class Timer {
public:
    Timer();

    void tic();
    void ticAndPrint(const char* msg);
    void print(const char* msg);
    void get(double& r, double& u, double& s, double& o) const;
    void doNothing(bool on);

private:
    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    TimePoint last_time;
    TimePoint curr_time;

    double utime;
    double stime;
    double otime;

#ifdef _OPENMP
    double omptime[2];
#endif

    int ilast;
    int icurr;
    bool donothing;

    void print_sec_hms(const char* pref, double sec, const char* suf);
};

} // namespace omigod


#endif

