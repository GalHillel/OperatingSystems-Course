#ifndef STPIPELINE_HPP
#define STPIPELINE_HPP

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include "ActiveObject.hpp"
#include "PrimeCheck.hpp"

class StPipeline
{
public:
    StPipeline(unsigned int N, unsigned int seed) : N_(N), seed_(seed) {}

    void run()
    {
        ActiveObject ao1(&queue1_);
        ActiveObject ao2(&queue2_);
        ActiveObject ao3(&queue3_);
        ActiveObject ao4(&queue4_);

        ao1.bindQueue(&queue1_);
        ao2.bindQueue(&queue2_);
        ao3.bindQueue(&queue3_);
        ao4.bindQueue(&queue4_);

        ao1.createActiveObject();
        ao2.createActiveObject();
        ao3.createActiveObject();
        ao4.createActiveObject();

        srand(seed_);

        for (unsigned int i = 0; i < N_; ++i)
        {
            unsigned int num = generateNumber();
            auto start = std::chrono::high_resolution_clock::now();

            ActiveObject::TaskFunction task1 = [&num, &ao2, &ao3, this]()
            {
                std::cout << num << std::endl;
                std::cout << (isPrime(num) ? "true" : "false") << std::endl;
                num += 11;
                ao2.getQueue()->enqueue([&num, &ao3, this]()
                                        {
                                            std::cout << num << std::endl;
                                            std::cout << (isPrime(num) ? "true" : "false") << std::endl;
                                            num -= 13;
                                            ao3.getQueue()->enqueue([&num, this]()
                                                                    {
                                                                        std::cout << num << std::endl;
                                                                        std::cout << (isPrime(num) ? "true" : "false") << std::endl;
                                                                        num += 2;
                                                                        std::cout << num << std::endl;
                                                                    }); });
            };

            queue1_.enqueue(task1);

            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::this_thread::sleep_for(std::chrono::milliseconds(1) - elapsed);
        }

        // Stop active objects
        ao1.stop();
        ao2.stop();
        ao3.stop();
        ao4.stop();

        // Wait for all threads to finish
        ao1.join();
        ao2.join();
        ao3.join();
        ao4.join();
    }

private:
    unsigned int N_;
    unsigned int seed_;
    Queue<ActiveObject::TaskFunction> queue1_;
    Queue<ActiveObject::TaskFunction> queue2_;
    Queue<ActiveObject::TaskFunction> queue3_;
    Queue<ActiveObject::TaskFunction> queue4_;

    unsigned int generateNumber()
    {
        return rand() % 900000 + 100000;
    }
};

#endif
