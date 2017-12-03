# memory_signature [![Build Status](https://travis-ci.org/JustasMasiulis/memory_signature.svg?branch=master)](https://travis-ci.org/JustasMasiulis/memory_signature) [![Build status](https://ci.appveyor.com/api/projects/status/fa5krpuii0s5535i?svg=true)](https://ci.appveyor.com/project/JustasMasiulis/memory-signature)
A light wrapper class that accepts numerous types of memory signature types and an easy way to search for them

## installation
the library requires c++11 and is headers only so copying it into your project directory and including it is enough.

## small example
```c++
// all of these are equalient
jm::memory_signature signature;
jm::memory_signature wildcard_sig({1, 2, 3, 5}, 2);

jm::memory_signature mask_sig({1, 2, 3, 5}, "x?xx");
jm::memory_signature mask_sig2({1, 2, 3, 5}, {1, 0, 1, 1});

jm::memory_signature ida_sig("1 ? 3 5");
jm::memory_signature ida_sig2("01 ?? 03 05");

// to search for the pattern the class has .find member function that returns iterator to the first occurence.
// there are 2 choices for it - one that accepts 2 iterators and one that accepts a container / range.
wildcard_sig.find(search_range_begin, search_range_end);
wildcard_sig.find(search_range);
```
