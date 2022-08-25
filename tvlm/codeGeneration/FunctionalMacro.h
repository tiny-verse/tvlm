#pragma once


#define LMB [=](const std::vector<VirtualRegisterPlaceholder> & regs)
#define LMBS [=](const std::vector<VirtualRegisterPlaceholder> & regs){ return new
#define LMBE ;}
#define LMBJUMP [=](const std::vector<VirtualRegisterPlaceholder> & regs){return new tiny::t86::JMP(tiny::t86::Label::empty());}
#define vR(REG) Register(regs[REG].getNumber())
#define vFR(REG) FRegister(regs[REG].getNumber())
