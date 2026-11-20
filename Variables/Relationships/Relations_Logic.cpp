//
//  Relations_Logic.cpp
//  Amethyne
//
//  Created by AnshuDiya on 11/1/26.
//

#pragma once

#include "Relations_Header.hpp"

#include <numeric>
#include <algorithm>
#include "Symbolic_representation_logic.hpp"
#include "Var_Storage.hpp"
#include "Metadata.h"
#include "Data_logs.hpp"
#include "Nucleus.hpp"

using namespace std;
/*
//sorts the datasets in var storage by tag
void Relation_Equation_Constructor::organization::sort_by_tag(bool ascending, bool sort_by_y_tag){
    
    auto& dSetVec = testingGround::results;
    std::sort(dSetVec.begin(), dSetVec.end(), [sort_by_y_tag, ascending](const Dataset& dSet1, const Dataset& dSet2){
        if(sort_by_y_tag){
            if(ascending){
                return dSet1.returnYtag() < dSet2.returnYtag();
            }else{
                return dSet1.returnYtag() > dSet2.returnYtag();
            }
        }else{
            if(ascending){
                return dSet1.returnXtag() < dSet2.returnXtag();
            }else{
                return dSet1.returnXtag() > dSet2.returnXtag();
            }
        }
    });
}*/



Dataset::Dataset(uint16_t tagX, uint16_t tagY, RelationshipType derRelation): xTag{tagX}, yTag{tagY}, relationBetweenVars{derRelation}{}

Dataset::Dataset(){xTag = 69; yTag = 69; relationBetweenVars = RelationshipType::sentinel;}

const uint16_t Dataset::returnYtag() const{return yTag;}

const uint16_t Dataset::returnXtag() const{return xTag;}

const RelationshipType Dataset::returnRelation() const{return relationBetweenVars;}

vector<float>* Relation_Equation_Constructor::sgels_data::call_sgels(){
    float queryResult = 0.0f;
    char noTranspose = trans;
    sgels_(&noTranspose, &acclDat.num_pts, &acclDat.num_unknown_coeffs, &acclDat.nrhs, x_vals, &acclDat.lead_dimension_x_vals, y_vals, &acclDat.lead_dimension_y_vals, &queryResult, &acclDat.lwork, &acclDat.info);
    if(acclDat.info != 0){throw std::runtime_error("couldnt query size of wkspace!");}
    acclDat.lwork = static_cast<__CLPK_integer>(ceil(queryResult));
    work.resize(acclDat.lwork);
    sgels_(&noTranspose, &acclDat.num_pts, &acclDat.num_unknown_coeffs, &acclDat.nrhs, x_vals, &acclDat.lead_dimension_x_vals, y_vals, &acclDat.lead_dimension_y_vals, work.data(), &acclDat.lwork, &acclDat.info);
    return &work;
}

//still needs to set the x/y vals ptr as well as fill data!
SymEngine::RCP<const SymEngine::Basic> Relation_Equation_Constructor::calculateCoefficients(unsigned int yTag){
    if(Nucleus.coldStorage.returnVarType_givenTag(yTag) == variableCategory::action){throw std::invalid_argument("Action variable cannot be related to anything!");}
    
    calculate_SGELSY sgelsy_calc;
        auto& initDat = sgelsy_calc.initDat;
    std::span<Dataset> relatedDatasets = Nucleus.coldStorage.returnDatasets_relatedTo_tag(yTag);//Nucleus.testingGround.return_req_relationTag_related_datasets(yTag);//modify later to request tags from cold storage
    if(relatedDatasets.size() <= 0){SymEngine::RCP<const SymEngine::Basic> nullexpr; return nullexpr;}
    auto systemOfEqns = returnSysOfEqns(relatedDatasets);//ALSO FILLS MATRIX WITH REQ POINTS!
    
        int calculatedRank;
        float rcond = 0.001f;
    initDat.numPts = RelationConstructionMetadata::points_per_var_eqn_construct_calculation_SGELSY;
    initDat.numTerms = (int)systemOfEquations.size();//the number of terms in the matrix
    initDat.writeBackRankTo = &calculatedRank;
    initDat.epsilonThreshold = &rcond;
    initDat.numEqnsToSolve = 1;//hardcoded, change later if needed
    
    std::string nameOfVarBeingSolvedFor = synthetic_parser::varNameReturn(yTag);
    auto iteratorToDataOfVarBeingSolvedFor = Nucleus.logsManager.returnDataOfRequestedTracker(nameOfVarBeingSolvedFor);
    std::vector<float> resultsVec(iteratorToDataOfVarBeingSolvedFor.size());
    //copy data points of var being solved for to the results vec so sgelsy can access it and overwrite with coefficients
    std::transform(iteratorToDataOfVarBeingSolvedFor.begin(), iteratorToDataOfVarBeingSolvedFor.end(), resultsVec.begin(), [](dataLog& dLog){
        return dLog.data;
    });
    initDat.ptrToOutputVec = resultsVec.data();
    initDat.ptrToInputVec = matriceData.data();
    
    sgelsy_calc.primeTheCalculator();
    sgelsy_calc.call_sgelsy();
    
    using namespace SymEngine;
    RCP<const Basic> resultExpr = integer(0);
    for(int q = 0; q < initDat.numTerms * initDat.numEqnsToSolve; q++){
        RCP<const Basic> modifiedTerm = mul(systemOfEqns[q], real_double(resultsVec[q]));
        resultExpr = add(resultExpr, modifiedTerm);
    }
    
    matriceData.clear();//super important
    return resultExpr;
}

