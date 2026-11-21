//
//  Equation_Form_Converter.hpp
//  Amethyne
//
//  Created by AnshuDiya on 11/7/26.
//

#pragma once

#include <stdio.h>
#include <symengine/integer.h>
#include <symengine/real_double.h>
//#include "Base_types.hpp"
//#include <symengine/set_basic.h>

#include "Var_Storage.hpp"
#include <symengine/visitor.h>
#include "Dispatcher.hpp"


struct MetalMatriceDimensionsFormat{
    unsigned char dimensionsOneThruFour = 0; unsigned char dimensionsFourThruEight = 0;
};
struct matriceDimensionsAndTheirProduct{
    MetalMatriceDimensionsFormat matriceDimensions;
    int productOfDimensions;
};
struct varDegreesAndCoefficient{//degrees and coefficient given a certain term of an expression
    unsigned char degreesOneThruFour = 0; unsigned char degreesFourThruEight = 0;
    float coefficient = 0.0f;
};

struct InfoBytes{//mirrors struct found in metal
    //change whats below so infobytes has 0 padding and consists of purely 4 chars followed by a float.
    MetalMatriceDimensionsFormat matriceDimensions;
    varDegreesAndCoefficient CoefficientAndVariableDegrees;
    InfoBytes(MetalMatriceDimensionsFormat derDimensions, varDegreesAndCoefficient termByTermInfo);
    InfoBytes() = default;
};
/*struct InfoBytesAndN_Choose_R{//what metal actually needs
    InfoBytes the_infoBytes;
    const uint32_t nChooseR_vals[12] = {1, 1, 0, 0, 1, 2, 1, 0, 1, 3, 3, 1};
};*/


//give it a symEngine expresson and indicate the term to input as well as a pointer to the metal buffer and it writes the require ddata into the metal buffers in metal format. also capable of reading the data
//*needs to convert existing variables into a polynomial with domain [0,1]!*
class BernsteinDataConverter{
    //when the solver runs, it replaces the variables with their bernstein equiavelnts to suit [0,1] domain. after it finishes running, its supposed to reverse engineer the variables. before doing any of that, it NEEDS to SAVE A LOCAL COPY and only perform the pre-processing modifications and processing ON THAT LOCAL COPY! This will not result in any conflict with coldStorage since its still the exact same symbols.
    
    class helperFuncs{
        enum class varConverterStatus{
          varsModifiedToBernsteinEquivalents,
            varsInOriginalState
        };static inline varConverterStatus STATUS  = varConverterStatus::varsInOriginalState;
        friend class BernsteinDataConverter;
        struct symbolAndIndex{
            SymEngine::RCP<const SymEngine::Symbol> symbol;
            int index;
            symbolAndIndex(SymEngine::RCP<const SymEngine::Symbol> derSymbol, int derIndex);
            symbolAndIndex() = default;
        };
            struct symbolAndDegree{
                SymEngine::RCP<const SymEngine::Symbol> symbol;
                uint8_t degree;
                symbolAndDegree(SymEngine::RCP<const SymEngine::Basic> derSymbol, uint8_t derDegree);
                symbolAndDegree() = default;
            };
            //stores the symbol degrees for each term as well as that term's coefficient. one struct per term.
            struct perTermCoefficientAndSymbolDegrees{
                std::vector<symbolAndDegree> symbol_Degrees;
                float coefficient;
            };
        
        //returns the degree a certain symbol belongs in
        static uint8_t returnSymbolIndice(decltype(SymEngine::free_symbols(std::declval<const SymEngine::Basic>())) &freed_symbols, SymEngine::RCP<const SymEngine::Basic> &derSymbol, std::vector<symbolAndIndex>* ptrToVector);
        
        
        static void writeSymbolIndexesIntoVector(SymEngine::RCP<const SymEngine::Basic> expr, std::vector<symbolAndIndex>* ptrToVector);
        class NodeLogic{
            friend class helperFuncs;
                //returns the degree of symbols and the coefficient in a single mult node given a reference to symbol_index vec
                static void multNodeLogic(SymEngine::RCP<const SymEngine::Basic> &expr, std::vector<symbolAndIndex>* ptrToReferenceVector, perTermCoefficientAndSymbolDegrees* ptrToStruct, decltype(SymEngine::free_symbols(std::declval<SymEngine::Basic>())) &freeSymbols);
                
