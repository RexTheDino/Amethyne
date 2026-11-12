//
//  Authentic_Vars_Creation.hpp
//  Amethyne
//
//  Created by AnshuDiya on 11/3/26.
//

#pragma once

#ifndef Authentic_Vars_Creation_hpp
#define Authentic_Vars_Creation_hpp

#endif /* Authentic_Vars_Creation_hpp */

#include <stdio.h>
#include <vector>
#include <random>
#include "Base_types.hpp"
#include <array>

//of type goal variable. multiple goal variables can exist but only one rules them all which is a composition of all the goal variables below. when aboslute max is being solved for, these minion goal variables are substituted for the relationships that have been relating to them. however, relationships between two goals and such where a goal is the x_tag CANNOT exist because by definition its a GOAL and not something that's supposed ot be optimized to pursue somethign else.
//there will be a vector of this struct inside var_storage and the goal variable to rule them all will simply sum all of them.
class GoalVariable{
private:
    struct Data{
        std::array<float, 2> rangeOfGoal;//the range this goal has been allotted.
        SymEngine::RCP<const SymEngine::Basic> ExpressionOfGoal;//the expression specified for this goal variable
    };
    std::string nameOfGoal;//name given by the user.
    std::string program_name;//program name given by the program.
public:
    inline SymEngine::RCP<const SymEngine::Basic> returnExpression();
    inline std::array<float, 2> returnDomain();
    inline GoalVariable(SymEngine::RCP<const SymEngine::Basic> expressionOfGoal, std::array<float, 2> rangeOfGoal);
};

struct range{
    float min = 0.0f;
    float max = 0.0f;
};

//goal func as a composition of all underling goals.
class MasterGoal{
    static std::vector<GoalVariable> Underlings;//all the underling goals as alloted by the user
    enum class status{
        updated,
        notUpdated//set by trasnfer manager and toggled back by this class when it has finished updating
    };
    range goalRange;
    //class addGoal--has passkey constructor only visible to transfer manager! because it constructs new symbols
    //class viewGoalFunc--hass passkey constructor only visible to heirarchy storage! because it requries heirarchy storage to check if it has been fully updated by transfer manager as well as this
public:
    inline void addGoal(SymEngine::RCP<const SymEngine::Basic> expressionOfGoal, range rangeOfGoal);
};

//DOES NOT automatically append to cold storage but is rather routed through transfer manager!!!
class varCreationAndUpdates{
public:
    void createMetric(std::string metric_name, varType typeOfVar, float minRange, float maxRange);//tag is auto-decided
        void updateMetric(void* dataPtr, std::string nameOfMetric);
    void createAction(std::string nameOfVar, float minDomain, float maxDomain);//tag is auto-decided
    void addGoal(std::string goalExpression, float minVal, float maxVal);
};
