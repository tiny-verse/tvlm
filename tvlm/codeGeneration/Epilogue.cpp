#include "Epilogue.h"

#include "t86/program/helpers.h"
#include "t86/instruction.h"
#include "tvlm/tvlm/codeGeneration/FunctionalMacro.h"

namespace tvlm{

    void NaiveEpilogue::visit(Return *ins) {
        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(CallStatic *ins) {
        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(Call *ins) {
        visitInstrHelper(ins);
    }

    Epilogue::TProgram NaiveEpilogue::translate(SProgram && sprogram) {

        program_ = std::move(sprogram);
        visit(getProgram(&program_).get());



        tiny::t86::Program rawProg = pb_.program();
        std::vector<tiny::t86::Instruction*> instrs = rawProg.moveInstructions();
        int line = 0;
        for(const tiny::t86::Instruction * i : instrs){
            std::cerr << tiny::color::blue << line++ << ": " << tiny::color::green << i->toString() << std::endl;
        }

        return {std::move(instrs), getData(&program_)};
    }

    void NaiveEpilogue::visitInstrHelper(Instruction * ins){
        auto registers = getAllocatedRegisters(&program_).find(ins);
        std::vector<VirtualRegisterPlaceholder> regs;
        if(registers == getAllocatedRegisters(&program_).end()){

            regs = std::vector<VirtualRegisterPlaceholder>();
        }else{
            regs = registers->second;
        }

        auto finstruction = getSelectedFInstrs(&program_).find(ins);
        if(finstruction == getSelectedFInstrs(&program_).end()){
            throw "instruction failed to compile -> no selectedInstruction";
            return;
        }

        auto & finsns = finstruction->second;
        for (auto & finsn : finsns){
            TInstruction * compiled = finsn(regs);

            getSelectedInstrs(&program_)[ins].emplace_back(compiled);
        }
        lastIns_ = add(ins);
        for(int c = 0; c < finsns.size();c++){
            compiledInsns_.emplace(std::make_pair(ins, c), lastIns_ + c);
        }

    }

    void NaiveEpilogue::visit(Instruction *ins) {
//        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(Jump *ins) {
        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(CondJump *ins) {
        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(Copy *ins) {
        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(Extend *ins) {
        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(Truncate *ins) {
        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(BinOp *ins) {
        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(UnOp *ins) {
        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(LoadImm *ins) {
        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(AllocL *ins) {
        if(ins->amount()){
            visitInstrHelper(ins);
        }
    }

    void NaiveEpilogue::visit(AllocG *ins) {
        if(ins->amount()){
            visitInstrHelper(ins);
        }
    }

    void NaiveEpilogue::visit(ArgAddr *ins) {
        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(PutChar *ins) {
        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(GetChar *ins) {
        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(Load *ins) {
        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(Store *ins) {
        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(Phi *ins) {
        //nothing to do
    }

    void NaiveEpilogue::visit(ElemAddrOffset *ins) {
        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(ElemAddrIndex *ins) {
        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(Halt *ins) {
        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(StructAssign *ins) {
        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(BasicBlock *bb) {
        tiny::t86::Label ret = pb_.currentLabel(); //next from last Instruction Label
        compiledBB_.emplace(bb, ret);
        auto insns = getBBsInstructions(bb);
        int64_t len = (int64_t) insns.size()-1;
        if(len == -1){
            lastIns_ = ret; // return ret; // empty BB;
            return;
        }
        for(int64_t i = 0; i < len ;i++){
            tiny::t86::Label tmp = pb_.currentLabel();
            visitChild(insns[i]);

        }
        auto last = insns[len];
        visitChild(last);

        lastIns_ = ret; // return ret;
    }

    void NaiveEpilogue::visit(Function *fce) {
        tiny::t86::Label ret =
                pb_.add(tiny::t86::PUSH(tiny::t86::Bp()), nullptr);
        pb_.add(tiny::t86::MOV(tiny::t86::Bp(), tiny::t86::Sp()), nullptr);
        pb_.add(tiny::t86::SUB(tiny::t86::Sp(), getFuncAlloc(fce)), nullptr);
        for (BasicBlock * bb : getFunctionBBs(fce)) {
            visitChild(bb);
        }
        lastIns_ = ret; //return ret;
    }

    void NaiveEpilogue::visit(Program *p) {
        auto globs = getProgramsGlobals(p);

        tiny::t86::Label start = pb_.add(tiny::t86::JMP(tiny::t86::Label::empty()), nullptr);

        //===================================FUNCTIONS=====================================
        for (auto & i : p->functions()){
            visitChild(i.second.get());
            Label fncLabel = lastIns_;
            functionTable_.emplace(i.first, fncLabel.address());
        }

//        //===================================MEM INTERCEPT=====================================
        tiny::t86::Label prolog = pb_.currentLabel();//t86::Label(lastInstruction_index +1);
        for(auto & f : functionTable_){
            const Instruction * fnc_addr = p->getGlobalVariableAddress( f.first);
            uint64_t value = f.second.address();

            program_.interceptInstruction(fnc_addr, {LMBS  tiny::t86::MOV(vR(0), value) LMBE});
        }
        compiledGlobalTable(globs);

        auto it = functionTable_.find(Symbol("main"));
        if(it == functionTable_.end()){
            throw tiny::ParserError( "main function not found" , tiny::SourceLocation(0,0,0) );
        }
        pb_.add(tiny::t86::CALL(it->second.address()), nullptr) ;

        pb_.add(
                tiny::t86::DBG(
                        [](tiny::t86::Cpu & cpu){
                            auto & os = std::cout;
                            os <<std::endl
                            << "*****************************************"
                            <<std::endl;
                            os << "Pc: " << cpu.getRegister(tiny::t86::Pc()) << '\n';
                            os << "Sp: " << cpu.getRegister(tiny::t86::Sp()) << '\n';
                            os << "Bp: " << cpu.getRegister(tiny::t86::Bp()) << '\n';
                            os << "Flags: " << cpu.getRegister(tiny::t86::Flags()) << '\n';
                            auto max = cpu.registersCount() < cpu.floatRegistersCount() ? cpu.floatRegistersCount() : cpu.registersCount();
                            for (std::size_t i = 0; i < max; ++i) {
                                if(i < cpu.registersCount()){
                                    os << "Reg(" << i << "): " << cpu.getRegister(tiny::t86::Reg(i)) << "\t\t";

                                }else{
                                    os << "\t\t\t\t";
                                }

                                if(i < cpu.floatRegistersCount()){
                                    os << "FReg(" << i << "): " << cpu.getFloatRegister(tiny::t86::FReg(i));
                                }
                                os << '\n';
                            }
                            os << std::flush;
                        }
                ), nullptr
                );
        pb_.add(tiny::t86::HALT(), nullptr);
        pb_.patch(start, prolog);

        for (const auto & toPatch: getJump_patches(&program_)) {
            auto it = compiledBB_.find(toPatch.second);
            if(it == compiledBB_.end()){
                throw "wat now? - I can't jump on invalid BB....";
            }
            pb_.patch(resolveInstruction(toPatch.first), it->second );
        }

        //===================================PATCHING Calls=====================================
        auto & tmp = getCall_patches(&program_);

        for( auto & toPatch : tmp ){
            auto it = functionTable_.find(toPatch.second);
            if(it == functionTable_.end()){
                throw "WTF failed patching calls - function name not found";
            }
            pb_.patch(resolveInstruction(toPatch.first),it->second);

        }

    }

    Label NaiveEpilogue::add(const Instruction *ins) {
        auto & selected = getSelectedInstrs(&program_);
        auto it = selected.find(ins);
        if(it == selected.end()){
            throw "instruction not compiled";
            return Label::empty();
        }
        return pb_.add(it->second, ins);

    }

    Label NaiveEpilogue::compiledGlobalTable(BasicBlock *globals) {
        Label tmp = pb_.currentLabel();
        for (auto * ins : getBBsInstructions(globals)) {
            visitChild(ins);
        }

        return tmp;
    }

}
