#include <Windows.h>
#include <iostream>
#define BOOST_DATE_TIME_NO_LIB
#include <boost/interprocess/windows_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <thread>
#include <condition_variable>
#include <mutex>
using namespace std;
condition_variable cv;
mutex mtx;

void receive(string src,byte* message,size_t &toReceive) {
	using namespace boost::interprocess;
	windows_shared_memory shm(	open_only,
								src.c_str(),
								read_write);
	mapped_region region(shm, read_write);

	byte* syncAdress = static_cast<byte*>(region.get_address());
	interprocess_mutex* mutex = (interprocess_mutex*)syncAdress;
	interprocess_semaphore* full = reinterpret_cast<interprocess_semaphore*>(syncAdress +
		sizeof(interprocess_mutex));
	interprocess_semaphore* empty = reinterpret_cast<interprocess_semaphore*>(syncAdress +
		sizeof(interprocess_mutex) +
		sizeof(interprocess_semaphore));
	size_t dataSize = *(syncAdress +
		sizeof(interprocess_mutex) +
		2 * sizeof(interprocess_semaphore));
	byte* dataAddr = syncAdress +
		sizeof(interprocess_mutex) +
		2 * sizeof(interprocess_semaphore) +
		sizeof(size_t);
	size_t i = 0,j = 0;
	while (1) {
		full->wait();
		mutex->lock();
		message[j++] = dataAddr[i];
		mutex->unlock();
		empty->post();
		i = i == dataSize - 1 ? 0 : ++i;

		if (--toReceive == 0) {
			cv.notify_one();
		}
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
		
		size_t toReceive = sizeof(A);
		A* obj2 = new A;
		
		string name = "MySharedMemory";
		thread th(receive,name, reinterpret_cast<byte*>(obj2), ref(toReceive));
		th.detach();
		
		unique_lock<mutex> ul(mtx);
		cv.wait(ul, [&toReceive]() {
			return toReceive == 0;
		});

		cout << obj2->a << " " << obj2->b << endl;
	}
	catch (const std::exception& exc)
	{
		cout << exc.what() << endl;
	}

	system("pause");
	return 0;
}


