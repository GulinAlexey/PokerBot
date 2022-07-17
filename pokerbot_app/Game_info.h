#pragma once
#include <fstream>
#include <iostream>
#include <random>
#include <chrono>
#include <tgbot/tgbot.h>
#include "Playing_card.h"

#define DEFAULT_PLAYER_STACK 100 //начальный размер стека игрока по умолчанию
#define DEFAULT_OPPONENT_STACK DEFAULT_PLAYER_STACK //начальный размер стека соперника по умолчанию
#define DEFAULT_BIG_BLIND 2 //размер большого блайнда по умолчанию

#define GAME_NOT_STARTED 0 //значение флага, что игрок ещё не начал новую игру
#define PREFLOP 1 //значение флага: первая стадия игры - префлоп
#define FLOP 2 //значение флага: вторая стадия игры - флоп
#define TURN 3 //значение флага: третья стадия игры - тёрн
#define RIVER 4 //значение флага: четвёртая стадия игры - ривер

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

	int pot; //банк (сумма всех поставленных игроками фишек)
	int player_bet; //ставка игрока
	int opponent_bet; //ставка соперника

	int player_stack; //кол-во фишек игрока на руках
	int opponent_stack; //кол-во фишек соперника на руках

	int big_blind; //регламентированный размер большого блайнда

	vector <Playing_card> player_cards; //карманные карты игрока (2 шт)
	vector <Playing_card> opponent_cards; //карманные карты соперника (2 шт)
	vector <Playing_card> common_cards; //открытые общие карты (3-5 шт)

	mt19937 random_generator; //случайный генератор для получения карты из колоды

public:
	Game_info();
	void init(int id_chat_input, int f_mode); //инициализация на основе идентификатора чата
	void read_from_file(); //чтение значений из файла (должно быть уже установлено значение id_chat)
	void write_to_file(); //запись значений в файл (перезапись, если файл уже был создан)
	void start_new_game(TgBot::Bot* bot, TgBot::Message::Ptr message); //начать новую игру
	Playing_card get_rand_card(); //получить случайную карту, не совпадающую с карманными картами игрока, соперника и общими картами
};
