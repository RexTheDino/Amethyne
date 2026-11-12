//
//  Authentic_Vars_Creation.cpp
//  Amethyne
//
//  Created by AnshuDiya on 11/3/26.
//

#pragma once

#include "Authentic_Vars_Creation.hpp"
#include <symengine/expression.h>

using SymEngine::Expression;

Expression z("z");
Expression expr = pow(z,2);//works!!!

