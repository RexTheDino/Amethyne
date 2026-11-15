//
//  Nucleus.hpp
//  Amethyne
//
//  Created by AnshuDiya on 11/14/26.
//

#pragma once

#include "Var_Storage.hpp"
#include "Transfer_Manager.hpp"
#include "Data_logs.hpp"
#include "Heirarchial_Storage.hpp"

class NUCLEUS{
public:
    cold_Storage coldStorage;
    Transfer_Manager TransferManager;
    TESTING_GROUND testingGround;
    LOGS_MANAGER logsManager;
    Expression_Storage ExpressionStorage;
    
};inline NUCLEUS Nucleus;
