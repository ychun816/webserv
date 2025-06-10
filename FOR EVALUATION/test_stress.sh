#!/bin/bash

# Fonction pour envoyer une requête et attendre la réponse
send_request() {
    printf "$1" | nc localhost 8080
    sleep 1
}

echo "Début des tests de stress..."

# 1. Requête avec requete vide
echo "Test 2: Méthode invalide"
printf "" | nc localhost 8080
sleep 1



# 2. Requête avec méthode invalide
echo "Test 2: Méthode invalide"
printf "INVALID / HTTP/1.1\r\n\r\n" | nc localhost 8080
sleep 1

# 3. Requête avec URI très longue
echo "Test 3: URI très longue"
printf "GET /%s HTTP/1.1\r\n\r\n" "$(printf 'a%.0s' {1..10000})" | nc localhost 8080
sleep 1

# 5. Requête avec content-length invalide
echo "Test 5: Content-Length invalide"
printf "POST / HTTP/1.1\r\nContent-Length: -1\r\n\r\n" | nc localhost 8080
sleep 1

# 6. Requête avec transfer-encoding chunked malformé
echo "Test 6: Transfer-Encoding chunked malformé"
printf "POST / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\ninvalid" | nc localhost 8080
sleep 1

# 7. Requête avec plusieurs content-length
echo "Test 7: Plusieurs Content-Length"
printf "POST / HTTP/1.1\r\nContent-Length: 5\r\nContent-Length: 10\r\n\r\n" | nc localhost 8080
sleep 1

# 8. Requête avec caractères spéciaux dans l'URI
echo "Test 8: Caractères spéciaux dans l'URI"
printf "GET /%%00%%01%%02%%03%%04%%05 HTTP/1.1\r\n\r\n" | nc localhost 8080
sleep 1

# 10. Requête avec version HTTP invalide
echo "Test 10: Version HTTP invalide"
printf "GET / HTTP/999.999\r\n\r\n" | nc localhost 8080
sleep 1

# 11. Requête avec headers très longs
echo "Test 11: Headers très longs"
printf "GET / HTTP/1.1\r\n%s\r\n" "$(printf 'X-Test: %s\r\n' $(printf 'a%.0s' {1..1000}))" | nc localhost 8080
sleep 1

# 12. Requête avec body très long
echo "Test 12: Body très long"
printf "POST / HTTP/1.1\r\nContent-Length: 1000000\r\n\r\n%s" "$(printf 'a%.0s' {1..1000000})" | nc localhost 8080
sleep 1

# 13. Requête avec plusieurs méthodes
echo "Test 13: Plusieurs méthodes"
printf "GET POST DELETE / HTTP/1.1\r\n\r\n" | nc localhost 8080
sleep 1



