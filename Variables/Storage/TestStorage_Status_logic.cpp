//
//  TestStorage_Status_logic.cpp
//  Amethyne
//
//  Created by AnshuDiya on 11/12/26.
//

#include "TestStorage_Status_logic.hpp"
#include "Metadata.h"
#include "SynthAddon_gen.hpp"
#include "Data_logs.hpp"
#include "Transfer_Manager.hpp"
#include "Nucleus.hpp"

void groundsWatchdog::setTestingGroundMode(testingGroundMode modeToSet){
    auto& tGround = Nucleus.testingGround;
    if(tGround.statusOfExploringTestingGround != testingGroundExploringStatus::empty){
    if((modeToSet == testingGroundMode::exploring)&&(tGround.statusOfExploringTestingGround != testingGroundExploringStatus::POSTPROCESSING_solidified)){
        throw std::runtime_error("Testing grounds modes are being changed before it has finished processing data!!!");
    }
       }//else if--add conditional checks to check for vice versa, if its being switched away frome xploring without real data having solidified
    
    tGround.modeOfTestingGround = modeToSet;
}

void groundsWatchdog::toggleStatus(){
    auto& tGround = Nucleus.testingGround;
    if(tGround.modeOfTestingGround == testingGroundMode::exploring){
        int explorerStatus = static_cast<int>(tGround.statusOfExploringTestingGround);
        int maxExplorerStatus = static_cast<int>(testingGroundExploringStatus::counter);
        explorerStatus == maxExplorerStatus-1 ? explorerStatus = 1 : explorerStatus++;//if at max explorer status/ a.k.a. the one before counter reset back to the first state
        tGround.statusOfExploringTestingGround = static_cast<testingGroundExploringStatus>(explorerStatus);
    }else if(tGround.modeOfTestingGround == testingGroundMode::analyzing){
        int analyzationStatus = static_cast<int>(tGround.statusOfAnalyzingTestingGround);
        int maxAnalyzerStatus = static_cast<int>(testingGroundAnalyzingStatus::counter);
        analyzationStatus == maxAnalyzerStatus -1 ? analyzationStatus = 1 : analyzationStatus++;//if at max analyzer status/ a.k.a. the one before counter reset back to the first state
    }
}
testingGroundExploringStatus groundsWatchdog::queryExploringStatus(){return Nucleus.testingGround.statusOfExploringTestingGround;}



void statusUpdaters::ExplorationUpdaters::generateSyntheticAddons(){
    auto& tGround = Nucleus.testingGround;
    if((tGround.modeOfTestingGround != testingGroundMode::exploring)||(tGround.statusOfExploringTestingGround != testingGroundExploringStatus::empty)){
        throw std::runtime_error("Testing ground not in exploring mode but asked to perform a preprocessing step OR state has not been cleared out to empty yet!");
    }
    
    int numVarsToMake = varCreationMetadata::synthVarBatchLimit;//obviously change when global sources of info are taken down
    auto& instrVec = tGround.Data.morphModInstructions;
    addonGenPasskey derPasskey;//passkey required to access and generate addons
    synthVarGen::createAndWriteSynthVars(numVarsToMake, instrVec, derPasskey);
    
    groundsWatchdog::toggleStatus();
}


int statusUpdaters::ExplorationUpdaters::copyRealDataIntoTestTagSlots(){
    auto& tGround = Nucleus.testingGround;
    if((tGround.modeOfTestingGround != testingGroundMode::exploring)||(tGround.statusOfExploringTestingGround != testingGroundExploringStatus::PREPROCESSING_generatedSynthAddons)){
        throw std::runtime_error("Testing ground not in exploring mode but asked to perform a preprocessing step OR synthetic addons have not been generated yet!");
    }
    
    int pointsPerVar = RelationConstructionMetadata::points_per_var;
    int maxTag = 0;
    std::vector<uint16_t> tagsOfAllVariables = Nucleus.coldStorage.returnALL_TAGS();
        for(int e = 0; e < tagsOfAllVariables.size(); e++){
            std::string programNameOfTaggedVar = synthetic_parser::varNameReturn(tagsOfAllVariables[e]);
            auto spanOfRealData = Nucleus.logsManager.returnDataOfRequestedTracker(programNameOfTaggedVar);
            int testTagOfData = tagAssigner::testTagManager.realTagToTestTag(tagsOfAllVariables[e]);
            
            int newPointsPerVar = (int)spanOfRealData.size();
            if(newPointsPerVar != pointsPerVar){throw std::runtime_error("Data sizes between logs do not match!!!");}
            
            //copy over real variable data into the respective slots
            auto& testVarDat = Nucleus.testingGround.Data.testVarsStorage;
                                                    if(testVarDat.size() <= (pointsPerVar * testTagOfData)){testVarDat.resize((pointsPerVar * testTagOfData)+1);}
            std::transform(spanOfRealData.begin(), spanOfRealData.end(), testVarDat.begin()+(pointsPerVar * testTagOfData), [&](dataLog& dLog){
                return dLog.data;
            });
            if(testTagOfData > maxTag){maxTag = testTagOfData;}
            
            if(testTagOfData % 2 != 0){
                throw std::runtime_error("Real-test-tag counterpart NOT on an even index!!!");
            }
        }
    
    groundsWatchdog::toggleStatus();
    
    //maxTag*pointsPerVar represents the start index of the final real tag. +1 the start of its synthetic tester counterpart's results. +2 the element next to the end of the last synthetic tester index
    int writeBackIndex = (maxTag+2)*pointsPerVar;
    return writeBackIndex;
}


