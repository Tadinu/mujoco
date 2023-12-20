set(MUJOCO_OPTIMIZATION_SIMULATION_SRCS
    AttachmentSpring.cpp
    AttachmentSpring.h
    Constants.cpp
    Constants.h
    Constraint.cpp
    Constraint.h
    FixedPoint.h
    Macros.h
    Particle.cpp
    Particle.h
    Primitive.cpp
    Primitive.h
    Simulation.cpp
    Simulation.h
    Spline.cpp
    Spline.h
    Spring.cpp
    Spring.h
    TriangleBending.cpp
    TriangleBending.h
    Triangle.cpp
    Triangle.h
    Timer.cpp
    Timer.h
    UtilityFunctions.h
)

target_sources(mujoco PRIVATE ${MUJOCO_OPTIMIZATION_SIMULATION_SRCS})
target_include_directories(
  mujoco
  PRIVATE simulation
)