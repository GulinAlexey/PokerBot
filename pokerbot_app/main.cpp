#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <ctime>   
#include <tgbot/tgbot.h>
#include "Game_info.h"

#define SYSTEM_CREATE_FOLDER "mkdir -p profiles/" //системная команда для создания директории с профилями пользователей

#define PATH_OF_TOKEN "data/token.dat" //путь к файлу, в котором хранится токен телеграм-бота
#define PATH_OF_LOG "log.txt" //путь к файлу для записи в лог

#define LOG_MAX_SIZE 6000000 //(примерно равно 5,7 МБ) - максимальный размер файла лога в символах 

#define START_TEXT "Добро пожаловать! Это бот для игры в покер на виртуальные фишки (один на один)\n\n"
#define HELP_TEXT "Справка об игре. Рекомендуем прочитать перед началом игры.\n\nВыберите интересующую вас информацию и нажмите на команду:\n\n/terms — Игровые термины\n/rules — Правила игры\n/combinations — Покерные комбинации карт\n/about_opponent — О сопернике в игре\n/author — О проекте\n/commands — Доступные на данный момент действия в игре\n/exit_help — Выйти из меню справки и вернуться к игре"
#define TERMS_TEXT "Игровые термины:\n\n● Фишки — игровые единицы, позволяющие делать ставки. В конце игры победитель забирает фишки из банка. В разделе Статистики можно посмотреть общее число заработанных фишек за все сессии. На каждую сессию выделяется ровно 100 фишек независимо от предыдущих побед/поражений.\n● Банк — сумма всех ставок игроков за данную игровую сессию.\n● Стек — кол-во фишек на руках у игрока, которые он может поставить.\n● Блайнд — обязательная ставка в начале каждого раунда. Бывают большой и малый блайнды. Большой блайнд в два раза больше малого.\n\n● Карманные карты — 2 закрытые карты игрока, розданные ему в раунде Preflop после блайндов. Значение карманных карт известно только самому игроку, а сопернику оно станет известно после вскрытия карт в конце игры.\n● Общие карты — 3, 4 или 5 открытых карт, розданных в раунде Flop и последующих. Все игроки видят значение данных карт.\n\n● Preflop — 1-й раунд игры. Игрокам раздаётся по 2 карманные карты, затем следует круг торгов.\n● Flop — 2-й раунд игры. Раздаётся 3 общие карты, затем следует круг торгов.\n● Turn — 3-й раунд игры. Раздаётся 4-я общая карта, затем следует круг торгов.\n● River — 4-й раунд игры. Раздаётся 5-я общая карта, затем следует круг торгов.\n● Вскрытие карт (Showdown) — 5-й (последний) раунд игры. Значение карманных карт игроков становится общеизвестным, и программа автоматически определяет игрока с более сильной комбинацией из его карманных карт и общих карт. Победитель забирает банк.\n\n● Круг торговли — этап, который будет завершён, как только ставки игроков сравняются. Игроки по очереди совершают действия из доступных (raise, call, check, fold).\n● Raise — повышение ставки до введённого значения. Для ставки у игрока должно быть достаточно фишек, ставка должна быть кратна большому блайнду. В данной реализации игры поставить все свои фишки (all-in) можно только в раунде River. В более ранних раундах можно делать ставки так, чтобы оставались фишки на блайнды в следующих раундах (например, оставить 6 фишек в стеке в раунде Preflop).\n● Call — уравнять свою ставку ставке соперника. После уравнивания ставок круг торгов завершается и начинается следующий раунд.\n● Check — передать ход сопернику. Можно только в случае, если в данном раунде вы поставили большой блайнд, и никто ещё не повышал ставок.\n● Fold — сбросить карты и завершить игру (проиграть). Имеет смысл, если вы считаете свои карты слабыми и не хотите потерять ещё больше фишек в процессе ставок. После сброса карт вы сможете узнать их значение.\n\n● Кикер — карта, по значению которой определяется победитель, если комбинации карт совпали. Например, оба игрока собрали каре, но кикеры у них разные. Если и кикеры совпадают, то игра заканчивается ничьей.\n\nВернуться в меню справки: /help\nВернуться к игре: /exit_help"
#define RULES_TEXT "Правила игры:\n\nДанный бот позволяет сыграть вам в покер. Разновидность — Техасский холдем, тип — Хедз-ап (один на один). Используется колода из 52 карт без джокеров (четыре масти, значения от двойки до туза).\n\nПримечание 1: Нежелательно отправлять боту новое сообщение, пока он не ответил на предыдущее. \n\nПримечание 2: Если символы мастей Пики и Трефы трудно отличимы друг от друга, советуем вам отключить тёмную тему в Telegram.\n\nВ начале игры у вас и вашего соперника есть по 100 фишек (виртуальных и не связанных с реальными деньгами), а банк пуст. Один из игроков (выбранный случайно) делает малый блайнд, а другой игрок — большой блайнд. Затем наступает раунд Preflop и игрокам раздаётся по две карманные карты, начинается круг торгов (первым действует игрок с малым блайндом). После круга торгов делаются блайнды (теперь игрок с предыдущим малым блайндом делает большой и наоборот), наступает раунд Flop. Игрокам раздаётся три общие карты, следует круг торгов (в данном и последующих раундах первым действует игрок с большим блайндом). Затем — блайнды, этап Turn (4-я общая карта) и круг торгов. После чего — блайнды, этап River (5-я общая карта) и круг торгов. После завершения торгов карты вскрываются и определяется победитель с более сильной комбинацией. Победитель забирает банк. Если комбинации совпали, то определяется игрок с более сильным кикером. Если кикеры совпали, то наступает ничья. При ничье игроки делят банк поровну (И выходят в ноль из-за уравненных ранее ставок).\n\nВернуться в меню справки: /help\nВернуться к игре: /exit_help"
#define COMBINATIONS_TEXT "Комбинации карт от более сильных к более слабым:\n1● Роял-флеш. Пять последовательных по старшинству карт одной масти от туза до десятки.\n2● Стрит-флеш. Пять последовательных по старшинству карт одной масти (старшей картой является не туз). Возможен малый стрит-флеш от пятёрки до туза, где туз является младшей картой. В качестве кикера используется старшая карта из комбинации.\n3● Каре. Комбинация из четырех карт одинакового ранга. Например, четыре короля и одна карта другого ранга. Пятой картой выбирается сильнейшая из оставшихся карт и выступает в роли кикера.\n4● Фуллхаус. Три и две карты одного достоинства (сет и пара). Например, два валета и три восьмёрки. В качестве значения кикера используется значение карты из сета.\n5● Флеш. Пять карт одной масти. В качестве кикера используется старшая карта из комбинации.\n6● Стрит. Пять последовательных по старшинству карт (масти не всех карт совпадают). Возможен малый стрит от пятёрки до туза, где туз является младшей картой. В качестве кикера используется старшая карта из комбинации.\n7● Сет. Три карты одного ранга. Например, три дамы. Оставшиеся две карты могут быть любыми, но сильнейшая из них выступает в роли кикера.\n8● Две пары. Две карты одного ранга и две карты другого ранга. Например, два короля и два туза. Пятой картой выбирается сильнейшая из оставшихся карт и выступает в роли кикера.\n9● Пара. Две карты одного ранга. Оставшиеся три карты могут быть любыми, но сильнейшая из них выступает в роли кикера.\n10● Старшая карта. Самая сильная из карт игрока. Например, король. Остальные четыре карты могут быть любыми. Старшая карта является одновременно и кикером.\n\nВернуться в меню справки: /help\nВернуться к игре: /exit_help"
#define ABOUT_OPPONENT_TEXT "О сопернике:\n\nВ качестве противника выступает компьютерный интеллект на основе расчёта вероятности. Соперник сделан не \"всесильным прорицателем\", а более человечным. Он будет продолжать игру и тогда, когда вероятность победить не слишком высока. Но и сбросить карты он тоже может. Особенностью Хедз-ап покера является то, что можно победить с более слабой комбинацией, нежели когда в игре 6-9 соперников. Поэтому и действовать нужно более рискованно. По действиям соперника можно попытаться примерно определить, насколько он уверен в победе. Но всё станет предельно ясно только в конце игры.\n\nВернуться в меню справки: /help\nВернуться к игре: /exit_help"
#define AUTHOR_TEXT "О проекте:\n\nДанный бот создан в качестве учебного проекта производственной практики. Автор: Гулин Алексей, группа ПИ-92, АлтГТУ им. И.И. Ползунова. 2022 г.\n\nВернуться в меню справки: /help\nВернуться к игре: /exit_help"

