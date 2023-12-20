#ifndef MUJOCO_SRC_OPTIMIZATION_PARTICLE_H
#define MUJOCO_SRC_OPTIMIZATION_PARTICLE_H

#include "Macros.h"

class Particle {
public:
    double mass = 0;
    Vec3d pos_rest = Vec3d(0.0, 0.0, 0.0); // position where strain will be 0
    Vec3d pos_init = Vec3d(0.0, 0.0, 0.0); // starting position
    Vec3d pos = Vec3d(0.0, 0.0, 0.0);
    double radii = 0;
    double area = 0; // the distributed area onto this particle

    Vec3d velocity = Vec3d(0.0, 0.0, 0.0);
    Vec3d velocity_init = Vec3d(0.0, 0.0, 0.0);
    Vec2i planeCoord = Vec2i(0,0); // int coordiant in 2D cloth plane1
    Vec3d normal = Vec3d(0.0, 0.0, 0.0);
    int idx = 0;


    Particle(double mass, const Vec3d &rest_pos, const Vec3d &pos_initial, const Vec3d &velocity,
             const Vec2i &planeCoord,
             int idx)
            : mass(mass), pos_rest(rest_pos), pos_init(pos_initial), pos(pos_initial), velocity(velocity),
              velocity_init(velocity),
              planeCoord(planeCoord), normal(0, 0, 0), idx(idx)
              {};

    void addNormal(const Vec3d &n) {
      normal += n;
    }

    void clearNormal() {
      normal = Vec3d(0.0, 0.0, 0.0);
    }

    void printState() const {
      std::printf("Paricle %d x=(%.2f,%.2f,%.2f) v=(%.9f,%.9f,%.9f)\n", idx, pos[0], pos[1], pos[2], velocity[0],
                  velocity[1], velocity[2]);
    }

};


#endif //MUJOCO_SRC_OPTIMIZATION_PARTICLE_H
