#pragma once

#include "t86/program/helpers.h"
#include "tvlm/tvlm/il/il.h"
#include "tvlm/tvlm/codeGeneration/ProgramBuilder.h"

namespace tvlm{
    using Register = tiny::t86::Register;
    using FRegister = tiny::t86::FloatRegister;
    using TInstruction = tiny::t86::Instruction;
    using ILInstruction = ::tvlm::Instruction;

    /*abstract*/ class RegisterAllocator {

    public:
        virtual ~RegisterAllocator() = default;
        explicit RegisterAllocator(ProgramBuilder * pb);

        /**
         * getReg : Code generator uses getReg function to determine the status of available registers
         *           and the location of name values. getReg works as follows:\n
         *
         * - If variable Y is already in register R, it uses that register.\n
         * - Else if some register R is available, it uses that register.\n
         * - Else if both the above options are not possible, it chooses a register that requires minimal number of load and store instructions.
         * */
        virtual Register getReg(const ILInstruction * ins) = 0;
        virtual FRegister getFloatReg(const ILInstruction * ins) = 0;

        virtual void clearInt(const ILInstruction * ins) = 0;
        virtual void clearFloat(const ILInstruction * ins) = 0;
        virtual void spillCallReg() = 0;
        virtual void clearAllReg() = 0;


        Register fillIntRegister(){
            auto reg = getFreeIntRegister();
            tmpIntRegs_.emplace(reg);
            return reg;
        }

        FRegister fillFloatRegister(){
            auto reg = getFreeFloatRegister();
            tmpFloatRegs_.emplace(reg);
            return reg;
        }

        virtual void clearIntRegister(const Register & reg ){
            tmpIntRegs_.erase(reg);
            size_t regi= reg.index();
            alloc_regs_[regi] = nullptr;
        }

        void replaceInt(const ILInstruction * from, const ILInstruction * to ){
            alloc_regs_[getIntRegister(from).index()] = to;
        }
        void replaceFloat(const ILInstruction * from, const ILInstruction * to ){
            alloc_fregs_[getFloatRegister(from).index()] = to;
        }
        virtual void spillAllReg()  = 0;

        virtual void registerPhi(Phi * phi) {
            for(auto & content : phi->contents() ){
                auto f = std::find(alloc_regs_.begin(), alloc_regs_.end(), content.second);
                if(f != alloc_regs_.end()){
                    *f = phi;
                    break;
                }
            }
        }


        virtual void prepareReturnValue(size_t size, const ILInstruction * ret) = 0;
        virtual void makeLocalAllocation(size_t size, const Register & reg, const ILInstruction * ins) = 0;

        virtual void allocateStructArg(const Type * type, const ILInstruction * ins) = 0;

        virtual void resetAllocSize() = 0;

        virtual void correctStackAlloc(size_t patch ) = 0;

    protected:
        virtual Register getIntRegister(const ILInstruction * ins) = 0;
        virtual FRegister getFloatRegister(const ILInstruction * ins) = 0;

        virtual Register getFreeIntRegister() = 0; //care not to take tmpAllocated
        virtual FRegister getFreeFloatRegister() = 0; //care not to take tmpAllocated


        std::vector< const ILInstruction *> alloc_regs_; //register descriptor
        std::vector< const ILInstruction *> alloc_fregs_;

        std::set<Register> tmpIntRegs_;
        std::set<FRegister> tmpFloatRegs_;

        ProgramBuilder * pb_;


    };
}