using namespace std;

fstream outlog(PATH_OF_LOG, ios::in|ios::app); //поток вывода лога работы в конец файла лог

string current_time() //получить строку с текущим временем (для лога)
{
    auto time_now = chrono::system_clock::now();
    time_t time_t_now = chrono::system_clock::to_time_t(time_now);
    string string_time = ctime(&time_t_now);
    return (string_time).erase(string_time.length()-1,1) + string(": ");
}

int main()
{
    string bot_token; //токен бота 
    try
    {
        bot_token = FileTools::read(PATH_OF_TOKEN); //прочитать из файла
    }
    catch (ifstream::failure& e)
    {
        cout << current_time() << "Отсутствует файл \"data/token.dat\" с токеном бота." << endl;
        outlog << current_time() << "Отсутствует файл \"data/token.dat\" с токеном бота." << endl;
        return 0;
    }

    system(SYSTEM_CREATE_FOLDER); //создать директорию для сохранения профилей пользователей
    TgBot::Bot bot(bot_token); //объявление объекта телеграм-бота
    Game_info current_game_info; //объект с информацией об игре текущего пользователя

    bot.getEvents().onCommand("start", [&bot, current_game_info](TgBot::Message::Ptr message) mutable { //при получении команды start
        bot.getApi().sendMessage(message->chat->id, START_TEXT + string(MAIN_MENU_MSG));
        current_game_info.init(message->chat->id, MODE_NEW_PROFILE, &bot, message); //инициализация профиля текущей игры и его создание, запись в файл
        });
    bot.getEvents().onCommand("new_game", [&bot, current_game_info](TgBot::Message::Ptr message) mutable { //при получении команды new_game
        current_game_info.init(message->chat->id, MODE_EXISTING_PROFILE, &bot, message); //инициализация профиля текущей игры из файла
        current_game_info.start_new_game(&bot, message); //начать новую игру
        });
    bot.getEvents().onCommand("make_blind", [&bot, current_game_info](TgBot::Message::Ptr message) mutable { //при получении команды make_blind
        current_game_info.init(message->chat->id, MODE_EXISTING_PROFILE, &bot, message); //инициализация профиля текущей игры из файла
        current_game_info.make_blind(&bot, message); //сделать блайнд
        });
    bot.getEvents().onCommand("exit", [&bot, current_game_info](TgBot::Message::Ptr message) mutable { //при получении команды exit
        current_game_info.init(message->chat->id, MODE_EXISTING_PROFILE, &bot, message); //инициализация профиля текущей игры из файла
        current_game_info.exit(&bot, message, true); //выйти из игры
        });
    bot.getEvents().onCommand("call", [&bot, current_game_info](TgBot::Message::Ptr message) mutable { //при получении команды call
        current_game_info.init(message->chat->id, MODE_EXISTING_PROFILE, &bot, message); //инициализация профиля текущей игры из файла
        current_game_info.action_of_player(CALL, 0, &bot, message); //выполнение действия уравнивания ставки
        });
    bot.getEvents().onCommand("check", [&bot, current_game_info](TgBot::Message::Ptr message) mutable { //при получении команды check
        current_game_info.init(message->chat->id, MODE_EXISTING_PROFILE, &bot, message); //инициализация профиля текущей игры из файла
        current_game_info.action_of_player(CHECK, 0, &bot, message); //выполнение действия передачи хода
        });
    bot.getEvents().onCommand("fold", [&bot, current_game_info](TgBot::Message::Ptr message) mutable { //при получении команды fold
        current_game_info.init(message->chat->id, MODE_EXISTING_PROFILE, &bot, message); //инициализация профиля текущей игры из файла
        current_game_info.action_of_player(FOLD, 0, &bot, message); //выполнение действия сброса карт
        });
    bot.getEvents().onCommand("raise", [&bot](TgBot::Message::Ptr message) { //при получении команды raise
        bot.getApi().sendMessage(message->chat->id, "Для повышения ставки просто напишите число, до которого хотите её повысить. Справка: /help");
        });
    bot.getEvents().onCommand("help", [&bot](TgBot::Message::Ptr message) { //при получении команды help
        bot.getApi().sendMessage(message->chat->id, HELP_TEXT);
        });
    bot.getEvents().onCommand("terms", [&bot](TgBot::Message::Ptr message) { //при получении команды terms
        bot.getApi().sendMessage(message->chat->id, TERMS_TEXT);
        }); 
    bot.getEvents().onCommand("rules", [&bot](TgBot::Message::Ptr message) { //при получении команды rules
        bot.getApi().sendMessage(message->chat->id, RULES_TEXT);
        });
    bot.getEvents().onCommand("combinations", [&bot](TgBot::Message::Ptr message) { //при получении команды combinations
        bot.getApi().sendMessage(message->chat->id, COMBINATIONS_TEXT);
        });
    bot.getEvents().onCommand("about_opponent", [&bot](TgBot::Message::Ptr message) { //при получении команды about_opponent
        bot.getApi().sendMessage(message->chat->id, ABOUT_OPPONENT_TEXT);
        });
    bot.getEvents().onCommand("author", [&bot](TgBot::Message::Ptr message) { //при получении команды author
        bot.getApi().sendMessage(message->chat->id, AUTHOR_TEXT);
        });
    bot.getEvents().onCommand("commands", [&bot, current_game_info](TgBot::Message::Ptr message) mutable { //при получении команды commands
        current_game_info.init(message->chat->id, MODE_EXISTING_PROFILE, &bot, message); //инициализация профиля текущей игры из файла
        current_game_info.send_actual_commands(true, &bot, message); //вывести доступные сейчас команды
        });
    bot.getEvents().onCommand("exit_help", [&bot, current_game_info](TgBot::Message::Ptr message) mutable { //при получении команды exit_help
        current_game_info.init(message->chat->id, MODE_EXISTING_PROFILE, &bot, message); //инициализация профиля текущей игры из файла
        current_game_info.send_actual_commands(true, &bot, message); //вывести доступные сейчас команды
        });
    bot.getEvents().onCommand("statistics", [&bot, current_game_info](TgBot::Message::Ptr message) mutable { //при получении команды statistics
        current_game_info.init(message->chat->id, MODE_EXISTING_PROFILE, &bot, message); //инициализация профиля текущей игры из файла
        current_game_info.statistics(&bot, message); //вывести статистику выигрышей и проигрышей
        });
    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) { //при получении любого сообщения
        ifstream::streampos filesize = outlog.tellg(); //узнать позицию вывода в файл
        if (filesize > LOG_MAX_SIZE) //очистить файл лога, если он превысил предел размера, и снова открыть для записи
        {
            outlog.close();
            outlog.open(PATH_OF_LOG, ios::out);
            outlog.close();
            outlog.open(PATH_OF_LOG, ios::in | ios::app); 
        }
        outlog << current_time() << "В чате " << message->chat->id << " пользователь написал: " << message->text.c_str() << endl;
        });
    bot.getEvents().onNonCommandMessage([&bot, current_game_info](TgBot::Message::Ptr message) mutable { //при получении не команды
        istringstream msg_stream(message->text.c_str()); //поток для обработки сообщения пользователя
        int arg = 0; //аргмент в сообщении пользователя
        msg_stream >> arg; //выделить аргумент из сообщения
        if (arg > 0)
        {
                current_game_info.init(message->chat->id, MODE_EXISTING_PROFILE, &bot, message); //инициализация профиля текущей игры из файла
                current_game_info.action_of_player(RAISE, arg, &bot, message); //выполнение действия повышения ставки
        }
        else
        {
            bot.getApi().sendMessage(message->chat->id, "Данное сообщение не является командой. Проверьте правильность ввода. Если вы хотели поднять ставку, напишите только число. Справка: /help");
        }
        });
    bot.getEvents().onUnknownCommand([&bot](TgBot::Message::Ptr message) { //при получении неизвестной команды
        bot.getApi().sendMessage(message->chat->id, "Данное сообщение является неизвестной командой. Проверьте правильность ввода. Справка: /help");
        });
    start_bot: //метка начала работы бота
    try 
    {
        cout << current_time() << "Бот запущен. Имя бота: " << bot.getApi().getMe()->username.c_str() << endl;
        outlog << current_time() << "Бот запущен. Имя бота: " << bot.getApi().getMe()->username.c_str() << endl;

        TgBot::TgLongPoll longPoll(bot);
        while (true) //цикл длинных опросов для обработки сообщений пользователей
        {
            longPoll.start();
        }
    }
    catch (TgBot::TgException& e) //при возникновении ошибки
    {
        cout << current_time() << "Бот остановлен. Ошибка: " << e.what() << endl;
        outlog << current_time() << "Бот остановлен. Ошибка: "<< e.what() << endl;
        goto start_bot; //попытаться начать работу заново, чтобы не пришлось вручную перезапускать бота
    }
    return 0;
}