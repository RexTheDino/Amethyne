//
//  TestStorage_Status_logic.hpp
//  Amethyne
//
//  Created by AnshuDiya on 11/12/26.
//

#pragma once

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
            
            static void generateSyntheticAddons();
            //returns the index to write back sythetic preprocessing results to
            static int copyRealDataIntoTestTagSlots();//copies the data of existing variables into their assigned test tag slots into testVarsStorage.
            static void initializeMetadata(MtdtStruct* ptrToMetalMetadataBuffer);//implement a later version that requires no parameter
            
            //the kernel is formulated such that the input data for derived diffs (except for metadata) is written back to the metla buffer in the exact place it needs to be so no copies are needed
            static void dispatchSynthPreprocessing();//to be implemented when the asynchronous metal dispatcher class has been implemented
            static bool queryPreprocessingStatus();//if metal finished, toggle mode. else return false. implement after metal dispatcher class
            
            static void copyPreprocessingResultsBackToTestStorage(float* ptrToMetalBuffer);//implement a later version that requires no parameter
            static void initializeDerivedDiffsMtdt(float* ptrToMetalDerivedDiffInitBuffer);
            static void dispatchRelationEstimation();//dispatch the relation estimation kernel.
            static bool queryPostProcessingResultStatus();
            static void copyPostProcessingResultsToDatasetsVec(Dataset* ptrToMetalResultsBuffer, int numTestedRelations);
            static void clearStorage();
        };
        class analyzationUpdaters{
            static void copyVariableDataIntoTestSlots();
            static void initializeRelationEstimationMetadata();
            static void dispatchMetal();
            static void queryRelationEstimationStatus();
            static void copyResultsBackToStorage();
            static void clearStorage();
        };
        
        static void queryMetalStatus();
        
    };
