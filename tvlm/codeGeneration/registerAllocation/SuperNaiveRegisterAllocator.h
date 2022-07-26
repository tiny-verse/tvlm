#pragma once
#include "t86/program/helpers.h"
#include "tvlm/codeGeneration/ProgramBuilder.h"

#include <queue>


namespace tvlm{


    enum class Location{
        Register,
        Memory,
        Stack
    };

    class LocationEntry{
    public:
        LocationEntry(Location location, int number) : loc_(location), num_(number){}
        Location loc() const {
            return loc_;
        }
        int stackOffset() const {
            if(loc_ == Location::Stack){
                return num_;
            }else{
                return INT32_MAX;
            }
        }
        int regIndex() const {
            if(loc_ == Location::Register){
                return num_;
            }else{
                return INT32_MIN;
            }
        };
        int memAddress()const{
            if (loc_ == Location::Memory){
                return num_;
            }else{
                return INT32_MIN;
            }
        }
        bool operator<(const LocationEntry & other) const {
            return loc_ < other.loc_ ||  (loc_ == other.loc_ && num_ < other.num_);
        }
    private:
        Location loc_;
        size_t num_;
    };



    class SuperNaiveRegisterAllocator :public ILVisitor{
        using Register = tiny::t86::Register;
        using VirtualRegister = tiny::t86::Register;
        using TargetProgramBuilder = tvlm::ProgramBuilder;

    public:
        virtual ~SuperNaiveRegisterAllocator() = default;
        SuperNaiveRegisterAllocator( TargetProgram & tp);


        TargetProgram run(){
            //implement logic of passing through the program;
            return targetProgram_; // TODO
        }


    private:
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

        Register getRegister(const VirtualRegister & reg);

        Register getRegToSpill();

        bool spill(const VirtualRegister & reg);
        void restore(const VirtualRegister & whereTo, const LocationEntry & from);

        Register getFreeRegister();
    protected:
        std::vector<tiny::t86::Instruction *> getSelected(const Instruction * ins) const;

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
        void enterNewBB(BasicBlock * bb);
        void exitBB();
        void enterNewFce(BasicBlock * bb);
        void exitFce();
    private:
        std::queue<Register>regQueue_;
        VirtualRegister currentWorkingReg_;
//        TargetProgramBuilder pb_;
        TargetProgram targetProgram_;
        std::list<Register> freeReg_;
        std::map<VirtualRegister, LocationEntry> regMapping_;
        std::map<VirtualRegister, LocationEntry> spillMapping_;
    };










}
