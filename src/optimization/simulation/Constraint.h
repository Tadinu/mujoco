#ifndef MUJOCO_SRC_OPTIMIZATION_CONSTRAINT_H
#define MUJOCO_SRC_OPTIMIZATION_CONSTRAINT_H

#include <Eigen/Sparse>
#include "Macros.h"
#include "Particle.h"

class Constraint {
public:
    enum ConstraintType {
        CONSTRAINT_SPRING_STRETCH,
        CONSTRAINT_ATTACHMENT,
        CONSTRAINT_TRIANGLE,
        CONSTRAINT_TRIANGLE_BENDING,
        CONSTRAINT_NUM
    };

    static std::vector<std::string> CONSTRAINT_NAME;
    static double k_stiff;

    ConstraintType constraintType;
    int constraintNum = -1;
    int c_idx = -1, c_weightless_idx = -1;
    double energyBuffer;

    Constraint() : constraintType(CONSTRAINT_NUM), constraintNum(-1) {}

    Constraint(ConstraintType t, int constraintNum) : constraintType(t), constraintNum(constraintNum) {}

    virtual ~Constraint() {}

    virtual void setConstraintWeight() {
      std::printf("WARNING\n");
      assert(false);
    };

    virtual VecXd project(const VecXd &x_vec) const {
      std::printf("WARNING\n");
      assert(false);
      return VecXd();
    }

    virtual void projectBackwardPrecompute(const VecXd &x_vec) {
      std::printf("WARNING\n");
      assert(false);
    }

    virtual void projectBackward(const VecXd &x_vec, TripleVector& triplets) {
      std::printf("WARNING\n");
      assert(false);
    }

    virtual VecXd dp_dk(const VecXd &x_vec) const {
      std::printf("WARNING\n");
      assert(false);
      return VecXd();
    }

    virtual double evaluateEnergy(const VecXd &x_new) {
      std::printf("WARNING\n");
      assert(false);
      return 0;
    }

    virtual void addConstraint(std::vector<Triplet> &tri, int &c_idx, bool withWeight = true) {
      std::printf("WARNING\n");
      assert(false);
    }
};


#endif //MUJOCO_SRC_OPTIMIZATION_CONSTRAINT_H
