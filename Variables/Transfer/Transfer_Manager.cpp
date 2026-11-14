//
//  Transfer_Manager.cpp
//  Amethyne
//
//  Created by AnshuDiya on 11/9/26.
//


#include "Transfer_Manager.hpp"
#include "Var_Storage.hpp"
#include "Initialization_Data_header.hpp"
#include "Metadata.h"

testingTagManager::synthAndRealRelationTestPair::synthAndRealRelationTestPair(uint16_t assignedRealTag, uint16_t assignedSynthTestTag){
    if((assignedRealTag % 2 != 0)||(assignedSynthTestTag % 2 != 1)){
        throw std::invalid_argument("Real-tester-tag is NOT even OR synth tester tag is NOT odd!");
    }
    
}

void testingTagManager::createTestTags(){
    assert(statusOfTestTagManager == managerStatus::returnedTestTagPairs);//needs to have returned test pairs before this step is taken
    
    realTagsToTestTags.clear();//clears all previous vals
            statusOfTestTagManager = managerStatus::cleared;//is now freshly reset
    
    std::vector<uint16_t> everyTag = coldStorage::returnALL_TAGS();
    std::sort(everyTag.begin(), everyTag.end(), [](uint16_t& tag1, uint16_t& tag2){//sorts vec of tags in ascending order
        return tag1 < tag2;
    });
    for(int a = 0; a < everyTag.size(); a++){
        int modified_even_only_Tag = (a*2);//only even tags
        realTagsToTestTags[everyTag[a]] = modified_even_only_Tag;
    }
    
    statusOfTestTagManager = managerStatus::createdTestTags;
}

uint16_t testingTagManager::realTagToTestTag(uint16_t realTag){
    auto returnValIterator = realTagsToTestTags.find(realTag);
    return (returnValIterator != realTagsToTestTags.end()) ? returnValIterator->second : throw std::runtime_error("Tag-value pair does not exist!");
}

uint16_t testingTagManager::testSynthTagCounterpartOfRealTag(uint16_t realTag){return realTag+1;}

std::vector<testingTagManager::synthAndRealRelationTestPair> testingTagManager::returnRandomSynthTestPairTags(){
    assert(statusOfTestTagManager == managerStatus::createdTestTags);
    
    std::vector<testingTagManager::synthAndRealRelationTestPair> resultVec;
    
    int sizeOfMap = (int)realTagsToTestTags.size();
        resultVec.reserve(sizeOfMap);
    auto iteratorToFirstVal = realTagsToTestTags.begin();
    std::uniform_int_distribution<uint16_t> randomRealTestTagGen(0, sizeOfMap-1);
    std::uniform_int_distribution<uint16_t> randomSynthTesterTagGen(0, varCreationMetadata::synthVarBatchLimit);
    
    for(int w = 0; w < sizeOfMap; w++){
        auto iteratorToRandomVal = std::next(iteratorToFirstVal, randomRealTestTagGen(Amethyne_rand));
        int randomTesterSynthTag = (randomSynthTesterTagGen(Amethyne_rand) * 2)+1;//*2 +1 to convert the synth id tag to a odd index!
        testingTagManager::synthAndRealRelationTestPair newTestPair(iteratorToRandomVal->first, randomTesterSynthTag);
        resultVec.push_back(newTestPair);
    }
    
    statusOfTestTagManager = managerStatus::returnedTestTagPairs;
    
    return resultVec;
}

