#include "server.hpp"
#include "httplib.h"
#include "encrypt.hpp"
#include "asio_ip.hpp"
#include "json.hpp"
#include "config.hpp"
#include "base.hpp"



void local_server(teajars& teakv){
    string cmd;
    vector<string> pcmd;
    
    cout << "teajarsKV version " << TJVERSION << endl << "tkv>";

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
        else if (pcmd[0]=="data"){
            cout<<"version:"<<TJVERSION<<endl;
            cout<<TJDATA<<endl;
        }
        else cout << "command error" << endl;
        
        cout << "tkv>";
    }
    return;
}



void net_server(int port,string& host,teajars& teakv){
    h::Server svr;
    svr.Get("/",[](const h::Request& req,h::Response& res){
        res.set_file_content("assets/show.html","text/html");
    });
    
    svr.Get("/api/version",[&](const h::Request& req,h::Response& res){
        json response = {
            {"version",TJVERSION},
            {"data",TJDATA}
        };
        res.set_content(response.dump(),"application/json");
    });

    svr.Get("/api/status",[](const h::Request& req,h::Response& res){
        res.set_content(R"({"status":"200"})","application/json");
    });

    svr.Get("/api/version",[](const h::Request& req,h::Response& res){
        json response = {
            {"version", TJVERSION}
        };
        res.set_content(response.dump(),"application/json");
    });

    svr.Get("/api/version",[](const h::Request& req,h::Response& res){
        json response = {
            {"version", TJVERSION}
        };
        res.set_content(response.dump(),"application/json");
    });

    svr.Get("/api/get", [&](const h::Request& req,h::Response& res) {
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

    svr.Post("/api/set",[&](const h::Request& req,h::Response& res){
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
    
    svr.Delete("/api/del",[&](const h::Request& req,h::Response& res){
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

    svr.Post("/api/save",[&](const h::Request& req,h::Response& res){
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

    svr.Get("/api/load",[&](const h::Request& req,h::Response& res){
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


    svr.Post("/api/set_encrypt_key",[&](const h::Request& req,h::Response& res){
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
    
    string local_ip = getLocalIP();

    cout<<"teajarsKV version "<<TJVERSION<<endl;
    cout<<"net server start at \nhost: "<<local_ip<<"\nport: "<<port<<endl;
    cout<<"local url: http://"<<local_ip<<":"<<port<<endl;
    
    svr.listen(getLocalIP(),port);
    return;
}