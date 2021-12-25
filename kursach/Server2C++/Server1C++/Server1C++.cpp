#include "stdafx.h"
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <string>
#include <ctime>
#include <chrono>
#include <iostream>
#include <tchar.h>
#include <Windows.h>
#include <tlhelp32.h>
using namespace std;

#pragma warning(disable: 4996)

const int Max_Connect = 20;
SOCKET Connect_array[Max_Connect];
string Message[Max_Connect];

string GetThreadsInform() {

	string result = "";
	int threadsCount = 0;

	//получает информацию о потоке с помощью моментального снимка процессов и потоков
	HANDLE hSnapshot;
	//Список всех потоков
	THREADENTRY32 teTreadEntry;
	teTreadEntry.dwSize = sizeof(THREADENTRY32);
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());//снимок 
	Thread32First(hSnapshot, &teTreadEntry);//первый поток

	result += "\nПотоки:\n";
	do {
		if (teTreadEntry.th32OwnerProcessID == GetCurrentProcessId()) {//если поток текущего процесса, записываем в результат
			result += "ID: "+std::to_string(teTreadEntry.th32ThreadID)+"\n";
			result += "Приоритет: "+ std::to_string(teTreadEntry.tpBasePri)+"\n";
		}
		threadsCount= threadsCount+1;// считается колличество потоков
	} while (Thread32Next(hSnapshot, &teTreadEntry));// перебор потоков
	CloseHandle(hSnapshot);
	result += "\nКоличество: "+ std::to_string(threadsCount) + "\n";
	return result;
}


void NewMessageTime() {

	auto mytime = std::chrono::system_clock::now();
	std::time_t timeSent = std::chrono::system_clock::to_time_t(mytime);
	std::cout << "Отправка очередного сообщения завершена!  " << std::ctime(&timeSent) << "\n";
}



void WorkWithClient(int index) {

	Message[index] = GetThreadsInform();
	
	char newMess[256];
	//преобразование строки к массиву символов для отправки
	strcpy(newMess, Message[index].c_str());
	//отправка сообщения
	send(Connect_array[index], newMess, sizeof(newMess), NULL);
	//вывод времени отправки
	NewMessageTime();

	//каждые 10 секунд сервер вычисляет информацю заново и если она изменилась, то отправляет снова клиенту
	while (true) {

		Sleep(10000);
		string inf= GetThreadsInform();

		if (Message[index] != inf) {

			Message[index] = inf;
			strcpy(newMess, Message[index].c_str());

			//если клиент отключися перестает отправляться
			if (send(Connect_array[index], newMess, sizeof(newMess), NULL) == SOCKET_ERROR) return;
			else NewMessageTime();//если соединение нормальное и отправка завершена, печатается время отправления
		}
	}
}


int main(int argc, char* argv[]) {

	setlocale(LC_ALL, "Russian");

	//создется мьютекс, которым уже завладевают
	HANDLE hMutex = CreateMutex(NULL, TRUE, L"Mutex2");
	//если пытаются открыть несколько экземпляров сервера, то они не работают и закрываются
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		exit(0);

	}
	std::cout << "\nВас приветствует сервер курсовой работы №2\n\nСервер ожидает подключения.............\n";

	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		exit(1);
	}

	SOCKADDR_IN addr;// структура для хранения адреса
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.2");//ip - local host
	addr.sin_port = htons(1112);//незарезервированный хост
	addr.sin_family = AF_INET;//константа для интернет протокола

	SOCKET sListen = socket(AF_INET,// семейство интернет протоколов
		SOCK_STREAM,//протокод соединения
		NULL);

	bind(sListen,(SOCKADDR*)&addr,sizeof(addr));

	listen(sListen, Max_Connect);

	SOCKET newConnection;
	for (int i = 0; i < Max_Connect; i++) {

		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

		if (newConnection == 0) {
			std::cout << "Не удалось выполнить устойчвое подключение! " << i + 1 << "\n";
		}
		else
		{
			std::cout << "Клиент" << i + 1 << " успешно подключился!" << "\n";
			Connect_array[i] = newConnection;
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)WorkWithClient, (LPVOID)(i), NULL, NULL);
		}
	}


	system("pause");
	return 0;
}