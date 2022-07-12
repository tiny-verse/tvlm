#include "IlTiler.h"
#include "tvlm/il/il.h"
#include "tvlm/il/il_builder.h"


namespace tvlm{

    void ILTiler::tile(ILBuilder &il) {
        // add rules to possible tilings for every instruction

        for (auto & f : il.functions()) {
            for (auto & bb : getFunctionBBs(f.second.get())) {
                for (auto & ins: getBBsInstructions(bb)) {
                    tile(ins);
                }
            }
        }

    }

    void ILTiler::tile(Instruction * ins){

    }


    void ILTiler::visit(Instruction *ins) {

    }


    void ILTiler::visit(Jump *ins) {

    }

    void ILTiler::visit(CondJump *ins) {

    }

    void ILTiler::visit(Return *ins) {

    }

    void ILTiler::visit(CallStatic *ins) {

    }

    void ILTiler::visit(Call *ins) {

    }

    void ILTiler::visit(Copy *ins) {

    }

    void ILTiler::visit(Extend *ins) {

    }

    void ILTiler::visit(Truncate *ins) {

    }

    void ILTiler::visit(BinOp *ins) {

    }

    void ILTiler::visit(UnOp *ins) {

    }

    void ILTiler::visit(LoadImm *ins) {

    }

    void ILTiler::visit(AllocL *ins) {

    }

    void ILTiler::visit(AllocG *ins) {

    }

    void ILTiler::visit(ArgAddr *ins) {

    }

    void ILTiler::visit(PutChar *ins) {

    }

    void ILTiler::visit(GetChar *ins) {

    }

    void ILTiler::visit(Load *ins) {

    }

    void ILTiler::visit(Store *ins) {

    }

    void ILTiler::visit(Phi *ins) {

    }

    void ILTiler::visit(ElemAddrOffset *ins) {

    }

    void ILTiler::visit(ElemAddrIndex *ins) {

    }

    void ILTiler::visit(Halt *ins) {

    }

    void ILTiler::visit(StructAssign *ins) {

    }

    void ILTiler::visit(BasicBlock *bb) {

    }

    void ILTiler::visit(Function *fce) {

    }

    void ILTiler::visit(Program *p) {

    }

}