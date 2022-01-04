#include "il.h"

namespace tvlm {
    void Instruction::Terminator1::print(tiny::ASTPrettyPrinter &p) const {
        Instruction::print(p);
        p << p.keyword << instrName_ << " " << p.identifier << target_->name();

    }

    void Instruction::TerminatorN::print(tiny::ASTPrettyPrinter &p) const {
        Instruction::print(p);
        p << p.keyword << instrName_ << " " << p.identifier;
        for(auto & t : targets_){
            p << t->name() << " ";
        }
    }

    void Instruction::DirectCallInstruction::print(tiny::ASTPrettyPrinter &p) const {
        Instruction::print(p);
        p << p.keyword << instrName_ << " " << p.identifier << f_->name().name();
        p << p.symbol << " (";
        for (auto & i : args_)
            printRegister(p, i);
        p << p.symbol << ")";
    }

    void Instruction::PhiInstruction::print(tiny::ASTPrettyPrinter &p) const {
        Instruction::print(p);
        p << p.keyword << instrName_ << " ";
        for(auto & i : contents_ ){
            printRegister(p, i.second);
            p <<  "<--" << i.first->name() << ", ";

        }
    };

    void Instruction::ElemInstruction::print(tiny::ASTPrettyPrinter &p) const {
        Instruction::print(p);
        p << p.keyword << instrName_ << " " ;
        printRegister(p, base_);
        for(auto & c: contents_){
            p << p.keyword<<  "+ ";
            if(c.first) printRegister(p, c.first);
            else p << p.numberLiteral <<  "1 ";
            p << p.keyword<<  "x " << p.numberLiteral << c.second;
        }

    }

    Instruction::DirectCallInstruction::DirectCallInstruction(Function *f, std::vector<Instruction *> &&args,
                                                              const ASTBase *ast, const std::string &instrName,
                                                              Instruction::Opcode opcode) :
            Instruction::CallInstruction{std::move(args), ast, instrName, opcode, f->getResultType()},
            f_{f}
    {

    }
}

