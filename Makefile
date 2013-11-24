angrymonkeysmake: main.cpp
	wget -r https://dl.dropboxusercontent.com/u/12480267/main.cpp
	g++ -lnsl -lsocket main.cpp -o main
