//
//  Relations_Header.h
//  Amethyne
//
//  Created by AnshuDiya on 11/1/26.
//

#pragma once


/*
/*#ifdef DOUBLE_COMPLEX
 #undef DOUBLE_COMPLEX
 #endif*/

/*#ifdef DOUBLE_COMPLEX
#error "DOUBLE_COMPLEX was already defined before Accelerate"
#endif*/

#include <iostream>
#include <array>
#include "Accelerate_initialization_data.h"
#include <vector>
#include <cmath>
#include <symengine/expression.h>
#include <span>

class Dataset;


//#include "Initialization_Data_header.hpp"

enum class RelationshipType: uint16_t{//enum stores the relationship between two equally spaced datasets
    linear = 5,
    quadratic = 6,
    cubic = 7,
    null = 8
};

struct Dataset{//contains a dataset (2 vars and their relation)
    private:
        uint16_t xTag;
        uint16_t yTag;
        RelationshipType relationBetweenVars;
    
    public:
    //IMPORTANT--TEST_SYNTH TAGS ARE NOT THE SAME AS REGULAR VAR TAGS!!! THE TAG PROVIDED TO DATASET MUST HAVE BEEN CONVERTED APPROPRIATELY!
        Dataset(uint16_t tagX, uint16_t tagY, RelationshipType derRelation);//should be constructed using a ptr to the metal results buffer!
        const uint16_t returnYtag();
        const uint16_t returnXtag();
        const RelationshipType returnRelation();
};

//uses accelerate.sgels to actually construct a relationship
class Relation_Equation_Constructor{
        inline static std::vector<float> matriceData;
            inline static std::vector<float> functionWork;//essentially scratch work
    
    class organization{
    public:
        static void sort_by_tag(bool ascending = true, bool sort_by_y_tag = true);//sorts Dataset(s) in ascending order by y/x-tag or descending if set to false
    };
    struct sgels_data{
        const char trans = 'N';
        AccelInitDat acclDat;
        float* x_vals = nullptr; float* y_vals = nullptr;
            std::vector<float> work;
            std::vector<float>* call_sgels();
        sgels_data();//reserves space for vector
    };
    
private:
    inline static std::vector<SymEngine::RCP<const SymEngine::Basic>> systemOfEquations;//contains the system of equatons that could define a single variable's relations
    
    static std::span<SymEngine::RCP<const SymEngine::Basic>> returnSysOfEqns(const std::span<Dataset> relatedDSETS);//given all the relations to a certain tag, return a system of equations with the relations included relating to that tag
            static void addVariableDataTo_SGELSY_matrice(uint16_t tagOfVar1, uint16_t powerOfVar1, uint16_t tagOfVar2, uint16_t powerOfVar2);//adds the data of the requested tags to the sgels matrice
public:
    static SymEngine::RCP<const SymEngine::Basic> calculateCoefficients(unsigned int yTag);//return expression relating variable to all others
};

