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

#include "glob.hpp"
#include <iostream>

using namespace loglang;

bool loglang::glob_match(const std::string &text, const std::string &glob){
	auto T=std::begin(text), endT=std::end(text);
	auto G=std::begin(glob), endG=std::end(glob);
	
// 	std::cerr<<text<<" "<<glob<<std::endl;
	while( T!=endT && G!=endG ){
		if (*G == '?'){
			++G; ++T;
		}
		else if (*G == '*'){
			++G;
			if (G==endG) // Easy * at end
				return true;
			// From here, up to end, check all posibilities.
			for(;T!=endT;++T)
				if (glob_match(std::string(T,endT), std::string(G, endG)))
					return true;
			return false;
		}
		else if (*G == *T){
			++G; ++T;
		}
		else
			return false;
		
	}
	
	return ( T==endT && G==endG );
}