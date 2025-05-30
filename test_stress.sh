#!/bin/bash

# Fonction pour envoyer une requête et attendre la réponse
send_request() {
    echo -e "$1" | nc localhost 8080
    sleep 0.1
}

echo "Début des tests de stress..."

# 1. Requête vide
echo "Test 1: Requête vide"
printf "" | nc localhost 8080
sleep 0.1

# 2. Requête avec méthode invalide
echo "Test 2: Méthode invalide"
echo -e "INVALID / HTTP/1.1\r\n\r\n" | nc localhost 8080
sleep 0.1

# 3. Requête avec URI très longue
echo "Test 3: URI très longue"
echo -e "GET /$(printf 'a%.0s' {1..10000}) HTTP/1.1\r\n\r\n" | nc localhost 8080
sleep 0.1

# 4. Requête avec headers malformés
echo "Test 4: Headers malformés"
echo -e "GET / HTTP/1.1\r\nInvalid: Header\r\n\r\n" | nc localhost 8080
sleep 0.1

# 5. Requête avec content-length invalide
echo "Test 5: Content-Length invalide"
echo -e "POST / HTTP/1.1\r\nContent-Length: -1\r\n\r\n" | nc localhost 8080
sleep 0.1

# 6. Requête avec transfer-encoding chunked malformé
echo "Test 6: Transfer-Encoding chunked malformé"
echo -e "POST / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\ninvalid" | nc localhost 8080
sleep 0.1

# 7. Requête avec plusieurs content-length
echo "Test 7: Plusieurs Content-Length"
echo -e "POST / HTTP/1.1\r\nContent-Length: 5\r\nContent-Length: 10\r\n\r\n" | nc localhost 8080
sleep 0.1

# 8. Requête avec caractères spéciaux dans l'URI
echo "Test 8: Caractères spéciaux dans l'URI"
echo -e "GET /%00%01%02%03%04%05 HTTP/1.1\r\n\r\n" | nc localhost 8080
sleep 0.1

# 9. Requête avec méthode très longue
echo "Test 9: Méthode très longue"
echo -e "$(printf 'G%.0s' {1..1000})ET / HTTP/1.1\r\n\r\n" | nc localhost 8080
sleep 0.1

# 10. Requête avec version HTTP invalide
echo "Test 10: Version HTTP invalide"
echo -e "GET / HTTP/999.999\r\n\r\n" | nc localhost 8080
sleep 0.1

# 11. Requête avec headers très longs
echo "Test 11: Headers très longs"
echo -e "GET / HTTP/1.1\r\n$(printf 'X-Test: %s\r\n' $(printf 'a%.0s' {1..1000}))\r\n" | nc localhost 8080
sleep 0.1

# 12. Requête avec body très long
echo "Test 12: Body très long"
echo -e "POST / HTTP/1.1\r\nContent-Length: 1000000\r\n\r\n$(printf 'a%.0s' {1..1000000})" | nc localhost 8080
sleep 0.1

# 13. Requête avec plusieurs méthodes
echo "Test 13: Plusieurs méthodes"
echo -e "GET POST DELETE / HTTP/1.1\r\n\r\n" | nc localhost 8080
sleep 0.1

# 14. Requête avec caractères de contrôle
echo "Test 14: Caractères de contrôle"
echo -e "GET / HTTP/1.1\r\n\r\n$(printf '\x00\x01\x02\x03\x04\x05')" | nc localhost 8080
sleep 0.1

# 15. Requête avec encodage invalide
echo "Test 15: Encodage invalide"
echo -e "GET / HTTP/1.1\r\nContent-Encoding: invalid\r\n\r\n" | nc localhost 8080
sleep 0.1

echo "Tests terminés" 