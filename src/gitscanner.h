#pragma once

#include <string>
#include <vector>

class GitScanner
{
	friend class GitUpdater;
public:
	GitScanner(uint16_t depth_limit = 10);
	~GitScanner() {}
private:
	void scan();
	void scan_folder(const std::string &path, uint16_t depth);
	std::vector<std::string> m_git_repositories = {};
	uint16_t m_depth_limit = UINT16_MAX;
};