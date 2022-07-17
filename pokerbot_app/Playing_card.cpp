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

string Playing_card::get_name() //получить имя карты для вывода в сообщении
{
	string card_name;
	if (rank >= TWO && rank <= TEN)
	{
		card_name = to_string(rank);
	}
	switch (rank)
	{
	case JACK:
		card_name = "J";
		break;
	case QUEEN:
		card_name = "Q";
		break;
	case KING:
		card_name = "K";
		break;
	case ACE:
		card_name = "A";
		break;
	}

	card_name += " ";

	switch (suit)
	{
	case HEARTS:
		card_name += "♥️";
		break;
	case DIAMONDS:
		card_name += "♦️";
		break;
	case CLUBS:
		card_name += "♣️";
		break;
	case SPADES:
		card_name += "♠️";
		break;
	}
	return card_name;
}