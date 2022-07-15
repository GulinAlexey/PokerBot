#pragma once
#include <fstream>
#include <iostream>

#define GAME_NOT_STARTED 0 //значение флага, что игрок ещё не начал новую игру

#define MODE_NEW_PROFILE 0 //режим создания нового профиля игры и его запись в файл
#define MODE_EXISTING_PROFILE 1 //режим чтения существующего профиля из файла

#define FOLDER "profiles/" //папка для сохранения данных пользователей

#define TYPE_OF_PROFILE_FILE ".dat" //тип файла для сохранения данных пользователей

using namespace std;

class Game_info //информация об игре для отдельного пользователя
{
private:
	int id_chat; //id чата игрока
	int f_game_stage; //флаг текущей стадии игры


public:
	Game_info();
	void init(int id_chat_input, int f_mode); //инициализация на основе идентификатора чата
	void read_from_file(); //чтение значений из файла (должно быть известно значение id_chat)
	void write_to_file(); //запись значений в файл (перезапись, если файл уже был создан)
};

