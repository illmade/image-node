//
//  timer.cpp
//  
//
//  Created by tim on 25/02/2017.
//
//
#include <sys/time.h>
#include "timer.hpp"

using namespace metrics;

timestamp_t timer::get_timestamp ()
{
    struct timeval now;
    gettimeofday (&now, NULL);
    return now.tv_usec / 1000.0 + (timestamp_t)now.tv_sec * 1000.0;
}


