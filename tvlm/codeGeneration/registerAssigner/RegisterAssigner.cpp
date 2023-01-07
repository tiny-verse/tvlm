#include "RegisterAssigner.h"
#include "t86/program/helpers.h"
#include "tvlm/tvlm/il/il.h"
#include "tvlm/tvlm/codeGeneration/FunctionalMacro.h"

namespace tvlm{


//    RegisterAssigner::RegisterAssigner():
////    pb_(pb)
//    ,functionLocalAllocSize(0)
//    ,regIntCounter_(1)
//    ,regFloatCounter_(1)
//    {}


    int64_t RegisterAssigner::makeLocalAllocation(int64_t size, const ILInstruction *ins) {
        functionLocalAllocSize += size;
        // already allocated, now just find addr for this allocation
//        targetProgram_->addF(LMBS tiny::t86::MOV( vR(reg), tiny::t86::Bp()) LMBE, ins);
//        targetProgram_->addF(LMBS tiny::t86::SUB( vR(reg), (int64_t) cpy ) LMBE , ins);
        auto cpy =  functionLocalAllocSize -1 ; // make cpy for lambda capture
        targetProgram_->registerAllocation(ins, cpy);
        return cpy;
    }

    int64_t RegisterAssigner::makeGlobalAllocation(int64_t size,  const ILInstruction *ins) {
        auto cpy = globalAllocSize; //make cpy for lambda capture
        globalAllocSize += size;
        targetProgram_->registerAllocation(ins, cpy);
//        targetProgram_->addF( LMBS tiny::t86::MOV(vR(reg), (int64_t) cpy) LMBE, ins);
        return cpy;
    }


    int64_t RegisterAssigner::getAllocOffset(const Instruction * ins) const{
        auto it = getAllocMapping(targetProgram_).find(ins);
        if(it != getAllocMapping(targetProgram_).end()){
            return it->second;
        }
        throw "[RegAssigner] cannot get allocation offset for non-registered Instruction";
    }
//    void RegisterAssigner::copyStruct(Register from,const Type *type, Register to,const ILInstruction *ins) {
//
//        auto tmpReg = getFreeIntRegister();
//        const Type::Struct *strct = dynamic_cast<const Type::Struct *>(type);
//        for (int i = 0; i < strct->size(); ++i) {
//            targetProgram_->addF(LMBS tiny::t86::MOV(tmpReg, tiny::t86::Mem(from + i)), ins);
//            targetProgram_->add(tiny::t86::MOV(tiny::t86::Mem(to + i), tmpReg), ins);
//        }
//    }
//
//    void RegisterAssigner::allocateStructArg(const Type *type, const ILInstruction *ins) {
//        functionLocalAllocSize += type->size();
//        auto reg = getReg(ins); // // reg with a structure
//        auto regTmp = getFreeIntRegister(); // Working reg
//        targetProgram_->add(tiny::t86::MOV(regTmp, tiny::t86::Bp()), ins);
//        targetProgram_->add(tiny::t86::SUB(regTmp, (int64_t) functionLocalAllocSize), ins);
//
//
//        copyStruct(reg, type, regTmp, ins);
//
//        targetProgram_->add(tiny::t86::PUSH(regTmp), ins);
//    }
//
//    void RegisterAssigner::correctStackAlloc(size_t patch) {
//        pb_->replace(patch, tiny::t86::SUB(tiny::t86::Sp(), (int64_t) functionLocalAllocSize));
//    }

    void RegisterAssigner::registerPhi(const Phi *phi) {
        //TODO
//        for(auto & content : phi->contents() ){
//            auto f = std::find(alloc_regs_.begin(), alloc_regs_.end(), content.second);
//            if(f != alloc_regs_.end()){
//                *f = phi;
//                break;
//            }
//        }
    }

//    void RegisterAssigner::prepareReturnValue(size_t size, const ILInstruction *ret) {
//        if(size == 0){
//            auto regTmp  = getReg(ret);  // tmpAddress prepared for return Value
//            targetProgram_->add(tiny::t86::PUSH(regTmp),ret );
////            clearIntRegister(regTmp);
//        }else{
//            functionLocalAllocSize += size;
//            auto regTmp  = getReg(ret);  // tmpAddress prepared for return Value
//            targetProgram_->add(tiny::t86::MOV(regTmp ,tiny::t86::Bp()), ret);
//            targetProgram_->add(tiny::t86::SUB(regTmp, (int64_t)functionLocalAllocSize), ret);
//            targetProgram_->add(tiny::t86::PUSH(regTmp), ret);
////            clearIntRegister(regTmp);
//        }
//    }

    void RegisterAssigner::resetAllocSize() {
        functionLocalAllocSize = 1;
    }

    void RegisterAssigner::exportAlloc(Function * fnc){
        getFuncLocalAlloc(targetProgram_)[fnc] = functionLocalAllocSize;
    }

    RegisterAssigner::RegisterAssigner( TargetProgram * program):
//    pb_(pb)
//    ,
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
//            clearIntRegister(regTmp);
            targetProgram_->addF( LMBS tiny::t86::MOV(vR(regTmp) ,tiny::t86::Bp()) LMBE, ret);
            targetProgram_->addF( LMBS tiny::t86::SUB(vR(regTmp), (int64_t)functionLocalAllocSize) LMBE, ret);
        }else{
            regTmp  = getReg(ret, me);  // tmpAddress prepared for return Value
            targetProgram_->addF( LMBS tiny::t86::MOV(vR(regTmp) ,tiny::t86::Bp()) LMBE, ret);
            targetProgram_->addF( LMBS tiny::t86::SUB(vR(regTmp), (int64_t)functionLocalAllocSize) LMBE, ret);
            functionLocalAllocSize += size;
//            targetProgram_->addF( LMBS tiny::t86::PUSH(vR(regTmp)) LMBE, ret);
//            clearIntRegister(regTmp);
        }
        return regTmp;
    }

}