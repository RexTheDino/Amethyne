//
//  Symbolic_representation_logic.hpp
//  Amethyne
//
//  Created by AnshuDiya on 11/5/26.
//

#ifndef Symbolic_representation_logic_hpp
#define Symbolic_representation_logic_hpp

#pragma once

#include <symengine/expression.h>
#include "Variable_Gen_Header.hpp"



//need to move this folder to a different folder as well. does not belong inside variables.

//the job of this is to take a group of synthetic addons and convert that into an expression. very simple.
class synthetic_parser{
    static std::vector<SymEngine::RCP<const SymEngine::Basic>> returnExpression;//to be reserved with 2 elements by default. if in a future case multiple expressions returned at once reserve it with more.
public:
    static std::string varNameReturn(uint16_t tag);//names a var given a tag. if name already exists returns that.
    //give it a variable and it returns the expression for it
    static SymEngine::RCP<const SymEngine::Basic> synthParser(uint16_t tag);
    //returns an expression given a vector of the permanent addons
    static SymEngine::RCP<const SymEngine::Basic> synthParser(std::vector<permanentSynthAddon> synths);
};

//source of truth for expressions and heirarchy between variables!! however, cannot modify the variables itself.
//for the line that actually bridges testingGround and colStorage a seperate class would have to be built.
//building this first so symbolic requirements for variables become clear.
class SymbolicManager{
    
};



#endif /* Symbolic_representation_logic_hpp */
