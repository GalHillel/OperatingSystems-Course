#ifndef PRIMECHECK_HPP
#define PRIMECHECK_HPP

bool isPrime(unsigned int num)
{
    if (num < 2)
        return false;
    if (num == 2 || num == 3)
        return true;
    if (num % 2 == 0 || num % 3 == 0)
        return false;
    for (unsigned int i = 5; i * i <= num; i += 6)
    {
        if (num % i == 0 || num % (i + 2) == 0)
            return false;
    }
    return true;
}

#endif // PRIMECHECK_HPP
