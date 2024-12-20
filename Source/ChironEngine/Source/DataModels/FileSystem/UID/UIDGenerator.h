#pragma once

#include "UID.h"

namespace Chiron
{
    class UIDGenerator
    {
    public:
        ~UIDGenerator();

        static UID GenerateUID();

    private:
        UIDGenerator();

        std::mt19937_64 SeededRandomEngine();

    private:

        static std::unique_ptr<UIDGenerator> _instance;
        friend std::unique_ptr<UIDGenerator> std::make_unique<UIDGenerator>();

        std::mt19937_64 _generator;
        std::uniform_int_distribution<UID> _uniforDistribution;
    };
}
