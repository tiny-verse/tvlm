#pragma once

#include "RegisterAllocator.h"
#include "t86/instruction.h"
#include "t86/t86_target"
#include "t86/program.h"
#include "t86/program/programbuilder.h"

namespace tvlm{

    class NaiveRegisterAllocator :public RegisterAllocator{

    public:
        NaiveRegisterAllocator(tiny::t86::ProgramBuilder * pb):
                RegisterAllocator(), pb_(pb){}

        Register fillIntRegister(Instruction *ins) override;

        FRegister fillFloatRegister(Instruction *ins) override;

        void clearInt(Instruction *ins) override;

        void clearFloat(Instruction *ins) override;

        void spillCallReg() override;

        void clearAllReg() override;

        void spillAllReg() override;

        void registerPhi(Phi *phi) override;

        void clearTmpIntRegister(const Register &reg) override;

        void prepareReturnValue(size_t size)override{
            if(size == 0){
                auto regTmp  = fillTmpIntRegister();  // tmpAddress prepared for return Value
                pb_->add(tiny::t86::PUSH(regTmp));
                clearTmpIntRegister(regTmp);
            }else{
                functionLocalAllocSize += size;
                auto regTmp  = fillTmpIntRegister();  // tmpAddress prepared for return Value
                pb_->add(tiny::t86::MOV(regTmp ,tiny::t86::Bp()));
                pb_->add(tiny::t86::SUB(regTmp, (int64_t)functionLocalAllocSize));
                pb_->add(tiny::t86::PUSH(regTmp));
                clearTmpIntRegister(regTmp);
            }
        }

        void makeLocalAllocation(size_t size, const Register &reg) override;

        void allocateStructArg(Type * type, Instruction * ins) override;

        void resetAllocSize() override;

        void correctStackAlloc(size_t patch) override;

    protected:
        Register getFreeIntRegister() override;

        FRegister getFreeFloatRegister() override;

        Register getIntRegister(Instruction *ins) override;

        FRegister getFloatRegister(Instruction *ins) override;

        int counter = 1;
        int fcounter = 1;
        const int reserve_upbound = 1;
        size_t functionLocalAllocSize = 0;

        tiny::t86::ProgramBuilder * pb_;

        std::unordered_map<const Instruction *, int64_t> spilled_; //ins, mem_offset
        std::pair<bool, int> findInIntRegs(Instruction *ins);
        std::pair<bool, int> findInFloatRegs(Instruction *ins);

        bool spillIntReg(const Register & reg){


            //if empty Reg - do nothing
            if (!alloc_regs_[reg.index()]) {
                return false;
            }

            const Instruction *ins_to_spill = alloc_regs_[reg.index()];

            int64_t mem_offset = std::numeric_limits<int64_t>::max();
            auto it = spilled_.find(ins_to_spill);
            if(it == spilled_.end()){
                functionLocalAllocSize+=1;
                mem_offset =  (int64_t)functionLocalAllocSize;
                //pb_.add(t86::SUB(t86::Sp(), 1)); // aggregating allocation - no need anymore
                spilled_.emplace(ins_to_spill, mem_offset);

            }else{
                mem_offset = it->second;
            }


            pb_->add(tiny::t86::MOV(tiny::t86::Mem(tiny::t86::Bp() - mem_offset), reg));

            alloc_regs_[reg.index()] = nullptr;
            return true;
        }

        bool spillFloatReg(const FRegister & reg){


            //if empty Reg - do nothing
            if (!alloc_fregs_[reg.index()]) {
                return false;
            }

            const Instruction *ins_to_spill = alloc_fregs_[reg.index()];

            int64_t mem_offset = std::numeric_limits<int64_t>::max();
            auto it = spilled_.find(ins_to_spill);
            if(it == spilled_.end()){
                functionLocalAllocSize+=1;
                mem_offset =  (int64_t)functionLocalAllocSize;
                //pb_.add(t86::SUB(t86::Sp(), 1)); // aggregating allocation - no need anymore
                spilled_.emplace(ins_to_spill, mem_offset);

            }else{
                mem_offset = it->second;
            }


            pb_->add(tiny::t86::MOV(tiny::t86::Mem(tiny::t86::Bp() - mem_offset), reg));

            alloc_fregs_[reg.index()] = nullptr;
            return true;
        }


        void copyStruct(Register aRegister, Type *pType, Register aRegister1);

        template<typename T>
        void replace(size_t label,const T & instruction){
            if(label >= pb_->currentLabel()){
                //ERROR
                std::cerr << "replace unable to continue" << std::endl;
                return;
            }
            auto p = pb_->program();
            auto instrs = p.moveInstructions();
            delete instrs[label];
            instrs[label] = new T(instruction);

            auto resProg = tiny::t86::Program(instrs, p.data());
            *pb_ = tiny::t86::ProgramBuilder(std::move(resProg));
        }
    };


}