using namespace std;

 // *IMPORTANT BUG--CURRENTLy ONLY PULLS DATASETS FROM TESTING GROUND OF NEWLY CREATED SYNTH VARS. BUT ALSO NEEDS TO PULL OLD DATASETS. FIX!!!*
//a system of equations ofr all the variables related to a single variable since all y-tags in dsets are supposed to be the same. only x-tags differ
span<SymEngine::RCP<const SymEngine::Basic>> Relation_Equation_Constructor::returnSysOfEqns(const std::span<Dataset> relatedDSETS){
    systemOfEquations.clear();
    /*if(relatedDSETS.size() > 8){
        std::uniform_int_distribution<int> selectRandDSet(0, (int)relatedDSETS.size()-1);
        
        throw std::runtime_error("COMBINATORIAL EXPLOSION OF NUMBER OF VARIABLES!");
        
    }*/
    using namespace SymEngine;
    for(int b = 0; b < (relatedDSETS.size() <= 8 ? relatedDSETS.size() : 8); b++){//if more than 8 dsets just choose first 
        int numTimesToRunBasedOnPow;
        switch (relatedDSETS[b].returnRelation()) {
            case RelationshipType::linear:
                numTimesToRunBasedOnPow = 1;
                break;
            case RelationshipType::quadratic:
                numTimesToRunBasedOnPow = 2;
                break;
            case RelationshipType::cubic:
                numTimesToRunBasedOnPow = 3;
                break;
                
            default:
                throw std::runtime_error("Relation_constructor asked to consturct an equation between variables that do NOT share a relaionship!");
                break;
        }
        RCP<const Basic> dominantVariable = symbol(synthetic_parser::varNameReturn(relatedDSETS[b].returnXtag()));
        for(int h = b+1; h < (relatedDSETS.size() <= 8 ? relatedDSETS.size() : 8); h++){
            RCP<const Basic> twoTermPair = integer(0);//all the combinations of powers between two terms
            RCP<const Basic> nonDominantVariable = symbol(synthetic_parser::varNameReturn(relatedDSETS[h].returnXtag()));//all y-tags in the datasets should be the same
            int maxDegreeOfNonDominant;
            switch (relatedDSETS[h].returnRelation()) {
                case RelationshipType::linear:
                    maxDegreeOfNonDominant = 1;
                    break;
                case RelationshipType::quadratic:
                    maxDegreeOfNonDominant = 2;
                    break;
                case RelationshipType::cubic:
                    maxDegreeOfNonDominant = 3;
                    break;
                    
                default:
                    throw std::runtime_error("Relation_constructor asked to consturct an equation between variables that do NOT share a relaionship!");
                    break;
            }
            for(int k = 1; k <= numTimesToRunBasedOnPow; k++){
                for(int r = 1; r <= maxDegreeOfNonDominant; r++){
                    twoTermPair = mul(pow(dominantVariable, integer(k)), pow(nonDominantVariable, integer(r)));
                    systemOfEquations.push_back(twoTermPair);
                    //k and r only go from 1 to 3
                    addVariableDataTo_SGELSY_matrice(relatedDSETS[b].returnXtag(), (uint16_t)k, relatedDSETS[h].returnXtag(), (uint16_t)r);
                }
            }
        }
        
        for(int p = 1; p <= numTimesToRunBasedOnPow; p++){
            RCP<const Basic> singleVariable = pow(dominantVariable, integer(p));
            systemOfEquations.push_back(singleVariable);
            addVariableDataTo_SGELSY_matrice(relatedDSETS[b].returnXtag(), (uint16_t)p, 0, 0);//dont fill var 2 obviously cuz single-var term
        }
    }
    std::span<RCP<const Basic>> sysEqns(systemOfEquations.begin(), systemOfEquations.end());
    return sysEqns;
}

