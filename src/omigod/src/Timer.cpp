#include "Timer.h"
#include <cstdio>
#include <cmath>

using namespace omigod;

Timer::Timer() {
    ilast = 0;
    icurr = 1;
    utime = stime = otime = 0.0;
    donothing = false;

    last_time = Clock::now();
    curr_time = Clock::now();

#ifdef _OPENMP
    omptime[ilast] = omp_get_wtime();
#endif
}

void Timer::ticAndPrint(const char* msg) {
    if (donothing) return;
    tic();
    print(msg);
}

void Timer::tic() {
    if (donothing) return;

    curr_time = Clock::now();
    std::chrono::duration<double> elapsed = curr_time - last_time;

    // Since we don't have user/sys time split in portable C++,
    // we assign everything to "utime"
    utime = elapsed.count();
    stime = 0.0;

#ifdef _OPENMP
    omptime[icurr] = omp_get_wtime();
    otime = omptime[icurr] - omptime[ilast];
#endif

    last_time = curr_time;
    ilast = 1 - ilast;
    icurr = 1 - icurr;
}

void Timer::print_sec_hms(const char* pref, double sec, const char* suf) {
    int m = static_cast<int>(std::floor(sec / 60.0));
    double s = sec - 60.0 * m;
    int h = m / 60;
    m = m - 60 * h;
    std::printf("%s%.6fs = %02dh%02dm%.3fs%s", pref, sec, h, m, s, suf);
}

void Timer::print(const char* msg) {
    if (donothing) return;
    double rtime = utime + stime;

    if (msg != nullptr)
        std::printf("%s", msg);

    print_sec_hms("RTime: ", rtime, ", ");
    print_sec_hms("UTime: ", utime, ", ");
    print_sec_hms("STime: ", stime, "");

#ifdef _OPENMP
    print_sec_hms(", OpenMP Time: ", otime, "\n");
#else
    std::printf("\n");
#endif
}

void Timer::get(double& r, double& u, double& s, double& o) const {
    r = utime + stime;
    u = utime;
    s = stime;
#ifdef _OPENMP
    o = otime;
#else
    o = -1.0;
#endif
}

void Timer::doNothing(bool on) {
    donothing = on;
}
