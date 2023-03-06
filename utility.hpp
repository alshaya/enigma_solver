#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <iterator>
#include <algorithm>
#include <tuple>
#include <cmath>
#include "common.hpp"
using namespace std;

vector<permutation_t> rotors_permutations() {
    vector<permutation_t> permutations;
    int perm[3] = {0, 0, 0}; // permutations in base 5 => [0, 1, 2, 3, 4]
    int carry = 0;

    while(!(perm[0] == 4 && perm[1] == 4 && perm[2] == 4)) {
        // calculate next permutation
        perm[0] = perm[0] + 1;
        carry = perm[0] / 5;
        if(carry) {
            perm[0] = 0;
        }
        perm[1] = perm[1] + carry;
        carry = perm[1] / 5;
        if(carry) {
            perm[1] = 0;
        }
        perm[2] = perm[2] + carry;

        // a rotor can be used only once
        if(perm[0] == perm[1] || perm[0] == perm[2] || perm[1] == perm[2]) {
            continue;
        }
        permutations.push_back({perm[0], perm[1], perm[2]});
    }
    return permutations;
}

string load_secret(const string& file_name) {
    ifstream file(file_name);
    string line;
    getline(file, line);
    line.erase(remove(line.begin(), line.end(), ' '), line.end());
    return line;
}

// taken from https://en.cppreference.com/w/cpp/string/basic_string/replace
size_t replace_all(string& inout, string_view what, string_view with)
{
    size_t count = 0;
    for (string::size_type pos{}; inout.npos != (pos = inout.find(what.data(), pos, what.length()));
         pos += with.length(), ++count) {
        inout.replace(pos, what.length(), with.data(), with.length());
    }
    return count;
}

unordered_set<string> load_common_words(const string& file_name) {
    unordered_set<string> common_words;
    ifstream file(file_name);
    string word;
    while(getline(file, word)) {
        transform(word.begin(), word.end(), word.begin(), [](char c) {
            return toupper(c);
        });
        common_words.insert(word);
    }
    return common_words;
}

// replace any letter other than capital letters with space
string remove_punctuation(string text) {
    replace_if(text.begin(), text.end(), [](char c) {
        return !isupper(c);
    }, ' ');
    return text;
}

int score_text(const unordered_set<string>& common_words, const string& text) {
    int score = 0;
    istringstream iss(text);
    vector<string> words{istream_iterator<string>{iss}, istream_iterator<string>{}};
    for(const auto& word: words) {
        if(word.size() <= 2) continue;
        score += common_words.count(word); 
    }
    return score;
}

// <text, score>
vector<tuple<string, int>> top_text(const unordered_set<string>& common_words, const vector<string>& text_list, int threshold_score) {
    vector<tuple<string, int>> filtered_list;
    for(const string& text : text_list) {
        string formatted = remove_punctuation(text);
        int score = score_text(common_words, formatted);
        if(score >= threshold_score) {
            filtered_list.push_back(make_tuple(text, score));
        }
    }
    sort(filtered_list.begin(), filtered_list.end(), [](const auto& left, const auto& right) {
        return get<1>(left) > get<1>(right); // sort desc
    });
    return filtered_list;
}

double index_of_coincidence(const string& text) {
    double ioc = 0.0;
    double N = text.size();
    vector<double> histogram(letters_count, 0.0);
    for(char letter : text) {
        letter = letter - 'A'; // index of capital letter starting at 0
        histogram[letter]++;
    }
    for(double frequency : histogram) {
        ioc += frequency * (frequency - 1.0);
    }
    ioc /= N * (N - 1);
    return ioc;
}

// match, starting position
vector<tuple<string, size_t>> generate_crib_matches(string secret, const string& crib) {
    vector<tuple<string, size_t>> matches;
    secret.erase(remove(secret.begin(), secret.end(), ' '), secret.end()); // remove all spaces
    for(size_t i = 0; i < secret.size() - crib.size(); i++) {
        string possible_match = secret.substr(i, crib.size());
        bool add = true;
        for(size_t c = 0; c < crib.size(); c++) {
            if(crib[c] == possible_match[c]) {
                add = false;
                break;
            }
        }
        if(add) matches.push_back(make_tuple(possible_match, i));
    }
    return matches;
}

// quadgrams file and explanation from: http://practicalcryptography.com/cryptanalysis/text-characterisation/quadgrams/
unordered_map<string, double> load_quadgrams(const string& file_name) {
    unordered_map<string, double> quadgrams;
    ifstream file(file_name);
    string quadgram;
    double count;
    double total_count = 0;
    while(file >> quadgram >> count) {
        quadgrams.emplace(quadgram, count);
        total_count += count;
    }
    for(const auto&[key, value] : quadgrams) {
        quadgrams[key] = log(value / total_count);
    }
    return quadgrams;
}

double score_quadgrams(const string& text, const unordered_map<string, double>& quadgrams) {
    // floor_score is used to represent the score missing quadgrams in the quadgrams map
    // the calculation is log(count/total number of quadgrams)
    // 0.001 is selected arbitrarly to represent the count of missing quadgrams because log(0) is -infinity
    const double floor_score = log(0.001 / quadgrams.size());
    double score = 0.0;
    for(size_t i = 0; i < text.size() - quadgram_size; i++) {
        string quadgram = text.substr(i, quadgram_size);
        if(quadgrams.contains(quadgram)) {
            score += quadgrams.at(quadgram);
        }
        else {
            score += floor_score;
        }
    }
    return score;
}

#endif  