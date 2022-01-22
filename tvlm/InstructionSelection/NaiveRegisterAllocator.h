#pragma once

#include "register_allocator.h"
namespace tvlm{

    class NaiveRegisterAllocator :public RegisterAllocator{
    public:
        NaiveRegisterAllocator():
                RegisterAllocator(){}

        Register fillIntRegister(Instruction *ins) override;

        FRegister fillFloatRegister(Instruction *ins) override;

        void clearInt(Instruction *ins) override;

        void clearFloat(Instruction *ins) override;

        void clearAllIntReg() override;

        void clearAllFloatReg() override;

        void clearAllReg() override;

        void spillAllReg() override;

        void registerPhi(Phi *phi) override;

    protected:
        Register getIntRegister(Instruction *ins) override;

        FRegister getFloatRegister(Instruction *ins) override;

        int counter = 0;
        int fcounter = 0;

        std::pair<bool, int> findInIntRegs(Instruction *ins);
        std::pair<bool, int> findInFloatRegs(Instruction *ins);
    };


}