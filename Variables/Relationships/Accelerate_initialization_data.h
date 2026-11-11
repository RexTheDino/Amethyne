//
//  Accelerate_initialization_data.h
//  Amethyne
//
//  Created by AnshuDiya on 11/10/26.
//

#ifndef Accelerate_initialization_data_h
#define Accelerate_initialization_data_h

#include <Accelerate/Accelerate.h>

struct AccelInitDat{
    __CLPK_integer num_pts = -1, num_unknown_coeffs = -1, nrhs = -1, lead_dimension_x_vals = -1, lead_dimension_y_vals = -1, lwork = -1;
    __CLPK_integer info = -1;
};

#endif /* Accelerate_initialization_data_h */
