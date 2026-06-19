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

[x] criar comando KICK
[x] criar comando INVITE
[x] criar comando MODE

@INFO: comando OPER promove um cliente a IRC operator, um admin global do servidor 
diferente de operador de canal, e isso o projeto nao exige

@INFO: o canal deve ter o nome passado pelo usuario e nao deve existir nome igual
@SOLUTION: ja esta, o nome real e salvo em '_name' e o nome para comparacao fica em '_nameLower' e 
sempre faco 'toLower' para operacoes como (JOIN, PART, PRIVMSG...), coisa que acontece em outros projetos
[x] tratar sobre os nomes de canais #geral #GERAL, salvar como passado e comparar literal

[x] testar leaks com QUIT
@INFO: quando todos os clientes usando QUIT antes de eu fechar o servidor nao da leaks

[x] criar handle_signal parar fechar o programa em caso de CTRL+C
[x] liberar fds e poll no encerramento do programa 

[x] para o exemplo ( PRIVMSG #geral,#hum :ola boa noite ) precisamos dividir o "#geral,#hum" em strings 
para que a mensagem seja enviada a todos os usuarios ou grupos listados 

[x] verificar se essa mensagem esta correta:
-----------------------------------------------------------------------
:irc.local 001 mario :Welcome to the IRC network mario!mario@127.0.0.1
:irc.local 002 mario :Your host is irc.local
privmsg luiza : OILAS
:irc.local 401 mario luiza :Not such nick
-----------------------------------------------------------------------

-- NAO OBRIGATORIO -- 
[ ] criar set para clientes únicos que precisam ser avisados quando um usuario usar QUIT
para que a mensagem nao se repita para todos os grupos partilhados por 2 usuarios

-- RELATORIO VALGRIND -- 
luiz-dos@NOTEBOOK-KETLYN:~/ft_irc/ft_irc$ valgrind --track-fds=yes ./ircserv 6667 ""
==9723== Memcheck, a memory error detector
==9723== Copyright (C) 2002-2024, and GNU GPL'd, by Julian Seward et al.
==9723== Using Valgrind-3.24.0 and LibVEX; rerun with -h for copyright info
==9723== Command: ./ircserv 6667
==9723==
Server listening on port 6667
[+] Client connected: fd=4 ip=127.0.0.1
[+] Client connected: fd=5 ip=127.0.0.1
^C
==9723==
==9723== FILE DESCRIPTORS: 3 open (3 std) at exit.
==9723==
==9723== HEAP SUMMARY:
==9723==     in use at exit: 0 bytes in 0 blocks
==9723==   total heap usage: 180 allocs, 180 frees, 84,851 bytes allocated
==9723==
==9723== All heap blocks were freed -- no leaks are possible
==9723==
==9723== For lists of detected and suppressed errors, rerun with: -s
==9723== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)


Dei fix nos erros de valgrind que estavam ser causados por um pointer que apontava para o client quando este saia do programa com ctrl-c, ele ficava na lista da channel ainda e se desse PRIVMSG tentava acessar um pointer que ja nao devia existir e dava erro.

Alterei PART, JOIN E PRIVMSG para aceita varias açoes ao mesmo tempo do tipo JOIN #chan1,#chan2#,chan3 mesmo com o part e PRIVMSG #chan1,#chan2,nick1,nick :hello cuidado que tem que ser sem espacos

Fiz uma alteraçao no server.cpp linha 349 para usar toLower quando compara nicks, assim BOB e bob vai ser o mesmo nick, e assim que o irc normal faz, mesmo com as channels.

Fiz tambem to comando MODE, testei bastante mas se encontrarem algum erro digam.
