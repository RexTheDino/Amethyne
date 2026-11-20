//
//  Derived_Differences_Calculation.metal
//  Amethyne
//
//  Created by AnshuDiya on 11/2/26.
//


#include <metal_stdlib>
#include "Metadata.h"
using namespace metal;

enum class relationships: uint16_t{
    linear = 5,
    quadratic = 6,
    cubic = 7,
    null = 8
};

struct relationshipResult{
    uint16_t xTag;
    uint16_t yTag;
    relationships relationship;
    relationshipResult(uint16_t tagX, uint16_t tagY, relationships derRelation): yTag{tagY}, xTag{tagX}, relationship{derRelation}{}
};

struct varTags{
    uint16_t tagX;
    uint16_t tagY;
};

enum class calculationMode: uint8_t{
    firstDiffs,
    secondDiffs,
    thirdDiffs
};


float calculateMedian(thread float& valToCalculateFor, int maxIterations, calculationMode modeOfCalculation, uint16_t thread_id){
    
    float2 currentTestRange = float2(simd_min(valToCalculateFor), simd_max(valToCalculateFor));//.y = max, .x = min
    float currentTestVal = (currentTestRange.y + currentTestRange.x)/2.0f;//find point between max and min to start at
    
    int rangeOfMedianLane = 1;//median can range from 32 +- 1
    int median = 31;
    
    if(modeOfCalculation == calculationMode::thirdDiffs){
        median = 30;
    }

    for(int y = 0; y < maxIterations; y++){
        auto vote_on_val = (uint64_t)simd_ballot(valToCalculateFor > currentTestVal);
        
        switch(modeOfCalculation){
            case calculationMode::firstDiffs:
                vote_on_val = vote_on_val << 1;//delete the 63'rd lane's result
                break;
            case calculationMode::secondDiffs:
                vote_on_val = vote_on_val << 2;//delete 63 and 62's results
                break;
            case calculationMode::thirdDiffs:
                vote_on_val = vote_on_val < 3;//delete results of 61-63;
                break;
            default:
                break;
        }
        
        int numThreadsWithGreaterVal = popcount((uint64_t)vote_on_val);//th enumber of threads iwth a value greater than this
        
        if((numThreadsWithGreaterVal <= median + rangeOfMedianLane)&&(numThreadsWithGreaterVal >= median - rangeOfMedianLane)){
            return currentTestVal;
        }else if(numThreadsWithGreaterVal > median + rangeOfMedianLane){
            currentTestRange = float2(( (currentTestRange.y + currentTestRange.x) / 2.0f), currentTestRange.y);
        }else{//if majority of threads have a LOWER value than was tested for
            currentTestRange = float2(currentTestRange.x,((currentTestRange.y + currentTestRange.x) / 2.0f));
        }
        currentTestVal = (currentTestRange.y + currentTestRange.x) / 2.0f;//set test val to the middle of current test range again
    }
    //if return it means many threads have the same val
    return currentTestVal;
}



float calculate_Robust_MAD(thread float& diffVal, calculationMode modeOfCalculation, float epsilon, uint16_t thread_ID){
    
    int maxIterations = 20;//change later if needed
    float medianOfDiffs = calculateMedian(diffVal, maxIterations, modeOfCalculation, thread_ID);
    float diffMinusMedian = abs(diffVal - medianOfDiffs);
    float MAD = calculateMedian(diffMinusMedian, maxIterations, modeOfCalculation, thread_ID);
        float robust_MAD = (1.4826 * MAD)/select(abs(medianOfDiffs), medianOfDiffs + epsilon, abs(medianOfDiffs) < abs(epsilon));//if medianOfDiffs is less than epsilon, set it to median+epsilon. else leave it be.
    
    return robust_MAD;
}



