//
//  Initialization_Data_header.h
//  Amethyne
//
//  Created by AnshuDiya on 11/1/26.
//

#ifndef Initialization_Data_header_hpp
#define Initialization_Data_header_hpp


#endif /* Initialization_Data_header_h */

#pragma once
#include <random>

struct RelationsMetadata{
    //refer to relationsData reltion calculatoin function for usage
    static float cutoffThresholdForVariableRelation;
    static float epsilon;
};

inline std::random_device Amethyne_random_device;
inline std::mt19937 Amethyne_rand(Amethyne_random_device());
