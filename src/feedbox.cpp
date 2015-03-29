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
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include <iostream>

#include "feedbox.hpp"
#include "context.hpp"
#include "utils.hpp"

#define INOTIFY_EVENT_SIZE  ( sizeof (struct inotify_event) )
#define INOTIFY_EVENT_BUF_LEN     ( 1024 * ( INOTIFY_EVENT_SIZE + 16 ) )

namespace loglang{
	extern bool debug;
	
	class FeedStream{
	public:
		bool is_secure;
		int fd;
		std::string filename;
		FILE *file;

		FeedStream(std::string filename_, bool is_secure, int epollfd) : filename(std::move(filename_)), is_secure(is_secure){
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
			
			struct epoll_event ev;
			memset(&ev, 0, sizeof(ev));
			ev.events=EPOLLIN;
			ev.data.fd=fd;
			
			if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) < 0){
				fclose(file);
				close(fd);
				throw std::runtime_error(std::string("Could not add poll descriptor: ")+strerror(errno));
			}
		}
		FeedStream(FeedStream &) = delete;
		FeedStream &operator=(FeedStream &) = delete;
		FeedStream(FeedStream &&) = delete;
		FeedStream &operator=(FeedStream &&) = delete;
		~FeedStream(){
			if (fd>=0){
				fclose(file);
				close(fd);
			}
		}
	};
	class FeedFile{
	public:
		bool is_secure;
		int wd; // inotify wait descriptor
		std::string filename;
		int lineno; // Last read line, to skip there. 
		char *line;
		size_t line_size;
		
		FeedFile(std::string filename_, bool is_secure, int inotifyfd, Context &ctx) : filename(std::move(filename_)), is_secure(is_secure){
			wd = inotify_add_watch( inotifyfd, filename.c_str(), IN_MODIFY | IN_CREATE );
			if (wd<0){
				throw std::runtime_error(std::string("Could not inotify this file: ")+filename);
			}
			lineno=0;
			line=(char*)malloc(1024);
			line_size=1024;
			feed_full_file(ctx);
			if (debug){
				std::cerr<<"Inotify open for fd "<<wd<<std::endl;
			}
		}
		FeedFile(FeedFile &) = delete;
		FeedFile &operator=(FeedFile &) = delete;
		FeedFile(FeedFile &&) = delete;
		FeedFile &operator=(FeedFile &&) = delete;
		~FeedFile(){
			free(line);
		}

		void feed_full_file(Context &ctx){
			FILE *file=fopen(filename.c_str(), "r");
			
			try{
				int current_line=0;
				while (!feof(file)){
					ssize_t len=getline(&line, &line_size, file);
					current_line++;
					if (current_line>=lineno){
						if (len>=0){
							line[len-1]=0; // Remove \n
							if (is_secure)
								ctx.feed_secure(line);
							else
								ctx.feed(line);
						}
					}
				}
				if (current_line<lineno){ // If lineno is greater, then truncated file
					if (debug){
						std::clog<<"File truncated: "<<filename<<std::endl;
					}
					lineno=0;
					feed_full_file(ctx); // Read from start
				}
				else{
					lineno=current_line; 
				}
			}
			catch(...){
				fclose(file);
				throw;
			}
			fclose(file);
		}
	};
}

using namespace loglang;

FeedBox::FeedBox(std::shared_ptr<Context> ctx) : ctx(ctx), rline(nullptr), inotify_buffer(nullptr)
{
	pollfd=epoll_create(8);
	if (pollfd<0){
		throw std::runtime_error("Could not create poller descriptor.");
	}
	inotifyfd=inotify_init();
	if (inotifyfd<0){
		close(pollfd);
		throw std::runtime_error("Could not create inotify descriptor.");
	}
	
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events=EPOLLIN;
	ev.data.fd=inotifyfd;
	
	if (epoll_ctl(pollfd, EPOLL_CTL_ADD, inotifyfd, &ev) < 0){
		close(pollfd);
		close(inotifyfd);
		throw std::runtime_error("Could not poll on inotify descriptor.");
	}
	
	inotify_buffer=(char*)malloc(INOTIFY_EVENT_BUF_LEN);
	rline=(char*)malloc(1024); // Must be malloc, as internally getline will use realloc
	rline_size=1024;
}

FeedBox::~FeedBox()
{	
	if (pollfd>=0)
		close(pollfd);
	if (inotifyfd>=0)
		close(inotifyfd);
	if (rline)
		free(rline);
	if (inotify_buffer)
		free(inotify_buffer);
}

/**
 * @short Adds a feed to the box
 */
void FeedBox::add_feed(std::string filename, bool is_secure)
{
	if (filename=="<stdin>"){
		auto feed=std::make_shared<FeedStream>(std::move(filename), is_secure, pollfd);
		feeds.insert(std::make_pair(feed->fd,std::move(feed)));
		++epoll_files;
	}
	else{
		struct stat st;
		if (stat(filename.c_str(), &st)>=0){ // If its a FIFO
			if (S_ISFIFO(st.st_mode)){
				auto feed=std::make_shared<FeedStream>(std::move(filename), is_secure, pollfd);
				feeds.insert(std::make_pair(feed->fd,std::move(feed)));
				++epoll_files;
				return;
			}
		}
		
		auto feed=std::make_shared<FeedFile>(std::move(filename), is_secure, inotifyfd, *ctx);
		filefeeds.insert(std::make_pair(feed->wd,std::move(feed)));
	}
}

void FeedBox::remove_feed(int fd){
	feeds.erase( feeds.find(fd) );
	
	if (epoll_ctl(pollfd, EPOLL_CTL_DEL, fd, NULL) < 0){
		throw std::runtime_error(std::string("Could not remove poll descriptor: ")+strerror(errno));
	}
}

void FeedBox::run(){
	running=true;
	while (running)
		run_once();
}
void FeedBox::run_once(){
	struct epoll_event events[8];
	if (epoll_files<=0){
		running=false;
		return;
	}
	int nfds = epoll_wait(pollfd, events, 8, -1);
	std::string line;
	
	for(int n = 0; n < nfds; ++n) {
		if (debug){
			std::clog<<"Event at fd "<<events[n].data.fd<<" "<<inotifyfd<<std::endl;
		}
		if (events[n].data.fd==inotifyfd){
			ssize_t length = read( inotifyfd, inotify_buffer, INOTIFY_EVENT_BUF_LEN ); 
			if (length<0){
				perror( "read" );
				continue;
			}
			int i=0;
			while (i<length){
				struct inotify_event *event = ( struct inotify_event * ) &inotify_buffer[ i ];
				auto feed=filefeeds[event->wd];
				feed->feed_full_file(*ctx);
				
				i+=INOTIFY_EVENT_SIZE+event->len;
			}
		}
		else{
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
			if (len>0){
				line=std::string(rline, len-1);

				if (feed->is_secure)
					ctx->feed_secure(line);
				else
					ctx->feed(line);
			}
		}
	}
}

void FeedBox::stop()
{
	running=false;
	close(pollfd); // FIXME Better have a signalfd and signal wakeup
	pollfd=-1;
}

