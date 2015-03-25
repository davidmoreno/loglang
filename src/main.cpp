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
#include <signal.h>

#include "context.hpp"
#include "utils.hpp"
#include "feedbox.hpp"

namespace loglang{
	std::function<void()> stop_cb;
	bool debug=false;
}

void stop(int){
	std::cerr<<"exit"<<std::endl;
	if (loglang::stop_cb)
		loglang::stop_cb();
}

int main(int argc, char **argv){
// 	auto &input=std::cin;
// 	input.sync_with_stdio(false);
// 	std::cout.sync_with_stdio(false);
	
	signal(SIGTERM, stop);
	signal(SIGINT, stop);
	
	auto context=std::make_shared<loglang::Context>();
	loglang::FeedBox feedbox(context);

// 	context->set_output([](const std::string &output){ std::cout<<">> "<<output<<std::endl; });
	loglang::stop_cb=[&feedbox](){ feedbox.stop(); };

	try{
		for(int i=1;i<argc;i++){
			if (argv[i]==std::string("--debug"))
				loglang::debug=true;
			else{
				try{
					feedbox.add_feed( argv[i], true);
				}
				catch(const std::exception &ex){
					std::cerr<<argv[i] <<": "<<ex.what()<<std::endl;
					return 1;
				}
			}
		}
		feedbox.add_feed( "<stdin>", false);
		
		feedbox.run();
	}catch(const std::exception &e){
		std::cerr<<"Uncatched exception. "<<e.what()<<std::endl;
		return 1;
	}
	if (loglang::debug){
		std::cerr<<"--- Final memory status:"<<std::endl;
		context->debug_values();
	}
	
	return 0;
}

