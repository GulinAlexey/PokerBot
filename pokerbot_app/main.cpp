#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <ctime>   
#include <tgbot/tgbot.h>
#include "Game_info.h"

#include "Game_info.cpp"
#include "Playing_card.cpp"

#define SYSTEM_CREATE_FOLDER "mkdir -p " //системная команда для создания директории

#define SYSTEM_FUNC(command, arg) command arg //объединение define строк для вызова системной команды с аргументами

#define PATH_OF_TOKEN "data/token.dat" //путь к файлу, в котором хранится токен телеграм-бота
#define PATH_OF_LOG "log.txt" //путь к файлу для записи в лог

#define HELP_TEXT "Чтобы начать новую игру, отправьте /new_game\n\nТермины:\nБанк — сумма всех ставок игроков.\nСтек — кол-во фишек на руках у игрока, которые он может поставить.\nБлайнд — обязательная ставка в начале раунда."

using namespace std;

ofstream outlog(PATH_OF_LOG, ios::app); //поток вывода лога работы в конец файла лог

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

    system(SYSTEM_FUNC(SYSTEM_CREATE_FOLDER, FOLDER)); //создать директорию для сохранения профилей пользователей
    TgBot::Bot bot(bot_token); //объявление объекта телеграм-бота
    Game_info current_game_info; //объект с информацией об игре текущего пользователя

    bot.getEvents().onCommand("start", [&bot, current_game_info](TgBot::Message::Ptr message) mutable { //при получении команды start
        bot.getApi().sendMessage(message->chat->id, "Добро пожаловать! Это бот для игры в покер (один на один, без джокеров).\n\nНачать новую игру: /new_game\nСправка: /help");
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
        current_game_info.exit(&bot, message); //выйти из игры
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
    bot.getEvents().onCommand("raise", [&bot, current_game_info](TgBot::Message::Ptr message) mutable { //при получении команды 
        bot.getApi().sendMessage(message->chat->id, "Команда raise употребляется без символа /");
        });
    bot.getEvents().onCommand("help", [&bot, current_game_info](TgBot::Message::Ptr message) mutable { //при получении команды help
        bot.getApi().sendMessage(message->chat->id, HELP_TEXT);
        });
    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) { //при получении любого сообщения
        outlog << current_time() << "В чате " << message->chat->id << " пользователь написал: " << message->text.c_str() << endl;
        });
    bot.getEvents().onNonCommandMessage([&bot, current_game_info](TgBot::Message::Ptr message) mutable { //при получении не команды
        istringstream msg_stream(message->text.c_str()); //поток для обработки сообщения пользователя
        string command = ""; //команда в сообщении пользователя
        int arg = 0; //аргмент в сообщении пользователя
        msg_stream >> command; //выделить команду из сообщения
        msg_stream >> arg; //выделить аргумент из сообщения
        if (command == "raise")
        {
            if (arg > 0)
            {
                current_game_info.init(message->chat->id, MODE_EXISTING_PROFILE, &bot, message); //инициализация профиля текущей игры из файла
                current_game_info.action_of_player(RAISE, arg, &bot, message); //выполнение действия повышения ставки
            }
            else
            {
                bot.getApi().sendMessage(message->chat->id, "Проверьте правильность ввода. После команды должно быть введено положительное число");
            }
        }
        else
        {
            bot.getApi().sendMessage(message->chat->id, "Данное сообщение не является командой. Проверьте правильность ввода. Справка: /help");
        }
        });
    bot.getEvents().onUnknownCommand([&bot](TgBot::Message::Ptr message) { //при получении неизвестной команды
        bot.getApi().sendMessage(message->chat->id, "Данное сообщение является неизвестной командой. Проверьте правильность ввода. Справка: /help");
        });
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
    }
    return 0;
}