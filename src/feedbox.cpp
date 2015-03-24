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
#include <fcntl.h>

#include <iostream>

#include "feedbox.hpp"
#include "context.hpp"
#include "utils.hpp"

namespace loglang{
	class Feed{
	public:
		enum type_t{
			VOID=0,
			EPOLL,
			INOTIFY,
		};
		bool is_secure;
		int fd;
		std::string filename;
		FILE *file;
		
		type_t type;
		
		Feed(std::string filename_, bool is_secure, Feed::type_t type) : filename(std::move(filename_)), fd(fd), is_secure(is_secure), type(type){
			if (filename=="<stdin>") // special name
				fd=0;
			else{
				fd=open(filename.c_str(), O_RDONLY);
				if (fd<0){
					throw std::ios_base::failure(std::string("Cant open ")+filename);
				}
			}
			file=fdopen(fd,"rt");
			if (!file)
				throw std::runtime_error("Bad file descriptor");
		}
		Feed(): fd(-1), is_secure(false), file(nullptr), type(VOID) {};
		Feed(Feed &&o){
			fd=o.fd;
			file=o.file;
			type=o.type;
			o.fd=-1;
			o.file=nullptr;
		}
		~Feed(){
			if (fd>=0){
				fclose(file);
				close(fd);
			}
		}
		
		Feed &operator=(Feed &&o){
			fd=o.fd;
			file=o.file;
			type=o.type;
			o.fd=-1;
			o.file=nullptr;
			return *this;
		}
		
		Feed(Feed &) = delete;
		Feed &operator=(Feed &) = delete;


		void feed(Context &ctx){
			switch(type){
			case EPOLL:
				feed_epoll(ctx);
				break;
			case INOTIFY:
				feed_full_file(ctx);
				break;
			}
		}
		
		void feed_epoll(Context &ctx){
		}
		
		void feed_full_file(Context &ctx){
			char *line=(char*)malloc(1024);
			size_t line_size=1024;
			
			while (!feof(file)){
				ssize_t len=getline(&line, &line_size, file);
				if (len>=0){
					if (is_secure)
						ctx.feed_secure(line);
					else
						ctx.feed(line);
				}
			}
			fclose(file);
			fd=-1;
			if (line)
				free(line);
		}
		
	};
}

using namespace loglang;

FeedBox::FeedBox()
{
	pollfd=epoll_create(8);
	if (pollfd<0){
		throw std::runtime_error("Could not create poller descriptor.");
	}
	rline=(char*)malloc(1024); // Must be malloc, as internally getline will use realloc
	rline_size=1024;
}

FeedBox::~FeedBox()
{	
	if (pollfd>=0)
		close(pollfd);
	if (rline)
		free(rline);
}

/**
 * @short Adds a feed to the box
 */
void FeedBox::add_feed(std::string filename, bool is_secure, Context &ctx)
{
	auto feed=std::make_shared<Feed>(std::move(filename), is_secure, Feed::EPOLL);
	
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events=EPOLLIN;
	ev.data.fd=feed->fd;
	
	if (epoll_ctl(pollfd, EPOLL_CTL_ADD, feed->fd, &ev) < 0){
		if (errno==EPERM){ // Quite probacly a normal file, so we dont add this as to be feed, but to be inotified and fully read. Fully read now too.
			feed->type=Feed::INOTIFY;
		}
		else
			throw std::runtime_error(std::string("Could not add poll descriptor: ")+strerror(errno));
	}
	
	
	if (feed->type==Feed::INOTIFY){
		feed->feed_full_file(ctx);
	}
	else
		++epoll_files;
	
	feeds.insert(std::make_pair(feed->fd,std::move(feed)));
}

void FeedBox::remove_feed(int fd){
	feeds.erase( feeds.find(fd) );
	
	if (epoll_ctl(pollfd, EPOLL_CTL_DEL, fd, NULL) < 0){
		throw std::runtime_error(std::string("Could not remove poll descriptor: ")+strerror(errno));
	}
}

void FeedBox::run(Context &ctx){
	running=true;
	while (running)
		run_once(ctx);
}

void FeedBox::run_once(Context &ctx){
	struct epoll_event events[8];
	if (epoll_files<=0){
		running=false;
		return;
	}
	int nfds = epoll_wait(pollfd, events, 8, -1);
	std::string line;
	for(int n = 0; n < nfds; ++n) {
		auto feed=feeds[events[n].data.fd];
		
		int len=getline(&rline, &rline_size, feed->file);
		if (len<0){
			if (!feof(feed->file))
				throw std::runtime_error(feed->filename+": Error reading data: "+std::to_string(len)+":"+std::string(strerror(errno)));
				
			std::cerr<<feed->filename<<": File closed."<<std::endl;
			remove_feed(feed->fd);
			--epoll_files;
			continue;
		}
		
		line=std::string(rline, len);
		loglang::clean(line);

		if (feed->is_secure)
			ctx.feed_secure(line);
		else
			ctx.feed(line);
	}
}

void FeedBox::stop()
{
	running=false;
	close(pollfd); // FIXME Better have a signalfd and signal wakeup
	pollfd=-1;
}
