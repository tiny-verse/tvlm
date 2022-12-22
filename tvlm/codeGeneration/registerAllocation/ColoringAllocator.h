#pragma once

#include <stack>
#include "RegisterAllocator.h"
#include "t86/instruction.h"
#include "t86/t86_target"
#include "t86/program.h"
#include "t86/program/programbuilder.h"
#include "tvlm/codeGeneration/ProgramBuilder.h"

#include "tvlm/analysis/liveness_analysis.h"
#include "tvlm/analysis/liveness_analysisTartget.h"
#include "SuperNaiveRegisterAllocator.h"


/**
 * Preparation:
 *      Instruction Selection done - TInstruction known
 *          - Nontrivial
 *      Naively assigned Registers
 *      Liveness analysis done //TODO
 *          - Cfg construction //TODO
 *      ColorPicking
 *          -make graph:
 *              -verticies: live range
 *              -edges: overlapping ranges
 *          - each live range gets color (Register) by rules:
 *              1) K colors: node with degree k-1 safely colored
 *              2) coloring this node remove them from graph (might enable other nodes)
 *              3) no such colorable? -> "(spill)" : might enable condition 1)
 *              removing from graph == push to regStack -> determines color
 *
 *
 *
 * Work:
 *      Rework assigned registers by idea of graph coloring
 * */
namespace tvlm{

    class ColorInfo{
    public:
        int color;
    };



    class LiveRange {
    public:
        LiveRange(Instruction * il, IL * start):
        il_({il}),
        start_(start),
        end_(start){

        }

        void add(Instruction * in) {
            il_.emplace(in);
        }

        void setEnd(const IL *end){
            end_ = end;
        }

        const std::set<ILInstruction *>& il() const {
            return il_;
        }
        IL * start() const {
            return start_;
        }
    private:
        std::set<ILInstruction *> il_;
        IL * start_;
        const IL * end_;
    };

    class ColoringAllocator : public SuperNaiveRegisterAllocator{
    public:
        ColoringAllocator(TargetProgram && prog):
        SuperNaiveRegisterAllocator(std::move(prog)),
        programChanged_(false){
        }

        bool changedProgram()const{
            return programChanged_;
        }
        TargetProgram & run(TargetProgram &&  prog){
            targetProgram_ = std::move(prog);
            return this->run();
        }

        TargetProgram & run()override{

        //----Preparation----
            //LivenessAnalysis
            Program * prog = getProgram(targetProgram_);
            programChanged_ = false;
            bool again = true;
//            while(again){

                auto la =  new ColoringLiveAnalysis<ColorInfo>(&targetProgram_);
                analysisResult_ = la->analyze(); //TODO check memory allocation
                auto tmp = la->getLiveRanges();
                for (auto * i : tmp) {
                    addLR(std::move(std::unique_ptr<CLiveRange>(i)));
                }


                //ColorPicking
               again = !generateLiveRanges();

//            }
            //Next convert result of colorPicking to update VirtualRegisterPlaceholders



            //implement logic of passing through the program;
            if(programChanged_){
                delete la;
                return targetProgram_;
            }

            setColors();



            delete la;
            return RegisterAllocator::run();

        }


//        void ReassignRegisters(ILBuilder & ilb /*or ProgramBuilderOLD and res of analysis*/){
//            auto prog = ilb.finish();
//            auto la = new LivenessAnalysis<ColorInfo>(&prog); // Integrate ILBuilder and ProgramBuilderOLD
//            analysisResult_ = la->analyze();
////            analysis_mapping_ = la->analysis_mapping()
//        }
//        void ReassignRegisters(Program * prog /*or ProgramBuilderOLD and res of analysis*/){
//            auto la = new LivenessAnalysis<ColorInfo>( prog); // Integrate ILBuilder and ProgramBuilderOLD
//            analysisResult_ = la->analyze();
//
//        }
//
//        Register getReg(const Instruction *ins) override;
//        Register getRegOutro(const Register & reg, const Instruction *ins);
//        void spillIntReg(const Instruction * ins);
//        Register pickForSpill(const Instruction * ins);
//
//        FRegister getFloatReg(const Instruction *ins) override;
//
//        void clearInt(const Instruction *ins) override;
//
//        void clearFloat(const Instruction *ins) override;
//
//        void spillCallReg() override;
//
//        void clearAllReg() override;
//
//        void spillAllReg() override;
//
//        void prepareReturnValue(size_t size, const Instruction * ret) override;
//
//        void makeLocalAllocation(size_t size, const Register &reg, const Instruction * ins) override;
//
//        void allocateStructArg(const Type *type, const Instruction *ins) override;
//
//        void resetAllocSize() override;
//
//        void correctStackAlloc(size_t patch) override;
//
//        bool isInsInRegister(const Instruction * ins) const;
//        bool isInsAtStack(const Instruction * ins) const;
//        bool isInsInMem(const Instruction * ins) const;

    protected:
//        Register getIntRegister(const Instruction *ins) override;
//
//        FRegister getFloatRegister(const Instruction *ins) override;
//
//        Register getFreeIntRegister() override;
//
//        FRegister getFreeFloatRegister() override;

        class LiveRangesComparator
        {
            // this member is required to let container be aware that
            // comparator is capable of dealing with types other than key
        public: using is_transparent = std::true_type;

        public: bool operator()(const ILInstruction * left, const std::pair<LiveRange *, size_t> & right) const
            {
                return left < right.first->start();
            }

        public: bool operator()(const std::pair<LiveRange *, size_t> & left, const ILInstruction * right) const
            {
                return left.first->start() < right;
            }

        public: bool operator()(const std::pair<LiveRange *, size_t> &  left, const std::pair<LiveRange *, size_t> &  right) const
            {
                return left.first->start() < right.first->start();
            }
        };


//        MAP<const CfgNode<ColorInfo> *,std::unordered_set<IL*>> analysisResult_;
        MAP<const CfgNode<ColorInfo> *,std::set<CLiveRange*>> analysisResult_;
        std::map<const CfgNode<ColorInfo> * , const Instruction *> analysis_mapping_;


        void addLR(std::unique_ptr<CLiveRange> && lr);
        //incidence graph
        std::map<const IL *, size_t> searchRanges_; // size_t -> index to liveRanges
        std::map<CLiveRange *, size_t> lrIndex_; //  liveRange to index
//        std::vector<Instruction *> searchInstrs_; // size_t -> index to liveRanges
        std::map<size_t, Instruction *> searchInstrs_; // size_t -> index to liveRanges
        std::vector<std::unique_ptr<CLiveRange>> liveRanges_;
        std::set<std::pair<LiveRange*, size_t>, LiveRangesComparator> rangesAlive_; // size_t -> index to liveRanges
        std::vector<std::set<size_t>> LRincidence_;

        std::map<const Instruction *, size_t> spillIndexes_; //size_t -> index in liveRanges_ //both: 1st where to spill ; 2nd: what to spill
        std::stack<size_t> colorPickingStack_; //size_t -> index in liveRanges_
        std::map<const Instruction * , size_t> colorPickingResult_;
        std::map<size_t , size_t> colorPickingSemiResult_;

        bool programChanged_;
        //create live ranges, and create incidence graph
        bool generateLiveRanges(); // true == assigned without spilling
        bool setColors();

        VirtualRegister getRegToSpill();
        VirtualRegister getReg(const Instruction *currentIns) override;
//
//        VirtualRegister getFReg(const Instruction *currentIns) override;

//        VirtualRegister getLastRegister(const Instruction *currentIns) override;

    };
}
