# client-server-process-interaction-model-based-on-sockets

Процесс сервер:
1)На вход серверу подаётся 1 аргумент - номер порта
2)Создание потокового сокета AF_INET для приема входящих подключений
3)Биндим сокет(биндим определенный порт)
4)Ожидание запросов связи на данном сокете
5)Настройка прослушивающего сокета
6)Ожидание входящих подключений или входящих данных 
 
Процесс клиент:
1)На вход клиенту подаются 2 параметра: 1 - номер порта; 2 - никнейм клиента
2)Создание потокового сокета AF_INET для приема входящих подключений
3)Инициализация соединения на сокете
4)Отправка ника на сервер
5)Пересылка сообщений или выход

Команды:
\users - получить от сервера список всех пользователей (имена), которые сейчас онлайн;

\quit <message> – выход из чата с прощальным сообщением;
  
\private <nickname> <message> - приватное сообщение пользователю с именем <nickname>, 
если пользователя-адресата на сервере нет, то выдается сообщение об ошибке;
  
\privates – получить от сервера имена пользователей которым вы отправляли приватные 
сообщения;
  
\help – вывод допустимых команд.
