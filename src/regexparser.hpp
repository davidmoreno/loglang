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
#include <functional>
#include <map>
#include <memory>

namespace loglang{
	class Context;
	class RegexParserPrivate;
	class RegexParser{
		std::unique_ptr<RegexParserPrivate> d;
	public:
		using cb_t=std::function<void (Context &context)>;
		
		RegexParser();
		~RegexParser();
		
		void addRegex(const std::string &regex, RegexParser::cb_t cb);
		bool parse(const std::string &str, Context &context);
	};
};
