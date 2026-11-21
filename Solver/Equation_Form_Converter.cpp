//
//  Equation_Form_Converter.cpp
//  Amethyne
//
//  Created by AnshuDiya on 11/7/26.
//

#include "Equation_Form_Converter.hpp"
#include "Nucleus.hpp"

using namespace SymEngine;

InfoBytes::InfoBytes(MetalMatriceDimensionsFormat derDimensions, varDegreesAndCoefficient termByTermInfo){
    matriceDimensions = derDimensions;
    CoefficientAndVariableDegrees = termByTermInfo;
}

BernsteinDataConverter::helperFuncs::symbolAndDegree::symbolAndDegree(SymEngine::RCP<const SymEngine::Basic> derSymbol, uint8_t derDegree){
    if(is_a<Symbol>(*derSymbol)){
        auto decastedVar = rcp_static_cast<const Symbol>(derSymbol);
        symbol = decastedVar;
        degree = derDegree;
    }else{
        throw std::runtime_error("Constructor for struct involving symbol called with parameter provided as a non-symbol!");
    }
}

BernsteinDataConverter::helperFuncs::symbolAndIndex::symbolAndIndex(SymEngine::RCP<const SymEngine::Symbol> derSymbol, int derIndex){
    symbol = derSymbol;
    index = derIndex;
}

//returns the PRODUCT OF THE METAL MATRICE DIMENSIONS
int BernsteinDataConverter::calculateInfoBytes(SymEngine::RCP<const SymEngine::Basic> &expr, std::vector<InfoBytes>* vectorToWriteTo){
    
    if(helperFuncs::STATUS != helperFuncs::varConverterStatus::varsModifiedToBernsteinEquivalents){
        throw std::runtime_error("Variables have not been modified to bernstein-friendly terms whose domains are mapped to [0,1]!");
    }
    
    std::vector<helperFuncs::perTermCoefficientAndSymbolDegrees> polynomialInfoVec;
    helperFuncs::writePerTermSymbolDegreesAndCoefficientsIntoVector(expr, &polynomialInfoVec);//fills vector
    
    matriceDimensionsAndTheirProduct matriceDimensionsAndProduct = convertDimensionsOfExpr(polynomialInfoVec);
    MetalMatriceDimensionsFormat matriceDimensions = matriceDimensionsAndProduct.matriceDimensions;
    
    std::vector<varDegreesAndCoefficient> degreeAndCoeffInfo_forMetal;
    convertDegreesAndCoefficientOfExprGivenTerm(expr, polynomialInfoVec, &degreeAndCoeffInfo_forMetal);//fills vector
    
    (*vectorToWriteTo).resize(degreeAndCoeffInfo_forMetal.size());
    for(int v = 0; v < degreeAndCoeffInfo_forMetal.size(); v++){
        InfoBytes returnStruct(matriceDimensions, degreeAndCoeffInfo_forMetal[v]);
        (*vectorToWriteTo)[v] = returnStruct;
    }
    
    return matriceDimensionsAndProduct.productOfDimensions;
}

