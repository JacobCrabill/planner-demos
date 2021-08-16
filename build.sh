g++ -std=c++17 -DFORCE_EXPERIMENTAL_FS main.cpp -c -o main.o
g++ -std=c++17 -DFORCE_EXPERIMENTAL_FS main.o -o main  -lX11 -lGL -lpthread -lpng -lstdc++fs
