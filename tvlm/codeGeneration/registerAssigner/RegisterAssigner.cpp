#include "RegisterAssigner.h"
#include "t86/program/helpers.h"
#include "tvlm/tvlm/il/il.h"

namespace tvlm{


    RegisterAssigner::RegisterAssigner(ProgramBuilder *pb):
    pb_(pb)
    ,functionLocalAllocSize(0)
    ,regIntCounter_(1)
    ,regFloatCounter_(1)
    {}


    void RegisterAssigner::makeLocalAllocation(size_t size, const Register &reg, const ILInstruction *ins) {
        functionLocalAllocSize += size;
        // already allocated, now just find addr for this allocation
        pb_->add(tiny::t86::MOV(reg, tiny::t86::Bp()), ins);
        pb_->add(tiny::t86::SUB(reg, (int64_t) functionLocalAllocSize), ins);
    }

    void RegisterAssigner::makeGlobalAllocation(size_t size, const Register &reg, const ILInstruction *ins) {
        pb_->add(tiny::t86::MOV(reg, (int64_t) globalAllocSize), ins); //TODO TODO TODO make funtional
        globalAllocSize += size;
    }

    void RegisterAssigner::copyStruct(Register from,const Type *type, Register to,const ILInstruction *ins) {

        auto tmpReg = getFreeIntRegister();
        const Type::Struct *strct = dynamic_cast<const Type::Struct *>(type);
        for (int i = 0; i < strct->size(); ++i) {
            pb_->add(tiny::t86::MOV(tmpReg, tiny::t86::Mem(from + i)), ins);
            pb_->add(tiny::t86::MOV(tiny::t86::Mem(to + i), tmpReg), ins);
        }
    }

    void RegisterAssigner::allocateStructArg(const Type *type, const ILInstruction *ins) {
        functionLocalAllocSize += type->size();
        auto reg = getReg(ins); // // reg with a structure
        auto regTmp = getFreeIntRegister(); // Working reg
        pb_->add(tiny::t86::MOV(regTmp, tiny::t86::Bp()), ins);
        pb_->add(tiny::t86::SUB(regTmp, (int64_t) functionLocalAllocSize), ins);


        copyStruct(reg, type, regTmp, ins);

        pb_->add(tiny::t86::PUSH(regTmp), ins);
    }

    void RegisterAssigner::correctStackAlloc(size_t patch) {
        pb_->replace(patch, tiny::t86::SUB(tiny::t86::Sp(), (int64_t) functionLocalAllocSize));
    }

    void RegisterAssigner::registerPhi(const Phi *phi) {
//        for(auto & content : phi->contents() ){
//            auto f = std::find(alloc_regs_.begin(), alloc_regs_.end(), content.second);
//            if(f != alloc_regs_.end()){
//                *f = phi;
//                break;
//            }
//        }
    }

    void RegisterAssigner::prepareReturnValue(size_t size, const ILInstruction *ret) {
        if(size == 0){
            auto regTmp  = getReg(ret);  // tmpAddress prepared for return Value
            pb_->add(tiny::t86::PUSH(regTmp),ret );
//            clearIntRegister(regTmp);
        }else{
            functionLocalAllocSize += size;
            auto regTmp  = getReg(ret);  // tmpAddress prepared for return Value
            pb_->add(tiny::t86::MOV(regTmp ,tiny::t86::Bp()), ret);
            pb_->add(tiny::t86::SUB(regTmp, (int64_t)functionLocalAllocSize), ret);
            pb_->add(tiny::t86::PUSH(regTmp), ret);
//            clearIntRegister(regTmp);
        }
    }

    void RegisterAssigner::resetAllocSize() {
        functionLocalAllocSize = 0;
    }

    void RegisterAssigner::exportAlloc(Function * fnc){
        targetProgram_->funcLocalAlloc_[fnc] = functionLocalAllocSize;
    }

    RegisterAssigner::RegisterAssigner(ProgramBuilder *pb, TargetProgram * program):
    pb_(pb)
    ,targetProgram_(program)
    ,functionLocalAllocSize(0)
    ,regIntCounter_(1)
    ,regFloatCounter_(1) {

    }

}