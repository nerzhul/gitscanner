# GitScanner

Gitscanner is a small tool which scan your homedir and use your SSH_AUTH_SOCK to fetch
automatically your local repository remotes and keep them update-to-date.

It's written in C++11.

# Command line options

* __-d__: daemon mode. Run process in background
* __-i <seconds>__: how many time to sleep between 2 scans (default: 60sec)

# Signals handled

* __SIGINT__, __SIGTERM__: terminate program after a finished scan or when program is sleeping.
* __SIGHUP__: awake sleeping program

# Next features

At this moment, gitscanner only fetches remotes on all repositories. It's planned to add

* local branch pull (rebase) if no uncommited files are present