/**
 * @file simulateur_telemetrie.cpp
 * @brief Simulateur de données de télémétrie pour bras robotisé 6 axes
 * 
 * Ce programme génère des trames de télémétrie réalistes sur la sortie standard,
 * simulant un contrôleur de bras robotisé. Il peut être utilisé avec un pipe
 * pour alimenter le programme d'analyse en temps réel :
 * 
 *     ./simulateur | ./analyseur
 * 
 * Ou pour générer un fichier de test :
 * 
 *     ./simulateur -n 100 > donnees.bin
 * 
 * Options :
 *     -n <nombre>   Nombre de trames à générer (0 = infini, défaut)
 *     -f <freq>     Fréquence en Hz (défaut: 100)
 *     -b <prob>     Probabilité de bruit entre trames (0.0-1.0, défaut: 0.05)
 *     -a <prob>     Probabilité d'alerte courant (0.0-1.0, défaut: 0.02)
 *     -h            Affiche l'aide
 * 
 * @author GRO221 - Université de Sherbrooke
 * @date 2025
 */

#include <iostream>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <chrono>
#include <thread>
#include <random>

// ============================================================================
// Constantes du protocole
// ============================================================================

const uint8_t SYNC_H = 0xAA;
const uint8_t SYNC_L = 0x55;
const size_t NB_AXES = 6;
const size_t TAILLE_TRAME = 39;  // 2 sync + 1 seq + 6*6 données

// Limites physiques réalistes pour un bras robotisé industriel
const float POSITION_MIN_DEG[NB_AXES] = {-170.0f, -90.0f, -80.0f, -190.0f, -120.0f, -360.0f};
const float POSITION_MAX_DEG[NB_AXES] = { 170.0f, 110.0f, 280.0f,  190.0f,  120.0f,  360.0f};
const float VITESSE_MAX_DEG_S[NB_AXES] = {250.0f, 250.0f, 250.0f, 430.0f, 430.0f, 630.0f};
const float COURANT_NOMINAL_A[NB_AXES] = {8.0f, 6.0f, 4.0f, 2.0f, 2.0f, 1.5f};

// ============================================================================
// État du simulateur
// ============================================================================
//
// Note: Le simulateur ne définit PAS de structures pour les trames.
// Les données sont générées directement en octets bruts pour que les étudiants
// apprennent à manipuler les données binaires sans avoir accès aux définitions
// de structures.
// ============================================================================

struct EtatAxe {
    float position_deg;      // Position actuelle en degrés
    float vitesse_deg_s;     // Vitesse actuelle en degrés/seconde
    float position_cible;    // Position cible pour le mouvement
    float courant_base_a;    // Courant de base en ampères
};

struct Simulateur {
    EtatAxe axes[NB_AXES];
    uint8_t sequence;
    std::mt19937 rng;
    
    // Paramètres de simulation
    float dt;                // Pas de temps (1/freq)
    float prob_bruit;        // Probabilité d'injecter du bruit
    float prob_alerte;       // Probabilité d'alerte courant
    
    Simulateur(float freq, float bruit, float alerte) 
        : sequence(0), prob_bruit(bruit), prob_alerte(alerte) {
        
        dt = 1.0f / freq;
        
        // Initialiser le générateur aléatoire
        std::random_device rd;
        rng.seed(rd());
        
        // Initialiser les axes à des positions de repos
        for (size_t i = 0; i < NB_AXES; i++) {
            axes[i].position_deg = 0.0f;
            axes[i].vitesse_deg_s = 0.0f;
            axes[i].position_cible = 0.0f;
            axes[i].courant_base_a = COURANT_NOMINAL_A[i] * 0.1f;  // Repos
        }
    }
};

// ============================================================================
// Fonctions de simulation
// ============================================================================

/**
 * @brief Met à jour la cible de position d'un axe (nouveau mouvement aléatoire)
 */
void nouvelle_cible(Simulateur& sim, size_t axe) {
    std::uniform_real_distribution<float> dist(
        POSITION_MIN_DEG[axe] * 0.8f, 
        POSITION_MAX_DEG[axe] * 0.8f
    );
    sim.axes[axe].position_cible = dist(sim.rng);
}

/**
 * @brief Simule le mouvement d'un axe pour un pas de temps
 */
