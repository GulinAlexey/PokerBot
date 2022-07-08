#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>   
#include <tgbot/tgbot.h>

#define PATH_OF_TOKEN "data/token.dat" //путь к файлу, в котором хранится токен телеграм-бота
#define PATH_OF_LOG "log.txt" //путь к файлу для записи лога

using namespace std;

ofstream outlog(PATH_OF_LOG); //поток вывода лога работы в файл

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

    TgBot::Bot bot(bot_token); //объявление объекта телеграм-бота

    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) //при получении команды start
        {
        bot.getApi().sendMessage(message->chat->id, "Добро пожаловать! Это бот для игры в покер.");
        });
    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) //при получении любого сообщения
        {
        outlog << current_time() << "Пользователь написал " << message->text.c_str() << endl;
        if (StringTools::startsWith(message->text, "/start")) 
        {
            return;
        }
        bot.getApi().sendMessage(message->chat->id, "Ваше сообщение: " + message->text);
        });
    try 
    {
        cout << current_time() << "Имя бота: " << bot.getApi().getMe()->username.c_str() << endl;
        cout << current_time() << "Бот запущен" << endl;

        outlog << current_time() << "Имя бота: " << bot.getApi().getMe()->username.c_str() << endl;
        outlog << current_time() << "Бот запущен" << endl;

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