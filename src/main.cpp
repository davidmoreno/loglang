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
#include <fcntl.h>

#include "context.hpp"
#include "utils.hpp"
#include "feedbox.hpp"

int main(int argc, char **argv){
// 	auto &input=std::cin;
// 	input.sync_with_stdio(false);
	std::cout.sync_with_stdio(false);
	
	loglang::Context context;
	loglang::FeedBox feedbox;
// 	parser.set_output([](const std::string &output){ std::cout<<">> "<<output<<std::endl; });

	try{
		for(int i=1;i<argc;i++){
			int fd=open(argv[i], O_RDONLY);
			try{
				if (fd<0){
					throw std::ios_base::failure(std::string("Cant open ")+argv[i]);
				}
				feedbox.add_feed( fd, true );
			}
			catch(const std::exception &ex){
				std::cerr<<argv[1] <<": "<<ex.what()<<std::endl;
				return 1;
			}
		}
		feedbox.add_feed( 0 );
		
		feedbox.run(context);
	}catch(const std::exception &e){
		std::cerr<<"Uncatched exception. "<<e.what()<<std::endl;
		return 1;
	}
#ifdef __DEBUG__
	std::cerr<<"---"<<std::endl;
	context.debug_values();
#endif
	
	return 0;
}

