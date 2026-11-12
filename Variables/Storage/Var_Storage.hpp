//
//  Var_Storage.hpp
//  Amethyne
//
//  Created by AnshuDiya on 11/3/26.
//
#pragma once

#ifndef Var_Storage_hpp
#define Var_Storage_hpp

#include <stdio.h>


#include <Variable_Gen_Header.hpp>
//#include "Relations_Header.hpp"

//#include <symengine/basic.h>
#include <unordered_map>
//#include "Symbolic_representation_logic.hpp"
//#include "Base_types.hpp"

#include "Transfer_Manager.hpp"

struct MtdtStruct{
    uint16_t pointsPerVar_as_multiple_of_64;//*as a multiple of 64! change derived_differences to also look for num_points and as a multiple of 64*
    uint16_t num_terms_per_synthetic;//the term count of this batch!
    uint16_t write_back_indice;//what starting indice to write back points to
};


class synthVarAddon;
//class permanentSynthAddon;
class metricVar;
class actionVar;
class synthVar;

//different from varType
enum class variableCategory{
    metric,
    action,
    synthetic
};


enum class testingGroundExploringStatus{
    empty,
    PREPROCESSING_generatedSynthAddons,
    PREPROCESSING_realVarsInTestSlots,
    PREPROCESSING_initializedMetadata,
    PREPROCESSING_running,
    FINISHED_PREPROCESSING,
    solidified,
    counter
};
//testing ground to store variables in only to be moved if a relationship is found
class testingGround{
    enum class testingGroundMode{
        testing,//for when it is testing relationships between ALREADY EXISTING VARIABLES
        exploring//for when it is testing relationships between existing variables as well as TEST SYNTHETICS
    };static testingGroundMode modeOfTestingGround;//can be manually set by a function
    
    static testingGroundExploringStatus statusOfExploringTestingGround;//must be toggled in between states, no direct set
    
    //when metal writes back the tags of the testing synths it just calculated back to the results vector, remember those tags DO NOT correspond with the tagging system for confirmed variables. they NEED TO BE CONVERTED before being accessed by other stuff.
    enum class datasetSynthTagConvertStatus{
        testSynthsTagsConverted,
        testSynthsTagsUNCONVERTED
    }; static datasetSynthTagConvertStatus testSynthTagSTATUS;
    //all vecs to be reserved with enough space
    struct Data{
        static std::vector<float> testVarsStorage;//to be reserved with enough space
        static std::vector<synthVarAddon> morphModInstructions;//to be reserved with enough space for morph instructions
        static std::vector<Dataset> results;//to contain results in the format listed in metal. is to be cleared with every batch tested. its size must be much mroe than needed.
    };
public:
    //returns the indice for the relationship containing the requested tag. From vector of type Dataset
    static inline std::span<Dataset> return_req_relationTag_related_datasets(uint16_t tag, bool sort_by_y_tag = true);//returns indice of requested tag. also leaves tags sorted
    static inline int return_yTag_of_dSet(uint16_t indexOfDset);
    static inline int return_xTag_of_dSet(uint16_t indexOfDset);
    static inline RelationshipType return_relation_of_dSet(uint16_t indexOfDset);

    
    class groundsWatchdog{
    private:
        
    public:
        static inline void setTestingGroundMode(testingGroundMode modeToSet);
        static inline void toggleStatus();
            static inline testingGroundExploringStatus queryExploringStatus();
    };
    class statusUpdaters{//updates the testing ground to be of a certain status
        static inline void generateSyntheticAddons();
        //returns the index to write back sythetic preprocessing results to
        static inline int copyRealDataIntoTestTagSlots();//copies the data of existing variables into their assigned test tag slots into testVarsStorage.
        static inline void initializeMetadata(MtdtStruct* ptrToMetalMetadataBuffer, int write_back_index);
        static inline void dispatchSynthPreprocessing();//to be implemented when the asynchronous metal dispatcher class has been implemented
        
        
        
        static inline void queryStatus();
        
    };
};

//for permanently storing variables even though they may be reevaluated later. shoudld be sorted by tag.
//how tags work is tag%3==0 tags are reserved for action variables
//tag%3==1 tags are reserved for metric variables
//and finally tag%3==2 tags are reserved for synthetic variables
//this way there are ZERO shared tags across variables! even synthetic addons meant for testing only contain tags as a reference to what existing variables they modify!
class coldStorage{//primarily intended as variable and NOT data storage!
    struct Data{
        static std::vector<metricVar> metricVariables;//all the metric variables are auto stored upon initialization
        static std::vector<actionVar> actionVariables;//all the action variables are auto stored upon initialization
        static std::vector<Dataset> datasetLogs;//logs all datasets from the testingGround results in time while deleting dupes
    };
    
    //stores tag-symbol pairs
    static std::unordered_map<SymEngine::RCP<const SymEngine::Basic>, uint16_t, SymEngine::RCPBasicHash, SymEngine::RCPBasicKeyEq> variableMap;
    
    
public:
    class synthVarManager{
        static std::vector<permanentSynthAddon> syntheticAddons;//all synthetic variable addons stored here
        static std::vector<synthVar> syntheticVariables;//all synthetic variables stored here
            static inline std::vector<permanentSynthAddon> resultVec;//the reference to this to eb output when requested
        
        static void sortSynthsByTag();//sorts synthetic variables by their tag in ascending order
        
        //passkey required for adding a synthetic variable since constructor is private
        class addSyntheticPasskey{friend class TransferManager; addSyntheticPasskey() = default;};
    public:
        synthVarManager();
        //returns the synthVarAddon operations for the specified tag
        static std::vector<permanentSynthAddon>* returnOpsForRequestedTag(const uint16_t tag);
        static std::string return_programNameOf_taggedVar(const uint16_t tag);//returns program name for var if stored.
        static domain returnDomainOfTaggedVar(const uint16_t tag);
        
        static void addSyntheticVariable(std::vector<permanentSynthAddon> correspondingAddons, synthVar derVariable, addSyntheticPasskey derPasskey);
                static inline uint16_t returnGroundedSynthTag();//returns the tag of a random grounded existing synthetic
                    static inline std::vector<uint16_t> returnALL_SYNTH_TAGS();//tag of every single synthetic variable currently existing
    };
    
    //returns the tag for a symbol
    static inline uint16_t returnTag_givenVarSymbol(SymEngine::RCP<const SymEngine::Basic> varSymbol);
    static inline SymEngine::RCP<const SymEngine::Basic>& returnReferenceToRequestedSymbol(uint16_t tag);
    static std::string return_programNameOf_taggedVar(const uint16_t tag);//returns program name for var if stored.
    static inline domain returnDomain(uint16_t tag);
    //checks what kind of tag it is through the modulo operator
    static inline variableCategory returnVarType_givenTag(uint16_t tag);
/*    A synthetic variable is grounded if and only if the chain/path it takes down towards the variables that comprise its base types are composed of action or metric variables.

    This means that when relationships are attempted to be found, action variables can only ever be on the x_tag portion which is the independent variable, never the y_tag portion! 
    Metric variables can be both in x_tag and y_tag position since it ocould be possible that they are described as a function of action variables but also that other synthetic variables are impossible to describe with a function due to randomness without accounting for that randomness using observed metrics from metric variables. Therefore both metric and action variables comprise a kind of ‘base’ tier; however, metric variables can be tested for relationships with action variables unlike the other way around.
*/
        static inline uint16_t returnGroundedTag();//returns the tag of a random grounded existing variable
        static inline std::vector<uint16_t> returnALL_TAGS();//tag of every single variable currently existing
};

#endif /* Var_Storage_hpp */
