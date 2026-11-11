//
//  Transfer_Manager.hpp
//  Amethyne
//
//  Created by AnshuDiya on 11/9/26.
//
#pragma once

#ifndef Transfer_Manager_hpp
#define Transfer_Manager_hpp

#include <stdio.h>
#include "Relations_Header.hpp"
#include <span>


struct tagCounters{
    int metric = 1;
    int action = 1;
    int synthetic = 1;
};

//responsible for the bridge between testingGround and coldStorage as well as updating the symbolic variables while they're being transferred! also assings TAGS.
//how tags work is tag%3==0 tags are reserved for action variables
//tag%3==1 tags are reserved for metric variables
//and finally tag%3==2 tags are reserved for synthetic variables
//this way there are ZERO shared tags across variables! even synthetic addons meant for testing only contain tags as a reference to what existing variables they modify!
class TransferManager{
    tagCounters tagCounter;
    //std::span view of all the data referenced by the tagged synthAddons in order
    //static inline void addSyntheticVariable(std::vector<std::span<const float>> taggedVars_data, Dataset corresponding_result);
};

#endif /* Transfer_Manager_hpp */
