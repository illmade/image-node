//
//  timer.hpp
//  
//
//  Created by tim on 25/02/2017.
//
//

#ifndef timer_hpp
#define timer_hpp

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
    
    namespace metrics {
        
        typedef unsigned long long timestamp_t;
        
        class timer {
            
            public:
            static timestamp_t get_timestamp();
            
        };
    }

#ifdef __cplusplus
}
#endif

#endif

