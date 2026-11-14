//
//  Transfer_Manager.hpp
//  Amethyne
//
//  Created by AnshuDiya on 11/9/26.
//
#pragma once


#include <unordered_map>
#include "Var_Storage.hpp"

class testingGround;


class testingTagManager{
    enum class managerStatus{
        cleared,
        createdTestTags,
        returnedTestTagPairs
        //add more if needed and also implement checks int he functions below later
    };inline static managerStatus statusOfTestTagManager;
    std::unordered_map<uint16_t, uint16_t> realTagsToTestTags;//key--real variable tag, value-- its test tag counterpart
    struct synthAndRealRelationTestPair{
    private:
        uint16_t realTag;//tagX
        uint16_t testTag;//tagY
    public:
        synthAndRealRelationTestPair(uint16_t assignedRealTag, uint16_t assignedSynthTestTag);//checks if the tag is even and odd for safety
        uint16_t returnX_Tag();
        uint16_t returnY_Tag();
    };
public:
    void createTestTags();//for every single currently existing variable, creates a test tag for them and stores it. clears previously stored tags.
    uint16_t realTagToTestTag(uint16_t realTag);//give a real tag and returns currently stored test tag for that real tag.
    uint16_t testSynthTagCounterpartOfRealTag(uint16_t realTag);//literally just hte realTag+1 since the real tag-test tag are only even indexes
    std::vector<synthAndRealRelationTestPair> returnRandomSynthTestPairTags();
};

struct tagCounters{
    int metric = 0;
    int action = 0;
    int synthetic = 0;
};
class tagAssigner{
    static inline tagCounters theTagCounter;
public:
    static testingTagManager testTagManager;
    static uint16_t assignTag(variableCategory typeOfVar);
};

//responsible for the bridge between testingGround and coldStorage as well as updating the symbolic variables while they're being transferred! also assings TAGS.
//how tags work is tag%3==0 tags are reserved for action variables
//tag%3==1 tags are reserved for metric variables
//and finally tag%3==2 tags are reserved for synthetic variables
//this way there are ZERO shared tags across variables! even synthetic addons meant for testing only contain tags as a reference to what existing variables they modify!
class TransferManager{
    class testingGroundsPasskey{
        friend class testingGround;
        testingGroundsPasskey() = default;
    };
public:
    static void transferAuthentic(std::variant<actionVar, metricVar> authenticVariable);
};

