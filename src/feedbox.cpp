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

#include <sys/epoll.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <iostream>

#include "feedbox.hpp"
#include "context.hpp"
#include "utils.hpp"

namespace loglang{
	class Feed{
	public:
		Feed(int fd, bool is_secure) : fd(fd), is_secure(is_secure){
			file=fdopen(fd,"rt");
			if (!file)
				throw std::runtime_error("Bad file descriptor");
		}
		Feed(): fd(-1), is_secure(false), file(nullptr) {};
		~Feed(){
			if (fd>=0){
				fclose(file);
				close(fd);
			}
		}
		
		bool is_secure;
		int fd;
		FILE *file;
		
		enum type_t{
			EPOLL,
			INOTIFY,
		};
		type_t type;
	};
}

using namespace loglang;

FeedBox::FeedBox()
{
	pollfd=epoll_create(8);
	if (pollfd<0){
		throw std::runtime_error("Could not create poller descriptor.");
	}
}

FeedBox::~FeedBox()
{	
	close(pollfd);
}


void FeedBox::add_feed(int fd, bool is_secure)
{
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events=EPOLLIN;
	ev.data.fd=fd;
	
	if (epoll_ctl(pollfd, EPOLL_CTL_ADD, fd, &ev) < 0){
		if (errno==EPERM){ // Quite probacly a normal file, so we dont add this as to be feed, but to be inotified and fully read. Fully read now too.
			
		}
		
		throw std::runtime_error(std::string("Could not add poll descriptor: ")+strerror(errno));
	}
	
	feeds[fd]=Feed(fd, is_secure);
}


void FeedBox::remove_feed(int fd){
	feeds.erase( feeds.find(fd) );
}

void FeedBox::run(Context &ctx){
	while (running)
		run_once(ctx);
}

void FeedBox::run_once(Context &ctx){
	struct epoll_event events[1];
	
	int nfds = epoll_wait(pollfd, events, 1, -1);
	std::string line;
	for(int n = 0; n < nfds; ++n) {
		Feed &feed=feeds[events[n].data.fd];
		
		int len=getline(&rline, &rline_size, feed.file);
		if (len<0){
			fprintf(stderr,"Error reading data");
			continue; 
		}
		
		line=std::string(rline, len);
		loglang::clean(line);
		
		if (feed.is_secure)
			ctx.feed_secure(line);
		else
			ctx.feed(line);
	}
}
