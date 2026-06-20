#include "base.hpp"
#include "encrypt.hpp"
#include "config.hpp"

using namespace std;

vector<string> parser_cmd(const string& cmd, char spilit) {
    string tmp = "";
    vector<string> ans;
    for (char c : cmd) {
        if (c == spilit) {
            if (!tmp.empty()) {
                ans.push_back(tmp);
                tmp = "";
            }
        }
        else {
            tmp += c;
        }
    }
    if (!tmp.empty()) ans.push_back(tmp);
    return ans;
}

bool check_key(const string& key) {
    if (key.empty()) return false;
    for (char c : key) {
        if (32 <= c && c <= 126) continue;
        else return false;
    }
    return true;
}

kv_type detectType(const string& input) {
    if (input == "true" || input == "false") return kv_type::Bool;
    if (regex_match(input, regex("^-?\\d+$")))     return kv_type::Int;
    if (regex_match(input, regex("^-?\\d*\\.\\d+$"))) return kv_type::Float;
    if (input.front() == '"' && input.back() == '"') return kv_type::String;
    return kv_type::None;
}

void teajars::set_kv(const tkv& kv) {
    data_map[kv.key] = kv;
}

tkv teajars::get_kv(const string& key) {
    auto it = data_map.find(key);
    if (it != data_map.end()) {
        return it->second;
    }
    return tkv{"", kv_type::None, ""};
}

void teajars::get_kv_type(const string& key) {
    tkv k = get_kv(key);
    //if (k.type == kv_type::None) cout << "key was not find" << endl;
    //else 
    cout << kvtype_str[static_cast<int>(k.type)] << endl;
}

void teajars::del_kv(const string& key) {
    data_map.erase(key);
}

void teajars::clear_all() {
    data_map.clear();
}

void teajars::save(const string& filename) {
    ofstream fout(filename, ios::binary);
    string header = Encrypt::encrypt(
        string("#teajarsKV version ") + TJVERSION, get_encrypt_key()
    );
    int header_length = header.length();
    fout.write((char*)&header_length, sizeof(header_length));
    fout.write(header.c_str(), header_length);
    int total_items = data_map.size();
    fout.write((char*)&total_items, sizeof(total_items));

    for (const auto& pair : data_map) {
        const tkv& item = pair.second;
        string encrypted_key   = Encrypt::encrypt(item.key, get_encrypt_key());
        string encrypted_value = Encrypt::encrypt(item.value, get_encrypt_key());

        int key_length = encrypted_key.length();
        fout.write((char*)&key_length, sizeof(key_length));
        fout.write(encrypted_key.c_str(), key_length);

        int value_length = encrypted_value.length();
        fout.write((char*)&value_length, sizeof(value_length));
        fout.write(encrypted_value.c_str(), value_length);
    }
    fout.close();
}

void teajars::open(const string& filename) {
    ifstream fin(filename, ios::binary);
    int header_length;
    fin.read((char*)&header_length, sizeof(header_length));

    string header;
    header.resize(header_length);
    fin.read(&header[0], header_length);

    if (Encrypt::decrypt(header, get_encrypt_key()) !=
        string("#teajarsKV version ") + TJVERSION) {
        throw runtime_error("open error");
    }

    int total_items;
    fin.read((char*)&total_items, sizeof(total_items));

    for (int i = 0; i < total_items; i++) {
        int key_length;
        fin.read((char*)&key_length, sizeof(key_length));
        string encrypted_key;
        encrypted_key.resize(key_length);
        fin.read(&encrypted_key[0], key_length);

        int value_length;
        fin.read((char*)&value_length, sizeof(value_length));
        string encrypted_value;
        encrypted_value.resize(value_length);
        fin.read(&encrypted_value[0], value_length);

        string decrypted_key   = Encrypt::decrypt(encrypted_key, get_encrypt_key());
        string decrypted_value = Encrypt::decrypt(encrypted_value, get_encrypt_key());

        set_kv(tkv{decrypted_key, detectType(decrypted_value), decrypted_value});
    }
    fin.close();
}

void teajars::set_encrypt_key(const string& key) {
    encrypt_key = key;
}

string teajars::get_encrypt_key() {
    return encrypt_key;
}