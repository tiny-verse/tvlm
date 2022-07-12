#include "ColoringAllocator.h"



namespace tvlm {

    Register ColoringAllocator::getRegOutro(const Register & reg, const Instruction * ins){
        register_descriptor_[reg].insert(ins);
        address_descriptor_[ins].emplace( LocationEntry(Location::Register, reg.index()) );
        return Register(reg);
    }

    Register ColoringAllocator::pickForSpill(const Instruction *ins) {
        //TODO
        return Register(0);
    }

    void ColoringAllocator::spillIntReg(const Instruction * ins){
        //TODO
        Register regToSpill = pickForSpill(ins);
        //inject code?
        address_descriptor_[ins].erase(LocationEntry(Location::Register, regToSpill.index()));
        //

    }

    Register ColoringAllocator::getReg(const Instruction *ins) {
        auto it = address_descriptor_.find(ins);
        if(it == address_descriptor_.end()){

        }else {
            auto places = it->second;
            auto reg = places.find(LocationEntry(Location::Register, 0));
            if(reg != places.end()){
                Register res = Register(reg->regIndex());
                return getRegOutro(res, ins);
            }else if (auto stack = places.find(LocationEntry(Location::Stack, 0)) != places.end()){
            //spilled

            //getFreeRegister
            bool freeReg;
            if(freeReg){
                return getRegOutro(Register(freeReg), ins);//TODO
            }else{
                spillIntReg(ins);


            }
            }else if (auto mem = places.find(LocationEntry(Location::Memory, 0)) != places.end()){
            //(global) variable


            }else{
                throw "invalid address descriptor!";
            }
        }
        return tvlm::Register(0);
    }

    FRegister ColoringAllocator::getFloatReg(const Instruction *ins) {
        return tvlm::FRegister(0);
    }

    void ColoringAllocator::clearInt(const Instruction *ins) {

    }

    void ColoringAllocator::clearFloat(const Instruction *ins) {

    }

    void ColoringAllocator::spillCallReg() {

    }

    void ColoringAllocator::clearAllReg() {

    }

    void ColoringAllocator::spillAllReg() {

    }

    void ColoringAllocator::prepareReturnValue(size_t size, const Instruction * ret) {

    }

    void ColoringAllocator::makeLocalAllocation(size_t size, const Register &reg, const Instruction * ins) {

    }

    void ColoringAllocator::allocateStructArg(Type *type, const Instruction *ins) {

    }

    void ColoringAllocator::resetAllocSize() {

    }

    void ColoringAllocator::correctStackAlloc(size_t patch) {

    }

    Register ColoringAllocator::getIntRegister(const Instruction *ins) {
        return tvlm::Register(0);
    }

    FRegister ColoringAllocator::getFloatRegister(const Instruction *ins) {
        return tvlm::FRegister(0);
    }

    Register ColoringAllocator::getFreeIntRegister() {
        return tvlm::Register(0);
    }

    FRegister ColoringAllocator::getFreeFloatRegister() {
        return tvlm::FRegister(0);
    }

    bool ColoringAllocator::isInsInMem(const Instruction *ins) const {
        auto tmp = address_descriptor_.at(ins);
        for (auto & record : tmp) {
            if(record.loc() == Location::Memory){
                return true;
            }
        }
        return false;
    }

    bool ColoringAllocator::isInsAtStack(const Instruction *ins) const {
        return false;
    }

    bool ColoringAllocator::isInsInRegister(const Instruction *ins) const {
        return false;
    }
}