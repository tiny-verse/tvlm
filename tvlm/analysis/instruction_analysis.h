#pragma once


#include <memory>
#include "./lattice/map_lattice.h"
#include "./lattice/powerset_lattice.h"
#include "cfg.h"
#include "tvlm/tvlm/il/il.h"
#include "../t86_backend.h"
#include "tvlm/backendUtility/VirtualRegister.h"

#include "analysis.h"

namespace tvlm{
    using Declaration = tvlm::IL;
    using Declarations = MAP< tvlm::Instruction *, Declaration*>;




    class InstructionAnalysis : public Analysis<Declarations>{
        using Env = MAP<VirtualRegisterPlaceholder*, Declaration*>;
    public:
        InstructionAnalysis(Program * p): Analysis<Declarations>(), p_(p){

        }
        InstructionAnalysis(ILBuilder * p): Analysis<Declarations>(), p_(p){

        }

        Declarations  analyze() override{
            auto visitor = std::make_unique<InsVisitor>();
            if(std::holds_alternative<Program*>(p_)){
                visitor->visit(std::get<Program*>(p_));
            }else{
                visitor->visit(std::get<ILBuilder*>(p_)->)
            }
            //for each
            return std::move(visitor->declarations);
        }

    protected:
        class InsVisitor : public ::tvlm::ILVisitor{
        public:
            InsVisitor():
            env_(Env())
            {}
            Declarations declarations;
        protected:
            Env env_;

            VirtualRegisterPlaceholder * getVirtualReg(const Declaration *pIl);
            std::map<const IL*, std::unique_ptr<VirtualRegisterPlaceholder>> virtualRegs_;

            Env extendEnv(Env & env, const std::vector<Declaration*> & decls);


            void visitChild(::tvlm::IL * il, const Env & env) {
//                Env tmp = env_;
//                env_ = env;
                ILVisitor::visitChild(il);
//                env_ = tmp;
            }


            template<typename T>
            void visitChild(std::unique_ptr<T> const &ptr, const Env & env) {
                return visitChild(ptr.get(), env);
            }

            void visit(Instruction *ins) override{};

        protected:
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

        public:
            void visit(Program *p) override;
            void visit(ILBuilder *p) override;
        };

    private:
        std::variant<Program *, ILBuilder*> p_;
        static size_t counter_;
    };
}
