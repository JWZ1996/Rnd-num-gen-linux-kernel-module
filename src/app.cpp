#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <math.h>
#include <cstring>

typedef char byte;

template<typename T>
struct Range
{
	Range(T bottomVal, T topVal) : bottom{bottomVal}, top{topVal}{}
	T bottom {0};
	T top {0};
	inline T getRng() {return top - bottom;}
};

template <typename T>
class RndGen
{
public:
	static std::string path;
	static T get() 
	{
		T  randNr {0};
		std::array<byte,sizeof(T)> byteArr;
		std::ifstream myFile;

		try
		{
			myFile.open(path, std::ios::in | std::ios::binary);
			myFile.read(byteArr.data(), sizeof(T));
			myFile.close();

			memcpy(&randNr,byteArr.data(),sizeof(T));
		}
		catch (const std::exception& e)
		{
			std::cout << "Exception occured: " << e.what() <<"\n File will be closed.";

			if(myFile.is_open())
				myFile.close();
		}		
		return randNr;
	}
	static T get(Range<T> rng)
	{
		return (abs(RndGen::get()) % rng.getRng()) + rng.bottom; 
	};
	
};

template<typename T>
std::string RndGen<T>::path {"/dev/char/240:0"};

// =====================
// ===== Main Part =====
// =====================


int main()
{
	std::cout << "Random nr "<< RndGen<int>::get(Range<int>(0,10)) << '\n';
	return 0;
}