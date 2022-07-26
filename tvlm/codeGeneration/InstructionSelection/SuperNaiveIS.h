#pragma once


#include "tvlm/tvlm/il/il.h"
#include "t86/program/label.h"
#include "tvlm/tvlm/codeGeneration/ProgramBuilder.h"
#include "tvlm/codeGeneration/registerAssigner/RegisterAssigner.h"
#include "t86/program/helpers.h"

namespace  tvlm{


    class SuperNaiveIS : public ILVisitor{

    using ILInstruction = Instruction;
    using TInstruction = tiny::t86::Instruction;
    using Label = tiny::t86::Label;
    using DataLabel = tiny::t86::DataLabel;
    using Register = tiny::t86::Register;
    using FRegister = tiny::t86::FloatRegister;
    private:

//        tvlm::ProgramBuilderOLD pb_;
        /*
     bool hardDBG_ = true;
      /*/
        bool hardDBG_ = false;/**/


//        Label lastIns_;
        TargetProgram program_;
//        std::unordered_map<tiny::Symbol, const Function * > functionTable_; // already in Program
//        std::unordered_map<const Instruction*, uint64_t> globalTable_;
//        std::vector<std::pair<Label, const BasicBlock*>> future_patch_;
        std::vector<std::pair<Label, Symbol>> unpatchedCalls_;

        std::unique_ptr<RegisterAssigner> regAssigner;
    public:
        ~SuperNaiveIS() override;
        SuperNaiveIS();
    static TargetProgram translate(ILBuilder &ilb) ;
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
//        Label visitChild(IL * il) {
//            ILVisitor::visitChild(il);
//            return lastIns_;
//        }


        Label visitChild(IL * il) {
            ILVisitor::visitChild(il);
        }

        template<typename T>
        Label visitChild(std::unique_ptr<T> const &ptr) {
            return visitChild(ptr.get());
        }


//        std::vector<std::pair<std::pair<const ILInstruction *, Label>, const BasicBlock*>> jump_patches_; // in program
//        std::map<const ILInstruction *, std::vector<TInstruction*>> translated_parts_; // in program


//        template<typename T>
//        Label add(const T& instruction, const ILInstruction * ins){
//            Label ret= pb_.add(instruction, ins);
//            if(hardDBG_){
//                pb_.add(tiny::t86::DBG(
//                        [](tiny::t86::Cpu & cpu){
//                            printAllRegisters(cpu,std::cerr);
//                            std::cin.get();
//                        }
//                ));
//            }
//    //            lastInstruction_index = ret.address();
//            return ret;
//        }

//        template<typename T>
//        Label add(const T& instruction, const ILInstruction * ins){
//            return program_.add(instruction, ins);
//        }

        Label addF(const TFInstruction & instruction, const ILInstruction * ins){
            return program_.addF(instruction, ins);
        }

        size_t getReg(const Instruction * ins){
            auto reg = regAssigner->getReg(ins);
            auto virt =  VirtualRegisterPlaceholder(RegisterType::INTEGER, reg.index());
            return program_.registerAdd(ins, std::move(virt));
        }
        size_t getExtraIntReg(const ILInstruction * ins){
            auto reg = regAssigner->getFreeIntRegister();
            auto virt =  VirtualRegisterPlaceholder(RegisterType::INTEGER, reg.index());
            return program_.registerAdd( ins, std::move(virt));
        }

        size_t getFReg(const Instruction * ins){
            auto reg = regAssigner->getFReg(ins);
            auto virt =  VirtualRegisterPlaceholder(RegisterType::FLOAT, reg.index());
            return program_.registerAdd(ins, std::move(virt));
        }
        void makeLocalAllocation(size_t size, size_t reg, const Instruction * ins){
            regAssigner->makeLocalAllocation(size, reg, ins);
        }
        void makeGlobalAllocation(size_t size, size_t reg, const Instruction * ins){
            regAssigner->makeGlobalAllocation(size, reg, ins);
        }
//        void copyStruct(const Register & from, Type * type, const Register & to, const ILInstruction * ins );
        void copyStruct(size_t from, Type * type,size_t to, const ILInstruction * ins );

        void makeGlobalTable(BasicBlock *globals);

        void compileGlobalTable(BasicBlock *globals);
    };
}
