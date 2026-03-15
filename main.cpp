#include<iostream>
#include<string>
#include<any>
#include<vector>
#include<fstream>
#include<regex>
#include<unordered_map>

#include "include/encrypt.hpp"
#include "include/httplib.h"
#include "include/json.hpp"

using namespace std;
namespace h = httplib;
using json = nlohmann::json;

#define TJVEROSIN "0.93"

/*还要完成
可以尝试优化，get时尝试去body找body中的keyok

建议的 API 路由结构
svr.Get("/api/v1/status", handle_status);ok           // 系统状态
svr.Get("/api/v1/get", handle_get_value);ok       // 获取单个值
svr.Post("/api/v1/set", handle_set_value);ok        // 设置键值
svr.Delete("/api/v1/del", handle_delete_key);ok     // 删除键
svr.Post("/api/v1/save", handle_save);ok            // 保存数据
svr.Post("/api/v1/load", handle_load);ok            // 加载数据
*/

//函数声明


bool check_key(const string& key){
    if (key.empty()) return false;
    for (char c : key){
        if (32 <= c && c <= 126) continue;
        else return false;
    }
    return true;
}

enum class kv_type{
    Int,
    Float,
    String,
    Bool,
    None
};

string kvtype_str[] = {"Int","Float","String","Bool","None"};

struct tkv{
    string key;
    kv_type type;
    string value;
};

