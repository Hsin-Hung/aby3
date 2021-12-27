
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
#include "utils.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace oc;
using namespace aby3;
std::vector<std::string> unitTestTag{"u", "unitTest"};

void help()
{

	std::cout << "-u                        ~~ to run all tests" << std::endl;
	std::cout << "-u n1 [n2 ...]            ~~ to run test n1, n2, ..." << std::endl;
	std::cout << "-u -list                  ~~ to list all tests" << std::endl;
	std::cout << "-intersect [-nn NN [-c C]] [-lf L -rf R] [-r R] ~~ to run the intersection benchmark with 2^NN set sizes, C 32-bit data columns,\n \
	L and R are optional table csv files, and the rank R(0-3) of the calling party."
			  << std::endl;
	std::cout << "-eric -nn NN              ~~ to run the eric benchmark with 2^NN set sizes" << std::endl;
	std::cout << "-threat -nn NN -s S       ~~ to run the threat log benchmark with 2^NN set sizes and S sets" << std::endl;
}

int main(int argc, char **argv)
{

	try
	{

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
			auto left = cmd.getOr<std::string>("lf", "");
			auto right = cmd.getOr<std::string>("rf", "");
			auto rank = cmd.getOr<int>("r", 0);
			auto ip0 = cmd.getOr<std::string>("ip0", "127.0.0.1");
			auto ip1 = cmd.getOr<std::string>("ip1", "127.0.0.1");
			auto ip2 = cmd.getOr<std::string>("ip2", "127.0.0.1");
			auto d = cmd.getOr<bool>("d", 0);

			std::pair<std::vector<std::pair<std::string, std::vector<int>>>,
					  std::vector<std::pair<std::string, std::vector<int>>>>
				data;

			if (rank < 0 || rank > 2)
			{
				std::cerr << "Invalid rank!" << std::endl;
				exit(1);
			}

			if (left != "" && right != "")
			{
				data.first = read_csv(left);
				data.second = read_csv(right);
			}

			if (nn.size() == 0)
				nn.push_back(1 << 16);

			for (auto n : nn)
			{
				auto size = 1 << n;
				if (!data.first.empty())
				{
					size = std::max(data.first.at(0).second.size(), data.second.at(0).second.size());
					c = std::max(data.first.size(), data.second.size()) - 1;
				}

				DB_Intersect(data, size, rank, getPartyIPs(), c, cmd.isSet("sum"), d);
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

		// if (cmd.isSet("add"))
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
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}

	return 0;
}
