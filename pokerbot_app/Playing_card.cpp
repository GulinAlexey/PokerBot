#include "Playing_card.h"

Playing_card::Playing_card(int rank_input, int suit_input) //инициализация
{
	rank = rank_input;
	suit = suit_input;
}

int Playing_card::get_rank() //получить достоинство карты
{
	return rank;
}
int Playing_card::get_suit() //получить масть карты
{
	return suit;
}