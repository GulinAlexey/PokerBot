﻿#include "Game_info.h"

Game_info::Game_info()
{
	random_generator.seed(chrono::steady_clock::now().time_since_epoch().count()); //инициализация генератора случайных чисел
}

void Game_info::init(int id_chat_input, int f_mode, TgBot::Bot* bot, TgBot::Message::Ptr message) //инициализация на основе идентификатора чата
{
	//очистить векторы с картами перед их заполнением
	player_cards.clear();
	opponent_cards.clear();
	common_cards.clear();

	switch (f_mode)
	{
	case MODE_NEW_PROFILE: //режим создания нового профиля игры и его запись в файл
create_new_profile: //метка создания нового профиля (если не был найден существующий)
		id_chat = id_chat_input;
		f_game_stage = GAME_NOT_STARTED;
		is_player_should_bet_big_blind = 0;
		pot = 0;
		player_bet = 0;
		opponent_bet = 0;
		player_stack = DEFAULT_PLAYER_STACK;
		opponent_stack = DEFAULT_OPPONENT_STACK;
		big_blind = DEFAULT_BIG_BLIND;

		write_to_file(); //запись значений в файл
		break;

	case MODE_EXISTING_PROFILE: //режим чтения существующего профиля из файла
		id_chat = id_chat_input;
		if(read_from_file()==false) //чтение значений из файла (с проверкой на его существование)
		{
			bot->getApi().sendMessage(message->chat->id, "Файл с вашим профилем не был найден на сервере бота.\nСоздан новый профиль");
			goto create_new_profile; //перейти к созданию нового профиля, если не был найден файл с данными
		}
		break;
	}
}

bool Game_info::read_from_file() //чтение значений из файла (должно быть уже установлено значение id_chat)
{
	ifstream fin;
	fin.open(FOLDER + to_string(id_chat) + TYPE_OF_PROFILE_FILE, ios::in); //открыть файл для чтения
	if (fin.is_open() == false) //если файл с данными пользователя отсутствует
	{
		return false;
	}
	fin >> id_chat;
	fin >> f_game_stage;
	fin >> is_player_should_bet_big_blind;
	fin >> pot;
	fin >> player_bet;
	fin >> opponent_bet;
	fin >> player_stack;
	fin >> opponent_stack;
	fin >> big_blind;

	int qty_player_cards; //кол-во карманных карт игрока
	fin >> qty_player_cards;
	for (int i = 0; i < qty_player_cards; i++)
	{
		int card_rank, card_suit;
		fin >> card_rank;
		fin >> card_suit;
		Playing_card card(card_rank, card_suit);
		player_cards.push_back(card);
	}

	int qty_opponent_cards; //кол-во карманных карт соперника
	fin >> qty_opponent_cards;
	for (int i = 0; i < qty_opponent_cards; i++)
	{
		int card_rank, card_suit;
		fin >> card_rank;
		fin >> card_suit;
		Playing_card card(card_rank, card_suit);
		opponent_cards.push_back(card);
	}

	int qty_common_cards; //кол-во открытых общих карт
	fin >> qty_common_cards;
	for (int i = 0; i < qty_common_cards; i++)
	{
		int card_rank, card_suit;
		fin >> card_rank;
		fin >> card_suit;
		Playing_card card(card_rank, card_suit);
		common_cards.push_back(card);
	}

	fin.close(); //закрыть файл
	return true;
}

