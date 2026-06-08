# ft_irc

Setup channels.
Some client command implemantation.
(maybe info messages for clients)

[x] mensagem de instrucoes (PASS, USER e NICK) quando o cliente se conectar

[ ] para o exemplo ( PRIVMSG #geral,#hum :ola boa noite ) precisamos dividir o "#geral,#hum" em strings para que
a mensagem seja enviada a todos os usuarios ou grupos listados

/* @QUESTION: a lista de membros de um canal pode ser por conexao no servidor ou deve ser por insercao no canal ?
	@SOLUTION: devo exibir o proprio nome e depois ou membros por ordem alfabetica */

@BUG: iniciando o servidor com senha vazia assim ( ./ircserv 6667 "" ) o parser permite,
mas os clientes nao conseguem se conectar pois o sistema responde com senha incorreta para isso ( pass "" ) 
e falta de argumentos para isso ( pass ).
@SOLUTION: fiz com que nao seja possivel iniciar o servidor com senha vazia ( ./ircserv 6667 "" )
[ ] fazer com que o seja possivel iniciar o servidor sem senha 

[ ] tratar sobre os nomes de canais #geral #GERAL, salvar como passado e comparar literal

[ ] criar set para clientes únicos que precisam ser avisados quando um usuario usar QUIT
para que a mensagem nao se repita para todos os grupos partilhados por 2 usuarios

@QUESTION: e possivel criar canal ja com topicou ou preciso cria-lo depois usar TOPIC para definir?
@INFO: comando OPER usado para promover clientes 
@INFO: o canal deve ter o nome passado pelo usuario e nao deve existir nome igual

@INFO: Atualmente apenas um leak de poll 
[ ] testar leaks com QUIT

Server listening on port 6667
[+] Client connected: fd=4 ip=127.0.0.1
[+] Client connected: fd=5 ip=127.0.0.1
[-] Client disconnected: fd=4
[-] Client disconnected: fd=5
^C==28183==
==28183== Process terminating with default action of signal 2 (SIGINT)
==28183==    at 0x4B80687: __internal_syscall_cancel (cancellation.c:64)
==28183==    by 0x4B806AC: __syscall_cancel (cancellation.c:75)
==28183==    by 0x4BF4A05: poll (poll.c:29)
==28183==    by 0x10C724: Server::run() (in /home/luiz-dos/ft_irc/ft_irc/ircserv)
==28183==    by 0x10B739: main (in /home/luiz-dos/ft_irc/ft_irc/ircserv)
==28183==
==28183== HEAP SUMMARY:
==28183==     in use at exit: 76,016 bytes in 17 blocks
==28183==   total heap usage: 254 allocs, 237 frees, 88,579 bytes allocated
==28183==
==28183== LEAK SUMMARY:
==28183==    definitely lost: 0 bytes in 0 blocks
==28183==    indirectly lost: 0 bytes in 0 blocks
==28183==      possibly lost: 0 bytes in 0 blocks
==28183==    still reachable: 76,016 bytes in 17 blocks
==28183==         suppressed: 0 bytes in 0 blocks
==28183== Rerun with --leak-check=full to see details of leaked memory
==28183==
==28183== For lists of detected and suppressed errors, rerun with: -s
==28183== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
