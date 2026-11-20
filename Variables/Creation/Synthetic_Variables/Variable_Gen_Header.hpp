//
//  Variable_Gen_Header.hpp
//  Amethyne
//
//  Created by AnshuDiya on 11/2/26.
//

#pragma once


//#include "Initialization_Data_header.hpp"
//#include "Relations_Header.hpp"
//#include "Metadata.h"
#include "Base_types.hpp"
#include <string>

#include <span>
#include <vector>
#include <optional>

class addonGenPasskey;

enum class morphOps: uint16_t{
    add = 1,//add the next term to this
    subtract = 2,//subtract the next term from this
    multiply = 3,//multiply the next term times this
    count = 4//size of morphOps. udpate if adding more variables!
};

//modifiers to this variable
enum class modOps: uint16_t{
    recp = 1,//set this number to its reciprocal
    square = 2,//set this number to its square
    none = 3,
    count = 4
};

//can/should be chained
class synthVarAddon{
    morphOps morph;//instruction on how to deal with the next variable or this one
    modOps mod;//how to modify this variable
    //if varChainPosition is greater than the number of complex vars allowed to be chained, throw exception
    uint16_t tag;//which base_var this addon points to
public:
    synthVarAddon(morphOps morphInstruction, modOps modInstruction, uint16_t assignedTag);
    synthVarAddon() = default;
    uint16_t returnTag() const;
    morphOps returnMorphOp() const;
    modOps returnModOp() const;
};

//the one on top is NOT meant to be permanent!
class permanentSynthAddon{
public:
    enum class linkage{//wether the addon links to a metric/action variable or not (linked to another synthetic).
        linkedToAuthentic,
        linkedToSynthetic
    };
private:
    linkage LINKAGE;
    synthVarAddon addon;
public:
    
    //also need to add an expression variable from symEngine
    linkage returnLINKAGE() const;
    uint16_t returnAddonTag() const;//returns tag the var the synthVarAddon is linked to. NOT A TAG BELONGING TO ITSELF!
    morphOps returnAddonMorphOp() const;
    modOps returnAddonModOp() const;
    permanentSynthAddon(linkage derLinkage, synthVarAddon derAddon);
};

class synthVar{//extremely barebones, intentionally so.
    std::span<const permanentSynthAddon> addonsVarIsLinkedTo;
    uint16_t tag;//tag of the synthetic var
    std::string program_name;//the name the program gives to this variable for use in symbolic expressions
    //cant have domain becuase it violates the varibale only knowing info about itself that belongs in heirarchial storage.
    std::optional<domain> rangeOfSynth;//to be initialized after its transferred but only once! (by synthVarManager when it adds it and differentiates the expression to find min/max values)
public:
    synthVar(uint16_t derTag, std::span<permanentSynthAddon> linkedAddons, std::string assigned_prog_name);
    const uint16_t returnTag();
    std::span<const permanentSynthAddon> returnLinkedAddons() const;
    std::string returnName() const;//PROGRAM NAME
    domain returnRange() const;
    
};


struct numAddonsPerTestSynth{
    uint16_t numAddons;
};

class synthVarGen{
    friend struct exploringTestingGround_Tester;
    
    struct riggedSynthVar_Info{
        uint16_t baseTag;
        synthVarAddon theAddonApplied;
    };
public:
    struct riggedSynthVarFuncReturn{
        std::vector<synthVarGen::riggedSynthVar_Info> riggedAddons;
        numAddonsPerTestSynth numAddons;
    };

    //writes synth vars to the adress provided through std::span
    //returns the number it decided for the addons/terms per synthVar
public:
    static inline int currentAddonsPerTestSynth;
    static numAddonsPerTestSynth createAndWriteSynthVars(uint16_t numVarsToCreate, std::vector<synthVarAddon>& resultVec, addonGenPasskey passkey);
    
    //creates and writes synth vars that have been rigged with a specific operation, so only one addon. returns a struct containing the info for the rigged synthetic variables
    static riggedSynthVarFuncReturn createAndWrite_RIGGED_synthVars(uint16_t numVarsToCreate, std::vector<synthVarAddon>& resultVec, addonGenPasskey passkey);
};
