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
#include "Equation_Form_Converter.hpp"
#include <random>

@interface RelationshipClassTesting : XCTestCase

@end

@implementation RelationshipClassTesting

struct progNameAndVectorOfData{
    std::string progName;
    std::vector<float> vecOfData;
};

struct logsAndCache{
    std::vector<dataLog> logs;
    std::vector<dataLog> cache;
};

struct domainAndVecOfProgNames{
    domain theDomain;
    std::vector<std::string> theProgramNames;
    domainAndVecOfProgNames(domain providedDomain, std::vector<std::string> providedProgNames): theDomain{providedDomain}, theProgramNames{providedProgNames}{}
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
    static domainAndVecOfProgNames addRequestedVariables(int numActionVariables, int numMetricVariables){//adds variables with a random name and domain. returns the random domain chosen.
        varCreation variableCreator;
        
        std::vector<std::string> progNames; progNames.reserve(numActionVariables + numMetricVariables);
        std::uniform_real_distribution<float> randDomain(50.0f, 200.0f);
                float selectedMinDomain = randDomain(Amethyne_rand);
        domain selectedDomain(selectedMinDomain, selectedMinDomain + 50.0f);
        std::string prefix = "actionTest";
        std::string userAlottedName;
        std::string programName;
        for(int y = 0; y < numActionVariables; y++){
            userAlottedName = prefix + std::to_string(y);
            variableCreator.createAction(userAlottedName, selectedDomain.min, selectedDomain.max);
            programName = Nucleus.coldStorage.returnProgName_given_UserAlottedName(userAlottedName);
            progNames.push_back(programName);
        }
        prefix = "metricTest";
        for(int j = 0; j < numMetricVariables; j++){
            userAlottedName = prefix + std::to_string(j);
            variableCreator.createMetric(userAlottedName, selectedDomain.min, selectedDomain.max);
            programName = Nucleus.coldStorage.returnProgName_given_UserAlottedName(userAlottedName);
            progNames.push_back(programName);
        }
        
        domainAndVecOfProgNames returnStruct(selectedDomain, progNames);
        return returnStruct;
    }
    static void addData(int numPtsToAdd, domain domainOfDat, int numTotalVariables, std::vector<std::string> programNames){ //returns the vector of data it added
            assert(numTotalVariables == programNames.size());
        assert((numPtsToAdd <= RelationConstructionMetadata::points_per_var)&&(numPtsToAdd > 4));
        
        
        float averageDomain = (domainOfDat.max + domainOfDat.min)/2;
        std::uniform_real_distribution<float> randPoints(domainOfDat.min, domainOfDat.max);
        
        std::vector<progNameAndDat> testVec; testVec.reserve(numTotalVariables);
        std::vector<progNameAndVectorOfData> dataVec; dataVec.resize(numTotalVariables);
        
        for(int o = 0; o < numTotalVariables; o++){
            std::uniform_real_distribution<float> randNearConstPoints(averageDomain - (averageDomain * o/2), averageDomain + (averageDomain * o/2));
            
            dataVec[o].vecOfData.resize(numPtsToAdd);
            dataVec[o].progName = programNames[o];
            
            for(int y = 0; y < numPtsToAdd; y++){
                //float randPoint = randPoints(Amethyne_rand);
                
                float nearConstPoint = randNearConstPoints(Amethyne_rand);
                float linearPoint = nearConstPoint + (10 * y);
                float quadraticPoint = nearConstPoint + (0.5 * y * y);
                
                if(o%10 == 0){
                    dataVec[o].vecOfData[y] = linearPoint;
                }else if(o%5 == 0){
                    dataVec[o].vecOfData[y] = quadraticPoint;
                }else{
                    dataVec[o].vecOfData[y] = nearConstPoint;
                }
                
                //dataVec[o].vecOfData[y] = randPoint;
            }
        }
        
        for(int g = 0; g < numPtsToAdd; g++){
            for(int u = 0; u < numTotalVariables; u++){
                if(g != 0){
                    testVec[u] = progNameAndDat(programNames[u], dataVec[u].vecOfData[g]);//the program names in data vec correspond to testVec.
                }else{
                    testVec.push_back(progNameAndDat(programNames[u], dataVec[u].vecOfData[g]));//the program names in data vec correspond to testVec.
                }
            }
            std::span<progNameAndDat> spanOfTestVec(testVec.begin(), testVec.end());
            Nucleus.logsManager.logDat(spanOfTestVec);
        }
        
        for(int h = 0; h < numTotalVariables; h++){
            logsAndCache theLogsAndCache = returnDatOfRequestedTracker(dataVec[h].progName);
            XCTAssertTrue(theLogsAndCache.logs[3].data == dataVec[h].vecOfData[3]);
        }
    }
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

struct transferTester{
    static void transferDSetsAndCallSGELS_Y(){
        Nucleus.TransferManager.transferTestingResultsToColdStorage();
        auto allTags = Nucleus.coldStorage.returnALL_TAGS();
        assert(allTags.size() != 0);
        for(int y = 0; y < allTags.size(); y++){
            if(Nucleus.coldStorage.returnVarType_givenTag(allTags[y]) != variableCategory::action){
                SymEngine::RCP<const SymEngine::Basic> resultExprofRelatedDatasets = Relation_Equation_Constructor::calculateCoefficients(allTags[y]);
                if(!resultExprofRelatedDatasets.is_null()){//its null when it has no relations.
                    calculateBernsteinCoeffs calcTheCoeffs;
                    domain bernsteinResultWriteback(0.0f,0.0f);//should be overwritten by metal results.
                    calcTheCoeffs.findMaxAndMinBernsteinCoeffs(resultExprofRelatedDatasets, &bernsteinResultWriteback, nullptr);
                }
            }
        }
    }
};

struct exploringTestingGround_Tester{
    static void generateSyntheticAddonsAndCopyDatIntoTestSlots(){
        groundsWatchdog::setTestingGroundMode(testingGroundMode::exploring);
        std::vector<subcommands> vecToModify;
        statusUpdaters::ExplorationUpdaters::explorationReference referenceDat;
        
        statusUpdaters::ExplorationUpdaters::generateSyntheticAddons(vecToModify, referenceDat);//statusUpdaters::ExplorationUpdaters::GENERATE_RIGGED_SYNTHETIC_ADDONS(vecToModify, referenceDat);
        statusUpdaters::ExplorationUpdaters::copyRealDataIntoTestTagSlots(vecToModify, referenceDat);
        statusUpdaters::ExplorationUpdaters::initializeMetadata(vecToModify, referenceDat);
        statusUpdaters::ExplorationUpdaters::dispatchSynthPreprocessing(vecToModify, referenceDat);
        statusUpdaters::ExplorationUpdaters::initializeDerivedDiffsMtdt(vecToModify, referenceDat);//statusUpdaters::ExplorationUpdaters::initializeRIGGED_DERIVED_DIFFS_MTDT(vecToModify, referenceDat);
        statusUpdaters::ExplorationUpdaters::dispatchRelationEstimation(vecToModify, referenceDat);
        statusUpdaters::ExplorationUpdaters::copyPostProcessingResultsAndPointsToVecs(vecToModify, referenceDat);
        
        GPU_lieutenant::FIRE_AWAY();
        
        auto& theResults = Nucleus.testingGround.Data.results;
        auto isThereRelation = std::find_if(theResults.begin(), theResults.end(), [](Dataset& dSet){
            return dSet.returnRelation() != RelationshipType::null;
        });
        if(isThereRelation != theResults.end()){std::cout << "found relationship!!! at: "; int dist = (int)std::distance(theResults.begin(), isThereRelation); std::cout << dist;}
        
        transferTester::transferDSetsAndCallSGELS_Y();
        
        //return  Nucleus.testingGround.Data.testVarsStorage[(2 * RelationConstructionMetadata::points_per_var) +1];
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
    domainAndVecOfProgNames initializerStruct = trackerTester::addRequestedVariables(2, 3);
    trackerTester::addData(128, initializerStruct.theDomain, 5, initializerStruct.theProgramNames);
}



-(void)testAddonGen{
    
    exploringTestingGround_Tester::generateSyntheticAddonsAndCopyDatIntoTestSlots();
}


- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

@end
