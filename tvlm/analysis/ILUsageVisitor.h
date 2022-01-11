#pragma once

#include "../il.h"
#include "instruction_analysis.h"


namespace tvlm{

    class ILUsageVisitor : public ILVisitor {
    public:
//        ILUsageVisitor(): ILVisitor(), result_(){}
        std::unordered_set<Declaration*>result(){
            return result_;
        }

        void begin(IL * ins){
            ILVisitor::visitChild(ins);
        }
    protected:
        void visit(Instruction *ins) override{};
        void visit(Instruction::Terminator0 *ins) override;

        void visit(Instruction::Terminator1 *ins) override;

        void visit(Instruction::TerminatorN *ins) override;

        void visit(Instruction::Returnator *ins) override;

        void visit(Instruction::DirectCallInstruction *ins) override;

        void visit(Instruction::IndirectCallInstruction *ins) override;

        void visit(Instruction::SrcInstruction *ins) override;

        void visit(Instruction::BinaryOperator *ins) override;

        void visit(Instruction::UnaryOperator *ins) override;

        void visit(Instruction::ImmIndex *ins) override;

        void visit(Instruction::ImmSize *ins) override;

        void visit(Instruction::ImmValue *ins) override;

        void visit(Instruction::VoidInstruction *ins) override;

        void visit(Instruction::LoadAddress *ins) override;

        void visit(Instruction::StoreAddress *ins) override;

        void visit(Instruction::PhiInstruction *ins) override;

        void visit(Instruction::ElemInstruction *ins) override;

        void visit(BasicBlock *bb) override;
        void visit(Function *fce) override;
        void visit(Program *p) override;

    protected:
        std::unordered_set<Declaration*> result_;
    };
}
