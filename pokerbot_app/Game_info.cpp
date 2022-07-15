#include "Game_info.h"

Game_info::Game_info()
{}

void Game_info::init(int id_chat_input, int f_mode) //инициализация на основе идентификатора чата
{
	switch (f_mode)
	{
	case MODE_NEW_PROFILE: //режим создания нового профиля игры и его запись в файл
		id_chat = id_chat_input;
		f_game_stage = GAME_NOT_STARTED;

		write_to_file(); //запись значений в файл
		break;

	case MODE_EXISTING_PROFILE: //режим чтения существующего профиля из файла
		id_chat = id_chat_input;
		read_from_file(); //чтение значений из файла
		break;
	}
}

void Game_info::read_from_file() //чтение значений из файла (должно быть уже установлено значение id_chat)
{
	ifstream fin;
	fin.open(FOLDER + to_string(id_chat) + TYPE_OF_PROFILE_FILE, ios::in); //открыть файл для чтения
	fin >> f_game_stage;

	fin.close(); //закрыть файл
}

void Game_info::write_to_file() //запись значений в файл (перезапись, если файл уже был создан)
{
	ofstream fout;
	fout.open(FOLDER + to_string(id_chat) + TYPE_OF_PROFILE_FILE, ios::out); //открыть файл для записи
	fout << id_chat << endl;
	fout << f_game_stage << endl;

	fout.close(); //закрыть файл
}

void Game_info::start_new_game(TgBot::Bot* bot, TgBot::Message::Ptr message) //начать новую игру
{
	f_game_stage = NEW_GAME_STARTED;

	bot->getApi().sendMessage(message->chat->id, "Запущена новая игра.");





	write_to_file(); //запись значений в файл
}