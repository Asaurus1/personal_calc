#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "Calc.h"

using namespace std;

int main2() //testing
{
    CMatrix mtrx{"[1 2 3; 4 5 6]"}, mtrx2{5}, mtrx3{"[1,2,3;4,5,6]"};
    CVariable v1{"ans", mtrx}, v2{"a",5.0}, v3{"b",4.0};

    cout << v1.Value() << endl;
    cout << v2.Value().IsSingle() << endl;
    v1 = v2.Value() + v3.Value();
    cout << v1.Value() << endl;
}

int main()
{
	// print welcome message
	cout << endl;
	cout << "\tWelcome to the EECS 211 MP#5: A Programmable Calculator" << endl;
	cout << "\t\tName: Alexander Martin"<< endl; // your name here
	cout << "\t\t   Copyright, 2014   " << endl << endl;

	string testfilename = "TestCase.txt";

	ifstream testfile(testfilename);

	Calc newCalc;

    //Create a new calculator from the right source.
	if (!testfile.is_open())
    {
        cout << "Error reading test file \"" << testfilename << '"' << endl;
        cout << "Reading from cin" << endl << endl;
    }
    else
    {
        cout << "Reading from test file \"" << testfilename << '"' << endl << endl;
        newCalc.setSource(testfile);
    }

    newCalc.run();

    return 0;
}

