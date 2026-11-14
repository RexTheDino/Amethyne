//
//  TestStorage_Status_logic.hpp
//  Amethyne
//
//  Created by AnshuDiya on 11/12/26.
//

#pragma once

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
private:
    
public:
    static inline void setTestingGroundMode(testingGroundMode modeToSet);
    static inline void toggleStatus();
        static inline testingGroundExploringStatus queryExploringStatus();
};

    class statusUpdaters{//updates the testing ground to be of a certain status
        class ExplorationUpdaters{//clean up later to match GPU_lieutenant syntax and requirements.
            static inline void generateSyntheticAddons();
            //returns the index to write back sythetic preprocessing results to
            static inline int copyRealDataIntoTestTagSlots();//copies the data of existing variables into their assigned test tag slots into testVarsStorage.
            static inline void initializeMetadata(MtdtStruct* ptrToMetalMetadataBuffer);//implement a later version that requires no parameter
            
            //the kernel is formulated such that the input data for derived diffs (except for metadata) is written back to the metla buffer in the exact place it needs to be so no copies are needed
            static inline void dispatchSynthPreprocessing();//to be implemented when the asynchronous metal dispatcher class has been implemented
            static inline bool queryPreprocessingStatus();//if metal finished, toggle mode. else return false. implement after metal dispatcher class
            
            static inline void copyPreprocessingResultsBackToTestStorage(float* ptrToMetalBuffer);//implement a later version that requires no parameter
            static inline void initializeDerivedDiffsMtdt(float* ptrToMetalDerivedDiffInitBuffer);
            static inline void dispatchRelationEstimation();//dispatch the relation estimation kernel.
            static inline bool queryPostProcessingResultStatus();
            static inline void copyPostProcessingResultsToDatasetsVec(Dataset* ptrToMetalResultsBuffer, int numTestedRelations);
            static inline void clearStorage();
        };
        class analyzationUpdaters{
            static inline void copyVariableDataIntoTestSlots();
            static inline void initializeRelationEstimationMetadata();
            static inline void dispatchMetal();
            static inline void queryRelationEstimationStatus();
            static inline void copyResultsBackToStorage();
            static inline void clearStorage();
        };
        
        static inline void queryMetalStatus();
        
    };
