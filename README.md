angry-monkeys
=============

ECE 2035 Final Project (Fall 2013)

Project Assignment details:
http://www.ece.gatech.edu/academic/courses/ece2035/assignments/Fall13/Project2/New%20Project2.pdf

TL;DR : A jank-ass Angry Birds clone. 

Install
-------

You will need a linux system with Python 2.7, Tkinter, and libsocket. See readme-socket.txt for more.
Then use make to build the backend. This requires g++, make, and related buildutils. Even with all this,
the code included with the assignment description (socket communication and GUI) is buggy. It ususally works
okay on 32-bit Ubuntu 12.04.3 LTS. The python script won't run at all on more recent versions of Tkinter
(tested on Arch with tk 8.6.1).


Play
----
start the GUI
  python P2_main.py
start the backend
  ./main

Then you can use the keypad to play the game.
Z - fire cannon
X - decrease angle
C - increase angle
V - toggle power
H - give me a hint
R - reset
Q - quit
D - ???


Notes
-----

My only work was on parts of main.cpp. The rest was included with the assignment.

I added some extra features. Use H to get a hint to kill the next monkey (not just the first one).

I altered the destruction algorithm to be more game-like from the assignment requirements. Trees
only die when they have been completely cut through.

I fixed some of the more buggy parts of the code included with the assignment. World data is stored in the heap,
and when it fails to initalize correctly (a bug widespread in the class), it retries.

The game ends when you run out of bombs or kill all the monkeys.

Levels are in a pretty simple CSV format if you want to make your own.
