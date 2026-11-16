//
//  TestStorage_Status_logic.hpp
//  Amethyne
//
//  Created by AnshuDiya on 11/12/26.
//

#pragma once
#include "Dispatcher.hpp"

struct exploringTestingGround_Tester;

enum class testingGroundMode;
struct MtdtStruct;
class Dataset;


enum class testingGroundMode{
    analyzing,//for when it is testing relationships between ALREADY EXISTING VARIABLES
    exploring//for when it is testing relationships between existing variables as well as TEST SYNTHETICS
};

enum class testingGroundExploringStatus{
    empty,
    PREPROCESSING_generatedSynthAddons,
    PREPROCESSING_realVarsInTestSlots,
    PREPROCESSING_initializedMetadata,
    PREPROCESSING_running,
    FINISHED_PREPROCESSING,
    PREPROCESSING_resultsCopiedBackToStorage,
    POSTPROCESSING_derivedDiffsMtdtInitialized,
    POSTPROCESSING_running,
    FINISHED_POSTPROCESSING,
    POSTPROCESSING_solidified,//results have been copied back to storage
    counter
};

enum class testingGroundAnalyzingStatus{
    empty,
    variableDataInTestSlots,//still odd indexes even tho it doesnt matter
    initializedMetadata,//generate tags and write to metal
    running,
    FINISHED_RELATION_ESTIMATION,
    resultsCopiedBackToStorage,
    solidified,
    counter
};

class groundsWatchdog{
    friend struct exploringTestingGround_Tester;
private:
    
public:
    static void setTestingGroundMode(testingGroundMode modeToSet);
    static void toggleStatus();
        static testingGroundExploringStatus queryExploringStatus();
};

    class statusUpdaters{//updates the testing ground to be of a certain status
        friend struct exploringTestingGround_Tester;
        
        class ExplorationUpdaters{//clean up later to match GPU_lieutenant syntax and requirements.
            friend struct exploringTestingGround_Tester;
            
            struct explorationReference{//filled by the functions as they move on
                std::vector<MtdtStruct> initDatSpan;
                int numberOfBatchesOfSynthAddons = -1;
                //add as needed...
                
                std::vector<uint16_t> relationTestTags;
                uint16_t numTagPairsToTest;
            };
            
            //every function has the subcommands parameter and same return type even if it doesnt need it so it can be called through array of function pointers.
            static void generateSyntheticAddons(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData);
            static void copyRealDataIntoTestTagSlots(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData);//copies the data of existing variables into their assigned test tag slots into testVarsStorage.
            static void initializeMetadata(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData);//implement a later version that requires no parameter
            
            
            //the kernel is formulated such that the input data for derived diffs (except for metadata) is written back to the metla buffer in the exact place it needs to be so no copies are needed
            static void dispatchSynthPreprocessing(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData);//to be implemented when the asynchronous metal dispatcher class has been implemented
            //REMOVE THE FUNC BELOW, IT IS UNNECESSARY
            static void queryPreprocessingStatus(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData);//if metal finished, toggle mode. else return false. implement after metal dispatcher class
            
            //remove the function below, copying back results is not needed
            static void copyPreprocessingResultsBackToTestStorage(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData);//implement a later version that requires no parameter
            static void initializeDerivedDiffsMtdt(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData);
            static void dispatchRelationEstimation(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData);//dispatch the relation estimation kernel.
            static void queryPostProcessingResultStatus(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData);
            static void copyPostProcessingResultsToDatasetsVec(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData);
            static void clearStorage();
        };
        class analyzationUpdaters{
            static void copyVariableDataIntoTestSlots(std::vector<subcommands>& subcommandsVec);
            static void initializeRelationEstimationMetadata(std::vector<subcommands>& subcommandsVec);
            static void dispatchMetal(std::vector<subcommands>& subcommandsVec);
            static void queryRelationEstimationStatus();
            static void copyResultsBackToStorage(std::vector<subcommands>& subcommandsVec);
            static void clearStorage(std::vector<subcommands>& subcommandsVec);
        };
        
        static void queryMetalStatus();
        
    };
