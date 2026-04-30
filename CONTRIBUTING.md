AI AND LLM ARE FORBIDDEN
========================

This project doesn't accept code or documentation generated
by LLMs or any other forms of generative AI.

Please don't submit pull requests based on generative AI
output, and don't use AI auto completion when you write
pull requests.

Additionally, don't use LLMs to create pull requests or issues.
Write them yourself, in English, even if your English is bad.

[Here are some reasons to reject LLMs](https://codeberg.org/small-hack/open-slopware#why-not-llms)


Contributions Welcome
=====================

Pull Requests and Issues are both welcome.


### Responsiveness

General priority order is:

* Crashes
* Security issues in koi_image or koi_image_write
* Bugs
* Warnings
* Enhancements (new features, performance improvement, etc)

Pull requests get priority over Issues. Some pull requests I take
as written; some I modify myself; some I will request changes before
accepting them.


### Pull requests

* Make sure you're using a special branch just for this pull request. (Sometimes people unknowingly use a default branch, then later update that branch, which updates the pull request with the other changes if it hasn't been merged yet.)
* Do NOT update the version number in the file. (This just causes conflicts.)
* Do add your name to the list of contributors. (Don't worry about the formatting.) I'll try to remember to add it if you don't, but I sometimes forget as it's an extra step.
* Your change needs to compile as both C and C++. Pre-C99 compilers should be supported (e.g. declare at start of block)
