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
    void modifyExpression(SymEngine::RCP<const SymEngine::Basic> theExpression);
    SymEngine::RCP<const SymEngine::Basic> returnExpression() const;
    std::string returnProgramName();
};

//contains all the variables in a certain momentum.
struct MomentumStorage{
private:
    std::vector<symbolicVariable> expressions;//expressions tagged with program names of all variables in this momentum
    uint16_t assignedMomentum;
public:
    MomentumStorage(uint16_t momentum);//reserves space for expressions
    MomentumStorage();//initializes with momentum of 0
    void appendSymbolicVariable(symbolicVariable varToAppend);
    symbolicVariable* returnExpressionOfRequestedVariable(std::string variableName);//returns nullptr if variable doesnt exist
    std::span<const symbolicVariable> returnSpanOfExpressions() const;
    uint16_t returnMomentum() const;
};

class Expression_Storage{
    std::vector<MomentumStorage> momenti;//every expression categorized by momentum!!!
public:
    Expression_Storage();//reserves space for the expressions
    SymEngine::RCP<const SymEngine::Basic> returnMasterExpression();//returns master expression NOT in terms of the underling goals but all the relationships that have been found with respect to the underling goals!!
    
    //for non-synthetic vars simply dont add the dataset parameter
    //momentum is evaluated as time goes on
    bool appendSymbolicVar(uint16_t tagOfVar, std::span<Dataset> dSetsRelatedToVar);
    uint16_t returnMomentumOfRequestedVariable(std::string variableName);
    //const symbolicVariable& returnSymbolicVarFromRequestedMomentum(uint16_t requested_momentum);
    std::span<const MomentumStorage> returnStoredMomenti() const;//returns all the stored momenti in a non-modifiable span.
};

#endif /* Heirarchial_Storage_hpp */