//assumes the vector has already been filled wth the variable degree and coefficient info!
matriceDimensionsAndTheirProduct BernsteinDataConverter::convertDimensionsOfExpr(const std::vector<helperFuncs::perTermCoefficientAndSymbolDegrees>& ptrToPolynomialInfoVector){
    matriceDimensionsAndTheirProduct returnStruct;
    
    MetalMatriceDimensionsFormat dimensionsStruct;
    if(ptrToPolynomialInfoVector.empty()){throw std::runtime_error("Variable degree and coefficient vector unfilled!");}
    
    std::vector<uint8_t> maxDimensions = {0,0,0,0,0,0,0,0};//8 slots corresponding to 8 variables max
    for (int d = 0; d < ptrToPolynomialInfoVector.size(); d++) {
        for (auto e = ptrToPolynomialInfoVector[d].symbol_Degrees.begin(); e != ptrToPolynomialInfoVector[d].symbol_Degrees.end(); e++) {//loop through each term's degrees
            int counter = (int)std::distance(ptrToPolynomialInfoVector[d].symbol_Degrees.begin(), e);
            maxDimensions[counter] = std::max(e->degree, maxDimensions[counter]);//fills with max dimensions
        }
    }
    int product = 1;
    for(int j = 0; j < maxDimensions.size(); j++){
        if(maxDimensions[j] == 0){break;}
        product *= maxDimensions[j]+1;//NEED TO DO PLUS 1 SINCE EVEN AT DEGREE 0 THERE IS A DIMENSION OF 1
    }
    
    for (int w = 0; w < 8; w++) {
        int bitshiftVal = w < 4 ? w*2 : (w-4)*2;
        unsigned char& modifierDimensionReference = w < 4 ? dimensionsStruct.dimensionsOneThruFour : dimensionsStruct.dimensionsFourThruEight;
        modifierDimensionReference = (maxDimensions[w] << bitshiftVal) | modifierDimensionReference;
    }
    
    returnStruct.matriceDimensions = dimensionsStruct;
    returnStruct.productOfDimensions = product;
    return returnStruct;
}

//assumes vector is already filled!
void BernsteinDataConverter::convertDegreesAndCoefficientOfExprGivenTerm(SymEngine::RCP<const SymEngine::Basic> &ProvidedExpr, const std::vector<helperFuncs::perTermCoefficientAndSymbolDegrees>& polynomialInfoArray,  std::vector<varDegreesAndCoefficient>* vectorToWriteInto){
    if(polynomialInfoArray.empty()){throw std::runtime_error("Polynomial info array not filled before variable degree and coefficient conversion function called!");}
    
    (*vectorToWriteInto).resize(polynomialInfoArray.size());
    //fills the vector to write into with the information for the polynomial degrees and stuff
    for (int h = 0; h < polynomialInfoArray.size(); h++) {
        varDegreesAndCoefficient degreeAndCoeffInfo;
        degreeAndCoeffInfo.coefficient = polynomialInfoArray[h].coefficient;
        for (auto s = polynomialInfoArray[h].symbol_Degrees.begin(); s != polynomialInfoArray[h].symbol_Degrees.end(); s++) {
            int counter = (int)std::distance(polynomialInfoArray[h].symbol_Degrees.begin(), s);
            int bitShiftVal = counter < 4 ? (counter*2) : ((counter-4)*2);
            unsigned char& modifierByteReference = counter < 4 ? degreeAndCoeffInfo.degreesOneThruFour : degreeAndCoeffInfo.degreesFourThruEight;
            modifierByteReference = (s->degree << bitShiftVal) | modifierByteReference;
        }
        (*vectorToWriteInto)[h] = degreeAndCoeffInfo;
    }
    
}




uint8_t BernsteinDataConverter::helperFuncs::returnSymbolIndice(decltype(SymEngine::free_symbols(std::declval<const SymEngine::Basic>())) &freed_symbols, SymEngine::RCP<const SymEngine::Basic> &derSymbol, std::vector<symbolAndIndex>* ptrToVector){
    auto correspondingIterator = freed_symbols.find(derSymbol);
    int correspondingIndex = (int)std::distance(freed_symbols.begin(), correspondingIterator);
    if(correspondingIndex > 7){
        throw std::runtime_error("More than 8 indexes/variables distance in the freed symbols for goal eqn!");
    }
    return (uint8_t)correspondingIndex;
}




void BernsteinDataConverter::helperFuncs::writeSymbolIndexesIntoVector(SymEngine::RCP<const SymEngine::Basic> expr, std::vector<symbolAndIndex> *ptrToVector){
    
    auto derSymbols = free_symbols(*expr);
    if(derSymbols.size() > 8){throw std::runtime_error("More than 8 variables in given equation!");}
    std::vector<symbolAndIndex> symbolsAndIndex; symbolsAndIndex.resize(derSymbols.size());//initialize vector of size derSymbols to accomodate symbols
    for (auto c = derSymbols.begin(); c != derSymbols.end(); c++) {
        auto symbol = rcp_static_cast<const Symbol>(*c);
        int correspondingIndex = (int)std::distance(derSymbols.begin(), c);
        symbolAndIndex structThatsFilled(symbol, correspondingIndex);
        if((*ptrToVector).size() > correspondingIndex){
            (*ptrToVector)[correspondingIndex] = structThatsFilled;
        }else{
            (*ptrToVector).push_back(structThatsFilled);//if size too small push back
        }
    }
}

