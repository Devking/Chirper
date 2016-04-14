all:
	@g++ -std=c++11 server.cpp api_mapping.cpp queries.cpp -o server
	@echo "Files compiled successfully. 'server' executable created."

clean:
	@test -s server || { echo "No 'server' executable to clean!"; exit 1; }
	@rm server
	@echo "'server' executable successfully removed."

run: all
	@./server
