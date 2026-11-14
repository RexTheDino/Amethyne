//
//  Heirarchial_Storage.cpp
//  Amethyne
//
//  Created by AnshuDiya on 11/7/26.
//

#pragma once

#include "Heirarchial_Storage.hpp"
#include "Var_Storage.hpp"

MomentumStorage::MomentumStorage(uint16_t momentum){expressions.reserve(50); assignedMomentum = momentum;}

using namespace SymEngine;

//assumes that DATASET HAS BEEN CONSTRUCTED WITH ITS TAGS APPROPRIATELY CONVERTED FROM SYNTH_VAR TAGS TO REGULAR VAR TAGS!
bool ExpressionStorage::appendSymbolicVar(uint16_t tagOfVar, std::span<Dataset> dSetsRelatedToVar, uint16_t momentumOfVar){
    RCP<const Basic> expressionForSymbolicVar;
    std::string program_name_of_new_var = synthetic_parser::varNameReturn(tagOfVar);
    
    auto valididtyCheckIterator = std::find_if_not(dSetsRelatedToVar.begin(), dSetsRelatedToVar.end(), [&](Dataset& dSet){
        return dSet.returnXtag() == tagOfVar;
    });
    if(valididtyCheckIterator == dSetsRelatedToVar.end()){throw std::runtime_error("Variable requested to be appended has relationships provided where its not part of the dependent/y-tag variable!");}
    
    //only non-action variables can have relationshps to something else.
    if(coldStorage::returnVarType_givenTag(tagOfVar) != variableCategory::action){//also need to implement this along with sgels!
        expressionForSymbolicVar = Relation_Equation_Constructor::calculateCoefficients(tagOfVar);
    }else{
        return false;
    }
    symbolicVariable newSymbolicVariable(program_name_of_new_var, expressionForSymbolicVar);
    auto doesMomentumExist = std::find_if(momenti.begin(), momenti.end(), [&](MomentumStorage& currentStorageStruct){
        return currentStorageStruct.returnMomentum() == momentumOfVar;
    });
    if(doesMomentumExist == momenti.end()){MomentumStorage newMomentum(momentumOfVar); momenti.push_back(newMomentum); momenti[momenti.size()-1].appendSymbolicVariable(newSymbolicVariable);}//if momentum storage with req momentum doesnt exist append it
    else {(*doesMomentumExist).appendSymbolicVariable(newSymbolicVariable);}//if it does exist dereference and append
    std::sort(momenti.begin(), momenti.end(), [](MomentumStorage& currentStorageStruct1, MomentumStorage& currentStorageStruct2){
        return currentStorageStruct1.returnMomentum() < currentStorageStruct2.returnMomentum();
    });
    return true;
}
