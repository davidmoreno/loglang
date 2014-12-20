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

#include "context.hpp"
#include "utils.hpp"

int main(int argc, char **argv){
	auto &input=std::cin;
	input.sync_with_stdio(false);
	std::cout.sync_with_stdio(false);
	
	std::string line;
	loglang::Context context;
// 	parser.set_output([](const std::string &output){ std::cout<<">> "<<output<<std::endl; });

	try{
		for(int i=1;i<argc;i++){
			std::ifstream fin(argv[i],std::ios::in);
			if (!fin.is_open()){
				throw std::ios_base::failure(std::string("Cant open ")+argv[i]);
			}
			while(!fin.eof()){
				std::getline(fin, line);
				loglang::clean(line);
				if (line.length()>0)
					context.feed_secure(line);
			}
			fin.close();
		}
	
		while(!input.eof()){
			std::getline(input, line);
			
			loglang::clean(line);
			if (line.length()>0)
				context.feed(line);
		}
	}catch(const std::exception &e){
		std::cerr<<"Uncatched exception. "<<e.what()<<std::endl;
		return 1;
	}
	std::cerr<<"---"<<std::endl;
	context.debug_values();
	
	return 0;
}

