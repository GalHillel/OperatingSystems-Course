#ifndef PRIMECHECK_HPP
#define PRIMECHECK_HPP

#include <cmath>

bool isPrime(unsigned int num)
{
    if (num <= 1)
    {
        return false;
    }

    unsigned int sqrtNum = static_cast<unsigned int>(std::sqrt(num));
    for (unsigned int i = 2; i <= sqrtNum; ++i)
    {
        if (num % i == 0)
        {
            return false;
        }
    }

    return true;
}

#endif
