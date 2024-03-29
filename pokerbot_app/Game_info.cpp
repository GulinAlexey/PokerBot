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
		won_chips = 0;
		lost_chips = 0;
		wins_qty = 0;
		losses_qty = 0;
		draws_qty = 0;
		f_game_stage = GAME_NOT_STARTED;
		f_stage_action = BEGIN_OF_STAGE;
		is_player_should_bet_big_blind = 0;
		pot = 0;
		player_bet = 0;
		opponent_bet = 0;
		opponent_chips_in_pot = 0;
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
	fin >> won_chips;
	fin >> lost_chips;
	fin >> wins_qty;
	fin >> losses_qty;
	fin >> draws_qty;
	fin >> f_game_stage;
	fin >> f_stage_action;
	fin >> is_player_should_bet_big_blind;
	fin >> pot;
	fin >> player_bet;
	fin >> opponent_bet;
	fin >> opponent_chips_in_pot;
	fin >> player_stack;
	fin >> opponent_stack;
	fin >> big_blind;

	int qty_player_cards; //кол-во карманных карт игрока
	fin >> qty_player_cards;
	for (int i = 0; i < qty_player_cards; i++)
	{
		int card_value, card_suit;
		fin >> card_value;
		fin >> card_suit;
		Playing_card card(card_value, card_suit);
		player_cards.push_back(card);
	}

	int qty_opponent_cards; //кол-во карманных карт соперника
	fin >> qty_opponent_cards;
	for (int i = 0; i < qty_opponent_cards; i++)
	{
		int card_value, card_suit;
		fin >> card_value;
		fin >> card_suit;
		Playing_card card(card_value, card_suit);
		opponent_cards.push_back(card);
	}

	int qty_common_cards; //кол-во открытых общих карт
	fin >> qty_common_cards;
	for (int i = 0; i < qty_common_cards; i++)
	{
		int card_value, card_suit;
		fin >> card_value;
		fin >> card_suit;
		Playing_card card(card_value, card_suit);
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
	fout << won_chips << endl;
	fout << lost_chips << endl;
	fout << wins_qty << endl;
	fout << losses_qty << endl;
	fout << draws_qty << endl;
	fout << f_game_stage << endl;
	fout << f_stage_action << endl;
	fout << is_player_should_bet_big_blind << endl;
	fout << pot << endl;
	fout << player_bet << endl;
	fout << opponent_bet << endl;
	fout << opponent_chips_in_pot << endl;
	fout << player_stack << endl;
	fout << opponent_stack << endl;
	fout << big_blind << endl;

	fout << player_cards.size() << endl;
	for (int i = 0; i < player_cards.size(); i++)
	{
		fout << player_cards[i].get_value() << " " << player_cards[i].get_suit() << endl;
	}

	fout << opponent_cards.size() << endl;
	for (int i = 0; i < opponent_cards.size(); i++)
	{
		fout << opponent_cards[i].get_value() << " " << opponent_cards[i].get_suit() << endl;
	}

	fout << common_cards.size() << endl;
	for(int i=0; i < common_cards.size(); i++)
	{
		fout << common_cards[i].get_value() << " " << common_cards[i].get_suit() << endl;
	}

	fout.close(); //закрыть файл
}


void Game_info::start_new_game(TgBot::Bot* bot, TgBot::Message::Ptr message) //начать новую игру
{
	if (f_game_stage != GAME_NOT_STARTED)
	{
		exit(bot, message, false); //если начать новую игру в процессе игры, то будет засчитано поражение
	}
	f_game_stage = PREFLOP; //записать в переменную флага текущую стадию игры
	f_stage_action = BEGIN_OF_STAGE;

	//обнулить данные об игровой сессии перед её началом
	is_player_should_bet_big_blind = 0;
	pot = 0;
	player_bet = 0;
	opponent_bet = 0;
	opponent_chips_in_pot = 0;
	player_stack = DEFAULT_PLAYER_STACK;
	opponent_stack = DEFAULT_OPPONENT_STACK;
	big_blind = DEFAULT_BIG_BLIND;
	//очистить векторы с картами
	player_cards.clear();
	opponent_cards.clear();
	common_cards.clear();
	
	bot->getApi().sendMessage(message->chat->id, "🔷 Запущена новая игра.\n\nБольшой блайнд = " + to_string(big_blind) + word_chip(big_blind) +", малый блайнд = " + to_string(big_blind/2) + word_chip(big_blind / 2));
	bot->getApi().sendMessage(message->chat->id, "🔷 Первый раунд : Preflop");
	send_game_status(bot, message); //вывести в сообщении текущее состояние стека, банка и карт

	uniform_int_distribution<int> f_big_blind_range(0, 1); //диапазон для случайной генерации флага большого блайнда
	is_player_should_bet_big_blind = f_big_blind_range(random_generator); //случайная генерация
	if (is_player_should_bet_big_blind == 1) //игрок должен сделать большой блайнд
	{
		make_bet(big_blind / 2, OPPONENT_BET); //соперник сделал малый блайнд
		bot->getApi().sendMessage(message->chat->id, "Согласно случайному выбору, вам выпал большой блайнд. Вам следует поставить " + to_string(big_blind) + word_chip(big_blind, ACCUSATIVE) + "."
			+ "\n\nВаш соперник сделал малый блайнд (" + to_string(big_blind / 2) + word_chip(big_blind / 2) + ").\n\n" + MSG_BEFORE_BLIND);																																			/////////////////////////////////////////////
	}
	else //игрок должен сделать малый блайнд
	{
		bot->getApi().sendMessage(message->chat->id, "Согласно случайному выбору, вам выпал малый блайнд. Вам следует поставить " + to_string(big_blind/2) + word_chip(big_blind / 2, ACCUSATIVE) + "."
			+ "\n\n" + MSG_BEFORE_BLIND);
	}

	write_to_file(); //запись значений в файл
}

