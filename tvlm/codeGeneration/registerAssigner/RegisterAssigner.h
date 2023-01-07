#pragma once

#include "t86/program/label.h"
#include "tvlm/tvlm/codeGeneration/ProgramBuilder.h"


namespace tvlm{

    class RegisterAssigner :public TargetProgramFriend {
    public:

        using Label = tiny::t86::Label;
        using DataLabel = tiny::t86::DataLabel;
        using Register = tiny::t86::Register;
        using FRegister = tiny::t86::FloatRegister;

        virtual ~RegisterAssigner() = default;

//        explicit RegisterAssigner(ProgramBuilderOLD * pb);
        explicit RegisterAssigner( TargetProgram * targetProg);

        size_t getReg(const Instruction * ins, const Instruction * me){
//        size_t getReg(const Instruction * ins){
            auto it = assignedIntRegisters_.find(ins);
            if(it != assignedIntRegisters_.end()){
//                auto allocReg = targetProgram_->alocatedRegisters_.find(ins);
//                if(allocReg == targetProgram_->alocatedRegisters_.end()){
//                    throw "RegAssigner: this shouldn't happen";
//                }
//                allocReg.
                auto virt =  VirtualRegisterPlaceholder(RegisterType::INTEGER, it->second.index());
                return targetProgram_->registerAdd(me, std::move(virt));
            }else{
                auto reg = getFreeIntRegister();
                assignedIntRegisters_.emplace(ins, reg);
//                return reg;
                auto virt =  VirtualRegisterPlaceholder(RegisterType::INTEGER, reg.index());
                return targetProgram_->registerAdd(me, std::move(virt));
            }


        }
        size_t getFReg(const Instruction * ins, const Instruction * me){
            auto it = assignedFloatRegisters_.find(ins);
            if(it != assignedFloatRegisters_.end()){
                auto virt =  VirtualRegisterPlaceholder(RegisterType::FLOAT, it->second.index());
                return targetProgram_->registerAdd(me, std::move(virt));
            }else{
                auto reg =  getFreeFloatRegister();
                assignedFloatRegisters_.emplace(ins, reg);
                auto virt =  VirtualRegisterPlaceholder(RegisterType::FLOAT, reg.index());
                return targetProgram_->registerAdd(me, std::move(virt));
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

        int64_t makeLocalAllocation(int64_t size, const Instruction * ins);
        int64_t makeGlobalAllocation(int64_t size, const Instruction * ins);

        int64_t getAllocOffset(const Instruction * ins) const;
//        void allocateStructArg(const Type * type,const Instruction * ins);
//        void correctStackAlloc(size_t patch);

        size_t prepareReturnValue(size_t size,const Instruction * ret, const Instruction * me);
//        void spillCallReg();

        void registerPhi(const Phi *phi);
//        virtual void prepareReturnValue(size_t size, const ILInstruction * ret);
        virtual void resetAllocSize();
        void exportAlloc(Function * fnc);

    private:

//        void copyStruct(Register aRegister,const Type *pType, Register aRegister1, const Instruction * ins);

//        ProgramBuilderOLD *  pb_;
        TargetProgram * targetProgram_;
        int64_t functionLocalAllocSize;
        int64_t globalAllocSize;
        std::map<const Instruction *, Register> assignedIntRegisters_;
        std::map<const Instruction *, FRegister> assignedFloatRegisters_;
        size_t regIntCounter_;
        size_t regFloatCounter_;

    };
}
