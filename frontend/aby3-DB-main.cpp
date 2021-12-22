
#include <cryptoTools/Common/CLP.h>

#include "cryptoTools/Common/BitIterator.h"
#include "cryptoTools/Common/Timer.h"
#include "cryptoTools/Crypto/PRNG.h"
#include <cryptoTools/Network/IOService.h>
#include <atomic>
#include "aby3-DB/DBServer.h"
#include <unordered_set>

using namespace oc; 


void DB_Intersect(std::pair<std::vector<std::pair<std::string, std::vector<int>>>, 
std::vector<std::pair<std::string, std::vector<int>>>> data, u32 rows, u32 cols, bool sum, int rank, std::string ip0, std::string ip1, std::string ip2)
{
	using namespace aby3;
	IOService ios;
	std::string firstIP;
	std::string secondIP;
	u32 firstPort;
	u32 secondPort;
	SessionMode firstMode;
	SessionMode secondMode;
	std::string firstName;	
	std::string secondName;

	ios.showErrorMessages(true);

	std::cout << "ip0: " << ip0 << "\n";
	std::cout << "ip1: " << ip1 << "\n";
	std::cout << "ip2: " << ip2 << std::endl;

	if (rank == 0) {
		firstIP = "0.0.0.0";
		secondIP = "0.0.0.0";
		firstMode = SessionMode::Server;
		secondMode = SessionMode::Server;
		firstPort = 1212;
		secondPort = 1213;
		firstName = "02";
		secondName = "01";
	} else if (rank == 1) {
		firstIP = ip0;
		secondIP = "0.0.0.0";
		firstMode = SessionMode::Client;
		secondMode = SessionMode::Server;
		firstPort = 1213;
		secondPort = 1214;
		firstName = "01";
		secondName = "12";
	} else if (rank == 2) {
		firstIP = ip1;
		secondIP = ip0;
		firstMode = SessionMode::Client;
		secondMode = SessionMode::Client;
		firstPort = 1214;
		secondPort = 1212;
		firstName = "12";
		secondName = "02";
	}
	std::cout << rank << " " << firstIP << " " << secondIP << std::endl;
	Session server(ios, firstIP, firstPort, firstMode, firstName);
	Session client(ios, secondIP, secondPort, secondMode, secondName);


	// A Peudorandom number generator implemented using AES-NI
	PRNG prng(oc::ZeroBlock);
	DBServer srvs;
	srvs.init(rank, server, client, prng);
	std::cout << "Prev Channel connected = " << srvs.mRt.mComm.mPrev.isConnected() << std::endl;
	std::cout << "Next Channel connected = " << srvs.mRt.mComm.mNext.isConnected() << std::endl;
	std::cout << "Connection made" << std::endl;

	auto has_data = !data.first.empty();

	// 80 bits;
	u32 hashSize = 80;

	auto keyBitCount = srvs.mKeyBitCount;
	std::vector<ColumnInfo>
		aCols = { ColumnInfo{ "key", TypeID::IntID, keyBitCount } },
		bCols = { ColumnInfo{ "key", TypeID::IntID, keyBitCount } };

	for (u32 i = 0; i < cols; ++i)
	{
		aCols.emplace_back("a" + std::to_string(i), TypeID::IntID, 32);
		bCols.emplace_back("b" + std::to_string(i), TypeID::IntID, 32);
	}

	std::cout << "aCols size: " << aCols.size() << std::endl;
	std::cout << "bCols size: " << bCols.size() << std::endl;
	
	auto left_table = data.first, right_table = data.second;

	Table a(rows, aCols)
		, b(rows, bCols);
	auto intersectionSize = (rows + 1) / 2;
	if(has_data){
		for (u64 i = 0; i < rows; ++i){
			for (u64 numCol = 0; numCol < a.mColumns.size(); ++numCol){
				for (u64 j = 0; j < a.mColumns[numCol].mData.cols(); ++j){
					if(i < left_table.at(numCol).second.size())
						a.mColumns[numCol].mData(i, j) = left_table.at(numCol).second.at(i);
					if(i < right_table.at(numCol).second.size())
						b.mColumns[numCol].mData(i, j) = right_table.at(numCol).second.at(i);
				}
			}
		}
	}else{
		for (u64 i = 0; i < rows; ++i)
		{
			auto out = (i >= intersectionSize);
			for (u64 numCol = 0; numCol < a.mColumns.size(); ++numCol){
				for (u64 j = 0; j < a.mColumns[numCol].mData.cols(); ++j)
				{
					a.mColumns[numCol].mData(i, j) = i + 1;
					b.mColumns[numCol].mData(i, j) = i + 1  + (rows * out);
				}
			}
		}
	}

	// std::cout << "rows: " << rows << ", cols: " << cols << std::endl;
	// std::cout << "Table A\n_____________" << std::endl;
	// std::cout << a << std::endl;
	// std::cout << "Table B\n_____________" << std::endl;
	// std::cout << b << std::endl;
	Timer t;

	int i = rank;
	bool failed = false;
	std::cout << "Start routine" << std::endl;
	setThreadName("t0");
	t.setTimePoint("start");
	// party 0 will get the local input and other parties will get it remotely from party 0
	std::cout << "Getting A" << std::endl;
	auto A = (i == 0) ? srvs.localInput(a) : srvs.remoteInput(0);
	std::cout << "Getting B" << std::endl;
	auto B = (i == 0) ? srvs.localInput(b) : srvs.remoteInput(0);


	if (i == 0)
		t.setTimePoint("inputs");

	if (i == 0)
		srvs.setTimer(t);
	

	// std::cout << "routine: " << i << std::endl; 
	// std::cout << "SharedTable A\n_____________" << std::endl;
	// std::cout << A << std::endl;
	// std::cout << "SharedTable B\n_____________" << std::endl;
	// std::cout << B << std::endl;
	


	SelectQuery query;
	query.noReveal("r");
	auto aKey = query.joinOn(A["key"], B["key"]);
	query.addOutput("key", aKey);

	for (u32 i = 0; i < cols; ++i)
	{
		//query.addOutput("a" + std::to_string(i), query.addInput(A["a" + std::to_string(i)]));
		query.addOutput("b" + std::to_string(i), query.addInput(B["b" + std::to_string(i)]));
	}

	auto C = srvs.joinImpl(query);

	// if (i == 0)
		// std::cout << "Join Implement Returns Shared Table: \n" << C << std::endl;
	if (i == 0)
		t.setTimePoint("intersect");


	if (sum)
	{

		Sh3BinaryEvaluator eval;

		BetaLibrary lib;
		BetaCircuit* cir = lib.int_int_add(64, 64, 64);

		auto task = srvs.mRt.noDependencies();

		sbMatrix AA(C.rows(), 64), BB(C.rows(), 64), CC(C.rows(), 64);
		task = eval.asyncEvaluate(task, cir, srvs.mEnc.mShareGen, { &AA, &BB }, { &CC });

		Sh3Encryptor enc;
		if (i == 0)
		{
			i64Matrix m(C.rows(), 1);
			enc.reveal(task, CC, m).get();
		}
		else
			enc.reveal(task, 0, CC).get();


		if (i == 0)
			t.setTimePoint("sum");
	}
	else if (C.rows())
	{
		aby3::i64Matrix c(C.mColumns[0].rows(), C.mColumns[0].i64Cols());
		srvs.mEnc.revealAll(srvs.mRt.mComm, C.mColumns[0], c);
		if (i == 0)
			t.setTimePoint("reveal");

		// std::cout << "reveal C: \n"<< C << std::endl;
		// std::cout << "reveal mini c: \n" << c << std::endl;
		
		

	}
	else
	{
		failed = true;
	}
	//std::cout << t << std::endl << srvs.getTimer() << std::endl;

	auto comm0 = (srvs.mRt.mComm.mNext.getTotalDataSent() + srvs.mRt.mComm.mPrev.getTotalDataSent());
	std::cout << "n = " << rows << "   " << comm0 << "\n" << t << std::endl;
	// std::cout << "n = " << rows << "   " << comm0 + comm1 + comm2 << "\n" << t << std::endl;

	if (failed)
	{
		std::cout << "bad intersection" << std::endl;
		throw std::runtime_error("");
	}
}


