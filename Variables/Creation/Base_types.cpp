//
//  Base_types.cpp
//  Amethyne
//
//  Created by AnshuDiya on 11/3/26.
//

#pragma once

#include "Base_types.hpp"

domain::domain(float provided_min, float provided_max): min{provided_min}, max{provided_max}{}

metricVar::metricVar(std::string metric_name, varType typeOfVar, uint16_t theTag, domain derDomain, std::string programNameOfMetric): varDomain{derDomain}, typeOfVarHeld{typeOfVar}, nameOfMetric{metric_name}, tag{theTag}, programName{programNameOfMetric}{}

const uint16_t metricVar::return_tag(){return tag;}

std::string metricVar::return_name(){return nameOfMetric;}//NOT program name!!!

const  std::string metricVar::return_program_name(){return programName;}

const  domain metricVar::returnDomain(){return varDomain;}


actionVar::actionVar(std::string nameOfVar, uint16_t tagOfVar, domain derDomain, std::string program_name_ofAction): nameOfAction{nameOfVar}, tag{tagOfVar}, varDomain{derDomain}, programName{program_name_ofAction}{}

const  uint16_t actionVar::return_tag(){return tag;}

const  std::string actionVar::return_name(){return nameOfAction;}

const  std::string actionVar::return_program_name(){return programName;}

const  domain actionVar::returnDomain(){return varDomain;}




