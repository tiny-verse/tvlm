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
        LocationEntry( size_t stackPos, const Instruction * ins)
        :
        loc_(Location::Stack),
        num_(stackPos),
        register_(VirtualRegisterPlaceholder(RegisterType::FLOAT, 0)),
        ins_(ins){}


        LocationEntry(const VirtualRegisterPlaceholder& reg, const Instruction * ins) : loc_(Location::Register),
        num_(0), addr_(nullptr),
        register_(reg), ins_(ins){

        }

        LocationEntry(const Instruction * addr, const Instruction * ins, int64_t memAddress) : loc_(Location::Memory),
        num_(memAddress), addr_(addr),
        register_( VirtualRegisterPlaceholder(RegisterType::FLOAT, -1)), ins_(ins){

        }
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
        VirtualRegisterPlaceholder regIndex() const {
            if(loc_ == Location::Register){
                return register_;
            }else{
                throw "[Location Entry] called for regIndex with no reg info";
            }
        };
        size_t memAddress()const{
            if (loc_ == Location::Memory){
                return num_;
            }else{
                return INT32_MIN;
            }
        }
        const Instruction* memAddressInstruction()const{
            if (loc_ == Location::Memory){
                return addr_;
            }else{
                return nullptr;
            }
        }
        bool operator<(const LocationEntry & other) const {
            return loc_ < other.loc_ ||  (loc_ == other.loc_ && num_ < other.num_);
        }
        const Instruction * ins() const{
            return ins_;
        }
    private:
        Location loc_;
        size_t num_;
        VirtualRegisterPlaceholder register_;
        const Instruction * addr_;
        const Instruction * ins_;
    };



    class SuperNaiveRegisterAllocator :public ILVisitor{
        using Register = tiny::t86::Register;
        using FRegister = tiny::t86::FloatRegister;
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

        VirtualRegister getRegister(VirtualRegister & reg, const Instruction * ins, const Instruction * currentIns);

        VirtualRegister getRegToSpill();

        //:/ INTERFACE
        bool spill(const VirtualRegister & reg, const Instruction * currentIns);
        void spillAll(const Instruction * currentIns);
        void registerMemLocation( const Store * ins, const Instruction * currentIns);

        void restore(const VirtualRegister & whereTo, const LocationEntry & from, const Instruction * currentIns);

        VirtualRegister getReg(const Instruction * currentIns);
        VirtualRegister getFreeFRegister(const Instruction * currentIns);

        // :/ ---------------------


        std::vector<VirtualRegisterPlaceholder> * getAllocatedVirtualRegisters(const Instruction * ins){
//            return targetProgram_.alocatedRegisters_[ins];
            auto it = targetProgram_.alocatedRegisters_.find(ins);
            if ( it != targetProgram_.alocatedRegisters_.end()){
                return  &(it->second);
            }else{
                    throw "trying to find allocated registers for ins that was not compiled";
                if(targetProgram_.selectedFInstrs_.find(ins) != targetProgram_.selectedFInstrs_.end()){
                    return  &(targetProgram_.alocatedRegisters_[ins]);
                }else{
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

        void setupRegister( VirtualRegisterPlaceholder & reg, const Instruction * ins, const Instruction * currentIns);
        void setupFRegister(VirtualRegisterPlaceholder & reg, const Instruction * ins, const Instruction * currentIns);
        void removeFromRegisterDescriptor(const Instruction * ins);
        void replaceInRegister(const Instruction * replace, const Instruction * with);

        VirtualRegister getLastRegister(const Instruction * currentIns);
        void releaseRegister(const VirtualRegister & reg);
        void updateJumpPatchPositions(const Instruction *ins);
        void updateCallPatchPositions(const Instruction *ins);


        std::list<VirtualRegister>regQueue_;

        VirtualRegister  _mtWorkingReg_;
        VirtualRegister * currentWorkingReg_;
        Function * currenFunction_;
//        TargetProgramBuilder pb_;
        TargetProgram & targetProgram_;
        std::queue<VirtualRegister> freeReg_;
        std::queue<VirtualRegister> freeFReg_;
//        std::map<VirtualRegister*, LocationEntry> regMapping_;
//        std::map<VirtualRegister*, LocationEntry> spillMapping_;
        std::map<const Instruction *, std::set<LocationEntry>> addressDescriptor_;
        std::map<VirtualRegister, std::set<const Instruction*>> registerDescriptor_;

        size_t writingPos_; //position for insertion of code (spill or load code)


//        void setupRegister(VirtualRegisterPlaceholder &reg, const Instruction *ins, const Instruction *currentIns);
        std::set<LocationEntry>::iterator findLocation(std::set<LocationEntry> & set1, Location location);

        void callingConvCallerSave(const Instruction *ins);
        void callingConvCalleeRestore(const Instruction *ins);
    };










}
