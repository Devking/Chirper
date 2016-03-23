all:
	@g++ -std=c++11 server.cpp -o server
	@echo "Files compiled successfully. 'server' executable created."

clean:
	@test -s server || { echo "No 'server' executable to clean!"; exit 1; }
	@rm server
	@echo "'server' executable successfully removed."

run:
	@g++ -std=c++11 server.cpp -o server
	@./server
