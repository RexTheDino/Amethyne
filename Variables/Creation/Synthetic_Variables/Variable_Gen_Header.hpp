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

class addonGenPasskey;

enum class morphOps{
    add = 1,//add the next term to this
    subtract = 2,//subtract the next term from this
    multiply = 3,//multiply the next term times this
    count = 4//size of morphOps. udpate if adding more variables!
};

//modifiers to this variable
enum class modOps{
    recp = 1,//set this number to its reciprocal
    square = 2,//set this number to its square
    count = 3
};

//can/should be chained
class synthVarAddon{
    morphOps morph;//instruction on how to deal with the next variable or this one
    modOps mod;//how to modify this variable
    //if varChainPosition is greater than the number of complex vars allowed to be chained, throw exception
    uint16_t tag;//which base_var this addon points to
public:
    synthVarAddon(morphOps morphInstruction, modOps modInstruction, uint16_t assignedTag);
    uint16_t returnTag();
    morphOps returnMorphOp();
    modOps returnModOp();
};

//the one on top is NOT meant to be permanent!
class permanentSynthAddon{
public:
    enum class status{//status of the addon
        rowStart,
        qualifier,
        rowEnd
    };
    enum class linkage{//wether the addon links to a metric/action variable or not (linked to another synthetic).
        linked,
        not_linked
    };
private:
    status STATUS;
    linkage LINKAGE;
    synthVarAddon addon;
public:
    
    //also need to add an expression variable from symEngine
    status returnSTATUS();
    linkage returnLINKAGE();
    uint16_t returnAddonTag();//returns tag the var the synthVarAddon is linked to. NOT A TAG BELONGING TO ITSELF!
    morphOps returnAddonMorphOp();
    modOps returnAddonModOp();
    permanentSynthAddon(status derStatus, linkage derLinkage, synthVarAddon derAddon);
};

class synthVar{//extremely barebones, intentionally so.
    uint16_t synthAddonStartIndice;
    uint16_t tag;//tag of the synthetic var
    std::string program_name;//the name the program gives to this variable for use in symbolic expressions
    domain derDomain;
public:
    synthVar(uint16_t derTag, uint16_t derAddonStartIndex, domain domainOfVar);
    const uint16_t returnTag();
    uint16_t returnStartIndice();
    domain returnDomain();
    std::string returnName();//PROGRAM NAME
};


struct numAddonsPerTestSynth{
    uint16_t numAddons;
};

class synthVarGen{
    //writes synth vars to the adress provided through std::span
    //returns the number it decided for the addons/terms per synthVar
public:
    static int currentAddonsPerTestSynth;
    static numAddonsPerTestSynth createAndWriteSynthVars(uint16_t numVarsToCreate, std::span<synthVarAddon> resultVec, addonGenPasskey passkey);
};
