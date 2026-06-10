# ft_irc

Setup channels.
Some client command implemantation.
(maybe info messages for clients)

[x] mensagem de instrucoes (PASS, USER e NICK) quando o cliente se conectar

@QUESTION: a lista de membros de um canal pode ser por conexao no servidor ou deve ser por insercao no canal ?
@ANSWER: RFC 1459: nao define nenhuma ordem especifica para o RPL_NAMREPLY. O servidor pode listar em qualquer ordem,
mas "operadores" antes de "membros comuns" pode ser bom
[x] listar operadores primeiro e depois membros comuns em qualquer ordem

@BUG: iniciando o servidor com senha vazia assim ( ./ircserv 6667 "" ) o parser permite,
mas os clientes nao conseguem se conectar pois o sistema responde com senha incorreta para isso ( pass "" ) 
e falta de argumentos para isso ( pass ).
@SOLUTION: fiz com que nao seja possivel iniciar o servidor com senha vazia ( ./ircserv 6667 "" )
[x] fazer com que o seja possivel iniciar o servidor sem senha 
@INFO: num servidor sem senha, agora e possivel se conectar sem passar 'pass', passando 'pass' vazia ou 
passando qualquer valor em 'pass' 

@QUESTION: e possivel criar canal ja com topicou ou preciso cria-lo depois usar TOPIC para definir?
@ANSWER: precisa criar e depois setar com TOPIC 
[ ] criar comando TOPIC 

[ ] criar comando KICK
[ ] criar comando INVITE
[ ] criar comando MODE

@INFO: comando OPER promove um cliente a IRC operator, um admin global do servidor 
diferente de operador de canal, e isso o projeto nao exige

@INFO: o canal deve ter o nome passado pelo usuario e nao deve existir nome igual
@SOLUTION: ja esta, o nome real e salvo em '_name' e o nome para comparacao fica em '_nameLower' e 
sempre faco 'toLower' para operacoes como (JOIN, PART, PRIVMSG...), coisa que acontece em outros projetos
[x] tratar sobre os nomes de canais #geral #GERAL, salvar como passado e comparar literal

[x] testar leaks com QUIT
@INFO: quando todos os clientes usando QUIT antes de eu fechar o servidor nao da leaks

[x] criar handle_signal parar fechar o programa em caso de CTRL+C
[ ] liberar fds e poll no encerramento do programa 

[ ] para o exemplo ( PRIVMSG #geral,#hum :ola boa noite ) precisamos dividir o "#geral,#hum" em strings 
para que a mensagem seja enviada a todos os usuarios ou grupos listados 

[ ] verificar se essa mensagem esta correta:
-----------------------------------------------------------------------
:irc.local 001 mario :Welcome to the IRC network mario!mario@127.0.0.1
:irc.local 002 mario :Your host is irc.local
privmsg luiza : OILAS
:irc.local 401 mario luiza :Not such nick
-----------------------------------------------------------------------

-- NAO OBRIGATORIO -- 
[ ] criar set para clientes únicos que precisam ser avisados quando um usuario usar QUIT
para que a mensagem nao se repita para todos os grupos partilhados por 2 usuarios

-- relatorio valgrind -- 
luiz@DESKTOPLUIZ:~/irc/ft_irc/ft_irc$ valgrind --track-fds=yes ./ircserv 6667 ""
==100904== Memcheck, a memory error detector
==100904== Copyright (C) 2002-2022, and GNU GPL'd, by Julian Seward et al.
==100904== Using Valgrind-3.22.0 and LibVEX; rerun with -h for copyright info
==100904== Command: ./ircserv 6667
==100904==
Server listening on port 6667
[+] Client connected: fd=4 ip=127.0.0.1
[+] Client connected: fd=5 ip=127.0.0.1
[+] Client connected: fd=6 ip=127.0.0.1
^C
==100904==
==100904== FILE DESCRIPTORS: 6 open (3 std) at exit.
==100904== Open AF_INET socket 6: 127.0.0.1:6667 <-> 127.0.0.1:36298
==100904==    at 0x4C39834: accept (accept.c:26)
==100904==    by 0x10D18E: Server::_acceptClient() (in /home/luiz/irc/ft_irc/ft_irc/ircserv)
==100904==    by 0x10CF75: Server::run() (in /home/luiz/irc/ft_irc/ft_irc/ircserv)
==100904==    by 0x10BB9D: main (in /home/luiz/irc/ft_irc/ft_irc/ircserv)
==100904==
==100904== Open AF_INET socket 5: 127.0.0.1:6667 <-> 127.0.0.1:39558
==100904==    at 0x4C39834: accept (accept.c:26)
==100904==    by 0x10D18E: Server::_acceptClient() (in /home/luiz/irc/ft_irc/ft_irc/ircserv)
==100904==    by 0x10CF75: Server::run() (in /home/luiz/irc/ft_irc/ft_irc/ircserv)
==100904==    by 0x10BB9D: main (in /home/luiz/irc/ft_irc/ft_irc/ircserv)
==100904==
==100904== Open AF_INET socket 4: 127.0.0.1:6667 <-> 127.0.0.1:49186
==100904==    at 0x4C39834: accept (accept.c:26)
==100904==    by 0x10D18E: Server::_acceptClient() (in /home/luiz/irc/ft_irc/ft_irc/ircserv)
==100904==    by 0x10CF75: Server::run() (in /home/luiz/irc/ft_irc/ft_irc/ircserv)
==100904==    by 0x10BB9D: main (in /home/luiz/irc/ft_irc/ft_irc/ircserv)
==100904==
==100904==
==100904== HEAP SUMMARY:
==100904==     in use at exit: 0 bytes in 0 blocks
==100904==   total heap usage: 222 allocs, 222 frees, 88,018 bytes allocated
==100904==
==100904== All heap blocks were freed -- no leaks are possible
==100904==
==100904== For lists of detected and suppressed errors, rerun with: -s
==100904== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
