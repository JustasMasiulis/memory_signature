#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <catch.hpp>
#include <memory_signature.hpp>
#include <random>

inline void fill_garbage(volatile unsigned char gar[0x1000])
{
    for (int i = 0; i < 0x1000; ++i)
        gar[i] = i;
}

inline auto get_random_ptr_in(volatile unsigned char gar[0x1000])
{
    std::random_device                 r;
    std::default_random_engine         rand(r());
    std::uniform_int_distribution<int> uniform_dist(1000, 0x1000 - 100);
    return &gar[0] + uniform_dist(rand);
}

TEST_CASE("memory_signature")
{
    volatile unsigned char garbage[0x1000] = {0};
    fill_garbage(garbage);

    const auto begin = reinterpret_cast<volatile unsigned char *>(&garbage);
    const auto end   = begin + 0x1000;

    SECTION("small signature 1 ? 3 5") {
        const auto real = get_random_ptr_in(garbage);
        real[0] = 1;
        real[1] = 20;
        real[2] = 3;
        real[3] = 5;

        jm::memory_signature signature;
        jm::memory_signature wildcard_sig{"\x1\x2\x3\x5", 0x2};
        jm::memory_signature wildcard_sig2({1, 2, 3, 5}, 2);
        jm::memory_signature mask_sig("\x1\x2\x3\x5", "x?xx");
        jm::memory_signature mask_sig2({1, 2, 3, 5}, {1, 0, 1, 1}, 0);
        jm::memory_signature ida_sig("1 ? 3 5");
        jm::memory_signature ida_sig2("01 ?? 3 5");

        CHECK(real == wildcard_sig.find(begin, end));
        CHECK(real == wildcard_sig2.find(begin, end));
        CHECK(real == mask_sig.find(begin, end));
        CHECK(real == mask_sig2.find(begin, end));
        CHECK(real == ida_sig.find(begin, end));
        CHECK(real == ida_sig2.find(begin, end));
    }

    SECTION("medium sized signature 01 ?? 36 54 ?? 12 ?? 56 ?? ?? ?? 89") {
        const auto real = get_random_ptr_in(garbage);
        real[0]  = 1;
        real[1]  = 54;
        real[2]  = 0x36;
        real[3]  = 0x54;
        real[4]  = 1;
        real[5]  = 0x12;
        real[6]  = 3;
        real[7]  = 0x56;
        real[8]  = 1;
        real[9]  = 20;
        real[10] = 3;
        real[11] = 0x89;

        jm::memory_signature signature;
        jm::memory_signature wildcard_sig{"\x1\xff\x36\x54\xff\x12\xff\x56\xff\xff\xff\x89", 0xff};
        jm::memory_signature wildcard_sig2({0x1, 0, 0x36, 0x54, 0, 0x12, 0, 0x56, 0, 0, 0, 0x89}, 0);
        jm::memory_signature mask_sig("\x1\xff\x36\x54\xff\x12\xff\x56\xff\xff\xff\x89", "x?xx?x?x???x");
        jm::memory_signature mask_sig2({0x1, 0, 0x36, 0x54, 0, 0x12, 0, 0x56, 0, 0, 0, 0x89}, {1, 0, 1, 1, 0, 1, 0, 1, 0
                                                                                               , 0, 0, 1}, 0);
        jm::memory_signature ida_sig("1 ? 36 54 ? 12 ? 56 ? ? ?? 89");
        jm::memory_signature ida_sig2("01 ?? 36 54 ?? 12 ?? 56 ?? ?? ?? 89");

        CHECK(real == wildcard_sig.find(begin, end));
        CHECK(real == wildcard_sig2.find(begin, end));
        CHECK(real == mask_sig.find(begin, end));
        CHECK(real == mask_sig2.find(begin, end));
        CHECK(real == ida_sig.find(begin, end));
        CHECK(real == ida_sig2.find(begin, end));
    }

}