//if no var 2, just have the power be 0 and/or the tag be 0
//called whenever the new terms are created
void Relation_Equation_Constructor::addVariableDataTo_SGELSY_matrice(uint16_t tagOfVar1, uint16_t powerOfVar1, uint16_t tagOfVar2, uint16_t powerOfVar2){
    std::string progNameVar1 = synthetic_parser::varNameReturn(tagOfVar1);
    std::string progNameVar2 = synthetic_parser::varNameReturn(tagOfVar2);
        auto var1DataSpan = Nucleus.logsManager.returnDataOfRequestedTracker(progNameVar1);
    std::span<dataLog> var2DataSpan;
    if(powerOfVar2 != 0){//if calculating for multipel variables then request var 2 data span.
        auto var2DataSpan = Nucleus.logsManager.returnDataOfRequestedTracker(progNameVar2);
    }
    
    int sizeOfDat = RelationConstructionMetadata::points_per_var_eqn_construct_calculation_SGELSY;
            functionWork.clear();
            functionWork.reserve(sizeOfDat);
    if(powerOfVar2 != 0){
        //transfers var 2's data over to function scratch work
        std::transform(var2DataSpan.begin(), var2DataSpan.end(), std::back_inserter(functionWork), [&](dataLog& dLog){
            return dLog.data;
        });
    }
    int strideSize = int(var1DataSpan.size()/sizeOfDat);
    //if matrix does not have enough capacity to hold new data, reserve enough
    if((matriceData.capacity()-matriceData.size()) <= sizeOfDat){matriceData.reserve(matriceData.capacity()+sizeOfDat+1);}
        int oldDataEndIndex = (int)matriceData.size();
    if(powerOfVar1 != 0){
        for(int k = 0; k < var1DataSpan.size(); k+=strideSize){
            float averageVal = var1DataSpan[k].data;
            for(int f = 1; f < strideSize; f++){
                averageVal += var1DataSpan[k+f].data;
            }
            averageVal /= strideSize;
            matriceData.push_back(averageVal);
        }
    } else{
        matriceData.resize(matriceData.size()+sizeOfDat,1.0f);//if exponent is 0 then fill with 1 starting from the new data slots.
    }
    std::vector<float> varPow((float)powerOfVar1, sizeOfDat);
    //now raise that array to the required power
    switch (powerOfVar1) {
        case 2:
            vDSP_vsq(&matriceData[oldDataEndIndex], 1, &matriceData[oldDataEndIndex], 1, sizeOfDat);//squares existing data
            break;
        case 3:
            vvpowsf(&matriceData[oldDataEndIndex], varPow.data(), &matriceData[oldDataEndIndex], &sizeOfDat);//cubes existing data
            break;
        default://dont do anything with a power of 1
            break;
    }
    
    std::fill(varPow.begin(), varPow.end(), (float)powerOfVar2);
    if(powerOfVar2 != 0){//no need to fill with 1's since if exponent was 0 for both this and var1 it was already filled
        for(int e = 0; e < powerOfVar2; e++){
            vDSP_vmul(functionWork.data(), 1, &matriceData[oldDataEndIndex], 1, &matriceData[oldDataEndIndex], 1, sizeOfDat);
        }
    }
}

















