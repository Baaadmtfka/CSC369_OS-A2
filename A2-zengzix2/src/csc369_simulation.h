/*
 * This code is provided solely for the personal and private use of students
 * taking the CSC369H course at the University of Toronto. Copying for purposes
 * other than this use is expressly prohibited. All forms of distribution of
 * this code, including but not limited to public repositories on GitHub,
 * GitLab, Bitbucket, or any other online platform, whether as given or with
 * any changes, are expressly prohibited.
 *
 * All of the files in this directory and all subdirectories are:
 * Copyright (c) 2019, 2021 Karen Reid
 * Copyright (c) 2023, Angela Brown, Kuei Sun
 * Copyright (c) 2024 Mario Badr
 */
#ifndef CSC369H1_MEMORY_SIMULATION_H
#define CSC369H1_MEMORY_SIMULATION_H

#include "csc369_policy.h"

#include <stdint.h>

/**
 * Manages a simulation.
 */
typedef struct csc369_simulation_t CSC369_Simulation;

/**
 * Create a simulation.
 *
 * @return A (newly-allocated) simulation, or NULL on error.
 */
CSC369_Simulation*
CSC369_SimulationCreate(char const* trace_file,
                        size_t frame_count,
                        size_t swap_size,
                        CSC369_Policy* policy);

/**
 * Destroy the simulation and free up any resources it holds.
 *
 * @param simulation A pointer to the simulation.
 *
 * @pre simulation is not NULL.
 */
void
CSC369_SimulationDestroy(CSC369_Simulation* simulation);

/**
 * Destroy the swap file without cleaning up memory.
 *
 * @param simulation A pointer to the simulation.
 *
 * @pre simulation is not NULL.
 */
void
CSC369_SimulationDestroySwap(CSC369_Simulation* simulation);

/**
 * Start the simulation.
 *
 * @param simulation A pointer to the simulation.
 *
 * @pre simulation is not NULL.
 */
void
CSC369_SimulationRun(CSC369_Simulation* simulation);

#endif // CSC369H1_MEMORY_SIMULATION_H