void Game_info::make_blind(TgBot::Bot* bot, TgBot::Message::Ptr message) //сделать блайнд
{
	if (f_game_stage == GAME_NOT_STARTED)
	{
		bot->getApi().sendMessage(message->chat->id, "Вы сейчас не находитесь в игре, поэтому команда блайнда не имеет смысла.\nСправка: /help");
		return;
	}
	if (f_stage_action != BEGIN_OF_STAGE)
	{
		bot->getApi().sendMessage(message->chat->id, "В данный момент игры команда блайнда не имеет смысла.\nСправка: /help");
		return;
	}

	bool f_blind_is_successful = false; //флаг об успешности ставки
	if (is_player_should_bet_big_blind == 0) //игрок должен сделать малый блайнд
	{
		f_blind_is_successful = make_bet(big_blind / 2, PLAYER_BET); //сделать ставку
		if (f_blind_is_successful == true)
			bot->getApi().sendMessage(message->chat->id, "Вы сделали малый блайнд (" + to_string(big_blind / 2) + word_chip(big_blind / 2) + ")");
	}
	else //игрок должен сделать большой блайнд
	{
		f_blind_is_successful = make_bet(big_blind, PLAYER_BET); //сделать ставку
		if (f_blind_is_successful == true)
			bot->getApi().sendMessage(message->chat->id, "Вы сделали большой блайнд (" + to_string(big_blind) + word_chip(big_blind) + ")");
	}
	if (f_blind_is_successful == false) //у игрока недостаточно фишек для блайнда
	{
		bot->getApi().sendMessage(message->chat->id, "У вас недостаточно фишек, чтобы сделать блайнд — обязательную ставку");
		end(OPPONENT_WON, bot, message, true, true);
	}

	if (is_player_should_bet_big_blind == 0) //игрок должен был сделать малый блайнд
	{
		make_bet(big_blind, OPPONENT_BET); //соперник сделал большой блайнд
		bot->getApi().sendMessage(message->chat->id, "Ваш соперник сделал большой блайнд (" + to_string(big_blind) + word_chip(big_blind) + ")");
	}

	switch (f_game_stage)
	{
	case PREFLOP:
		//раздача 2 карманных карт игрокам по 1 карте, сначала карту получает игрок с малым блайндом
		if (is_player_should_bet_big_blind == 0) //игрок должен был сделать малый блайнд
		{
			player_cards.push_back(get_rand_card(player_cards, opponent_cards, common_cards));
			opponent_cards.push_back(get_rand_card(player_cards, opponent_cards, common_cards));
			player_cards.push_back(get_rand_card(player_cards, opponent_cards, common_cards));
			opponent_cards.push_back(get_rand_card(player_cards, opponent_cards, common_cards));
		}
		else //игрок должен был сделать большой блайнд
		{
			opponent_cards.push_back(get_rand_card(player_cards, opponent_cards, common_cards));
			player_cards.push_back(get_rand_card(player_cards, opponent_cards, common_cards));
			opponent_cards.push_back(get_rand_card(player_cards, opponent_cards, common_cards));
			player_cards.push_back(get_rand_card(player_cards, opponent_cards, common_cards));
		}
		bot->getApi().sendMessage(message->chat->id, "Вам и вашему сопернику розданы карманные карты");
		break;
	case FLOP:
		//раздача 3 общих карт
		common_cards.push_back(get_rand_card(player_cards, opponent_cards, common_cards));
		common_cards.push_back(get_rand_card(player_cards, opponent_cards, common_cards));
		common_cards.push_back(get_rand_card(player_cards, opponent_cards, common_cards));
		bot->getApi().sendMessage(message->chat->id, "Розданы 3 общие карты");

		break;
	case TURN:
		//раздача 4-й общей карты
		common_cards.push_back(get_rand_card(player_cards, opponent_cards, common_cards));
		bot->getApi().sendMessage(message->chat->id, "Роздана четвёртая общая карта");
		break;
	case RIVER:
		//раздача 5-й общей карты
		common_cards.push_back(get_rand_card(player_cards, opponent_cards, common_cards));
		bot->getApi().sendMessage(message->chat->id, "Роздана пятая общая карта");
		break;
	}
	f_stage_action = BETTING_ROUND; //установить флаг текущего состояния игры в значение торговли
	write_to_file(); //запись значений в файл
	bot->getApi().sendMessage(message->chat->id, "🔹 Начат круг торговли");

	//первым в торгах в префлопе действует игрок с малым блайндом, в остальных раундах - игрок с большим блайндом
	//проверка, должен ли соперник первым действовать в торгах
	if ((is_player_should_bet_big_blind == 1 && f_game_stage == PREFLOP) || (is_player_should_bet_big_blind == 0 && f_game_stage != PREFLOP))
	{
		auto_action(bot, message);//соперник действует в торгах
		if (player_bet == opponent_bet) //если ставки уравнялись
		{
			to_next_stage(bot, message); //перейти к следующей стадии игры
			return;
		}
	}
	if (f_game_stage != GAME_NOT_STARTED && f_stage_action == BETTING_ROUND)
	{
		send_game_status(bot, message); //вывести в сообщении текущее состояние стека, банка и карт
		bot->getApi().sendMessage(message->chat->id, TAKE_ACTON_MSG); //сообщение с списком возможных действий игрока в круге торговли
	}
}

void Game_info::action_of_player(int type_of_action, int bet_size, TgBot::Bot* bot, TgBot::Message::Ptr message) //действие игрока в круге торговли
{
	if (f_game_stage == GAME_NOT_STARTED)
	{
		if (type_of_action != RAISE)
		{
			bot->getApi().sendMessage(message->chat->id, "Вы сейчас не находитесь в игре, поэтому команда торговли не имеет смысла.\nСправка: /help");
		}
		else
		{
			bot->getApi().sendMessage(message->chat->id, "Данное сообщение не является командой. Проверьте правильность ввода. Если вы хотели поднять ставку, то в данный момент это не имеет смысла, так как вы не находитесь в игре.\nСправка: /help");
		}
		return;
	}
	if (f_stage_action != BETTING_ROUND)
	{
		if (type_of_action != RAISE)
		{
			bot->getApi().sendMessage(message->chat->id, "В данный момент игры команда торговли не имеет смысла.\nСправка: /help");
		}
		else
		{
			bot->getApi().sendMessage(message->chat->id, "Данное сообщение не является командой. Проверьте правильность ввода. Если вы хотели поднять ставку, то в данный момент игры это не имеет смысла.\nСправка: /help");
		}
		return;
	}
	bool f_success = false; //флаг успеха действия
	switch (type_of_action)
	{
	case RAISE:
		f_success = raise(bet_size, PLAYER_BET);
		if (f_success == true)
		{
			bot->getApi().sendMessage(message->chat->id, "Вы повысили ставку до " + to_string(player_bet) + word_chip(player_bet, GENITIVE));
		}
		else
		{
			bot->getApi().sendMessage(message->chat->id, "Повысить ставку не удалось. Проверьте правильность ввода.\n\nРейз должен быть больше текущих ставок, быть кратным большому блайнду, и у вас должно быть достаточно фишек в стеке. Кроме того, фишек должно хватить на ставку блайндов в следующих раундах");
			send_game_status(bot, message); //отправить инфо о банке, фишках игрока и соперника
			bot->getApi().sendMessage(message->chat->id, TAKE_ACTON_MSG); //сообщение с списком возможных действий игрока в круге торговли
			return; //прекратить дальшейшее выполнение действий и ждать повторного ввода
		}
		break;
	case CALL:
		f_success = call(PLAYER_BET);
		if (f_success == true)
		{
			bot->getApi().sendMessage(message->chat->id, "Вы уравняли вашу ставку до " + to_string(player_bet) + word_chip(player_bet, GENITIVE));
		}
		else
		{
			if (player_bet < opponent_bet)
				bot->getApi().sendMessage(message->chat->id, "Уравнять ставку не удалось. У вас недостаточно фишек в стеке. Выберите другое действие");
			else
				bot->getApi().sendMessage(message->chat->id, "Уравнять ставку не удалось, так как ставка соперника меньше вашей. Выберите другое действие");
			send_game_status(bot, message); //отправить инфо о банке, фишках игрока и соперника
			bot->getApi().sendMessage(message->chat->id, TAKE_ACTON_MSG); //сообщение с списком возможных действий игрока в круге торговли
			return; //прекратить дальшейшее выполнение действий и ждать повторного ввода
		}
		break;
	case CHECK:
		f_success = check(PLAYER_BET);
		if (f_success == true)
		{
			bot->getApi().sendMessage(message->chat->id, "Вы передали ход вашему сопернику");
		}
		else
		{
			bot->getApi().sendMessage(message->chat->id, "Передать ход не удалось. Это возможно, если ваша ставка равна большому блайнду, а ваш соперник ещё не повышал и не уравнивал ставку. Выберите другое действие");
			send_game_status(bot, message); //отправить инфо о банке, фишках игрока и соперника
			bot->getApi().sendMessage(message->chat->id, TAKE_ACTON_MSG); //сообщение с списком возможных действий игрока в круге торговли
			return; //прекратить дальшейшее выполнение действий и ждать повторного ввода
		}
		break;
	case FOLD:
		fold(PLAYER_BET, bot, message);
		return; //прекратить дальшейшее выполнение действий, так как игра окончена
		break;
	}

	write_to_file(); //запись значений в файл

	if (player_bet == opponent_bet) //если ставки уравнялись
	{
		to_next_stage(bot, message); //перейти к следующей стадии игры
	}
	else
	{
		auto_action(bot, message); //соперник действует в торгах
		if (player_bet == opponent_bet) //если ставки уравнялись
		{
			to_next_stage(bot, message); //перейти к следующей стадии игры
		}
		else
		{
			if (f_game_stage != GAME_NOT_STARTED)
			{
				send_game_status(bot, message); //отправить инфо о банке, фишках игрока и соперника
				bot->getApi().sendMessage(message->chat->id, TAKE_ACTON_MSG); //сообщение с списком возможных действий игрока в круге торговли
			}
		}
	}
}

