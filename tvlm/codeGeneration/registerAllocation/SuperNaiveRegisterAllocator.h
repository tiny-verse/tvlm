#pragma once
#include "t86/program/helpers.h"
#include "tvlm/codeGeneration/ProgramBuilder.h"
#include "RegisterAllocator.h"

#include <queue>


namespace tvlm{

    class SuperNaiveRegisterAllocator :public RegisterAllocator{
        
    public:
        virtual ~SuperNaiveRegisterAllocator() = default;
        SuperNaiveRegisterAllocator( TargetProgram && tp);
        SuperNaiveRegisterAllocator( SuperNaiveRegisterAllocator && all) = default;


    protected:
        void visit( Label & label){ // helper for run()
//            if(auto instr = pb_.instructions_.at(label).first){
//                if(auto jump = dynamic_cast<tiny::t86::JMP * >(instr)){
////                    bbsToCompile_.push(jump->d);
//                }else if (auto condJump = dynamic_cast<tiny::t86::ConditionalJumpInstruction * >(instr)){
//                }
//            }
        }

        std::queue<Label> functions_;
        std::queue<Label> bbsToCompile_;

        VirtualRegister getRegToSpill();

        //:/ INTERFACE
        void registerMemLocation( const Store * ins, const Instruction * currentIns);

        VirtualRegister getReg(const Instruction * currentIns) override;
        VirtualRegister getFReg(const Instruction * currentIns) override;
//        VirtualRegister getFreeFRegister(const Instruction * currentIns);

        // :/ ---------------------


    protected:
        void releaseRegister(const VirtualRegister & reg);
        void eraseFreeReg(VirtualRegisterPlaceholder & reg);

        VirtualRegister getLastRegister(const Instruction *currentIns) override;

    protected:
void removeFromRegisterDescriptor(const Instruction * ins);


        size_t _regOffset__ = 1;
        std::list<VirtualRegister>regQueue_;

//        TargetProgramBuilder pb_;
        std::list<VirtualRegister> freeReg_;
        std::list<VirtualRegister> freeFReg_;
//        std::map<VirtualRegister*, LocationEntry> regMapping_;
//        std::map<VirtualRegister*, LocationEntry> spillMapping_;

//        void setupRegister(VirtualRegisterPlaceholder &reg, const Instruction *ins, const Instruction *currentIns);

    };










}
