#include <Windows.h>
#include <iostream>
#include <string>
#define BOOST_DATE_TIME_NO_LIB
#include <boost/interprocess/windows_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
using namespace std;

int main()
{
	using namespace boost::interprocess;
	setlocale(0, "rus");
	constexpr size_t dataSize = 10;
	constexpr size_t syncSize = sizeof(interprocess_mutex)+
								2*sizeof(interprocess_semaphore)+
								sizeof(size_t);//dataSize
	constexpr size_t bufferSize = dataSize + syncSize;
	string sharedMemoryName("MySharedMemory");

	windows_shared_memory shm(create_only,
		sharedMemoryName.c_str(),
		read_write,
		bufferSize);
	mapped_region region(shm, read_write);

	
	byte* syncAdress = static_cast<byte*>(region.get_address());
	interprocess_mutex* mutex = new(syncAdress) interprocess_mutex;
	interprocess_semaphore* full =	new(syncAdress +
									sizeof(interprocess_mutex))
									interprocess_semaphore(0);
	interprocess_semaphore* empty = new(syncAdress + 
									sizeof(interprocess_mutex) + 
									sizeof(interprocess_semaphore))
									interprocess_semaphore(dataSize);
	*(syncAdress + sizeof(interprocess_mutex) +
		2 * sizeof(interprocess_semaphore)) = dataSize;
	int* syncAddrInt = reinterpret_cast<int*>(syncAdress);
	while (true)
	{
		/*for (std::size_t i = 0; i < 3; ++i)
		{
			cout << (int) * (syncAdress + 4 * i) << " "
				<< (int) * (syncAdress + 1 + 4 * i) << " "
				<< (int) * (syncAdress + 2 + 4 * i) << " "
				<< (int) * (syncAdress + 3 + 4 * i) << endl;
			
		}*/
		cout << "Mutex: " << syncAddrInt[0] << endl;
		cout << "Full: " << syncAddrInt[1] << endl;
		cout << "Empty: " << syncAddrInt[2] << endl;
		cout << "dataSize: " << (size_t)syncAddrInt[3] << endl;
		Sleep(1000);
		system("cls");
	}
	system("pause");
	return 0;
}