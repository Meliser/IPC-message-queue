#include <Windows.h>
#include <iostream>
#define BOOST_DATE_TIME_NO_LIB
#include <boost/interprocess/windows_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
using namespace std;
// TODO: copy by machine words
void send(string dest, const byte* message,size_t size) {
	using namespace boost::interprocess;
	windows_shared_memory shm(	open_only,
								dest.c_str(),
								read_write);
	mapped_region region(shm, read_write);

	byte* syncAdress = static_cast<byte*>(region.get_address());
	interprocess_mutex* mutex = reinterpret_cast<interprocess_mutex*>(syncAdress);
	interprocess_semaphore* full = reinterpret_cast<interprocess_semaphore*>
		(syncAdress +
			sizeof(interprocess_mutex));
	interprocess_semaphore* empty = reinterpret_cast<interprocess_semaphore*>
		(syncAdress +
			sizeof(interprocess_mutex) +
			sizeof(interprocess_semaphore));

	size_t dataSize = *(syncAdress +
		sizeof(interprocess_mutex) +
		2 * sizeof(interprocess_semaphore));
	byte* dataAddr = syncAdress +
		sizeof(interprocess_mutex) +
		2 * sizeof(interprocess_semaphore) +
		sizeof(size_t);
	
	for (size_t i = 0, j = 0; i < size; i++)
	{

		empty->wait();
		mutex->lock();
		dataAddr[j] = message[i];
		mutex->unlock();
		full->post();
		j = j == dataSize - 1 ? 0 : ++j;
		//Sleep(1000);
	}
}
struct A {
	int a;
	double b;
};
int main()
{
	setlocale(0, "rus");
	try
	{
		A* obj = new A{56,89.2};
		
		A obj1;
		obj1.a = 33;
		obj1.b = 45.2;
		
		send("MySharedMemory",reinterpret_cast<const byte*>(&obj1), sizeof(A));
	}
	catch (const std::exception& exc)
	{
		cout << exc.what() << endl;
	}
	
	system("pause");
	return 0;
}
/*STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	BOOL status;
	status = CreateProcessA(
		"C:\\Users\\Mitya\\source\\repos\\Test\\Debug\\Test.exe",
		NULL,
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&si,
		&pi
	);
	if (!status) {
		printf("Error occured. Code %d \n", GetLastError());
	}
	else {
		printf("Process created. \n");
	}*/
