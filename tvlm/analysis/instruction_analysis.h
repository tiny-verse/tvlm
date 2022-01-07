#pragma once
#include "../il.h"

#include "./lattice/map_lattice.h"
#include "./lattice/powerset_lattice.h"
#include "cfg.h"
#include "analysis.h"


namespace tvlm{
    using Declaration = tvlm::IL*;
    using Declarations = MAP< tvlm::Instruction *, Declaration>;





    class InstructionAnalysis : public Analysis<Declarations>{
        using Env = MAP<VirtualRegisterPlaceholder *, Declaration>;
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
            InsVisitor():env_(nullptr){

            }
        protected:
            Env env_;

            VirtualRegisterPlaceholder * getVirtualReg(const Declaration pIl);
            std::map<IL*, std::unique_ptr<VirtualRegisterPlaceholder>> virtualRegs_;

            Env extendEnv(Env & env, const std::vector<Declaration> & decls){
                auto acc = env;
                for (const auto & d : decls) {
                    VirtualRegisterPlaceholder * reg = getVirtualReg(d);
                    auto tmp = acc.access(reg);
                    if(!tmp){
                        acc.insert(std::make_pair( reg, d));
                    }
                }
                return acc;
            }


            void visitChild(::tvlm::IL * il, const Env & env) {
                Env tmp = env_;
                env_ = env;
                ILVisitor::visitChild(il);
                env_ = tmp;
            }

            template<typename T>
            void visitChild(std::unique_ptr<T> const &ptr, const Env & env) {
                return visitChild(ptr.get(), env);
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