void DB_cardinality(u32 rows)
{

	IOService ios;
	Session s01(ios, "127.0.0.1", SessionMode::Server, "01");
	Session s10(ios, "127.0.0.1", SessionMode::Client, "01");
	Session s02(ios, "127.0.0.1", SessionMode::Server, "02");
	Session s20(ios, "127.0.0.1", SessionMode::Client, "02");
	Session s12(ios, "127.0.0.1", SessionMode::Server, "12");
	Session s21(ios, "127.0.0.1", SessionMode::Client, "12");


	PRNG prng(oc::ZeroBlock);
	DBServer srvs[3];
	srvs[0].init(0, s02, s01, prng);
	srvs[1].init(1, s10, s12, prng);
	srvs[2].init(2, s21, s20, prng);


	// 80 bits;
	u32 hashSize = 80;



	auto keyBitCount = srvs[0].mKeyBitCount;
	std::vector<ColumnInfo>
		aCols = { ColumnInfo{ "key", TypeID::IntID, keyBitCount } },
		bCols = { ColumnInfo{ "key", TypeID::IntID, keyBitCount } };

	Table a(rows, aCols)
		, b(rows, bCols);
	auto intersectionSize = (rows + 1) / 2;


	for (u64 i = 0; i < rows; ++i)
	{
		auto out = (i >= intersectionSize);
		for (u64 j = 0; j < a.mColumns[0].mData.cols(); ++j)
		{
			a.mColumns[0].mData(i, j) = i + 1;
			b.mColumns[0].mData(i, j) = i + 1 + (rows * out);
		}
	}

	Timer t;

	bool failed = false;
	auto routine = [&](int i) {
		setThreadName("t0");
		t.setTimePoint("start");


		auto A = (i == 0) ? srvs[i].localInput(a) : srvs[i].remoteInput(0);
		auto B = (i == 0) ? srvs[i].localInput(b) : srvs[i].remoteInput(0);

		if (i == 0)
			t.setTimePoint("inputs");

		if (i == 0)
			srvs[i].setTimer(t);



		std::array<SharedTable::ColRef, 2> AB{
			A["key"],
			B["key"]
		};
		std::array<u64, 2> reveals{ 1,2 };
		aby3::i64Matrix keys = srvs[i].computeKeys(AB, reveals);


		if (i == 0)
		{
			t.setTimePoint("keys");

			std::vector<u64> b0, b1;
			auto f0 = srvs[i].mRt.mComm.mPrev.asyncRecv(b0);
			auto f1 = srvs[i].mRt.mComm.mNext.asyncRecv(b0);

			f0.get();
			f1.get();

			std::unordered_set<u64> set;
			for (u64 i = 0; i < b0.size(); i += 2)
			{
				set.insert(b0[i]);
			}

			int count = 0;
			for (u64 i = 0; i < b1.size(); i += 2)
				count += set.find(b1[i]) != set.end();

			t.setTimePoint("intersect");
		}
		if (i)
		{
			if (keys.cols() != 2)
				throw RTE_LOC;

			auto begin = (std::array<u64, 2>*) keys.data();
			auto end = (std::array<u64, 2>*) (keys.data() + keys.size());

			PRNG prng2(ZeroBlock);
			std::shuffle(begin, end, prng2);


			if (i == 1)
				srvs[i].mRt.mComm.mPrev.send(keys.data(), keys.size());
			else
				srvs[i].mRt.mComm.mNext.send(keys.data(), keys.size());

		}

		//std::cout << t << std::endl << srvs[i].getTimer() << std::endl;
	};

	auto t0 = std::thread(routine, 0);
	auto t1 = std::thread(routine, 1);
	routine(2);

	t0.join();
	t1.join();

	auto comm0 = (srvs[0].mRt.mComm.mNext.getTotalDataSent() + srvs[0].mRt.mComm.mPrev.getTotalDataSent());
	auto comm1 = (srvs[1].mRt.mComm.mNext.getTotalDataSent() + srvs[1].mRt.mComm.mPrev.getTotalDataSent());
	auto comm2 = (srvs[2].mRt.mComm.mNext.getTotalDataSent() + srvs[2].mRt.mComm.mPrev.getTotalDataSent());
	std::cout << "n = " << rows << "   " << comm0 + comm1 + comm2 << "\n" << t << std::endl;

	if (failed)
	{
		std::cout << "bad intersection" << std::endl;
		throw std::runtime_error("");
	}
}





