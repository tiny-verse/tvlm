#pragma once
#include "tvlm/tvlm/il/il.h"
#include "t86/program/label.h"
#include "tvlm/tvlm/codeGeneration/ProgramBuilder.h"
#include "tvlm/tvlm/codeGeneration/registerAllocation/RegisterAllocator.h"
#include "tvlm/tvlm/codeGeneration/registerAssigner/RegisterAssigner.h"

namespace tvlm{


//
//    class NaiveIS  : public ILVisitor{
//    public:
//
//        using Label = tiny::t86::Label;
//        using DataLabel = tiny::t86::DataLabel;
//        using Register = tiny::t86::Register;
//        using FRegister = tiny::t86::FloatRegister;
//
//
//        static tiny::t86::Program translate(ILBuilder &ilb);
//
//        void visit(Program *p) override;
//
//        ~NaiveIS() override ;
//    protected:
//        NaiveIS();
//        void visit(Instruction *ins) override{};
//
//        void visit(ILBuilder & ilb);
//        void visit(Jump *ins) override;
//        void visit(CondJump *ins) override;
//        void visit(Return *ins) override;
//        void visit(CallStatic *ins) override;
//        void visit(Call *ins) override;
//        void visit(Copy *ins) override;
//        void visit(Extend *ins) override;
//        void visit(Truncate *ins) override;
//        void visit(BinOp *ins) override;
//        void visit(UnOp *ins) override;
//        void visit(LoadImm *ins) override;
//        void visit(AllocL *ins) override;
//        void visit(AllocG *ins) override;
//        void visit(ArgAddr *ins) override;
//        void visit(PutChar *ins) override;
//        void visit(GetChar *ins) override;
//        void visit(Load *ins) override;
//        void visit(Store *ins) override;
//        void visit(Phi *ins) override;
//        void visit(ElemAddrOffset *ins) override;
//        void visit(ElemAddrIndex *ins) override;
//        void visit(Halt *ins) override;
//        void visit(StructAssign *ins) override;
//
//        void visit(BasicBlock *bb) override;
//        void visit(Function *fce) override;
//
//    private:
//        Label visitChild(IL * il) {
//            ILVisitor::visitChild(il);
//            return lastIns_;
//        }
//
//        template<typename T>
//        Label visitChild(std::unique_ptr<T> const &ptr) {
//            return visitChild(ptr.get());
//        }
//
////        template<typename T>
////        Label add(Instruction * ins, const T& instruction) {
////            lastIns_ = pb_.add( instruction);
////            if(ins != nullptr ) compiled_.emplace(ins, lastIns_);
////            return lastIns_;
////        }
//
////        template<typename T>
////        Label add( const T& instruction) {
////            lastIns_ = pb_.add( instruction);
////            return lastIns_;
////        }
//
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
////            lastInstruction_index = ret.address();
//            return ret;
//        }
//
//        Label find(Instruction * ins){
//            auto it = compiled_.find(ins);
//            if(it == compiled_.end()){
//                return Label::empty();
//            }
//            return it->second;
//        }
//
//
//
////        FRegister fillTmpFloatRegister(){
////            return getFreeFloatRegister();
////            return regAllocator->getFReg();
////        }
////        auto clearTmpIntRegister(const Register & reg ){
////            return regAllocator->clearIntRegister(reg);
////        }
////        auto clearIntReg(Instruction * ins){
////            return regAllocator->clearInt(ins);
////        }
////        auto clearFloatReg(Instruction * ins){
////            return regAllocator->clearFloat(ins);
////        }
//
//
////        auto clearReg(Instruction * ins){
////            switch (ins->resultType()) {
////                case ResultType::StructAddress:
////                case ResultType::Integer:
////                    return clearIntReg(ins);
////                    break;
////                case ResultType::Double:
////                    return clearFloatReg(ins);
////                    break;
////                case ResultType::Void:
////                    break;
////            }
////        }
//
//        void copyStruct(const Register & from, Type * type, const Register & to, const ILInstruction * ins );
////        int getTrueMemSize(Type * type) const {
////
////
////        }
//
//
//        tvlm::ProgramBuilderOLD pb_;
//           /*
//        bool hardDBG_ = true;
//         /*/
//        bool hardDBG_ = false;/**/
//
//
//        Label lastIns_;
//        std::unordered_map<tiny::Symbol, Label> functionTable_;
//        std::unordered_map<const Instruction*, uint64_t> globalTable_;
//        std::unordered_map<const Instruction *, Label> compiled_;
//        std::unordered_map<const BasicBlock *, Label> compiledBB_;
//        std::vector<std::pair<Label, const BasicBlock*>> future_patch_;
//        std::vector<std::pair<Label, Symbol>> unpatchedCalls_;
////        size_t functionLocalAllocSize = 0;
//        size_t globalPointer_ = 0;
//
//
//        std::unordered_map<const Instruction* ,const Instruction * > instructionToEmplace;
////        std::unique_ptr<RegisterAllocator> regAllocator;
//        std::unique_ptr<RegisterAssigner> regAssigner;
//
//        // ******************** Virtual Registers ******************************
//        std::map<const Instruction *, Register> assignedIntRegisters_;
//        std::map<const Instruction *, FRegister> assignedFloatRegisters_;
//        size_t regIntCounter_;
//        size_t regFloatCounter_;
//
//
//        Register getReg(const Instruction * ins){
//            return regAssigner->getReg(ins);
//        }
//        FRegister getFReg(const Instruction * ins){
//            return regAssigner->getFReg(ins);
//        }
//        Register getFreeIntRegister() {
//            return regAssigner->getFreeIntRegister();
//        }
//        FRegister getFreeFloatRegister() {
//            return regAssigner->getFreeFloatRegister();
//        }
//
//        void replaceIntReg(const Instruction * ins, const Instruction * with){
//            regAssigner->replaceIntReg(ins, with);
//        }
//        void replaceFloatReg(const Instruction * ins, const Instruction * with){
//            regAssigner->replaceFloatReg(ins, with);
//        }
//
//        void allocateStructArg(const Type * type, const Instruction * ins){
//            regAssigner->allocateStructArg(type, ins);
//        }
//        void prepareReturnValue(size_t size, const Instruction * ins){
//            regAssigner->prepareReturnValue(size, ins);
//        }
//
//        void makeLocalAllocation(size_t size, const Register &reg, const Instruction * ins){
//            regAssigner->makeLocalAllocation(size, reg, ins);
//        }
//
//        void registerPhi(const Phi * phi){
//            regAssigner->registerPhi(phi);
//        }
//        void correctStackAlloc(size_t patch) {
//            regAssigner->correctStackAlloc(patch);
//        }
//        void resetAllocSize(){
//            regAssigner->resetAllocSize();
//        }
//        // ******************** \\ Virtual Registers ******************************
//
//
//        void makeGlobalTable(BasicBlock *pBlock);
//        Label compileGlobalTable(BasicBlock *pBlock);
//
//        uint64_t functionAddr(const std::string &)const;
//
//        void addFunction(Symbol symbol, Label label);
//
//        size_t countArgOffset(std::vector<const Instruction *> & args, size_t index);
//
//        template<typename T>
//        void replace(Label label, const T& sub);
//
//    };
}
