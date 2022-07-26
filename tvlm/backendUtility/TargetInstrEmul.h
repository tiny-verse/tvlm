#pragma once

#include "t86/program/programbuilder.h"
#include "tvlm/tvlm/backendUtility/VirtualRegister.h"

#include <vector>
#include <functional>

namespace tvlm {

    class TargetInstrEmul{
    public:
        using FCreate = std::function<tiny::t86::Instruction * (const std::vector<VirtualRegisterPlaceholder *> &)> ;
        virtual ~TargetInstrEmul() = default;
        TargetInstrEmul(const FCreate & create, const std::vector<VirtualRegisterPlaceholder *> & registers ):
                create_(create)
                ,registers_(registers){

        }
    private:
        FCreate create_;
        std::vector<VirtualRegisterPlaceholder *> registers_;
    };



}
