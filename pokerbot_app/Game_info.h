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
#define SHOWDOWN 5 //значение флага: завершение игры - вскрытие карт и определение победителя с сильнейшей комбинацией карт

#define BEGIN_OF_STAGE 0 //флаг: начало данной стадии игры
#define BETTING_ROUND 1 //игрок и соперник сделали блайнды, розданы карты, начат круг торговли

#define PLAYER_BET 0 //игрок делает ставку
#define OPPONENT_BET 1 //соперник делает ставку

#define MODE_NEW_PROFILE 0 //режим создания нового профиля игры и его запись в файл
#define MODE_EXISTING_PROFILE 1 //режим чтения существующего профиля из файла

#define FOLDER "profiles/" //папка для сохранения данных пользователей
#define TYPE_OF_PROFILE_FILE ".dat" //тип файла для сохранения данных пользователей

#define MAIN_MENU_MSG "Начать новую игру: /new_game\nСправка: /help\nСтатистика: /statistics"
#define TAKE_ACTON_MSG "У вас право хода.\n\nСписок возможных действий:\n\n⬥Напишите только число, чтобы повысить ставку (raise) до введённого кол-ва фишек (кратно большому блайнду)\n⬥/call — уравнять ставку\n⬥/check — передать ход (может только большой блайнд до повышения ставок)\n⬥/fold — сбросить карты" //сообщение с описанием возможных действий в круге торговли
#define MSG_BEFORE_BLIND "Сделать блайнд: /make_blind\nВыйти из игры: /exit" //сообщение о том, что игроку следует сделать блайнд

#define RAISE 0 //тип действия в торговле - повысить ставку
#define CALL 1 //тип действия в торговле - уравнять ставку
#define CHECK 2 //тип действия в торговле - передать ход
#define FOLD 3 //тип действия в торговле - сбросить карты

#define EMPTY_CARDS -1 //невозможно определить комбинацию карт, так как игрокам ещё не выдали карманные карты
#define HIGHCARD 0 //комбинация карт - старшая карта
#define PAIR 1 //комбинация карт - пара
#define TWO_PAIRS 2 //комбинация карт - две пары
#define THREE_OF_A_KIND 3 //комбинация карт - сет (тройка)
#define STRAIGHT 4 //комбинация карт - стрит
#define FLUSH 5 //комбинация карт - флеш
#define FULL_HOUSE 6 //комбинация карт - фуллхаус (три плюс два)
#define FOUR_OF_A_KIND 7 //комбинация карт - каре
#define STRAIGHT_FLUSH 8 //комбинация карт - стрит-флеш
#define ROYAL_FLUSH 9 //комбинация карт - роял-флеш

#define GENITIVE 0 //родительный падеж
#define ACCUSATIVE 1 //винительный падеж

#define OPPONENT_WON 0 //соперник победил и забирает банк
#define PLAYER_WON 1 //игрок победил и забирает банк
#define DRAW 2 //ничья, банк делится поровну

using namespace std;

class Game_info //информация об игре для отдельного пользователя
{
private:
	int id_chat; //id чата игрока

	int won_chips; //общее кол-во выигранных фишек за все игры
	int	lost_chips; //общее кол-во проигранных фишек за все игры
	int wins_qty; //всего побед
	int losses_qty; //всего поражений
	int draws_qty; //всего ничьих

	int f_game_stage; //флаг текущей стадии игры
	int f_stage_action; //флаг текущего действия в игре (игрок сделал блайнд, получил карты, сделал ставку и т.д.)
	int is_player_should_bet_big_blind; //должен ли игрок на данном этапе сделать большой блайнд (если =1), или же малый (=0)

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
	void init(int id_chat_input, int f_mode, TgBot::Bot* bot, TgBot::Message::Ptr message); //инициализация на основе идентификатора чата
	bool read_from_file(); //чтение значений из файла (должно быть уже установлено значение id_chat)
	void write_to_file(); //запись значений в файл (перезапись, если файл уже был создан)
	void start_new_game(TgBot::Bot* bot, TgBot::Message::Ptr message); //начать новую игру
	Playing_card get_rand_card(); //получить случайную карту, не совпадающую с карманными картами игрока, соперника и общими картами
	void send_game_status(TgBot::Bot* bot, TgBot::Message::Ptr message); //отправить инфо о банке, фишках игрока и соперника
	bool make_bet(int bet_size, int player_or_opponent); //сделать ставку
	void exit(TgBot::Bot* bot, TgBot::Message::Ptr message); //выйти из игры
	void make_blind(TgBot::Bot* bot, TgBot::Message::Ptr message); //сделать блайнд
	void end(int player_wins, TgBot::Bot* bot, TgBot::Message::Ptr message); //конец игры
	void auto_action(TgBot::Bot* bot, TgBot::Message::Ptr message); //действие соперника в круге торговли
	void action_of_player(int type_of_action, int bet_size, TgBot::Bot* bot, TgBot::Message::Ptr message); //действие игрока в круге торговли
	bool raise(int bet_size, int player_or_opponent); //повысить ставку
	bool call(int player_or_opponent); //уравнять ставку
	bool check(int player_or_opponent); //передать ход
	void fold(int player_or_opponent, TgBot::Bot* bot, TgBot::Message::Ptr message); //сбросить карты
	void to_next_stage(TgBot::Bot* bot, TgBot::Message::Ptr message); //перейти к следующей стадии игры
	void statistics(TgBot::Bot* bot, TgBot::Message::Ptr message); //вывести статистику выигрышей и проигрышей
	vector <Playing_card> determine_card_combination(int player_or_opponent, int* combination, int* kicker_value); //определить карточную комбинацию и кикер для сравнения комбинаций игроков
	string word_chip(int qty_chip); //получить слово "фишки" в именительном падеже с правильным окончанием в зависимости от кол-ва фишек
	string word_chip(int qty_chip, int word_case); //получить слово "фишки" в родительном или винительном падеже с правильным окончанием в зависимости от кол-ва фишек
	void send_main_menu(TgBot::Bot* bot, TgBot::Message::Ptr message); //отправить сообщение с основными командами вне игры (аналогично стартовому сообщению)
	string get_combination_name(int combination_type); //получить имя комбинации карт для вывода в сообщении
};
