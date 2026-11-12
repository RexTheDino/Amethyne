//
//  Data_logs.hpp
//  Amethyne
//
//  Created by AnshuDiya on 11/10/26.
//

#pragma once

#include <iostream>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <span>

struct dataLog{
    float data;
    std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> timeStamp;
};
class tracker{//to track inputted data
    std::string nameOfVariableTracked;//the name given by the USER for the tracked variable BUT MODIFIED TO BE A PROGRAM NAME BY ADDING A "~" OUT FRONT
    std::vector<dataLog> logs;
public:
    tracker(std::string nameOf_varTracked);//the name entered here is modified to have a ~ in front by the manager; this IS THE PROGRAM NAME!
    inline void logData(float data);
};
class syntheticTracker{
    std::string program_name_ofVar;//program name assigned to this var
    std::vector<dataLog> logs;
public:
    syntheticTracker(std::string assignedProgramName);
    inline std::span<float> allotSpaceForNewData(uint32_t sizeOfNewData);//allots space for new data points by resizing and returns span to new space
};

struct synthDataUpdateInfo{
    std::span<float> dataToLog;
    uint16_t tagToLogFor;
};


class logsManager{
    static std::unordered_map<std::string, tracker> trackerMap;//only contains trackers for action/metric variables!
    static std::unordered_map<uint16_t, syntheticTracker> syntheticTrackerMap;//[tag, tracker]
    
public:
    //routed through transfer manager to first update cold storage
    static inline void addDataTracker(std::string nameOfTrackedVariable);
    static inline void addSyntheticTracker(uint16_t linkedTag);
    //also routed through transfer manager to convert different types of data into their appropriate float counterparts
    static inline void logDat(std::span<std::string> varName, std::span<float> data);//logs a new data point for ALL trackers, even ones not listed. whats not listed defaults to the previous value.
    static inline void logSyntheticData(std::span<synthDataUpdateInfo> dataUpdateMtdt);
    
    static inline std::span<dataLog> returnDataOfRequestedTracker(std::string program_name_of_var);
};
