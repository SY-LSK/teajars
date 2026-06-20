#include "src/base.hpp"
#include "src/config.hpp"
#include "src/server.hpp"

int main(int argc, char* argv[]){
    teajars teakv;

    int kvmode = 0, kvport = 5000;
    string kvhost = "localhost";
    
    if(argc == 1) {
        local_server(teakv);
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
        net_server(kvport,kvhost,teakv);
        return 0;
    }
    
    local_server(teakv);
    return 0;
}