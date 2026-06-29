#!/bin/bash

SERVER="localhost"
PORT="6667"
PASS="12"

passed=0
failed=0

run_test()
{
    local name="$1"
    local commands="$2"
    local expected="$3"

    response=$(echo -e "$commands" | sed 's/$/\r/' | nc -q 2 $SERVER $PORT 2>/dev/null)

    if echo "$response" | grep -q "$expected"; then
        echo "✅  $name"
        passed=$((passed + 1))
    else
        echo "❌  $name"
        echo "    esperado : $expected"
        echo "    recebido : $(echo "$response" | tr '\r' ' ' | grep -v "^$" | tail -5)"
        failed=$((failed + 1))
    fi
}

run_test_multi()
{
    local name="$1"
    local file_to_check="$2"
    local expected="$3"

    if grep -q "$expected" "$file_to_check" 2>/dev/null; then
        echo "✅  $name"
        passed=$((passed + 1))
    else
        echo "❌  $name"
        echo "    esperado : $expected"
        echo "    recebido : $(cat $file_to_check | tr '\r' ' ' | grep -v "^$" | tail -5)"
        failed=$((failed + 1))
    fi
}

echo "================================================"
echo " ft_irc — suite de testes"
echo "================================================"
echo ""

# ── 1. autenticação ───────────────────────────────────────────────────────────
echo "[ autenticação ]"

run_test "1.  PASS correta recebe 001" \
"PASS $PASS\nNICK u1\nUSER u1 0 * :U One\nQUIT :bye" \
"001"

run_test "2.  PASS errada recebe 464" \
"PASS senhaerrada\nNICK u1\nUSER u1 0 * :U One" \
"464"

