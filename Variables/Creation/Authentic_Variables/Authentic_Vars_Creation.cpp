//
//  Authentic_Vars_Creation.cpp
//  Amethyne
//
//  Created by AnshuDiya on 11/3/26.
//


#include "Authentic_Vars_Creation.hpp"
#include "Transfer_Manager.hpp"

void varCreation::createAction(std::string nameOfVar, float minDomain, float maxDomain){
    variableCategory typeOfVar = variableCategory::action;
    uint16_t tag = tagAssigner::assignTag(typeOfVar);
    std::string prog_name = synthetic_parser::varNameReturn(tag);
    domain derDomain(minDomain, maxDomain);
    
    actionVar derAction(nameOfVar, tag, derDomain, prog_name);
    TransferManager::transferAuthentic(derAction);
}
void varCreation::createMetric(std::string metric_name, float data, float minRange, float maxRange){
    variableCategory typeOfVar = variableCategory::metric;
    uint16_t tag = tagAssigner::assignTag(typeOfVar);
    std::string prog_name = synthetic_parser::varNameReturn(tag);
    domain derDomain(minRange, maxRange);
    
    metricVar derMetric(metric_name, tag, derDomain, prog_name);
    TransferManager::transferAuthentic(derMetric);
}
