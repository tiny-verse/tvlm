#pragma once

#include <set>
#include "tvlm/tvlm/analysis/lattice/lattice.h"


namespace tvlm{


    template<typename A>
    class PowersetLattice : public Lattice<std::set<A>>{
    public:
        explicit PowersetLattice(std::set<A> & elements )
        :elements_(elements){}

        virtual std::set<A> top() override{
            return elements_;
        }

        virtual std::set<A> bot() override{
            return std::set<A>();
        }

        virtual std::set<A> lub( std::set<A> & x, std::set<A> & y) override{
            std::set<A>res(x);
            res.insert(y.begin(), y.end());
            return res;
        }

    private:
        std::set<A> elements_;
    };
}