void Game_info::to_next_stage(TgBot::Bot* bot, TgBot::Message::Ptr message) //перейти к следующей стадии игры
{
	f_game_stage++; //установить флаг следующей стадии игры
	f_stage_action = BEGIN_OF_STAGE; //установить флаг начала стадии
	bot->getApi().sendMessage(message->chat->id, "🔹 Ставки уравнены, торги завершены");
	send_game_status(bot, message); //вывести в сообщении текущее состояние стека, банка и карт

	opponent_chips_in_pot += opponent_bet; //учёт суммы ставок соперника в предыдущих кругах торговли
	player_bet = 0; //отсчёт ставок в новом раунде начинается заново
	opponent_bet = 0;

	if (is_player_should_bet_big_blind == 1) //игрок в прошлом раунде сделал большой блайнд
	{
		is_player_should_bet_big_blind = 0; //теперь игрок должен сделать малый блайнд
	}
	else //игрок в прошлом раунде сделал малый блайнд
	{
		is_player_should_bet_big_blind = 1; //теперь игрок должен сделать большой блайнд
	}

	switch (f_game_stage)
	{
	case FLOP:
		bot->getApi().sendMessage(message->chat->id, "🔷 Второй раунд : Flop");
		break;
	case TURN:
		bot->getApi().sendMessage(message->chat->id, "🔷 Третий раунд : Turn");
		break;
	case RIVER:
		bot->getApi().sendMessage(message->chat->id, "🔷 Четвёртый раунд : River");
		break;
	case SHOWDOWN:
		bot->getApi().sendMessage(message->chat->id, "🔷 Вскрытие карт");
		string str_output = "Ваши карманные карты:"; //строка для вывода инфо в сообщении
		for (int i = 0; i < player_cards.size(); i++) //вывести карманные карты игрока
		{
			str_output += "\n" + player_cards[i].get_name();
		}
		str_output += "\n\nКарманные карты соперника:";
		for (int i = 0; i < opponent_cards.size(); i++) //вывести карманные карты соперника
		{
			str_output += "\n" + opponent_cards[i].get_name();
		}
		str_output += "\n\nОбщие карты:";
		for (int i = 0; i < common_cards.size(); i++) //вывести общие карты
		{
			str_output += "\n" + common_cards[i].get_name();
		}
		int player_combination_type; //тип собранной комбинации игрока
		int player_kicker_value; //значение кикера игрока (для сравнения при совпадении комбинаций)
		vector <Playing_card> player_combination_cards = determine_card_combination(player_cards, common_cards, &player_combination_type, &player_kicker_value); //карты в комбинации игрока (для вывода на экран)

		int opponent_combination_type; //тип собранной комбинации соперника
		int opponent_kicker_value; //значение кикера соперника (для сравнения при совпадении комбинаций)
		vector <Playing_card> opponent_combination_cards = determine_card_combination(opponent_cards, common_cards, &opponent_combination_type, &opponent_kicker_value); //карты в комбинации соперника (для вывода на экран)

		str_output += "\n\nВаша комбинация — " + get_combination_name(player_combination_type) + ":";
		for (int i = 0; i < player_combination_cards.size(); i++) //вывести комбинацию карт игрока
		{
			str_output += "\n" + player_combination_cards[i].get_name();
		}

		str_output += "\n\nКомбинация соперника — " + get_combination_name(opponent_combination_type) + ":";
		for (int i = 0; i < opponent_combination_cards.size(); i++) //вывести комбинацию карт игрока
		{
			str_output += "\n" + opponent_combination_cards[i].get_name();
		}

		bot->getApi().sendMessage(message->chat->id, str_output);

		if (player_combination_type > opponent_combination_type) //комбинация игрока сильнее комбинации соперника
		{
			end(PLAYER_WON, bot, message, true, false); //игрок победил
		}
		else if (player_combination_type < opponent_combination_type) //комбинация соперника сильнее комбинации игрока
		{
			end(OPPONENT_WON, bot, message, true, false); //соперник победил
		}
		else //комбинации игрока и соперника совпали
		{
			//определение победителя по кикеру
			if (player_kicker_value > opponent_kicker_value)
			{
				end(PLAYER_WON, bot, message, true, false); //игрок победил
			}
			else if (player_kicker_value < opponent_kicker_value)
			{
				end(OPPONENT_WON, bot, message, true, false); //соперник победил
			}
			else //карты-кикеры у игрока и соперника тоже совпали
			{
				end(DRAW, bot, message, true, false); //ничья
			}

		}
		return; //игра завершена
		break;
	}

	if (is_player_should_bet_big_blind == 1) //игрок должен сделать большой блайнд
	{
		make_bet(big_blind / 2, OPPONENT_BET); //соперник сделал малый блайнд
		bot->getApi().sendMessage(message->chat->id, "Вам следует поставить большой блайнд (" + to_string(big_blind) + word_chip(big_blind, ACCUSATIVE) + ")."
			+ "\n\nВаш соперник сделал малый блайнд (" + to_string(big_blind / 2) + word_chip(big_blind / 2) + ").\n\n" + MSG_BEFORE_BLIND);
	}
	else //игрок должен сделать малый блайнд
	{
		bot->getApi().sendMessage(message->chat->id, "Вам следует поставить малый блайнд (" + to_string(big_blind / 2) + word_chip((big_blind / 2), ACCUSATIVE) + ")."
			+ "\n\n" + MSG_BEFORE_BLIND);
	}

	write_to_file(); //запись значений в файл
}

void Game_info::exit(TgBot::Bot* bot, TgBot::Message::Ptr message, bool is_send_main_menu) //выйти из игры
{
	if (f_game_stage == GAME_NOT_STARTED)
	{
		bot->getApi().sendMessage(message->chat->id, "Вы сейчас не находитесь в игре, поэтому команда выхода не имеет смысла. Справка: /help");
	}
	else
	{
		bot->getApi().sendMessage(message->chat->id, "Вы вышли из текущей игры");
		end(OPPONENT_WON, bot, message, is_send_main_menu, true); //выход из игры приравнивается к поражению
	}
}

void Game_info::end(int player_wins, TgBot::Bot* bot, TgBot::Message::Ptr message, bool is_send_main_menu, bool is_was_fold) //конец игры
{
	f_game_stage = GAME_NOT_STARTED;
	if (player_wins == OPPONENT_WON) //игрок проиграл
	{
		bot->getApi().sendMessage(message->chat->id, "Игра окончена, вы проиграли.\n\nВаш соперник забирает банк: " + to_string(pot) + word_chip(pot) + ".\n\nС учётом начального стека в " + to_string(DEFAULT_PLAYER_STACK) + word_chip(DEFAULT_PLAYER_STACK, ACCUSATIVE) + ":\nв итоге у вас -" + to_string(DEFAULT_PLAYER_STACK - player_stack) + word_chip(DEFAULT_PLAYER_STACK - player_stack) + ",\nв итоге у соперника +" + to_string(pot - (DEFAULT_OPPONENT_STACK - opponent_stack)) + word_chip(pot - (DEFAULT_OPPONENT_STACK - opponent_stack)) + ".");
		lost_chips += DEFAULT_PLAYER_STACK - player_stack; //увеличить общее число проигранных фишек за всё время
		losses_qty++; //увеличить общее число проигрышей
	}
	else if (player_wins == PLAYER_WON)//игрок победил
	{
		bot->getApi().sendMessage(message->chat->id, "Игра завершена, вы выиграли.\n\nВы забираете банк: " + to_string(pot) + word_chip(pot) + ".\n\nС учётом начального стека в " + to_string(DEFAULT_PLAYER_STACK) + word_chip(DEFAULT_PLAYER_STACK, ACCUSATIVE) + ":\nв итоге у вас +" + to_string(pot - (DEFAULT_PLAYER_STACK - player_stack)) + word_chip(pot - (DEFAULT_PLAYER_STACK - player_stack)) + ",\nв итоге у соперника -" + to_string(DEFAULT_OPPONENT_STACK - opponent_stack) + word_chip(DEFAULT_OPPONENT_STACK - opponent_stack) + ".");
		won_chips += pot - (DEFAULT_PLAYER_STACK - player_stack); //увеличить общее число выигранных фишек за всё время
		wins_qty++; //увеличить общее число выигрышей
	}
	else // ничья
	{
		int chips_to_opponent = pot / 2; //часть банка, доставшаяся сопернику
		int chips_to_player = pot - chips_to_opponent; //часть банка, доставшаяся игроку
		bot->getApi().sendMessage(message->chat->id, "Игра завершена, ничья.\n\nБанк делится поровну (" + to_string(chips_to_player) + word_chip(chips_to_player) + " игроку и " + to_string(chips_to_opponent) + word_chip(chips_to_opponent) + " сопернику).\n\nС учётом начального стека в " + to_string(DEFAULT_PLAYER_STACK) + word_chip(DEFAULT_PLAYER_STACK, ACCUSATIVE) + ":\nв итоге у вас +" + to_string(chips_to_player - (DEFAULT_PLAYER_STACK - player_stack)) + word_chip(chips_to_player - (DEFAULT_PLAYER_STACK - player_stack)) + ",\nв итоге у соперника +" + to_string(chips_to_opponent - (DEFAULT_OPPONENT_STACK - opponent_stack)) + word_chip(chips_to_opponent - (DEFAULT_OPPONENT_STACK - opponent_stack)) + ".");
		won_chips += chips_to_player - (DEFAULT_PLAYER_STACK - player_stack);
		draws_qty++; //увеличить общее число ничьих
	}

	write_to_file(); //запись значений в файл
	if (is_was_fold == true) //если игра закончилась, но карты соперника не были показаны (ранее произошёл сброс карт)
		send_combinations_after_fold(bot, message); //вывести потенциальные карточные комбинации игроков после выхода из игры
	if (is_send_main_menu == true)
	{
		send_main_menu(bot, message); //вывести команды главного меню
	}
}


