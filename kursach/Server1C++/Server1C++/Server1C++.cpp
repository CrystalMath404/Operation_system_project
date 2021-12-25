#include "stdafx.h"
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <string>
#include <ctime>
#include <chrono>
using namespace std;
#pragma warning(disable: 4996)


const int Max_Connect = 20;
SOCKET Connect_array[Max_Connect];

int CursorWidth[Max_Connect];
int CursorHeight[Max_Connect];
int MouseButtonsCount[Max_Connect];

void NewMessageTime() {

	//вывод времени отправки
	auto timesent = std::chrono::system_clock::now();
	std::time_t timeSent = std::chrono::system_clock::to_time_t(timesent);
	std::cout << "Отправка очередного сообщения завершена! " << std::ctime(&timeSent) << "\n";
}

	//получение параметров
	CursorWidth[index] = GetSystemMetrics(SM_CXCURSOR);//ширина курсора
	CursorHeight[index] = GetSystemMetrics(SM_CYCURSOR);//высота курсора
	MouseButtonsCount[index] = GetSystemMetrics(SM_CMOUSEBUTTONS);//количество клавишь мыши

	//перобразование к массиву символов для отправки сообщения
	char metricsBuffer[256];
	sprintf(metricsBuffer, "Ширина курсора: %d пикселей, высота курсора: %d пикселей\nКоличество клавишь мыши: %d", CursorWidth[index], CursorHeight[index], MouseButtonsCount[index]);

	//отправка
	send(Connect_array[index], metricsBuffer, sizeof(metricsBuffer), NULL);
	NewMessageTime();

	while (true) {

		//задержка в 10 секунд
		Sleep(10000);
		//вычисление новых значение
		int newCursorWidth = GetSystemMetrics(SM_CXCURSOR);//ширина курсора
		int newCursorHeight = GetSystemMetrics(SM_CYCURSOR);//высота курсора
		int newMouseButtonsCount = GetSystemMetrics(SM_CMOUSEBUTTONS);//количество клавишь мыши
		//проверка изменились ли параметры по сравнению с прошлыми отправленными
		if (newCursorWidth!= CursorWidth[index] || newCursorHeight!= CursorHeight[index] || newMouseButtonsCount!= MouseButtonsCount[index]) {

			//если данные отличаются пересылка новых данных
			CursorWidth[index] = newCursorWidth;
			CursorHeight[index] = newCursorHeight;
			MouseButtonsCount[index] = newMouseButtonsCount;
			sprintf(metricsBuffer, "\nНовые данные с сервера:\nШирина курсора: %d, высота курсора: %d\nКоличество клавишь мыши: %d", CursorWidth[index], CursorHeight[index], MouseButtonsCount[index]);

			//если клиент отключися перестает отправляться
			if (send(Connect_array[index], metricsBuffer, sizeof(metricsBuffer), NULL) == SOCKET_ERROR) return;
			else NewMessageTime();//если соединение нормальное и отправка завершена, печатается время отправления
		}
	}
}



int main(int argc, char* argv[]) {

	//устаовка русского языка на консоли
	setlocale(LC_ALL, "Russian");

	//создется мьютекс, которым уже завладевают
	HANDLE hMutex = CreateMutex(NULL, TRUE, L"Mutex1");
	//если пытаются открыть несколько экземпляров сервера, то они не работают и закрываются
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		exit(0);
	}

	std::cout << "\nВас приветствует сервер курсовой работы №1\n\nСервер ожидает подключения.............\n";
	//Загрузка библиотеки 
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		exit(1);
	}

	//создание адреса
	SOCKADDR_IN addr;// структура для хранения адреса
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");//ip - local host
	addr.sin_port = htons(1111);//незарезервированный хост
	addr.sin_family = AF_INET;//константа для интернет протокола

	//создание сокета
	SOCKET sListen = socket(
		AF_INET,// семейство интернет протоколов
		SOCK_STREAM,//протокод соединения
		NULL);
	//привязка адреса сокету
	bind(sListen,(SOCKADDR*)&addr,sizeof(addr));
    //ожидание соединения
	listen(sListen, Max_Connect);

	SOCKET newMess;
	for (int i = 0; i < Max_Connect; i++) {

		//для каждого нового соединения открывается новый поток для отправки сообщений
		//в него передается функция отправки (LPTHREAD_START_ROUTINE)SentMyMetrics и номер соединения (LPVOID)(i) по которому отправлять
		newMess = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

		if (newMess == 0) {
			std::cout << "Не удалось выполнить устойчвое подключение! "<<"\n";
		}
		else 
		{
			std::cout << "Клиент"<<i+1<< " успешно подключился!"<<"\n";
			Connect_array[i] = newMess;
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)SentMyMetrics, (LPVOID)(i), NULL, NULL);
		}
	}

	system("pause");
	return 0;
}