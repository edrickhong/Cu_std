#include "ttype.h"

/*
check available controllers by index. if they are not in the controller list, send event to add to list.
if disconnected, send event
*/

#ifdef __cplusplus
extern "C" {
#endif

b32 CInitControllers();

#ifdef __cplusplus
}
#endif
