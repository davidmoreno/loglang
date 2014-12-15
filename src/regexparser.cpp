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

#include <vector>
#include <iostream>
#include <memory>

#include <pcre.h>

#include "regexparser.hpp"
#include "context.hpp"
#include "parser.hpp"

using namespace loglang;

class ::loglang::RegexParserPrivate{
public:
	struct regex_cb{
		pcre *re=nullptr;
		loglang::RegexParser::cb_t cb;
		int namecount;
		
		regex_cb() = delete;
		regex_cb& operator=(const regex_cb &o) = delete;
		regex_cb(regex_cb &) = delete;
		
		regex_cb(pcre *_re, loglang::RegexParser::cb_t _cb, int _namecount) : 
			re(_re), cb(_cb), namecount(_namecount){
		}
		
		regex_cb(regex_cb &&o){
			if (re)
				pcre_free(re);
			re=o.re;
			cb=std::move(o.cb);
			namecount=o.namecount;

			o.re=nullptr;
		}

		~regex_cb(){
			if (re)
				pcre_free(re);
			re=nullptr;
		}
	};
	
	std::vector<regex_cb> regex_list;
};

RegexParser::RegexParser()
{
	d=std::unique_ptr<RegexParserPrivate>(new RegexParserPrivate);
}

RegexParser::~RegexParser()
{

}

void RegexParser::addRegex(const std::string &regex, RegexParser::cb_t cb)
{
// 	std::cerr<<"Added regex "<<regex<<std::endl;
	const char *error;
	int erroroffset;
	pcre *re=pcre_compile(regex.c_str(), 0, &error, &erroroffset, NULL);
	if (!re){
		throw loglang::parsing_exception("Cant parse the regex");
	}
	std::vector<std::string> namegroups;
	
	int namecount;
	pcre_fullinfo(re, NULL, PCRE_INFO_NAMECOUNT, &namecount);
	
	d->regex_list.push_back(RegexParserPrivate::regex_cb(re, cb, namecount));
}

bool RegexParser::parse(const std::string& str, Context &ctx)
{
// 	std::cerr<<"Parse "<<str<<std::endl;
	int consumed;
	const int max_args=16;
	std::string args[max_args];
	const int OVECCOUNT=30;
	int ovector[OVECCOUNT];
	for(auto &re_cb: d->regex_list){
		auto rc=pcre_exec( re_cb.re, NULL, str.c_str(), str.length(), 0, 0, ovector, OVECCOUNT);
		if (rc!=PCRE_ERROR_NOMATCH){
			if (rc<=0){
				throw parsing_exception("Error executing the regex");
			}
			char *name_table;
			int name_entry_size;
			pcre_fullinfo(re_cb.re, NULL, PCRE_INFO_NAMETABLE, &name_table); 
			pcre_fullinfo(re_cb.re, NULL, PCRE_INFO_NAMEENTRYSIZE, &name_entry_size);
			int i;
			char *tabptr = name_table;
			for (i = 0; i < rc; i++){
				std::string val=std::string(std::begin(str)+ovector[2*i], std::begin(str)+ovector[2*i+1]);
				ctx.get_value(std::string(".")+std::to_string(i)).set(loglang::to_any(val), ctx);
			}

			for (i = 0; i < re_cb.namecount; i++){
				int n=(tabptr[0] << 8) | tabptr[1];
				std::string key(tabptr+2, name_entry_size-3);
				std::string val=std::string(std::begin(str)+ovector[2*n], std::begin(str)+ovector[2*n+1]);

// 				std::cerr<<"S "<<key<<" "<<val<<std::endl;
				any an=loglang::to_any(val);
// 				std::cerr<<"V "<<std::to_string(an)<<std::endl;
				ctx.get_value(key).set(std::move(an), ctx);
				tabptr+=name_entry_size;
			}
			
			re_cb.cb(ctx);
			return true;
		}
	}
	return false;
}
