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
#include <git2/repository.h>
#include <git2/remote.h>
#include "gitupdater.h"

class GitRemote
{
public:
	GitRemote(git_repository *repo, const char *remote_name)
	{
		git_remote_lookup(&m_remote, repo, remote_name);
	}

	~GitRemote()
	{
		git_remote_free(m_remote);
	}

	GitRemote &fetch()
	{
		git_fetch_options fetch_options = GIT_FETCH_OPTIONS_INIT;
		fetch_options.download_tags = GIT_REMOTE_DOWNLOAD_TAGS_ALL;
		fetch_options.prune = GIT_FETCH_PRUNE;
//			git_remote_fetch(remote, nullptr, &fetch_options, nullptr);
		return *this;
	}
private:
	git_remote *m_remote = nullptr;
};

class GitRepository
{
public:
	GitRepository(const std::string &path)
	{
		m_fail = git_repository_open(&m_repo, std::string(path + "/.git").c_str()) != 0;
		m_remotes.strings = nullptr;
		m_remotes.count = 0;
	}

	~GitRepository()
	{
		if (!m_fail) {
			git_repository_free(m_repo);
		}
	}

	GitRepository &list_remotes()
	{
		if (m_fail) {
			return *this;
		}

		git_strarray_free(&m_remotes);
		git_remote_list(&m_remotes, m_repo);
		return *this;
	}

	GitRepository &fetchall()
	{
		if (m_fail) {
			return *this;
		}

		for (uint8_t i = 0; i < m_remotes.count; i++) {
			GitRemote(m_repo, m_remotes.strings[i]).fetch();
		}
		return *this;
	}

private:
	bool m_fail = false;
	git_repository *m_repo;
	git_strarray m_remotes;
};

GitUpdater& GitUpdater::operator<<(const GitScanner &scanner)
{
	for (const auto &path: scanner.m_git_repositories) {
		std::cout << "Fetching repository path: " << path << std::endl;
		GitRepository(path)
				.list_remotes()
				.fetchall();
	}

	return *this;
}