#include "t86_backend.h"

#include <memory>
#include "t86/instruction.h"
#include "tvlm/codeGeneration/InstructionSelection/SuperNaiveIS.h"
#include "tvlm/codeGeneration/registerAllocation/SuperNaiveRegisterAllocator.h"
#include "tvlm/codeGeneration/registerAllocation/ColoringAllocator.h"
#include "tvlm/codeGeneration/Epilogue.h"

namespace tvlm{

    bool setupColorConfig(){
        tiny::config.setDefaultIfMissing("-colorRegisterAllocation", "1");
        int c;
        try{
        c= stoi(tiny::config.get("-colorRegisterAllocation"));
        }catch (...) {
            return true;
        }
        return c > 0;
    }


    t86_Backend::TargetProgram t86_Backend::compileToTarget(t86_Backend::IL &&il) {

        auto selected =  SuperNaiveIS::translate(std::move(il));
        bool color = setupColorConfig();
        std::unique_ptr<RegisterAllocator> regAllocator;
        std::stringstream ss;
        auto printer = tiny::ASTPrettyPrinter(ss);
        auto index = 0;
        if(color){
            regAllocator = std::make_unique<ColoringAllocator>( std::move(selected));
            selected = std::move(regAllocator->run());
            bool again = regAllocator->changedProgram();
            while(again){
                ss.str("");
                selected.program_->print(printer);
                std::cerr << tiny::color::lightBlue << "IL" << index++  << ":\n" << ss.str() << std::endl;
                selected = std::move(SuperNaiveIS::translate(std::move(selected)));
                auto newregAllocator = ColoringAllocator(std::move(selected));
                selected = std::move(newregAllocator.run());
                again = newregAllocator.changedProgram();

            }
        }else{
            regAllocator = std::make_unique<SuperNaiveRegisterAllocator>( std::move(selected));
            selected = std::move(regAllocator->run());

        }
        ss.str("");
        selected.program_->print(printer);
        std::cerr << tiny::color::lightBlue << "IL" << index++  << ":\n" << ss.str() << std::endl;

        std::cerr << "-----------------------------------------------\n\n\n" << std::endl;
        ss.str("");
        selected.program_->printAlloc(printer);
        std::cerr << tiny::color::lightBlue << "Alloc Print" <<  ":\n" << ss.str() << std::endl;
        auto epiloged = NaiveEpilogue().translate(std::move(selected));
        return epiloged;

    }

} //namespace tvlm


#if (defined TARGET_t86)
int tvlm::t86_Backend::MemoryCellSize = 4;

void tvlm::t86_Backend::tests() {

}
// inBytes
#else
int tvlm::t86_Backend::MemoryCellSize = 1; // inBytes
#endif
