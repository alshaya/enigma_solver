#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <tuple>
#include <vector>
#include <iterator>
#include "common.hpp"
using namespace std;

// The input rotor (etw) is ignored because it maps each character to itself
struct enigma_components_t {
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
};

string parse_plugboard(const string &config_line) 
{
    istringstream iss(config_line);
    string plugboard;
    string config;
    for(char c = 'A'; c <= 'Z'; c++) { // populate with capital letters
        plugboard.push_back(c);
    }
    while(iss >> config) {
        char first = config[0], second = config[1];
        plugboard[first - 'A'] = second;
        plugboard[second - 'A'] = first;
    }
    for(char& c: plugboard) c = c - 'A';
    return plugboard;
}

string parse_rotor(const string& config_line) {
    string rotor = config_line;
    for(char& c: rotor) c = c - 'A';
    return rotor;
}

string generate_rotor_reverse(const string& rotor) {
    string rotor_reverse(rotor.size(), ' ');
    for(size_t i = 0; i < rotor.size(); i++) {
        char c = rotor[i];
        size_t reverse_index = c;
        rotor_reverse[reverse_index] = i;
    }
    return rotor_reverse;
}

vector<char> parse_rotors_notch(const string& config_line) {
    istringstream iss(config_line);
    vector<char> notches{istream_iterator<char>(iss), istream_iterator<char>()};
    for(char& c: notches) c = c - 'A';
    return notches;
}

string parse_reflector(const string& config_line) {
    string reflector = config_line;
    for(char& c: reflector) c = c - 'A';
    return reflector;
}

/*  read the components configuration file (enigma_config). Each line represents a components:
    1) plugboard mapping
    2) rotor 1
    3) rotor 2
    4) rotor 3
    5) rotor 4
    6) rotor 5
    7) reflector
    8) rotor 1 notch [space] rotor 2 notch [space] ...
*/
enigma_components_t load_components(const string& file_name) {
    enigma_components_t components;
    ifstream config_file(file_name);
    string config_line;

    // plugboard
    getline(config_file, config_line);
    components.plugboard = parse_plugboard(config_line);
    // rotors
    for(int r = 0; r < rotors_count; r++) {
        getline(config_file, config_line);
        string rotor = parse_rotor(config_line);
        components.rotors.push_back(rotor);
        components.rotors_reverse.push_back(generate_rotor_reverse(rotor));
    }
    // reflector
    getline(config_file, config_line);
    components.reflector = parse_reflector(config_line);
    // rotors notch
    getline(config_file, config_line);
    components.notches = parse_rotors_notch(config_line);

    return components;
}

#endif  