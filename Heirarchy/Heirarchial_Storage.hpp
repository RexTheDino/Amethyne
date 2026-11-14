//
//  Heirarchial_Storage.hpp
//  Amethyne
//
//  Created by AnshuDiya on 11/7/26.
//

#pragma once

#ifndef Heirarchial_Storage_hpp
#define Heirarchial_Storage_hpp

#include "Symbolic_representation_logic.hpp"
#include "Relations_Header.hpp"


struct symbolicVariable{
private:
    std::string programName;//the name assigned by the program to this variable
    SymEngine::RCP<const SymEngine::Basic> expression;//the expression relating it to other variables
public:
    symbolicVariable(std::string program_name, SymEngine::RCP<const SymEngine::Basic> assigned_expression);
};

//contains all the variables in a certain momentum.
struct MomentumStorage{
private:
    std::vector<symbolicVariable> expressions;//expressions tagged with program names of all variables in this momentum
    uint16_t assignedMomentum;
public:
    MomentumStorage(uint16_t momentum);//reserves space for expressions
    void appendSymbolicVariable(symbolicVariable varToAppend);
    symbolicVariable returnExpressionOfRequestedVariable(std::string variableName);
    uint16_t returnMomentum();
};

class ExpressionStorage{
    static std::vector<MomentumStorage> momenti;//every expression categorized by momentum!!!
public:
    ExpressionStorage();//reserves space for the expressions
    static SymEngine::RCP<const SymEngine::Basic> returnMasterExpression();//returns master expression NOT in terms of the underling goals but all the relationships that have been found with respect to the underling goals!!
    
    //for non-synthetic vars simply dont add the synthVarAddon parameter
    static bool appendSymbolicVar(uint16_t tagOfVar, std::span<Dataset> dSetsRelatedToVar, uint16_t momentumOfVar);
};

#endif /* Heirarchial_Storage_hpp */
