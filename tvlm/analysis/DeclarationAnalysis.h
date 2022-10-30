#pragma once

#include "tvlm/tvlm/il/il.h"
#include "instruction_analysis.h"


namespace tvlm{

    class DeclarationAnalysis : public ILVisitor {
    public:
        DeclarationAnalysis(Program * prog):program_(prog){}
        virtual ~DeclarationAnalysis() = default;
//        ILUsageVisitor(): ILVisitor(), result_(){}
        std::unordered_set<Declaration*>result(){
            return result_;
        }

        void begin(IL * ins){
            ILVisitor::visitChild(ins);
        }
    protected:
        Program * program_;
        void visit(Instruction *ins) override{};
    public:
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

    protected:
        void visit(BasicBlock *bb) override;
        void visit(Function *fce) override;
        void visit(Program *p) override;

    protected:
        std::unordered_set<Declaration*> result_;
    };
}
