#pragma once

#include "t86/program/programbuilder.h"

namespace tvlm{

    class VirtualRegisterPlaceholder{
    public:
        virtual VirtualRegisterPlaceholder * makeCopy() = 0;
        virtual ~VirtualRegisterPlaceholder() = default;
    };

    class VirtualRegister : public VirtualRegisterPlaceholder{
    public:
        VirtualRegister(): reg_(tiny::t86::Register(-1)), set_(false){}
        VirtualRegister(const VirtualRegister & other): reg_(other.reg_), set_(other.set_){}
    private:
        VirtualRegisterPlaceholder *makeCopy() override {
            return new VirtualRegister(*this);
        }

    public:
        void setRegister(const tiny::t86::Register & reg){
            reg_ = reg;
            set_ = true;
        }
    private:
        tiny::t86::Register reg_;
        bool set_;
    };

    class VirtualFloatRegister : public VirtualRegisterPlaceholder{
    public:
        VirtualFloatRegister(): reg_(tiny::t86::FloatRegister(-1)), set_(false){}
        VirtualFloatRegister(const VirtualFloatRegister & other): reg_(other.reg_), set_(other.set_){}
        void setRegister(const tiny::t86::FloatRegister & reg){
            reg_ = reg;
            set_ = true;
        }

    private:
        VirtualRegisterPlaceholder *makeCopy() override {
            return new VirtualFloatRegister(*this);
        }

        tiny::t86::FloatRegister reg_;
        bool set_;
    };
}