void statusUpdaters::ExplorationUpdaters::initializeMetadata(MtdtStruct* ptrToMetalMetadataBuffer){
    auto& tGround = Nucleus.testingGround;
    if((tGround.modeOfTestingGround != testingGroundMode::exploring)||(tGround.statusOfExploringTestingGround != testingGroundExploringStatus::PREPROCESSING_realVarsInTestSlots)){
        throw std::runtime_error("Testing ground not in exploring mode but asked to perform a preprocessing step OR real var data has not been copied to test slots yet!");
    }
    
    MtdtStruct initializationStruct;
    initializationStruct.num_terms_per_synthetic = synthVarGen::currentAddonsPerTestSynth;
    uint16_t pointsPerVarAsMultipleOfSixtyFour = (uint16_t)RelationConstructionMetadata::points_per_var;
    if(pointsPerVarAsMultipleOfSixtyFour % 64 != 0){throw std::runtime_error("Points per variable not a multiple of 64!");}
    initializationStruct.pointsPerVar_as_multiple_of_64 = pointsPerVarAsMultipleOfSixtyFour/64;
        ptrToMetalMetadataBuffer[0] = initializationStruct;
    
    groundsWatchdog::toggleStatus();
}

//void statusUpdaters::ExplorationUpdaters::statusUpdaters::ExplorationUpdaters::dispatchSynthPreprocessing()-- need to implement

//void statusUpdaters::ExplorationUpdaters::statusUpdaters::ExplorationUpdaters::queryPreprocessingStatus()-- need to implement

void statusUpdaters::ExplorationUpdaters::copyPreprocessingResultsBackToTestStorage(float* ptrToMetalBuffer){
    auto& tGround = Nucleus.testingGround;
    if((tGround.modeOfTestingGround != testingGroundMode::exploring)||(tGround.statusOfExploringTestingGround != testingGroundExploringStatus::FINISHED_PREPROCESSING)){
        throw std::runtime_error("Testing ground not in exploring mode but asked to perform a preprocessing step OR real var data has not been copied to test slots yet!");
    }
    
    int numPtsPerVar = RelationConstructionMetadata::points_per_var;
    int synthBatchSize = varCreationMetadata::synthVarBatchLimit;
    for(int w = 0; w < synthBatchSize; w++){
        //copy data thats present on the odd indexes in the metal results buffer to the var storage odd indexes for subsequent processing
        //ptrToMetalBuffer+(((w*2)+1) * numPtsPerVar)+numPtsPerVar-1 represents the end/last index of the target slot
        std::copy(ptrToMetalBuffer+(((w*2)+1) * numPtsPerVar), ptrToMetalBuffer+(((w*2)+1) * numPtsPerVar)+numPtsPerVar-1, tGround.Data.testVarsStorage.begin()+(((w * 2)+1) * numPtsPerVar));
    }
}

void statusUpdaters::ExplorationUpdaters::initializeDerivedDiffsMtdt(float *ptrToMetalDerivedDiffInitBuffer){
    auto& tGround = Nucleus.testingGround;
    if((tGround.modeOfTestingGround != testingGroundMode::exploring)||(tGround.statusOfExploringTestingGround != testingGroundExploringStatus::PREPROCESSING_resultsCopiedBackToStorage)){
        throw std::runtime_error("Testing ground not in exploring mode but asked to perform a preprocessing step OR real var data has not been copied to test slots yet!");
    }
    
    auto addons = tagAssigner::testTagManager.returnRandomSynthTestPairTags();
    ptrToMetalDerivedDiffInitBuffer[0] = RelationConstructionMetadata::points_per_var;//element 0 to contain the points per var for kernel to use
    
    for(int w = 0; w < addons.size(); w++){
        ptrToMetalDerivedDiffInitBuffer[2+(w*2)] = addons[w].returnX_Tag();//packed_float2.x to contain x/independent variable. in this case the real var.
        ptrToMetalDerivedDiffInitBuffer[2+(w*2)+1] = addons[w].returnY_Tag();//packed_float2.y to contain y/dependent variable. in this case the synth-test var.
    }
}

//void statusUpdaters::ExplorationUpdaters::statusUpdaters::ExplorationUpdaters::dispatchRelationEstimation()-- need to implement

//void statusUpdaters::ExplorationUpdaters::statusUpdaters::ExplorationUpdaters::queryPostProcessingResultStatus()-- need to implement

using namespace std;
//kernel alreayd copies back data into the format derived diffs needs to execute so this is unnecessary
/*void statusUpdaters::ExplorationUpdaters::copyPostProcessingResultsToDatasetsVec(Dataset* ptrToMetalResultsBuffer, int numTestedRelations){
    if((testingGround::modeOfTestingGround != testingGroundMode::exploring)||(testingGround::statusOfExploringTestingGround != testingGroundExploringStatus::FINISHED_POSTPROCESSING)){
        throw std::runtime_error("Testing ground not in exploring mode but asked to perform a preprocessing step OR real var data has not been copied to test slots yet!");
    }
    
    //copy(&ptrToMetalResultsBuffer[0], &ptrToMetalResultsBuffer[numTestedRelations-1], Data::results.begin());
    testingGround::Data::results.assign(ptrToMetalResultsBuffer, ptrToMetalResultsBuffer + numTestedRelations);
    
    groundsWatchdog::toggleStatus();
}*/

//void statusUpdaters::ExplorationUpdaters::statusUpdaters::ExplorationUpdaters::clearStorage()-- need to implement



void statusUpdaters::analyzationUpdaters::copyVariableDataIntoTestSlots(){
    
}
