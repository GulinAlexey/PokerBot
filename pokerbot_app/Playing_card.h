#pragma once

#define JACK 11 //численное обозначение - валет
#define QUEEN 12 //численное обозначение - дама
#define KING 13 //численное обозначение - король
#define ACE 14 //численное обозначение - туз

#define MIN_RANK 2 //наименьшее достоинство карты
#define MAX_RANK ACE //наибольшее достоинство карты

#define HEARTS 1 //масть - червы
#define DIAMONDS 2 //масть - бубны
#define CLUBS 3 //масть - трефы
#define SPADES 4 //масть - пики

#define FISRT_SUIT HEARTS //первая масть при начальном построении колоды
#define LAST_SUIT SPADES //последняя масть при начальном построении колоды

class Playing_card //игральная карта
{
private:
	int rank; //достоинство карты
	int suit; //масть карты
public:
	Playing_card(int rank_input, int suit_input); //инициализация
	int get_rank(); //получить достоинство карты
	int get_suit(); //получить масть карты
};