bool Game_info::make_bet(int bet_size, int player_or_opponent) //сделать ставку
{
	if (player_or_opponent == PLAYER_BET) //игрок делает ставку
	{
		int new_stack = player_stack - (bet_size - player_bet);
		if (new_stack < 0) //у игрока недостаточно фишек на руках для такой ставки
		{
			return false;
		}
		player_stack = new_stack;
		pot += (bet_size - player_bet);
		player_bet = bet_size;
		return true;
	}
	else //соперник делает ставку
	{
		int new_stack = opponent_stack - (bet_size - opponent_bet);
		if (new_stack < 0)
		{
			return false;
		}
		opponent_stack = new_stack;
		pot += (bet_size - opponent_bet);
		opponent_bet = bet_size;
		return true;
	}
}

bool Game_info::raise(int bet_size, int player_or_opponent) //повысить ставку
{
	if (bet_size <= player_bet || bet_size <= opponent_bet) //если ставка меньше, чем текущие ставки игрока и соперника, или равна им
	{
		return false;
	}

	int stack; //текущий стек игрока или соперника
	int last_bet; //текущая ставка игрока или соперника
	if (player_or_opponent == PLAYER_BET)
	{
		stack = player_stack;
		last_bet = player_bet;
	}
	else
	{
		stack = opponent_stack;
		last_bet = opponent_bet;
	}

	switch (f_game_stage) //запретить делать ставки, из-за которых на последующих этапах для блайндов не хватит фишек
	{
	case PREFLOP:
		if (bet_size > stack + last_bet - (big_blind * 3))
			return false;
		break;
	case FLOP:
		if (bet_size > stack + last_bet - (big_blind * 2))
			return false;
		break;
	case TURN:
		if (bet_size > stack + last_bet - big_blind)
			return false;
		break;
	}
	if (bet_size % big_blind != 0) //если повышенная ставка не кратна большому блайнду
	{
		return false;
	}
	if (player_or_opponent == PLAYER_BET) //действие игрока
	{
		return make_bet(bet_size, PLAYER_BET);
	}
	else //действие соперника
	{
		return make_bet(bet_size, OPPONENT_BET);
	}
}

bool Game_info::call(int player_or_opponent) //уравнять ставку
{
	if (player_or_opponent == PLAYER_BET) //действие игрока
	{
		if (player_bet < opponent_bet) //уравнивать свою ставку можно только с более высокой ставкой соперника
		{
			return make_bet(opponent_bet, PLAYER_BET);
		}
		else
			return false;
	}
	else //действие соперника
	{
		if (opponent_bet < player_bet) //уравнивать ставку соперника можно только с более высокой ставкой пользователя
		{
			return make_bet(player_bet, OPPONENT_BET);
		}
		else
			return false;
	}
}

