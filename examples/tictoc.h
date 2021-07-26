#pragma once
#ifdef __cplusplus
extern "C"{
#endif
// ---------
// Profiling
// ---------
#include <stdint.h>

typedef struct _tic_toc_timer
{ uint64_t last; ///< last observation on the counter
  uint64_t rate; ///< counts per second  
} TicTocTimer;

TicTocTimer tic(void);              ///< Intitializes a timer.
double      toc(TicTocTimer *last); ///< Updates timer. \param[in,out] if \a last is NULL, a global timer is used. \returns time since last in seconds.


#ifdef __cplusplus
}
#endif
                    
