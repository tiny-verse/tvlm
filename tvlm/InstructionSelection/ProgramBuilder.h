#pragma once

#include "tvlm/il/il.h"
#include "t86/program.h"
#include "t86/instruction.h"
#include "t86/program/label.h"

#include <cassert>
#include <vector>

namespace tvlm {

    using TInstruction =tiny::t86::Instruction;
    using ILInstruction =::tvlm::Instruction;
    using Label = tiny::t86::Label;
    using DataLabel = tiny::t86::DataLabel;
    using TProgram = tiny::t86::Program;


    class ProgramBuilder {
    public:
        ProgramBuilder(bool release = false) : release_(release) {}

//        ProgramBuilder(tiny::t86::Program program, bool release = false)
//                : instructions_(program.moveInstructions()),
//                  data_(program.data()),
//                  release_(release) {}

        template<typename T>
        Label add(const T& instruction, const ILInstruction * ilIns) {
            instruction.validate();
//            instructions_.emplace_back(std::make_pair<TInstruction*, const ILInstruction*>(new T(instruction), ilIns ));
            instructions_.emplace_back(new T(instruction), ilIns );
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

            return TProgram(tmp, std::move(data_));
        }

        template<typename T>
        void replace(Label label, const T & instruction) {
            delete instructions_[label].first;
            instructions_[label].first = new T(instruction);
        }

    private:
        std::vector<std::pair<TInstruction*, const ILInstruction*>> instructions_;

        std::vector<int64_t> data_;

        bool release_;
    };
}