void BernsteinDataConverter::helperFuncs::writePerTermSymbolDegreesAndCoefficientsIntoVector(SymEngine::RCP<const SymEngine::Basic> &expr, std::vector<perTermCoefficientAndSymbolDegrees> *ptrToVector){
    std::vector<symbolAndIndex> symbolsAndIndexes;
    std::vector<symbolAndIndex>* referenceVecPtr = &symbolsAndIndexes;
    writeSymbolIndexesIntoVector(expr, &symbolsAndIndexes);//retrieve the indexes the symbols are supposed to go into
    auto expandedExpr = expand(expr);
    auto freedSymbols = free_symbols(*expandedExpr);
    
    if (is_a<Add>(*expandedExpr)) {
        auto decastedVar = rcp_static_cast<const Add>(expandedExpr);
        auto terms = decastedVar->get_args();
        (*ptrToVector).resize(terms.size());
        for (int z = 0; z < terms.size(); z++) {
            //pointer to the corresponding struct that'll carry the term degree and coefficient info and stuff
            perTermCoefficientAndSymbolDegrees* ptrToInfoStruct = &(*ptrToVector)[z];
            //only one of these actually runs depending on if the term is a mult or sym or pow node
            NodeLogic::multNodeLogic(terms[z], referenceVecPtr, ptrToInfoStruct, freedSymbols);
            NodeLogic::Sym_or_PowNodeLogic(terms[z], referenceVecPtr, ptrToInfoStruct, freedSymbols);
        }
    }else{//the other possibility is that there is a mult or pow or sym standalone which means only one term. only one runs.
        (*ptrToVector).resize(1);
        NodeLogic::multNodeLogic(expandedExpr, referenceVecPtr, &(*ptrToVector)[0], freedSymbols);
        NodeLogic::Sym_or_PowNodeLogic(expandedExpr, referenceVecPtr, &(*ptrToVector)[0], freedSymbols);
    }
}

//returns the degree of symbols and the coefficient in a single mult node given a reference to symbol_index vec
void BernsteinDataConverter::helperFuncs::NodeLogic::multNodeLogic(SymEngine::RCP<const SymEngine::Basic> &expr, std::vector<symbolAndIndex> *ptrToReferenceVector, perTermCoefficientAndSymbolDegrees* ptrToStruct, decltype(SymEngine::free_symbols(std::declval<SymEngine::Basic>())) &freeSymbols){
    if (is_a<Mul>(*expr)) {
        auto decastedVar = rcp_static_cast<const Mul>(expr);
        auto dictionary = decastedVar->get_dict();
        
        (*ptrToStruct).symbol_Degrees.resize(freeSymbols.size());
        for (auto g = dictionary.begin(); g != dictionary.end(); g++) {
            auto base = g->first; auto exponent = g->second; auto expAsSynInt = rcp_static_cast<const Integer>(exponent);     uint8_t rawIntExponent = expAsSynInt->as_int();
            uint8_t indexToFill = returnSymbolIndice(freeSymbols, base, ptrToReferenceVector);
            symbolAndDegree symAndDegree(base, rawIntExponent);
            (*ptrToStruct).symbol_Degrees[indexToFill] = symAndDegree;
        }
        
            auto SymCoefficient = rcp_static_cast<const RealDouble>(decastedVar->get_coef());
            (*ptrToStruct).coefficient = (float)eval_double(*SymCoefficient);
    }else{//if not mul then return
        return;
    }
}

