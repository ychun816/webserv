#!/usr/bin/env python3
# Fichier: stress_test.py avancé

import time
import requests
import concurrent.futures
import argparse
import sys
import random
import string
import os
from collections import defaultdict
from statistics import mean, median, stdev
import multiprocessing

def generate_random_payload(size_kb):
    """Génère des données aléatoires de taille spécifiée en KB"""
    chars = string.ascii_letters + string.digits
    return ''.join(random.choice(chars) for _ in range(size_kb * 1024))

def send_request(url, method="GET", data=None, headers=None, keep_alive=False, random_params=False):
    """Envoie une requête HTTP avec options avancées"""
    start_time = time.time()
    
    # Ajout d'un paramètre aléatoire pour éviter le cache
    if random_params:
        rand_val = ''.join(random.choice(string.ascii_lowercase) for _ in range(8))
        url = f"{url}{'&' if '?' in url else '?'}_nocache={rand_val}"
    
    # Configuration des en-têtes
    if headers is None:
        headers = {}
        
    if not keep_alive:
        headers["Connection"] = "close"
    else:
        headers["Connection"] = "keep-alive"
        
    # Headers supplémentaires pour simuler un navigateur
    user_agents = [
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.1.1 Safari/605.1.15",
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.101 Safari/537.36"
    ]
    headers["User-Agent"] = random.choice(user_agents)
    
    try:
        if method == "GET":
            response = requests.get(url, headers=headers, timeout=10)
        elif method == "POST":
            response = requests.post(url, data=data, headers=headers, timeout=10)
        elif method == "PUT":
            response = requests.put(url, data=data, headers=headers, timeout=10)
        elif method == "DELETE":
            response = requests.delete(url, headers=headers, timeout=10)
        else:
            raise ValueEr_argument("url", help="URL du serveur à tester")
    parser.add_argument("-n", "--requests", type=int, default=1000, help="Nombre de requêtes à envoyer")
    parser.add_argument("-c", "--concurrency", type=int, default=10, help="Niveau de concurrence")
    parser.add_argument("-m", "--method", default="GET", choices=["GET", "POST", "PUT", "DELETE"], help="Méthode HTTP")
    parser.add_argument("-p", "--processes", type=int, default=1, help="Nombre de processus à utiliser")
    parser.add_argument("-s", "--payload-size", type=int, default=0, help="Taille du payload en KB pour POST/PUT")
    parser.add_argument("-r", "--ramp-up", action="store_true", help="Activer la montée en charge progressive")
    parser.add_argument("-k", "--keep-alive", action="store_true", help="Activer les connexions persistantes")
    parser.addror(f"Méthode non supportée: {method}")
            
        end_time = time.time()
        elapsed = (end_time - start_time) * 1000  # en ms
        return {
            "status_code": response.status_code,
            "time": elapsed,
            "success": 200 <= response.status_code < 400,
            "size": len(response.content),
            "content_type": response.headers.get("Content-Type", "")
        }
    except Exception as e:
        end_time = time.time()
        elapsed = (end_time - start_time) * 1000  # en ms
        return {
            "status_code": 0,
            "time": elapsed,
            "success": False,
            "error": str(e),
            "size": 0
        }

def worker_process(args):
    """Fonction exécutée par chaque processus pour distribuer la charge"""
    url, batch_size, concurrency, method, payload_size, keep_alive, random_params = args
    results = []
    
    with concurrent.futures.ThreadPoolExecutor(max_workers=concurrency) as executor:
        if method in ["POST", "PUT"] and payload_size > 0:
            data = generate_random_payload(payload_size)
            futures = [
                executor.submit(send_request, url, method, data, None, keep_alive, random_params)
                for _ in range(batch_size)
            ]
        else:
            futures = [
                executor.submit(send_request, url, method, None, None, keep_alive, random_params)
                for _ in range(batch_size)
            ]
            
        for future in concurrent.futures.as_completed(futures):
            results.append(future.result())
            
    return results

