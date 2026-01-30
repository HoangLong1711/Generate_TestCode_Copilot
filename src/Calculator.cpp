
#include "Calculator.hpp"

/**
 * Hàm complexAdd tinh chỉnh để test Coverage:
 * - C0 (Statement): Chạy qua mọi dòng.
 * - C1 (Branch): Đi vào cả nhánh if và nhánh else.
 * - C2 (Condition): Test các tổ hợp của (a > 0) và (b > 0).
 */
// int Calculator::complexAdd(int a, int b)
// {
//     int result = 0;

//     // Logic phức tạp hơn để gen coverage C1/C2
//     if (a > 0 || b > 0)
//     {
//         if (a + b < 100)
//         {
//             result = a + b;
//         }
//         else
//         {
//             result = 100;
//         }
//     }
//     else
//     {
//         result = -1;
//     }

//     return result;
// }

int Calculator::addThree(int a, int b, int c)
{
    int result = 0;

    // call addTwo on an instance because it's an instance method
    result = a + math.addTwo(b, c);

    return result;
}

// int Calculator::addFour(int a, int b, int c, int d)
// {
//     int result = 0;

//     // call addThree on an instance because it's an instance method
//     result = a + Calculator::addThree(b, c, d);

//     return result;
// }