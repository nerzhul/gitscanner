/*
 * Copyright (c) 2017, Loic Blot <loic.blot@unix-experience.fr>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <csignal>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <iostream>
#include <getopt.h>
#include <cstring>
#include <unistd.h>
#include "gitscanner.h"
#include "gitupdater.h"

static bool stop_main_loop = false;
static int sleeping_time = 60;
static bool daemonize = false;
std::condition_variable cv_main_sleep;

static void signal_should_stop(int signum)
{
	stop_main_loop = true;
	cv_main_sleep.notify_all();
}

static void signal_awake_scan(int signum)
{
	cv_main_sleep.notify_all();
}

static bool is_number(const char *str)
{
	int x = 0;
	int len = strlen(str);

	while(x < len) {
		if (!isdigit(*(str+x)))
			return false;

		++x;
	}

	return true;
}

static void read_opts(int argc, char * const *argv)
{
	int c;
	while ((c = getopt(argc, argv, "di:")) != -1) {
		switch (c) {
			case 'd':
				std::cout << "Daemon mode enabled." << std::endl;
				daemonize = true;
				break;
			case 'i':
				if (!is_number(optarg)) {
					std::cerr << "Option '-i' requires a number." << std::endl;
					exit(EXIT_FAILURE);
				}

				sleeping_time = std::atoi(optarg);
				break;
			default: exit(EXIT_FAILURE);
		}
	}
}

int main(int argc, char * const *argv)
{
	read_opts(argc, argv);

	if (daemonize) {
		pid_t pid = fork();
		// Fork failed
		if (pid < 0) {
			std::cerr << "I failed to fork. Dying." << std::endl;
			exit(EXIT_FAILURE);
		}
		// Fork success, parent exits
		else if (pid > 0) {
			exit(EXIT_SUCCESS);
		}
	}

	signal(SIGTERM, signal_should_stop);
	signal(SIGINT, signal_should_stop);
	signal(SIGHUP, signal_awake_scan);

	std::mutex m_cv_lock;

	while (!stop_main_loop) {
		std::unique_lock<std::mutex> cv_u_lock(m_cv_lock);
		GitUpdater() << GitScanner();

		std::cout << "GitUpdater finished. Now sleeping for " << sleeping_time
				<< " seconds." << std::endl;

		cv_main_sleep.wait_for(cv_u_lock, std::chrono::seconds(sleeping_time));
	}

	return 0;
}