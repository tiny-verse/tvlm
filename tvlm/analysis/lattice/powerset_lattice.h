#pragma once

#include <unordered_set>
#include "tvlm/tvlm/analysis/lattice/lattice.h"


namespace tvlm{


    template<typename A>
    class PowersetLattice : public Lattice<std::unordered_set<A>>{
    public:
        explicit PowersetLattice(const std::unordered_set<A> & elements )
        :elements_(elements){}

        virtual std::unordered_set<A> top() override;

        virtual std::unordered_set<A> bot() override;

        std::unordered_set<A> lub(const std::unordered_set<A> &x, const std::unordered_set<A> &y) override;

    private:
        std::unordered_set<A> elements_;
    };


    template<typename A>
    std::unordered_set<A> PowersetLattice<A>::lub(const std::unordered_set<A> &x, const std::unordered_set<A> &y)  {
        std::unordered_set<A>res(x);
        res.insert(y.begin(), y.end());
        return res;
    }

    template<typename A>
    std::unordered_set<A> PowersetLattice<A>::bot(){
        return std::unordered_set<A>();
    }

    template<typename A>
    std::unordered_set<A> PowersetLattice<A>::top() {
        return elements_;
    }

}
