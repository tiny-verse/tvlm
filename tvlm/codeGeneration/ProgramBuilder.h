#pragma once

#include "tvlm/il/il.h"
#include "t86/program.h"
#include "t86/instruction.h"
#include "t86/program/label.h"
#include "tvlm/tvlm/backendUtility/VirtualRegister.h"
#include "tvlm/tvlm/backendUtility/TargetInstrEmul.h"


#include <cassert>
#include <vector>

namespace tvlm {

    using TInstruction =tiny::t86::Instruction;
    using TFInstruction =std::function<TInstruction * (std::vector<VirtualRegisterPlaceholder>&)>;
    using ILInstruction =::tvlm::Instruction;
    using Label = tiny::t86::Label;
    using DataLabel = tiny::t86::DataLabel;
    using TProgram = tiny::t86::Program;

//class VirtualReg {
//    public:
//        virtual ~VirtualReg() = default;
//        VirtualReg(size_t number):number_(number){
//
//        }
//        VirtualReg(const VirtualReg & other):
//        number_(other.number_){
//
//        }
//        void changeNumber(size_t newNumber){
//            number_ = newNumber;
//        }
//    private:
//        size_t number_;
//    };

    class TargetProgram{
    public:
        using Register = tiny::t86::Register;
        using FRegister = tiny::t86::FloatRegister;
        friend class RegisterAssigner;
        friend class Epilogue;
        friend class NaiveEpilogue;
//        friend class RegisterAllocator;
        friend class SuperNaiveRegisterAllocator;
        virtual ~TargetProgram() = default;
        TargetProgram():
        program_(nullptr)
        , funcLocalAlloc_()
        , selectedInstrs_()
        , selectedFInstrs_()
//        , assignedIntRegisters_()
//        , assignedFloatRegisters_()
        ,alocatedRegisters_()
        ,jump_patches_()
        ,call_patches_()
        ,globalTable_()
        ,data_(){

        }
        TargetProgram(const TargetProgram & prog):
        program_(prog.program_)
        , funcLocalAlloc_(prog.funcLocalAlloc_)
        , selectedInstrs_(prog.selectedInstrs_)
        , selectedFInstrs_(prog.selectedFInstrs_)
//        , assignedIntRegisters_(prog.assignedIntRegisters_)
//        , assignedFloatRegisters_(prog.assignedFloatRegisters_)
        ,alocatedRegisters_(prog.alocatedRegisters_)
        ,alocatedTMPRegisters_(prog.alocatedTMPRegisters_)
        ,jump_patches_(prog.jump_patches_)
        ,call_patches_(prog.call_patches_)
        ,globalTable_(prog.globalTable_)
        ,data_(prog.data_)
        {

        }

        void setProgram(Program * program){
            program_ = program;
        }



        Label addF(const TFInstruction & instruction, const ILInstruction * ins){
//            selectedInstrs_[ins].push_back(new T(instruction));
            selectedFInstrs_[ins].push_back(instruction);
            return selectedFInstrs_[ins].size() -1;
        }
//        template<typename T>
//        Label add(const T& instruction, const ILInstruction * ins){
//            selectedInstrs_[ins].push_back(new T(instruction));
//            return selectedInstrs_[ins].size() -1;
//        }


        DataLabel addData(int64_t data) {
            data_.push_back(data);
            return data_.size() - 1;
        }

        // Add data of specified size to data segment with specified value(s)
        DataLabel addData(int64_t value, std::size_t size) {
            DataLabel label = data_.size();
            data_.reserve(size);
            for (std::size_t i = 0; i < size; ++i) {
                data_.push_back(value);
            }
            return label;
        }

        DataLabel addData(const std::string& str) {
            DataLabel label = data_.size();
            for (char c : str) {
                addData(c);
            }
            addData('\0');
            return label;
        }

        void patchJump(const Instruction * ins, const Label & label, const BasicBlock * dest){
            jump_patches_.emplace_back(std::make_pair(ins,label), dest);
        }

        void patchCall(const Instruction * ins, const Label & label, Symbol & dest){
            call_patches_.emplace_back(std::make_pair(ins, label), dest);
        }

        auto globalFind(const Instruction * ins)const{
            return globalTable_.find(ins);
        }
        auto globalEnd() const{
            return globalTable_.end();
        }

        void globalEmplace(const Instruction * ins, uint64_t val ){
            globalTable_.emplace(std::make_pair(ins, val));
        }

