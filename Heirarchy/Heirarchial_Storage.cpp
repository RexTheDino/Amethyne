//
//  Heirarchial_Storage.cpp
//  Amethyne
//
//  Created by AnshuDiya on 11/7/26.
//

#pragma once

#include "Heirarchial_Storage.hpp"

MomentumStorage::MomentumStorage(){expressions.reserve(50);}

using namespace SymEngine;

//assumes that DATASET HAS BEEN CONSTRUCTED WITH ITS TAGS APPROPRIATELY CONVERTED FROM SYNTH_VAR TAGS TO REGULAR VAR TAGS!
void ExpressionStorage::appendSymbolicVar(uint16_t tagOfVar, std::span<Dataset> dSetsRelatedToVar, std::span<synthVarAddon> correspondingAddons){
    RCP<const Basic> expressionForSymbolicVar;
    
    auto valididtyCheckIterator = std::find_if_not(dSetsRelatedToVar.begin(), dSetsRelatedToVar.end(), [&](Dataset& dSet){
        return dSet.returnXtag() == tagOfVar;
    });
    if(valididtyCheckIterator == dSetsRelatedToVar.end()){throw std::runtime_error("Variable requested to be appended has relationships provided where its not part of the dependent/y-tag variable!");}
    
    if(!correspondingAddons.empty()){//also need to implement this along with sgels!
        expressionForSymbolicVar = synthetic_parser::synthParser(tagOfVar);
    }else{
        //need to fix sgels function first to implement this!
    }
}
