//
//  Data_logs.cpp
//  Amethyne
//
//  Created by AnshuDiya on 11/10/26.
//

#include "Data_logs.hpp"
#include "Symbolic_representation_logic.hpp"
#include "Var_Storage.hpp"


tracker::tracker(std::string nameOf_varTracked){
    if(nameOf_varTracked == ""){throw std::invalid_argument("Name of tracked variable is an empty string!");}
    nameOfVariableTracked = nameOf_varTracked;
    logs.reserve(10000);
}

std::span<dataLog> tracker::returnData(){std::span<dataLog> returnSpan(logs.begin(), logs.end()); return returnSpan;}

void tracker::logData(float data){
    assert(logs.size() <= RelationConstructionMetadata::points_per_var);//size of logs is always less than or equal to points_per_var.
    std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> currentTime = std::chrono::high_resolution_clock::now();
    dataLog derLog; derLog.timeStamp = currentTime; derLog.data = data;
    if(data == -69.420f){data = logs[logs.size()-1].data;}//if data point provided is -69.420, then set to previous val
    if(cache.size() < RelationConstructionMetadata::points_per_var){
        cache.push_back(derLog);
    }else{
        std::transform(cache.begin(), cache.end(), logs.begin(), logs.begin(), [&](dataLog& cacheLog, dataLog& dLog){
            dataLog returnLog; returnLog.timeStamp = currentTime;
            returnLog.data = ((cacheLog.data + dLog.data)/2.0f);
            return returnLog;//return the average of the cache and the storage log
        });
        cache.clear();
    }
}

syntheticTracker::syntheticTracker(std::string assignedProgramName): program_name_ofVar{assignedProgramName}{}

void syntheticTracker::logData(float data){
    assert(logs.size() <= RelationConstructionMetadata::points_per_var);//size of logs is always less than or equal to points_per_var.
    std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> currentTime = std::chrono::high_resolution_clock::now();
    dataLog derLog; derLog.timeStamp = currentTime; derLog.data = data;
    if(data == -69.420f){data = logs[logs.size()-1].data;}//if data point provided is -69.420, then set to previous val
    if(cache.size() < RelationConstructionMetadata::points_per_var){
        cache.push_back(derLog);
    }else{
        std::transform(cache.begin(), cache.end(), logs.begin(), logs.begin(), [&](dataLog& cacheLog, dataLog& dLog){
            dataLog returnLog; returnLog.timeStamp = currentTime;
            returnLog.data = ((cacheLog.data + dLog.data)/2.0f);
            return returnLog;//return the average of the cache and the storage log
        });
        cache.clear();
    }
}

std::span<dataLog> syntheticTracker::returnData(){std::span<dataLog> returnSpan(logs.begin(), logs.end()); return returnSpan;}

void logsManager::addDataTracker(std::string nameOfTrackedVariable){
    nameOfTrackedVariable.insert(0, "~");//THIS IS THE PROGRAM NAME!!!
    tracker newTracker(nameOfTrackedVariable);
    trackerMap.try_emplace(nameOfTrackedVariable, newTracker);
}

void logsManager::addSyntheticTracker(uint16_t linkedTag){
    std::string nameForSynthTracker = synthetic_parser::varNameReturn(linkedTag);//returns name for the synthetic variable
    syntheticTracker newSyntheticTracker(nameForSynthTracker);
    syntheticTrackerMap.try_emplace(linkedTag, newSyntheticTracker);
}

void logsManager::logDat(std::span<std::string> varName, std::span<float> data){
    if((varName.empty())||(data.empty())){throw std::invalid_argument("Empty variable string span or data span provided!");}
    if((varName.size())!=(data.size())){throw std::invalid_argument("Variable name and variable data array not of same size!");}
    std::unordered_map<std::string, tracker*> foundKeys;//found keys recorded
    for (int b = 0; b < varName.size(); b++) {
        auto iterator = trackerMap.find(varName[b]);
        if(iterator != trackerMap.end()){
            iterator->second.logData(data[b]);
            foundKeys[varName[b]] = &iterator->second;//record what keys have already been logged
        }else{
            SymEngine::RCP<const SymEngine::Basic> varSymbol = SymEngine::symbol(varName[b]);
            uint16_t tagOfVar = coldStorage::returnTag_givenVarSymbol(varSymbol);
            auto synthIterator = syntheticTrackerMap.find(tagOfVar);
            if(synthIterator != syntheticTrackerMap.end()){
                (synthIterator->second).logData(data[b]);
            }else{
                throw std::invalid_argument("Variable name provided for a variable that does not exist!!!");
            }
        }
    }
    for(const auto& [key, value] : trackerMap){
        auto iterator = foundKeys.find(key);
        if(iterator == foundKeys.end()){
            auto trackerToModify = trackerMap.find(key);
            trackerToModify->second.logData(-69.420f);
        }
    }
}

/*void logsManager::logSyntheticData(std::span<synthDataUpdateInfo> dataUpdateMtdt){
    int sizeOfPreviousUpdate = (int)dataUpdateMtdt[0].dataToLog.size();
    for (int v = 0; v < dataUpdateMtdt.size(); v++) {
        auto iterator = syntheticTrackerMap.find(dataUpdateMtdt[v].tagToLogFor);
        if(iterator != syntheticTrackerMap.end()){
            int sizeOfNewUpdate = (int)dataUpdateMtdt[v].dataToLog.size();
                //if new update size differs from previous update, throw error. else set previous update size to this update's size
                sizeOfNewUpdate != sizeOfPreviousUpdate ? throw std::invalid_argument("Synthetic data update sizes do not match!") : sizeOfPreviousUpdate = sizeOfNewUpdate;
            auto spaceToCopyDatTo = iterator->second.allotSpaceForNewData(sizeOfNewUpdate);//returns a std::span
            if(spaceToCopyDatTo.size() != sizeOfNewUpdate){throw std::runtime_error("Allotted space for data in synth tracker doesnt match requested size!");}
            auto& vecToCopyFrom = dataUpdateMtdt[v].dataToLog;
            std::copy(vecToCopyFrom.begin(), vecToCopyFrom.end(), spaceToCopyDatTo.begin());//copies data to tracker
        }
    }
}*/

std::span<dataLog> logsManager::returnDataOfRequestedTracker(std::string program_name_of_var){
    SymEngine::RCP<const SymEngine::Basic> varSymbol = SymEngine::symbol(program_name_of_var);
    uint16_t varTag = coldStorage::returnTag_givenVarSymbol(varSymbol);
    if((coldStorage::returnVarType_givenTag(varTag) == variableCategory::action) || (coldStorage::returnVarType_givenTag(varTag) == variableCategory::metric)){
        auto derTrackerIterator = trackerMap.find(program_name_of_var);
        assert(derTrackerIterator != trackerMap.end());
        tracker derTracker = derTrackerIterator->second;
        return derTracker.returnData();
    }else if(coldStorage::returnVarType_givenTag(varTag) == variableCategory::synthetic){
        auto trackerIterator = syntheticTrackerMap.find(varTag);
        assert(trackerIterator != syntheticTrackerMap.end());
        syntheticTracker theTracker = trackerIterator->second;
        return theTracker.returnData();
    }else{
        throw std::invalid_argument("Variable with given program name does not exist!");
    }
}