void Game_info::write_to_file() //запись значений в файл (перезапись, если файл уже был создан)
{
	ofstream fout;
	fout.open(FOLDER + to_string(id_chat) + TYPE_OF_PROFILE_FILE, ios::out); //открыть файл для записи
	fout << id_chat << endl;
	fout << f_game_stage << endl;
	fout << is_player_should_bet_big_blind << endl;
	fout << pot << endl;
	fout << player_bet << endl;
	fout << opponent_bet << endl;
	fout << player_stack << endl;
	fout << opponent_stack << endl;
	fout << big_blind << endl;

	fout << player_cards.size() << endl;
	for (int i = 0; i < player_cards.size(); i++)
	{
		fout << player_cards[i].get_rank() << " " << player_cards[i].get_suit() << endl;
	}

	fout << opponent_cards.size() << endl;
	for (int i = 0; i < opponent_cards.size(); i++)
	{
		fout << opponent_cards[i].get_rank() << " " << opponent_cards[i].get_suit() << endl;
	}

	fout << common_cards.size() << endl;
	for(int i=0; i < common_cards.size(); i++)
	{
		fout << common_cards[i].get_rank() << " " << common_cards[i].get_suit() << endl;
	}

	fout.close(); //закрыть файл
}

void Game_info::start_new_game(TgBot::Bot* bot, TgBot::Message::Ptr message) //начать новую игру
{
	f_game_stage = PREFLOP; //записать в переменную флага текущую стадию игры

	//обнулить данные об игровой сессии перед её началом
	is_player_should_bet_big_blind = 0;
	pot = 0;
	player_bet = 0;
	opponent_bet = 0;
	player_stack = DEFAULT_PLAYER_STACK;
	opponent_stack = DEFAULT_OPPONENT_STACK;
	big_blind = DEFAULT_BIG_BLIND;
	//очистить векторы с картами
	player_cards.clear();
	opponent_cards.clear();
	common_cards.clear();
	
	bot->getApi().sendMessage(message->chat->id, "🔷 Запущена новая игра");
	bot->getApi().sendMessage(message->chat->id, "🔹 Первый раунд : Preflop");

	uniform_int_distribution<int> f_big_blind_range(0, 1); //диапазон для случайной генерации флага большого блайнда
	is_player_should_bet_big_blind = f_big_blind_range(random_generator); //случайная генерация
	if (is_player_should_bet_big_blind == 1) //игрок должен сделать большой блайнд
	{
		bot->getApi().sendMessage(message->chat->id, "Делайте большой блайнд.");//////////////////////
		/////////////////////////////////////////////
	}
	else //игрок должен сделать малый блайнд
	{
		bot->getApi().sendMessage(message->chat->id, "Делайте малый блайнд.");//////////////////////
		///////////////////////////////////////////////////
	}
	

	write_to_file(); //запись значений в файл
}

Playing_card Game_info::get_rand_card() //получить случайную карту, не совпадающую с карманными картами игрока, соперника и общими картами
{
	uniform_int_distribution<int> suit_range(FISRT_SUIT, LAST_SUIT); //диапазон для случайной генерации масти карты
	uniform_int_distribution<int> rank_range(MIN_RANK, MAX_RANK); //диапазон для случайной генерации достоинства карты

	int rand_rank;  //случайное достоинство карты
	int rand_suit; //случайная масть карты
	int f_duplicate; //флаг, что найдено совпадение с картами игрока, соперника или общими картами

	do //генерировать случайные значения, пока не будет найдена карта, не совпадающая с имеющимися
	{
		rand_rank = rank_range(random_generator); //случайное достоинство карты
		rand_suit = suit_range(random_generator); //случайная масть карты
		f_duplicate = 0;

		for (int i = 0; i < player_cards.size(); i++)
		{
			if (player_cards[i].get_rank() == rand_rank && player_cards[i].get_suit() == rand_suit)
			{
				f_duplicate = 1;
				break;
			}
		}
		for (int i = 0; i < opponent_cards.size(); i++)
		{
			if (opponent_cards[i].get_rank() == rand_rank && opponent_cards[i].get_suit() == rand_suit)
			{
				f_duplicate = 1;
				break;
			}
		}
		for (int i = 0; i < common_cards.size(); i++)
		{
			if (common_cards[i].get_rank() == rand_rank && common_cards[i].get_suit() == rand_suit)
			{
				f_duplicate = 1;
				break;
			}
		}
	} while (f_duplicate == 1);

	Playing_card rand_card(rand_rank, rand_suit);
	return  rand_card;
}