        size_t registerAdd(const ILInstruction * instr, VirtualRegisterPlaceholder && virt){
            auto it = alocatedRegisters_.find(instr);
            if(it == alocatedRegisters_.end()){
                std::vector<VirtualRegisterPlaceholder>tmp;
                tmp.emplace_back(std::move(virt));
                alocatedRegisters_.emplace(instr, std::move(tmp));
                return 0;
            }else{
                auto & regs = it->second;
                regs.emplace_back(std::move(virt));
                return  regs.size() -1;
            }
        }
//        size_t registerTMPAdd( const VirtualRegisterPlaceholder & virt){
//            alocatedTMPRegisters_.push_back(virt);
//            return alocatedTMPRegisters_.size() - 1;
//        }

        void registerCall(const Instruction * call, Label & label,const Symbol & fncName){
            unpatchedFCalls_.emplace_back(std::make_pair(call, label), fncName );
        }
    private:
        Program * program_;
        std::map<const Function * ,size_t> funcLocalAlloc_;
        std::map<const ILInstruction*, std::vector<TInstruction*>> selectedInstrs_;
        std::map<const ILInstruction*, std::vector<TFInstruction>> selectedFInstrs_;
//        std::map<const Instruction *, Register> assignedIntRegisters_;
//        std::map<const Instruction *, FRegister> assignedFloatRegisters_;

        std::map<const ILInstruction*, std::vector<VirtualRegisterPlaceholder>> alocatedRegisters_;
        std::vector<VirtualRegisterPlaceholder> alocatedTMPRegisters_;
        std::vector<std::pair<std::pair<const ILInstruction *, Label>, const BasicBlock*>> jump_patches_;
        std::vector<std::pair<std::pair<const ILInstruction *, Label>, Symbol>> call_patches_;
        std::map<const Instruction*, uint64_t> globalTable_;
//        std::vector<std::pair<const ILInstruction *, const BasicBlock*>> jumpPatches_;
        std::vector<std::pair<std::pair<const Instruction *, Label>, Symbol>> unpatchedFCalls_;

        std::vector<int64_t> data_;
    };



    class ProgramBuilder {
        friend class SuperNaiveRegisterAllocator;
    public:
        ProgramBuilder(bool release = false) : release_(release) {}

//        ProgramBuilder(tiny::t86::Program program, bool release = false)
//                : instructions_(program.moveInstructions()),
//                  data_(program.data()),
//                  release_(release) {}

        Label add(std::vector<tiny::t86::Instruction*>& instructions, const ILInstruction * ilIns) {
            Label tmp = instructions_.size();
            for (auto i : instructions) {
                i->validate();
                instructions_.emplace_back(i, ilIns );
            }
            return tmp;
        }
        template<typename T>
        Label add(const T& instruction, const ILInstruction * ilIns) {
            instruction.validate();
//            instructions_.emplace_back(std::make_pair<TInstruction*, const ILInstruction*>(new T(instruction), ilIns ));
            instructions_.emplace_back(new T(instruction), ilIns );
            return Label(instructions_.size() - 1);
        }

        Label add(tiny::t86::Instruction * instruction, const ILInstruction * ilIns) {
            instruction->validate();
//            instructions_.emplace_back(std::make_pair<TInstruction*, const ILInstruction*>(new T(instruction), ilIns ));
            instructions_.emplace_back(instruction, ilIns );
            return Label(instructions_.size() - 1);
        }

        Label add(const tiny::t86::DBG& instruction) {
            if (!release_) {
                instructions_.emplace_back(new tiny::t86::DBG(instruction), nullptr);
                return instructions_.size() - 1;
            }
            // This returns the next added instruction
            // if the DBG was last, you will get into a infinite NOP loop
            // but you would get there with DBG as last anyways
            return instructions_.size();
        }

        // This returns current Label, this label will point to next added instruction
        Label currentLabel() const {
            return instructions_.size();
        }

        DataLabel addData(int64_t data) {
            data_.push_back(data);
            return data_.size() - 1;
        }

        // Add data of specified size to data segment with specified value(s)
        DataLabel addData(int64_t value, std::size_t size) {
            DataLabel label = data_.size();
            data_.reserve(size);
            for (std::size_t i = 0; i < size; ++i) {
                data_.push_back(value);
            }
            return label;
        }

        DataLabel addData(const std::string& str) {
            DataLabel label = data_.size();
            for (char c : str) {
                addData(c);
            }
            addData('\0');
            return label;
        }

