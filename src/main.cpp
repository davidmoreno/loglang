/*
 * Copyright 2014 David Moreno
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <fstream>
#include "logparser.hpp"

int main(int argc, char **argv){
	auto &input=std::cin;
	input.sync_with_stdio(false);
	std::cout.sync_with_stdio(false);
	
	std::string line;
	loglang::LogParser parser;
// 	parser.set_output([](const std::string &output){ std::cout<<">> "<<output<<std::endl; });
	
	for(int i=1;i<argc;i++){
		std::ifstream fin(argv[i],std::ios::in);
		if (!fin.is_open()){
			throw std::exception();
		}
		while(!fin.eof()){
			std::getline(fin, line);
			
			parser.feed(line);
		}
		fin.close();
	}
	
	while(!input.eof()){
		std::getline(input, line);
		
		parser.feed(line);
	}
	std::cerr<<"---"<<std::endl;
	parser.debug_values();
	
	return 0;
}

