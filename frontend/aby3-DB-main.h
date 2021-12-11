#include <cryptoTools/Common/CLP.h>
#include <string>
#include <vector>


void DB_Intersect(std::pair<std::vector<std::pair<std::string, std::vector<int>>>, 
std::vector<std::pair<std::string, std::vector<int>>>> data, oc::u32 rows, oc::u32 cols = 0, bool sum = false, int rank = 0, std::string ip0 = "127.0.0.0", std::string ip1 = "127.0.0.0", std::string ip2 = "127.0.0.0");
void DB_cardinality(oc::u32 rows);
void DB_threat(oc::u32 rows, oc::u32 setCount);
oc::i64 Sh3_add_test(oc::u64 n);
