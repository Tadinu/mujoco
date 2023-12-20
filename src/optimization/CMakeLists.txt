set(MUJOCO_OPTIMIZATION_SRCS
    backward_task_solver.cpp
    backward_task_solver.h
    optimization_helper.cpp
    optimization_helper.h
    optimization_task_configs.cpp
    optimization_task_configs.h
    optimization_task_setup.cpp
    optimization_task_setup.h
)

find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
    message("OpenMP version ${OpenMP_CXX_VERSION}")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DOPENMP_AVAILABLE)
endif()

target_sources(mujoco PRIVATE ${MUJOCO_OPTIMIZATION_SRCS})

find_package(Eigen3 REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Eigen3::Eigen)
add_subdirectory(simulation)