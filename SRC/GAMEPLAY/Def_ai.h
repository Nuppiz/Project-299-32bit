#ifndef DEF_AI_H
#define DEF_AI_H

/* Defines related to AI functions */

#define CHASE_DISTANCE          75
#define CHASE_DISTANCE_SQ       (CHASE_DISTANCE*CHASE_DISTANCE)
#define MIN_CHASE_DISTANCE      60
#define MIN_CHASE_DISTANCE_SQ   (MIN_CHASE_DISTANCE*MIN_CHASE_DISTANCE)
#define CHASE_TIMEOUT           200
#define TURN_THRESHOLD          5.0

#define VIEW_ANGLE_COS      0.5
#define LOS_STEP            10
#define LOS_STEP_SQ         (LOS_STEP*LOS_STEP)
#define OUT_OF_SIGHT        0
#define IN_SIGHT            1

#define LEFT_SIDE           1
#define RIGHT_SIDE          2

#endif /* DEF_AI_H */
