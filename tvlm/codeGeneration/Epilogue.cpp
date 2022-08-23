#include "Epilogue.h"

#include "t86/program/helpers.h"
#include "t86/instruction.h"


namespace tvlm{







    Epilogue::TProgram NaiveEpilogue::translate(Epilogue::SProgram & sprogram) {
//        return Epilogue::translate(program);
        this->program_ = sprogram;
        visit(getProgramprogram(sprogram));



        tiny::t86::Program rawProg = pb_.program();
        std::vector<tiny::t86::Instruction*> instrs = rawProg.moveInstructions();
        int line = 0;
        for(const tiny::t86::Instruction * i : instrs){
            std::cerr << tiny::color::blue << line++ << ": " << tiny::color::green << i->toString() << std::endl;
        }

        return {std::move(instrs), rawProg.data()};
    }

    void NaiveEpilogue::visitInstrHelper(Instruction * ins){
        auto registers = program_.alocatedRegisters_.find(ins);
        if(registers == program_.alocatedRegisters_.end()){
            throw "instruction failed to compile -> no registers allocated";
            return;
        }
        auto & regs = registers->second;

        auto finstruction = program_.selectedFInstrs_.find(ins);
        if(finstruction == program_.selectedFInstrs_.end()){
            throw "instruction failed to compile -> no selectedInstruction";
            return;
        }

        auto & finsns = finstruction->second;

        for ( size_t i = 0; finsns.size(); i++){
            TInstruction * compiled = finsns[i](regs);
//            auto selected = program_.selectedInstrs_.find(ins);
//            if(selected == program_.selectedInstrs_.end()){
//                program_.selectedInstrs_[ins] = std::vector<TInstruction *>();
//            }
            program_.selectedInstrs_[ins].emplace_back(compiled);
        }
        lastIns_ = add(ins);
        for(int c = 0; c < finsns.size();c++){
            compiledInsns_.emplace(std::make_pair(ins, c), lastIns_ + c);
        }
        return;
//--------------------------------------------------
        auto selected = getSelectedInstrs(program_);
        auto it = selected.find(ins);
        if(it != selected.end()){
//            for (auto * inss :it->second) {
//                Label tmp = pb_.add(inss, ins);
//            }
            lastIns_ = add(ins);
            for(int c = 0; c < it->second.size();c++){
                compiledInsns_.emplace(std::make_pair(ins, c), lastIns_ + c);
            }
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

    void NaiveEpilogue::visit(Return *ins) {
        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(CallStatic *ins) {
        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(Call *ins) {
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
        visitInstrHelper(ins);
    }

    void NaiveEpilogue::visit(AllocG *ins) {
//        visitInstrHelper(ins);
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
        visitInstrHelper(ins);
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
//            compiled_.emplace(insns[i], tmp);
//            compiledInsns_.emplace(insns[i], tmp);
        }
        auto last = insns[len];
//        regAllocator->spillAllReg();
        visitChild(last);
//        regAllocator->clearAllReg();
        lastIns_ = ret; // return ret;
    }

    void NaiveEpilogue::visit(Function *fce) {
        tiny::t86::Label ret =
                pb_.add(tiny::t86::PUSH(tiny::t86::Bp()), nullptr);//TODO maybe in RA
        pb_.add(tiny::t86::MOV(tiny::t86::Bp(), tiny::t86::Sp()), nullptr);
        pb_.add(tiny::t86::SUB(tiny::t86::Sp(), getFuncAlloc(fce)), nullptr); //TODO maybe in RA
        for (BasicBlock * bb : getFunctionBBs(fce)) {
            visitChild(bb);
        }
        lastIns_ = ret; //return ret;
    }

    void NaiveEpilogue::visit(Program *p) {
        auto globs = getProgramsGlobals(p);
        for (const auto & str : p->stringLiterals() ) {
            tiny::t86::DataLabel data = pb_.addData(str.first);
            add( str.second);
        }

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
//            instructionToEmplace.emplace(fnc_addr, new LoadImm((int64_t)f.second.address(), nullptr));
            program_.globalEmplace(fnc_addr, f.second.address());
        }
        compiledGlobalTable(globs);


        auto it = functionTable_.find(Symbol("main"));
        if(it == functionTable_.end()){
            throw tiny::ParserError( "main function not found" , tiny::SourceLocation(0,0,0) );
        }
        pb_.add(tiny::t86::CALL(it->second.address()), nullptr) ;

        pb_.add(tiny::t86::HALT(), nullptr);
        pb_.patch(start, prolog);



        for (const auto & toPatch: getJump_patches(program_)) {
            auto it = compiledBB_.find(toPatch.second);
            if(it == compiledBB_.end()){
                throw "wat now? - I can't jump on invalid BB....";
            }
            pb_.patch(resolveInstruction(toPatch.first), it->second );
        }

        //===================================PATCHING Calls=====================================
        for( auto & toPatch : getCall_patches(program_)){
            auto it = functionTable_.find(toPatch.second);
            if(it == functionTable_.end()){
                throw "WTF failed patching calls - function name not found";
            }
            pb_.patch(resolveInstruction(toPatch.first),it->second);
            std::cerr << "patching call at " << resolveInstruction(toPatch.first) << " with " << it->second << std::endl;
        }

    }

    Label NaiveEpilogue::add(const Instruction *ins) {
        auto & selected = getSelectedInstrs(program_);
        auto it = selected.find(ins);
        if(it == selected.end()){
            return Label::empty();
            throw "instruction not compiled";
        }
        return pb_.add(it->second, ins);

    }

    Label NaiveEpilogue::compiledGlobalTable(BasicBlock *globals) {
        Label tmp = pb_.currentLabel();
        for (auto * ins : getBBsInstructions(globals)) {
            add(ins);
        }
        return tmp;
    }

}
