#pragma once

namespace tvlm{


    template< typename A>
    class Lattice {
    public:
        /** Returns the ⊤ element*/
        virtual A top() = 0;
        /** Returns the ⊥ element*/
        virtual A bot() = 0;
        /** Return the least upper bound ⨆{x,y} (x ⊔ y) */
        virtual A lub(A &x, A  &y) = 0;
    };
} //namespace tvlm
