#ifndef DEF_STE_H
#define DEF_STE_H

/* Defines related to the state machine */

#include "Def_bit.h"

#define STATE_IS_ACTIVE        BIT_0
#define STATE_ENABLE_UPDATE    BIT_1
#define STATE_ENABLE_DRAW      BIT_2

#define ALL_STATE_FLAGS (STATE_IS_ACTIVE|STATE_ENABLE_UPDATE|STATE_ENABLE_DRAW)

#endif /* DEF_STE_H */
