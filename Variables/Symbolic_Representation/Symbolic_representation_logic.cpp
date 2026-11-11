//
//  Symbolic_representation_logic.cpp
//  Amethyne
//
//  Created by AnshuDiya on 11/5/26.
//

#include "Symbolic_representation_logic.hpp"
#include "Initialization_Data_header.hpp"

using namespace SymEngine;

std::string synthetic_parser::varNameReturn(uint16_t tag){
    std::srand(tag);
    char randString[5];
    for (int h = 0; h < 5; h++) {
        randString[h] = char(((float(std::rand())*25.0f)/float(RAND_MAX))+65.0f);
    }
    std::string returnString(randString);
    return returnString;
}

//fix after implementing transfer manager
SymEngine::RCP<const SymEngine::Basic> synthetic_parser::synthParser(uint16_t tag){
    /*returnExpression.resize(2);
    auto& result = returnExpression[0];
    result = symbol(varNameReturn(tag));
    auto& intermediaryTerm = returnExpression[1];
    morphOps previousOp;//stores the previous op of the last variable
    auto& addonVec = *(coldStorage::synthVarManager::returnOpsForRequestedTag(tag));
    for (int b = 0; b < addonVec.size(); b++) {
        //set intermediary term to expression returned by the addon for the specific indice
        intermediaryTerm = coldStorage::returnExprForReqTag(addonVec[b].returnAddonTag());
        switch (addonVec[0].returnAddonModOp()) {
            case modOps::recp :
                intermediaryTerm = div(integer(1), intermediaryTerm);
                break;
            case modOps::square :
                intermediaryTerm = pow(intermediaryTerm, real_double(2.0f));
                break;
        }
        if(b != 0){
            int previousTermIndex = b-1;//for clarity
            switch (addonVec[previousTermIndex].returnAddonMorphOp()) {
                case morphOps::add :
                    result = add(result, intermediaryTerm);
                    break;
                case morphOps::subtract :
                    result = sub(result, intermediaryTerm);
                    break;
                case morphOps::multiply :
                    result = mul(result, intermediaryTerm);
                    break;
            }
        } else {
            result = intermediaryTerm;
        }
        
    }
    return result;*/
    RCP<const Basic> dummyResult;
    return dummyResult;
}
