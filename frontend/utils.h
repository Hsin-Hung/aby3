#ifndef UTILS_H
#define UTILS_H 

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#define IP_FILE_NAME "ips.txt"

typedef struct
{
    std::string ip0, ip1, ip2;
} Party_IPs;

std::vector<std::pair<std::string, std::vector<int>>>
read_csv(std::string filename);
Party_IPs getPartyIPs();

#endif