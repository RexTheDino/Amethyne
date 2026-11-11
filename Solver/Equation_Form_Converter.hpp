//
//  Equation_Form_Converter.hpp
//  Amethyne
//
//  Created by AnshuDiya on 11/7/26.
//

#ifndef Equation_Form_Converter_hpp
#define Equation_Form_Converter_hpp

#include <stdio.h>
#include <symengine/integer.h>
#include <symengine/real_double.h>
//#include "Base_types.hpp"
//#include <symengine/set_basic.h>

#include "Var_Storage.hpp"
#include <symengine/visitor.h>


struct MetalMatriceDimensionsFormat{
    unsigned char dimensionsOneThruFour = 0; unsigned char dimensionsFourThruEight = 0;
};
struct varDegreesAndCoefficient{//degrees and coefficient given a certain term of an expression
    unsigned char degreesOneThruFour = 0; unsigned char degreesFourThruEight = 0;
    float coefficient = 0.0f;
};

struct InfoBytes{//mirrors struct found in metal
    MetalMatriceDimensionsFormat matriceDimensions;
    varDegreesAndCoefficient CoefficientAndVariableDegrees;
    InfoBytes(MetalMatriceDimensionsFormat derDimensions, varDegreesAndCoefficient termByTermInfo);
    InfoBytes() = default;
};


//give it a symEngine expresson and indicate the term to input as well as a pointer to the metal buffer and it writes the require ddata into the metal buffers in metal format. also capable of reading the data
//*needs to convert existing variables into a polynomial with domain [0,1]!*
class BernsteinDataConverter{
    //when the solver runs, it replaces the variables with their bernstein equiavelnts to suit [0,1] domain. after it finishes running, its supposed to reverse engineer the variables. before doing any of that, it NEEDS to SAVE A LOCAL COPY and only perform the pre-processing modifications and processing ON THAT LOCAL COPY! This will not result in any conflict with coldStorage since its still the exact same symbols.
    enum class varConverterStatus{
      varsModifiedToBernsteinEquivalents,
        varsInOriginalState
    };static varConverterStatus STATUS;
    
    class helperFuncs{
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
        static inline uint8_t returnSymbolIndice(decltype(SymEngine::free_symbols(std::declval<SymEngine::Basic>())) &freed_symbols, SymEngine::RCP<const SymEngine::Basic> &derSymbol, std::vector<symbolAndIndex>* ptrToVector);
        
        
        static inline void writeSymbolIndexesIntoVector(SymEngine::RCP<const SymEngine::Basic> expr, std::vector<symbolAndIndex>* ptrToVector);
        class NodeLogic{
            friend class helperFuncs;
                //returns the degree of symbols and the coefficient in a single mult node given a reference to symbol_index vec
                static inline void multNodeLogic(SymEngine::RCP<const SymEngine::Basic> &expr, std::vector<symbolAndIndex>* ptrToReferenceVector, perTermCoefficientAndSymbolDegrees* ptrToStruct, decltype(SymEngine::free_symbols(std::declval<SymEngine::Basic>())) &freeSymbols);
                
                static inline void Sym_or_PowNodeLogic(SymEngine::RCP<const SymEngine::Basic> &expr, std::vector<symbolAndIndex>* ptrToReferenceVector, perTermCoefficientAndSymbolDegrees* ptrToStruct, decltype(SymEngine::free_symbols(std::declval<SymEngine::Basic>())) &freeSymbols);
        
        public:
                static inline SymEngine::RCP<const SymEngine::Basic> returnBernsteinAdjustedExpression(SymEngine::RCP<const SymEngine::Basic> &expr);
        private:
                            //given a symbol, returns the same symbol but adjusted for with its domain to [0,1]
                            static inline SymEngine::RCP<const SymEngine::Basic> returnBernsteinAdjustedSymbol(SymEngine::RCP<const SymEngine::Basic> &symbol);
                            static inline SymEngine::RCP<const SymEngine::Basic> returnBernsteinAdjustedPow_Or_SymNode(SymEngine::RCP<const SymEngine::Basic> &expr);//returns the expression but with symbols converted to [0,1] domain
                            static inline SymEngine::RCP<const SymEngine::Basic> returnBernsteinAdjustedMultNode(SymEngine::RCP<const SymEngine::Basic> &expr);
            
        };
    //writes the term degrees and coefficients of the given expression into the provided vector
    static inline void writePerTermSymbolDegreesAndCoefficientsIntoVector(SymEngine::RCP<const SymEngine::Basic> &expr, std::vector<perTermCoefficientAndSymbolDegrees>* ptrToVector);
    };
            //converts a given expression to the compressed bit format of the dimensions required for metal to calculate terms
            static MetalMatriceDimensionsFormat convertDimensionsOfExpr(const std::vector<helperFuncs::perTermCoefficientAndSymbolDegrees>& ptrToPolynomialInfoVector);
            //converts a given expression's term to the term degrees and coefficient format and fills a vector with the info
            static void convertDegreesAndCoefficientOfExprGivenTerm(SymEngine::RCP<const SymEngine::Basic> &ProvidedExpr, const std::vector<helperFuncs::perTermCoefficientAndSymbolDegrees>& polynomialInfoArray,  std::vector<varDegreesAndCoefficient>* vectorToWriteInto);
    
    
    
    //it writes results to a local vector after which memcpy should be done to transfer to metal buffer
    //returns a InfoBytes vector (what the kernel needs to know) hwhen you give it the expression corresponding to terms
    static inline void calculateInfoBytes(SymEngine::RCP<const SymEngine::Basic> &expr, std::vector<InfoBytes>* vectorToWriteTo);
    
public:
    static inline void processExpressionForBernsteinCoefficientsCalculation(SymEngine::RCP<const SymEngine::Basic> &expression, std::vector<InfoBytes>* referenceToInfoByteBufferThatMetalAccesses);
};














#endif /* Equation_Form_Converter_hpp */
