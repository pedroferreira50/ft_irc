#!/bin/bash

SERVER="localhost"
PORT="6667"
PASS="1722"

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

# --- NOVOS TESTES ADICIONADOS AQUI ---

# Teste 16 e 17: KICK de múltiplos usuários em um único canal (O que você implementou)
# O 'boss' entra no #canal1 e chuta 'chato1' e 'chato2' de uma vez por "KICK #canal1 chato1,chato2 :limpar"
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

# Teste 18: Segurança do Parser (Múltiplos canais devem falhar/ignorar se você não suporta)
# Se mandarem "KICK #canal1,#canal2 chato3", o servidor não pode crashar, deve dar 403 (ou tratar o canal literal com vírgula)
run_test "18. KICK com múltiplos canais não quebra o servidor (Gera 403)" \
"PASS $PASS\nNICK u5\nUSER u5 0 * :U Five\nKICK #canal1,#canal2 chato3\nQUIT :bye" \
"403"


# ── resultado final ───────────────────────────────────────────────────────────
echo ""
echo "================================================"
echo " $passed passaram  |  $failed falharam"
echo "================================================"

rm -f /tmp/c1_nick.txt /tmp/op_i.txt /tmp/conv_i.txt \
      /tmp/op_k.txt /tmp/op_l.txt /tmp/kicker.txt \
      /tmp/vitima.txt /tmp/naoop.txt /tmp/boss.txt \
      /tmp/vit1.txt /tmp/vit2.txt