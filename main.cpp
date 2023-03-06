/*  This solution assumes the Enigma machine is Enigma I with 3 out of 5 rotors.
    In addition, the plugboard configuration is given in the challenge.
    Also, the given reflector is UKW-C.

    Thanks to: https://summersidemakerspace.ca/projects/enigma-machine/ for the amazing simulator.

    Special thanks go to http://practicalcryptography.com/cryptanalysis/breaking-machine-ciphers/cryptanalysis-enigma/
    for the amazing explanation of how to break ENIGMA encryption. After many trials, I could not break it.
    Once I understood the method described I learned about fitness functions, statistical analysis of text and
    most important how crack ENIGMA!
    
*/

#include <iostream>
#include <string>
#include <tuple>
#include <vector>
#include <iomanip>
#include "utility.hpp"
#include "components.hpp"
#include "enigma.hpp"
using namespace std;

void keep_top_n(vector<quadgrams_score_t>& scored_list, size_t n) {
    sort(scored_list.begin(), scored_list.end(), [](const auto& left, const auto& right) {
        return get<1>(left) > get<1>(right); // sort desc by score
    });
    scored_list.erase(scored_list.begin() + n, scored_list.end()); // keep the top N only
}

void print(vector<quadgrams_score_t>& scored_list) {
    for(auto [text, score, rotors_config] : scored_list) {
        // format as capital characters for indicators and rings
        auto [rotor_1, rotor_1_pos, rotor_1_ring] = rotors_config[0];
        auto [rotor_2, rotor_2_pos, rotor_2_ring] = rotors_config[1];
        auto [rotor_3, rotor_3_pos, rotor_3_ring] = rotors_config[2];
        rotor_1_pos += 'A'; rotor_1_ring += 'A';
        rotor_2_pos += 'A'; rotor_2_ring += 'A';
        rotor_3_pos += 'A'; rotor_3_ring += 'A';

        cout << "[rotors=" << (int)rotor_3 << (int)rotor_2 << (int)rotor_1 << "] "
             << "[indicators=" << rotor_3_pos << rotor_2_pos << rotor_1_pos <<  "] "
             << "[rings=" << rotor_3_ring << rotor_2_ring << rotor_1_ring << "] "
             << "[score=" << setw(8) << setprecision(2) << fixed <<  score <<  "] "
             << "[" << text << "]"
             << endl;
    }
}

int main() {
    enigma_components_t components = load_components("config/enigma_config.txt");
    vector<permutation_t> permutations = rotors_permutations();
    string secret = load_secret("config/secret.txt");
    unordered_map<string, double> quadgrams = load_quadgrams("config/english_quadgrams.txt");
    vector<quadgrams_score_t> scored_indicators;
    vector<quadgrams_score_t> scored_rings;
    const size_t top_n = 5;

    // try all rotors permutations
    for(size_t i = 0; i < permutations.size(); i++) {
        // try all rotors positions
        for(size_t j = 0; j < letters_count; j++) {
            for(size_t k = 0; k < letters_count; k++) {
                for(size_t n = 0; n < letters_count; n++) {
                    auto [rotor_1, rotor_2, rotor_3] = permutations[i];
                    vector<rotor_config_t> rotors_config = {
                        {rotor_1, n, 0},
                        {rotor_2, k, 0},
                        {rotor_3, j, 0}
                    };
                    enigma_t enigma = create_enigma(components, rotors_config);
                    string processed = process_secret(secret, enigma);
                    double score = score_quadgrams(processed, quadgrams);
                    scored_indicators.push_back({processed, score, rotors_config});
                }
            }
        }
    }

    keep_top_n(scored_indicators, top_n); // keep top 5 only in the list and discard the rest

    for(auto [text, score, rotors_config] : scored_indicators) {
        // try all rings positions
        for(size_t j = 0; j < letters_count; j++) {
            for(size_t k = 0; k < letters_count; k++) {
                for(size_t n = 0; n < letters_count; n++) {
                    auto& [rotor_1, rotor_1_pos, rotor_1_ring] = rotors_config[0];
                    auto& [rotor_2, rotor_2_pos, rotor_2_ring] = rotors_config[1];
                    auto& [rotor_3, rotor_3_pos, rotor_3_ring] = rotors_config[2];
                    rotor_1_ring = n;
                    rotor_2_ring = k;
                    rotor_3_ring = j;
                    // adjust the notches only and start with the same decoded letter
                    rotor_1_pos = (rotor_1_pos + rotor_1_ring) % letters_count;
                    rotor_2_pos = (rotor_2_pos + rotor_2_ring) % letters_count;
                    rotor_3_pos = (rotor_3_pos + rotor_3_ring) % letters_count;
                    enigma_t enigma = create_enigma(components, rotors_config);
                    string processed = process_secret(secret, enigma);
                    double score = score_quadgrams(processed, quadgrams);
                    scored_rings.push_back({processed, score, rotors_config});
                }
            }
        }
    }

    keep_top_n(scored_rings, top_n); // keep top 5 only in the list and discard the rest

    print(scored_rings);

    return EXIT_SUCCESS;
}