void simuler_axe(Simulateur& sim, size_t axe) {
    EtatAxe& a = sim.axes[axe];
    
    // Erreur de position
    float erreur = a.position_cible - a.position_deg;
    
    // Si proche de la cible, nouvelle cible (probabilité)
    if (std::abs(erreur) < 1.0f) {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        if (dist(sim.rng) < 0.02f) {  // 2% de chance par frame
            nouvelle_cible(sim, axe);
            erreur = a.position_cible - a.position_deg;
        }
    }
    
    // Contrôle proportionnel simple pour la vitesse
    float vitesse_desiree = erreur * 2.0f;  // Gain proportionnel
    
    // Limiter la vitesse
    float v_max = VITESSE_MAX_DEG_S[axe];
    if (vitesse_desiree > v_max) vitesse_desiree = v_max;
    if (vitesse_desiree < -v_max) vitesse_desiree = -v_max;
    
    // Accélération limitée (lissage)
    float acc_max = v_max * 2.0f;  // Accélération max
    float delta_v = vitesse_desiree - a.vitesse_deg_s;
    float delta_v_max = acc_max * sim.dt;
    if (delta_v > delta_v_max) delta_v = delta_v_max;
    if (delta_v < -delta_v_max) delta_v = -delta_v_max;
    
    a.vitesse_deg_s += delta_v;
    
    // Mise à jour de la position
    a.position_deg += a.vitesse_deg_s * sim.dt;
    
    // Limites de position (butées)
    if (a.position_deg < POSITION_MIN_DEG[axe]) {
        a.position_deg = POSITION_MIN_DEG[axe];
        a.vitesse_deg_s = 0.0f;
    }
    if (a.position_deg > POSITION_MAX_DEG[axe]) {
        a.position_deg = POSITION_MAX_DEG[axe];
        a.vitesse_deg_s = 0.0f;
    }
    
    // Courant : fonction de la vitesse et de l'accélération
    float ratio_vitesse = std::abs(a.vitesse_deg_s) / VITESSE_MAX_DEG_S[axe];
    a.courant_base_a = COURANT_NOMINAL_A[axe] * (0.1f + 0.9f * ratio_vitesse);
    
    // Bruit sur le courant
    std::normal_distribution<float> bruit(0.0f, a.courant_base_a * 0.05f);
    a.courant_base_a += bruit(sim.rng);
    if (a.courant_base_a < 0.0f) a.courant_base_a = 0.0f;
}

/**
 * @brief Écrit une valeur 16 bits dans un buffer en little-endian
 * @param buffer Pointeur vers le buffer de destination
 * @param valeur Valeur à écrire
 */
void ecrire_uint16_le(uint8_t* buffer, uint16_t valeur) {
    buffer[0] = valeur & 0xFF;         // Octet de poids faible
    buffer[1] = (valeur >> 8) & 0xFF;  // Octet de poids fort
}

/**
 * @brief Génère une trame de télémétrie brute (39 octets) à partir de l'état actuel
 *
 * La trame est générée directement en octets bruts, sans utiliser de structures.
 * Ceci force les étudiants à comprendre le format binaire et l'ordre des octets.
 *
 * Format de la trame:
 *   [0]      : SYNC_H (0xAA)
 *   [1]      : SYNC_L (0x55)
 *   [2]      : Numéro de séquence
 *   [3-8]    : Axe 1 (position, vitesse, courant - 6 octets)
 *   [9-14]   : Axe 2
 *   [15-20]  : Axe 3
 *   [21-26]  : Axe 4
 *   [27-32]  : Axe 5
 *   [33-38]  : Axe 6
 *
 * @param sim État du simulateur
 * @param buffer Buffer de 39 octets où écrire la trame
 */
void generer_trame(Simulateur& sim, uint8_t* buffer) {
    // En-tête de la trame
    buffer[0] = SYNC_H;
    buffer[1] = SYNC_L;
    buffer[2] = sim.sequence++;

    std::uniform_real_distribution<float> dist_alerte(0.0f, 1.0f);
    std::uniform_int_distribution<int> dist_axe(0, NB_AXES - 1);

    // Déterminer s'il y a une alerte et sur quel axe
    int axe_alerte = -1;
    uint16_t courant_alerte = 0;
    if (dist_alerte(sim.rng) < sim.prob_alerte) {
        axe_alerte = dist_axe(sim.rng);
        // Courant élevé : 5.5 à 8.0 A (en milliampères)
        std::uniform_real_distribution<float> dist_courant(5500.0f, 8000.0f);
        courant_alerte = static_cast<uint16_t>(dist_courant(sim.rng));
    }

    // Générer les données de chaque axe
    uint8_t* ptr = buffer + 3;  // Pointeur vers les données des axes

    for (size_t i = 0; i < NB_AXES; i++) {
        simuler_axe(sim, i);

        // Conversion vers les unités brutes (entiers)
        int16_t position_brut = static_cast<int16_t>(sim.axes[i].position_deg * 100.0f);
        int16_t vitesse_brut = static_cast<int16_t>(sim.axes[i].vitesse_deg_s * 10.0f);
        uint16_t courant_brut = static_cast<uint16_t>(sim.axes[i].courant_base_a * 1000.0f);

        // Appliquer l'alerte si c'est cet axe
        if (static_cast<int>(i) == axe_alerte) {
            courant_brut = courant_alerte;
        }

        // Écrire les données en little-endian (octet de poids faible en premier)
        // Position (int16_t - 2 octets)
        ecrire_uint16_le(ptr, static_cast<uint16_t>(position_brut));
        ptr += 2;

        // Vitesse (int16_t - 2 octets)
        ecrire_uint16_le(ptr, static_cast<uint16_t>(vitesse_brut));
        ptr += 2;

        // Courant (uint16_t - 2 octets)
        ecrire_uint16_le(ptr, courant_brut);
        ptr += 2;
    }
}

