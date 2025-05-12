#pragma once

#include <string>
#include <fstream>

using namespace std;

string readShader(const char* fname) {
	//Read shader code from file and return as string
	ifstream shader(fname);
	
	return string(istreambuf_iterator<char>(shader), istreambuf_iterator<char>());
}