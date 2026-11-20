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
    numAddonsPerTestSynth batchSize = synthVarGen::createAndWriteSynthVars(numVarsToMake, instrVec, derPasskey);
    referenceData.numberOfBatchesOfSynthAddons = (int)instrVec.size()/batchSize.numAddons;
    assert(instrVec.size() % batchSize.numAddons == 0);
    std::span<synthVarAddon> spanOfInstrVec(instrVec.begin(), instrVec.end());
    
    //copies to staging buffer
    GPU_lieutenant::addCopyCommand(spanOfInstrVec, bufferTypes::PRIVATE_instructionBuffer);
    //appends data copy operation to subcommands vector
    subcommandsHelper::appendDataCopyOp<synthVarAddon>(spanOfInstrVec, bufferTypes::PRIVATE_instructionBuffer, subcommandsVec);
    
    groundsWatchdog::toggleStatus();
}

//RELATIONSHIPS ARE RGGED, DO NOT CALL AFTER FUNC ABOVE
void statusUpdaters::ExplorationUpdaters::GENERATE_RIGGED_SYNTHETIC_ADDONS(std::vector<subcommands> &subcommandsVec, explorationReference &referenceData){
    
        subcommandsVec.clear();
        auto& tGround = Nucleus.testingGround;
        if((tGround.modeOfTestingGround != testingGroundMode::exploring)||(tGround.statusOfExploringTestingGround != testingGroundExploringStatus::empty)){
            throw std::runtime_error("RIGGED FUNCTION COMBINED WITH REGULAR FUNCTION!");
        }
        
        int numVarsToMake = varCreationMetadata::synthVarBatchLimit;//obviously change when global sources of info are taken down
        auto& instrVec = tGround.Data.morphModInstructions;
        addonGenPasskey derPasskey;//passkey required to access and generate addons
    referenceData.RIGGED_REFERENCE.riggedInfo = synthVarGen::createAndWrite_RIGGED_synthVars(numVarsToMake, instrVec, derPasskey);
        referenceData.numberOfBatchesOfSynthAddons = (int)instrVec.size()/referenceData.RIGGED_REFERENCE.riggedInfo.numAddons.numAddons;
        assert(instrVec.size() % referenceData.RIGGED_REFERENCE.riggedInfo.numAddons.numAddons == 0);
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
            std::transform(spanOfRealData.begin(), spanOfRealData.end(), testVarDat.begin()+(pointsPerVar * testTagOfData) , [&](dataLog& dLog){
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
    
    std::vector<subcommands> preprocessingCopyBackCommand;//needs to be a seperate subcommand so that it executes AFTER the metal dispatch command!!!
    int expectedResultsSize = (numThreadgroups +1) * RelationConstructionMetadata::points_per_var * 2;//number of points expected as results
    std::span<float> expectedResultSpan(Nucleus.testingGround.Data.testVarsStorage.begin(), expectedResultsSize);//span of the data expected to be filled as results
    subcommandsHelper::appendDataCopyOp(expectedResultSpan, bufferTypes::PRIVATE_inputDataBuffer, preprocessingCopyBackCommand, true);//TRUE TO REVERSE COPY SYNTH PROCESSING DATA BACK!
    
    GPU_lieutenant::addMetalCommand(preprocessingCopyBackCommand);//copy back preprocessing results!!!
    
    GPU_lieutenant::addCopyCommand(expectedResultSpan, bufferTypes::PRIVATE_inputDataBuffer, true);//THE TRUE INITIATES REVERSE COPY OF THE PROCESSED SYNTHS BACK!!!
    
    groundsWatchdog::toggleStatus();
}

void statusUpdaters::ExplorationUpdaters::initializeDerivedDiffsMtdt(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData){
    auto& tGround = Nucleus.testingGround;
    if((tGround.modeOfTestingGround != testingGroundMode::exploring)||(tGround.statusOfExploringTestingGround != testingGroundExploringStatus::PREPROCESSING_initializedDispatch)){
        throw std::runtime_error("Testing ground not in exploring mode but asked to perform a preprocessing step OR real var data has not been copied to test slots yet!");
    }
    
    auto testTagPairs = tagAssigner::testTagManager.returnRandomSynthTestPairTags();//change this so it only returns grounded tags. also metrics can only be part of the y-tag when paired with an x-tag thats grounded to an action variable.
                            referenceData.numTagPairsToTest = testTagPairs.size();//important for the dispatcher function, it needs to know this
    referenceData.relationTestTags.resize(2+(testTagPairs.size()*2));
    
    for(int w = 0; w < testTagPairs.size(); w++){
        //doing w*2 since relation test tgas is a regular uint16_t vector whereas each element of addons contains a pair.
        referenceData.relationTestTags[w*2] = testTagPairs[w].returnX_Tag();//packed_float2.x to contain x/independent variable. in this case the real var.
        referenceData.relationTestTags[(w*2)+1] = testTagPairs[w].returnY_Tag();//packed_float2.y to contain y/dependent variable. in this case the synth-test var.
    }
    
    std::span<uint16_t> spanOfDat(referenceData.relationTestTags.begin(), referenceData.relationTestTags.end());
    //clear previous instructions from synthetics preprocessing
    GPU_lieutenant::addCopyCommand<uint16_t>(spanOfDat, bufferTypes::CLEAR_PRIVATE_instructionBuffer);
    //add tags to the private instruction staging buffer
    GPU_lieutenant::addCopyCommand<uint16_t>(spanOfDat, bufferTypes::PRIVATE_instructionBuffer);
    //queue command to later copy the private instructon staging buffer data to the private metal buffer
    subcommandsHelper::appendDataCopyOp<uint16_t>(spanOfDat, bufferTypes::PRIVATE_instructionBuffer, subcommandsVec);
    
    groundsWatchdog::toggleStatus();
}

void statusUpdaters::ExplorationUpdaters::initializeRIGGED_DERIVED_DIFFS_MTDT(std::vector<subcommands> &subcommandsVec, explorationReference &referenceData){
    auto& tGround = Nucleus.testingGround;
    if((tGround.modeOfTestingGround != testingGroundMode::exploring)||(tGround.statusOfExploringTestingGround != testingGroundExploringStatus::PREPROCESSING_initializedDispatch)){
        throw std::runtime_error("RIGGED DERIVED DIFFS FUNC COMBINED WITH REGULAR FUNC!");
    }
    
    auto& testTagPairs = referenceData.RIGGED_REFERENCE.riggedTagPairs;//vector of rigged tags

    auto& synthAddons = tGround.Data.morphModInstructions;//should be filled with synth addons by now
    //the batches of addons should be a multiple of the btach limit
    assert(referenceData.numberOfBatchesOfSynthAddons % varCreationMetadata::synthVarBatchLimit == 0);
        int batchSize = (int)synthAddons.size() / varCreationMetadata::synthVarBatchLimit;
                        assert(batchSize == 2);//for now its hardcoded to 2 rigged addons so check if thats true
    
    testTagPairs.clear();testTagPairs.reserve(varCreationMetadata::synthVarBatchLimit);
    for(int u = 0; u < varCreationMetadata::synthVarBatchLimit; u++){
        RIGGED_TAG_PAIR tagPair;
        tagPair.realTag = synthAddons[u*batchSize].returnTag();//the test tag of the real variable the addon is based on
        tagPair.testTag = (tagPair.realTag*2)+1;//since the results of the calculated synth woudlve been written to an odd slot calculated as (threadgroup_id * 2)+1 where threadgroup_id also corresponds to the batch of synthAddons calculated for
        testTagPairs.push_back(tagPair);
    }
    referenceData.numTagPairsToTest = testTagPairs.size();//important for the dispatcher function, it needs to know this
    referenceData.relationTestTags.resize(2+(testTagPairs.size()*2));
    
    for(int w = 0; w < testTagPairs.size(); w++){//((w*2) +1) == y-tag, (w*2) == xTag
        //doing w*2 since relation test tgas is a regular uint16_t vector whereas each element of addons contains a pair.
        referenceData.relationTestTags[w*2] = testTagPairs[w].realTag;//the real test-tag.
        referenceData.relationTestTags[(w*2)+1] = testTagPairs[w].testTag;//the synths's tag
    }
    
    std::span<uint16_t> spanOfDat(referenceData.relationTestTags.begin(), referenceData.relationTestTags.end());
    //clear previous instructions from synthetics preprocessing
    GPU_lieutenant::addCopyCommand<uint16_t>(spanOfDat, bufferTypes::CLEAR_PRIVATE_instructionBuffer);
    //add tags to the private instruction staging buffer
    GPU_lieutenant::addCopyCommand<uint16_t>(spanOfDat, bufferTypes::PRIVATE_instructionBuffer);
    //queue command to later copy the private instructon staging buffer data to the private metal buffer
    subcommandsHelper::appendDataCopyOp<uint16_t>(spanOfDat, bufferTypes::PRIVATE_instructionBuffer, subcommandsVec);
    
    groundsWatchdog::toggleStatus();
}

void statusUpdaters::ExplorationUpdaters::dispatchRelationEstimation(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData){
    int numThreadsPerGroup = 64;//hardcoded for now, change if needed
    int numberOfThreadgroups = referenceData.numTagPairsToTest;//num pairs of tags is the number of threadgroups!
    
        subcommandsHelper::appendKernelOp(kernels::relationEstimation, numberOfThreadgroups, numThreadsPerGroup, subcommandsVec);
    
    groundsWatchdog::toggleStatus();
}

//useless cuz dispatcher will wake this thread up when metal is finished
//void statusUpdaters::ExplorationUpdaters::statusUpdaters::ExplorationUpdaters::queryPostProcessingResultStatus()-- need to implement

using namespace std;
void statusUpdaters::ExplorationUpdaters::copyPostProcessingResultsAndPointsToVecs(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData){
    auto& tGround = Nucleus.testingGround;
    if((tGround.modeOfTestingGround != testingGroundMode::exploring)||(tGround.statusOfExploringTestingGround != testingGroundExploringStatus::POSTPROCESSING_dispatchedDerivedDiffs)){
        throw std::runtime_error("Testing ground not in exploring mode but asked to perform a preprocessing step OR real var data has not been copied to test slots yet!");
    }
    
    int expectedResultsSize = referenceData.numTagPairsToTest;//number of datasets that could be output is the # of pairs that were tested.
                                                assert(expectedResultsSize >= 1);
        auto& resultVec = tGround.Data.results;
            resultVec.resize(expectedResultsSize);//resize if of lower sizde than required.
    std::span<Dataset> spanOfResults(resultVec.begin(), resultVec.begin() + expectedResultsSize);
    subcommandsHelper::appendDataCopyOp<Dataset>(spanOfResults, bufferTypes::PRIVATE_outputBuffer, subcommandsVec);//data copy op to copy back results staging buffer
    
    GPU_lieutenant::addMetalCommand(subcommandsVec);//adds the command to execute everything
    
   //add command to copy from private output staging buffer to results vector in data
    GPU_lieutenant::addCopyCommand<Dataset>(spanOfResults, bufferTypes::PRIVATE_outputBuffer, false, &statusUpdaters::ExplorationUpdaters::convertTestTagsToRealTags);
    
    //commands still need to be fired...
    
    groundsWatchdog::toggleStatus();
    
}

//replaces the dataset tags with updated tags.
void statusUpdaters::ExplorationUpdaters::convertTestTagsToRealTags(){
    auto& tGround = Nucleus.testingGround;
    if((tGround.modeOfTestingGround != testingGroundMode::exploring)||(tGround.statusOfExploringTestingGround != testingGroundExploringStatus::POSTPROCESSING_copiedResultsBack)){
        throw std::runtime_error("Testing ground not in exploring mode but asked to perform a preprocessing step OR rsults havent been copied to test slots yet!");
    }
    
    auto mapOfTestTagToReals = tagAssigner::testTagManager.mapOftestTagsToRealTags();
    auto& resultsVec = Nucleus.testingGround.Data.results;
    for(int u = 0; u < resultsVec.size(); u++){
        //y-tag is test-synth, x-tag is real-tester-tag
        auto& dSet = resultsVec[u];
        assert((dSet.returnYtag() % 2 == 1)&&(dSet.returnXtag() % 2 == 0));
        
        uint16_t realTestTag = dSet.returnXtag();
        uint16_t synthTestTag = dSet.returnYtag();
        
        auto realCounterpart_ITER = mapOfTestTagToReals.find(realTestTag);
        assert(realCounterpart_ITER != mapOfTestTagToReals.end());
        //the synth tag assigned isnt a real tag yet but rather the its position in the batch it was generated in
        int convertedSynthTag = tagAssigner::testTagManager.convertTestSynthTagToTagCorrespondingPositionInBatch(synthTestTag);
        Dataset replacementDSet = Dataset(realCounterpart_ITER->second, convertedSynthTag, dSet.returnRelation());
        
        resultsVec[u] = replacementDSet;
    }
    auto& instrVec = Nucleus.testingGround.Data.morphModInstructions;
    assert(instrVec.size() > 0);//hsouldnt have been emptied out yet
    for(int e = 0; e < instrVec.size(); e++){
        int baseTagOfAddon = instrVec[e].returnTag();
        auto realCounterpartOfTestBase_ITER = mapOfTestTagToReals.find(baseTagOfAddon);
        assert(realCounterpartOfTestBase_ITER != mapOfTestTagToReals.end());
        morphOps morphOpOfAddon = instrVec[e].returnMorphOp();
        modOps modOpOfAddon = instrVec[e].returnModOp();
        int realTag = realCounterpartOfTestBase_ITER->second;
        
        synthVarAddon replacementAddon = synthVarAddon(morphOpOfAddon, modOpOfAddon, realTag);
        instrVec[e] = replacementAddon;//replace synthetic addons with the correct tags too!
    }
}

std::span<const synthVarAddon> statusUpdaters::ExplorationUpdaters::returnAddonsCorrespondingToTag(uint16_t synthTestTag, testingGroundPasskey viewPasskey){
    
    auto& addonVec = Nucleus.testingGround.Data.morphModInstructions;
    //if no addons aka testing ground was in analyzing mode and no syntths made then return empty span
    if(addonVec.size() == 0){assert(Nucleus.testingGround.queryMode() == testingGroundMode::analyzing); return {}; }
    
    uint16_t correspondingPositionInBatch = synthTestTag;//tags were already converted to positon in batch by convertTestTagsToRealTags.
    
    size_t batchSize = static_cast<size_t>(synthVarGen::currentAddonsPerTestSynth);
    assert(addonVec.size() % batchSize == 0);//addons need to be a multiple of the size of batch
    std::span<const synthVarAddon> returnSpan(addonVec.begin() + batchSize*correspondingPositionInBatch, batchSize);
    
    return returnSpan;
}

std::span<float> statusUpdaters::ExplorationUpdaters::returnPointsCorrespondingToTag(uint16_t synthTestTag, testingGroundPasskey passkey){
    auto& resultsVec = Nucleus.testingGround.Data.testVarsStorage; assert(resultsVec.size() > 0);//shouldnt have been cleared yet
    uint16_t correspondingPositionInBatch = synthTestTag;//tags were already converted to positon in batch by convertTestTagsToRealTags.
    int startIndexOfPoints = RelationConstructionMetadata::points_per_var * ((synthTestTag*2)+1);//converting position in batch to position in data
    
    std::span<float> returnSpan(&resultsVec[startIndexOfPoints], RelationConstructionMetadata::points_per_var);    
    return returnSpan;
}

//void statusUpdaters::ExplorationUpdaters::statusUpdaters::ExplorationUpdaters::clearStorage()-- need to implement



void statusUpdaters::analyzationUpdaters::copyVariableDataIntoTestSlots(std::vector<subcommands>& subcommandsVec){
    
}
