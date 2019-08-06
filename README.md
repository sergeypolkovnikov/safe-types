# safe-types

# Motivation
The developers use many types in programm, but frequently use only the limited set from a language for its representation (like int, float, char, std::string etc). It may cause some problems in runtime. 

For example we should implement chess game. In that case you should have some class which represents a square on a board. Each square can be shown with file and rank (e. g. 'e4' - 5-th file and 4-th rank on a board). Ok, let's try to implement it
~~~C++
class Square
{
public:
	explicit Square(char file_, int rank_): file {file_}, rank {rank_}
	{}
private:
	char file;
	int rank;
};
~~~
It's a pity but the following code is still compilable
~~~C++
	Square correct('e', 4);
	Square incorrect(4, 'e');
~~~
We have missed file and rank order and compiler can't do anything about it (it can show warning only). 

Ok. let's try to implement different types for each entity in our program
~~~C++
class FileType
{
	public:
	explicit FileType(char file_): file {file_}
	{}
private:
	char file;
};

class RankType
{
	public:
	explicit RankType(char rank_): rank {rank_}
	{}
private:
	char rank;
};

class Square
{
public:
	explicit Square(FileType file_, RankType rank_): file {file_}, rank {rank_}
	{}
private:
	FileType file;
	RankType rank;
};

int main() {
	Square correct(FileType{'e'}, RankType{4});
	// Square incorrect(RankType{4}, FileType{'e'});
	// error: no matching function for call to ‘Square::Square(RankType, FileType)’
	return 0;
}
~~~
It works as expected. But at this stage we have to think about arithmetic and comparing operations with each new type. If we implement each new operators (like +, -, ==, <=, >=, <, >, !=), our code will grown unexpectedly. At this stage many developers prefer first unsafe code.

This library gives the opportunity to implement new type with no much effort
~~~C++
class FileDimension;
using FileType = safe_type::singleton<char, FileDimension>;

class RankDimension;
using FileType = safe_type::singleton<int, RankDimension>;

class Square
//see below
~~~

# References
* lib catch is used for unit tests https://github.com/catchorg/Catch2
* library chrono (msvc implementation) is used like example for most arithmetic functions
* reply from Jarod42 https://stackoverflow.com/questions/23855712/how-can-a-type-be-removed-from-a-template-parameter-pack is used like example for struct remove
