#pragma once
#include "liveness_analysis.h"
#include "../il.h"

namespace tvlm{
    using Declaration = tvlm::Instruction*;
    using Declarations = MAP< tvlm::Instruction *, Declaration>;





    class InstructionAnalysis : public Analysis<Declarations>{
        using Env = MAP<ILInstruction *, Declaration>;
    public:
        InstructionAnalysis(Program * p): Analysis<Declarations>(), p_(p){

        }

        Declarations analyze() override{
            auto visitor = new InsVisitor();
            visitor->visit(p_);
            return visitor->declarations;
        }

    protected:
        class InsVisitor : public ::tvlm::ILVisitor{
        public:
            Declarations declarations;
        protected:
//            Env env_;
//            Env extendEnv(Env & env, std::vector<Declaration> & decls){
//                auto acc = env;
//                for (const auto & d : decls) {
//                    auto tmp = acc.access(d->name());
//                    acc.insert(std::make_pair(d->name(), d));
//                }
//            }


            void visitChild(::tvlm::IL * il) {
                ILVisitor::visitChild(il);
            }

            template<typename T>
            void visitChild(std::unique_ptr<T> const &ptr) {
                return visitChild(ptr.get());
            }

            void visit(Instruction *ins) override;

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

            void visit(::tvlm::BasicBlock *bb) override;

            void visit(Function *fce) override;

        public:
            void visit(Program *p) override;
        };

    private:
        Program * p_;
    };
}
