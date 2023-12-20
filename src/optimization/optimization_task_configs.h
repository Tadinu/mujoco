#ifndef MUJOCO_SRC_OPTIMIZATION_SIMULATIONCONSTANTS_H
#define MUJOCO_SRC_OPTIMIZATION_SIMULATIONCONSTANTS_H

#include "Simulation.h"
#include "simulation/Constants.h"

class OptimizationTaskConfigurations {
public:
    static Simulation::FabricConfiguration    normalFabric6lowres,   slopeFabricRestOnPlane,
      conitnuousNormalTestFabric, tshirt1000,   agenthat579, sock482,  dressv7khandsUpDrape, sphereFabric,
            normalFabric6;


    static Simulation::SceneConfiguration simpleScene,   rotatingSphereScene, windScene,
       tshirtScene,
            hatScene,  sockScene, dressScene,
         continousNormalScene, slopeSimplifiedScene;

    static Simulation::TaskConfiguration demoSphere, demoTshirt, demoWInd, demoHat, demoSock, demoDress, demoWindSim2Real, demoSlope;
    static std::vector<Simulation::SceneConfiguration> sceneConfigArrays;

    static std::map<Demos, Simulation::TaskConfiguration> demoNumToConfigMap;

};


#endif //MUJOCO_SRC_OPTIMIZATION_SIMULATIONCONSTANTS_H
