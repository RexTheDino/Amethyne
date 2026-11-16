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


//doesnt generate any commands that need to be performed by emtal
void statusUpdaters::ExplorationUpdaters::generateSyntheticAddons(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData){
    subcommandsVec.clear();
    auto& tGround = Nucleus.testingGround;
    if((tGround.modeOfTestingGround != testingGroundMode::exploring)||(tGround.statusOfExploringTestingGround != testingGroundExploringStatus::empty)){
        throw std::runtime_error("Testing ground not in exploring mode but asked to perform a preprocessing step OR state has not been cleared out to empty yet!");
    }
    
    int numVarsToMake = varCreationMetadata::synthVarBatchLimit;//obviously change when global sources of info are taken down
    auto& instrVec = tGround.Data.morphModInstructions;
    addonGenPasskey derPasskey;//passkey required to access and generate addons
    synthVarGen::createAndWriteSynthVars(numVarsToMake, instrVec, derPasskey);
    std::span<synthVarAddon> spanOfInstrVec(instrVec.begin(), instrVec.end());
    
    //copies to staging buffer
    GPU_lieutenant::addCopyCommand(spanOfInstrVec, bufferTypes::PRIVATE_instructionBuffer);
    //appends data copy operation to subcommands vector
    subcommandsHelper::appendDataCopyOp<synthVarAddon>(spanOfInstrVec, bufferTypes::PRIVATE_instructionBuffer, subcommandsVec);
    
    groundsWatchdog::toggleStatus();
}


void statusUpdaters::ExplorationUpdaters::copyRealDataIntoTestTagSlots(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData){
    auto& tGround = Nucleus.testingGround;
    if((tGround.modeOfTestingGround != testingGroundMode::exploring)||(tGround.statusOfExploringTestingGround != testingGroundExploringStatus::PREPROCESSING_generatedSynthAddons)){
        throw std::runtime_error("Testing ground not in exploring mode but asked to perform a preprocessing step OR synthetic addons have not been generated yet!");
    }
    
    //copy over real variable data into the respective slots
    auto& testVarDat = Nucleus.testingGround.Data.testVarsStorage;
    int pointsPerVar = RelationConstructionMetadata::points_per_var;
    int maxTag = 0;
    std::vector<uint16_t> tagsOfAllVariables = Nucleus.coldStorage.returnALL_TAGS();
        for(int e = 0; e < tagsOfAllVariables.size(); e++){
            std::string programNameOfTaggedVar = synthetic_parser::varNameReturn(tagsOfAllVariables[e]);
            auto spanOfRealData = Nucleus.logsManager.returnDataOfRequestedTracker(programNameOfTaggedVar);
            int testTagOfData = tagAssigner::testTagManager.realTagToTestTag(tagsOfAllVariables[e]);
            
            int newPointsPerVar = (int)spanOfRealData.size();
            if(newPointsPerVar != pointsPerVar){throw std::runtime_error("Data sizes between logs do not match!!!");}
            
            
                                                    if(testVarDat.size() <= (pointsPerVar * testTagOfData)){testVarDat.resize((pointsPerVar * (testTagOfData +1)));}
            std::transform(spanOfRealData.begin(), spanOfRealData.end(), testVarDat.begin()+(pointsPerVar * testTagOfData) +1, [&](dataLog& dLog){
                return dLog.data;
            });
            if(testTagOfData > maxTag){maxTag = testTagOfData;}
            
            if(testTagOfData % 2 != 0){
                throw std::runtime_error("Real-test-tag counterpart NOT on an even index!!!");
            }
        }
    //doesnt need to be in referenceData since testVarDat is a permanent object
    std::span<float> spanOfDat(testVarDat.begin(), testVarDat.begin() + (pointsPerVar * (maxTag+1)));//span of the full range of data that had been written to
    
    //copies into the staging buffer
    GPU_lieutenant::addCopyCommand<float>(spanOfDat, bufferTypes::PRIVATE_inputDataBuffer);
    
    //appends the copy command that actually fires blit!
    subcommandsHelper::appendDataCopyOp<float>(spanOfDat, bufferTypes::PRIVATE_inputDataBuffer, subcommandsVec);//appends the subcommand that metal will process
    
    groundsWatchdog::toggleStatus();
}

//needs to genereate a metal comand to copy over metadata to metal buffer. specifically a dataCopyOp into a shared buffer
void statusUpdaters::ExplorationUpdaters::initializeMetadata(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData){
    
    
    auto& tGround = Nucleus.testingGround;
    if((tGround.modeOfTestingGround != testingGroundMode::exploring)||(tGround.statusOfExploringTestingGround != testingGroundExploringStatus::PREPROCESSING_realVarsInTestSlots)){
        throw std::runtime_error("Testing ground not in exploring mode but asked to perform a preprocessing step OR real var data has not been copied to test slots yet!");
    }
    
    MtdtStruct initializationStruct;
    initializationStruct.num_terms_per_synthetic = synthVarGen::currentAddonsPerTestSynth;
        uint16_t pointsPerVarAsMultipleOfSixtyFour = (uint16_t)RelationConstructionMetadata::points_per_var;
        if(pointsPerVarAsMultipleOfSixtyFour % 64 != 0){throw std::runtime_error("Points per variable not a multiple of 64!");}
    
    referenceData.initDatSpan = {initializationStruct};
    std::span<MtdtStruct> spanOfVec(referenceData.initDatSpan.begin(), referenceData.initDatSpan.end());
    
    //copy command to SHARED buffer
    GPU_lieutenant::addCopyCommand<MtdtStruct>(spanOfVec, bufferTypes::SHARED_initializerBuffer);
    
    groundsWatchdog::toggleStatus();
}

