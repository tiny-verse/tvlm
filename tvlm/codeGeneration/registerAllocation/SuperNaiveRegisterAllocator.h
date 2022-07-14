#pragma once
#include "t86/program/helpers.h"

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

    public:
        virtual ~SuperNaiveRegisterAllocator() = default;
        SuperNaiveRegisterAllocator(){}



    private:
        Register getRegister(const Register & reg){
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

        void spill(const Register & reg) {

        }

        void restore(const Register & whereTo, const LocationEntry & from){

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

        //TargetProgramBuilder pb_;
        std::list<Register> freeReg_;
        std::map<Register, LocationEntry> regMapping_;
        std::map<Register, LocationEntry> spillMapping_;
    };










}
