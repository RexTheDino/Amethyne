//
//  Var_Storage.cpp
//  Amethyne
//
//  Created by AnshuDiya on 11/3/26.
//


//#include "Var_Storage.hpp"
#include <numeric>
#include <algorithm>
//#include "Symbolic_representation_logic.hpp"
#include "Data_logs.hpp"
#include "SynthAddon_gen.hpp"
#include "Transfer_Manager.hpp"
#include <random>
#include "Initialization_Data_header.hpp"
#include "Nucleus.hpp"


using namespace std;

//returns a span of all the datasets related to the requested tag. for example if y tag requested then span of all dataset with that tag as the y_tag
std::span<Dataset> TESTING_GROUND::return_req_relationTag_related_datasets(uint16_t tag, bool sort_by_y_tag){
    if(testSynthTagSTATUS != datasetSynthTagConvertStatus::testSynthsTagsConverted){
        throw std::runtime_error("Datasets accessed before thier temporary synthetic tags have been converted!!!");
    }
    auto& dSetVec = Nucleus.testingGround.Data.results;
    std::sort(dSetVec.begin(), dSetVec.end(), [sort_by_y_tag](Dataset& dSet1, Dataset& dSet2){
        if(sort_by_y_tag){
            return dSet1.returnYtag() < dSet2.returnYtag();
        }else{
            return dSet1.returnXtag() < dSet2.returnXtag();
        }
    });
    auto startDSetIterator = (lower_bound(dSetVec.begin(), dSetVec.end(), tag, [&](Dataset& dSet, uint16_t tag){
        return (sort_by_y_tag) ? dSet.returnYtag() < tag : dSet.returnXtag() < tag;
    }));
    auto endDSetIterator = (upper_bound(dSetVec.begin(), dSetVec.end(), tag, [&](uint16_t tag, Dataset& dSet){
        return (sort_by_y_tag) ? tag < dSet.returnYtag() : tag < dSet.returnXtag();
    }));
    std::span<Dataset> dSetSpanReturn(startDSetIterator, endDSetIterator-startDSetIterator);
    
    if(!sort_by_y_tag){
        //resort back to y-tag sorting
        std::sort(dSetVec.begin(), dSetVec.end(), [](Dataset& dSet1, Dataset& dSet2){
            return dSet1.returnYtag() < dSet2.returnYtag();
        });
    }
    return dSetSpanReturn;
}

int TESTING_GROUND::return_yTag_of_dSet(uint16_t indexOfDset){
    auto& results = Nucleus.testingGround.Data.results;
    if(indexOfDset >= results.size()){throw std::runtime_error("Requested dataset indice exceeds results array size!");}
    return results[indexOfDset].returnYtag();
}

int TESTING_GROUND::return_xTag_of_dSet(uint16_t indexOfDset){
    auto& results = Nucleus.testingGround.Data.results;
    if(indexOfDset >= results.size()){throw std::runtime_error("Requested dataset indice exceeds results array size!");}
    return results[indexOfDset].returnXtag();
}

RelationshipType TESTING_GROUND::return_relation_of_dSet(uint16_t indexOfDset){
    auto& results = Nucleus.testingGround.Data.results;
    if(indexOfDset >= results.size()){throw std::runtime_error("Requested dataset indice exceeds results array size!");}
    return results[indexOfDset].returnRelation();
}




variableCategory cold_Storage::returnVarType_givenTag(uint16_t tag){
    int tagMod3 = tag%3;
    switch (tagMod3) {
        case 0:
            return variableCategory::action;
            break;
        case 1:
            return variableCategory::metric;
            break;
        case 2:
            return variableCategory::synthetic;
            break;
        default:
            throw std::invalid_argument("Tag type not found!!!");
    }
}