//threadgroups of 64 means each threadgroup on a SIMD unit
//number of threadgroups equals number of pairs to test
kernel void derivedDiffs(device relationshipResult* resultOutBuffer [[buffer(1)]], //to output in [tagX, tagY, relationship]
                         device const float* variableData [[buffer(2)]],
                         device const varTags* numberOfPointsAndVarTags [[buffer(3)]],//contains [tagX, tagY]. 0th index is pointsPerVar
                         uint16_t group_id [[threadgroup_position_in_grid]],
                         unsigned short simd_thread_id [[thread_index_in_simdgroup]]){
    
    const float epsilon = 0.0005;
    const float cutoffThreshold = 1.1f;//0.5f;//modified to suit MAD
    varTags varData;
    uint16_t pointsPerVar = RelationConstructionMetadata::points_per_var;
        if(simd_thread_id == 5){varData = numberOfPointsAndVarTags[group_id];}
    
    varData.tagX = simd_broadcast(varData.tagX, 5); varData.tagY = simd_broadcast(varData.tagY, 5);
    const uint32_t var1Start = varData.tagX * pointsPerVar;//tagX*pointsPerVar. this is *varX*
    const uint32_t var2Start = varData.tagY * pointsPerVar;//tagY*pointsPerVar. this is *varY*
    
    //x is var1, y is var 2
    float y1 = 0.0f; float x1 = 0.0f;
        for(int y = 0; y < (pointsPerVar/64); y++){
            x1 += variableData[var1Start + simd_thread_id +(y * 64)];
            y1 += variableData[var2Start + simd_thread_id +(y * 64)];
        }
    y1 /= (pointsPerVar/64);
    x1 /= (pointsPerVar/64);
    
    float y2 = simd_shuffle(y1, clamp((uint32_t)simd_thread_id+1U, 1U, 63U)); float x2 = simd_shuffle(x1, clamp((uint16_t)simd_thread_id+1U, 1U, 63U));

    
    float ratio;
    float sDiffFinal1;
    float fDiffFinal1;
    {float fDiff;
        {float y2_y1 = y2 - y1;
            float x2_x1 = x2 - x1;
            fDiff = select(y2_y1/x2_x1, 0.0f, (simd_thread_id+1) > 63U);}
        {float range = (simd_max(fDiff)-simd_min(fDiff));float mean = simd_sum(fDiff)/(64-1);
         range = (abs(range) >= abs(epsilon)) ? range : range + epsilon; mean = (abs(mean) >= abs(epsilon)) ? mean : mean+epsilon;
         float stdDev = sqrt(simd_sum(pow(fDiff-mean, 2.0f))/(pointsPerVar-1));//-1 to account for n-1 differences*/
        ratio =  stdDev/abs(mean);}    //calculate_Robust_MAD(fDiff, calculationMode::firstDiffs, epsilon, simd_thread_id);
        if(ratio <= cutoffThreshold){
            if(simd_thread_id == 5){
                resultOutBuffer[group_id] = relationshipResult(varData.tagX, varData.tagY, static_cast<relationships>(5));
                return;
            }
            return;
        }else{
            fDiffFinal1 = fDiff;
        }
    }
    
    {float sDiff;
        //cant pull fDiff from last thread in SIMD which didnt have a y2 to use for fDiff and only contains 0
        float fDiffFinal2 = simd_shuffle(fDiffFinal1, clamp((uint32_t)(simd_thread_id+1U), 1U, 62U));
        sDiff = fDiffFinal2-fDiffFinal1;
        //if this thread is one that wouldve brought stale data in, zero the data
        sDiff = select(sDiff, 0.0f, (simd_thread_id+1) > 62U);
        float range = (simd_max(sDiff)-simd_min(sDiff));
        float mean = simd_sum(sDiff)/(64-2);//to account for n-2 differences surviving
        mean = (abs(mean) >= abs(epsilon)) ? mean : mean+epsilon;
        range = (abs(range) >= abs(epsilon)) ? range : range+epsilon;
        float stdDev = sqrt(simd_sum(pow(sDiff-mean, 2.0f))/(pointsPerVar-2));
        ratio = stdDev/abs(mean);//dividing by supposedly constant second diffs mean  //abs(range); //calculate_Robust_MAD(sDiff, calculationMode::secondDiffs, epsilon, simd_thread_id);
        if(ratio <= cutoffThreshold){
            if(simd_thread_id == 5){
                resultOutBuffer[group_id] = relationshipResult(varData.tagX, varData.tagY, static_cast<relationships>(6));
                return;
            }
            return;
        }else{
            sDiffFinal1 = sDiff;
        }
    }
    {float sDiffFinal2 = simd_shuffle(sDiffFinal1, clamp((uint16_t)simd_thread_id+1U, 1U, 61U));//n-2 survivors
        float tDiff = sDiffFinal2-sDiffFinal1;
        tDiff = select(tDiff, 0.0f, (simd_thread_id+1) > 61U);//zero stale data threads
        float mean = simd_sum(tDiff)/(64-3);
        float range = (simd_max(tDiff)-simd_min(tDiff));
        mean = (abs(mean) >= abs(epsilon)) ? mean : mean+epsilon;
        range = (abs(range) >= abs(epsilon)) ? range : range+epsilon;
        float stdDev = sqrt(simd_sum(pow((tDiff-mean),2.0f))/(pointsPerVar-3));
        ratio = stdDev/abs(mean);//dividing by supposedly constant third diffs mean  //abs(range); //calculate_Robust_MAD(tDiff, calculationMode::thirdDiffs, epsilon, simd_thread_id);
        if(ratio <= cutoffThreshold){
            if(simd_thread_id == 5){
                resultOutBuffer[group_id] = relationshipResult(varData.tagX, varData.tagY, static_cast<relationships>(7));
                return;
            }
            return;
        }else{
            resultOutBuffer[group_id] = relationshipResult(varData.tagX, varData.tagY, static_cast<relationships>(8));//null relationship
        }
    }
}
