#include "Game_info.h"

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
		f_stage_action = BEGIN_OF_STAGE;
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
	fin >> f_stage_action;
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
	fout << f_stage_action << endl;
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
	f_stage_action = BEGIN_OF_STAGE;

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
	
	bot->getApi().sendMessage(message->chat->id, "🔷 Запущена новая игра\n\nБольшой блайнд = " + to_string(big_blind) + " фишки, малый блайнд = " + to_string(big_blind/2) + " фишка.");
	send_game_status(bot, message); //отправить инфо о банке, фишках игрока и соперника
	bot->getApi().sendMessage(message->chat->id, "🔹 Первый раунд : Preflop");

	uniform_int_distribution<int> f_big_blind_range(0, 1); //диапазон для случайной генерации флага большого блайнда
	is_player_should_bet_big_blind = f_big_blind_range(random_generator); //случайная генерация
	if (is_player_should_bet_big_blind == 1) //игрок должен сделать большой блайнд
	{
		make_bet(big_blind / 2, OPPONENT_BET); //соперник сделал малый блайнд
		bot->getApi().sendMessage(message->chat->id, "Согласно случайному выбору, вам выпал большой блайнд. Вам следует поставить " + to_string(big_blind) + " фишки."
			+ "\n\nВаш соперник сделал малый блайнд (" + to_string(big_blind / 2) + " фишка).\n\n" + MSG_BEFORE_BLIND);																																			/////////////////////////////////////////////
	}
	else //игрок должен сделать малый блайнд
	{
		bot->getApi().sendMessage(message->chat->id, "Согласно случайному выбору, вам выпал малый блайнд. Вам следует поставить " + to_string(big_blind/2) + " фишку."
			+ "\n\n" + MSG_BEFORE_BLIND);
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

void Game_info::send_game_status(TgBot::Bot* bot, TgBot::Message::Ptr message) //отправить инфо о банке, фишках игрока и соперника
{
	string game_status = "Текущие показатели:\nБанк: " + to_string(pot) + " фишек"
		+ "\nВаш стек: " + to_string(player_stack) + " фишек"
		+ "\nСтек соперника: " + to_string(opponent_stack) + " фишек"
		+ "\nВаша текущая ставка: " + to_string(player_bet) + "фишек"
		+ "\nТекущая ставка соперника: " + to_string(opponent_bet) + "фишек";
	if (player_cards.size()>0)
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

void Game_info::exit(TgBot::Bot* bot, TgBot::Message::Ptr message) //выйти из игры
{
	if (f_game_stage == GAME_NOT_STARTED)
	{
		bot->getApi().sendMessage(message->chat->id, "Вы сейчас не находитесь в игре, поэтому команда выхода не имеет смысла");
	}
	else
	{
		bot->getApi().sendMessage(message->chat->id, "Вы вышли из игры");
		end(false, bot, message); //выход из игры приравнивается к поражению

		write_to_file(); //запись значений в файл
	}
}

void Game_info::make_blind(TgBot::Bot* bot, TgBot::Message::Ptr message) //сделать блайнд
{
	if (f_game_stage == GAME_NOT_STARTED)
	{
		bot->getApi().sendMessage(message->chat->id, "Вы сейчас не находитесь в игре, поэтому команда блайнда не имеет смысла");
		return;
	}
	if (f_stage_action != BEGIN_OF_STAGE)
	{
		bot->getApi().sendMessage(message->chat->id, "В данный момент игры команда блайнда не имеет смысла");
		return;
	}

	bool f_blind_is_successful = false; //флаг об успешности ставки
	if (is_player_should_bet_big_blind == 0) //игрок должен сделать малый блайнд
	{
		f_blind_is_successful = make_bet(big_blind / 2, PLAYER_BET); //сделать ставку
	}
	else //игрок должен сделать большой блайнд
	{
		f_blind_is_successful = make_bet(big_blind, PLAYER_BET); //сделать ставку
	}
	if (f_blind_is_successful == false) //у игрока недостаточно фишек для блайнда
	{
		bot->getApi().sendMessage(message->chat->id, "У вас недостаточно фишек, чтобы сделать блайнд — обязательную ставку");
		end(false, bot, message);
	}

	if (is_player_should_bet_big_blind == 0) //игрок должен был сделать малый блайнд
	{
		make_bet(big_blind, OPPONENT_BET); //соперник сделал большой блайнд
		bot->getApi().sendMessage(message->chat->id, "Ваш соперник сделал большой блайнд (" + to_string(big_blind) + " фишки).");
	}
	
	switch (f_game_stage)
	{
	case PREFLOP:
		//раздача 2 карманных карт игрокам по 1 карте, сначала карту получает игрок с малым блайндом
		if (is_player_should_bet_big_blind == 0) //игрок должен был сделать малый блайнд
		{
			player_cards.push_back(get_rand_card());
			opponent_cards.push_back(get_rand_card());
			player_cards.push_back(get_rand_card());
			opponent_cards.push_back(get_rand_card());
		}
		else //игрок должен был сделать большой блайнд
		{
			opponent_cards.push_back(get_rand_card());
			player_cards.push_back(get_rand_card());
			opponent_cards.push_back(get_rand_card());
			player_cards.push_back(get_rand_card());
		}
		bot->getApi().sendMessage(message->chat->id, "Вам и вашему сопернику розданы карманные карты");
		break;
	case FLOP:
		//раздача 3 общих карт
		common_cards.push_back(get_rand_card());
		common_cards.push_back(get_rand_card());
		common_cards.push_back(get_rand_card());
		bot->getApi().sendMessage(message->chat->id, "Розданы 3 общие карты");

		break;
	case TURN:
		//раздача 4-й общей карты
		common_cards.push_back(get_rand_card());
		bot->getApi().sendMessage(message->chat->id, "Роздана четвёртая общая карта");
		break;
	case RIVER:
		//раздача 5-й общей карты
		common_cards.push_back(get_rand_card());
		bot->getApi().sendMessage(message->chat->id, "Роздана пятая общая карта");
		break;
	}
	f_stage_action = BETTING_ROUND; //установить флаг текущего состояния игры в значение торговли
	send_game_status(bot, message); //вывести в сообщении текущее состояние стека, банка и карт
	bot->getApi().sendMessage(message->chat->id, "Начат круг торговли");

	//первым в торгах в префлопе действует игрок с малым блайндом, в остальных раундах - игрок с большим блайндом
	//проверка, должен ли соперник первым действовать в торгах
	if ((is_player_should_bet_big_blind == 1 && f_game_stage == PREFLOP) || (is_player_should_bet_big_blind == 0 && f_game_stage != PREFLOP))
	{
		auto_action(bot, message);//соперник действует в торгах
	}

	bot->getApi().sendMessage(message->chat->id, TAKE_ACTON_MSG); //сообщение с списком возможных действий игрока в круге торговли

	write_to_file(); //запись значений в файл
}

void Game_info::end(bool player_wins, TgBot::Bot* bot, TgBot::Message::Ptr message) //конец игры
{
	f_game_stage = GAME_NOT_STARTED;
	if (player_wins == false) //игрок проиграл
	{
		bot->getApi().sendMessage(message->chat->id, "Игра окончена, вы проиграли.\n\nВаш соперник забирает банк: " + to_string(pot) + " фишек.\nВаш результат: -" + to_string(DEFAULT_PLAYER_STACK - player_stack) + " фишек.\nРезультат противника: +" + to_string(pot - (DEFAULT_OPPONENT_STACK - opponent_stack)) + "фишек.");
	}
	else //игрок победил
	{
		bot->getApi().sendMessage(message->chat->id, "Игра окончена, вы выиграли.\n\nВы забираете банк: " + to_string(pot) + " фишек.\nВаш результат: +" + to_string(pot - (DEFAULT_PLAYER_STACK - player_stack)) + " фишек.\nРезультат противника: -" + to_string(DEFAULT_OPPONENT_STACK - opponent_stack) + "фишек.");
	}

	write_to_file(); //запись значений в файл
}

void Game_info::auto_action(TgBot::Bot* bot, TgBot::Message::Ptr message) //ставка соперника в круге торговли
{
	///////////////////////////////
}

void Game_info::action_of_player(int type_of_action, int bet_size, TgBot::Bot* bot, TgBot::Message::Ptr message) //действие игрока в круге торговли
{
	if (f_game_stage == GAME_NOT_STARTED)
	{
		bot->getApi().sendMessage(message->chat->id, "Вы сейчас не находитесь в игре, поэтому команда торговли не имеет смысла");
		return;
	}
	if (f_stage_action != BETTING_ROUND)
	{
		bot->getApi().sendMessage(message->chat->id, "В данный момент игры команда торговли не имеет смысла");
		return;
	}
	bool f_success = false; //флаг успеха действия
	switch (type_of_action)
	{
	case RAISE:
		f_success = raise(bet_size, PLAYER_BET);
		if (f_success == true)
		{
			bot->getApi().sendMessage(message->chat->id, "Вы повысили ставку до " + to_string(player_bet) + " фишек");
		}
		else
		{
			bot->getApi().sendMessage(message->chat->id, "Повысить ставку не удалось. Проверьте правильность ввода. Рейз должен быть больше текущих ставок, быть кратным большому блайнду, и у вас должно быть достаточно фишек в стеке");
			send_game_status(bot, message); //отправить инфо о банке, фишках игрока и соперника
			bot->getApi().sendMessage(message->chat->id, TAKE_ACTON_MSG); //сообщение с списком возможных действий игрока в круге торговли
			return; //прекратить дальшейшее выполнение действий и ждать повторного ввода
		}
		break;
	case CALL:
		f_success = call(PLAYER_BET);
		if (f_success == true)
		{
			bot->getApi().sendMessage(message->chat->id, "Вы уравняли вашу ставку до " + to_string(player_bet) + " фишек");
		}
		else
		{
			bot->getApi().sendMessage(message->chat->id, "Уравнять ставку не удалось. У вас недостаточно фишек в стеке. Выберите другое действие");
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

	send_game_status(bot, message); //отправить инфо о банке, фишках игрока и соперника

	if (player_bet == opponent_bet) //если ставки уравнялись
	{
		to_next_stage(bot, message); //перейти к следующей стадии игры
	}
	else
	{
		auto_action(bot, message); //соперник действует в торгах
	}

	bot->getApi().sendMessage(message->chat->id, TAKE_ACTON_MSG); //сообщение с списком возможных действий игрока в круге торговли
}

bool Game_info::raise(int bet_size, int player_or_opponent) //повысить ставку
{
	if (bet_size <= player_bet || bet_size <= opponent_bet) //если ставка меньше, чем текущие ставки игрока и соперника, или равна им
	{
		return false;
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
		return make_bet(opponent_bet, PLAYER_BET);
	}
	else //действие соперника
	{
		return make_bet(player_bet, OPPONENT_BET);
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
		end(false, bot, message); //игрок проиграл
	}
	else //действие соперника
	{
		bot->getApi().sendMessage(message->chat->id, "Ваш соперник сбросил карты");
		end(true, bot, message); //игрок выиграл
	}
}

void Game_info::to_next_stage(TgBot::Bot* bot, TgBot::Message::Ptr message) //перейти к следующей стадии игры
{
	f_game_stage++; //установить флаг следующей стадии игры
	f_stage_action = BEGIN_OF_STAGE; //установить флаг начала стадии
	bot->getApi().sendMessage(message->chat->id, "Ставки уравнены, торги завершены");

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
		bot->getApi().sendMessage(message->chat->id, "🔹 Второй раунд : Flop");
		break;
	case TURN:
		bot->getApi().sendMessage(message->chat->id, "🔹 Третий раунд : Turn");
		break;
	case RIVER:
		bot->getApi().sendMessage(message->chat->id, "🔹 Четвёртый раунд : River");
		break;
	case SHOWDOWN:
		bot->getApi().sendMessage(message->chat->id, "🔹 Вскрытие карт");
		//////////////////////////////////

		return;
		break;
	}

	if (is_player_should_bet_big_blind == 1) //игрок должен сделать большой блайнд
	{
		
	}
	else //игрок должен сделать малый блайнд
	{
		
	}
}