/**
 * @brief Génère des octets de bruit aléatoires (simule désynchronisation)
 */
void generer_bruit(Simulateur& sim, int nb_octets) {
    std::uniform_int_distribution<int> dist(0, 255);
    
    for (int i = 0; i < nb_octets; i++) {
        uint8_t octet = static_cast<uint8_t>(dist(sim.rng));
        // Éviter de générer accidentellement les octets de sync
        if (octet == SYNC_H || octet == SYNC_L) {
            octet = 0x00;
        }
        std::cout.write(reinterpret_cast<char*>(&octet), 1);
    }
}

// ============================================================================
// Fonction principale
// ============================================================================

void afficher_aide(const char* prog) {
    std::cerr << "Usage: " << prog << " [options]\n"
              << "\n"
              << "Simulateur de télémétrie pour bras robotisé 6 axes.\n"
              << "Génère des trames binaires sur stdout.\n"
              << "\n"
              << "Options:\n"
              << "  -n <nombre>   Nombre de trames à générer (0 = infini, défaut)\n"
              << "  -f <freq>     Fréquence en Hz (défaut: 100)\n"
              << "  -b <prob>     Probabilité de bruit entre trames (0.0-1.0, défaut: 0.05)\n"
              << "  -a <prob>     Probabilité d'alerte courant (0.0-1.0, défaut: 0.02)\n"
              << "  -r            Mode temps réel (attente entre trames)\n"
              << "  -h            Affiche cette aide\n"
              << "\n"
              << "Exemples:\n"
              << "  " << prog << " -n 100 > donnees.bin    # Fichier de 100 trames\n"
              << "  " << prog << " -r | ./analyseur        # Flux temps réel via pipe\n"
              << "  " << prog << " -n 1000 -b 0.1 > test.bin  # Avec 10% de bruit\n";
}

int main(int argc, char* argv[]) {
    // Paramètres par défaut
    int nb_trames = 0;           // 0 = infini
    float frequence = 100.0f;    // Hz
    float prob_bruit = 0.05f;    // 5% de chance de bruit
    float prob_alerte = 0.02f;   // 2% de chance d'alerte
    bool temps_reel = false;
    
    // Analyse des arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            afficher_aide(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            nb_trames = std::atoi(argv[++i]);
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            frequence = std::atof(argv[++i]);
        } else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            prob_bruit = std::atof(argv[++i]);
        } else if (strcmp(argv[i], "-a") == 0 && i + 1 < argc) {
            prob_alerte = std::atof(argv[++i]);
        } else if (strcmp(argv[i], "-r") == 0) {
            temps_reel = true;
        } else {
            std::cerr << "Option inconnue: " << argv[i] << "\n";
            std::cerr << "Utilisez -h pour l'aide.\n";
            return 1;
        }
    }
    
    // Validation
    if (frequence <= 0.0f) {
        std::cerr << "Erreur: fréquence invalide\n";
        return 1;
    }
    
    // Initialiser le simulateur
    Simulateur sim(frequence, prob_bruit, prob_alerte);
    
    // Calculer le délai entre trames pour le mode temps réel
    auto periode = std::chrono::microseconds(static_cast<int>(1000000.0f / frequence));
    
    // Générateur pour le bruit
    std::uniform_real_distribution<float> dist_bruit(0.0f, 1.0f);
    std::uniform_int_distribution<int> dist_nb_bruit(1, 10);
    
    // Boucle principale de génération
    int compteur = 0;
    while (nb_trames == 0 || compteur < nb_trames) {
        auto debut = std::chrono::steady_clock::now();
        
        // Occasionnellement, injecter du bruit avant la trame
        if (dist_bruit(sim.rng) < sim.prob_bruit) {
            int nb = dist_nb_bruit(sim.rng);
            generer_bruit(sim, nb);
        }
        
        // Générer et écrire la trame (39 octets bruts)
        uint8_t trame[TAILLE_TRAME];
        generer_trame(sim, trame);
        std::cout.write(reinterpret_cast<char*>(trame), TAILLE_TRAME);
        std::cout.flush();
        
        compteur++;
        
        // Attente pour le mode temps réel
        if (temps_reel) {
            auto fin = std::chrono::steady_clock::now();
            auto duree = std::chrono::duration_cast<std::chrono::microseconds>(fin - debut);
            if (duree < periode) {
                std::this_thread::sleep_for(periode - duree);
            }
        }
    }
    
    return 0;
}
