//
//  Var_Storage.hpp
//  Amethyne
//
//  Created by AnshuDiya on 11/3/26.
//
#pragma once

#ifndef Var_Storage_hpp
#define Var_Storage_hpp


#include "Symbolic_representation_logic.hpp"
#include "Relations_Header.hpp"
#include "Variable_Gen_Header.hpp"

//#include <symengine/basic.h>
//#include "Symbolic_representation_logic.hpp"
//#include "Base_types.hpp"

//#include "Transfer_Manager.hpp"

enum class RelationshipType: uint16_t;
class groundsWatchdog;
class statusUpdaters;
enum class testingGroundExploringStatus;
enum class testingGroundMode;
enum class testingGroundAnalyzingStatus;

struct MtdtStruct{
    uint16_t num_terms_per_synthetic;//the term count of this batch!
};
struct relationshipResult{
    uint16_t xTag;
    uint16_t yTag;
    RelationshipType typeOfRelationship;
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



//testing ground to store variables in only to be moved if a relationship is found
class TESTING_GROUND{
    friend struct exploringTestingGround_Tester;
    
    friend class groundsWatchdog;
    friend class statusUpdaters;
    
    testingGroundMode modeOfTestingGround;//can be manually set by a function
    
    testingGroundExploringStatus statusOfExploringTestingGround;//must be toggled in between states, no direct set
    testingGroundAnalyzingStatus statusOfAnalyzingTestingGround;
    
    
    //when metal writes back the tags of the testing synths it just calculated back to the results vector, remember those tags DO NOT correspond with the tagging system for confirmed variables. they NEED TO BE CONVERTED before being accessed by other stuff.
    enum class datasetSynthTagConvertStatus{
        testSynthsTagsConverted,
        testSynthsTagsUNCONVERTED
    }testSynthTagSTATUS;
    //all vecs to be reserved with enough space
    struct DATA{
        friend struct exploringTestingGround_Tester;
        
        std::vector<float> testVarsStorage = std::vector<float>(500000);//to be reserved with enough space
        std::vector<synthVarAddon> morphModInstructions;//to be reserved with enough space for morph instructions
        std::vector<Dataset> results;//to contain results in the format listed in metal. is to be cleared with every batch tested. its size must be much mroe than needed.
    }Data;
public:
    //returns the indice for the relationship containing the requested tag. From vector of type Dataset
    std::span<Dataset> return_req_relationTag_related_datasets(uint16_t tag, bool sort_by_y_tag = true);//returns indice of requested tag. also leaves tags sorted
    int return_yTag_of_dSet(uint16_t indexOfDset);
    int return_xTag_of_dSet(uint16_t indexOfDset);
    RelationshipType return_relation_of_dSet(uint16_t indexOfDset);
};

//for permanently storing variables even though they may be reevaluated later. shoudld be sorted by tag.
//how tags work is tag%3==0 tags are reserved for action variables
//tag%3==1 tags are reserved for metric variables
//and finally tag%3==2 tags are reserved for synthetic variables
//this way there are ZERO shared tags across variables! even synthetic addons meant for testing only contain tags as a reference to what existing variables they modify!
class cold_Storage{//primarily intended as variable and NOT data storage!
    struct DATA{
        std::vector<metricVar> metricVariables;//all the metric variables are auto stored upon initialization
        std::vector<actionVar> actionVariables;//all the action variables are auto stored upon initialization
        std::vector<Dataset> datasetLogs;//logs all datasets from the testingGround results in time while deleting dupes
    }Data;
    
    struct UserAllotedNameAndTag{
        std::string UserAllotedName;
        uint16_t tag;
    };
    
    //stores tag-symbol pairs
    std::unordered_map<SymEngine::RCP<const SymEngine::Basic>, UserAllotedNameAndTag, SymEngine::RCPBasicHash, SymEngine::RCPBasicKeyEq> variableMap;
    
public:
    class SYNTH_VAR_MANAGER{
        std::vector<permanentSynthAddon> syntheticAddons;//all synthetic variable addons stored here
        std::vector<synthVar> syntheticVariables;//all synthetic variables stored here
            std::vector<permanentSynthAddon> resultVec;//the reference to this to eb output when requested
        
        void sortSynthsByTag();//sorts synthetic variables by their tag in ascending order
        
        //passkey required for adding a synthetic variable since constructor is private
        class addSyntheticPasskey{friend class TransferManager; addSyntheticPasskey() = default;};
    public:
        SYNTH_VAR_MANAGER();
        //returns the synthVarAddon operations for the specified tag
        std::vector<permanentSynthAddon>* returnOpsForRequestedTag(const uint16_t tag);
        std::string return_programNameOf_taggedVar(const uint16_t tag);//returns program name for var if stored.
        domain returnDomainOfTaggedVar(const uint16_t tag);
        
        void addSyntheticVariable(std::vector<permanentSynthAddon> correspondingAddons, synthVar derVariable, addSyntheticPasskey derPasskey);
                uint16_t returnGroundedSynthTag();//returns the tag of a random grounded existing synthetic
                    std::vector<uint16_t> returnALL_SYNTH_TAGS();//tag of every single synthetic variable currently existing
    }synthVarManager;
    
        class TransferManagerPasskey{friend class Transfer_Manager; TransferManagerPasskey() = default;};
    void addActionVar(TransferManagerPasskey passkey, actionVar varToAdd);
    void addMetricVar(TransferManagerPasskey passkey, metricVar varToAdd);
    
    //returns the tag for a symbol
    uint16_t returnTag_givenVarSymbol(SymEngine::RCP<const SymEngine::Basic> varSymbol);
    SymEngine::RCP<const SymEngine::Basic>& returnReferenceToRequestedSymbol(uint16_t tag);
    std::string return_programNameOf_taggedVar(const uint16_t tag);//returns program name for var if stored.
    domain returnDomain(uint16_t tag);
    std::string returnProgName_given_UserAlottedName(std::string UserAlottedName);
    //checks what kind of tag it is through the modulo operator
    variableCategory returnVarType_givenTag(uint16_t tag);
/*    A synthetic variable is grounded if and only if the chain/path it takes down towards the variables that comprise its base types are composed of action or metric variables.

    This means that when relationships are attempted to be found, action variables can only ever be on the x_tag portion which is the independent variable, never the y_tag portion! 
    Metric variables can be both in x_tag and y_tag position since it ocould be possible that they are described as a function of action variables but also that other synthetic variables are impossible to describe with a function due to randomness without accounting for that randomness using observed metrics from metric variables. Therefore both metric and action variables comprise a kind of ‘base’ tier; however, metric variables can be tested for relationships with action variables unlike the other way around.
*/
        std::vector<uint16_t> returnRandTags(int numTagsToReturn);
        std::vector<uint16_t> returnALL_TAGS();//tag of every single variable currently existing
};

#endif /* Var_Storage_hpp */
