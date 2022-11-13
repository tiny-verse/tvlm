#include "ProgramBuilder.h"

#include "tvlm/tvlm/il/il.h"


namespace tvlm{

    Program *TargetProgramFriend::getProgram(TargetProgram *p) const {
        return p->program_.get();
    }

     Program *TargetProgramFriend::getProgram(TargetProgram &program) const {
        return program.program_.get();
    }

     std::map<const Function *, size_t> &TargetProgramFriend::getFuncLocalAlloc(TargetProgram *program) const {
        return program->funcLocalAlloc_;
    }

     std::map<const ILInstruction*, std::vector<TInstruction*>> & TargetProgramFriend::getSelectedInstrs(TargetProgram * program)const {
        return program->selectedInstrs_;
    }

     std::vector<std::pair<std::pair<const ILInstruction *, Label>, const BasicBlock*>> & TargetProgramFriend::getJump_patches(TargetProgram * program)const {
        return program->jump_patches_;
    }


     std::vector<std::pair<std::pair<const ILInstruction *, Label>, Symbol>> & TargetProgramFriend::getCall_patches(TargetProgram * program)const {
//        return program->call_patches_;
        return program->unpatchedFCalls_;
    }

     std::map<const Instruction *, uint64_t> & TargetProgramFriend::getGlobalTable(TargetProgram * program)const{
        return program->globalTableAddress_;
    }

     std::map<const Instruction *, std::vector<VirtualRegisterPlaceholder>> & TargetProgramFriend::getAllocatedRegisters(TargetProgram * program)const{
        return program->alocatedRegisters_;
    }

     std::map<const Instruction *, std::list<std::function<tvlm::TInstruction *(std::vector<VirtualRegisterPlaceholder> &)>>> &
    TargetProgramFriend::getSelectedFInstrs(TargetProgram * program)const{
        return program->selectedFInstrs_;
    }


     std::map<const Instruction *, std::vector<size_t>> &
    TargetProgramFriend::getJumpPos(TargetProgram * program)const{
        return program->jumpPos_;
    }

     std::map<const Instruction *, std::vector<size_t>> &
    TargetProgramFriend::getCallPos(TargetProgram * program)const{
        return program->callPos_;
    }

     std::vector<std::pair<std::pair<const Instruction *, tiny::t86::Label>, tiny::Symbol>> &
    TargetProgramFriend::getUnpatchedFCalls(TargetProgram * program)const{
        return program->unpatchedFCalls_;
    }



     std::map<const Instruction *, int64_t> &
    TargetProgramFriend::getAllocMapping(TargetProgram * program)const{
        return program->allocMapping_;
    }


     std::map<const Function *, size_t> &TargetProgramFriend::getFuncLocalAlloc(TargetProgram &program) const {
        return program.funcLocalAlloc_;
    }

     std::map<const ILInstruction*, std::vector<TInstruction*>> & TargetProgramFriend::getSelectedInstrs(TargetProgram & program)const {
        return program.selectedInstrs_;
    }

     std::vector<std::pair<std::pair<const ILInstruction *, Label>, const BasicBlock*>> & TargetProgramFriend::getJump_patches(TargetProgram & program)const {
        return program.jump_patches_;
    }


     std::vector<std::pair<std::pair<const ILInstruction *, Label>, Symbol>> & TargetProgramFriend::getCall_patches(TargetProgram & program)const {
//        return program.call_patches_;
        return program.unpatchedFCalls_;
    }

     std::map<const Instruction *, uint64_t> & TargetProgramFriend::getGlobalTable(TargetProgram & program)const{
        return program.globalTableAddress_;
    }

     std::map<const Instruction *, std::vector<VirtualRegisterPlaceholder>> & TargetProgramFriend::getAllocatedRegisters(TargetProgram & program)const{
        return program.alocatedRegisters_;
    }

     std::map<const Instruction *, std::list<std::function<tvlm::TInstruction *(std::vector<VirtualRegisterPlaceholder> &)>>> &
    TargetProgramFriend::getSelectedFInstrs(TargetProgram & program)const{
        return program.selectedFInstrs_;
    }


     std::map<const Instruction *, std::vector<size_t>> &
    TargetProgramFriend::getJumpPos(TargetProgram & program)const{
        return program.jumpPos_;
    }

     std::map<const Instruction *, std::vector<size_t>> &
    TargetProgramFriend::getCallPos(TargetProgram & program)const{
        return program.callPos_;
    }

     std::vector<std::pair<std::pair<const Instruction *, tiny::t86::Label>, tiny::Symbol>> &
    TargetProgramFriend::getUnpatchedFCalls(TargetProgram & program)const{
        return program.unpatchedFCalls_;
    }



     std::map<const Instruction *, int64_t> &
    TargetProgramFriend::getAllocMapping(TargetProgram & program)const{
        return program.allocMapping_;
    }





}