//degree and coefficient of a single pow node written tot he pointer to the struct
void BernsteinDataConverter::helperFuncs::NodeLogic::Sym_or_PowNodeLogic(SymEngine::RCP<const SymEngine::Basic> &expr, std::vector<symbolAndIndex> *ptrToReferenceVector, perTermCoefficientAndSymbolDegrees *ptrToStruct, decltype(SymEngine::free_symbols(std::declval<SymEngine::Basic>())) &freeSymbols){
    if (is_a<Pow>(*expr)) {
        (*ptrToStruct).coefficient = 1.0f;//if pow or exp then ocefficient is 1.0 !
        auto decastedVar = rcp_static_cast<const Pow>(expr);
        auto base = decastedVar->get_base();
            auto exponent = decastedVar->get_exp(); auto expAsSymInt = rcp_static_cast<const Integer>(exponent);
            auto rawIntExponent = expAsSymInt->as_int();
        uint8_t indexToFill = returnSymbolIndice(freeSymbols, base, ptrToReferenceVector);
        symbolAndDegree symAndDegree(base, rawIntExponent);
        (*ptrToStruct).symbol_Degrees.resize(freeSymbols.size());
        (*ptrToStruct).symbol_Degrees[indexToFill] = symAndDegree;
    }else if(is_a<Symbol>(*expr)){
        (*ptrToStruct).coefficient = 1.0f;
        auto decastedVar = rcp_static_cast<const Basic>(expr);//in this case, decasted var is the base!
        uint8_t indexToFill = returnSymbolIndice(freeSymbols, decastedVar, ptrToReferenceVector);
        symbolAndDegree symAndDegree(decastedVar, 1);
        (*ptrToStruct).symbol_Degrees.resize(freeSymbols.size());
        (*ptrToStruct).symbol_Degrees[indexToFill] = symAndDegree;
    }else if(is_a<RealDouble>(*expr)){
        auto decastedVar = rcp_static_cast<const RealDouble>(expr);
        (*ptrToStruct).coefficient = decastedVar->as_double();//if just a const then fill with the coefficient
        return;//do nothing if not a pow or symbol node
    }
}

//does NOT modify the expression itself.
RCP<const Basic> BernsteinDataConverter::helperFuncs::NodeLogic::returnBernsteinAdjustedExpression(SymEngine::RCP<const SymEngine::Basic> &expr){
    if(STATUS != varConverterStatus::varsInOriginalState){
        throw std::runtime_error("Variables not reverted to original state whne bernstein-friendly modificaiton logic called!");
    }
    RCP<const Basic> result = integer(0);
    if(is_a<Add>(*expr)){
        auto decastedVar = rcp_static_cast<const Add>(expr);
        auto arguments = decastedVar->get_args();
        //the functions below take a reference to but do not modify expression!
        for(int n = 0; n < arguments.size(); n++){//is not modified if not of the correct type
            RCP<const Basic> possibleResult;
            possibleResult = returnBernsteinAdjustedPow_Or_SymNode(arguments[n]);
            if(possibleResult.is_null()){//if returned a null, means the node wasnt a pow or sym, so assign it mult logic
                possibleResult = returnBernsteinAdjustedMultNode(arguments[n]);
            }
            arguments[n] = possibleResult;
        }
        //re-construct expression with modified bernstein-friendly terms
        for(int k = 0; k < arguments.size(); k++){
            result = add(result, arguments[k]);
        }
        return result;
    }else{//can be a mult or a sym or a pow node
        RCP<const Basic> possibleResult;
        possibleResult = returnBernsteinAdjustedPow_Or_SymNode(expr);
        if(possibleResult.is_null()){
            possibleResult = returnBernsteinAdjustedMultNode(expr);
        }
        result = possibleResult;
    }
    return result;
}

RCP<const Basic> BernsteinDataConverter::helperFuncs::NodeLogic::returnBernsteinAdjustedSymbol(SymEngine::RCP<const SymEngine::Basic> &symbol){
    
    if(is_a<Symbol>(*symbol)){
        
        uint16_t tagOfSymbol = Nucleus.coldStorage.returnTag_givenVarSymbol(rcp_static_cast<const SymEngine::Symbol>(symbol));
        domain domainOfSymbol = Nucleus.coldStorage.returnDomain(tagOfSymbol);
        RCP<const Basic> returnExpr = add(real_double((double)domainOfSymbol.min), mul(real_double((double)domainOfSymbol.max - (double)domainOfSymbol.min),symbol));
        return returnExpr;
    }else{
        throw std::runtime_error("Non-symbol node provided to function that only takes symbol-type nodes!");
    }
}

