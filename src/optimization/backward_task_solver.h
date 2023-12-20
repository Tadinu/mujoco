#ifndef MUJOCO_SRC_OPTIMIZATION_BACKWARDTASKSOLVER_H
#define MUJOCO_SRC_OPTIMIZATION_BACKWARDTASKSOLVER_H

#include <time.h>
#include "optimization_task_configs.h"
#include "optimization_helper.h"
#include "Simulation.h"
#include "simulation/Constants.h"

class BackwardTaskSolver {
public:
    static void runSim(Demos demoId, bool isRandom, int srandSeed);
    static void
    optimizeLBFGS(Simulation *system, OptimizeHelper& helper,
                  int FORWARD_STEPS,  Demos demoId, bool isRandom,
                  int srandSeed, const std::function<void(const std::string &)> &setTextBoxCB);



    static void setWindSim2realInitialParams(
                                             Simulation::ParamInfo &paramGroundtruth,
                                             Simulation::BackwardTaskInformation &taskInfo, Simulation *system);

    static void setDemoSceneConfigAndConvergence(Simulation* system, Demos demoId, Simulation::BackwardTaskInformation &taskInfo);

    static void resetSplineConfigsForControlTasks(Demos demoId, Simulation *system,
                                                  Simulation::ParamInfo &paramGroundtruth);

 

    static void
    setLossFunctionInformationAndType(LossType &lossType, Simulation::LossInfo &lossInfo, Simulation *system,
                                      Demos demoId);


    static void
    setInitialConditions(Demos demoId, Simulation *system,
                         Simulation::ParamInfo &paramGroundtruth, Simulation::BackwardTaskInformation &taskInfo);

    static void
    solveDemo(Simulation *system, const std::function<void(const std::string &)> &setTextBoxCB,
              Demos demoId, bool isRandom, int srandSeed);

    static OptimizeHelper getOptimizeHelper(Simulation *system, Demos demoId);

    static OptimizeHelper *getOptimizeHelperPointer(Simulation *system, Demos demoId);

};


#endif //MUJOCO_SRC_OPTIMIZATION_BACKWARDTASKSOLVER_H
