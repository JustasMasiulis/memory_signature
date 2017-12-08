#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <catch.hpp>
#include <memory_signature.hpp>
#include <random>

constexpr static std::size_t fake_memory_size = 0x10000;

inline int get_rand_idx()
{
    std::random_device                 r;
    std::mt19937                       rand(r());
    std::uniform_int_distribution<int> uniform_dist(1024, fake_memory_size - 1024 * 4);
	return uniform_dist(rand);
}

#define CREATE_TEST_MEMORY                                               \
std::unique_ptr<std::uint8_t[]> mem(new std::uint8_t[fake_memory_size]); \
for (int i = 0; i < fake_memory_size; ++i)                               \
	mem[i] = static_cast<std::uint8_t>(i % 256);                         \
auto* begin    = mem.get();	                                             \
const auto end = begin + fake_memory_size;


TEST_CASE("simple searches")
{
	CREATE_TEST_MEMORY

	SECTION("small signature 1 ? 3 5") {
		const auto real = begin + get_rand_idx();
		real[0] = 1;
		real[1] = 20;
		real[2] = 3;
		real[3] = 5;

		jm::memory_signature wildcard_sig({ 1, 2, 3, 5 }, 2);
		jm::memory_signature mask_sig({ 1, 2, 3, 5 }, "x?xx");
		jm::memory_signature mask_sig2({ 1, 2, 3, 5 }, { 1, 0, 1, 1 });
		jm::memory_signature ida_sig("1 ? 3 5");
		jm::memory_signature ida_sig2("01 ?? 3 5");

		CHECK(real == wildcard_sig.find(begin, end));
		CHECK(real == mask_sig.find(begin, end));
		CHECK(real == mask_sig2.find(begin, end));
		CHECK(real == ida_sig.find(begin, end));
		CHECK(real == ida_sig2.find(begin, end));
	}

	SECTION("medium sized signature 01 ?? 36 54 ?? 12 ?? 56 ?? ?? ?? 89") {
		const auto real = begin + get_rand_idx();
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

		jm::memory_signature wildcard_sig({ 0x1, 0, 0x36, 0x54, 0, 0x12, 0, 0x56, 0, 0, 0, 0x89 }, 0);
		jm::memory_signature mask_sig({ 0x1, 0, 0x36, 0x54, 0, 0x12, 0, 0x56, 0, 0, 0, 0x89 }, "x?xx?x?x???x");
		jm::memory_signature mask_sig2({ 0x1, 0, 0x36, 0x54, 0, 0x12, 0, 0x56, 0, 0, 0, 0x89 }, { 1, 0, 1, 1, 0, 1, 0, 1, 0
			, 0, 0, 1 }, 0);
		jm::memory_signature ida_sig("1 ? 36 54 ? 12 ? 56 ? ? ?? 89");
		jm::memory_signature ida_sig2("01 ?? 36 54 ?? 12 ?? 56 ?? ?? ?? 89");

		CHECK(real == wildcard_sig.find(begin, end));
		CHECK(real == mask_sig.find(begin, end));
		CHECK(real == mask_sig2.find(begin, end));
		CHECK(real == ida_sig.find(begin, end));
		CHECK(real == ida_sig2.find(begin, end));
	}
}

TEST_CASE("constructors") {
	CREATE_TEST_MEMORY

	const auto real = begin + get_rand_idx();
	real[0] = 6;
	real[1] = 20;
	real[2] = 2;
	real[3] = 1;

	jm::memory_signature signature;

	SECTION("copy") {
		jm::memory_signature sig1({ 6, 2, 2, 1 }, { 1, 0, 1, 1 }, 0);
		jm::memory_signature sig2("6 ? 2 1");
		jm::memory_signature sig3(sig1);
		jm::memory_signature sig4(sig2);

		CHECK(real == sig1.find(begin, end));
		CHECK(real == sig2.find(begin, end));

		CHECK(real == sig3.find(begin, end));
		CHECK(real == sig4.find(begin, end));
	}

	SECTION("move") {
		jm::memory_signature sig1({ 6, 2, 2, 1 }, { 1, 0, 1, 1 }, 0);
		jm::memory_signature sig2("6 ? 2 1");
		jm::memory_signature sig3(std::move(sig1));
		jm::memory_signature sig4(std::move(sig2));

		CHECK(real == sig3.find(begin, end));
		CHECK(real == sig4.find(begin, end));
	}
}

TEST_CASE("assignment") {
	CREATE_TEST_MEMORY
	INFO("begins at " << reinterpret_cast<std::uintptr_t>(begin) << '\n');
	INFO("ends at " << reinterpret_cast<std::uintptr_t>(end) << '\n');

	const auto real = begin + get_rand_idx();
	real[0] = 0x33;
	real[1] = 20;
	real[2] = 0x44;
	real[3] = 0x55;

	SECTION("copy") {
		const jm::memory_signature sig1({ 0x33, 2, 0x44, 0x55 }, { 1, 0, 1, 1 }, 0);
		const jm::memory_signature sig2("33 ? 44 55");
		jm::memory_signature sig3("12");
		sig3 = sig1;
		jm::memory_signature sig4({ 1, 2 }, "x?");
		sig4 = sig2;

		CHECK(real == sig1.find(begin, end));
		CHECK(real == sig2.find(begin, end));

		CHECK(real == sig3.find(begin, end));
		CHECK(real == sig4.find(begin, end));
	}

	SECTION("move") {
		jm::memory_signature sig1({ 0x33, 2, 0x44, 0x55 }, { 1, 0, 1, 1 }, 0);
		jm::memory_signature sig2("33 ? 44 55");
		jm::memory_signature sig3("12");
		jm::memory_signature sig4({ 1, 2 }, "x?");
		sig3 = std::move(sig1);
		sig4 = std::move(sig2);

		CHECK(real == sig3.find(begin, end));
		CHECK(real == sig4.find(begin, end));
	}
}
