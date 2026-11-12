//
//  Accelerate_Init_Logic.cpp
//  Amethyne
//
//  Created by AnshuDiya on 11/11/26.
//

#include "Accelerate_initialization_data.h"
#include <vector>
calculate_SGELSY::STATUS calculate_SGELSY::status = calculate_SGELSY::STATUS::unprimed;//initialize to unprimed

void calculate_SGELSY::primeTheCalculator(){calculate_SGELSY::status = calculate_SGELSY::STATUS::primed;}

void calculate_SGELSY::call_sgelsy(){
    std::vector<__CLPK_integer> pivotVec(initDat.numTerms, 0);
        __CLPK_integer ldb = std::max(initDat.numPts, initDat.numTerms);
        __CLPK_integer lda = initDat.numPts;
        initDat.lwork = -1;
        float workSize = 0.0f;
    //query work-vec size
    sgelsy_(&initDat.numPts, &initDat.numTerms, &initDat.numEqnsToSolve, initDat.ptrToInputVec, &lda, initDat.ptrToOutputVec, &ldb, pivotVec.data(), initDat.epsilonThreshold, initDat.writeBackRankTo, &workSize, &initDat.lwork, &initDat.info);
            if(initDat.info != 0){throw std::runtime_error("SGELSY failed to initialize work-space!");}
    
    initDat.lwork = static_cast<__CLPK_integer>(workSize);
    std::vector<float> workVec(initDat.lwork);
    
    //calculate stuff
    sgelsy_(&initDat.numPts, &initDat.numTerms, &initDat.numEqnsToSolve, initDat.ptrToInputVec, &lda, initDat.ptrToOutputVec, &ldb, pivotVec.data(), initDat.epsilonThreshold, initDat.writeBackRankTo, workVec.data(), &initDat.lwork, &initDat.info);
            if(initDat.info != 0){throw std::runtime_error("SGELSY failed to calculate coefficients!");}
}