//just updates the subcommands vector to have a kernel dispatch subcommand
void statusUpdaters::ExplorationUpdaters::dispatchSynthPreprocessing(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData){
    
    int numThreadgroups = referenceData.numberOfBatchesOfSynthAddons;
    assert(numThreadgroups != -1);//-1 is the sentinel value. if its there it means the data hadnt been filled out yet
    int threadsPerGroup = 64;//hardcoded for now, change later if needed
    kernels typeOfKernel = kernels::synthetics_preprocessing;
    
    subcommandsHelper::appendKernelOp(typeOfKernel, numThreadgroups, threadsPerGroup, subcommandsVec);
    GPU_lieutenant::addMetalCommand(subcommandsVec);//before the copy instructions that follow below execute we want the kernel to execute.
    subcommandsVec.clear();
}

//remove this
//void statusUpdaters::ExplorationUpdaters::statusUpdaters::ExplorationUpdaters::queryPreprocessingStatus()-- need to implement

//remove this
/*void statusUpdaters::ExplorationUpdaters::copyPreprocessingResultsBackToTestStorage(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData){
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
}*/

void statusUpdaters::ExplorationUpdaters::initializeDerivedDiffsMtdt(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData){
    auto& tGround = Nucleus.testingGround;
    if((tGround.modeOfTestingGround != testingGroundMode::exploring)||(tGround.statusOfExploringTestingGround != testingGroundExploringStatus::PREPROCESSING_resultsCopiedBackToStorage)){
        throw std::runtime_error("Testing ground not in exploring mode but asked to perform a preprocessing step OR real var data has not been copied to test slots yet!");
    }
    
    auto addons = tagAssigner::testTagManager.returnRandomSynthTestPairTags();//change this so it only returns grounded tags. also metrics can only be part of the y-tag when paired with an x-tag thats grounded to an action variable.
                            referenceData.numTagPairsToTest = addons.size();//important for the dispatcher function, it needs to know this
    referenceData.relationTestTags.resize(2+(addons.size()*2));
    
    for(int w = 0; w < addons.size(); w++){
        //doing w*2 since relation test tgas is a regular uint16_t vector whereas each element of addons contains a pair.
        referenceData.relationTestTags[w*2] = addons[w].returnX_Tag();//packed_float2.x to contain x/independent variable. in this case the real var.
        referenceData.relationTestTags[(w*2)+1] = addons[w].returnY_Tag();//packed_float2.y to contain y/dependent variable. in this case the synth-test var.
    }
    
    std::span<uint16_t> spanOfDat(referenceData.relationTestTags.begin(), referenceData.relationTestTags.end());
    //clear previous instructions from synthetics preprocessing
    GPU_lieutenant::addCopyCommand<uint16_t>(spanOfDat, bufferTypes::CLEAR_PRIVATE_instructionBuffer);
    //add tags to the private instruction staging buffer
    GPU_lieutenant::addCopyCommand<uint16_t>(spanOfDat, bufferTypes::PRIVATE_instructionBuffer);
    //queue command to later copy the private instructon staging buffer data to the private metal buffer
    subcommandsHelper::appendDataCopyOp<uint16_t>(spanOfDat, bufferTypes::PRIVATE_instructionBuffer, subcommandsVec);
}

void statusUpdaters::ExplorationUpdaters::dispatchRelationEstimation(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData){
    int numThreadsPerGroup = 64;//hardcoded for now, change if needed
    int numberOfThreadgroups = referenceData.numTagPairsToTest;//num pairs of tags is the number of threadgroups!
        subcommandsHelper::appendKernelOp(kernels::relationEstimation, numberOfThreadgroups, numThreadsPerGroup, subcommandsVec);
}

//useless cuz dispatcher will wake this thread up when metal is finished
//void statusUpdaters::ExplorationUpdaters::statusUpdaters::ExplorationUpdaters::queryPostProcessingResultStatus()-- need to implement

using namespace std;
void statusUpdaters::ExplorationUpdaters::copyPostProcessingResultsToDatasetsVec(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData){
    auto& tGround = Nucleus.testingGround;
    if((tGround.modeOfTestingGround != testingGroundMode::exploring)||(tGround.statusOfExploringTestingGround != testingGroundExploringStatus::FINISHED_POSTPROCESSING)){
        throw std::runtime_error("Testing ground not in exploring mode but asked to perform a preprocessing step OR real var data has not been copied to test slots yet!");
    }
    
    int expectedResultsSize = referenceData.numTagPairsToTest;//number of datasets that could be output is the # of pairs that were tested.
                                                assert(expectedResultsSize >= 1);
        auto& resultVec = tGround.Data.results;
    std::span<Dataset> spanOfResults(resultVec.begin(), resultVec.begin() + expectedResultsSize - 1);
    subcommandsHelper::appendDataCopyOp<Dataset>(spanOfResults, bufferTypes::PRIVATE_outputBuffer, subcommandsVec);//data copy op to copy back results staging buffer
    
    GPU_lieutenant::addMetalCommand(subcommandsVec);//adds the command to execute everything
    
   //add command to copy from private output staging buffer to results vector in data
    GPU_lieutenant::addCopyCommand<Dataset>(spanOfResults, bufferTypes::PRIVATE_outputBuffer);
    
    //commands still need to be fired...
    
    groundsWatchdog::toggleStatus();
}

//void statusUpdaters::ExplorationUpdaters::statusUpdaters::ExplorationUpdaters::clearStorage()-- need to implement



void statusUpdaters::analyzationUpdaters::copyVariableDataIntoTestSlots(std::vector<subcommands>& subcommandsVec){
    
}
