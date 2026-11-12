//
//  Accelerate_initialization_data.h
//  Amethyne
//
//  Created by AnshuDiya on 11/10/26.
//

#pragma once

#include <Accelerate/Accelerate.h>

class calculate_SGELSY;

struct AccelInitDat{
    __CLPK_integer num_pts = -1, num_unknown_coeffs = -1, nrhs = -1, lead_dimension_x_vals = -1, lead_dimension_y_vals = -1, lwork = -1;
    __CLPK_integer info = -1;
};

struct SGELSY_INIT_DATA{
    friend class calculate_SGELSY;
private:
    __CLPK_integer lwork, info;
public:
    __CLPK_integer numPts, numTerms, numEqnsToSolve;
    float* ptrToInputVec;float* ptrToOutputVec;float* epsilonThreshold;
    __CLPK_integer* writeBackRankTo;
};

class calculate_SGELSY{
    enum class STATUS{
        primed,
        unprimed
    }; static STATUS status;
public:
    SGELSY_INIT_DATA initDat;//should be filled before below func is called!
    inline void primeTheCalculator();//called when all data has been filled
    inline void call_sgelsy();//checks if it has been primed
};