void DB_threat(u32 rows, u32 setCount)
{
	using namespace aby3;

	IOService ios;
	Session s01(ios, "127.0.0.1", SessionMode::Server, "01");
	Session s10(ios, "127.0.0.1", SessionMode::Client, "01");
	Session s02(ios, "127.0.0.1", SessionMode::Server, "02");
	Session s20(ios, "127.0.0.1", SessionMode::Client, "02");
	Session s12(ios, "127.0.0.1", SessionMode::Server, "12");
	Session s21(ios, "127.0.0.1", SessionMode::Client, "12");


	PRNG prng(oc::ZeroBlock);
	DBServer srvs[3];
	srvs[0].init(0, s02, s01, prng);
	srvs[1].init(1, s10, s12, prng);
	srvs[2].init(2, s21, s20, prng);


	// 80 bits;
	u32 hashSize = 80;



	auto keyBitCount = srvs[0].mKeyBitCount;
	std::vector<ColumnInfo>
		aCols = { ColumnInfo{ "key", TypeID::IntID, keyBitCount } },
		bCols = { ColumnInfo{ "key", TypeID::IntID, keyBitCount } };

	Table a(rows, aCols)
		, b(rows, bCols);
	auto intersectionSize = (rows + 1) / 2;


	for (u64 i = 0; i < rows; ++i)
	{
		auto out = (i >= intersectionSize);
		for (u64 j = 0; j < a.mColumns[0].mData.cols(); ++j)
		{
			a.mColumns[0].mData(i, j) = i + 1;
			b.mColumns[0].mData(i, j) = i + 1 + (rows * out);
		}
	}

	Timer t;

	bool failed = false;
	auto routine = [&](int i) {
		setThreadName("t0");
		if (i == 0)
			t.setTimePoint("start");

		BetaLibrary lib;
		BetaCircuit * cir = lib.int_int_add(32, 32, 32);

		auto A = (i == 0) ? srvs[i].localInput(a) : srvs[i].remoteInput(0);
		auto B = (i == 0) ? srvs[i].localInput(b) : srvs[i].remoteInput(0);
		SharedTable C;

		if (i == 0)
			t.setTimePoint("inputs");

		//if (i == 0)
		//	srvs[i].setTimer(t);

		auto ss = setCount;
		while (ss > 1)
		{

			int loops = ss / 2;

			for (u64 j = 0; j < loops; ++j)
			{
				C = srvs[i].rightUnion(A["key"], B["key"], { A["key"] }, { B["key"] });
			}

			A = C;
			B = C;
			ss -= loops;
		}

		if (i == 0)
			t.setTimePoint("union");

		Sh3BinaryEvaluator eval;


		auto task = srvs[i].mRt.noDependencies();

		sbMatrix AA(C.rows(), 32), BB(C.rows(), 32), CC(C.rows(), 32);
		task = eval.asyncEvaluate(task, cir, srvs[i].mEnc.mShareGen, { &AA, &BB }, { &CC });
		task = eval.asyncEvaluate(task, cir, srvs[i].mEnc.mShareGen, { &AA, &BB }, { &CC });
		task.get();

		if (i == 0)
			t.setTimePoint("done");
		if (C.rows())
		{
			aby3::i64Matrix c(C.mColumns[0].rows(), C.mColumns[0].i64Cols());
			srvs[i].mEnc.revealAll(srvs[i].mRt.mComm, C.mColumns[0], c);

			if (i == 0)
				t.setTimePoint("reveal");
		}
		else
		{
			failed = true;
		}
		//std::cout << t << std::endl << srvs[i].getTimer() << std::endl;
	};

	auto t0 = std::thread(routine, 0);
	auto t1 = std::thread(routine, 1);
	routine(2);

	t0.join();
	t1.join();

	auto comm0 = (srvs[0].mRt.mComm.mNext.getTotalDataSent() + srvs[0].mRt.mComm.mPrev.getTotalDataSent());
	auto comm1 = (srvs[1].mRt.mComm.mNext.getTotalDataSent() + srvs[1].mRt.mComm.mPrev.getTotalDataSent());
	auto comm2 = (srvs[2].mRt.mComm.mNext.getTotalDataSent() + srvs[2].mRt.mComm.mPrev.getTotalDataSent());
	std::cout << "n = " << rows << "   " << comm0 + comm1 + comm2 << "\n" << t << std::endl;

	if (failed)
	{
		std::cout << "bad intersection" << std::endl;
		throw std::runtime_error("");
	}
}