std::vector<uint16_t> cold_Storage::returnRandTags(int numTags){
    assert(numTags <= (returnALL_TAGS()).size());//numTags to generate must be lesser than or equal to the number of total tags
    
    std::bernoulli_distribution biasedCoinFlip(0.9);
    std::vector<uint16_t> returnVec;
    //90% chance for one of the tags to be in the same momentum as the initial momentum and 10% to not.
    bool useSameMomentumY_Tag = biasedCoinFlip(Amethyne_rand);
        auto spanOfMomenti = Nucleus.ExpressionStorage.returnStoredMomenti();
    
    
    if(spanOfMomenti.size() <= 0){//if no expressions have been appended yet, just return random tags from cold storage regardless of momentum
        std::vector<uint16_t> tagsToPickFrom = returnALL_TAGS();
        assert(tagsToPickFrom.size() > 0);
        std::uniform_int_distribution<uint16_t> randTag(0, tagsToPickFrom.size()-1);
        for(int y = 0; y < numTags; y++){
            returnVec.push_back(tagsToPickFrom[randTag(Amethyne_rand)]);
        }
        
    }else{
        std::uniform_int_distribution<uint16_t> momentumToSelect(0, spanOfMomenti.size()-1);
        auto& selectedMomentum = spanOfMomenti[momentumToSelect(Amethyne_rand)];//returns a momentum to select in the range of momentums
        
        
        auto selectedExpressions = selectedMomentum.returnSpanOfExpressions();
        
        for(int g = 0; g < numTags; g++){
            bool tagIsPresentInResultVec = true;
            SymEngine::RCP<const SymEngine::Basic> selectedTagExpression;
            
            while(tagIsPresentInResultVec){
                
                if((!useSameMomentumY_Tag)&&(spanOfMomenti.size() > 1)){
                    int newMomentum = selectedMomentum.returnMomentum();
                    while (newMomentum == selectedMomentum.returnMomentum()) {
                        const MomentumStorage* candidateMomentum = &spanOfMomenti[momentumToSelect(Amethyne_rand)];
                        newMomentum = (*candidateMomentum).returnMomentum();
                        if(newMomentum != selectedMomentum.returnMomentum()){selectedExpressions = (*candidateMomentum).returnSpanOfExpressions();}
                    }
                }
                
                std::uniform_int_distribution<uint16_t> randExprGen(0, selectedExpressions.size()-1);
                selectedTagExpression = selectedExpressions[randExprGen(Amethyne_rand)].returnExpression();
                auto selected_TagExpr_Iterator = variableMap.find(selectedTagExpression);
                int candidateTag = (selected_TagExpr_Iterator->second).tag;
                auto isCandidateAlreadyPresent = std::find(returnVec.begin(), returnVec.end(), candidateTag);
                
                if(isCandidateAlreadyPresent == returnVec.end()){//if tag isnt already present, exit while loop
                    returnVec.push_back(candidateTag);
                    tagIsPresentInResultVec = false;//couldve also used break; but this is safer
                }
            }
        }
    }
    return returnVec;
}

cold_Storage::SYNTH_VAR_MANAGER::SYNTH_VAR_MANAGER(){
    resultVec.reserve(100);
}

void cold_Storage::SYNTH_VAR_MANAGER::sortSynthsByTag(){
    std::sort(syntheticVariables.begin(), syntheticVariables.end(), [&](synthVar& var1, synthVar& var2){
        return var1.returnTag() < var2.returnTag();
    });
}

std::vector<permanentSynthAddon>* cold_Storage::SYNTH_VAR_MANAGER::returnOpsForRequestedTag(const uint16_t tag){
    resultVec.clear();
    sort(syntheticVariables.begin(), syntheticVariables.begin(), [](synthVar& synth1, synthVar& synth2){
        return synth1.returnTag() < synth2.returnTag();
    });
    auto startIterator = lower_bound(syntheticVariables.begin(), syntheticVariables.end(), tag, [](synthVar& derSynth, uint16_t tag){
        return derSynth.returnTag() < tag;
    });
    int startIndice = int(syntheticVariables.begin()-startIterator);
    auto targetSynth = syntheticVariables[startIndice];
    if(syntheticAddons[targetSynth.returnStartIndice()].returnSTATUS() != permanentSynthAddon::status::rowStart){
        throw runtime_error("Error: The index pointed to by the synthetic variable for the start of the synthetic addon isnt actually a row start!");
    }
    int counter = 0;//counter for the length of the chain of synthetic addons
    int tempIndice = targetSynth.returnStartIndice();
    while(tempIndice < syntheticAddons.size()-targetSynth.returnStartIndice()){
        //if have reached the end of the row, increment counter and break.
        if(syntheticAddons[tempIndice].returnSTATUS() == permanentSynthAddon::status::rowEnd){
            counter++;
            break;
        }//if reached row start before end throw an exception!
        else if (syntheticAddons[tempIndice].returnSTATUS() == permanentSynthAddon::status::rowStart){
            throw runtime_error("Reached start of another synthetic addon row before reaching the end!!!");
            break;
        }
        tempIndice++;
        counter++;
    }
    resultVec.reserve(counter);//resizes to number of synthetic addons chained for this variable
    for(int n = 0; n < counter;n ++){
        //fill the result vector with the synthetic addons
        resultVec.push_back(syntheticAddons[targetSynth.returnStartIndice()+n]);
    }
    return &resultVec;
}