//void VariableData::setData(std::vector<float>& varData){(*data) = varData;}

//inserts the variables listed and resorts the array of variables in coldStorage
/* void RelationDetector::storeVariables(std::vector<VariableData>& variablesToAdd){
    auto& storageVec = coldStorage.variables;
    storageVec.insert(storageVec.end(), variablesToAdd.begin(),variablesToAdd.end());
    std::sort(storageVec.begin(),storageVec.end(),[](const VariableData& var1, const VariableData& var2){
        return var1.tag < var2.tag;
    });
}*/

/*//both variables logged in relation to time
RelationshipType RelationDetector::relationReturn(VariableData var1, VariableData var2){
    auto& vec1 = *var1.returnData();
    auto& vec2 = *var2.returnData();
    if(vec1.size() != vec2.size()){
        throw std::invalid_argument("Variable data sizes do not match for relation-checking!");
    } else if (vec1.size() < 4){
        throw std::invalid_argument("Not enough elements to calculate upto third diff!");
    }
    std::vector<Coordinate> Coordinates = std::vector<Coordinate>(vec1.size());//intiializes an array of coordinate points corresponding to the number of coordinates there are
    for (int u = 0; u < vec1.size(); u++) {
        //sorts the two variable points into coordinates
        Coordinates[u].x = vec1[u];
        Coordinates[u].y = vec2[u];
    }
    //sorts the coordinates in ascending order of x
    std::sort(Coordinates.begin(), Coordinates.end(), [](const Coordinate& coord1, const Coordinate& coord2){
        return coord1.x < coord2.x;
    });
    std::vector<float> DiffsVec1; DiffsVec1.resize(vec1.size());
    std::vector<float> DiffsVec2; DiffsVec2.resize(vec2.size());
    std::vector<float> yOverX; yOverX.resize(vec1.size());
    for (int g = 0; g < DiffsVec1.size(); g++) {
        DiffsVec1[g] = Coordinates[g].x;
        DiffsVec2[g] = Coordinates[g].y;
    }
    for (int n = 0; n < 3; n++) {
            //if first differences are constant, then return a linear relationship
            std::adjacent_difference(DiffsVec1.begin(),DiffsVec1.end(), DiffsVec1.begin());
                DiffsVec1.erase(DiffsVec1.begin());
            std::adjacent_difference(DiffsVec2.begin(), DiffsVec2.end(), DiffsVec2.begin());
                DiffsVec2.erase(DiffsVec2.begin());
        
        yOverX.resize(DiffsVec2.size());
        std::transform(DiffsVec1.begin(), DiffsVec1.end(), DiffsVec2.begin(), yOverX.begin(), std::divides<>());
        
        float meanOfSet = 0.0f;
        float stdDev = 0.0f;
        for (int p = 0; p < yOverX.size(); p++) {
            if(!std::isnan(yOverX[p])){
                meanOfSet += yOverX[p];
            }
        }
        meanOfSet /= yOverX.size();
        
        for (int j = 0; j < yOverX.size(); j++) {
            float addThis = std::pow(yOverX[j]-meanOfSet,2.0f);
            if(!std::isnan(addThis)){stdDev += addThis;}
        }
        stdDev /= DiffsVec1.size();
        stdDev = std::sqrt(stdDev);
            //replace using range as a metric. its a TERRIBLE metric. replace w/mean!!!
            //float maxRange = *(std::max_element(yOverX.begin(),yOverX.end()));
            //float minRange = *(std::min_element(yOverX.begin(), yOverX.end()));
        //float range = maxRange-minRange;
        //if(range < RelationsMetadata::epsilon){range = RelationsMetadata::epsilon;}
        if(std::abs(meanOfSet) < std::abs(RelationsMetadata::epsilon)){meanOfSet = RelationsMetadata::epsilon;}
        float ratio = stdDev/meanOfSet;//range;
        if (std::abs(ratio) < RelationsMetadata::cutoffThresholdForVariableRelation) {
            switch (n) {
                case 0:
                    return RelationshipType::linear;
                    break;
                case 1:
                    return RelationshipType::quadratic;
                    break;
                case 2:
                    return RelationshipType::cubic;
                    break;
            }
        }
    }
    return RelationshipType::null;
}
*/
