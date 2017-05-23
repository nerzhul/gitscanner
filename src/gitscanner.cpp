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
	if (depth >= m_depth_limit) {
		return;
	}

	for (const auto &p : fs::directory_iterator(path)) {
		try {
			if (p.status().type() == fs::file_type::directory) {
				// Ignore hidden files
				if (p.path().filename().string()[0] != '.') {
					scan_folder(p.path(), depth + 1);
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