#ifndef CALCULATOR_HPP
#define CALCULATOR_HPP

#include "Math.hpp"

class Calculator
{
public:
    // Chỉ khai báo hàm ở đây
    int complexAdd(int a, int b);

    int addThree(int a, int b, int c);

    mt::Math math;

    int addFour(int a, int b, int c, int d);
};

#endif