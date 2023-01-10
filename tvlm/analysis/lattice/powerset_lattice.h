#pragma once

#include <unordered_set>
#include <set>
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

    template<typename A, class Hash = std::less<A> >
    class CPowersetLattice : public Lattice<std::set<A, Hash>>{
    public:
        explicit CPowersetLattice(const std::set<A, Hash> & elements )
                :elements_(elements){}

        virtual std::set<A, Hash> top() override;

        virtual std::set<A, Hash> bot() override;

        std::set<A, Hash> lub(const std::set<A, Hash> &x, const std::set<A, Hash> &y) override;

    private:
        std::set<A, Hash> elements_;
    };


    template<typename A, class Hash>
    std::set<A, Hash> CPowersetLattice<A, Hash>::lub(const std::set<A, Hash> &x, const std::set<A, Hash> &y)  {
        std::set<A, Hash>res(x);
        std::set<A, Hash>tmp(y);
        res.insert(tmp.begin(), tmp.end());
        return res;
    }

    template<typename A, class Hash>
    std::set<A, Hash> CPowersetLattice<A, Hash>::bot(){
        return std::set<A, Hash>();
    }

    template<typename A, class Hash>
    std::set<A, Hash> CPowersetLattice<A, Hash>::top() {
        return elements_;
    }

}
