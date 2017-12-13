# SirLogsalot
A very simple IRC bot written in C. Its primary purpose is logging what people say on channels.

# Compiling
There are multiple ways to compile SirLogsalot.

1. Running `make` or `make compile` will compile the code and leave an executable called _sirlogsalot_.
2. Running `make run` will compile the code and run the executable in one step.
3. Running `make test` will compile the code and perform memory tests using _valgrind_.
4. Running `make clean` will remove the executable and only leave the code files.

# How to Use
The program logs the messages, joins and leaves to filenames in the format `nick.log.txt` or `#channel.log.txt`. It adds the current date and time to every line. It allows configuration with the config.txt file.

# The Config File
The program can be configured by editing the config.txt file. The file is in the `key = value` format. The settings are:

1. **server**: Used to set the server ip to connect to. Example: server = 195.154.200.232
2. **port**: Used to set the port of the server. The default irc port is 6667. Example: port = 6667
3. **nick**: Used to set the nick of the bot. Example: nick = SirLogsalot
4. **channels**: Used to set the channel list for the bot, comma seperated. Example: channels = #WatchPeopleCode,#archlinux
