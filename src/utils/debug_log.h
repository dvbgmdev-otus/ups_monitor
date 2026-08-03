#ifndef DEBUG_LOG_H
#define DEBUG_LOG_H

#ifdef ENABLE_DEBUG_LOG
#include <iostream>
#define DEBUG_LOG(msg) \
    do { std::cout << msg << '\n'; } while (false)
#else
#define DEBUG_LOG(msg) \
    do { } while (false)
#endif  // ENABLE_DEBUG_LOG

#endif  // DEBUG_LOG_H
