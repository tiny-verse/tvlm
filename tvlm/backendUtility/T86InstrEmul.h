#pragma once

#include "t86/t86/program/programbuilder.h"

#include "tvlm/backendUtility/VirtualRegister.h"

#include <functional>

namespace tvlm {

//    class T86InstrEmul {
//    public:
//        static std::function<tiny::t86::Instruction *(const std::vector<VirtualRegisterPlaceholder*> & )>ADD(){
//            return [&](const std::vector<VirtualRegisterPlaceholder*> & registers){
//                return new tiny::t86::ADD(getRegister(registers[0]), getRegister(registers[1]));
//            }
//        }
//    protected:
//        static auto getRegister(VirtualRegisterPlaceholder * placeholder) {
//            if(auto reg = dynamic_cast<VirtualRegister *>(placeholder) ){
//                return reg.reg_;
//            }else if (auto freg = dynamic_case<VirtualFloatRegister *> (placeholder)){
//                return freg.reg_;
//            }else{
//                throw "ERROR wrong register placeholder";
//            }
//        }
//    };

}
