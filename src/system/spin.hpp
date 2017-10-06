#pragma once

/*
 * class Spin
 */

#include <numeric>
#include <vector>


enum SPINTYPE{UP=2,DOWN=0};   // define possible Spin types


class Spin
{
    typedef Spin selftype;
    typedef std::vector<std::reference_wrapper<selftype> > neighbours_vec;

    const unsigned int  ID;
    SPINTYPE            type;
    neighbours_vec      neighbours {};

public:
    Spin(const unsigned int, SPINTYPE);
    inline auto get_ID()   const { return ID; }
    inline auto get_type() const { return type; }

    inline void set_type(SPINTYPE _t) { type = _t; }
    inline void set_neighbours(const neighbours_vec& _N) { neighbours = _N; }

    inline void flip() { type = ( type == UP ? DOWN : UP ); }

    template<SPINTYPE T> constexpr int num() const;
    template<SPINTYPE T> constexpr int num_signed() const;

    inline auto begin()   { return std::begin(neighbours); }
    inline auto begin()   const { return std::cbegin(neighbours); }
    inline auto cbegin()  const { return std::cbegin(neighbours); }
    inline auto end()     { return std::end(neighbours); }
    inline auto end()     const { return std::cend(neighbours); }
    inline auto cend()    const { return std::cend(neighbours); }

};

inline Spin::Spin(const unsigned int _id, SPINTYPE _type)
 : ID (_id)
 , type (_type)
{}



template<SPINTYPE T>
constexpr inline int Spin::num() const
{
    // return sum over neighbours of type T
    return std::accumulate(std::begin(neighbours), std::end(neighbours), 0, [](int i, const selftype& N)
                {
                    return N.get_type() == T ? i+1 : i;
                });
}



template<SPINTYPE T>
constexpr inline int Spin::num_signed() const
{
    // return signed sum over neighbours of type T
    // sign = + if T == type, sign = - if T != type
    const int num = std::accumulate(std::begin(neighbours), std::end(neighbours), 0,[](int i, const selftype& N)
                    {
                        return N.get_type() == T ? i+1 : i;
                    });
    return T == type ? num : num*(-1);
}