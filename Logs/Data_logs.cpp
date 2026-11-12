//
//  Data_logs.cpp
//  Amethyne
//
//  Created by AnshuDiya on 11/10/26.
//

#include "Data_logs.hpp"
#include "Symbolic_representation_logic.hpp"


tracker::tracker(std::string nameOf_varTracked){
    if(nameOf_varTracked == ""){throw std::invalid_argument("Name of tracked variable is an empty string!");}
    nameOfVariableTracked = nameOf_varTracked;
    logs.reserve(10000);
}

void tracker::logData(float data){
    std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> currentTime = std::chrono::high_resolution_clock::now();
    dataLog derLog; derLog.timeStamp = currentTime; derLog.data  = data;
    if(data == -69.420f){data = logs[logs.size()-1].data;}//if data point provided is -69.420, then set to previous val
    logs.push_back(derLog);
}

syntheticTracker::syntheticTracker(std::string assignedProgramName): program_name_ofVar{assignedProgramName}{}

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
            throw std::invalid_argument("Variable name provided for a variable that does not exist!!!");
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

void logsManager::logSyntheticData(std::span<synthDataUpdateInfo> dataUpdateMtdt){
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
}
