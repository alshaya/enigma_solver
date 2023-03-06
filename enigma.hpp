#ifndef ENIGMA_HPP
#define ENIGMA_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>
#include <array>
#include <algorithm>
#include "common.hpp"
#include "utility.hpp"
#include "components.hpp"
using namespace std;

struct enigma_t {
    // the pairs plugboard
    string plugboard;
    // input going into reflector
    vector<string> rotors;
    // output going out of reflector
    vector<string> rotors_reverse;
    // reflector
    string reflector;
    // rotors notch
    vector<char> notches;
    // wheels position
    vector<char> wheels_pos;
    // rings position
    vector<char> ring_pos;
};

string rotate_rotor_ring(const string& rotor, int ring_pos) {
    string rotated(rotor.size(), ' ');
    for(size_t letter = 0; letter < rotor.size(); letter++) {
        // it should have been the following expression, however, C++ does not have math modulo!
        // (letter - ring_pos) % letters_count
        char rotated_letter = (letter + letters_count - ring_pos) % letters_count;
        char encoded_letter = (rotor[rotated_letter] + ring_pos) % letters_count;
        rotated[letter] = encoded_letter;
    }
    return rotated;
}

enigma_t create_enigma(const enigma_components_t& components, vector<rotor_config_t> rotors_config) {
    enigma_t enigma;
    enigma.plugboard = components.plugboard;
    enigma.reflector = components.reflector;
    for(auto [rotor_number, wheel_pos, ring_pos]: rotors_config) {
        string rotor = rotate_rotor_ring(components.rotors[rotor_number], ring_pos);
        string reverse_rotor = rotate_rotor_ring(components.rotors_reverse[rotor_number], ring_pos);
        enigma.rotors.push_back(rotor);
        enigma.rotors_reverse.push_back(reverse_rotor);
        enigma.notches.push_back(components.notches[rotor_number]);
        enigma.wheels_pos.push_back(wheel_pos);
        enigma.ring_pos.push_back(ring_pos);
    }
    return enigma;
}

void advance_rotors(enigma_t& enigma) {
    array<int, installed_rotors_count> stepping {
        1, // always advance the first rotor one step
        enigma.wheels_pos[0] == enigma.notches[0] || enigma.wheels_pos[1] == enigma.notches[1], // include double stepping possibility
        enigma.wheels_pos[1] == enigma.notches[1]
    };
    for(int p = 0; p < installed_rotors_count; p++) {
        enigma.wheels_pos[p] = (enigma.wheels_pos[p] + stepping[p]) % letters_count;
    }
}

char encode_letter(char letter, const enigma_t& enigma) {
    // normalize letter
    letter = letter - 'A';
    // plugboard
    letter = enigma.plugboard[letter];
    // input through rotors
    for(int r = 0; r < installed_rotors_count; r++) {
        letter = (letter + enigma.wheels_pos[r]) % letters_count;
        letter = enigma.rotors[r][letter];
        // it should have been the following expression, however, C++ does not have math modulo!
        // (letter - enigma.wheels_pos[r]) % letters_count
        letter = (letter + letters_count - enigma.wheels_pos[r]) % letters_count;
    }
    // reflector
    letter = enigma.reflector[letter];
    // output through rotors
    for(int r = installed_rotors_count - 1; r >= 0; r--) {
        letter = (letter + enigma.wheels_pos[r]) % letters_count;
        letter = enigma.rotors_reverse[r][letter];
        // it should have been the following expression, however, C++ does not have math modulo!
        // (letter - enigma.wheels_pos[r]) % letters_count
        letter = (letter + letters_count - enigma.wheels_pos[r]) % letters_count;
    }
    // plugboard reverse
    letter = enigma.plugboard[letter];
    return letter + 'A';
}

string process_secret(const string& secret, enigma_t& enigma) {
    string processed;
    for(char letter : secret) {
        // only capital letters are processed, other letters are added as-is without affecting the state of the enigma
        if(!isupper(letter)) {
            processed.push_back(letter);
            continue;
        }
        advance_rotors(enigma);
        char encoded_letter = encode_letter(letter, enigma);
        processed.push_back(encoded_letter);
    }
    return processed;
}

// based on explaination from https://www.codesandciphers.org.uk/documents/egenproc/page06.htm
string format_processed(const string& processed) {
    unordered_map<string, string> punctuation = {
        {"KLAM", "|"},
        {"ZZ", ","},
        {"YY", "."},
        {"FRAGE", "?"},
        {"FRAGEZ", "?"},
        {"FRAQ", "?"},
        {"X", " "},
        {"Q", "CH"}
    };
    string formatted = processed;
    formatted.erase(remove(formatted.begin(), formatted.end(), ' '), formatted.end());
    for(auto [key, value] : punctuation) {
        replace_all(formatted, key, value);
    }
    return formatted;
}

#endif