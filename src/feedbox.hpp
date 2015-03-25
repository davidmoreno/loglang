/*
 * Copyright 2015 David Moreno
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

#include <map>
#include <memory>

namespace loglang{
	/**
	* @short Manages feeds of loglang
	* 
	* Loglang can have many feeds, secure and unsecure. Each should be added to the feedbox, and
	* call the run method.
	* 
	* More can be added dynamically if needed.
	*/
	class Feed;
	class Context;
	
	class FeedBox{
		std::map<int, std::shared_ptr<Feed>> feeds;
		int wd; // inotify descriptor
		int pollfd;
		bool running;
		char *rline; // Temporal storage for line
		size_t rline_size;
		size_t epoll_files=0;
		std::shared_ptr<Context> ctx;
	public:
		FeedBox(std::shared_ptr<Context> ctx);
		~FeedBox();
		FeedBox() = delete;
		FeedBox(FeedBox &) = delete;
		FeedBox(FeedBox &&) = delete;
		FeedBox &operator=(FeedBox &) =delete;
		FeedBox &&operator=(FeedBox &&) =delete;
		
		void add_feed(std::string filename, bool is_secure);
		void remove_feed(int fd);
		
		void run();
		void run_once();
		
		void stop();
	};
}
