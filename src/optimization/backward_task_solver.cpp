#include "LBFGSB.h"
#include "LBFGSpp/Param.h"
#include "backward_task_solver.h"
#include "optimization_task_setup.h"
#include "optimization_task_configs.h"
#include "simulation/Simulation.h"

void BackwardTaskSolver::runSim(Demos demoId, bool isRandom, int srandSeed) {
  Simulation::SceneConfiguration initSceneProfile = OptimizationTaskConfigurations::hatScene;
  Simulation *clothSystem = Simulation::createSystem(
                                                     initSceneProfile,
                                                     Vec3d(0, 0, 0), true);

  BackwardTaskSolver::solveDemo(clothSystem, [&](const std::string &v) {}, demoId, isRandom,
                                srandSeed);
  delete clothSystem;
}

void BackwardTaskSolver::solveDemo(Simulation *system, const std::function<void(const std::string &)> &setTextBoxCB,
                                   Demos demoId, bool isRandom, int srandSeed) {
  OptimizeHelper helper = getOptimizeHelper(system, demoId);

  helper.taskInfo.optimizer = Optimizer::LBFGS;
  optimizeLBFGS(system,helper, system->sceneConfig.stepNum, demoId, isRandom, srandSeed, setTextBoxCB);


}


void BackwardTaskSolver::optimizeLBFGS(Simulation *system, OptimizeHelper& helper,
                                       int FORWARD_STEPS, Demos demoId, bool isRandom,
                                       int srandSeed, const std::function<void(const std::string &)> &setTextBoxCB) {


  LBFGSpp::LBFGSBParam<double> param;
  param.delta = 0.001; // objective function convergence param
  param.m = 10;
  param.max_linesearch = 20;



  LBFGSpp::LBFGSBSolver<double> solver(param);


  std::printf("Parameter bounds are:\n");
  std::cout << "Lower:" << helper.paramLowerBound.transpose() << "\n";
  std::cout << "Upper:" << helper.paramUpperBound.transpose() << "\n";
  VecXd bestParam = helper.paramInfoToVecXd(helper.param_guess);

  if (isRandom ) {
    std::printf("Srand seed is : %d\n", srandSeed);
    bestParam = helper.getRandomParam(srandSeed);
  }

  std::cout << "lbfgs param lowerBound:" << helper.getLowerBound().transpose() << std::endl;
  std::cout << "lbfgs param upperBound:" << helper.getUpperBound().transpose() << std::endl;
  std::cout << "lbfgs starts with param:" << bestParam.transpose() << std::endl;
  double minLoss = 0;
  try {
    int niter = solver.minimize(helper, bestParam, minLoss, helper.getLowerBound(),
                                helper.getUpperBound());
    system->exportStatistics((Demos) demoId, helper.statistics, helper.taskInfo, true);
    if (niter == 1)
      helper.saveLastIter();
    std::printf("LBFGS terminated\n");
    std::cout << niter << " iterations" << std::endl;
    std::cout << "x = \n" << bestParam.transpose() << std::endl;
    std::cout << "f(x) = " << minLoss << std::endl;
  } catch (const std::exception &error) {
    std::printf("LBFGS terminated early: %s\n", error.what());
    system->exportStatistics((Demos) demoId, helper.statistics, helper.taskInfo, true);

  }
}


OptimizeHelper *BackwardTaskSolver::getOptimizeHelperPointer(Simulation *system, Demos demoId) {
  //Logging::logOk("getOptimizeHelperPointer for task" +  DEMOS_STRINGS[demoId] + "\n");
  OptimizeHelper helper = getOptimizeHelper(system, demoId);

  return new OptimizeHelper(static_cast<Demos>(demoId), helper.system, helper.lossInfo, helper.taskInfo, helper.lossType,
                            helper.system->sceneConfig.stepNum, helper.param_actual);
}

OptimizeHelper BackwardTaskSolver::getOptimizeHelper(Simulation *system, Demos demoId) {
  if (demoId == Demos::DEMO_WIND_SIM2REAL) {
    std::string folder = std::string(SOURCE_PATH) + "/src/assets/animation/flag-ryanwhite";
    std::vector<std::string>  frameFileNames = Utils::listFiles(folder, ".obj");

    std::sort(frameFileNames.begin(), frameFileNames.end());
    system->loadWindSim2RealAnimationSequence("flag-ryanwhite", frameFileNames, true);
  }

  system->runBackward = true;
  system->debugPointTargetPos.clear();
  system->debugShapeTargetPos.clear();
  system->backwardRecordsFiniteDiff.clear();
  system->backwardOptimizationRecords.clear();
  system->backwardOptimizationGuesses.clear();


  Simulation::BackwardTaskInformation taskInfo = Simulation::taskConfigDefault;
  Simulation::ParamInfo  paramGroundtruth;
  Simulation::LossInfo lossInfo;
  LossType lossType;

  setDemoSceneConfigAndConvergence(system, demoId, taskInfo);
  if (demoId != Demos::DEMO_WIND_SIM2REAL)  {
    system->createClothMesh();
    system->initScene();
  }

  setInitialConditions(demoId, system,  paramGroundtruth, taskInfo);
  if (OptimizationTaskConfigurations::demoNumToConfigMap[demoId].hasGroundtruth) {
    //Logging::logColor("Groundtruth param:" + Simulation::parameterToString(taskInfo, paramGroundtruth) + "\n",
                      //Logging::LogColor::MAGENTA);
  }


  setLossFunctionInformationAndType(lossType, lossInfo, system, demoId);


  if (OptimizationTaskConfigurations::demoNumToConfigMap[demoId].generateGroundtruthSimulation) {
    system->resetSystemWithParams(taskInfo, paramGroundtruth);
    for (int i = 0; i < system->sceneConfig.stepNum; i++) system->step();
    system->groundTruthForwardRecords = system->forwardRecords;
  }
  setLossFunctionInformationAndType(lossType, lossInfo, system, demoId);
  for (int i = 0; i < system->sysMat.size(); i++) {
    taskInfo.splineTypes.emplace_back();
    for (Spline &s : system->sysMat[i].controlPointSplines) {
      taskInfo.splineTypes[i].emplace_back(s.type);
    }
  }


  system->groundtruthParam = paramGroundtruth;
  system->taskInfo = taskInfo;

  int srandSeed = time(NULL);
  std::srand(srandSeed);

  return OptimizeHelper(static_cast<Demos>(demoId), system, lossInfo, taskInfo, lossType,
                        system->sceneConfig.stepNum, paramGroundtruth);
}
