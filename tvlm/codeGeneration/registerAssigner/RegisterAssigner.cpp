#include "RegisterAssigner.h"
#include "t86/program/helpers.h"
#include "tvlm/tvlm/il/il.h"
#include "tvlm/tvlm/codeGeneration/FunctionalMacro.h"

namespace tvlm{

    int64_t RegisterAssigner::makeLocalAllocation(int64_t size, const ILInstruction *ins) {
        functionLocalAllocSize += size;
        auto cpy =  functionLocalAllocSize -1 ; // make cpy for lambda capture
        targetProgram_->registerAllocation(ins, cpy);
        return cpy;
    }

    int64_t RegisterAssigner::makeGlobalAllocation(int64_t size,  const ILInstruction *ins) {
        auto cpy = globalAllocSize; //make cpy for lambda capture
        globalAllocSize += size;
        targetProgram_->registerAllocation(ins, cpy);
        return cpy;
    }


    int64_t RegisterAssigner::getAllocOffset(const Instruction * ins) const{
        auto it = getAllocMapping(targetProgram_).find(ins);
        if(it != getAllocMapping(targetProgram_).end()){
            return it->second;
        }
        throw "[RegAssigner] cannot get allocation offset for non-registered Instruction";
    }

    void RegisterAssigner::registerPhi(const Phi *phi) {

    }

    void RegisterAssigner::resetAllocSize() {
        functionLocalAllocSize = 1;
    }

    void RegisterAssigner::exportAlloc(Function * fnc){
        getFuncLocalAlloc(targetProgram_)[fnc] = functionLocalAllocSize;
    }

    RegisterAssigner::RegisterAssigner( TargetProgram * program):
    targetProgram_(program)
    ,functionLocalAllocSize(1)
    ,globalAllocSize(0)
    ,regIntCounter_(1)
    ,regFloatCounter_(1) {
    resetAllocSize();
    }

    //returns gerister in which the address of value should be placed
    size_t RegisterAssigner::prepareReturnValue(size_t size, const Instruction *ret, const Instruction * me) {
        size_t regTmp;
        if(size == 0){
            regTmp  = getReg(ret, me);  // tmpAddress prepared for return Value
//            targetProgram_->addF( LMBS tiny::t86::PUSH(regTmp) LMBE,ret ); only imaginary  ... move stack pointer is enough ( we dont have value in tmpReg
            targetProgram_->addF( LMBS tiny::t86::SUB(tiny::t86::Sp(), 1) LMBE,ret ); functionLocalAllocSize++; // Caller allocs place(on stack) for return value

            targetProgram_->addF( LMBS tiny::t86::MOV(vR(regTmp) ,tiny::t86::Bp()) LMBE, ret);
            targetProgram_->addF( LMBS tiny::t86::SUB(vR(regTmp), (int64_t)functionLocalAllocSize) LMBE, ret);
        }else{
            regTmp  = getReg(ret, me);  // tmpAddress prepared for return Value
            targetProgram_->addF( LMBS tiny::t86::MOV(vR(regTmp) ,tiny::t86::Bp()) LMBE, ret);
            targetProgram_->addF( LMBS tiny::t86::SUB(vR(regTmp), (int64_t)functionLocalAllocSize) LMBE, ret);
            functionLocalAllocSize += size;
        }
        return regTmp;
    }

}