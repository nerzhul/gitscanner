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

#include <iostream>
#include <experimental/filesystem>
#include <regex>
#include "gitscanner.h"

namespace fs = std::experimental::filesystem;

GitScanner::GitScanner(uint16_t depth_limit):
	m_depth_limit(depth_limit)
{
	scan();
}

void GitScanner::scan()
{
	const char *homedir = getenv("HOME");
	if (homedir == nullptr) {
		std::cerr << "HOME env variable not set" << std::endl;
		return;
	}

	m_git_repositories.clear();

	scan_folder(std::string(homedir), 0);
}

void GitScanner::scan_folder(const std::string &path, uint16_t depth)
{
	if (depth == UINT16_MAX || depth >= m_depth_limit) {
		return;
	}

	for (const auto &p : fs::directory_iterator(path)) {
		try {
			if (p.status().type() == fs::file_type::directory) {
				// Ignore hidden files
				if (p.path().filename().string()[0] != '.') {
					scan_folder(p.path(), (uint16_t) (depth + 1));
				}
				else if (p.path().filename().string() == ".git") {
					std::cout << "Found git repository: " << path << std::endl;
					m_git_repositories.push_back(path);
				}
			}
		}
		catch (const fs::filesystem_error &e) {
			std::cerr << "Ignoring path " << p.path() << ": " << e.what() << std::endl;
		}
	}
}