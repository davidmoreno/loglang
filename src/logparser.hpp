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

#include <string>
#include <functional>
#include <unordered_map>

#include "dataitem.hpp"
#include "program.hpp"

namespace loglang{
	class LogParser{
		std::function<void (const std::string &output)> _output;
		std::unordered_map<std::string, DataItem> datastore;
		std::unordered_map<std::string, std::shared_ptr<Program>> programs;
	public:
		LogParser();
		void feed(const std::string &data);
		void set_output(std::function<void (const std::string &output)> &&);
		void output(const std::string &str){ _output(str); }
		void output(const std::string &str, const std::string &str2);
		DataItem &get_value(const std::string &key);
		
		void debug_values();
	};
};
