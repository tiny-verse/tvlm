#pragma once

#include <unordered_set>
#include "tvlm/tvlm/analysis/lattice/lattice.h"


namespace tvlm{


    template<typename A>
    class PowersetLattice : public Lattice<std::unordered_set<A>>{
    public:
        explicit PowersetLattice(const std::unordered_set<A> & elements )
        :elements_(elements){}

        virtual std::unordered_set<A> top() override{
            return elements_;
        }

        virtual std::unordered_set<A> bot() override{
            return std::unordered_set<A>();
        }

        std::unordered_set<A> lub(const std::unordered_set<A> &x, const std::unordered_set<A> &y) override {
            std::unordered_set<A>res(x);
            res.insert(y.begin(), y.end());
            return res;
        }

    private:
        std::unordered_set<A> elements_;
    };
}