i64 Sh3_add_test(u64 n)
{
	BetaLibrary lib;
	BetaCircuit* cir = lib.int_int_add(32, 32, 32);


	IOService ios;
	Session s01(ios, "127.0.0.1", SessionMode::Server, "01");
	Session s10(ios, "127.0.0.1", SessionMode::Client, "01");
	Session s02(ios, "127.0.0.1", SessionMode::Server, "02");
	Session s20(ios, "127.0.0.1", SessionMode::Client, "02");
	Session s12(ios, "127.0.0.1", SessionMode::Server, "12");
	Session s21(ios, "127.0.0.1", SessionMode::Client, "12");

	Channel chl01 = s01.addChannel("c");
	Channel chl10 = s10.addChannel("c");
	Channel chl02 = s02.addChannel("c");
	Channel chl20 = s20.addChannel("c");
	Channel chl12 = s12.addChannel("c");
	Channel chl21 = s21.addChannel("c");


	CommPkg comms[3], debugComm[3];
	comms[0] = { chl02, chl01 };
	comms[1] = { chl10, chl12 };
	comms[2] = { chl21, chl20 };

	debugComm[0] = { comms[0].mPrev.getSession().addChannel(), comms[0].mNext.getSession().addChannel() };
	debugComm[1] = { comms[1].mPrev.getSession().addChannel(), comms[1].mNext.getSession().addChannel() };
	debugComm[2] = { comms[2].mPrev.getSession().addChannel(), comms[2].mNext.getSession().addChannel() };

	cir->levelByAndDepth();
	u64 width = n;
	bool failed = false;
	//bool manual = false;
	using namespace aby3;

	enum Mode { Manual, Auto, Replicated };


	auto aSize = cir->mInputs[0].size();
	auto bSize = cir->mInputs[1].size();
	auto cSize = cir->mOutputs[0].size();
	Timer t;
	auto t0 = std::thread([&]() {
		auto i = 0;
		i64Matrix a(width, 1), b(width, 1), c(width, 1);
		i64Matrix aa(width, 1), bb(width, 1);

		PRNG prng(ZeroBlock);
		for (u64 i = 0; i < a.size(); ++i)
		{
			a(i) = prng.get<i64>();
			b(i) = prng.get<i64>();
		}

		Sh3Runtime rt(i, comms[i]);

		sbMatrix A(width, aSize), B(width, bSize), C(width, cSize);

		Sh3Encryptor enc;
		enc.init(i, toBlock(i), toBlock(i + 1));

		auto task = rt.noDependencies();
		t.setTimePoint("start");
		enc.localBinMatrix(rt.noDependencies(), a, A).get();


		task = enc.localBinMatrix(rt.noDependencies(), b, B);

		Sh3BinaryEvaluator eval;

		t.setTimePoint("eval");

		task = eval.asyncEvaluate(task, cir, enc.mShareGen, { &A, &B }, { &C });
		task.get();
		t.setTimePoint("done");

	});

	auto routine = [&](int i)
	{
		PRNG prng;

		Sh3Runtime rt(i, comms[i]);

		sbMatrix A(width, aSize), B(width, bSize), C(width, cSize);

		Sh3Encryptor enc;
		enc.init(i, toBlock(i), toBlock((i + 1) % 3));


		auto task = rt.noDependencies();
		// queue up the operations
		enc.remoteBinMatrix(rt.noDependencies(), A).get();
		task = enc.remoteBinMatrix(rt.noDependencies(), B);

		Sh3BinaryEvaluator eval;

		task = eval.asyncEvaluate(task, cir, enc.mShareGen, { &A, &B }, { &C });

		// actually execute the computation
		task.get();

	};

	auto t1 = std::thread(routine, 1);
	auto t2 = std::thread(routine, 2);

	t0.join();
	t1.join();
	t2.join();


	auto comm0 = (comms[0].mNext.getTotalDataSent() + comms[0].mNext.getTotalDataSent());
	auto comm1 = (comms[1].mNext.getTotalDataSent() + comms[1].mNext.getTotalDataSent());
	auto comm2 = (comms[2].mNext.getTotalDataSent() + comms[2].mNext.getTotalDataSent());
	std::cout << "n = " << n << "   " << comm0 + comm1 + comm2 << "\n" << t << std::endl;
	if (failed)
		throw std::runtime_error(LOCATION);

	return 0;
}
