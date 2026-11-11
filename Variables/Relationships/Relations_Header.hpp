//
//  Relations_Header.h
//  Amethyne
//
//  Created by AnshuDiya on 11/1/26.
//

//#pragma once


#ifndef Relations_Header_hpp
#define Relations_Header_hpp

/*
/*#ifdef DOUBLE_COMPLEX
 #undef DOUBLE_COMPLEX
 #endif*/

/*#ifdef DOUBLE_COMPLEX
#error "DOUBLE_COMPLEX was already defined before Accelerate"
#endif*/

#include <stdio.h>
#include <iostream>
#include <chrono>
#include <array>
#include <memory>
#include "Accelerate_initialization_data.h"
//#include <Accelerate/Accelerate.h>
#include <vector>
#include <cmath>



//#include "Initialization_Data_header.hpp"

enum class RelationshipType{//enum stores the relationship between two equally spaced datasets
    linear = 5,
    quadratic = 6,
    cubic = 7,
    null = 8
};

struct Dataset{//contains a dataset (2 vars and their relation)
    private:
        uint16_t yTag;
        uint16_t xTag;
        RelationshipType relationBetweenVars;
    
    public:
        Dataset(uint16_t tagX, uint16_t tagY, RelationshipType derRelation);//should be constructed using a ptr to the metal results buffer!
        const inline uint16_t returnYtag();
        const inline uint16_t returnXtag();
        const inline RelationshipType returnRelation();
};

//uses accelerate.sgels to actually construct a relationship
class Relation_Equation_Constructor{
    class organization{
    public:
        static inline void sort_by_tag(bool ascending = true, bool sort_by_y_tag = true);//sorts Dataset(s) in ascending order by y/x-tag or descending if set to false
    };
    struct sgels_data{
        const char trans = 'N';
        AccelInitDat acclDat;
        float* x_vals = nullptr; float* y_vals = nullptr;
            std::vector<float> work;
            inline std::vector<float>* call_sgels();
        sgels_data();//reserves space for vector
    };
    struct tags_and_relationships_return{
        unsigned int xTag;
        RelationshipType relationTo_Y_tag;
    };
private:
    static std::vector<tags_and_relationships_return> results;//should reserve 100 elements by default.
public:
    inline std::vector<tags_and_relationships_return>* calculateCoefficients(unsigned int yTag);
};
/*
//currently useless but add dispatch capabilities and semi-random generation of relation combinations to test later
class RelationDetector{//variables are assumed to be provided in float format with equal size
    
    struct logs{//logs important data
        static std::vector<Dataset>& DatasetsWithRelations; //logs datasets w/relations
    };
    
    //stores variables provided to this class that in the future may have their relationships tested or equations constructed for them
    /*struct VariableStorage{
            std::vector<VariableData>& variables;//references to variables stored in ascending order of their tag
            VariableStorage();
    } coldStorage;*/
    
    /*
public:
    //inline void storeVariables(std::vector<VariableData>& variablesToAdd);//stores variables in coldStorage in ascending order of their tag in coldStorage
    
    //returns the relationship between two variables' datasets and logs it if its linear/quadratic/cubic
    //RelationshipType relationReturn(VariableData var1, VariableData var2);
    void analyzeColdStorage();//schedules dispatcher to calculate relations between ALL variables in cold storage. this is done by returning an enum state notifying the dispatcher exactly what kind of metal func needs to be executed and writing the data needed for the metal kernel into the required buffers. the relations are then logged in logs. it also purges these variables at the end.
        //void logsEquationCalculation();//constructs equations for datasets with relationships stored in logs
    
};

/*struct VariableData{//contains data for a singular variable
    protected:
    std::vector<float>* data;
    public:
        unsigned int tag;//the tag assigned to this variable when it was created.
        inline void setData(std::vector<float>& varData);
        std::vector<float>* returnData(){return data;}//returns copy of this variable's data
    
};*/

/*struct Coordinate{
    float x;
    float y;
};*/

#endif /*Relations_Header_h*/
