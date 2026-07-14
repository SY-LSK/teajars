#pragma once

#include <string>
#include "base.hpp"

using namespace std;

void local_server(teajars& teakv);
void net_server(int port,string& host,teajars& teakv);