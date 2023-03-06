#ifndef COMMON_HPP
#define COMMON_HPP

#include <tuple>
#include <vector>
using namespace std;

// define permutation type tuple<fast rotor, middle rotor, slow rotor>
using permutation_t = tuple<int, int, int>;

// <rotor number, wheel position, ring position>
using rotor_config_t = tuple<char, char, char>;

// quadgrams scoring type
using quadgrams_score_t = tuple<string, double, vector<rotor_config_t>>;

// number of rotors to choose from
const int rotors_count = 5;

// number of rotors installed in the machine
const int installed_rotors_count = 3;

// capital letters count
const int letters_count = 26;

// 4 letters quadgram size
const size_t quadgram_size = 4;


#endif