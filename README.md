# GRO221 - Analyseur de télémétrie

Ce dossier contient les fichiers pour la problématique de GRO221.

## Fichiers fournis

| Fichier | Description |
|---------|-------------|
| `simulateur_telemetrie.cpp` | Simulateur complet (fourni) - génère des données réalistes |
| `analyseur_telemetrie.cpp` | **Squelette à compléter** - analyse les trames |
| `Makefile` | Script de compilation |

## Compilation

```bash
# Compiler les deux programmes
make

# Compiler seulement le simulateur
make simulateur

# Compiler seulement l'analyseur  
make analyseur
```

## Utilisation

### Simulateur (fourni, complet)

Le simulateur génère des trames de télémétrie réalistes sur la sortie standard.

```bash
# Générer un fichier de 100 trames
./simulateur -n 100 > donnees.bin

# Générer un flux continu en temps réel
./simulateur -r

# Options disponibles
./simulateur -h
```

Options :
- `-n <nombre>` : Nombre de trames (0 = infini)
- `-f <freq>` : Fréquence en Hz (défaut: 100)
- `-b <prob>` : Probabilité de bruit entre trames (défaut: 0.05)
- `-a <prob>` : Probabilité d'alerte courant (défaut: 0.02)
- `-r` : Mode temps réel (attend entre les trames)

### Analyseur (à compléter)

L'analyseur lit des données depuis un fichier ou stdin et produit un rapport.

```bash
# Analyser un fichier
./analyseur donnees.bin

# Analyser un fichier avec sortie dans un fichier
./analyseur donnees.bin rapport.txt

# Analyser avec un seuil d'alerte personnalisé (4.5 A)
./analyseur donnees.bin rapport.txt 4.5

# Analyser depuis un pipe (stdin)
./simulateur -n 50 | ./analyseur -

# Flux continu temps réel
./simulateur -r | ./analyseur - rapport.txt
```

## Tests

```bash
# Test automatique avec fichier
make test

# Test avec pipe
make test-pipe
```

## Format des trames

Chaque trame fait 39 octets :

```
+--------+--------+--------+--------+--------+--------+--------+...
| 0xAA   | 0x55   | SEQ    | AXE 1 (6 oct)   | AXE 2 (6 oct)   |...
+--------+--------+--------+--------+--------+--------+--------+...
```

Structure d'un axe (6 octets, little-endian) :
- Position : `int16_t`, centièmes de degré
- Vitesse : `int16_t`, dixièmes de degré/seconde  
- Courant : `uint16_t`, milliampères

## Travail à réaliser

Dans `analyseur_telemetrie.cpp`, compléter les sections marquées `TODO` :

1. **Structures de données** :
   - Définir `DonneesAxe` (3 membres : position, vitesse, courant)
   - Définir `Trame` (sync1, sync2, sequence, axes[6])
   - **IMPORTANT** : Utiliser `#pragma pack(push, 1)` pour des structures compactes
   - Vérifier les tailles : `sizeof(DonneesAxe)` doit être 6, `sizeof(Trame)` doit être 39

2. **Fonctions de conversion** : `position_en_degres()`, `vitesse_en_deg_s()`, `courant_en_amperes()`

3. **Détection de trames** : `trouver_sync()`, `trame_valide()`, `decoder_trame()`

4. **Analyse** : `est_en_alerte()`, `analyser_trame()`

5. **Entrées/sorties** : `lire_donnees()`, `ecrire_rapport_trame()`

6. **Boucle principale** : Parcours du buffer et traitement des trames

**Note importante** : Le simulateur ne contient pas de définitions de structures -
il génère les trames directement en octets bruts. Vous devez créer vos propres
structures basées sur la spécification du format dans la problématique.

## Exemple de sortie attendue

```
Analyse de télémétrie - Seuil d'alerte: 5.0 A
========================================

Trame #  0
  Axe 1:   0.00° |   0.0°/s | 0.812 A
  Axe 2:  15.23° |  12.5°/s | 1.245 A
  Axe 3: -45.67° | -23.1°/s | 2.103 A
  Axe 4:  90.00° |   0.0°/s | 0.198 A
  Axe 5:   5.50° |   5.2°/s | 0.456 A
  Axe 6: 180.00° |  45.0°/s | 0.789 A

Trame #  1
  Axe 1:   0.12° |   1.2°/s | 0.834 A
  ...

========================================
STATISTIQUES
========================================
Octets lus          : 3942
Octets de bruit     : 42
Trames valides      : 100
Trames avec alerte  : 2
Séquence min        : 0
Séquence max        : 99
========================================
```

## Conseils

- **Commencez par définir les structures** : C'est la base de tout !
- Vérifiez immédiatement les tailles avec `sizeof()` :
  ```cpp
  std::cout << "Taille DonneesAxe: " << sizeof(DonneesAxe) << " (doit être 6)\n";
  std::cout << "Taille Trame: " << sizeof(Trame) << " (doit être 39)\n";
  ```
- Si les tailles ne correspondent pas, vous avez oublié `#pragma pack` ou mal défini les types
- Testez d'abord avec un fichier (`./simulateur -n 10 > test.bin`) avant le pipe
- Utilisez un débogueur (`gdb`) pour comprendre le parcours mémoire
- Attention à l'ordre des octets (little-endian) - sur les machines modernes (x86, ARM),
  vous pouvez utiliser le transtypage direct avec `reinterpret_cast`
- N'essayez pas de copier les structures depuis le simulateur - elles n'existent pas !
  C'est voulu pour que vous appreniez à les définir vous-mêmes.