std::string cold_Storage::SYNTH_VAR_MANAGER::return_programNameOf_taggedVar(const uint16_t tag){
    if(Nucleus.coldStorage.returnVarType_givenTag(tag) == variableCategory::synthetic){
        auto iteratorOfRequestedVar = std::find_if(syntheticVariables.begin(), syntheticVariables.end(), [&](synthVar& derSynth){
            return derSynth.returnTag() == tag;
        });
        return iteratorOfRequestedVar->returnName();
    }else{
        std::string nullStr = "";
        return nullStr;
    }
}

domain cold_Storage::SYNTH_VAR_MANAGER::returnDomainOfTaggedVar(const uint16_t tag){
    domain emptyDomain(0.0f, 0.0f);
    if(Nucleus.coldStorage.returnVarType_givenTag(tag) == variableCategory::synthetic){
        auto iterator = std::find_if(syntheticVariables.begin(), syntheticVariables.end(), [&](synthVar& derSynth){
            return derSynth.returnTag() == tag;
        });
        return (iterator != syntheticVariables.end()) ? iterator->returnDomain() : emptyDomain;
    }else{
        return emptyDomain;
    }
}

void cold_Storage::SYNTH_VAR_MANAGER::addSyntheticVariable(std::vector<permanentSynthAddon> correspondingAddons, synthVar derVariable, addSyntheticPasskey derPasskey){
    syntheticVariables.push_back(derVariable);
    syntheticAddons.insert(syntheticAddons.end(), correspondingAddons.begin(), correspondingAddons.end());
}

std::vector<uint16_t> cold_Storage::SYNTH_VAR_MANAGER::returnALL_SYNTH_TAGS(){
    std::vector<uint16_t> returnVec;
    for(int e = 0; e < syntheticVariables.size(); e++){returnVec.push_back(syntheticVariables[e].returnTag());}
    
    return returnVec;
}


void cold_Storage::addActionVar(TransferManagerPasskey passkey, actionVar varToAdd){
    std::string userAlottedName = varToAdd.return_name();
    uint16_t tag = varToAdd.return_tag();
    UserAllotedNameAndTag returnStruct; returnStruct.UserAllotedName = userAlottedName; returnStruct.tag = tag;
    SymEngine::RCP<const SymEngine::Basic> symbolOfVar = SymEngine::symbol(varToAdd.return_program_name());
    variableMap.try_emplace(symbolOfVar, returnStruct);
    Nucleus.coldStorage.Data.actionVariables.push_back(varToAdd);
}
void cold_Storage::addMetricVar(TransferManagerPasskey passkey, metricVar varToAdd){
    std::string userAllotedName = varToAdd.return_name();
    uint16_t tag = varToAdd.return_tag();
    UserAllotedNameAndTag returnStruct; returnStruct.UserAllotedName = userAllotedName; returnStruct.tag = tag;
    SymEngine::RCP<const SymEngine::Basic> symbolOfVar = SymEngine::symbol(varToAdd.return_program_name());
    variableMap.try_emplace(symbolOfVar, returnStruct);
    Nucleus.coldStorage.Data.metricVariables.push_back(varToAdd);
}

