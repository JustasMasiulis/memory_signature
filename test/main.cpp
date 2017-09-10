#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <catch.hpp>
#include <memory_signature.hpp>
#include <random>

inline void fill_garbage(volatile unsigned char gar[0x1000])
{
    for (int i = 0; i < 0x1000; ++i)
        gar[i] = i;
}

inline auto gen_sig(volatile unsigned char gar[0x1000])
{
    std::random_device                 r;
    std::default_random_engine         rand(r());
    std::uniform_int_distribution<int> uniform_dist(1000, 0x1000);
    auto                               ptr = &gar[0] + uniform_dist(rand);
    ptr[0] = 1;
    ptr[1] = 20;
    ptr[2] = 3;
    ptr[3] = 5;
    return ptr;
}

TEST_CASE("memory_signature")
{
    volatile unsigned char garbage[0x1000] = {0};
    fill_garbage(garbage);

    const auto begin = reinterpret_cast<volatile unsigned char *>(&garbage);
    const auto end   = begin + 0x1000;
    const auto real  = gen_sig(garbage);

    jm::memory_signature signature;
    jm::memory_signature wildcard_sig{"\x1\x2\x3\x5", 0x2};
    jm::memory_signature wildcard_sig2({1, 2, 3, 5}, 2);
    jm::memory_signature mask_sig("\x1\x2\x3\x5", "x?xx");
    jm::memory_signature mask_sig2({1, 2, 3, 5}, {1, 0, 1, 1}, 0);
    jm::memory_signature ida_sig("1 ? 3 5");

    CHECK(real == wildcard_sig.find(begin, end));
    CHECK(real == wildcard_sig2.find(begin, end));
    CHECK(real == mask_sig.find(begin, end));
    CHECK(real == mask_sig2.find(begin, end));
    CHECK(real == ida_sig.find(begin, end));
}