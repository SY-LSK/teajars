#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <regex>
#include <filesystem>

using namespace std;

enum class kv_type{
    Int,
    Float,
    String,
    Bool,
    None
};

inline string kvtype_str[] = {"Int","Float","String","Bool","None"};

struct tkv{
    string key;
    kv_type type;
    string value;
};

class teajars{
private:
    unordered_map<string,tkv> data_map;
    string encrypt_key = "";
public:
    bool is_debug = false;

    void set_kv(const tkv& kv);
    tkv get_kv(const string& key);
    void get_kv_type(const string& key);
    void del_kv(const string& key);
    void clear_all();
    void save(const string& filename="teajars.kv");
    void open(const string& filename);
    void set_encrypt_key(const string& key);
    string get_encrypt_key();
};

vector<string> parser_cmd(const string& cmd, char spilit = ' ');
bool check_key(const string& key);
kv_type detectType(const string& input);
void config_run(filesystem::path file_path,teajars& teakv);