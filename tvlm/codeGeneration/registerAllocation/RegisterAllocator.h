#pragma once

#include "t86/program/helpers.h"
#include "tvlm/codeGeneration/ProgramBuilder.h"

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

        LocationEntry(const LocationEntry & other) = default;
        LocationEntry( LocationEntry && other) = default;
        LocationEntry& operator=(const  LocationEntry & other) = default;


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


    class RegisterAllocator : public ILVisitor, public TargetProgramFriend{
    protected:
        using Register = tiny::t86::Register;
        using FRegister = tiny::t86::FloatRegister;
        using VirtualRegister = VirtualRegisterPlaceholder;

    public:
        virtual ~RegisterAllocator() = default;
        RegisterAllocator( TargetProgram  && tp);
        RegisterAllocator( RegisterAllocator  && rA) = default;
        virtual bool changedProgram()const = 0;

        virtual TargetProgram run(){
            //implement logic of passing through the program;
            visit(getProgram(&targetProgram_).get());
            return std::move(targetProgram_);
        }

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

        //Inner Helpers
        void updateStructures(const VirtualRegisterPlaceholder & regToAssign, const Instruction * ins);
        bool spill(const VirtualRegister & reg, Instruction * currentIns);
        void spillAll( Instruction * currentIns);
        void forgetAllRegs(Instruction * currentIns);
        void restore(const VirtualRegister & whereTo, const LocationEntry & from, const Instruction * currentIns);
        virtual void resetFreeRegs(const Instruction * except = nullptr);


        virtual VirtualRegister getReg( Instruction * ins, Instruction * currentIns) = 0;
        virtual VirtualRegister getFReg( Instruction * ins, Instruction * currentIns) = 0;

        virtual VirtualRegister getLastRegister( Instruction * currentIns) = 0;
        virtual void releaseRegister(const VirtualRegister & reg) = 0;
        void unsubscribeRegister(const VirtualRegister & reg);
        void allocAddrRegwritePos(const Instruction * ins){
            if(auto allocl = dynamic_cast<const AllocL*>(ins)){
//                res = regAssigner_->getReg(ins, currentIns);
//                int64_t baseOffset = regAssigner_->getAllocOffset(ins);
//                addF(LMBS tiny::t86::MOV(vR(res), tiny::t86::Bp()) LMBE, currentIns);
                writingPos_++;
//                addF(LMBS tiny::t86::SUB(vR(res), baseOffset) LMBE, currentIns);
                writingPos_++;
            }else if (auto allocg =dynamic_cast<const AllocG*>(ins)){
//                res = regAssigner_->getReg(ins, currentIns);
//                int64_t baseOffset = regAssigner_->getAllocOffset(ins);
//                addF(LMBS tiny::t86::MOV(vR(res),baseOffset) LMBE, currentIns);
                writingPos_++;
            }else{
//                return  regAssigner_->getReg(ins,currentIns);

            }
        }

       void updateJumpPatchPositions(const Instruction *ins);
        void updateCallPatchPositions(const Instruction *ins);
        void setCallPatchPositions(const Instruction *ins);

        virtual void callingConvCallerSave( Instruction *ins);
        void callingConvCalleeRestore( Instruction *ins);

        //Inner Interface
        void setupRegister( VirtualRegisterPlaceholder & reg, Instruction * ins, Instruction * currentIns);
        void setupFRegister(VirtualRegisterPlaceholder & reg, Instruction * ins, Instruction * currentIns);
        void replaceInRegister(const Instruction * replace, const Instruction * with);


        Function * currenFunction_;
        TargetProgram targetProgram_;

        std::map<const Instruction *, std::set<LocationEntry>> addressDescriptor_;
        std::map<VirtualRegister, std::set<const Instruction*>> registerDescriptor_;
        size_t writingPos_; //position for insertion of code (spill or load code)

        //:/Helpers
        std::set<LocationEntry>::iterator findLocation(std::set<LocationEntry> & set1, Location location);
        std::vector<VirtualRegisterPlaceholder> * getAllocatedVirtualRegisters(const Instruction * ins);
        std::string generateInstrName(VirtualRegister & reg, bool global = false){
            std::string prefix;
            if(global){
                prefix = "g";
            }else{
                prefix = "r";
            }
            if(reg.getType() == RegisterType::FLOAT){
                prefix += "d";
            }else if (reg.getType() == RegisterType::INTEGER){
                prefix += "i";
            }
            prefix += std::to_string(reg.getNumber());
            return prefix;
        }
    };

}

