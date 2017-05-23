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
#include <cassert>
#include <git2.h>
#include <functional>
#include "gitupdater.h"

class GitRemote
{
public:
	GitRemote(git_repository *repo, const char *remote_name): m_remote_name(remote_name)
	{
		assert(m_remote_name);
		m_fail = git_remote_lookup(&m_remote, repo, remote_name) != GIT_OK;
	}

	~GitRemote()
	{
		if (git_remote_connected(m_remote)) {
			git_remote_disconnect(m_remote);
		}

		git_remote_free(m_remote);
	}

	GitRemote &fetch()
	{
		assert(m_remote);

		if (m_fail) {
			return *this;
		}

		git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
		fetch_opts.prune = GIT_FETCH_NO_PRUNE;
		fetch_opts.download_tags = GIT_REMOTE_DOWNLOAD_TAGS_ALL;
		fetch_opts.callbacks.credentials = GitRemote::cred_acquire_cb;
		git_strarray refspecs = {};
		if (git_remote_get_fetch_refspecs(&refspecs, m_remote) != GIT_OK) {
			const git_error *e = giterr_last();
			std::cerr << "Unable to fetch refspecs for remote " << m_remote_name
					<< ". Error was: " << e->message << std::endl;
			return *this;
		}

		if (git_remote_fetch(m_remote, &refspecs, &fetch_opts, "fetch") != GIT_OK) {
			const git_error *e = giterr_last();
			std::cerr << "Unable to fetch remote " << m_remote_name
					<< ". Error was: " << e->message << std::endl;
			return *this;
		}

		return *this;
	}

	static int cred_acquire_cb(git_cred **out,
			const char * url,
			const char * username_from_url,
			unsigned int allowed_types,
			void * payload)
	{
		return git_cred_ssh_key_from_agent(out, username_from_url);
	}

	const bool failed() const { return m_fail; }

private:
	bool m_fail = false;
	git_remote *m_remote = nullptr;
	const char *m_remote_name = nullptr;
};

class GitRepository
{
public:
	GitRepository(const std::string &path)
	{
		m_fail = git_repository_open(&m_repo,
				std::string(path + "/.git").c_str()) != GIT_OK;
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