# cliente 1 fica online para teste de nick duplicado
{
    echo -e "PASS $PASS\r\nNICK dupl\r\nUSER dupl 0 * :Dupl\r\n"
    sleep 2
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/c1_nick.txt 2>/dev/null &
C1_PID=$!
sleep 0.4

run_test "3.  NICK duplicado recebe 433" \
"PASS $PASS\nNICK dupl\nUSER dupl2 0 * :Dupl2\nQUIT :bye" \
"433"

wait $C1_PID 2>/dev/null

run_test "4.  Comando antes do registro recebe 451" \
"PASS $PASS\nJOIN #geral" \
"451"

# ── 2. canais básicos ─────────────────────────────────────────────────────────
echo ""
echo "[ canais básicos ]"

run_test "5.  JOIN canal válido recebe 353" \
"PASS $PASS\nNICK u1\nUSER u1 0 * :U One\nJOIN #geral\nQUIT :bye" \
"353"

run_test "6.  JOIN canal inválido recebe 403" \
"PASS $PASS\nNICK u1\nUSER u1 0 * :U One\nJOIN semhash\nQUIT :bye" \
"403"

run_test "7.  PART canal inexistente recebe 403" \
"PASS $PASS\nNICK u1\nUSER u1 0 * :U One\nPART #inexistente\nQUIT :bye" \
"403"

# cliente que segura o canal
{
    echo -e "PASS $PASS\r\nNICK holder\r\nUSER holder 0 * :Holder\r\n"
    sleep 0.5
    echo -e "JOIN #testpart\r\n"
    sleep 3
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/holder.txt 2>/dev/null &
HOLDER_PID=$!
sleep 0.5

run_test "8.  PART canal sem ser membro recebe 442" \
"PASS $PASS\nNICK u1\nUSER u1 0 * :U One\nJOIN #testpart\nPART #testpart\nPART #testpart\nQUIT :bye" \
"442"

wait $HOLDER_PID 2>/dev/null

# ── 3. modo +i invite only ────────────────────────────────────────────────────
echo ""
echo "[ modo +i — invite only ]"

{
    echo -e "PASS $PASS\r\nNICK op\r\nUSER op 0 * :Op\r\n"
    sleep 0.5
    echo -e "JOIN #privado\r\n"
    sleep 0.3
    echo -e "MODE #privado +i\r\n"
    sleep 1.5
    echo -e "INVITE convidado #privado\r\n"
    sleep 1.5
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/op_i.txt 2>/dev/null &
OP_PID=$!
sleep 0.5

{
    echo -e "PASS $PASS\r\nNICK convidado\r\nUSER conv 0 * :Conv\r\n"
    sleep 0.8
    echo -e "JOIN #privado\r\n"
    sleep 1.5
    echo -e "JOIN #privado\r\n"
    sleep 0.5
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/conv_i.txt 2>/dev/null

run_test_multi "9.  JOIN canal +i sem convite recebe 473" "/tmp/conv_i.txt" "473"
run_test_multi "10. JOIN canal +i com INVITE recebe 353" "/tmp/conv_i.txt" "353"

wait $OP_PID 2>/dev/null

# ── 4. modo +k senha ──────────────────────────────────────────────────────────
echo ""
echo "[ modo +k — senha ]"

{
    echo -e "PASS $PASS\r\nNICK op2\r\nUSER op2 0 * :Op2\r\n"
    sleep 0.5
    echo -e "JOIN #secreto\r\n"
    sleep 0.3
    echo -e "MODE #secreto +k senhadocanal\r\n"
    sleep 2
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/op_k.txt 2>/dev/null &
OP2_PID=$!
sleep 1

run_test "11. JOIN canal +k com senha errada recebe 475" \
"PASS $PASS\nNICK u2\nUSER u2 0 * :U Two\nJOIN #secreto senhaerrada\nQUIT :bye" \
"475"

run_test "12. JOIN canal +k com senha certa recebe 353" \
"PASS $PASS\nNICK u3\nUSER u3 0 * :U Three\nJOIN #secreto senhadocanal\nQUIT :bye" \
"353"

wait $OP2_PID 2>/dev/null

# ── 5. modo +l limite ─────────────────────────────────────────────────────────
echo ""
echo "[ modo +l — limite de membros ]"

{
    echo -e "PASS $PASS\r\nNICK op3\r\nUSER op3 0 * :Op3\r\n"
    sleep 0.5
    echo -e "JOIN #limitado\r\n"
    sleep 0.3
    echo -e "MODE #limitado +l 1\r\n"
    sleep 3
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/op_l.txt 2>/dev/null &
OP3_PID=$!
sleep 1

run_test "13. JOIN canal +l cheio recebe 471" \
"PASS $PASS\nNICK u4\nUSER u4 0 * :U Four\nJOIN #limitado\nQUIT :bye" \
"471"

wait $OP3_PID 2>/dev/null

# ── 6. KICK ───────────────────────────────────────────────────────────────────
echo ""
echo "[ kick ]"

{
    echo -e "PASS $PASS\r\nNICK kicker\r\nUSER kicker 0 * :Kicker\r\n"
    sleep 0.5
    echo -e "JOIN #arena\r\n"
    sleep 1.5
    echo -e "KICK #arena vitima :fora\r\n"
    sleep 0.5
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/kicker.txt 2>/dev/null &
KICK_PID=$!
sleep 0.5

{
    echo -e "PASS $PASS\r\nNICK vitima\r\nUSER vitima 0 * :Vitima\r\n"
    sleep 0.8
    echo -e "JOIN #arena\r\n"
    sleep 1.5
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/vitima.txt 2>/dev/null &
VIT_PID=$!

sleep 0.3

{
    echo -e "PASS $PASS\r\nNICK naoop\r\nUSER naoop 0 * :NaoOp\r\n"
    sleep 0.8
    echo -e "JOIN #arena\r\n"
    sleep 0.5
    echo -e "KICK #arena vitima :tentativa\r\n"
    sleep 0.5
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/naoop.txt 2>/dev/null

run_test_multi "14. KICK por operador funciona" "/tmp/vitima.txt" "KICK"
run_test_multi "15. KICK por não-operador recebe 482" "/tmp/naoop.txt" "482"

wait $KICK_PID $VIT_PID 2>/dev/null

# ── 7. KICK múltiplos ─────────────────────────────────────────────────────────
echo ""
echo "[ kick múltiplos ]"

{
    echo -e "PASS $PASS\r\nNICK boss\r\nUSER boss 0 * :Boss\r\n"
    sleep 0.5
    echo -e "JOIN #canal1\r\n"
    sleep 1.5
    echo -e "KICK #canal1 chato1,chato2 :limpar\r\n"
    sleep 0.5
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/boss.txt 2>/dev/null &
BOSS_PID=$!
sleep 0.3

{
    echo -e "PASS $PASS\r\nNICK chato1\r\nUSER chato1 0 * :Chato1\r\n"
    sleep 0.6
    echo -e "JOIN #canal1\r\n"
    sleep 2.0
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/vit1.txt 2>/dev/null &
VIT1_PID=$!

{
    echo -e "PASS $PASS\r\nNICK chato2\r\nUSER chato2 0 * :Chato2\r\n"
    sleep 0.6
    echo -e "JOIN #canal1\r\n"
    sleep 2.0
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/vit2.txt 2>/dev/null &
VIT2_PID=$!

wait $BOSS_PID $VIT1_PID $VIT2_PID 2>/dev/null

run_test_multi "16. KICK múltiplos users (user 1 de 2) foi expulso" "/tmp/vit1.txt" "KICK #canal1 chato1"
run_test_multi "17. KICK múltiplos users (user 2 de 2) foi expulso" "/tmp/vit2.txt" "KICK #canal1 chato2"

run_test "18. KICK com múltiplos canais não quebra o servidor (Gera 403)" \
"PASS $PASS\nNICK u5\nUSER u5 0 * :U Five\nKICK #canal1,#canal2 chato3\nQUIT :bye" \
"403"

# ── 8. PRIVMSG ────────────────────────────────────────────────────────────────
echo ""
echo "[ privmsg ]"

# op entra no canal e fica à espera de mensagem
{
    echo -e "PASS $PASS\r\nNICK recvr\r\nUSER recvr 0 * :Recvr\r\n"
    sleep 0.5
    echo -e "JOIN #chat\r\n"
    sleep 3
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/recvr.txt 2>/dev/null &
RECV_PID=$!
sleep 0.5

# sender entra e manda mensagem para o canal
{
    echo -e "PASS $PASS\r\nNICK sendr\r\nUSER sendr 0 * :Sendr\r\n"
    sleep 0.8
    echo -e "JOIN #chat\r\n"
    sleep 0.3
    echo -e "PRIVMSG #chat :ola canal\r\n"
    sleep 0.5
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/sendr.txt 2>/dev/null &
SEND_PID=$!
sleep 0.3

# dm: sender2 manda mensagem privada para recvr2
{
    echo -e "PASS $PASS\r\nNICK recvr2\r\nUSER recvr2 0 * :Recvr2\r\n"
    sleep 0.5
    sleep 3
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/recvr2.txt 2>/dev/null &
RECV2_PID=$!
sleep 0.3

{
    echo -e "PASS $PASS\r\nNICK sendr2\r\nUSER sendr2 0 * :Sendr2\r\n"
    sleep 0.8
    echo -e "PRIVMSG recvr2 :mensagem direta\r\n"
    sleep 0.5
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/sendr2.txt 2>/dev/null

wait $SEND_PID $RECV2_PID 2>/dev/null

run_test_multi "19. PRIVMSG canal — receptor recebe mensagem" "/tmp/recvr.txt" "ola canal"
run_test_multi "20. PRIVMSG direto — receptor recebe mensagem" "/tmp/recvr2.txt" "mensagem direta"

run_test "21. PRIVMSG sem destinatário recebe erro" \
"PASS $PASS\nNICK u6\nUSER u6 0 * :U Six\nPRIVMSG :sem destinatario\nQUIT :bye" \
"41"

run_test "22. PRIVMSG para nick inexistente recebe 401" \
"PASS $PASS\nNICK u6\nUSER u6 0 * :U Six\nPRIVMSG fantasma :oi\nQUIT :bye" \
"401"

run_test "23. PRIVMSG para canal inexistente recebe 403" \
"PASS $PASS\nNICK u6\nUSER u6 0 * :U Six\nPRIVMSG #naoeexiste :oi\nQUIT :bye" \
"403"

wait $RECV_PID $SEND_PID $RECV2_PID 2>/dev/null

# ── 9. TOPIC ──────────────────────────────────────────────────────────────────
echo ""
echo "[ topic ]"

# op entra, define tópico, membro comum tenta mudar com +t ativo
{
    echo -e "PASS $PASS\r\nNICK topop\r\nUSER topop 0 * :TopOp\r\n"
    sleep 0.5
    echo -e "JOIN #topico\r\n"
    sleep 0.5
    echo -e "TOPIC #topico :topico inicial\r\n"
    sleep 0.3
    echo -e "MODE #topico +t\r\n"
    sleep 2
    echo -e "QUIT\r\n"
} | nc -q 3 $SERVER $PORT > /tmp/topop.txt 2>/dev/null &
TOPOP_PID=$!
sleep 0.5

{
    echo -e "PASS $PASS\r\nNICK topmem\r\nUSER topmem 0 * :TopMem\r\n"
    sleep 0.8
    echo -e "JOIN #topico\r\n"
    sleep 0.3
    echo -e "TOPIC #topico :tentativa membro\r\n"
    sleep 0.5
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/topmem.txt 2>/dev/null &
TOPOP_PID=$!
sleep 0.5

wait $TOPOP_PID 2>/dev/null
wait $TOPMEM_PID 2>/dev/null

run_test_multi "24. TOPIC definido pelo operador recebe confirmação" "/tmp/topop.txt" "topico inicial"
run_test_multi "25. TOPIC por membro em canal +t recebe 482" "/tmp/topmem.txt" "482"

wait $TOPOP_PID 2>/dev/null

run_test "26. TOPIC canal inexistente recebe 403" \
"PASS $PASS\nNICK u7\nUSER u7 0 * :U Seven\nTOPIC #naoeexiste :topico\nQUIT :bye" \
"403"

# ── 10. MODE consulta e múltiplas flags ───────────────────────────────────────
echo ""
echo "[ mode ]"

run_test "27. MODE canal inexistente recebe 403" \
"PASS $PASS\nNICK u8\nUSER u8 0 * :U Eight\nMODE #naoeexiste +i\nQUIT :bye" \
"403"

run_test "28. MODE por não-operador recebe 482" \
"PASS $PASS\nNICK u8\nUSER u8 0 * :U Eight\nJOIN #modetest\nPART #modetest\nQUIT :bye" \
"PART"

# op entra, activa múltiplas flags de uma vez
{
    echo -e "PASS $PASS\r\nNICK modeop\r\nUSER modeop 0 * :ModeOp\r\n"
    sleep 0.5
    echo -e "JOIN #modetest\r\n"
    sleep 0.3
    echo -e "MODE #modetest +itk segredo\r\n"
    sleep 0.5
    echo -e "MODE #modetest\r\n"
    sleep 0.5
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/modeop.txt 2>/dev/null

run_test_multi "29. MODE +itk combinado é aceite" "/tmp/modeop.txt" "+itk"

# membro comum tenta alterar modo
{
    echo -e "PASS $PASS\r\nNICK modeop2\r\nUSER modeop2 0 * :ModeOp2\r\n"
    sleep 0.5
    echo -e "JOIN #modetest2\r\n"
    sleep 2
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/modeop2.txt 2>/dev/null &
MODEOP2_PID=$!
sleep 0.4

{
    echo -e "PASS $PASS\r\nNICK modemem\r\nUSER modemem 0 * :ModeMem\r\n"
    sleep 0.8
    echo -e "JOIN #modetest2\r\n"
    sleep 0.3
    echo -e "MODE #modetest2 +i\r\n"
    sleep 0.5
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/modemem.txt 2>/dev/null

run_test_multi "30. MODE por membro comum recebe 482" "/tmp/modemem.txt" "482"

wait $MODEOP2_PID 2>/dev/null

# ── 11. NICK ──────────────────────────────────────────────────────────────────
echo ""
echo "[ nick ]"

run_test "31. NICK vazio recebe 431" \
"PASS $PASS\nNICK u9\nUSER u9 0 * :U Nine\nNICK\nQUIT :bye" \
"431"

run_test "32. NICK com caracter inválido recebe 432" \
"PASS $PASS\nNICK u9\nUSER u9 0 * :U Nine\nNICK nick@invalido\nQUIT :bye" \
"432"

# dois clientes, um muda para o nick do outro
{
    echo -e "PASS $PASS\r\nNICK nickA\r\nUSER nickA 0 * :NickA\r\n"
    sleep 2
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/nickA.txt 2>/dev/null &
NICKA_PID=$!
sleep 0.4

run_test "33. NICK mudar para nick em uso recebe 433" \
"PASS $PASS\nNICK nickB\nUSER nickB 0 * :NickB\nNICK nickA\nQUIT :bye" \
"433"

wait $NICKA_PID 2>/dev/null

# nick muda e os outros membros do canal são notificados
{
    echo -e "PASS $PASS\r\nNICK watcher\r\nUSER watcher 0 * :Watcher\r\n"
    sleep 0.5
    echo -e "JOIN #nickwatch\r\n"
    sleep 2.5
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/watcher.txt 2>/dev/null &
WATCH_PID=$!
sleep 0.5

{
    echo -e "PASS $PASS\r\nNICK changer\r\nUSER changer 0 * :Changer\r\n"
    sleep 0.8
    echo -e "JOIN #nickwatch\r\n"
    sleep 0.3
    echo -e "NICK newname\r\n"
    sleep 0.5
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/changer.txt 2>/dev/null

run_test_multi "34. Mudança de NICK notifica outros membros do canal" "/tmp/watcher.txt" "NICK"

wait $WATCH_PID 2>/dev/null

# ── 12. QUIT ──────────────────────────────────────────────────────────────────
echo ""
echo "[ quit ]"

{
    echo -e "PASS $PASS\r\nNICK quitter\r\nUSER quitter 0 * :Quitter\r\n"
    sleep 0.5
    echo -e "JOIN #quitroom\r\n"
    sleep 2
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/quitter.txt 2>/dev/null &
QUIT_PID=$!
sleep 0.5

{
    echo -e "PASS $PASS\r\nNICK observer\r\nUSER observer 0 * :Observer\r\n"
    sleep 0.8
    echo -e "JOIN #quitroom\r\n"
    sleep 0.5
    echo -e "QUIT :saindo\r\n"
    sleep 0.3
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/observer.txt 2>/dev/null

run_test_multi "35. QUIT notifica outros membros do canal" "/tmp/quitter.txt" "QUIT"

wait $QUIT_PID 2>/dev/null

# ── 13. INVITE ────────────────────────────────────────────────────────────────
echo ""
echo "[ invite ]"

run_test "36. INVITE para canal inexistente recebe 403" \
"PASS $PASS\nNICK u10\nUSER u10 0 * :U Ten\nINVITE alguem #naoeexiste\nQUIT :bye" \
"403"

run_test "37. INVITE nick inexistente recebe 401" \
"PASS $PASS\nNICK u10\nUSER u10 0 * :U Ten\nJOIN #invroom\nINVITE fantasma #invroom\nQUIT :bye" \
"401"

# op convida e o convidado recebe a notificação
{
    echo -e "PASS $PASS\r\nNICK invop\r\nUSER invop 0 * :InvOp\r\n"
    sleep 0.5
    echo -e "JOIN #viproom\r\n"
    sleep 1.5
    echo -e "INVITE guest #viproom\r\n"
    sleep 1
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/invop.txt 2>/dev/null &
INVOP_PID=$!
sleep 0.4

{
    echo -e "PASS $PASS\r\nNICK guest\r\nUSER guest 0 * :Guest\r\n"
    sleep 0.8
    sleep 2
    echo -e "QUIT\r\n"
} | nc $SERVER $PORT > /tmp/guest.txt 2>/dev/null

run_test_multi "38. INVITE — convidado recebe notificação INVITE" "/tmp/guest.txt" "INVITE"
run_test_multi "39. INVITE — operador recebe 341 RPL_INVITING" "/tmp/invop.txt" "341"

wait $INVOP_PID 2>/dev/null

# ── 14. parser — dados parciais ───────────────────────────────────────────────
echo ""
echo "[ parser — dados parciais ]"

# subject exige que o servidor agrupe pacotes parciais antes de processar
# simula o teste do subject: envia "com" + "man" + "d\n" em partes separadas
{
    printf "PASS %s\r\n" "$PASS"
    sleep 0.2
    printf "NIC"
    sleep 0.1
    printf "K u11\r\n"
    sleep 0.2
    printf "USER u11 0 * :U Eleven\r\n"
    sleep 0.3
    printf "QUI"
    sleep 0.1
    printf "T\r\n"
} | nc $SERVER $PORT > /tmp/partial.txt 2>/dev/null

run_test_multi "40. Parser agrega dados parciais correctamente (recebe 001)" "/tmp/partial.txt" "001"

# ── resultado final ───────────────────────────────────────────────────────────
echo ""
echo "================================================"
echo " $passed passaram  |  $failed falharam"
echo "================================================"

rm -f /tmp/c1_nick.txt /tmp/op_i.txt /tmp/conv_i.txt \
      /tmp/op_k.txt /tmp/op_l.txt /tmp/kicker.txt \
      /tmp/vitima.txt /tmp/naoop.txt /tmp/boss.txt \
      /tmp/vit1.txt /tmp/vit2.txt /tmp/holder.txt \
      /tmp/recvr.txt /tmp/sendr.txt /tmp/recvr2.txt /tmp/sendr2.txt \
      /tmp/topop.txt /tmp/topmem.txt /tmp/modeop.txt \
      /tmp/modeop2.txt /tmp/modemem.txt /tmp/nickA.txt \
      /tmp/watcher.txt /tmp/changer.txt /tmp/quitter.txt \
      /tmp/observer.txt /tmp/invop.txt /tmp/guest.txt /tmp/partial.txt