#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <tgbot/tgbot.h>

#define PATH_OF_TOKEN "data/token.dat" //путь к файлу, в котором хранится токен телеграм-бота
#define PATH_OF_LOG "log.txt" //путь к файлу для записи лога

using namespace std;

ofstream outlog(PATH_OF_LOG); //поток вывода лога работы в файл

int main()
{
    string bot_token; //токен бота
    try
    {
        bot_token = FileTools::read(PATH_OF_TOKEN); //прочитать из файла
    }
    catch (ifstream::failure& e)
    {
        cout << "Отсутствует файл \"data/token.dat\" с токеном бота." << endl;
        outlog << "Отсутствует файл \"data/token.dat\" с токеном бота." << endl;
        return 0;
    }

    TgBot::Bot bot(bot_token); //объявление объекта телеграм-бота

    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) //при получении команды start
        {
        bot.getApi().sendMessage(message->chat->id, "Hi!");
        });
    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) //при получении любого сообщения
        {
        outlog << "Пользователь написал " << message->text.c_str() << endl;
        if (StringTools::startsWith(message->text, "/start")) 
        {
            return;
        }
        bot.getApi().sendMessage(message->chat->id, "Ваше сообщение: " + message->text);
        });
    try 
    {
        cout << "Имя бота: " << bot.getApi().getMe()->username.c_str() << endl;
        cout << "Бот запущен" << endl;
        outlog << "Имя бота: " << bot.getApi().getMe()->username.c_str() << endl;
        outlog << "Бот запущен" << endl;

        TgBot::TgLongPoll longPoll(bot);
        while (true) //цикл длинных опросов для обработки сообщений пользователей
        {
            outlog << "Длинный опрос запущен" << endl;
            longPoll.start();
        }
    }
    catch (TgBot::TgException& e) //при возникновении ошибки
    {
        cout << "Бот остановлен. Ошибка: " << e.what() << endl;
        outlog << "Бот остановлен. Ошибка: "<< e.what() << endl;
    }
    return 0;
}