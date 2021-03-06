#pragma once
#include "tvlm/tvlm/il/il.h"
#include "t86/program/label.h"
#include "t86/program/programbuilder.h"
#include "RegisterAllocator.h"
#include "NaiveRegisterAllocator.h"

namespace tvlm{



    class NaiveIS  : public ILVisitor{
    public:
        using Label = tiny::t86::Label;
        using DataLabel = tiny::t86::DataLabel;
        using Register = tiny::t86::Register;
        using FRegister = tiny::t86::FloatRegister;


        static tiny::t86::Program translate(ILBuilder &ilb);

        void visit(Program *p) override;
    protected:
        NaiveIS();

        void visit(Instruction *ins) override{};

    public:

        void visit(ILBuilder & ilb);
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

    private:
        Label visitChild(IL * il) {
            ILVisitor::visitChild(il);
            return lastIns_;
        }

        template<typename T>
        Label visitChild(std::unique_ptr<T> const &ptr) {
            return visitChild(ptr.get());
        }

//        template<typename T>
//        Label add(Instruction * ins, const T& instruction) {
//            lastIns_ = pb_.add( instruction);
//            if(ins != nullptr ) compiled_.emplace(ins, lastIns_);
//            return lastIns_;
//        }

        template<typename T>
        Label add( const T& instruction) {
            lastIns_ = pb_.add( instruction);
            return lastIns_;
        }

        Label find(Instruction * ins){
            auto it = compiled_.find(ins);
            if(it == compiled_.end()){
                return Label::empty();
            }
            return it->second;
        }

        /** Prepare register for use with its value (sill if necessary)
         * */
        auto fillIntRegister(Instruction * ins){
            return regAllocator->fillIntRegister(ins);
        }
        auto fillFloatRegister(Instruction * ins){
            return regAllocator->fillFloatRegister(ins);
        }
        auto clearIntReg(Instruction * ins){
            return regAllocator->clearInt(ins);
        }
        auto clearFloatReg(Instruction * ins){
            return regAllocator->clearFloat(ins);
        }
//
//        auto clearAllIntReg(){
//            return regAllocator->clearAllIntReg();
//        }
//
//        void clearAllFloatReg(){
//            return regAllocator->clearAllFloatReg();
//        }

        auto clearAllReg(){
            return regAllocator->clearAllReg();
        }

        auto spillAllReg(){
            return regAllocator->spillAllReg();
        }

        auto clearReg(Instruction * ins){
            switch (ins->resultType()) {
                case ResultType::Integer:
                    return clearIntReg(ins);
                    break;
                case ResultType::Double:
                    return clearFloatReg(ins);
                    break;
                case ResultType::Void:
                    break;
            }
        }


        tiny::t86::ProgramBuilder pb_;
        Label lastIns_;
        std::unordered_map<tiny::Symbol, Label> functionTable_;
        std::unordered_map<Instruction*, uint64_t> globalTable_;
        std::unordered_map<Instruction *, Label> compiled_;
        std::unordered_map<BasicBlock *, Label> compiledBB_;
        std::vector<std::pair<Label, BasicBlock*>> future_patch_;
        std::vector<std::pair<Label, Symbol>> unpatchedCalls_;
        size_t functionLocalAllocSize = 0;
        size_t globalPointer_ = 0;


        std::unordered_map<const Instruction* ,Instruction * > instructionToEmplace;
        std::unique_ptr<RegisterAllocator> regAllocator;


        void makeGlobalTable(BasicBlock *pBlock);
        Label compileGlobalTable(BasicBlock *pBlock);

        uint64_t functionAddr(const std::string &)const;

        void addFunction(Symbol symbol, Label label);
    };
}