//assumes expanded expr!
RCP<const Basic> BernsteinDataConverter::helperFuncs::NodeLogic::returnBernsteinAdjustedPow_Or_SymNode(SymEngine::RCP<const SymEngine::Basic> &expr){
    RCP<const Basic> result;
    if (is_a<Pow>(*expr)) {
        auto decastedVar = rcp_static_cast<const Pow>(expr);
        auto base = decastedVar->get_base();
        auto exponent = decastedVar->get_exp();
            base = returnBernsteinAdjustedSymbol(base);//modify base to adjust for domain
        result = pow(base, exponent);//re-exponentiate it.
        result = expand(result);
        return result;
    }else if(is_a<Symbol>(*expr)){
        result = returnBernsteinAdjustedSymbol(expr);//modify expression in-place--cuz its a symbol, no need to decast
        result = expand(result);
        return result;
    } else if(is_a<RealDouble>(*expr)){
        return expr;//if just a constant no change
    } else{
        return RCP<const Basic>();//if not a pow node then return null
    }
}

//assumes expanded expr!
RCP<const Basic> BernsteinDataConverter::helperFuncs::NodeLogic::returnBernsteinAdjustedMultNode(SymEngine::RCP<const SymEngine::Basic> &expr){
    if (is_a<Mul>(*expr)) {
        RCP<const Basic> result = integer(1);
        auto decastedVar = rcp_static_cast<const Mul>(expr);
        auto arguments = decastedVar->get_args();
        for(int v = 0; v < arguments.size(); v++){
            arguments[v] = returnBernsteinAdjustedPow_Or_SymNode(arguments[v]);
        }
        //now that all arguments have been replaced, reconstruct the original expression
        for(int h = 0; h < arguments.size(); h++){
            result = mul(result, arguments[h]);
        }
        result = expand(result);
        return result;//dont modify the original expression but return a copy of the bernstein-modified values
    }else{
        return RCP<const Basic>();//if not a mult node then return null
    }
}



int BernsteinDataConverter::processExpressionForBernsteinCoefficientsCalculation(SymEngine::RCP<const SymEngine::Basic> &expression, std::vector<InfoBytes>* ptrToInfoByteBuffer){
    
    RCP<const Basic> localCopy = expression;
    localCopy = helperFuncs::NodeLogic::returnBernsteinAdjustedExpression(expression);
        helperFuncs::STATUS = helperFuncs::varConverterStatus::varsModifiedToBernsteinEquivalents;
    int productOfDimensions = calculateInfoBytes(localCopy, ptrToInfoByteBuffer);
    helperFuncs::STATUS = helperFuncs::varConverterStatus::varsInOriginalState;//original expression remains unmodified
    
    return productOfDimensions;
}




