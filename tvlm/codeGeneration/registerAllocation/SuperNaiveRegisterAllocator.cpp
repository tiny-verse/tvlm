#include "SuperNaiveRegisterAllocator.h"

namespace tvlm {



    SuperNaiveRegisterAllocator::SuperNaiveRegisterAllocator(TargetProgram && tp):
            RegisterAllocator(std::move(tp))
    {
        size_t regSize = tiny::t86::Cpu::Config::instance().registerCnt();
        for(size_t i = _regOffset__ ; i < regSize ; i++){
            freeReg_.emplace( VirtualRegisterPlaceholder(RegisterType::INTEGER, i));
        }
        size_t fregSize = tiny::t86::Cpu::Config::instance().floatRegisterCnt();
        for(size_t i = _regOffset__; i < fregSize ; i++){
            freeFReg_.emplace( VirtualRegisterPlaceholder(RegisterType::FLOAT, i));
        }


    }

    SuperNaiveRegisterAllocator::VirtualRegister SuperNaiveRegisterAllocator::getRegToSpill() {
        VirtualRegister res = regQueue_.front();
        regQueue_.erase(regQueue_.begin());
        return res;
    }


    SuperNaiveRegisterAllocator::VirtualRegister SuperNaiveRegisterAllocator::getReg( Instruction * currentIns) {
        VirtualRegister res = VirtualRegister(RegisterType::INTEGER, 0);
        if(freeReg_.size() > 1){
            res = *freeReg_.begin();
            freeReg_.erase(freeReg_.begin());

            regQueue_.push_back(res);
        }else {
            VirtualRegisterPlaceholder ress = getRegToSpill();
            spill( ress, currentIns);
            res = ress;
            regQueue_.push_back(res);
        }
        bool global = false;
        if(*currentIns->name().begin() == 'g'){global = true;}
        currentIns->setAllocName(generateInstrName(res, global));
        return res;
    }

    SuperNaiveRegisterAllocator::VirtualRegister SuperNaiveRegisterAllocator::getFReg( Instruction * currentIns) {
        VirtualRegister res = VirtualRegister(RegisterType::FLOAT, 0);
        if(!freeFReg_.empty()){
            res = *freeFReg_.begin();
            freeFReg_.erase(freeFReg_.begin());

            regQueue_.push_back(res);
        }else {
            VirtualRegisterPlaceholder ress = getRegToSpill();
            spill( ress, currentIns);
            res = ress;
            regQueue_.push_back(res);
        }bool global = false;
        if(*currentIns->name().begin() == 'g'){global = true;}
        currentIns->setAllocName(generateInstrName(res, global));

        return res;
    }

    SuperNaiveRegisterAllocator::VirtualRegister SuperNaiveRegisterAllocator::getLastRegister(const Instruction * currentIns) {
        VirtualRegister res = VirtualRegister(RegisterType::INTEGER, 0);

        return res;
    }

    void SuperNaiveRegisterAllocator::releaseRegister(const VirtualRegister & reg) {
        if(reg.getType() == RegisterType::INTEGER ){
            if( reg.getNumber() >= _regOffset__){

                freeReg_.emplace( reg);
                auto it = std::find(regQueue_.begin(), regQueue_.end(),reg);
                if(it != regQueue_.end()){
                    regQueue_.erase(it);
                }
            }

        }else if(reg.getType() == RegisterType::FLOAT ){
            if( reg.getNumber() >= _regOffset__){

                freeFReg_.emplace( reg);
                auto it = std::find(regQueue_.begin(), regQueue_.end(),reg);
                if(it != regQueue_.end()){
                    regQueue_.erase(it);
                }
            }

        }else {
            throw "[SuperNaiveRegisterAllocator]not Implemented 1656435";
        }
    }


    void SuperNaiveRegisterAllocator::removeFromRegisterDescriptor(const Instruction *ins) {
        throw "not Implemented [RA] removeFromRegisterDescriptor fnc ";
    }
    void SuperNaiveRegisterAllocator::registerMemLocation(const Store *ins, const Instruction *currentIns) {

        auto it = getAllocMapping(&targetProgram_).find(ins->address());
        if(it != getAllocMapping(&targetProgram_).end()){

            addressDescriptor_[ins->value()].emplace( LocationEntry( ins->address(), ins->value(), it->second));
        }
    }

    void SuperNaiveRegisterAllocator::eraseFromFreeReg(VirtualRegisterPlaceholder &reg) {

            if (reg.getType() == RegisterType::INTEGER){
                auto it = std::find(freeReg_.begin(), freeReg_.end(), reg);
                if(it != freeReg_.end()){
                    freeReg_.erase(it);
                }

            }else if(reg.getType() == RegisterType::FLOAT){
                auto it = std::find(freeFReg_.begin(), freeFReg_.end(), reg);
                if(it != freeFReg_.end()){
                    freeFReg_.erase(it);
                }

            }else {
                throw "[Register Allocator] unknown RegisterType";
            }
        }
}