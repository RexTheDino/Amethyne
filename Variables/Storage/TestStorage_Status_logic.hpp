//
//  TestStorage_Status_logic.hpp
//  Amethyne
//
//  Created by AnshuDiya on 11/12/26.
//

#pragma once
#include "Dispatcher.hpp"
#include "Variable_Gen_Header.hpp"

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
    PREPROCESSING_initializedDispatch,
    POSTPROCESSING_derivedDiffsMtdtInitialized,
    POSTPROCESSING_dispatchedDerivedDiffs,
    POSTPROCESSING_copiedResultsBack,
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
    solidified,//aka tags converted back to real tgas.
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
    public:
        class ExplorationUpdaters{//clean up later to match GPU_lieutenant syntax and requirements.
            friend struct exploringTestingGround_Tester;
            struct RIGGED_TAG_PAIR{
                uint16_t realTag;
                uint16_t testTag;
            };
            struct RIGGED_EXPLORATION_REFERENCE{//only to be used when the functions are in RIGGED mode for testing relationships!
                synthVarGen::riggedSynthVarFuncReturn riggedInfo;
                std::vector<RIGGED_TAG_PAIR> riggedTagPairs;
            };
            
            struct explorationReference{//filled by the functions as they move on
                std::vector<MtdtStruct> initDatSpan;
                int numberOfBatchesOfSynthAddons = -1;
                //add as needed...
                
                std::vector<uint16_t> relationTestTags;
                uint16_t numTagPairsToTest;
                
                RIGGED_EXPLORATION_REFERENCE RIGGED_REFERENCE;//to only be used when trying ot rig relationships and test
            };
            
            
            //every function has the subcommands parameter and same return type even if it doesnt need it so it can be called through array of function pointers.
            
            static void generateSyntheticAddons(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData);
            
            static void GENERATE_RIGGED_SYNTHETIC_ADDONS(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData);
            
            static void copyRealDataIntoTestTagSlots(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData);//copies the data of existing variables into their assigned test tag slots into testVarsStorage.
            static void initializeMetadata(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData);//implement a later version that requires no parameter
            
            
            //the kernel is formulated such that the input data for derived diffs (except for metadata) is written back to the metla buffer in the exact place it needs to be so no copies are needed
            static void dispatchSynthPreprocessing(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData);//to be implemented when the asynchronous metal dispatcher class has been implemented
            

            static void initializeDerivedDiffsMtdt(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData);
            
            static void initializeRIGGED_DERIVED_DIFFS_MTDT(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData);
            
            static void dispatchRelationEstimation(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData);//dispatch the relation estimation kernel.
            static void copyPostProcessingResultsAndPointsToVecs(std::vector<subcommands>& subcommandsVec, explorationReference& referenceData);
            static void convertTestTagsToRealTags();
            

        public:
            class testingGroundPasskey{friend class TESTING_GROUND; testingGroundPasskey() = default;};
            static std::span<const synthVarAddon> returnAddonsCorrespondingToTag(uint16_t synthTestTag, testingGroundPasskey viewPasskey);
            static std::span<float> returnPointsCorrespondingToTag(uint16_t synthTestTag, testingGroundPasskey passkey);
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
