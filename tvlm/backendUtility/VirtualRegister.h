#pragma once

#include "t86/program/programbuilder.h"

namespace tvlm{

    enum class RegisterType{
        INTEGER,
        FLOAT,
    };

    class VirtualRegisterPlaceholder{
    public:
//        virtual VirtualRegisterPlaceholder * makeCopy() = 0;
        virtual ~VirtualRegisterPlaceholder() = default;
        VirtualRegisterPlaceholder(const RegisterType & regType, size_t number):
        regType_(regType)
        ,number_(number)
        {}

        VirtualRegisterPlaceholder(VirtualRegisterPlaceholder && other):
        regType_(std::move(other.regType_))
        ,number_(std::move(other.number_))
        {}

        void setNumber(size_t number){
            number_ = number;
        }

        RegisterType getType()const{
            return regType_;
        }

        size_t getNumber()const {
            return number_;
        }
        bool operator<(const VirtualRegisterPlaceholder & rhs) const{
            return (this->regType_ < rhs.regType_ ) ? true :  this->regType_ == rhs.regType_ && this->number_ < rhs.number_ ;
        }
    private:
        size_t number_;
        RegisterType regType_;

    };
//
//    class VirtualRegister : public VirtualRegisterPlaceholder{
//    public:
//        VirtualRegister(): reg_(tiny::t86::Register(-1)), set_(false){}
//        VirtualRegister(const VirtualRegister & other): reg_(other.reg_), set_(other.set_){}
//    private:
//        VirtualRegisterPlaceholder *makeCopy() override {
//            return new VirtualRegister(*this);
//        }
//
//    public:
//        void setRegister(const tiny::t86::Register & reg){
//            reg_ = reg;
//            set_ = true;
//        }
//    private:
//        tiny::t86::Register reg_;
//        bool set_;
//    };
//
//    class VirtualFloatRegister : public VirtualRegisterPlaceholder{
//    public:
//        VirtualFloatRegister(): reg_(tiny::t86::FloatRegister(-1)), set_(false){}
//        VirtualFloatRegister(const VirtualFloatRegister & other): reg_(other.reg_), set_(other.set_){}
//        void setRegister(const tiny::t86::FloatRegister & reg){
//            reg_ = reg;
//            set_ = true;
//        }
//
//    private:
//        VirtualRegisterPlaceholder *makeCopy() override {
//            return new VirtualFloatRegister(*this);
//        }
//
//        tiny::t86::FloatRegister reg_;
//        bool set_;
//    };
}
