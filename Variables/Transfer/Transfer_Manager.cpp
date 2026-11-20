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
#include "Nucleus.hpp"
#include "TestStorage_Status_logic.hpp"

testingTagManager::synthAndRealRelationTestPair::synthAndRealRelationTestPair(uint16_t assignedRealTag, uint16_t assignedSynthTestTag){
    if((assignedRealTag % 2 != 0)||(assignedSynthTestTag % 2 != 1)){
        throw std::invalid_argument("Real-tester-tag is NOT even OR synth tester tag is NOT odd!");
    }
    realTag = assignedRealTag;
    testTag = assignedSynthTestTag;
}

uint16_t testingTagManager::synthAndRealRelationTestPair::returnX_Tag(){return realTag;}
uint16_t testingTagManager::synthAndRealRelationTestPair::returnY_Tag(){return testTag;}

void testingTagManager::createTestTags(){
    assert(statusOfTestTagManager == managerStatus::cleared);//needs to have returned test pairs before this step is taken
    
    realTagsToTestTags.clear();//clears all previous vals
            statusOfTestTagManager = managerStatus::cleared;//is now freshly reset
    
    std::vector<uint16_t> everyTag = Nucleus.coldStorage.returnALL_TAGS();
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

std::unordered_map<uint16_t, uint16_t> testingTagManager::mapOftestTagsToRealTags(){
    
    assert(realTagsToTestTags.size() > 0);//cant have been cleared when this step is performed
    std::unordered_map<uint16_t, uint16_t> returnMap; returnMap.reserve(realTagsToTestTags.size());
    for(const auto& [key, value]: realTagsToTestTags){
        returnMap[value] = key;
    }
    
    return returnMap;
}


std::vector<testingTagManager::synthAndRealRelationTestPair> testingTagManager::returnRandomSynthTestPairTags(){
    assert(statusOfTestTagManager == managerStatus::createdTestTags);
    
    std::vector<testingTagManager::synthAndRealRelationTestPair> resultVec;
    
    int sizeOfMap = (int)realTagsToTestTags.size();
    int totalNumTags = varCreationMetadata::synthVarBatchLimit * sizeOfMap;
                                                                            if((totalNumTags) >= 100000){totalNumTags = 100000;}
        resultVec.reserve(totalNumTags);
    auto iteratorToFirstVal = realTagsToTestTags.begin();
    std::uniform_int_distribution<uint16_t> randomRealTestTagGen(0, sizeOfMap-1);
    std::uniform_int_distribution<uint16_t> randomSynthTesterTagGen(0, varCreationMetadata::synthVarBatchLimit-1);//doing -1 since starting count from 0.
    
    for(int w = 0; w < totalNumTags; w++){//sizeOfMap*batchLimit <= 10000 number of pairs
        auto iteratorToRandomVal = std::next(iteratorToFirstVal, randomRealTestTagGen(Amethyne_rand));
            int randomTesterSynthTag = (randomSynthTesterTagGen(Amethyne_rand) * 2)+1;//*2 +1 to convert the synth id tag to a odd index!
            testingTagManager::synthAndRealRelationTestPair newTestPair(iteratorToRandomVal->second, randomTesterSynthTag);//y-tag is synth, x-tag is real so its GROUNDED!
            resultVec.push_back(newTestPair);
    }
    
    statusOfTestTagManager = managerStatus::cleared;
    
    return resultVec;
}

uint16_t testingTagManager::convertTestSynthTagToTagCorrespondingPositionInBatch(uint16_t tag){
    assert(tag % 2 == 1);//synth tester tag has to be an odd number
    int result = (tag-1)/2;//its position in the synth var test batch
    return result;
}

uint16_t tagAssigner::assignTag(variableCategory typeOfVar){
    switch (typeOfVar) {
        case variableCategory::action:
            theTagCounter.action++;
            return theTagCounter.action*3;//since actionTag%3 == 0
            break;
        case variableCategory::metric:
            theTagCounter.metric++;
            return (theTagCounter.metric*3)+1;//since metric is metricTag%3 == 1
            break;
        case variableCategory::synthetic:
            theTagCounter.synthetic++;
            return (theTagCounter.synthetic*3)+2;//since syntheticTag%3 == 2
            break;
            
        default:throw std::invalid_argument("Enum value for action/metric/synthetic variable does not fall within a valid range!");
            break;
    }
}



void Transfer_Manager::transferTestingResultsToColdStorage(){
    auto& testingGround = Nucleus.testingGround;
    
    auto passkey = TESTING_GROUND::passkeyForDatasetTransfer();
    auto groundTestResults = testingGround.returnTestingResults(passkey);//returns vector of sturcts each containing a variable and a span of that variable's datasets.
    auto& cStorage = Nucleus.coldStorage;
    for(auto g = groundTestResults.begin(); g != groundTestResults.end(); g++){
        if(!g->correspondingAddons.empty()){//is a test synth thats being dealt with
            
            std::vector<permanentSynthAddon> addonsForSynthetic; addonsForSynthetic.reserve(g->correspondingAddons.size());
            for(int f = 0; f < g->correspondingAddons.size(); f++){
                permanentSynthAddon::linkage linkageOfPermAddon;
                
                if(cStorage.returnVarType_givenTag(g->correspondingAddons[f].returnTag()) == variableCategory::synthetic){
                    linkageOfPermAddon = permanentSynthAddon::linkage::linkedToSynthetic;
                }else{
                    linkageOfPermAddon = permanentSynthAddon::linkage::linkedToAuthentic;
                }
                
                const permanentSynthAddon addonToAppend = permanentSynthAddon(linkageOfPermAddon, g->correspondingAddons[f]);
                addonsForSynthetic.push_back(addonToAppend);
            }
            
            int tagToAssign = tagAssigner::assignTag(variableCategory::synthetic);
            std::string progNameofSynth = synthetic_parser::varNameReturn(tagToAssign);
            std::span<float> synthData = g->spanOfCorrespondingSynthData; assert(synthData.size() > 0);//shouldnt have been cleared by now. logs manager will immediately log data.
            synthVar newSynthVar = synthVar(tagToAssign, addonsForSynthetic, progNameofSynth);
            auto syntheticPasskey = cold_Storage::SYNTH_VAR_MANAGER::addSyntheticPasskey();
            cStorage.synthVarManager.addSyntheticVariable(newSynthVar, syntheticPasskey);
            Nucleus.logsManager.addSyntheticTracker(newSynthVar.returnTag(), synthData);
        }
        
        auto dSetPasskey = cold_Storage::TransferManagerPasskey();
        for(int u = 0; u < g->datasetsCorrespondingToVar.size(); u++){
            if(g->datasetsCorrespondingToVar[u].returnRelation() != RelationshipType::null){//only add a dataset that isnt null
                assert(static_cast<uint16_t>(g->datasetsCorrespondingToVar[u].returnRelation()) < static_cast<uint16_t>(RelationshipType::sentinel));//no garbage vals
                
                auto dSetPasskey = cold_Storage::TransferManagerPasskey();
                cStorage.addDataset(g->datasetsCorrespondingToVar[u], dSetPasskey);
            }
        }
    }
    
}

void Transfer_Manager::transferAuthentic(std::variant<actionVar, metricVar> authenticVariable){
    cold_Storage::TransferManagerPasskey passkey;
    if(std::holds_alternative<actionVar>(authenticVariable)){
        actionVar theActionVar = std::get<actionVar>(authenticVariable);
        Nucleus.coldStorage.addActionVar(passkey, theActionVar);
            Nucleus.logsManager.addDataTracker(theActionVar.return_program_name());
    }else if(std::holds_alternative<metricVar>(authenticVariable)){
        metricVar theMetricVar = std::get<metricVar>(authenticVariable);
        Nucleus.coldStorage.addMetricVar(passkey, theMetricVar);
            Nucleus.logsManager.addDataTracker(theMetricVar.return_program_name());
    }else{throw std::invalid_argument("Variant doesnt contain an authentic!");}
}
