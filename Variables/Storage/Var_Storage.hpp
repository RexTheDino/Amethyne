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
#include "Symbolic_representation_logic.hpp"
//#include "Base_types.hpp"

#include "Transfer_Manager.hpp"



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

enum testStatus{//status for wehther testing ground has finished testing its results or is still waiting
    solidified,
    running
};

//testing ground to store variables in only to be moved if a relationship is found
class testingGround{
    //all vecs to be reserved with enough space
    struct Data{
        static std::vector<float> testVarsStorage;//to be reserved with enough space
        static std::vector<synthVarAddon> morphModInstructions;//to be reserved with enough space for morph instructions
        static std::vector<Dataset> results;//to contain results in the format listed in metal. is to be cleared with every batch tested. its size must be much mroe than needed.
    };
    testStatus STATUS;//metal changes this when its done
public:
    //returns the indice for the relationship containing the requested tag. From vector of type Dataset
    static inline int return_req_relationTag_indice(uint16_t tag, bool sort_by_y_tag = true);//returns indice of requested tag
    static inline int return_yTag_of_dSet(uint16_t indexOfDset);
    static inline int return_xTag_of_dSet(uint16_t indexOfDset);
    static inline RelationshipType return_relation_of_dSet(uint16_t indexOfDset);
};

//for permanently storing variables even though they may be reevaluated later. shoudld be sorted by tag.
//how tags work is tag%3==0 tags are reserved for action variables
//tag%3==1 tags are reserved for metric variables
//and finally tag%3==2 tags are reserved for synthetic variables
//this way there are ZERO shared tags across variables! even synthetic addons meant for testing only contain tags as a reference to what existing variables they modify!
class coldStorage{
    struct Data{
        static std::vector<metricVar> metricVariables;//all the metric variables are auto stored upon initialization
        static std::vector<actionVar> actionVariables;//all the action variables are auto stored upon initialization
        static std::vector<Dataset> datasetLogs;//logs all datasets from the testingGround results in time while deleting dupes
    };
    
    //stores tag-symbol pairs
    static std::unordered_map<SymEngine::RCP<const SymEngine::Basic>, uint16_t, SymEngine::RCPBasicHash, SymEngine::RCPBasicKeyEq> variableMap;
    
    //checks what kind of tag it is through the modulo operator
    static inline variableCategory returnVarType_givenTag(uint16_t tag);
    
    
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
    };
    
    //returns the tag for a symbol
    static inline uint16_t returnTag_givenVarSymbol(SymEngine::RCP<const SymEngine::Basic> varSymbol);
    static std::string return_programNameOf_taggedVar(const uint16_t tag);//returns program name for var if stored.
    static inline domain returnDomain(uint16_t tag);
};

#endif /* Var_Storage_hpp */
