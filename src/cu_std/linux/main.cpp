#include "thread_mode.h"

#include "linux_global.cpp"

#ifdef DEBUG
// cos thread tracking in the vulkan layers will not work withh raw threads
//#include "tthread.cpp"
#else
#endif

// thread
#if _use_pthread
#include "pthread_tthread.cpp"
#else
#include "raw_tthread.cpp"
#endif

// audio
#include "aaudio.cpp"
// controller
#include "ccontroller.cpp"
// fileio
#include "ffileio.cpp"
// timer
#include "ttimer.cpp"

// window
#include "wwindow.cpp"