void calculateBernsteinCoeffs::calculateAndWriteInfoBytes(SymEngine::RCP<const SymEngine::Basic>& expr, std::vector<subcommands>& subcommandsVec){
    assert(bernCoeffStatus == bernsteinCoeffCalculationStatus::unprimed);
    assert(subcommandsVec.empty());//shoudlnt have been filled before this
    
    //calculate info bytes
    std::vector<InfoBytes> infoByteBuffer; refDat.productOfAllDimensions = BernsteinDataConverter::processExpressionForBernsteinCoefficientsCalculation(expr, &infoByteBuffer);
    assert(infoByteBuffer.size() > 0);//shouldve been filled
    refDat.metalInfVec = infoByteBuffer;
    
    auto& metalInfBuffer = refDat.metalInfVec;
    //stuff is broken not functioning yet fix the bernstein kernel such that you only fire it once for a whole groupp of polynomials so that the result finding func only fires once and also fix up result finder kernel to be more than just one threadgroup of 64 leaving it now just to test
        std::span<InfoBytes> infoByteSpan(metalInfBuffer.begin(), metalInfBuffer.end());//need to fire bernstein coeffs seperately for each term, and also need to copy it to the very first slot.
        
        GPU_lieutenant::addCopyCommand<InfoBytes>(infoByteSpan, bufferTypes::PRIVATE_inputDataBuffer);//copy infobytes to priate data staging buffer
        subcommandsHelper::appendDataCopyOp(infoByteSpan, bufferTypes::PRIVATE_inputDataBuffer, subcommandsVec);//copy infobytes to gpu
    refDat.numTerms = infoByteSpan.size();
    std::span<uint16_t> numTermsSpan(&refDat.numTerms,1);
    GPU_lieutenant::addCopyCommand<uint16_t>(numTermsSpan, bufferTypes::SHARED_initializerBuffer);//the number of info bytes. copy this over so each thread can execute all the variables. copied to shared buffer.
    
    bernCoeffStatus = bernsteinCoeffCalculationStatus::DISPATCHED_INFO_BYTES;
}

void calculateBernsteinCoeffs::bindCalculationAndMAXMIN_FINDER_kernels(std::vector<subcommands>& subcommandsVec){
    assert(bernCoeffStatus == bernsteinCoeffCalculationStatus::DISPATCHED_INFO_BYTES);
    assert(subcommandsVec.size() == 1);//the one command it should have is the data copy op of the infobytes
    
    int threadsPerGroup = 64;
        
        subcommandsHelper::appendKernelOp(kernels::bernstein_coefficient_calculations, refDat.productOfAllDimensions, threadsPerGroup, subcommandsVec);
    
    int threadgroupsForMaxMin = 1;//change later if needed
    int threadsPerGroupForMaxMin = 64;
    std::span<uint32_t> spanOfMatriceVolume(&refDat.productOfAllDimensions, 1);
    GPU_lieutenant::addCopyCommand(spanOfMatriceVolume, bufferTypes::SHARED_initializerBuffer);//copy over number of points
    subcommandsHelper::appendKernelOp(kernels::bernstein_coeffs_max_min_finder, threadgroupsForMaxMin, threadsPerGroupForMaxMin, subcommandsVec);
    
    bernCoeffStatus = bernsteinCoeffCalculationStatus::DISPATCHED_KERNEL_OP;
}

void calculateBernsteinCoeffs::dispatchResultCopyCommand(std::vector<subcommands>& subcommandsVec){
    assert(bernCoeffStatus == bernsteinCoeffCalculationStatus::DISPATCHED_KERNEL_OP);
    std::span<domain> domainSpan(refDat.writeBackValsTo, 1);
    subcommandsHelper::appendDataCopyOp<domain>(domainSpan, bufferTypes::PRIVATE_outputBuffer, subcommandsVec);//data copy op only looks at span for size and not where to write to. unlike copyCommand.
    GPU_lieutenant::addMetalCommand(subcommandsVec);
    
    GPU_lieutenant::addCopyCommand(domainSpan, bufferTypes::PRIVATE_outputBuffer, false, refDat.funcToCallbackAferResult);
    
    bernCoeffStatus = bernsteinCoeffCalculationStatus::DISPATCHED_COPY_BACK_CMD;
    
    //still needs to be fired away...
}

void calculateBernsteinCoeffs::findMaxAndMinBernsteinCoeffs(SymEngine::RCP<const SymEngine::Basic>& expr, domain* writeBackResultsTo, void(*funcToCallbackAferResult)()){
    refDat.writeBackValsTo = writeBackResultsTo;
    //assert(funcToCallbackAferResult != nullptr);//metal is supposed to check if its nullptr
    refDat.funcToCallbackAferResult = funcToCallbackAferResult;
    
    calculateAndWriteInfoBytes(expr, refDat.subcommandsVec);
    bindCalculationAndMAXMIN_FINDER_kernels(refDat.subcommandsVec);
    dispatchResultCopyCommand(refDat.subcommandsVec);
}


