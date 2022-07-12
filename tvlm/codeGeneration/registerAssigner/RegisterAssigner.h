#pragma once

#include "t86/program/label.h"
#include "tvlm/tvlm/codeGeneration/ProgramBuilder.h"


namespace tvlm{

    class RegisterAssigner {
    public:

        using Label = tiny::t86::Label;
        using DataLabel = tiny::t86::DataLabel;
        using Register = tiny::t86::Register;
        using FRegister = tiny::t86::FloatRegister;

        virtual ~RegisterAssigner() = default;

        explicit RegisterAssigner(ProgramBuilder * pb);
        Register getReg(const Instruction * ins){
            auto it = assignedIntRegisters_.find(ins);
            if(it != assignedIntRegisters_.end()){
                return it->second;
            }else{
                auto reg = getFreeIntRegister();
                assignedIntRegisters_.emplace(ins, reg);
                return reg;
            }

        }
        FRegister getFReg(const Instruction * ins){
            auto it = assignedFloatRegisters_.find(ins);
            if(it != assignedFloatRegisters_.end()){
                return it->second;
            }else{
                auto reg =  getFreeFloatRegister();
                assignedFloatRegisters_.emplace(ins, reg);
                return reg;
            }

        }
        Register getFreeIntRegister() {
            return {regIntCounter_++};
        }
        FRegister getFreeFloatRegister() {
            return {regFloatCounter_++};
        }

        void replaceIntReg(const Instruction * ins, const Instruction * with){
            auto it = assignedIntRegisters_.find(ins);
            if(it != assignedIntRegisters_.end()){

                assignedIntRegisters_.emplace(with, it->second);
            }else{
                return;
            }
        }
        void replaceFloatReg(const Instruction * ins, const Instruction * with){
            auto it = assignedFloatRegisters_.find(ins);
            if(it != assignedFloatRegisters_.end()){

                assignedFloatRegisters_.emplace(with, it->second);
            }else{
                return;
            }
        }

        virtual void registerPhi(Phi * phi) {
//            for(auto & content : phi->contents() ){
//                auto f = std::find(alloc_regs_.begin(), alloc_regs_.end(), content.second);
//                if(f != alloc_regs_.end()){
//                    *f = phi;
//                    break;
//                }
//            }
        }

        void makeLocalAllocation(size_t size, const Register &reg, const Instruction * ins);

    private:
        ProgramBuilder *  pb_;
        size_t functionLocalAllocSize;
        std::map<const Instruction *, Register> assignedIntRegisters_;
        std::map<const Instruction *, FRegister> assignedFloatRegisters_;
        size_t regIntCounter_;
        size_t regFloatCounter_;

    };
}