//returns empty str if var does not exist
std::string cold_Storage::return_programNameOf_taggedVar(const uint16_t tag){
    std::string emptyStr = "";
    if(returnVarType_givenTag(tag) == variableCategory::action){//action variable
        auto iterator = std::find_if(Nucleus.coldStorage.Data.actionVariables.begin(), Nucleus.coldStorage.Data.actionVariables.end(), [&](actionVar& derActionVar){
            return derActionVar.return_tag() == tag;
        });
        if(iterator != Nucleus.coldStorage.Data.actionVariables.end()){
            return iterator->return_program_name();
        }else{
            return emptyStr;
        }
    }else if(returnVarType_givenTag(tag) == variableCategory::metric){
        auto iterator = std::find_if(Nucleus.coldStorage.Data.metricVariables.begin(), Nucleus.coldStorage.Data.metricVariables.end(), [&](metricVar& derMetricVar){
            return derMetricVar.return_tag() == tag;
        });
        return (iterator != Nucleus.coldStorage.Data.metricVariables.end()) ? iterator->return_program_name() : emptyStr;
    }else if(returnVarType_givenTag(tag) == variableCategory::synthetic){
        Nucleus.coldStorage.synthVarManager.return_programNameOf_taggedVar(tag);
    }else{
        throw std::runtime_error("Unrecognized variable tag!");
    }
        return emptyStr;
}

uint16_t cold_Storage::returnTag_givenVarSymbol(SymEngine::RCP<const SymEngine::Basic> varSymbol){
    auto variableIterator = variableMap.find(varSymbol);
    if(variableIterator != variableMap.end()){
        uint16_t variableTag = (variableIterator->second).tag;
        return variableTag;
    }else{
        throw std::invalid_argument("Symbol not stored in unordered map!!!");
    }
}

domain cold_Storage::returnDomain(uint16_t tag){
    domain emptyDomain(0.0f, 0.0f);
    variableCategory varType = returnVarType_givenTag(tag);
    switch (varType) {
        case variableCategory::action:{
            auto& actionVarVec = Nucleus.coldStorage.Data.actionVariables;
            auto iterator = std::find_if(actionVarVec.begin(), actionVarVec.end(), [&](actionVar& derActionVar){
                return derActionVar.return_tag() == tag;
            });
            if(iterator != actionVarVec.end()){
                return iterator->returnDomain();
            }else{
                return emptyDomain;
            }
        }
            break;
        case variableCategory::metric:{
            auto& metricVarVec = Nucleus.coldStorage.Data.metricVariables;
            auto iterator = std::find_if(metricVarVec.begin(), metricVarVec.end(), [&](metricVar& derMetricVar){
                return derMetricVar.return_tag() == tag;
            });
            return (iterator != metricVarVec.end()) ? iterator->returnDomain() : emptyDomain;
        }
            break;
        case variableCategory::synthetic: {
            return Nucleus.coldStorage.synthVarManager.returnDomainOfTaggedVar(tag);
        }
    }
    return emptyDomain;
}

//intended for testing purposes so loop isnt exactly fast
std::string cold_Storage::returnProgName_given_UserAlottedName(std::string UserAlottedName){
    auto& DataStruct = Nucleus.coldStorage.Data;
    for(int e = 0; e < DataStruct.actionVariables.size(); e++){
        if(DataStruct.actionVariables[e].return_name() == UserAlottedName){return DataStruct.actionVariables[e].return_program_name();}
    }
    for(int w = 0; w < DataStruct.metricVariables.size(); w++){
        if(DataStruct.metricVariables[w].return_name() == UserAlottedName){return DataStruct.metricVariables[w].return_program_name();}
    }
    throw std::invalid_argument("No such authentic variable with given user-alotted name exists!");
}


std::vector<uint16_t> cold_Storage::returnALL_TAGS(){
    std::vector<uint16_t> returnVec = Nucleus.coldStorage.synthVarManager.returnALL_SYNTH_TAGS();
    auto& DataStruct = Nucleus.coldStorage.Data;
    for(int q = 0; q < DataStruct.metricVariables.size(); q++){returnVec.push_back(DataStruct.metricVariables[q].return_tag());}
    for(int q = 0; q < DataStruct.actionVariables.size(); q++){returnVec.push_back(DataStruct.actionVariables[q].return_tag());}
    
    return returnVec;
}
