#include "src/base.hpp"
#include "src/config.hpp"
#include "src/server.hpp"

teajars teakv;
int main(int argc, char* argv[]){

    int kvmode = 0, kvport = 5000;
    string kvhost = "localhost";

    if(argc == 1) {
        if (filesystem::exists("tjcfg.json")) {
            config_run(filesystem::path("tjcfg.json"),teakv);
        }
        else if (filesystem::exists("config.json")) {
            config_run(filesystem::path("config.json"),teakv);
        }
        else {
            local_server(teakv);
            return 0;
        }
    }
 
    else if(argc >= 2) {
        if (string(argv[1]) == "net") {
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
            net_server(kvport,kvhost,teakv);
            return 0;
        }

        else if (string(argv[1]) == "-config" && argc >= 3) {
            if (filesystem::exists(argv[2]) && filesystem::is_regular_file(argv[2])) {
                config_run(filesystem::path(argv[2]),teakv);
                return 0;
            }
            else {
                cout<<"Error: Invalid config file"<<endl;
                return 0;
            }

        }
        else if (string(argv[1]) == "local") {
            local_server(teakv);
            return 0;
        }

        if (string(argv[1]) != "help") cout<<"Error: Invalid command"<<endl;
        cout<<"teajarsKV version "<<TJVERSION<<endl;
        cout<<TJDATA<<endl;
        cout<<"Usage: teajarsKV [mode: net/local] [-port port] [-host host]"<<endl;
    }
    return 0;
}