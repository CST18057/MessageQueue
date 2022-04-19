CPP_STANDARD=c++17

.PHONY:clean main
main: main.o ojSocket.o configure.o jsonObject.o log.o
	g++ -o main main.o ojSocket.o configure.o jsonObject.o log.o
main.o: main.cpp fileio.h configure.h jsonObject.h log.h
	g++ -c main.cpp -std=$(CPP_STANDARD)
ojSocket.o: ojSocket.h ojSocket.cpp
	g++ -c ojSocket.cpp -std=$(CPP_STANDARD)
fileio.o: fileio.h fileio.cpp 
	g++ -c fileio.cpp -std=$(CPP_STANDARD)
configure.o: configure.h configure.cpp
	g++ -c configure.cpp -std=$(CPP_STANDARD)
jsonObject.o: jsonObject.h jsonObject.cpp
	g++ -c jsonObject.cpp -std=$(CPP_STANDARD)
log.o: log.h log.cpp
	g++ -c log.cpp -std=$(CPP_STANDARD)
clean:
	-rm *.o
	-rm main