def run_stress_test(url, num_requests, concurrency, method="GET", processes=1, 
                   payload_size=0, ramp_up=False, keep_alive=False, random_params=False):
    """Fonction principale de test de charge avec options avancées"""
    print(f"Démarrage du test de stress sur {url}")
    print(f"Requêtes: {num_requests}, Concurrence: {concurrency}, Méthode: {method}")
    print(f"Processus: {processes}, Taille de payload: {payload_size}KB")
    print(f"Keep-Alive: {'Oui' if keep_alive else 'Non'}, Params aléatoires: {'Oui' if random_params else 'Non'}")
    
    if ramp_up:
        print("Mode montée_argument("url", help="URL du serveur à tester")
    parser.add_argument("-n", "--requests", type=int, default=1000, help="Nombre de requêtes à envoyer")
    parser.add_argument("-c", "--concurrency", type=int, default=10, help="Niveau de concurrence")
    parser.add_argument("-m", "--method", default="GET", choices=["GET", "POST", "PUT", "DELETE"], help="Méthode HTTP")
    parser.add_argument("-p", "--processes", type=int, default=1, help="Nombre de processus à utiliser")
    parser.add_argument("-s", "--payload-size", type=int, default=0, help="Taille du payload en KB pour POST/PUT")
    parser.add_argument("-r", "--ramp-up", action="store_true", help="Activer la montée en charge progressive")
    parser.add_argument("-k", "--keep-alive", action="store_true", help="Activer les connexions persistantes")
    parser.add en charge: augmentation progressive de la charge")
    
    results = []
    start_time = time.time()
    
    # Diviser les requêtes entre les processus
    requests_per_process = [num_requests // processes] * processes
    for i in range(num_requests % processes):
        requests_per_process[i] += 1
    
    # Montée en charge progressive si demandée
    if ramp_up:
        concurrency_levels = []
        current = max(1, concurrency // 10)
        while current < concurrency:
            concurrency_levels.append(current)
            current = min(concurrency, int(current * 1.5))
        concurrency_levels.append(concurrency)
        
        for level in concurrency_levels:
            print(f"Niveau de concurrence: {level}")
            process_args = [
                (url, req, level // processes, method, payload_size, keep_alive, random_params)
                for req in requests_per_process
            ]
            
            with multiprocessing.Pool(processes=processes) as pool:
                batch_results = pool.map(worker_process, process_args)
                for batch in batch_results:
                    results.extend(batch)
            
            # Petite pause entre les niveaux
            if level < concurrency:
                print("Pause de 2 secondes avant le prochain niveau...")
                time.sleep(2)
    else:
        # Exécution standard
        process_args = [
            (url, req, concurrency // processes, method, payload_size, keep_alive, random_params)
            for req in requests_per_process
        ]
        
        with multiprocessing.Pool(processes=processes) as pool:
            batch_results = pool.map(worker_process, process_args)
            for batch in batch_results:
                results.extend(batch)
    
    end_time = time.time()
    total_time = end_time - start_time
    
    # Analyse des résultats
    success_count = sum(1 for r in results if r.get("success", False))
    status_codes = defaultdict(int)
    response_times = [r["time"] for r in results if "time" in r]
    total_bytes = sum(r.get("size", 0) for r in results)
    
    for result in results:
        status_codes[result.get("status_code", 0)] += 1
    
    # Utiliser le nombre de résultats réels plutôt que num_requests
    total_requests = len(results)
    
    print("\n--- Résultats ---")
    print(f"Temps total d'exécution: {total_time:.2f} secondes")
    print(f"Requêtes par seconde: {total_requests / total_time:.2f}")
    print(f"Requêtes réussies: {success_count} ({success_count / total_requests * 100:.2f}%)")
    print(f"Requêtes échouées: {total_requests - success_count} ({(total_requests - success_count) / total_requests * 100:.2f}%)")
    print(f"Total des données transférées: {total_bytes / 1024:.2f} KB")
    
    print("\n--- Codes de statut ---")
    for code, count in sorted(status_codes.items()):
        print(f"Code {code}: {count} requêtes ({count / total_requests * 100:.2f}%)")
    
    if response_times:
        print("\n--- Temps de réponse (ms) ---")
        print(f"Min: {min(response_times):.2f}")
        print(f"Max: {max(response_times):.2f}")
        print(f"Moyenne: {mean(response_times):.2f}")
        print(f"Médiane: {median(response_times):.2f}")
        if len(response_times) > 1:
            print(f"Écart-type: {stdev(response_times):.2f}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Script de test de charge pour serveur web")
    parser.add_argument("url", help="URL du serveur à tester")
    parser.add_argument("-n", "--requests", type=int, default=1000, help="Nombre de requêtes à envoyer")
    parser.add_argument("-c", "--concurrency", type=int, default=10, help="Niveau de concurrence")
    parser.add_argument("-m", "--method", default="GET", choices=["GET", "POST", "PUT", "DELETE"], help="Méthode HTTP")
    parser.add_argument("-p", "--processes", type=int, default=1, help="Nombre de processus à utiliser")
    parser.add_argument("-s", "--payload-size", type=int, default=0, help="Taille du payload en KB pour POST/PUT")
    parser.add_argument("-r", "--ramp-up", action="store_true", help="Activer la montée en charge progressive")
    parser.add_argument("-k", "--keep-alive", action="store_true", help="Activer les connexions persistantes")
    parser.add_argument("--random-params", action="store_true", help="Ajouter des paramètres aléatoires pour éviter le cache")
    
    args = parser.parse_args()
    
    run_stress_test(args.url, args.requests, args.concurrency, args.method, args.processes, 
                    args.payload_size, args.ramp_up, args.keep_alive, args.random_params)