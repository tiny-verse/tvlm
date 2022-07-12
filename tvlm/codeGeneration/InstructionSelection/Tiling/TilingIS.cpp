#include "TilingIS.h"
#include "IlTiler.h"


namespace tvlm {


    tiny::t86::Program TilingIS::translate(ILBuilder &ilb) {
        TilingIS g;
        // tile
            ILTiler tiler;
            tiler.tile(ilb);
        // select best

        // export program

        return tiny::t86::Program();
    }
}