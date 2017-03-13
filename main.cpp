#include "MineGame.h"
#include "gtest/gtest.h"
#include <iostream>

using namespace std;

// int main(int argc, char ** argv){
// 	::testing::InitGoogleTest(&argc, argv);
// 	RUN_ALL_TESTS();
// 	return 0;
// }

enum Method{
	normal = 0, memory
};

Method tip(int argc, char ** argv){
	Method choice;
	if (argc == 2 && strncmp(argv[1], "-m", 2) == 0){									// 1 : memory method
		choice = memory;
	}
	else if (argc == 1 || (argc == 2 && strncmp(argv[1], "-n", 2) == 0)){				// 0 : normal method
		choice = normal;
	}
	else{
		string info;
		ostringstream ofs;
		ofs << "Usage : " << argv[0] << " -[hmn]\n"
			<< "\t-h\thelp\n"
			<< "\t-m\tread memory directly\n"
			<< "\t-n[default]\tnormal method to simulate people's work" << endl;
		throw exception(ofs.str().c_str());
	}
	return choice;
}

int main(int argc, char ** argv){
	try	{
		Method choice = tip(argc, argv);
		AutoSweepMine autoSweepMine;
		choice == 1 ? autoSweepMine.DoAutoSweepMine_WithMemory() : autoSweepMine.DoAutoSweepMine();
	}
	catch (exception & e){
		cout << e.what() << endl;
	}
	
	return 0;
}
