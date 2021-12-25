#include "stdafx.h"
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#pragma warning(disable: 4996)

const char* IP1 = "127.0.0.1";
const char* IP2 = "127.0.0.2";
const u_short Port1 = 1111;
const u_short Port2 = 1112;
bool ConnectToServer1 = false;
bool ConnectToServer2 = false;

void CreateConnect(int numServ) {

	// структура для хранения адреса
	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);

	//адрес и порт для подключения устанавливаются в соответствии с номером сервера
	addr.sin_addr.s_addr = numServ== 1 ? inet_addr(IP1) : inet_addr(IP2);//ip - local host
	addr.sin_port = numServ == 1 ? htons(Port1) : htons(Port2);//незарезервированный хост
	addr.sin_family = AF_INET;//константа для интернет протокола

	//создание сокета
	SOCKET Connection = socket(
		AF_INET,//семейство интернет протоколов
		SOCK_STREAM,//протокод соединения
		NULL);

	//осуществление подключение и проверка на успех
	if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
		std::cout << "Попытка подключения к серверу не удалась.\n";
		return;
	}

	//Для того, чтобы не было повторных подключений
	if (numServ == 1)ConnectToServer1 = true;
	else ConnectToServer2 = true;

	std::cout << "Попытка соединения с сервером " << numServ<< " удалась!\n";
	char msg[256];

	//чтение всех сообщений и вывод их на экран
	while (true) {

		//если не вознимает ошибки читаем и выводим сообщение
		if (recv(Connection, msg, sizeof(msg), NULL) == SOCKET_ERROR) break;
		else std::cout << msg << std::endl;;
	}
}


//Выполняется в главном потоке рекурсивно непрерывно, для распознования команд пользователя
void Menu() {

	int result;
	std::cin >> result;

	switch (result)
	{
	//для подключения к двум серверам используются отдельные потоки, которые выполняют функции подключения
	case 1: {
		//логическая переменная ConnectToServer1 позволяет проверить создано ли уже подключение
		if (!ConnectToServer1) {
			//в поток передается функция для подключения (LPTHREAD_START_ROUTINE)CreateConnect и номер сервера  (LPVOID)(1)
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)CreateConnect, (LPVOID)(1), NULL, NULL);
			
		}
		else {
			std::cout << "Нельзя повторно подключиться к серверу!";}
		Menu();
		break;
	}
	case 2: {
		//логическая переменная ConnectToServer2 позволяет проверить создано ли уже подключение
		if (!ConnectToServer2) {
			//в поток передается функция для подключения (LPTHREAD_START_ROUTINE)CreateConnect и номер сервера (LPVOID)(2)
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)CreateConnect, (LPVOID)(2), NULL, NULL);
		}
		else{
			std::cout << "Нельзя повторно подключиться к серверу!";}
		Menu();
		break;
	}

	default:
		std::cout << "Некорректная команда!";
		Menu();
		break;
	}
}


int main(int argc, char* argv[]) {

	//установка русского языка для вывода в консоль
	setlocale(LC_ALL, "Russian");

	//Загрузка библиотеки для осуществления подключния
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		exit(1);
	}

	//вывод меню для работы с приложением
	std::cout << "\nВас приветствует клиент курсовой работы\n\nМожно осуществить подключение к серверам\n1-Получение информации с первого сервера\n2-Получение информации со второго сервера\n\n";
	Menu();
	
	//окончание работы
	system("pause");
	return 0;
}



