#pragma once


#include "tvlm/tvlm/il/il.h"
#include "t86/program/label.h"
#include "tvlm/tvlm/codeGeneration/ProgramBuilder.h"


namespace  tvlm{


    class SuperNaiveIS : public ILVisitor{

    using Label = tiny::t86::Label;
    using DataLabel = tiny::t86::DataLabel;
    using Register = tiny::t86::Register;
    using FRegister = tiny::t86::FloatRegister;

    public:
        ~SuperNaiveIS() = default;
        SuperNaiveIS(){

        }
    static tiny::t86::Program translate(ILBuilder &ilb) ;
    protected:
        void visit(Instruction *ins) override;
        void visit(Jump *ins) override;
        void visit(CondJump *ins) override;
        void visit(Return *ins) override;
        void visit(CallStatic *ins) override;
        void visit(Call *ins) override;
        void visit(Copy *ins) override;
        void visit(Extend *ins) override;
        void visit(Truncate *ins) override;
        void visit(BinOp *ins) override;
        void visit(UnOp *ins) override;
        void visit(LoadImm *ins) override;
        void visit(AllocL *ins) override;
        void visit(AllocG *ins) override;
        void visit(ArgAddr *ins) override;
        void visit(PutChar *ins) override;
        void visit(GetChar *ins) override;
        void visit(Load *ins) override;
        void visit(Store *ins) override;
        void visit(Phi *ins) override;
        void visit(ElemAddrOffset *ins) override;
        void visit(ElemAddrIndex *ins) override;
        void visit(Halt *ins) override;
        void visit(StructAssign *ins) override;
        void visit(BasicBlock *bb) override;
        void visit(Function *fce) override;
        void visit(Program *p) override;

    private:

    };
}