                static void Sym_or_PowNodeLogic(SymEngine::RCP<const SymEngine::Basic> &expr, std::vector<symbolAndIndex>* ptrToReferenceVector, perTermCoefficientAndSymbolDegrees* ptrToStruct, decltype(SymEngine::free_symbols(std::declval<SymEngine::Basic>())) &freeSymbols);
        
        public:
                static SymEngine::RCP<const SymEngine::Basic> returnBernsteinAdjustedExpression(SymEngine::RCP<const SymEngine::Basic> &expr);
        private:
                            //given a symbol, returns the same symbol but adjusted for with its domain to [0,1]
                            static SymEngine::RCP<const SymEngine::Basic> returnBernsteinAdjustedSymbol(SymEngine::RCP<const SymEngine::Basic> &symbol);
                            static SymEngine::RCP<const SymEngine::Basic> returnBernsteinAdjustedPow_Or_SymNode(SymEngine::RCP<const SymEngine::Basic> &expr);//returns the expression but with symbols converted to [0,1] domain
                            static SymEngine::RCP<const SymEngine::Basic> returnBernsteinAdjustedMultNode(SymEngine::RCP<const SymEngine::Basic> &expr);
            
        };
    //writes the term degrees and coefficients of the given expression into the provided vector
    static void writePerTermSymbolDegreesAndCoefficientsIntoVector(SymEngine::RCP<const SymEngine::Basic> &expr, std::vector<perTermCoefficientAndSymbolDegrees>* ptrToVector);
    };
            //converts a given expression to the compressed bit format of the dimensions required for metal to calculate terms
            static matriceDimensionsAndTheirProduct convertDimensionsOfExpr(const std::vector<helperFuncs::perTermCoefficientAndSymbolDegrees>& ptrToPolynomialInfoVector);
            //converts a given expression's term to the term degrees and coefficient format and fills a vector with the info
            static void convertDegreesAndCoefficientOfExprGivenTerm(SymEngine::RCP<const SymEngine::Basic> &ProvidedExpr, const std::vector<helperFuncs::perTermCoefficientAndSymbolDegrees>& polynomialInfoArray,  std::vector<varDegreesAndCoefficient>* vectorToWriteInto);
    
    
    
    //it writes results to a local vector after which memcpy should be done to transfer to metal buffer
    //writes a InfoBytes vector (what the kernel needs to know) hwhen you give it the expression corresponding to terms
    //returns the PRODUCT OF THE DIMENSIONS OF THE MATRICE
    static int calculateInfoBytes(SymEngine::RCP<const SymEngine::Basic> &expr, std::vector<InfoBytes>* vectorToWriteTo);
    
public:
    //returns the PRODUCT OF THE DIMENSIONS OF THE MATRICE
    static int processExpressionForBernsteinCoefficientsCalculation(SymEngine::RCP<const SymEngine::Basic> &expression, std::vector<InfoBytes>* ptrToInfoByteBuffer);
};


enum class bernsteinCoeffCalculationStatus{
    unprimed,
    DISPATCHED_INFO_BYTES,
    DISPATCHED_KERNEL_OP,
    DISPATCHED_COPY_BACK_CMD,
};

struct bernsteinCoeffsReferenceData{
    std::vector<subcommands> subcommandsVec;
    uint32_t productOfAllDimensions;//for the number of threadgroups
    uint16_t numTerms;//the number of total terms in the polynomial being calculated for.
    domain* writeBackValsTo = nullptr;//to be filled by the public func. this is the space that the caller wants to record the domain results in.
    std::vector<InfoBytes> metalInfVec;
    void(*funcToCallbackAferResult)();
};

//given an expression returns the bernstein coefficient for that
class calculateBernsteinCoeffs{
private:
    
    bernsteinCoeffCalculationStatus bernCoeffStatus = bernsteinCoeffCalculationStatus::unprimed;
    static inline bernsteinCoeffsReferenceData refDat;
    
    void calculateAndWriteInfoBytes(SymEngine::RCP<const SymEngine::Basic>& expr, std::vector<subcommands>& subcommandsVec);
    void bindCalculationAndMAXMIN_FINDER_kernels(std::vector<subcommands>& subcommandsVec);
    void dispatchResultCopyCommand(std::vector<subcommands>& subcommandsVec);//also dispatches command to copy back into result variable
    
public:
    void findMaxAndMinBernsteinCoeffs(SymEngine::RCP<const SymEngine::Basic>& expr, domain* writeBackResultsTo, void(*funcToCallbackAferResult)());
};
