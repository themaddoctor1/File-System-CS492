# File System Assignment #

This repository contains our code for the third assignment for CS 496.

### What is this repository for? ###

The assignment was to simulate a file system using C. We had to develop several features, including the ability to manage files. This required the creation of mechanisms that allowed for files and directories to be created, modified, and destroyed.

### How do I get set up? ###

To set up the repository, simply download the source files (including the Makefile). To build the program, simply run `make all`. Other useful make instructions can be viewed in the Makefile. To execute, simply run the executable `exec` with any of the following flags:

Flag | Description 
-----|------------
`-b [LONG]` | Specifies the size of a block in the system memory.
`-d [FILENAME]` | Provides the location of a file containing a list of directories to include.
`-f [FILENAME]` | Provides the location of a file containing a list of files to include.
`-s [LONG]` | Specifies the capacity of the system memory.

Note that files provided with `-d` must be built by running `find [DIR] -type d > [FILENAME]` with a directory, or must be shaped to have the same format. Similarly, files provided with `f` must be built with `find [DIR] -type f -ls > [FILENAME]` with a directory of choice. If a path cannot be resolved for some of the files, they will not be added.

The default sizes for the simulated filesystem are to use 512B blocks with a 64kB capacity. If a block size or a disk size are not given, a warning will be thrown to notify the user of the default values. If files are too big to fit in remaining space, an error will be thrown and the file will be skipped.

### How do I use the filesystem? ###

The file system simulator comes with the ability to run several bash-like commands. These include:

Command | Description
--------|------------
`cd [DIR (optional)]` | Changes directory. Acts nearly identically to bash.
`ls [DIR (optional)]` | Lists the contents of the directory. Acts similarly to `ls -al`.
`mkdir [DIR1] [DIR2] ...` | Creates a set of directories. Acts nearly identically to bash.
`create [FILE1] [FILE2] ...` | Creates a set of files. Will not create file if owner directory doesn't exist.
`append [FILE] [LONG]` | Appends a number of bytes to a file.
`remove [FILE] [LONG]` | Remove a number of bytes from a file.
`delete [FILE1] [FILE2] ...` | Removes a set of files or directories.
`exit [NUM (optional)]` | Exits the system. Optionally return an error code.
`dir [PATH (optional)]` | Does a BFS print of the file system.
`prfiles [PATH (optional)]` | Does a BFS print of all files with space analysis.
`prdisk` | Displays disk allocation information.

### Honor Pledge ###
We pledge our honor that we have abided by the Stevens Honor System.

`Christopher Hittner`

`James Romph`