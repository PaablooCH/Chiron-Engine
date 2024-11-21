#include "Pch.h"
#include "UIDGenerator.h"

namespace Chiron
{
    std::unique_ptr<UIDGenerator> UIDGenerator::_instance = nullptr;

    UIDGenerator::~UIDGenerator()
    {
    }

    UID UIDGenerator::GenerateUID()
    {
        if (_instance == nullptr)
        {
            _instance = std::make_unique<UIDGenerator>();
        }
        return _instance->_uniforDistribution(_instance->_generator);
    }

    UIDGenerator::UIDGenerator()
    {
        _generator = SeededRandomEngine();
    }

    std::mt19937_64 UIDGenerator::SeededRandomEngine()
    {
        std::random_device source;
        auto n = std::mt19937_64::state_size * sizeof(std::mt19937_64::result_type);
        auto random_data = std::views::iota(std::size_t(), (n - 1) / sizeof(source()) + 1)
            | std::views::transform([&](auto) { return source(); });
        std::seed_seq seeds(std::begin(random_data), std::end(random_data));
        return std::mt19937_64(seeds);
    }
}

