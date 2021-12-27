#include <cryptoTools/Common/CLP.h>
#include <string>
#include <vector>
#include "utils.h"


void DB_Intersect(std::pair<std::vector<std::pair<std::string, std::vector<int>>> , 
std::vector<std::pair<std::string, std::vector<int>>>> data, oc::u32 rows, int rank, Party_IPs party_ips, oc::u32 cols = 0, bool sum = false, bool debug = false);
void DB_cardinality(oc::u32 rows);
void DB_threat(oc::u32 rows, oc::u32 setCount);
oc::i64 Sh3_add_test(oc::u64 n);
