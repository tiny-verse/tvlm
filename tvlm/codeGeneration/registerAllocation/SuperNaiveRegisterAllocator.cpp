#include "SuperNaiveRegisterAllocator.h"

namespace tvlm {


    void SuperNaiveRegisterAllocator::visit(Instruction *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(Jump *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(CondJump *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(Return *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(CallStatic *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(Call *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(Copy *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(Extend *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(Truncate *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(BinOp *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(UnOp *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(LoadImm *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(AllocL *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(AllocG *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(ArgAddr *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(PutChar *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(GetChar *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(Load *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(Store *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(Phi *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(ElemAddrOffset *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(ElemAddrIndex *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(Halt *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(StructAssign *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(BasicBlock *bb) {
        enterNewBB(bb);

        for (auto * ins : getBBsInstructions(bb)) {
            visitChild(ins);
        }
        exitBB();
    }

    void SuperNaiveRegisterAllocator::visit(Function *fce) {

        for (BasicBlock * bb : getFunctionBBs(fce)){
            visitChild(bb);
        }


    }

    void SuperNaiveRegisterAllocator::visit(Program *p) {
        auto globals = getProgramsGlobals(p);
        for (auto * gl : getBBsInstructions(globals)) {
            enterNewBB(globals);
            visitChild(gl);
            exitBB();
        }

        for(auto & f : getProgramsFunctions(p)){
            visitChild(f.second);
        }

    }


}