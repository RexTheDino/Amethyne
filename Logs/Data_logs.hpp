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
#include "Metadata.h"

struct trackerTester;

struct progNameAndDat{
    std::string progName;
    float data;
    progNameAndDat(std::string theProgramName, float theData);
};


struct dataLog{
    float data;
    std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> timeStamp;
};
class tracker{//to track inputted data
    friend struct trackerTester;
    
    std::string nameOfVariableTracked;//the name given by the USER for the tracked variable BUT MODIFIED TO BE A PROGRAM NAME BY ADDING A "~" OUT FRONT
    std::vector<dataLog> logs;
    std::vector<dataLog> cache;//to be filled with points that are averaged out and written to logs once they reach a multiple of points_per_var.
public:
    tracker(std::string nameOf_varTracked);//the name entered here is modified to have a ~ in front by the manager; this IS THE PROGRAM NAME!
    std::span<dataLog> returnData();
    void logData(float data);
};
class syntheticTracker{
    std::string program_name_ofVar;//program name assigned to this var
    std::vector<dataLog> logs;
    std::vector<dataLog> cache;//to be filled with points that are averaged out and written to logs once they reach a multiple of points_per_var.
public:
    syntheticTracker(std::string assignedProgramName);
    //std::span<float> allotSpaceForNewData(uint32_t sizeOfNewData);//allots space for new data points by resizing and returns span to new space
    void logData(float data);
    std::span<dataLog> returnData();
};

struct synthDataUpdateInfo{
    std::span<float> dataToLog;
    uint16_t tagToLogFor;
};


class LOGS_MANAGER{
    friend struct trackerTester;
    
    std::unordered_map<std::string, tracker> trackerMap;//only contains trackers for action/metric variables! stores by program_name
    std::unordered_map<uint16_t, syntheticTracker> syntheticTrackerMap;//[tag, tracker]
    
public:
    //routed through transfer manager to first update cold storage
    void addDataTracker(std::string nameOfTrackedVariable);
    void addSyntheticTracker(uint16_t linkedTag);
    //also routed through transfer manager to convert different types of data into their appropriate float counterparts
    void logDat(std::span<progNameAndDat> varNamesAndData);//logs a new data point for ALL trackers, even ones not listed. whats not listed defaults to the previous value.
    //static void logSyntheticData(std::span<synthDataUpdateInfo> dataUpdateMtdt);
    
    std::span<dataLog> returnDataOfRequestedTracker(std::string program_name_of_var);
};
