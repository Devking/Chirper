all:
	@g++ -std=c++11 dataserver.cpp mappings.cpp queries.cpp -o dataserver
	@echo "Files compiled successfully. 'dataserver' executable created."

clean:
	@test -s dataserver || { echo "No 'dataserver' executable to clean!"; exit 1; }
	@rm dataserver
	@echo "'dataserver' executable successfully removed."

run: all
	@./dataserver
