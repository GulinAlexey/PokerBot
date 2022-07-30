# PokerBot
Текстовый Telegram-бот для игры в покер. Протестирован на Ubuntu 18.04 LTS. 

Тип игры - Техасский Холдем хедз-ап (один на один) с колодой из 52 карт.

Использована библиотека tgbot-cpp: https://github.com/reo7sp/tgbot-cpp

Библиотеки tgbot-cpp и Boost подключаются соответственно их руководствам. 

Токен бота следует записать в файл "data/token.dat". Для запуска программы после её компиляции последовательно использовать команды <code>screen</code> (для фоновой работы) и <code>./pokerbot_app &</code> при нахождении в директории программы. После этого программа будет работать в фоне и наиболее важные сообщения выводить в консоль (например, ошибки). Более подробная информация о ходе работы выводится в файл log.txt, для просмотра его изменений в реальном времени используйте <code>tail -f log.txt</code>, а для завершения работы бота — <code>killall pokerbot_app</code>. Команда возврата к текущей сессии после повторного подключения к серверу — <code>screen -r</code>.
