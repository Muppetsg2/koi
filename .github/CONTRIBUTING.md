Pull Requests and Issues are both welcome.

# Responsiveness

General priority order is:

* Crashes
* Security issues in stb_image
* Bugs
* Security concerns in other libs
* Warnings
* Enhancements (new features, performance improvement, etc)

Pull requests get priority over Issues. Some pull requests I take
as written; some I modify myself; some I will request changes before
accepting them.

# Pull requests

* Make sure you're using a special branch just for this pull request. (Sometimes people unknowingly use a default branch, then later update that branch, which updates the pull request with the other changes if it hasn't been merged yet.)
* Do NOT update the version number in the file. (This just causes conflicts.)
* Do add your name to the list of contributors. (Don't worry about the formatting.) I'll try to remember to add it if you don't, but I sometimes forget as it's an extra step.
* Your change needs to compile as both C and C++. Pre-C99 compilers should be supported (e.g. declare at start of block)