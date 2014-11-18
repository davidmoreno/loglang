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

#pragma once

#include <string>
#include <vector>
#include <memory>

#include "program.hpp"

namespace loglang{
	class LogParser;
	
	class DataItem{
		std::vector<std::shared_ptr<Program>> at_modify;
		std::string value;
		std::string name;
	public:
		DataItem(std::string _name);
		void run_at_modify(std::shared_ptr<Program> at_modify);
		void remove_program(std::shared_ptr<Program> at_modify);
		
		void set(const std::string &str, LogParser &context);
		std::string get();
	};
}
