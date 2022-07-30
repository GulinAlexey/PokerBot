#include "Playing_card.h"

Playing_card::Playing_card(int value_input, int suit_input) //инициализация
{
	value = value_input;
	suit = suit_input;
}

int Playing_card::get_value() //получить достоинство карты
{
	return value;
}
int Playing_card::get_suit() //получить масть карты
{
	return suit;
}

string Playing_card::get_name() //получить имя карты для вывода в сообщении
{
	string card_name;
	if (value >= TWO && value <= TEN)
	{
		card_name = to_string(value);
	}
	switch (value)
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