vector<string> parser_cmd(const string& cmd, char spilit = ' '){
    string tmp = "";
    vector<string> ans;
    for (char c : cmd){
        if(c == spilit){
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

kv_type detectType(const string& input) {
    if (input == "true" || input == "false") return kv_type::Bool;
    if (regex_match(input, regex("^-?\\d+$"))) return kv_type::Int;
    if (regex_match(input, regex("^-?\\d*\\.\\d+$"))) return kv_type::Float;
    return kv_type::String;
}

class teajars{
private:
    unordered_map<string,tkv> data_map;
    string encrypt_key = "";
public:
    //string teafilekey = "";
    bool is_debug = false;
    void set_kv(const tkv& kv){
        data_map[kv.key] = kv;
    }

    tkv get_kv(const string& key){
        auto it = data_map.find(key);
        if (it != data_map.end()) {
            return it->second;
        }
        return tkv{"",kv_type::None,""};
    }
    
    void get_kv_type(const string& key){
        tkv k = get_kv(key);
        if (k.type == kv_type::None) cout << "key was not find" << endl;
        else cout << kvtype_str[static_cast<int>(k.type)] << endl;
    }

    void del_kv(const string& key){
        data_map.erase(key);
    }

    void clear_all(){
        data_map.clear();
    }
    
    void save(const string& filename="teajars.kv"){
        ofstream fout(filename,ios::binary);
        string header = Encrypt::encrypt(string("#teajarsKV version ")+TJVEROSIN,get_encrypt_key());
        int header_length = header.length();
        fout.write((char*)&header_length,sizeof(header_length));
        fout.write(header.c_str(),header_length);
        int total_items = data_map.size();
        fout.write((char*)&total_items,sizeof(total_items));

        for(const auto& pair : data_map) {
            const tkv& item = pair.second;
            // 加密键和值
            string encrypted_key = Encrypt::encrypt(item.key,get_encrypt_key());
            string encrypted_value = Encrypt::encrypt(item.value,get_encrypt_key());
            // 写入加密后的键
            int key_length = encrypted_key.length();
            fout.write((char*)&key_length,sizeof(key_length));
            fout.write(encrypted_key.c_str(),key_length);
            // 写入加密后的值
            int value_length = encrypted_value.length();
            fout.write((char*)&value_length,sizeof(value_length));
            fout.write(encrypted_value.c_str(),value_length);
        }
        
        fout.close();
    }

    void open(const string& filename){
        ifstream fin(filename,ios::binary);
        // 读取header长度
        int header_length;
        fin.read((char*)&header_length,sizeof(header_length));
        // 读取header内容
        string header;
        header.resize(header_length);
        fin.read(&header[0],header_length);
        // 解密并验证header
        if (Encrypt::decrypt(header,get_encrypt_key()) != string("#teajarsKV version ")+TJVEROSIN){
            throw runtime_error("open error");
        }
        // 读取数据项总数
        int total_items;
        fin.read((char*)&total_items,sizeof(total_items));
        // 逐个读取数据项
        for(int i = 0; i < total_items; i++) {
            // 读取键的长度和内容
            int key_length;
            fin.read((char*)&key_length,sizeof(key_length));
            string encrypted_key;
            encrypted_key.resize(key_length);
            fin.read(&encrypted_key[0],key_length);
            // 读取值的长度和内容
            int value_length;
            fin.read((char*)&value_length,sizeof(value_length));
            string encrypted_value;
            encrypted_value.resize(value_length);
            fin.read(&encrypted_value[0],value_length);
            // 解密键和值
            string decrypted_key = Encrypt::decrypt(encrypted_key,get_encrypt_key());
            string decrypted_value = Encrypt::decrypt(encrypted_value,get_encrypt_key());
            
            set_kv(tkv{decrypted_key,detectType(decrypted_value),decrypted_value});
        }
        
        fin.close();
    }

    void set_encrypt_key(const string& key){
        encrypt_key = key;
    }

    string get_encrypt_key(){
        return encrypt_key;
    }
};

//全局变量
teajars teakv;


void local_server(){
    string cmd;
    vector<string> pcmd;
    
    cout << "teajarsKV version " << TJVEROSIN << endl << "tkv>";

    while(getline(cin, cmd)){
        if (cmd.empty()){
            cout << "tkv>";
            continue;
        }

        pcmd = parser_cmd(cmd, ' ');

        if (pcmd[0] == "exit") break;

        else if (pcmd[0] == "set" && pcmd.size() >= 3){
            teakv.set_kv(tkv{pcmd[1],detectType(pcmd[2]),pcmd[2]});
        }
        else if (pcmd[0] == "get" && pcmd.size() >= 2){
            tkv k = teakv.get_kv(pcmd[1]);
            if (k.key == "") cout << "key was not find" << endl;
            else cout << k.value << endl;
        }
        else if (pcmd[0] == "type" && pcmd.size() >= 2){
            teakv.get_kv_type(pcmd[1]);
        }
        else if (pcmd[0] == "del" && pcmd.size() >= 2){
            teakv.del_kv(pcmd[1]);
        }
        else if (pcmd[0] == "save"){
            try{
                if (pcmd.size() >= 2) teakv.save(pcmd[1]);
                else teakv.save();
                cout << "save success" << endl;
            }
            catch(...){
                cout << "save error" << endl;
            }
        }
        else if (pcmd[0]=="open" && pcmd.size() >= 2){
            try{
                teakv.open(pcmd[1]);
                cout << "open success" << endl;
            }
            catch(...){
                cout << "open error" << endl;
            }
        }
        else if (pcmd[0] == "debug"){
            teakv.is_debug = true;
            cout << "debug on" << endl;
        }
        else if (pcmd[0] == "encryptkey" && pcmd.size() >= 2){
            if (!check_key(pcmd[1])){
                cout << "key error" << endl;
                continue;
            }
            teakv.set_encrypt_key(pcmd[1]);
        }
        else if (pcmd[0]=="clearall"){
            teakv.clear_all();
        }
        else cout << "command error" << endl;
        
        cout << "tkv>";
    }
    return;
}


void net_server(int port,string& host){
    h::Server svr;
    svr.Get("/",[](const h::Request& req,h::Response& res){
        res.set_file_content("assets/show.html","text/html");
    });
    
    svr.Get("/api/hello",[](const h::Request& req,h::Response& res){
        res.set_file_content("assets/show.html","text/html");
    });

    svr.Get("/api/status",[](const h::Request& req,h::Response& res){
        res.set_content(R"({"status":"200"})","application/json");
    });

    svr.Get("/api/get", [](const h::Request& req, h::Response& res) {
    try {
        string key = req.get_param_value("key");
        if (key.empty()){
            try {
                json req_body = json::parse(req.body);
                if (req_body.contains("key")) {
                    key = req_body["key"];
                }
                else{
                    res.status = 400;
                    res.set_content(R"({"error":"Missing 'key' parameter"})","application/json");
                    return;
                }
            }
            catch (const json::parse_error&){
                res.status = 400;
                res.set_content(R"({"error":"Missing 'key' parameter"})","application/json");
                return;
            }
        }
        if (!check_key(key)){
            res.status = 400;
            res.set_content(R"({"error":"Invalid key format"})","application/json");
            return;
        }

        // 查询数据库
        tkv k = teakv.get_kv(key);
        if (k.key == "") {
            res.status = 404;
            res.set_content(R"({"error":"Key not found"})", "application/json");
        } else {
            json response = {
                {"key", k.key},
                {"value", k.value}
            };
            res.set_content(response.dump(),"application/json");
        }
    }
    catch (const exception& e){
        res.status = 500;
        res.set_content(R"({"error":"Internal server error"})","application/json");
    }
});

    svr.Post("/api/set",[](const h::Request& req,h::Response& res){
        try{
            json req_body = json::parse(req.body);
            if (!req_body.contains("key") || !req_body.contains("value")){
                res.status = 400;
                res.set_content(R"({"error":"Missing 'key' or 'value' field in JSON body"})","application/json");
            }
            string key = req_body["key"];
            string value = req_body["value"];
            if (!check_key(key)){
                res.status = 400;
                res.set_content(R"({"error":"Invalid key format"})","application/json");
                return;
            }
            
            teakv.set_kv(tkv{key,detectType(value),value});
            json success_response = {
                {"status", "200"},
            };
            res.set_content(success_response.dump(), "application/json");
            
        }
        catch (const json::parse_error& e){
            res.status = 400;
            res.set_content(R"({"error":"Invalid JSON format"})","application/json");
        }
        catch (const exception& e){
            res.status = 500;
            res.set_content(R"({"error":"Internal server error"})","application/json");
        }
    });
    
    svr.Delete("/api/del",[](const h::Request& req,h::Response& res){
        try{
            json req_body = json::parse(req.body);
            if (!req_body.contains("key")){
                res.status = 400;
                res.set_content(R"({"error":"Missing 'key' field in JSON body"})","application/json");
                return;
            }
            string key = req_body["key"];
            if (!check_key(key)){
                res.status = 400;
                res.set_content(R"({"error":"Invalid key format"})","application/json");
                return;
            }
            teakv.del_kv(key);
            json success_response = {
                {"status", "200"},
            };
            res.set_content(success_response.dump(), "application/json");
        }
        catch (const exception& e){
            res.status = 500;
            res.set_content(R"({"error":"Internal server error"})","application/json");
        }
    });

    svr.Post("/api/save",[](const h::Request& req,h::Response& res){
        try{
            json req_body = json::parse(req.body);
            if (!req_body.contains("file")){
                res.status = 400;
                res.set_content(R"({"error":"Missing 'file' field in JSON body"})","application/json");
                return;
            }
            string filename = req_body["file"];
            teakv.save(filename);
            res.set_content(R"({"status":"200"})","application/json");
        }
        catch (const exception& e){
            res.status = 500;
            res.set_content(R"({"error":"Internal server error"})","application/json");
        }
    });

    svr.Get("/api/load",[](const h::Request& req,h::Response& res){
        try{
            json req_body = json::parse(req.body);
            if (!req_body.contains("file")){
                res.status = 400;
                res.set_content(R"({"error":"Missing 'file' field in JSON body"})","application/json");
                return;
            }
            if (req_body.contains("clear")){
                if (req_body["clear"] == "true"){
                    teakv.clear_all();
                }
            }
            string filename = req_body["file"];
            teakv.open(filename);
            res.set_content(R"({"status":"200"})","application/json");
        }
        catch (const exception& e){
            res.status = 500;
            res.set_content(R"({"error":"Internal server error"})","application/json");
        }
    });


    svr.Post("/api/set_encrypt_key",[](const h::Request& req,h::Response& res){
        try{
            json req_body = json::parse(req.body);
            if (!req_body.contains("encrypt_key")){
                res.status = 400;
                res.set_content(R"({"error":"Missing 'encrypt_key' field in JSON body"})","application/json");
                return;
            }
            string key = req_body["encrypt_key"];
            if (!check_key(key)){
                res.status = 400;
                res.set_content(R"({"error":"Invalid key format"})","application/json");
                return;
            }
            teakv.set_encrypt_key(key);
            res.set_content(R"({"status":"200"})","application/json");
        }
        catch (const exception& e){
            res.status = 500;
            res.set_content(R"({"error":"Internal server error"})","application/json");
        }
    });


    //http://192.168.0.103:5000
    cout<<"net server start at \nhost:"<<host<<"\nport:"<<port<<endl;
    svr.listen(host,port);
    return;
}


int main(int argc, char* argv[]){
    int kvmode = 0, kvport = 5000;
    string kvhost = "localhost";
    
    if(argc == 1) {
        local_server();
        return 0;
    }
    
    if(argc >= 2 && string(argv[1]) == "net") {
        kvmode = 1;
        
        for(int i = 2; i < argc; i++) {
            if(string(argv[i]) == "-port" && i+1 < argc) {
                kvport = stoi(argv[i+1]);
                i++;
            }
            else if(string(argv[i]) == "-host" && i+1 < argc) {
                kvhost = argv[i+1];
                i++;
            }
        }
        net_server(kvport, kvhost);
        return 0;
    }
    
    local_server();
    return 0;
}