bool Game_info::check(int player_or_opponent) //передать ход
{
	if (player_or_opponent == PLAYER_BET) //действие игрока
	{
		//если игрок сделал большой блайнд, а его соперник ещё не повышал ставки, тогда успешное выполнение передачи хода
		if (player_bet == big_blind && opponent_bet == big_blind / 2)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else //действие соперника
	{
		//если соперник сделал большой блайнд, а игрок ещё не повышал ставки, тогда успешное выполнение передачи хода
		if (opponent_bet == big_blind && player_bet == big_blind / 2)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

void Game_info::fold(int player_or_opponent, TgBot::Bot* bot, TgBot::Message::Ptr message) //сбросить карты
{
	if (player_or_opponent == PLAYER_BET) //действие игрока
	{
		bot->getApi().sendMessage(message->chat->id, "Вы сбросили карты");
		end(OPPONENT_WON, bot, message, true, true); //игрок проиграл
	}
	else //действие соперника
	{
		bot->getApi().sendMessage(message->chat->id, "Ваш соперник сбросил карты");
		end(PLAYER_WON, bot, message, true, true); //игрок выиграл
	}
}


void Game_info::auto_action(TgBot::Bot* bot, TgBot::Message::Ptr message) //ставка соперника в круге торговли
{
	//получить вероятность победы соперника (с текущими своими карманными и общими картами) методом моделирования
	double probability_opponent_win = get_win_probability(opponent_cards, common_cards);

	int type_of_auto_action = FOLD; //тип решения, которое в итоге примет соперник
	bool f_action_success = false; //успех выполнения действия

	//принятие решения
	if (probability_opponent_win > 0.6) //большая вероятность победы позволяет рискнуть и повысить ставку
		type_of_auto_action = RAISE;
	else if (probability_opponent_win >= 0.3 && probability_opponent_win <= 0.6) //вероятность победы средняя, действовать пассивно (чек или колл)
	{
		type_of_auto_action = CHECK;
		if (probability_opponent_win <= 0.4 && player_bet >= 10) //если шансы малы, а игрок начинает давить высокими ставками, то сбросить карты
			type_of_auto_action = FOLD;
	}
	else
		type_of_auto_action = FOLD; //вероятность победы низкая, лучше сбросить карты

	uniform_int_distribution<int> reraise_rand_range(0, 1); //диапазон для случайной генерации решения, нужно ли повторно повышать ставку
	uniform_int_distribution<int> special_rise_rand_range(AUTO_MAKE_RAISE, AUTO_MAKE_DOUBLE_RAISE); //диапазон для случайной генерации решения, каким образом повысить ставку

	int f_make_reraise = reraise_rand_range(random_generator); //случайно решить, нужно ли повторно повышать ставку
	if (probability_opponent_win >= 0.83)
		f_make_reraise = 1; //если вероятность выиграть высокая, то в любом случае ещё раз повысить ставку
	int f_make_special_rise = special_rise_rand_range(random_generator); //случайно решить, как повысить ставку (на 1 или 2 больших блайнда или в два раза)
	int new_bet; //новое значение ставки (в рейзе)

	//выполнение действия
	switch (type_of_auto_action)
	{
	case RAISE:
		if (f_make_reraise == 0 && (player_bet > big_blind || opponent_bet > big_blind))
			goto make_check; //сделать чек, если ранее ставка уже повышалась, а также было случайно решено сделать чек вместо рейза

		if (opponent_bet > player_bet)
			new_bet = opponent_bet; //новое значение ставки
		else
			new_bet = player_bet;

		switch (f_make_special_rise) //выбрать тип повышения ставки на основе случайного выбора
		{
		case AUTO_MAKE_RAISE:
			new_bet += big_blind;
			break;
		case AUTO_MAKE_RAISE_2_BLINDS:
			new_bet += big_blind * 2;
			break;
		case AUTO_MAKE_DOUBLE_RAISE:
			new_bet += new_bet;
			break;
		}

		f_action_success = raise(new_bet, OPPONENT_BET);
		
		if (f_action_success == true)
		{
			bot->getApi().sendMessage(message->chat->id, "Ваш соперник повысил ставку до " + to_string(opponent_bet) + word_chip(opponent_bet, GENITIVE));
			break;
		}
	case CHECK:
		make_check: //метка действия - чек
		f_action_success = check(OPPONENT_BET);
		if (f_action_success == true)
		{
			bot->getApi().sendMessage(message->chat->id, "Ваш соперник передал вам ход");
			break;
		}
	case CALL:
		f_action_success = call(OPPONENT_BET);
		if (f_action_success == true)
		{
			bot->getApi().sendMessage(message->chat->id, "Ваш соперник уравнял ставку до " + to_string(opponent_bet) + word_chip(opponent_bet, GENITIVE));
			break;
		}
	case FOLD:
		fold(OPPONENT_BET, bot, message);
		break;
	}

	write_to_file(); //запись значений в файл
}

double Game_info::get_win_probability(vector<Playing_card> my_pocket_cards, vector<Playing_card> my_common_cards) //получить вероятность победы с текущими карманными и общими картами
{
	int qty_model_wins = 0; //количество побед при моделировании
	int qty_model_losses = 0; //количество проигрышей при моделировании

	//промоделировать результат случайной раздачи неизвестных на данный момент карт много раз
	for (int i = 0; i < QTY_MODEL_GAMES_FOR_PROBABILITY; i++) 
	{
		int f_model_result = model_game_result(my_pocket_cards, my_common_cards);
		if (f_model_result == LOSE_IN_GAME)
			qty_model_losses++;  
		else
			qty_model_wins++;
	}

	//получить итоговую вероятность победы
	return double(qty_model_wins) / (qty_model_wins + qty_model_losses);
}

int Game_info::model_game_result(vector<Playing_card> my_pocket_cards, vector<Playing_card> my_common_cards) //случайно заполнить неизвестные на данный момент общие карты и карты противника и получить результат (победа или нет)
{
	vector <Playing_card> model_common_cards = my_common_cards; //предполагаемые общие карты (случайно дополняются до 5 шт.)
	vector <Playing_card> model_enemy_cards; //предполагаемые карманные карты противника (для соперника-бота противником является пользователь-игрок)
											
	//случайно заполнить карты противника
	model_enemy_cards.push_back(get_rand_card(my_pocket_cards, model_enemy_cards, model_common_cards));
	model_enemy_cards.push_back(get_rand_card(my_pocket_cards, model_enemy_cards, model_common_cards));
	
	//случайно дополнить общие карты до 5 шт.
	while (model_common_cards.size() != QTY_RIVER_COMMON_CARDS)
		model_common_cards.push_back(get_rand_card(my_pocket_cards, model_enemy_cards, model_common_cards));

	int my_model_combination_type; //тип комбинации данного игрока
	int my_model_kicker_value; //кикер данного игрока
	//определить комбинацию
	determine_card_combination(my_pocket_cards, model_common_cards, &my_model_combination_type, &my_model_kicker_value);

	int model_enemy_combination_type; //тип комбинации противника данного игрока
	int model_enemy_kicker_value; //кикер противника данного игрока
	//определить комбинацию
	determine_card_combination(model_enemy_cards, model_common_cards, &model_enemy_combination_type, &model_enemy_kicker_value);

	//определить победителя
	if (my_model_combination_type > model_enemy_combination_type) //комбинация данного игрока сильнее
	{
		return WIN_IN_GAME; //данный игрок победил
	}
	else if (my_model_combination_type < model_enemy_combination_type) //комбинация данного игрока слабее
	{
		return LOSE_IN_GAME; //данный игрок проиграл
	}
	else //комбинации данного игрока и его противника совпали
	{
		//определение победителя по кикеру
		if (my_model_kicker_value > model_enemy_kicker_value)
		{
			return WIN_IN_GAME; //данный игрок победил
		}
		else if (my_model_kicker_value < model_enemy_kicker_value)
		{
			return LOSE_IN_GAME; //данный игрок проиграл
		}
		else //карты-кикеры у данного игрока и его противника тоже совпали
		{
			return DRAW; //ничья
		}

	}
}


void Game_info::send_game_status(TgBot::Bot* bot, TgBot::Message::Ptr message) //отправить инфо о банке, фишках игрока и соперника
{
	string game_status = "Текущие показатели:\n\nБанк: " + to_string(pot) + word_chip(pot)
		+ "\nВаш стек: " + to_string(player_stack) + word_chip(player_stack)
		+ "\nСтек соперника: " + to_string(opponent_stack) + word_chip(opponent_stack)
		+ "\n\nВаша текущая ставка: " + to_string(player_bet) + word_chip(player_bet)
		+ "\nТекущая ставка соперника: " + to_string(opponent_bet) + word_chip(opponent_bet);
	if (player_cards.size() > 0)
	{
		game_status += "\n\nВаши карманные карты:";
		for (int i = 0; i < player_cards.size(); i++)
		{
			game_status += "\n" + player_cards[i].get_name();
		}
	}
	if (common_cards.size() > 0)
	{
		game_status += "\n\nОбщие карты:";
		for (int i = 0; i < common_cards.size(); i++)
		{
			game_status += "\n" + common_cards[i].get_name();
		}
	}

	bot->getApi().sendMessage(message->chat->id, game_status);
}

void Game_info::send_main_menu(TgBot::Bot* bot, TgBot::Message::Ptr message) //отправить сообщение с основными командами вне игры (аналогично стартовому сообщению)
{
	bot->getApi().sendMessage(message->chat->id, "Главное меню\n\n" + string(MAIN_MENU_MSG));
}

void Game_info::statistics(TgBot::Bot* bot, TgBot::Message::Ptr message) //вывести статистику выигрышей и проигрышей
{
	double percent_wins = 0; //средний процент побед игрока
	double percent_losses = 0; //средний процент поражений игрока
	if (wins_qty != 0)
	{
		percent_wins = round(double(wins_qty) / (wins_qty + losses_qty + draws_qty) * 100 * 100) / 100; //округлить до 2 знаков после запятой
	}
	if (losses_qty != 0)
	{
		percent_losses = round(double(losses_qty) / (wins_qty + losses_qty + draws_qty) * 100 * 100) / 100; //округлить до 2 знаков после запятой
	}

	string str_percent_wins = to_string(percent_wins); //перевод double в string
	string str_percent_losses = to_string(percent_losses); //перевод double в string
	str_percent_wins.erase(str_percent_wins.size() - 4); //стереть 4 лишних нуля в дробной части (нужно из-за исп. типа double)
	str_percent_losses.erase(str_percent_losses.size() - 4); //стереть 4 лишних нуля в дробной части (нужно из-за исп. типа double)

	string stat = "Ваша статистика за всё время:\n\nПроведено игр: " + to_string(wins_qty + losses_qty + draws_qty)
		+ "\nПобеды: " + to_string(wins_qty) + " (" + str_percent_wins + "%)"
		+ "\nПоражения: " + to_string(losses_qty) + " (" + str_percent_losses + "%)"
		+ "\nНичьи: " + to_string(draws_qty)
		+ "\nКол-во выигранных фишек: " + to_string(won_chips) + " шт."
		+ "\nКол-во проигранных фишек: " + to_string(lost_chips) + " шт."
		+ "\nИтоговое кол-во фишек: " + to_string(won_chips - lost_chips) + " шт.";

	bot->getApi().sendMessage(message->chat->id, stat); //вывести статистику
	send_actual_commands(false, bot, message); //вывести доступные сейчас команды
}

void Game_info::send_combinations_after_fold(TgBot::Bot* bot, TgBot::Message::Ptr message) //вывести потенциальные карточные комбинации игроков после сброса карт или выхода из игры
{
	if (player_cards.size() < QTY_POCKET_CARDS || opponent_cards.size() < QTY_POCKET_CARDS)
		return; //у игроков нет карт, выводить в сообщении нечего

	string str_output; //строка для вывода сообщения

	if (common_cards.size() < QTY_FLOP_COMMON_CARDS) //общих карт ещё нет, есть только карманные
	{
		str_output = "Теперь можете оценить, какие у вас были шансы на победу в прошедшей игре, сравнив карты:";
		str_output += "\n\nВаши карманные карты:";
		for (int i = 0; i < player_cards.size(); i++)
			str_output += "\n" + player_cards[i].get_name();

		str_output += "\n\nКарманные карты соперника:";
		for (int i = 0; i < opponent_cards.size(); i++)
			str_output += "\n" + opponent_cards[i].get_name();
	}
	else
	{
		str_output = "Теперь можете оценить, какие у вас были шансы на победу в прошедшей игре, сравнив комбинации:";
		str_output += "\n\nВаши карманные карты:";
		for (int i = 0; i < player_cards.size(); i++)
			str_output += "\n" + player_cards[i].get_name();

		str_output += "\n\nКарманные карты соперника:";
		for (int i = 0; i < opponent_cards.size(); i++)
			str_output += "\n" + opponent_cards[i].get_name();

		str_output += "\n\nОбщие карты:";
		for (int i = 0; i < common_cards.size(); i++)
			str_output += "\n" + common_cards[i].get_name();

		int player_combination_type; //тип собранной комбинации игрока
		int player_kicker_value; //значение кикера игрока
		vector <Playing_card> player_combination_cards = determine_card_combination(player_cards, common_cards, &player_combination_type, &player_kicker_value); //карты в комбинации игрока (для вывода на экран)

		int opponent_combination_type; //тип собранной комбинации соперника
		int opponent_kicker_value; //значение кикера соперника
		vector <Playing_card> opponent_combination_cards = determine_card_combination(opponent_cards, common_cards, &opponent_combination_type, &opponent_kicker_value); //карты в комбинации соперника (для вывода на экран)

		str_output += "\n\nВы могли бы собрать комбинацию " + get_combination_name(player_combination_type) + ":";
		for (int i = 0; i < player_combination_cards.size(); i++) //вывести комбинацию карт игрока
			str_output += "\n" + player_combination_cards[i].get_name();

		str_output += "\n\nСоперник мог бы собрать комбинацию " + get_combination_name(opponent_combination_type) + ":";
		for (int i = 0; i < opponent_combination_cards.size(); i++) //вывести комбинацию карт игрока
			str_output += "\n" + opponent_combination_cards[i].get_name();
	}
	bot->getApi().sendMessage(message->chat->id, str_output);
}

void Game_info::send_actual_commands(bool is_was_help_exit, TgBot::Bot* bot, TgBot::Message::Ptr message) //вывести сообщение с текущими доступными командами
{
	if (f_game_stage == GAME_NOT_STARTED)
	{
		if(is_was_help_exit == true)
			bot->getApi().sendMessage(message->chat->id, "Вы сейчас не находитесь в игре, поэтому вернулись в главное меню");
		send_main_menu(bot, message); //вывести команды главного меню
		return;
	}
	send_game_status(bot, message); //вывести в сообщении текущее состояние стека, банка и карт
	string str_output = "Продолжается раунд ";
	switch (f_game_stage)
	{
	case PREFLOP:
		str_output += "1 (Preflop).\n\n";
		break;
	case FLOP:
		str_output += "2 (Flop).\n\n";
		break;
	case TURN:
		str_output += "3 (Turn).\n\n";
		break;
	case RIVER:
		str_output += "4 (River).\n\n";
		break;
	}

	if (f_stage_action == BEGIN_OF_STAGE)
	{
		str_output += "Вам следует поставить блайнд — ";
		if (is_player_should_bet_big_blind)
			str_output += to_string(big_blind) + word_chip(big_blind, ACCUSATIVE);
		else
			str_output += to_string(big_blind / 2) + word_chip(big_blind / 2, ACCUSATIVE);
		str_output += ".\n\n";
		bot->getApi().sendMessage(message->chat->id, str_output + MSG_BEFORE_BLIND);
		return;
	}
	if (f_stage_action == BETTING_ROUND)
	{
		bot->getApi().sendMessage(message->chat->id, str_output + TAKE_ACTON_MSG);
		return;
	}
}


Playing_card Game_info::get_rand_card(vector<Playing_card> now_player_cards, vector<Playing_card> now_opponent_cards, vector<Playing_card> now_common_cards) //получить случайную карту, не совпадающую с карманными картами игрока, соперника и общими картами
{
	uniform_int_distribution<int> suit_range(FISRT_SUIT, LAST_SUIT); //диапазон для случайной генерации масти карты
	uniform_int_distribution<int> value_range(MIN_VALUE, MAX_VALUE); //диапазон для случайной генерации достоинства карты

	int rand_value;  //случайное достоинство карты
	int rand_suit; //случайная масть карты
	bool f_duplicate; //флаг, что найдено совпадение с картами игрока, соперника или общими картами

	do //генерировать случайные значения, пока не будет найдена карта, не совпадающая с имеющимися
	{
		rand_value = value_range(random_generator); //случайное достоинство карты
		rand_suit = suit_range(random_generator); //случайная масть карты
		f_duplicate = false;

		for (int i = 0; i < now_player_cards.size(); i++)
		{
			if (now_player_cards[i].get_value() == rand_value && now_player_cards[i].get_suit() == rand_suit)
			{
				f_duplicate = true;
				break;
			}
		}
		for (int i = 0; i < now_opponent_cards.size(); i++)
		{
			if (now_opponent_cards[i].get_value() == rand_value && now_opponent_cards[i].get_suit() == rand_suit)
			{
				f_duplicate = true;
				break;
			}
		}
		for (int i = 0; i < now_common_cards.size(); i++)
		{
			if (now_common_cards[i].get_value() == rand_value && now_common_cards[i].get_suit() == rand_suit)
			{
				f_duplicate = true;
				break;
			}
		}
	} while (f_duplicate == true);

	Playing_card rand_card(rand_value, rand_suit);
	return  rand_card;
}

vector <Playing_card> Game_info::determine_card_combination(vector<Playing_card> now_pocket_cards, vector<Playing_card> now_common_cards, int* combination_type, int* kicker_value) //определить карточную комбинацию и кикер для сравнения комбинаций игроков
{
	vector <Playing_card> card_combination; //карты в комбинации
	if (now_pocket_cards.size() < QTY_POCKET_CARDS) //если карт у игрока ещё нет, то вернуть значение пустой комбинации
	{
		(*combination_type) = EMPTY_CARDS;
		(*kicker_value) = EMPTY_CARDS;
		return card_combination;
	}
	
	if (now_common_cards.size() == 0) //общие карты ещё не были розданы, но у игроков есть карманные карты
	{
		//(в данном случае функция используется соперником для просчёта хода в начале игры, поэтому выводить карты из комбинации в return нет надобности)
		if (now_pocket_cards[0].get_value() == now_pocket_cards[1].get_value()) //среди карманных карт есть пара
		{
			(*combination_type) = PAIR;
			(*kicker_value) = now_pocket_cards[0].get_value();
		}
		else //определить старшую карту среди карманных карт
		{
			(*combination_type) = HIGHCARD;
			if (now_pocket_cards[0].get_value() >= now_pocket_cards[1].get_value())
				(*kicker_value) = now_pocket_cards[0].get_value();
			else
				(*kicker_value) = now_pocket_cards[1].get_value();
		}
		return card_combination;
	}
	else //общие карты уже розданы (как минимум 3 шт.)
	{
		vector <Playing_card> pocket_and_common_cards; //карты, из которых составляется комбинация (карманные карты игрока + общие карты)
		pocket_and_common_cards = now_pocket_cards; //скопировать карманные карты в карты для комбинаций
		for (int i = 0; i < now_common_cards.size(); i++) //скопировать общие карты в карты для комбинаций
			pocket_and_common_cards.push_back(now_common_cards[i]);
		
		//сортировка карт по убыванию достоинств
		for (int k = 1; k < pocket_and_common_cards.size(); k++) //метод пузырька
			for (int i = 0; i < pocket_and_common_cards.size() - k; i++)
				if (pocket_and_common_cards[i].get_value() < pocket_and_common_cards[i + 1].get_value())
				{
					Playing_card tmp = pocket_and_common_cards[i];
					pocket_and_common_cards[i] = pocket_and_common_cards[i + 1];
					pocket_and_common_cards[i + 1] = tmp;
				}
		
		//поиск комбинации стрит-флеш (и роял-флеш)
		for (int i = 0; i < pocket_and_common_cards.size(); i++)
		{
			card_combination.clear(); //очистить комбинацию
			card_combination.push_back(pocket_and_common_cards[i]); //поместить карту - начало комбинации

			for (int j = i + 1; j < pocket_and_common_cards.size(); j++) //найти оставшиеся 4 карты комбинации
			{
				if (pocket_and_common_cards[j].get_value() == card_combination.back().get_value()-1 && pocket_and_common_cards[j].get_suit() == card_combination.back().get_suit())
				{
					card_combination.push_back(pocket_and_common_cards[j]);
					if (card_combination.size() == COMBINATION_SIZE)
						break;
				}
			}
			if (card_combination.size() == COMBINATION_SIZE-1 && card_combination.back().get_value() == TWO) //найти туз для комбинации - младший стрит-флеш (5,4,3,2,A)
			{
				for (int j = 0; j < pocket_and_common_cards.size(); j++)
				{
					if (pocket_and_common_cards[j].get_value() == ACE && pocket_and_common_cards[j].get_suit() == card_combination.back().get_suit())
					{
						card_combination.push_back(pocket_and_common_cards[j]);
						break;
					}
				}
			}
			if (card_combination.size() == COMBINATION_SIZE) //комбинация из 5 карт найдена
			{
				(*kicker_value) = card_combination.front().get_value();
				if (*kicker_value == ACE) //найден роял-флеш
					(*combination_type) = ROYAL_FLUSH;
				else //найден стрит-флеш
					(*combination_type) = STRAIGHT_FLUSH;
				return card_combination;
			}
		}
		
		//поиск комбинации каре
		for (int i = 0; i < pocket_and_common_cards.size(); i++)
		{
			card_combination.clear(); //очистить комбинацию
			card_combination.push_back(pocket_and_common_cards[i]); //поместить карту - начало комбинации

			for (int j = i + 1; j < pocket_and_common_cards.size(); j++) //найти оставшиеся 4 карты комбинации
			{
				if (pocket_and_common_cards[j].get_value() == card_combination.back().get_value())
				{
					card_combination.push_back(pocket_and_common_cards[j]);
					if (card_combination.size() == FOUR_OF_A_KIND_SIZE)
						break;
				}
			}

			if (card_combination.size() == FOUR_OF_A_KIND_SIZE) //комбинация каре найдена
			{
				(*combination_type) = FOUR_OF_A_KIND;
				Playing_card kicker(0, 0);
				for (int j = 0; j < pocket_and_common_cards.size(); j++) //поиск кикера
				{
					if (pocket_and_common_cards[j].get_value() != card_combination.front().get_value())
					{
						kicker = pocket_and_common_cards[j];
						break;
					}
				}
				(*kicker_value) = kicker.get_value();
				card_combination.push_back(kicker); //кикер тоже является частью комбинации
				return card_combination;
			}
		}

		//поиск комбинации фуллхаус (сет + пара)
		for (int i = 0; i < pocket_and_common_cards.size(); i++)
		{
			card_combination.clear(); //очистить комбинацию
			card_combination.push_back(pocket_and_common_cards[i]); //поместить карту - начало комбинации
			for (int j = i + 1; j < pocket_and_common_cards.size(); j++) //поиск сета
			{
				if (pocket_and_common_cards[j].get_value() == card_combination.back().get_value())
				{
					card_combination.push_back(pocket_and_common_cards[j]);
					if (card_combination.size() == THREE_OF_A_KIND_SIZE)
						break;
				}
			}
			if (card_combination.size() == THREE_OF_A_KIND_SIZE) //сет найден
			{
				Playing_card pair_card(0,0); //первая карта из пары
				bool f_success_pair = false; //флаг успеха нахождения второй карты из пары
				for (int j = 0; j < pocket_and_common_cards.size(); j++) //поиск первой карты из пары
				{
					if (pocket_and_common_cards[j].get_value() != card_combination.front().get_value()) //парная карта не является частью сета
					{
						pair_card = pocket_and_common_cards[j];
						for (int k = j + 1; k < pocket_and_common_cards.size(); k++) //поиск второй карты из пары
						{
							if (pocket_and_common_cards[k].get_value() == pair_card.get_value()) //пара найдена
							{
								f_success_pair = true;
								card_combination.push_back(pair_card);
								card_combination.push_back(pocket_and_common_cards[k]);
								break;
							}
							else
								break; //из-за сортировки по убыванию парная карта должна находиться рядом, иначе её вообще нет
						}
						if (f_success_pair == true) //пара найдена
							break;
					}
				}
				if (f_success_pair == true) //пара найдена, значит фуллхаус собран
				{
					(*combination_type) = FULL_HOUSE;
					(*kicker_value) = card_combination.front().get_value();
					return card_combination;
				}
			}
		}
		
		//поиск комбинации флеш
		for (int i = 0; i < pocket_and_common_cards.size(); i++)
		{
			card_combination.clear(); //очистить комбинацию
			card_combination.push_back(pocket_and_common_cards[i]); //поместить карту - начало комбинации
			for (int j = i + 1; j < pocket_and_common_cards.size(); j++) //поиск комбинации
			{
				if (pocket_and_common_cards[j].get_suit() == card_combination.back().get_suit())
				{
					card_combination.push_back(pocket_and_common_cards[j]);
					if (card_combination.size() == COMBINATION_SIZE)
					{
						break;
					}
				}
			}
			if (card_combination.size() == COMBINATION_SIZE) //комбинация флеш найдена
			{
				(*combination_type) = FLUSH;
				(*kicker_value) = card_combination.front().get_value();
				return card_combination;
			}
		}

		//поиск комбинации стрит
		for (int i = 0; i < pocket_and_common_cards.size(); i++)
		{
			card_combination.clear(); //очистить комбинацию
			card_combination.push_back(pocket_and_common_cards[i]); //поместить карту - начало комбинации

			for (int j = i + 1; j < pocket_and_common_cards.size(); j++) //найти оставшиеся 4 карты комбинации
			{
				if (pocket_and_common_cards[j].get_value() == card_combination.back().get_value() - 1)
				{
					card_combination.push_back(pocket_and_common_cards[j]);
					if (card_combination.size() == COMBINATION_SIZE)
						break;
				}
			}
			if (card_combination.size() == COMBINATION_SIZE-1 && card_combination.back().get_value() == TWO) //найти туз для комбинации - младший стрит (5,4,3,2,A)
			{
				for (int j = 0; j < pocket_and_common_cards.size(); j++)
				{
					if (pocket_and_common_cards[j].get_value() == ACE)
					{
						card_combination.push_back(pocket_and_common_cards[j]);
						break;
					}
				}
			}
			if (card_combination.size() == COMBINATION_SIZE) //комбинация из 5 карт найдена
			{
				(*kicker_value) = card_combination.front().get_value();
				(*combination_type) = STRAIGHT;
				return card_combination;
			}
		}

		//поиск комбинации сет
		for (int i = 0; i < pocket_and_common_cards.size(); i++)
		{
			card_combination.clear(); //очистить комбинацию
			card_combination.push_back(pocket_and_common_cards[i]); //поместить карту - начало комбинации
			for (int j = i + 1; j < pocket_and_common_cards.size(); j++) //поиск комбинации
			{
				if (pocket_and_common_cards[j].get_value() == card_combination.back().get_value())
				{
					card_combination.push_back(pocket_and_common_cards[j]);
					if (card_combination.size() == THREE_OF_A_KIND_SIZE)
					{
						break;
					}
				}
			}
			if (card_combination.size() == THREE_OF_A_KIND_SIZE) //сет найден
			{
				(*combination_type) = THREE_OF_A_KIND;
				Playing_card kicker(0, 0);
				for (int j = 0; j < pocket_and_common_cards.size(); j++) //поиск кикера
				{
					if (pocket_and_common_cards[j].get_value() != card_combination.front().get_value())
					{
						kicker = pocket_and_common_cards[j];
						break;
					}
				}
				(*kicker_value) = kicker.get_value();
				card_combination.push_back(kicker); //кикер тоже является частью комбинации
				for (int j = 0; j < pocket_and_common_cards.size(); j++) //поиск 5-й карты комбнации (любой из оставшихся)
				{
					if (pocket_and_common_cards[j].get_value() != card_combination.front().get_value() && pocket_and_common_cards[j].get_value() != card_combination.back().get_value())
					{
						card_combination.push_back(pocket_and_common_cards[j]);
						break;
					}
				}
				return card_combination;
			}
		}
		
		//поиск комбинации две пары
		for (int i = 0; i < pocket_and_common_cards.size(); i++)
		{
			card_combination.clear(); //очистить комбинацию
			card_combination.push_back(pocket_and_common_cards[i]); //поместить карту - начало комбинации
			for (int j = i + 1; j < pocket_and_common_cards.size(); j++) //поиск первой пары
			{
				if (pocket_and_common_cards[j].get_value() == card_combination.back().get_value())
				{
					card_combination.push_back(pocket_and_common_cards[j]);
					break;
				}
				else
					break; //из-за сортировки по убыванию парная карта должна находиться рядом, иначе её вообще нет
			}
			if (card_combination.size() == PAIR_SIZE) //первая пара найдена
			{
				Playing_card second_pair_card(0, 0); //первая карта из второй пары
				bool f_success_pair = false; //флаг успеха нахождения второй карты из второй пары
				for (int j = 0; j < pocket_and_common_cards.size(); j++) //поиск первой карты из второй пары
				{
					if (pocket_and_common_cards[j].get_value() != card_combination.front().get_value()) //парная карта не является частью первой пары
					{
						second_pair_card = pocket_and_common_cards[j];
						for (int k = j + 1; k < pocket_and_common_cards.size(); k++) //поиск второй карты из второй пары
						{
							if (pocket_and_common_cards[k].get_value() == second_pair_card.get_value()) //вторая пара найдена
							{
								f_success_pair = true;
								card_combination.push_back(second_pair_card);
								card_combination.push_back(pocket_and_common_cards[k]);
								break;
							}
							else
								break; //из-за сортировки по убыванию парная карта должна находиться рядом, иначе её вообще нет
						}
						if (f_success_pair == true) //вторая пара найдена
							break;
					}
				}
				if (f_success_pair == true) //вторая пара найдена, значит две пары собрано
				{
					(*combination_type) = TWO_PAIRS;
					Playing_card kicker(0, 0);
					for (int j = 0; j < pocket_and_common_cards.size(); j++) //поиск кикера
					{
						if (pocket_and_common_cards[j].get_value() != card_combination.front().get_value() && pocket_and_common_cards[j].get_value() != card_combination.back().get_value())
						{
							kicker = pocket_and_common_cards[j];
							break;
						}
					}
					(*kicker_value) = kicker.get_value();
					card_combination.push_back(kicker); //кикер тоже является частью комбинации
					return card_combination;
				}
			}
		}

		//поиск комбинации пара
		for (int i = 0; i < pocket_and_common_cards.size(); i++)
		{
			card_combination.clear(); //очистить комбинацию
			card_combination.push_back(pocket_and_common_cards[i]); //поместить карту - начало комбинации
			for (int j = i + 1; j < pocket_and_common_cards.size(); j++) //поиск пары
			{
				if (pocket_and_common_cards[j].get_value() == card_combination.back().get_value())
				{
					card_combination.push_back(pocket_and_common_cards[j]);
					break;
				}
				else
					break; ////из-за сортировки по убыванию парная карта должна находиться рядом, иначе её вообще нет
			}
			if (card_combination.size() == PAIR_SIZE) //пара найдена
			{
				(*combination_type) = PAIR;

				Playing_card kicker(0, 0);
				for (int j = 0; j < pocket_and_common_cards.size(); j++) //поиск кикера
				{
					if (pocket_and_common_cards[j].get_value() != card_combination.front().get_value())
					{
						kicker = pocket_and_common_cards[j];
						break;
					}
				}
				(*kicker_value) = kicker.get_value();
				card_combination.push_back(kicker); //кикер тоже является частью комбинации
				for (int j = 0; j < pocket_and_common_cards.size(); j++) //поиск 4-й и 5-й карт комбнации (любые из оставшихся)
				{
					if (pocket_and_common_cards[j].get_value() != card_combination.front().get_value() && pocket_and_common_cards[j].get_value() != kicker.get_value())
					{
						card_combination.push_back(pocket_and_common_cards[j]);
						if (card_combination.size() == COMBINATION_SIZE)
						{
							break;
						}
					}
				}
				return card_combination;
			}
		}
			
		//поиск старшей карты
		card_combination.clear();
		for (int i = 0; i < COMBINATION_SIZE; i++)
		{
			card_combination.push_back(pocket_and_common_cards[i]);
		}
		(*combination_type) = HIGHCARD;
		(*kicker_value) = card_combination.front().get_value();
		return card_combination;
	}
}

string Game_info::get_combination_name(int combination_type) //получить имя комбинации карт для вывода в сообщении
{
	switch (combination_type)
	{
	case ROYAL_FLUSH:
		return "Роял-флеш";
		break;
	case STRAIGHT_FLUSH:
		return "Стрит-флеш";
		break;
	case FOUR_OF_A_KIND:
		return "Каре";
		break;
	case FULL_HOUSE:
		return "Фуллхаус";
		break;
	case FLUSH:
		return "Флеш";
		break;
	case STRAIGHT:
		return "Стрит";
		break;
	case THREE_OF_A_KIND:
		return "Сет";
		break;
	case TWO_PAIRS:
		return "Две пары";
		break;
	case PAIR:
		return "Пара";
		break;
	default:
		return "Старшая карта";
	}
}


string Game_info::word_chip(int qty_chip) //получить слово "фишки" в именительном падеже с правильным окончанием в зависимости от кол-ва фишек
{
	int q = abs(qty_chip)%100; //взять число по модулю и отбросить сотни
	if (q % 10 == 1 && q != 11)
		return " фишка"; // "1 фишка"
	else if ((q % 10 >= 2 && q % 10 <= 4) && !(q >= 11 && q <= 14)) //"2 фишки"
		return " фишки";
	else
		return " фишек"; //"5 фишек"

}

string Game_info::word_chip(int qty_chip, int word_case) //получить слово "фишки" в родительном или винительном падеже с правильным окончанием в зависимости от кол-ва фишек
{
	int q = abs(qty_chip)%100; //взять число по модулю и отбросить сотни
	if (q % 10 == 1 && q != 11)
	{
		if (word_case == GENITIVE) //родительный падеж
			return " фишки"; // "до 1 фишки"
		else //винительный падеж
			return " фишку"; // "поставить 1 фишку"
	}
	else
	{
		if (word_case == GENITIVE) //родительный падеж
			return " фишек"; // "до 3 фишек"
		else //винительный падеж
			return word_chip(q); // "поставить 2 фишки", "поставить 5 фишек"
	}
}
