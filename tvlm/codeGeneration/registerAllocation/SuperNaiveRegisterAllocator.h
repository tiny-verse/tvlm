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
        using VirtualRegister = VirtualRegisterPlaceholder;
        using TargetProgramBuilder = tvlm::ProgramBuilderOLD;

    public:
        virtual ~SuperNaiveRegisterAllocator() = default;
        SuperNaiveRegisterAllocator( TargetProgram & tp);


        TargetProgram run(){
            //implement logic of passing through the program;
            visit(targetProgram_.program_);

            return targetProgram_; // TODO pick registers and fill selectedInstrs with correct registers
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

        Register getRegister(VirtualRegister & reg);

        Register getRegToSpill();

        bool spill(VirtualRegister & reg);
        void restore(const Register & whereTo, const LocationEntry & from);

        Register getFreeRegister();

        std::vector<VirtualRegisterPlaceholder> & getAllocatedVirtualRegisters(const Instruction * ins){
//            return targetProgram_.alocatedRegisters_[ins];
            auto it = targetProgram_.alocatedRegisters_.find(ins);
            if ( it != targetProgram_.alocatedRegisters_.end()){
                return it->second;
            }else{
                if(targetProgram_.selectedFInstrs_.find(ins) != targetProgram_.selectedFInstrs_.end()){
                    return targetProgram_.alocatedRegisters_[ins];
                }else{
                    throw "trying to fing allocated registers for ins that was not compiled";
                }
            }
        }

    protected:
        std::list<TFInstruction> getSelectedInstr(const Instruction * ins) const;
        std::vector<VirtualRegisterPlaceholder>& getSelectedRegisters(const Instruction * ins);

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
        void enterNewFce(Function * fce);
        void exitFce();
    private:

        void setupRegister(VirtualRegisterPlaceholder * reg, const Instruction * ins, size_t pos);
        void setupFRegister(VirtualRegisterPlaceholder * reg, const Instruction * ins, size_t pos);
        void removeFromRegisterDescriptor(const Instruction * ins);
        void replaceInRegisterDescriptor(const Instruction * replace, const Instruction * with);
        Register getReg(const Instruction * ins);
        std::queue<Register>regQueue_;

        VirtualRegister  _mtWorkingReg_;
        VirtualRegister * currentWorkingReg_;
//        TargetProgramBuilder pb_;
        TargetProgram & targetProgram_;
        std::list<Register> freeReg_;
        std::map<VirtualRegister*, LocationEntry> regMapping_;
        std::map<const Instruction *, LocationEntry> addressDescriptor_;
        std::map<Register, std::set<const Instruction*>> registerDescriptor_;
        std::map<VirtualRegister*, LocationEntry> spillMapping_;
    };










}
