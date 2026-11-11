//
//  RelationshipClassTesting.m
//  RelationshipClassTesting
//
//  Created by AnshuDiya on 11/1/26.
//

#import <XCTest/XCTest.h>
#include "Relations_Header.hpp"
#include "Initialization_Data_header.hpp"
#import "AbsMin_Tester_Header.h"
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <iostream>
#include <random>

@interface RelationshipClassTesting : XCTestCase

@end

@implementation RelationshipClassTesting

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
    MetalSetup();
    execute();
}

- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

@end