        void patch(Label instruction, Label destination) {
            TInstruction* ins = instructions_.at(instruction).first;
            auto* jmpInstruction = dynamic_cast<tiny::t86::PatchableJumpInstruction*>(ins);
            assert(jmpInstruction && "You can patch only jump instructions");
            jmpInstruction->setDestination(destination);
        }

        TProgram program() {
            std::vector<TInstruction *> tmp;
            for (auto & i: instructions_) {
                tmp.emplace_back(i.first);
            }
            instructions_.clear();

            return TProgram(std::move(tmp), std::move(data_));
        }

        template<typename T>
        void replace(Label label, const T & instruction) {
            delete instructions_[label].first;
            instructions_[label].first = new T(instruction);
        }

        void setProgram(Program * prog){
            program_ = prog;
        }

    private:
        std::vector<std::pair<TInstruction*, const ILInstruction*>> instructions_;

        std::vector<int64_t> data_;

        Program * program_;
        bool release_;
    };


    class ProgramBuilderOLD {
        friend class SuperNaiveRegisterAllocator;
    public:
        ProgramBuilderOLD(bool release = false) : release_(release) {}

//        ProgramBuilderOLD(tiny::t86::Program program, bool release = false)
//                : instructions_(program.moveInstructions()),
//                  data_(program.data()),
//                  release_(release) {}

        Label add(std::vector<tiny::t86::Instruction*>& instructions, const ILInstruction * ilIns) {
            Label tmp = instructions_.size();
            for (auto i : instructions) {
                i->validate();
                instructions_.emplace_back(i, ilIns );
            }
            return tmp;
        }
        template<typename T>
        Label add(const T& instruction, const ILInstruction * ilIns) {
            instruction.validate();
//            instructions_.emplace_back(std::make_pair<TInstruction*, const ILInstruction*>(new T(instruction), ilIns ));
            instructions_.emplace_back(new T(instruction), ilIns );
            return Label(instructions_.size() - 1);
        }

        Label add(tiny::t86::Instruction * instruction, const ILInstruction * ilIns) {
            instruction->validate();
//            instructions_.emplace_back(std::make_pair<TInstruction*, const ILInstruction*>(new T(instruction), ilIns ));
            instructions_.emplace_back(instruction, ilIns );
            return Label(instructions_.size() - 1);
        }

        Label add(const tiny::t86::DBG& instruction) {
            if (!release_) {
                instructions_.emplace_back(new tiny::t86::DBG(instruction), nullptr);
                return instructions_.size() - 1;
            }
            // This returns the next added instruction
            // if the DBG was last, you will get into a infinite NOP loop
            // but you would get there with DBG as last anyways
            return instructions_.size();
        }

        // This returns current Label, this label will point to next added instruction
        Label currentLabel() const {
            return instructions_.size();
        }

        DataLabel addData(int64_t data) {
            data_.push_back(data);
            return data_.size() - 1;
        }

        // Add data of specified size to data segment with specified value(s)
        DataLabel addData(int64_t value, std::size_t size) {
            DataLabel label = data_.size();
            data_.reserve(size);
            for (std::size_t i = 0; i < size; ++i) {
                data_.push_back(value);
            }
            return label;
        }

        DataLabel addData(const std::string& str) {
            DataLabel label = data_.size();
            for (char c : str) {
                addData(c);
            }
            addData('\0');
            return label;
        }

        void patch(Label instruction, Label destination) {
            TInstruction* ins = instructions_.at(instruction).first;
            auto* jmpInstruction = dynamic_cast<tiny::t86::PatchableJumpInstruction*>(ins);
            assert(jmpInstruction && "You can patch only jump instructions");
            jmpInstruction->setDestination(destination);
        }

        TProgram program() {
            std::vector<TInstruction *> tmp;
            for (auto & i: instructions_) {
                tmp.emplace_back(i.first);
            }
            instructions_.clear();

            return TProgram(std::move(tmp), std::move(data_));
        }

        template<typename T>
        void replace(Label label, const T & instruction) {
            delete instructions_[label].first;
            instructions_[label].first = new T(instruction);
        }

        void setProgram(Program * prog){
            program_ = prog;
        }

    private:
        std::vector<std::pair<TInstruction*, const ILInstruction*>> instructions_;

        std::vector<int64_t> data_;

        Program * program_;
        bool release_;
    };
}
