//
//  Heirarchial_Storage.cpp
//  Amethyne
//
//  Created by AnshuDiya on 11/7/26.
//

#pragma once

#include "Heirarchial_Storage.hpp"
#include "Var_Storage.hpp"
#include "Nucleus.hpp"

symbolicVariable::symbolicVariable(std::string program_name, SymEngine::RCP<const SymEngine::Basic> assigned_expression): programName{program_name}, expression{assigned_expression}{}

void symbolicVariable::modifyExpression(SymEngine::RCP<const SymEngine::Basic> theExpression){expression = theExpression;}

SymEngine::RCP<const SymEngine::Basic> symbolicVariable::returnExpression() const {return expression;}

std::string symbolicVariable::returnProgramName(){return programName;}

MomentumStorage::MomentumStorage(){assignedMomentum = 0;}//default constructor

MomentumStorage::MomentumStorage(uint16_t momentum){expressions.reserve(50); assignedMomentum = momentum;}

void MomentumStorage::appendSymbolicVariable(symbolicVariable varToAppend){expressions.push_back(varToAppend);}

symbolicVariable* MomentumStorage::returnExpressionOfRequestedVariable(std::string variableName){//returns nullptr if it doesnt exist
    auto doesVariableExist = std::find_if(expressions.begin(), expressions.end(), [&](symbolicVariable& theVariable){
        return theVariable.returnProgramName() == variableName;
    });
    if(doesVariableExist != expressions.end()){return &(*doesVariableExist);}else{return nullptr;}
}

std::span<const symbolicVariable> MomentumStorage::returnSpanOfExpressions() const {
    std::span<const symbolicVariable> spanOfExpressions(expressions.begin(), expressions.end());
    return spanOfExpressions;
}

uint16_t MomentumStorage::returnMomentum() const {return assignedMomentum;}

using namespace SymEngine;

Expression_Storage::Expression_Storage(){
    momenti.reserve(20);
}

//assumes that DATASET HAS BEEN CONSTRUCTED WITH ITS TAGS APPROPRIATELY CONVERTED FROM SYNTH_VAR TAGS TO REGULAR VAR TAGS!
//default appended to momentum 0 for testing.
bool Expression_Storage::appendSymbolicVar(uint16_t tagOfVar, std::span<Dataset> dSetsRelatedToVar){
    RCP<const Basic> expressionForSymbolicVar;
    std::string program_name_of_new_var = synthetic_parser::varNameReturn(tagOfVar);
    
    auto valididtyCheckIterator = std::find_if_not(dSetsRelatedToVar.begin(), dSetsRelatedToVar.end(), [&](Dataset& dSet){
        return dSet.returnXtag() == tagOfVar;
    });
    if(valididtyCheckIterator == dSetsRelatedToVar.end()){throw std::runtime_error("Variable requested to be appended has relationships provided where its not part of the dependent/y-tag variable!");}
    
    //only non-action variables can have relationshps to something else.
    if(Nucleus.coldStorage.returnVarType_givenTag(tagOfVar) != variableCategory::action){//also need to implement this along with sgels!
        expressionForSymbolicVar = Relation_Equation_Constructor::calculateCoefficients(tagOfVar);
    }else{
        return false;
    }
    symbolicVariable newSymbolicVariable(program_name_of_new_var, expressionForSymbolicVar);
    auto momentumZeroIterator = std::find_if(momenti.begin(), momenti.end(), [&](MomentumStorage& momentum){//must exist, is defualt initialized
        return momentum.returnMomentum() == 0;
    });
    if(momentumZeroIterator == momenti.end()){MomentumStorage zeroMomentum(0); momenti.push_back(zeroMomentum); momentumZeroIterator = momenti.begin();}
    momentumZeroIterator->appendSymbolicVariable(newSymbolicVariable);
    return true;
}

uint16_t Expression_Storage::returnMomentumOfRequestedVariable(std::string variableName){
    for(int p = 0; p < momenti.size(); p++){
        auto symbolicVarPtr = momenti[p].returnExpressionOfRequestedVariable(variableName);//returns nullptr if it doesnt exist
        if(symbolicVarPtr != nullptr){return momenti[p].returnMomentum();}//if variable exists in this momentum find and return its momentum
    }
    throw std::runtime_error("Variable does not exist in heirarchy!");
}

std::span<const MomentumStorage> Expression_Storage::returnStoredMomenti() const {
    std::span<const MomentumStorage> constReturnSpan(momenti.begin(), momenti.end());
    return constReturnSpan;
}
