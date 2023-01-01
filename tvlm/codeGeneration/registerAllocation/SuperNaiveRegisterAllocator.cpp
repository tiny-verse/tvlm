#include "SuperNaiveRegisterAllocator.h"

#include "tvlm/tvlm/codeGeneration/FunctionalMacro.h"

namespace tvlm {



    SuperNaiveRegisterAllocator::SuperNaiveRegisterAllocator(TargetProgram && tp):
            RegisterAllocator(std::move(tp))
    {
        size_t regSize = tiny::t86::Cpu::Config::instance().registerCnt();
        for(size_t i = 1 ; i < regSize ; i++){
            freeReg_.emplace(VirtualRegisterPlaceholder(RegisterType::INTEGER, i));
        }
        size_t fregSize = tiny::t86::Cpu::Config::instance().floatRegisterCnt();
        for(size_t i = 1; i < fregSize ; i++){
            freeFReg_.emplace(VirtualRegisterPlaceholder(RegisterType::FLOAT, i));
        }


    }
//
//    SuperNaiveRegisterAllocator::VirtualRegister
//    SuperNaiveRegisterAllocator::getRegister(SuperNaiveRegisterAllocator::VirtualRegister &reg,const Instruction * ins,  const Instruction * currentIns) {
//        auto it = addressDescriptor_.find(ins); //auto it = regMapping_.find(&reg);
//        if(it != addressDescriptor_.end() /*regMapping_.end()*/){
//            switch (it->second.begin()->loc()){
//                case Location::Register:
//                    return it->second.begin()->regIndex();
//
//                    break;
//                case Location::Stack:
//                case Location::Memory:
//
//                    auto free = getReg(currentIns);
//                    restore( free, * it->second.begin(), ins);
////                    addressDescriptor_.erase(it); //regMapping_.erase(it);
//                    addressDescriptor_[ins].emplace( LocationEntry( free, ins) );   //regMapping_.emplace( &reg,LocationEntry( free.index(), ins) );
//                    registerDescriptor_[free].emplace(ins);
//                    return free;
//                    break;
//            }
//        }else{
//            //notFound --> Allocate new
//            VirtualRegister newReg = getReg(currentIns);
//            addressDescriptor_[ins].emplace( LocationEntry( newReg, ins)); //regMapping_.emplace(&reg, LocationEntry( newReg.index(), ins));
//            return newReg;
//        }
//        throw "error this shouldn't happen 123";
//    }

    SuperNaiveRegisterAllocator::VirtualRegister SuperNaiveRegisterAllocator::getRegToSpill() {
        VirtualRegister res = regQueue_.front();
        regQueue_.erase(regQueue_.begin());
        return res;
    }


    SuperNaiveRegisterAllocator::VirtualRegister SuperNaiveRegisterAllocator::getReg( const Instruction * currentIns) {
        VirtualRegister res = VirtualRegister(RegisterType::INTEGER, 0);
        if(freeReg_.size() > 1){
            res = freeReg_.front();
            freeReg_.pop();

            regQueue_.push_back(res);//TODO
        }else {
            VirtualRegisterPlaceholder ress = getRegToSpill();
            spill( ress, currentIns);
            res = ress;
            regQueue_.push_back(res);
        }
        return res;
    }

    SuperNaiveRegisterAllocator::VirtualRegister SuperNaiveRegisterAllocator::getFReg( const Instruction * currentIns) {
        VirtualRegister res = VirtualRegister(RegisterType::FLOAT, 0);
        if(freeFReg_.size() > 0){
            res = freeFReg_.front();
            freeFReg_.pop();

            regQueue_.push_back(res);
        }else {
            VirtualRegisterPlaceholder ress = getRegToSpill();
            spill( ress, currentIns);
            res = ress;
            regQueue_.push_back(res);
        }
        return res;
    }

    SuperNaiveRegisterAllocator::VirtualRegister SuperNaiveRegisterAllocator::getLastRegister(const Instruction * currentIns) {
        VirtualRegister res = VirtualRegister(RegisterType::INTEGER, 0);
        if(!freeReg_.empty()){
            res = freeReg_.front();
            freeReg_.pop();
        }else {
            throw "no free register";
        }
        return res;
    }

    void SuperNaiveRegisterAllocator::releaseRegister(const VirtualRegister & reg) {
        freeReg_.push(reg);
        auto it = std::find(regQueue_.begin(), regQueue_.end(),reg);
        if(it != regQueue_.end()){
            regQueue_.erase(it);
        }
    }


    void SuperNaiveRegisterAllocator::removeFromRegisterDescriptor(const Instruction *ins) {
        throw "not Implemented [RA] removeFromRegisterDescriptor fnc ";
        auto address = addressDescriptor_.find(ins);
        if(address != addressDescriptor_.end()){
//            if (address->second.loc() == Location::Register){
//                registerDescriptor_[address->second.regIndex()].erase(ins);
//            }
        }

    }
    void SuperNaiveRegisterAllocator::registerMemLocation(const Store *ins, const Instruction *currentIns) {

        ins->address(); //TODO
        auto it = getAllocMapping(targetProgram_).find(ins->address());
        if(it != getAllocMapping(targetProgram_).end()){

            addressDescriptor_[ins->value()].emplace( LocationEntry( ins->address(), ins->value(), it->second));
        }


    }


}