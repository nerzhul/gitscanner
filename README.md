# GitScanner

Gitscanner is a small tool which scan your homedir and use your SSH_AUTH_SOCK to fetch
automatically your local repository remotes and keep them update-to-date.

It's written in C++11.

# Next features

At this moment, gitscanner only fetches remotes on all repositories. It's planned to add

* daemon mode
* custom wait duration
* local branch pull (rebase) if no uncommited files are present