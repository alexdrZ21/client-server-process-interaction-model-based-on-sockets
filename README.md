# client-server-process-interaction-model-based-on-sockets

Процесс сервер:

Процесс клиент:

Команды:
\users - получить от сервера список всех пользователей (имена), которые сейчас онлайн;

\quit <message> – выход из чата с прощальным сообщением;
  
\private <nickname> <message> - приватное сообщение пользователю с именем <nickname>, 
если пользователя-адресата на сервере нет, то выдается сообщение об ошибке;
  
\privates – получить от сервера имена пользователей которым вы отправляли приватные 
сообщения;
  
\help – вывод допустимых команд.
