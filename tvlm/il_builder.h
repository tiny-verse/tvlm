#pragma once

#include "il.h"

namespace tvlm {

    class ILBuilder {
    public:

        struct Context {
            BasicBlock * breakTarget;
            BasicBlock * continueTarget;

            static Context Loop(BasicBlock * breakTarget, BasicBlock * continueTarget) {
                return Context{breakTarget, continueTarget};
            }

            static Context Switch(BasicBlock * breakTarget) {
                return Context{breakTarget, nullptr};
            }

            static Context Empty() {
                return Context{nullptr, nullptr};
            }
        };

        ILBuilder( ):
            globals_{new BasicBlock{}},
            env_{new Environment{}} {
            globals_->setName("globals");
            bb_ = globals_.get();
        }

        Instruction * add(Instruction * ins) {
            assert(bb_ != nullptr);
            bb_->add(ins);
            if (ins->name().empty()) {
                if (f_ != nullptr)
                    ins->setName(STR("r" << (localRegisterCounter_++)));
                else
                    ins->setName(STR("g" << (globalRegisterCounter_++)));
            }
            lastInstr_ = ins;
            return ins;
        }

        Instruction * globalAdd(Instruction * ins){
            globals_->add(ins);
            if(ins->name().empty()){
                ins->setName(STR("g" << (globalRegisterCounter_++)));
            }
            lastInstr_ = ins;
            return ins;
        }

        BasicBlock * createBasicBlock(std::string const & name) {
            assert(f_ != nullptr);
            BasicBlock * result = new BasicBlock{};
            result->setName(name);
            f_->bbs_.push_back(std::unique_ptr<BasicBlock>{result});
            return result;
        }

        BasicBlock * createBasicBlock() {
            assert(f_ != nullptr);
            BasicBlock * result = new BasicBlock{};
            result->setName(STR("bb_" << f_->bbs_.size()));
            f_->bbs_.push_back(std::unique_ptr<BasicBlock>{result});
            return result;
        }

        Context const & context() const {
            return context_;
        }

        Context enterContext(Context newContext) {
            Context result = context_;
            context_ = newContext;
            return context_;
        }

        void leaveContext(Context oldContext) {
            context_ = oldContext;
        }

        BasicBlock * enterBasicBlock(BasicBlock * b) {
            assert(f_ != nullptr);
            assert(f_->contains(b));
            bb_ = b;
            return b;
        }

        Function * enterFunction(Function * f) {
            functions_.emplace_back(f->name(), f);
            f_ = f;
            enterBasicBlock(createBasicBlock());
            // enter new environment for the function
            enterEnvironment();
            return f;
        }

        void leaveFunction() {
            assert(f_ != nullptr);
            f_ = nullptr;
            bb_ = globals_.get();
            leaveEnvironment();
        }

        void enterEnvironment() {
            env_.reset(new Environment{env_.release()});
        }

        void leaveEnvironment() {
            Environment * old = env_.release();
            env_.reset(old->parent);
            delete old;
        }

        Instruction * getVariableAddress(Symbol name) const {
            return env_->getVariableAddress(name);
        }

        Instruction * addVariable(Symbol name, Instruction * alloc) {
            env_->names.insert(std::make_pair(name, alloc));
            return alloc;
        }

        Instruction * addGlobalVariable(Symbol name, Instruction * alloc) {
            Environment * env = env_.get();
            while (env->parent != nullptr)
                env = env->parent;
            env->names.insert(std::make_pair(name, alloc));
            return alloc;
        }

        Instruction * getStringLiteral(std::string const & lit, const tiny::ASTBase * ast){
            auto i = stringLiterals_.find(lit);
            if (i == stringLiterals_.end()) {
                Instruction * addr = globalAdd(new tvlm::AllocG{lit.size() + 1,nullptr , ast});
                stringLiterals_.insert(std::make_pair(lit, addr));
                return addr;
            } else {
                return i->second;
            }
        }

        void print(tiny::ASTPrettyPrinter & p) const {
            globals_->print(p);
            for (auto & i : functions_)
                i.second->print(p);
        }
        void finalize(){
//            add(new Halt{nullptr}); // TODO append halt instr
        }

        tvlm::Program finish(){
            return tvlm::Program(std::move(stringLiterals_),
                           std::move(functions_),
                           std::move(globals_));
        }

        const std::vector<std::pair<Symbol, std::unique_ptr<Function>>> & functions()const {
            return functions_;
        }

        Function * findFnc(Symbol & name) const{
            auto r = functions_.begin();
            for(;r != functions_.end();r++){if(r->first == name) break;}
            return (r != functions_.end()) ? r->second.get() : nullptr;
        }

//        size_t sizeOfType(Type * t) {
//            return 4; //TODO
//        }

        Instruction * lastInstr()const {
            return lastInstr_;
        }


        bool globalEnv(){
            return env_->parent == nullptr;
        }

    private:
        // wrappers to backend who manages the types for us
        /*
        Type * getType(Symbol symbol);
        Type * getTypeVoid();
        Type * getTypeInt();
        Type * getTypeDouble();
        Type * getTypeChar();
        bool isPointer(Type * t);
        bool isPOD(Type * t);
        bool convertsToBool(Type * t);
        */


        struct Environment {
            Environment * parent = nullptr;
            std::unordered_map<Symbol, Instruction*> names;

            Environment() = default;

            Environment(Environment * parent):
                parent{parent} {
            }

            Instruction * getVariableAddress(Symbol name) {
                auto i = names.find(name);
                if (i != names.end())
                    return i->second;
                else if (parent != nullptr)
                    return parent->getVariableAddress(name);
                else
                    return nullptr;
            }
            Instruction * getVariableType(Symbol name) {
                auto i = names.find(name);
                if (i != names.end())
                    return i->second;
                else if (parent != nullptr)
                    return parent->getVariableType(name);
                else
                    return nullptr;
            }

        };

        //Backend & backend_;

        std::unordered_map<std::string, Instruction*> stringLiterals_;
        std::vector<std::pair<Symbol, std::unique_ptr<Function>>> functions_;
        std::unique_ptr<BasicBlock> globals_;
        std::unique_ptr<Environment> env_;
        Context context_ = Context::Empty();
        Function * f_ = nullptr;
        BasicBlock * bb_ = nullptr;

        size_t localRegisterCounter_ = 0;
        size_t globalRegisterCounter_ = 0;

        Instruction * lastInstr_ = nullptr;
    }; // Builder




} // namespace tvlm