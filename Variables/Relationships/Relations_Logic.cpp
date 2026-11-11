//
//  Relations_Logic.cpp
//  Amethyne
//
//  Created by AnshuDiya on 11/1/26.
//

#include "Relations_Header.hpp"

#include <numeric>
#include <algorithm>
#include "Var_Storage.hpp"
#include "Metadata.h"

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

const inline uint16_t Dataset::returnYtag(){return yTag;}

const inline uint16_t Dataset::returnXtag(){return xTag;}

const inline RelationshipType Dataset::returnRelation(){return relationBetweenVars;}

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

vector<Relation_Equation_Constructor::tags_and_relationships_return>* Relation_Equation_Constructor::calculateCoefficients(unsigned int yTag){
    
    organization::sort_by_tag();
    
    int startIndice = testingGround::return_req_relationTag_indice(yTag);
    
    sgels_data sgelMtdt;
    sgelMtdt.acclDat.num_pts = RelationConstructionMetadata::points_per_var;
    sgelMtdt.acclDat.lead_dimension_x_vals = sgelMtdt.acclDat.num_pts;
    
    int num_x_variables = 0;
    int thisYtag = testingGround::return_yTag_of_dSet(startIndice);
    //while we remain on this tag keep countin up
    while(thisYtag == testingGround::return_yTag_of_dSet(startIndice)){
        num_x_variables++;
        thisYtag = testingGround::return_yTag_of_dSet(startIndice+num_x_variables);//next indice
    }
    
    sgelMtdt.acclDat.lead_dimension_y_vals = max(sgelMtdt.acclDat.lead_dimension_x_vals, num_x_variables);
    sgelMtdt.acclDat.num_unknown_coeffs = num_x_variables;
    sgelMtdt.acclDat.nrhs = 1;//could change later
    
    vector<float>* resultCoefficients = sgelMtdt.call_sgels();
    results.resize((*resultCoefficients).size());//prepares to be filled
        for(int k = 0; k < results.size(); k++){
            tags_and_relationships_return tagNrelation;
            tagNrelation.xTag = testingGround::return_xTag_of_dSet(startIndice+num_x_variables);
            tagNrelation.relationTo_Y_tag = static_cast<RelationshipType>(testingGround::return_relation_of_dSet(startIndice+k));
            results[k] = tagNrelation;
        }
    
    return &results;
}




















//void VariableData::setData(std::vector<float>& varData){(*data) = varData;}

//inserts the variables listed and resorts the array of variables in coldStorage
/*inline void RelationDetector::storeVariables(std::vector<VariableData>& variablesToAdd){
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
