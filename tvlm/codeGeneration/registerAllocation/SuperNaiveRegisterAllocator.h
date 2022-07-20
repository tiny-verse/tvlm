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



    class SuperNaiveRegisterAllocator {
        using Register = tiny::t86::Register;
        using VirtualRegister = tiny::t86::Register;
        using TargetProgramBuilder = tvlm::ProgramBuilder;

    public:
        virtual ~SuperNaiveRegisterAllocator() = default;
        SuperNaiveRegisterAllocator(TargetProgramBuilder && pb):
                currentWorkingReg_(-1),
                pb_(std::move(pb))
        {
            size_t regSize = tiny::t86::Cpu::Config::instance().registerCnt();
            for(size_t i = 0 ; i < regSize ; i++){
                freeReg_.push_back(Register(i));
            }


        }


        TargetProgramBuilder run(){
            //implement logic of passing through the program;


        }


    private:
        void visit( Label & label){ // helper for run()
            if(auto instr = pb_.instructions_.at(label).first){
                if(auto jump = dynamic_cast<tiny::t86::JMP * >(instr)){
//                    bbsToCompile_.push(jump->d);

                }else if (auto condJump = dynamic_cast<tiny::t86::ConditionalJumpInstruction * >(instr)){
                }







            }
        }

        std::queue<Label> functions_;
        std::queue<Label> bbsToCompile_;

        Register getRegister(const VirtualRegister & reg){
            currentWorkingReg_ = reg;
            auto it = regMapping_.find(reg);
            if(it != regMapping_.end()){
                switch (it->second.loc()){
                    case Location::Register:
                        return Register(it->second.regIndex());

                        break;
                    case Location::Stack:
                    case Location::Memory:

                        auto free = getFreeRegister();
                        restore( free, it->second);
                        regMapping_.erase(it);
                        regMapping_.emplace(reg, LocationEntry(Location::Register, free.index()));
                        return free;
                        break;
                }
            }else{
                //notFound --> Allocate new
                Register newReg = getFreeRegister();
                regMapping_.emplace(reg, LocationEntry(Location::Register, newReg.index()));
                return newReg;
            }
        }


        Register getRegToSpill(){
            Register res = regQueue_.front();
            regQueue_.pop();
            return res;
        }

        bool spill(const VirtualRegister & reg) {
            int stackOffset;
            VirtualRegister virtualRegister = reg;
            auto accomodategReg = regMapping_.find(reg);
            if(accomodategReg == regMapping_.end() && accomodategReg->second.loc() != Location::Register){
                return false;
            }
            Register regToSPill = Register(accomodategReg->second.regIndex());
            auto it = spillMapping_.find(reg);
            if(it != spillMapping_.end()){
                switch (it->second.loc()){
                case Location::Register:{

                    throw "spilling from register to register not implemented";
                    break;
                }
                case Location::Stack:{

                    int stackOffset = it->second.stackOffset();
                    tiny::t86::MOV(tiny::t86::Mem(tiny::t86::Sp() + stackOffset), reg); // TODO insert code

                    //update structures

                    regMapping_.erase(virtualRegister);
                    spillMapping_.emplace(virtualRegister, LocationEntry(Location::Stack, stackOffset));

                    break;
                }
                case Location::Memory:{

                    int memAddress = it->second.memAddress();
                    tiny::t86::MOV(tiny::t86::Mem(memAddress), reg); // TODO insert code


                    //update structures

                    regMapping_.erase(virtualRegister);
                    spillMapping_.emplace(virtualRegister, LocationEntry(Location::Memory, memAddress));
                        throw "spilling to memory not implemented";
                    break;
                }
                }
            }else{
                if (0 /*determine if it has its address == variable, or it is tmp*/){
                    //variable
//                    tiny::t86::MOV(tiny::t86::Mem(address), reg); // TODO insert code
                }else{
                    //tmp
                    //stackOffset = getNewStackOffset() //allocate next tmp local
                    tiny::t86::MOV(tiny::t86::Mem(tiny::t86::Sp() + stackOffset), reg); // TODO insert code
                }

            }


            //update structures

            regMapping_.erase(virtualRegister);
            spillMapping_.emplace(virtualRegister, LocationEntry(Location::Stack, stackOffset));

            return true;
        }

        void restore(const VirtualRegister & whereTo, const LocationEntry & from){

        }

        Register getFreeRegister(){
            Register res = Register(0);
            if(freeReg_.empty()){
                res = freeReg_.front();
                freeReg_.erase(freeReg_.begin());
            }else {
                res = getRegToSpill();
                spill(res);
            }
                regQueue_.push(res);
                return res;
        }

        std::queue<Register>regQueue_;
        VirtualRegister currentWorkingReg_;
        TargetProgramBuilder pb_;
        std::list<Register> freeReg_;
        std::map<VirtualRegister, LocationEntry> regMapping_;
        std::map<VirtualRegister, LocationEntry> spillMapping_;
    };










}
