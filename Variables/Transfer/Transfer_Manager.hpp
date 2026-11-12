//
//  Transfer_Manager.hpp
//  Amethyne
//
//  Created by AnshuDiya on 11/9/26.
//
#pragma once

#ifndef Transfer_Manager_hpp
#define Transfer_Manager_hpp

#include "Relations_Header.hpp"
#include <unordered_map>

class testingTagManager{
    std::unordered_map<uint16_t, uint16_t> realTagsToTestTags;//key--real variable tag, value-- its test tag counterpart
public:
    inline void createTestTags();//for every single currently existing variable, creates a test tag for them and stores it. clears previously stored tags.
    inline int realTagToTestTag(uint16_t realTag);//give a real tag and returns currently stored test tag.
    inline int testSynthTagCounterpartOfRealTag(uint16_t realTag);//literally just hte realTag+1 since the real tag-test tag are only even indexes
};

class tagAssigner{
    struct tagCounters{
        static int metric;
        static int action;
        static int synthetic;
    };
public:
    static testingTagManager testTagManager;
};

//responsible for the bridge between testingGround and coldStorage as well as updating the symbolic variables while they're being transferred! also assings TAGS.
//how tags work is tag%3==0 tags are reserved for action variables
//tag%3==1 tags are reserved for metric variables
//and finally tag%3==2 tags are reserved for synthetic variables
//this way there are ZERO shared tags across variables! even synthetic addons meant for testing only contain tags as a reference to what existing variables they modify!
class TransferManager{
    //std::span view of all the data referenced by the tagged synthAddons in order
    //static inline void addSyntheticVariable(std::vector<std::span<const float>> taggedVars_data, Dataset corresponding_result);
};

#endif /* Transfer_Manager_hpp */
