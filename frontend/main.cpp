
#include <cryptoTools/Common/CLP.h>

#include "aby3_tests/aby3_tests.h"
#include "eric.h"
#include "aby3-DB-main.h"
#include "aby3-DB_tests/UnitTests.h"
#include <tests_cryptoTools/UnitTests.h>
#include <aby3-ML/main-linear.h>
#include <aby3-ML/main-logistic.h>

#include "tests_cryptoTools/UnitTests.h"
#include "cryptoTools/Crypto/PRNG.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace oc;
using namespace aby3;
std::vector<std::string> unitTestTag{ "u", "unitTest" };


std::vector<std::pair<std::string, std::vector<int>>> read_csv(std::string filename){
    // Reads a CSV file into a vector of <string, vector<int>> pairs where
    // each pair represents <column name, column values>

    // Create a vector of <string, int vector> pairs to store the result
    std::vector<std::pair<std::string, std::vector<int>>> result;

    // Create an input filestream
	std::cout << filename << std::endl;
    std::ifstream myFile(filename);

    // Make sure the file is open
    if(!myFile.is_open()) throw std::runtime_error("Could not open file");

    // Helper vars
    std::string line, colname;
    int val;

    // Read the column names
    if(myFile.good())
    {
        // Extract the first line in the file
        std::getline(myFile, line);

        // Create a stringstream from line
        std::stringstream ss(line);

        // Extract each column name
        while(std::getline(ss, colname, ',')){
            
            // Initialize and add <colname, int vector> pairs to result
            result.push_back({colname, std::vector<int> {}});
        }
    }

    // Read data, line by line
    while(std::getline(myFile, line))
    {
        // Create a stringstream of the current line
        std::stringstream ss(line);
        
        // Keep track of the current column index
        int colIdx = 0;
        
        // Extract each integer
        while(ss >> val){
            
            // Add the current integer to the 'colIdx' column's values vector
            result.at(colIdx).second.push_back(val);
            
            // If the next token is a comma, ignore it and move on
            if(ss.peek() == ',') ss.ignore();
            
            // Increment the column index
            colIdx++;
        }
    }

    // Close file
    myFile.close();

    return result;
}


void help()
{

	std::cout << "-u                        ~~ to run all tests" << std::endl;
	std::cout << "-u n1 [n2 ...]            ~~ to run test n1, n2, ..." << std::endl;
	std::cout << "-u -list                  ~~ to list all tests" << std::endl;
	std::cout << "-intersect [-nn NN [-c C]] [-l L -r R]  ~~ to run the intersection benchmark with 2^NN set sizes, C 32-bit data columns, L and R are table csv files" << std::endl;
	std::cout << "-eric -nn NN              ~~ to run the eric benchmark with 2^NN set sizes" << std::endl;
	std::cout << "-threat -nn NN -s S       ~~ to run the threat log benchmark with 2^NN set sizes and S sets" << std::endl;
}


int main(int argc, char** argv)
{


	try {


		bool set = false;
		oc::CLP cmd(argc, argv);


		if (cmd.isSet(unitTestTag))
		{
			auto tests = tests_cryptoTools::Tests;
			tests += aby3_tests;
			tests += DB_tests;

			tests.runIf(cmd);
			return 0;
		}

		if (cmd.isSet("linear-plain"))
		{
			set = true;
			linear_plain_main(cmd);
		}
		if (cmd.isSet("linear"))
		{
			set = true;
			linear_main_3pc_sh(cmd);
		}

		if (cmd.isSet("logistic-plain"))
		{
			set = true;
			logistic_plain_main(cmd);
		}

		if (cmd.isSet("logistic"))
		{
			set = true;
			logistic_main_3pc_sh(cmd);
		}

		if (cmd.isSet("eric"))
		{
			set = true;

			auto nn = cmd.getMany<int>("nn");
			if (nn.size() == 0)
				nn.push_back(16);

			for (auto n : nn)
			{
				eric(1 << n);
			}
		}


		if (cmd.isSet("intersect"))
		{
			set = true;

			auto nn = cmd.getMany<int>("nn");
			auto c = cmd.getOr("c", 0);
			auto left = cmd.getOr<std::string>("l", "");
			auto right = cmd.getOr<std::string>("r", "");
			auto rank = cmd.getOr<int>("r", 0);
			auto ip0 = cmd.getOr<std::string>("ip0", "machine_1");
			auto ip1 = cmd.getOr<std::string>("ip1", "machine_2");
			auto ip2 = cmd.getOr<std::string>("ip2", "machine_3");

			std::pair<std::vector<std::pair<std::string, std::vector<int>>>, 
			std::vector<std::pair<std::string, std::vector<int>>>> data;
			
			if(left !=  "" && right != ""){
				data.first = read_csv(left);
				data.second = read_csv(right);
			}

			if (nn.size() == 0)
				nn.push_back(1 << 16);

			for (auto n : nn)
			{
				auto size = 1 << n;
				if(!data.first.empty()){
					size = std::max(data.first.at(0).second.size(), data.second.at(0).second.size());
					c = std::max(data.first.size(), data.second.size()) - 1;
				}

				DB_Intersect(data, size, c, cmd.isSet("sum"), rank, ip0, ip1, ip2);

			}
		}


		if (cmd.isSet("threat"))
		{
			set = true;

			auto nn = cmd.getMany<int>("nn");
			auto c = cmd.getOr("s", 2);
			if (nn.size() == 0)
				nn.push_back(1 << 16);

			for (auto n : nn)
			{
				auto size = 1 << n;
				DB_threat(size, c);
			}
		}



		if (cmd.isSet("card"))
		{
			set = true;

			auto nn = cmd.getMany<int>("nn");
			if (nn.size() == 0)
				nn.push_back(1 << 16);

			for (auto n : nn)
			{
				auto size = 1 << n;
				DB_cardinality(size);
			}
		}
		
		//if (cmd.isSet("add"))
		//{
		//	set = true;

		//	auto nn = cmd.getMany<int>("nn");
		//	if (nn.size() == 0)
		//		nn.push_back(1 << 16);

		//	for (auto n : nn)
		//	{
		//		auto size = 1 << n;
		//		Sh3_add_test(size);
		//	}
		//}

		if (set == false)
		{
			help();
		}

	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	return 0;
}
