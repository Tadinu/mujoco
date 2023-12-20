#ifndef MUJOCO_SRC_OPTIMIZATION_FIXEDPOINT_H
#define MUJOCO_SRC_OPTIMIZATION_FIXEDPOINT_H

#include "Macros.h"

class FixedPoint {
public:
    Vec3d pos;
    Vec3d pos_rest;
    int idx;
    FixedPoint() : pos(0,0,0), pos_rest(0,0,0), idx(0){};
    FixedPoint(Vec3d pos_rest, int idx) : pos(pos_rest), pos_rest(pos_rest), idx(idx) {};
};


#endif //MUJOCO_SRC_OPTIMIZATION_FIXEDPOINT_H
