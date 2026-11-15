//
//  RelationshipClassTesting.m
//  RelationshipClassTesting
//
//  Created by AnshuDiya on 11/1/26.
//

#import <XCTest/XCTest.h>
#include "Authentic_Vars_Creation.hpp"
#include "Variable_Gen_Header.hpp"
#include "Symbolic_representation_logic.hpp"
#include "Nucleus.hpp"
#include "Initialization_Data_header.hpp"
#include "SynthAddon_gen.hpp"
#include "TestStorage_Status_logic.hpp"
//#include <symengine/expression.h>
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <iostream>
#include "SynthAddon_gen.hpp"
#include <random>

@interface RelationshipClassTesting : XCTestCase

@end

@implementation RelationshipClassTesting

struct logsAndCache{
    std::vector<dataLog> logs;
    std::vector<dataLog> cache;
};

struct trackerTester{
    static logsAndCache returnDatOfRequestedTracker(std::string program_name){
        auto theTrackerIterator = Nucleus.logsManager.trackerMap.find(program_name);
        if(theTrackerIterator == Nucleus.logsManager.trackerMap.end()){throw std::invalid_argument("No such authentic with requested prog name exists!");}
            logsAndCache returnStruct;
            returnStruct.logs = (theTrackerIterator->second).logs;
            returnStruct.cache = (theTrackerIterator->second).cache;
                return returnStruct;
    };
};

struct synthVarGenTester{
    struct addonsAndNumAddons{
        std::vector<synthVarAddon> addons;
        numAddonsPerTestSynth numAddonsPerBatch;
    };
    
    static addonsAndNumAddons genTest(uint16_t numVarsToMake){
        std::vector<synthVarAddon> resultVec;//will be filled by the synthetic variable generator
            addonGenPasskey passkey;
        auto numberOfAddonsPerBatch = synthVarGen::createAndWriteSynthVars(numVarsToMake, resultVec, passkey);
        
        addonsAndNumAddons returnStruct; returnStruct.addons = resultVec; returnStruct.numAddonsPerBatch = numberOfAddonsPerBatch;
        
        return returnStruct;
    }
};

struct exploringTestingGround_Tester{
    static float generateSyntheticAddonsAndCopyDatIntoTestSlots(){
        groundsWatchdog::setTestingGroundMode(testingGroundMode::exploring);
        
        statusUpdaters::ExplorationUpdaters::generateSyntheticAddons();
        statusUpdaters::ExplorationUpdaters::copyRealDataIntoTestTagSlots();
        
        return  Nucleus.testingGround.Data.testVarsStorage[((2 * RelationConstructionMetadata::points_per_var) + RelationConstructionMetadata::points_per_var -1)];
    }
};


- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

/*- (void)testExample {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    std::random_device randDevice;
    std::mt19937 gen(randDevice());
    std::uniform_int_distribution<float> distrib(1.0,9999.0);
    
    std::vector<float> quadraticPolynomial; quadraticPolynomial.resize(300);
    std::vector<float> quadraticPolynomialTwo; quadraticPolynomialTwo.resize(300);
    for(int k = 0; k < 300.0f; k++){
        quadraticPolynomial[k] = std::pow(k,2.1f)+(0.01*distrib(gen));
        quadraticPolynomialTwo[k] = std::pow(2*k, 2.0f);
    }
    //VariableData pol1; pol1.setData(quadraticPolynomial); pol1.tag = 1UL;
    //VariableData pol2; pol2.setData(quadraticPolynomialTwo); pol2.tag = 2UL;
     
    RelationDetector derTester;
    RelationshipType result = derTester.relationReturn(pol1, pol2);
    
    XCTAssertEqual(result, RelationshipType::linear, @"Since both are polynomials, should detect linear relationship");
}*/

- (void)testMetalAbsMin {
    //MetalSetup();
    //execute();
}

-(void)testActionAndMetricCreation{//works
    varCreation variableCreator;
    std::string testPrefix = "test";
    std::uniform_real_distribution<float> dataGen(50.0,100.0);
        std::vector<progNameAndDat> testVec;
    for(int u = 0; u < 100; u++){//data array and var string array need to be of equal sizes
        std::string userAlottedName;
        if(u % 2 == 0){
            userAlottedName = testPrefix+std::to_string(u);
            variableCreator.createAction(userAlottedName, u*2, u*3.5);
        }else{
            userAlottedName = testPrefix+std::to_string(u);
            variableCreator.createMetric(userAlottedName, u-20, (u+55)*2);
        }
        
        float randPoint = dataGen(Amethyne_rand);
        std::string progName = Nucleus.coldStorage.returnProgName_given_UserAlottedName(userAlottedName);
        progNameAndDat nameAndData(progName, randPoint);
        testVec.push_back(nameAndData);
    }
        std::span<progNameAndDat> testSpan(testVec.begin(), testVec.end());
    
    for(int d = 0; d < 200; d++){
        Nucleus.logsManager.logDat(testSpan);
        for(int s = 0; s < testVec.size(); s++){
            logsAndCache trackerResult = trackerTester::returnDatOfRequestedTracker(testVec[s].progName);
            XCTAssertEqual(trackerResult.logs[d].data, testVec[s].data, @"The test vector data should be the same as was logged in the tracker.");
        }
        
        for(int f = 0; f < testVec.size(); f++){
            float randPoint = dataGen(Amethyne_rand);
            testVec[f].data = randPoint;
        }
    }
    
}



-(void)testAddonGen{
    
    //synthVarGenTester::addonsAndNumAddons generationResult = synthVarGenTester::genTest(20);
    float secondTestTagSlotFirstIndex = exploringTestingGround_Tester::generateSyntheticAddonsAndCopyDatIntoTestSlots();
    XCTAssertTrue((secondTestTagSlotFirstIndex >= 50.0f)&&(secondTestTagSlotFirstIndex <= 100.0f));
}


- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

@end
