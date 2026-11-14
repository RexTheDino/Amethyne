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



using namespace std;

//returns a span of all the datasets related to the requested tag. for example if y tag requested then span of all dataset with that tag as the y_tag
std::span<Dataset> testingGround::return_req_relationTag_related_datasets(uint16_t tag, bool sort_by_y_tag){
    if(testSynthTagSTATUS != datasetSynthTagConvertStatus::testSynthsTagsConverted){
        throw std::runtime_error("Datasets accessed before thier temporary synthetic tags have been converted!!!");
    }
    auto& dSetVec = Data::results;
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

int testingGround::return_yTag_of_dSet(uint16_t indexOfDset){
    if(indexOfDset >= Data::results.size()){throw std::runtime_error("Requested dataset indice exceeds results array size!");}
    return Data::results[indexOfDset].returnYtag();
}

int testingGround::return_xTag_of_dSet(uint16_t indexOfDset){
    if(indexOfDset >= Data::results.size()){throw std::runtime_error("Requested dataset indice exceeds results array size!");}
    return Data::results[indexOfDset].returnXtag();
}

RelationshipType testingGround::return_relation_of_dSet(uint16_t indexOfDset){
    if(indexOfDset >= Data::results.size()){throw std::runtime_error("Requested dataset indice exceeds results array size!");}
    return Data::results[indexOfDset].returnRelation();
}




variableCategory coldStorage::returnVarType_givenTag(uint16_t tag){
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

coldStorage::synthVarManager::synthVarManager(){
    resultVec.reserve(100);
}

void coldStorage::synthVarManager::sortSynthsByTag(){
    std::sort(syntheticVariables.begin(), syntheticVariables.end(), [&](synthVar& var1, synthVar& var2){
        return var1.returnTag() < var2.returnTag();
    });
}

std::vector<permanentSynthAddon>* coldStorage::synthVarManager::returnOpsForRequestedTag(const uint16_t tag){
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

std::string coldStorage::synthVarManager::return_programNameOf_taggedVar(const uint16_t tag){
    if(returnVarType_givenTag(tag) == variableCategory::synthetic){
        auto iteratorOfRequestedVar = std::find_if(syntheticVariables.begin(), syntheticVariables.end(), [&](synthVar& derSynth){
            return derSynth.returnTag() == tag;
        });
        return iteratorOfRequestedVar->returnName();
    }else{
        std::string nullStr = "";
        return nullStr;
    }
}

domain coldStorage::synthVarManager::returnDomainOfTaggedVar(const uint16_t tag){
    domain emptyDomain(0.0f, 0.0f);
    if(returnVarType_givenTag(tag) == variableCategory::synthetic){
        auto iterator = std::find_if(syntheticVariables.begin(), syntheticVariables.end(), [&](synthVar& derSynth){
            return derSynth.returnTag() == tag;
        });
        return (iterator != syntheticVariables.end()) ? iterator->returnDomain() : emptyDomain;
    }else{
        return emptyDomain;
    }
}

void coldStorage::synthVarManager::addSyntheticVariable(std::vector<permanentSynthAddon> correspondingAddons, synthVar derVariable, addSyntheticPasskey derPasskey){
    syntheticVariables.push_back(derVariable);
    syntheticAddons.insert(syntheticAddons.end(), correspondingAddons.begin(), correspondingAddons.end());
}

std::vector<uint16_t> coldStorage::synthVarManager::returnALL_SYNTH_TAGS(){
    std::vector<uint16_t> returnVec;
    for(int e = 0; e < syntheticVariables.size(); e++){returnVec.push_back(syntheticVariables[e].returnTag());}
    
    return returnVec;
}


void coldStorage::addActionVar(TransferManagerPasskey passkey, actionVar varToAdd){
    std::string userAlottedName = varToAdd.return_name();
    uint16_t tag = varToAdd.return_tag();
    UserAllotedNameAndTag returnStruct; returnStruct.UserAllotedName = userAlottedName; returnStruct.tag = tag;
    SymEngine::RCP<const SymEngine::Basic> symbolOfVar = SymEngine::symbol(varToAdd.return_program_name());
    variableMap.try_emplace(symbolOfVar, returnStruct);
    Data::actionVariables.push_back(varToAdd);
}
void coldStorage::addMetricVar(TransferManagerPasskey passkey, metricVar varToAdd){
    std::string userAllotedName = varToAdd.return_name();
    uint16_t tag = varToAdd.return_tag();
    UserAllotedNameAndTag returnStruct; returnStruct.UserAllotedName = userAllotedName; returnStruct.tag = tag;
    SymEngine::RCP<const SymEngine::Basic> symbolOfVar = SymEngine::symbol(varToAdd.return_program_name());
    variableMap.try_emplace(symbolOfVar, returnStruct);
    Data::metricVariables.push_back(varToAdd);
}

//returns empty str if var does not exist
std::string coldStorage::return_programNameOf_taggedVar(const uint16_t tag){
    std::string emptyStr = "";
    if(returnVarType_givenTag(tag) == variableCategory::action){//action variable
        auto iterator = std::find_if(Data::actionVariables.begin(), Data::actionVariables.end(), [&](actionVar& derActionVar){
            return derActionVar.return_tag() == tag;
        });
        if(iterator != Data::actionVariables.end()){
            return iterator->return_program_name();
        }else{
            return emptyStr;
        }
    }else if(returnVarType_givenTag(tag) == variableCategory::metric){
        auto iterator = std::find_if(Data::metricVariables.begin(), Data::metricVariables.end(), [&](metricVar& derMetricVar){
            return derMetricVar.return_tag() == tag;
        });
        return (iterator != Data::metricVariables.end()) ? iterator->return_program_name() : emptyStr;
    }else if(returnVarType_givenTag(tag) == variableCategory::synthetic){
        synthVarManager::return_programNameOf_taggedVar(tag);
    }else{
        throw std::runtime_error("Unrecognized variable tag!");
    }
        return emptyStr;
}

uint16_t coldStorage::returnTag_givenVarSymbol(SymEngine::RCP<const SymEngine::Basic> varSymbol){
    auto variableIterator = variableMap.find(varSymbol);
    if(variableIterator != variableMap.end()){
        uint16_t variableTag = (variableIterator->second).tag;
        return variableTag;
    }else{
        throw std::invalid_argument("Symbol not stored in unordered map!!!");
    }
}

domain coldStorage::returnDomain(uint16_t tag){
    domain emptyDomain(0.0f, 0.0f);
    variableCategory varType = returnVarType_givenTag(tag);
    switch (varType) {
        case variableCategory::action:{
            auto iterator = std::find_if(Data::actionVariables.begin(), Data::actionVariables.end(), [&](actionVar& derActionVar){
                return derActionVar.return_tag() == tag;
            });
            if(iterator != Data::actionVariables.end()){
                return iterator->returnDomain();
            }else{
                return emptyDomain;
            }
        }
            break;
        case variableCategory::metric:{
            auto iterator = std::find_if(Data::metricVariables.begin(), Data::metricVariables.end(), [&](metricVar& derMetricVar){
                return derMetricVar.return_tag() == tag;
            });
            return (iterator != Data::metricVariables.end()) ? iterator->returnDomain() : emptyDomain;
        }
            break;
        case variableCategory::synthetic: {
            return synthVarManager::returnDomainOfTaggedVar(tag);
        }
    }
    return emptyDomain;
}


std::vector<uint16_t> coldStorage::returnALL_TAGS(){
    std::vector<uint16_t> returnVec = synthVarManager::returnALL_SYNTH_TAGS();
    for(int q = 0; q < Data::metricVariables.size(); q++){returnVec.push_back(Data::metricVariables[q].return_tag());}
    for(int q = 0; q < Data::actionVariables.size(); q++){returnVec.push_back(Data::actionVariables[q].return_tag());}
    
    return returnVec;
}
