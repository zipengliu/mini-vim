mini-vim
========

OVERVIEW
===
A mini version vim implementation.  It is a command line text editor,
based on the ncurses library.  It supports basic file editing like
insert and delete, and regular expression searching.

AUTHOR
===
Zipeng Liu
Zhaoqian Lan

INSTALL
===

* You need to install ncurses library (`apt-get install ncurses-dev` on
  Ubuntu).
* make

USAGE
===

* Files
  - `./minivim` to open a new buffer
  - `./minivim [FILE]` to open a file buffer specified
* Navigation
  - vim like navigation
  - number + hjkl for fast jump
  - ctrl-u, d, b, f to scroll half or one screen
* Search
  - type "/" to enter search mode, and then input a regular expression.
  - in control mode, type "n" to locate the next match, and "N" the
    previous
* Others
  - `:w FILE` for saving to file
  - `:q` or `:q!` for quiting minivim (WARNING: do save your edited file
    before quiting!)


