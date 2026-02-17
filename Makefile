# Makefile pour GRO221 - Analyseur de télémétrie
#
# Cibles:
#   make            - Compile les deux programmes
#   make simulateur - Compile seulement le simulateur
#   make analyseur  - Compile seulement l'analyseur
#   make test       - Génère un fichier de test et l'analyse
#   make clean      - Supprime les fichiers générés

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

.PHONY: all clean test

all: simulateur analyseur

simulateur: simulateur_telemetrie.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

analyseur: analyseur_telemetrie.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

# Génère un fichier de test de 100 trames
donnees_test.bin: simulateur
	./simulateur -n 100 > $@

# Test avec fichier
test: all donnees_test.bin
	./analyseur donnees_test.bin rapport.txt 5.0
	@echo ""
	@echo "=== Rapport généré dans rapport.txt ==="
	@head -50 rapport.txt

# Test avec pipe (10 trames seulement)
test-pipe: all
	./simulateur -n 10 | ./analyseur -

clean:
	rm -f simulateur analyseur donnees_test.bin rapport.txt
