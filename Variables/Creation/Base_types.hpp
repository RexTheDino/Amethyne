//
//  Base_types.hpp
//  Amethyne
//
//  Created by AnshuDiya on 11/3/26.
//

#pragma once

#ifndef Base_types_hpp
#define Base_types_hpp

#include <stdio.h>

//#include "Relations_Header.hpp"
#include <chrono>
#include <variant>
#include <symengine/expression.h>

struct domain{
    float min, max = 0.0f;
    domain(float provided_min, float provided_max);
};

enum class varType{
    string,
    data,
    array
};

class stringVar{//implement later if/when needed
    //inline void updateStr(std::string updatedStr, uint16_t tag, unsigned short indice);
};
//contains any numeric scalar data
class dataVar{//implement later if/when needed
public:
    //inline void updateData(float data, uint16_t tag);//updates data inside VariableData
};
//contains a pointer to the start of an array of floats or doubles. converts doubles to floats when updating data.
class arrayVar{//implement later if/when needed
    //inline void updateArrayVar(void* ptrToStart, bool isTypeDouble = false);//updates data
};

//the metric variables
class metricVar{
    std::variant<stringVar, dataVar, arrayVar> baseVarTypes;
    varType typeOfVarHeld;
    //tag is private since we dont want things to accidentally modify tag after it has been created
    uint16_t tag;//tag relating to the data indice this struct this is linked to.
    std::string nameOfMetric;//for ease of use. defined by user.
    std::string programName;//program name for var used for symEngine expressions.
    domain varDomain;//min_x and max_x of metric
    
    //REMOVE BOTTOM--VARIABLES MUST ONLY CONTAIN INFO PERTAINING TO THEMSELVES, NONE ABOUT OTHER VARIABLES!
    //SymEngine::RCP<const SymEngine::Basic> varExpr;//expression that relates this to other vars if such an expr exists
public:
    inline metricVar(std::string metric_name, varType typeOfVar, uint16_t theTag, domain derDomain, std::string programNameOfMetric);
    const inline uint16_t return_tag();//return tag but stuff cant modify it. either way should be able to find tag by position unless its not sorted
    inline std::string return_name();//returns name set by user
    const inline std::string return_program_name();//returns name set by program for symEngine expressions.
    const inline domain returnDomain();
    
    /*implement later if/when needed
    inline void updateData(char* charPtr);//throws exception if baseVar is NOT of enum type string
    inline void updateData(float* datPtr);//throws exception if baseVar IS of enum type string
     */
};

//action variable. but remember, all variable tags point to the same data array! tags NOT organized by order of variable category
//you may think action vars dont point to any data, but in fact, they do, they point to the data the model logs itself
class actionVar{
    std::string nameOfAction;//for ease-of-use and set by user
    uint16_t tag;//points to the tag of the data
    domain varDomain;//min_x_val and max_x_val of action variable
    std::string programName;//program name for var used for symEngne expressions.
public:
    inline actionVar(std::string nameOfVar, uint16_t tagOfVar, domain derDomain, std::string program_name_ofAction);
    const inline uint16_t return_tag();
    const inline std::string return_name();//USER-SET NAME NOT PROGRAM_NAME!
    const inline std::string return_program_name();
    const inline domain returnDomain();
};

#endif /* Base_types_hpp */
