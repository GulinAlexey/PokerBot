# PokerBot
Текстовый Telegram-бот для игры в покер. Протестирован на Ubuntu 18.04 LTS. 

Использована библиотека tgbot-cpp: https://github.com/reo7sp/tgbot-cpp

Библиотеки tgbot-cpp и Boost подключаются соответственно их руководствам. 

Для запуска программы после её компиляции использовать <code>./pokerbot_app &</code> при нахождении в директории программы. После этого программа будет работать в фоне и наиболее важные сообщения выводить в консоль (например, ошибки). Более подробная информация о ходе работы выводится в файл log.txt, для просмотра его изменений в реальном времени используйте <code>tail -f log.txt</code>, а для завершения работы бота — <code>killall pokerbot_app</code>.
