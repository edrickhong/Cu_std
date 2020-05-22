#include "thread_mode.h"

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
