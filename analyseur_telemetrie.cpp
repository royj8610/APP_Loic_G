/**
 * @file analyseur_telemetrie.cpp
 * @brief Analyseur de trames de télémétrie pour bras robotisé 6 axes
 * 
 * Ce programme lit des données de télémétrie depuis un fichier ou l'entrée
 * standard (stdin), détecte et décode les trames, puis produit un rapport.
 * 
 * Usage :
 *     ./analyseur <fichier_entree> [fichier_sortie] [seuil_courant]
 *     ./simulateur | ./analyseur - [fichier_sortie] [seuil_courant]
 * 
 * Arguments :
 *     fichier_entree   Fichier binaire à analyser, ou "-" pour stdin
 *     fichier_sortie   Fichier de rapport (défaut: stdout)
 *     seuil_courant    Seuil d'alerte en ampères (défaut: 5.0)
 * 
 * @author Loïc Giroux
 * @date 2026-02-17
 * 
 * GRO221 - Introduction à la programmation système
 * Université de Sherbrooke
 */

#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <vector>
#include <iomanip>

// ============================================================================
// Constantes du protocole
// ============================================================================

const uint8_t SYNC_H = 0xAA;
const uint8_t SYNC_L = 0x55;
const size_t NB_AXES = 6;
const size_t TAILLE_AXE = 6;        // 2 + 2 + 2 octets
const size_t TAILLE_ENTETE = 3;     // sync1 + sync2 + sequence
const size_t TAILLE_TRAME = TAILLE_ENTETE + NB_AXES * TAILLE_AXE;  // 39 octets


// ============================================================================
// Structures de données
// ============================================================================

// Attention: Pensez à l'alignement des structures!

struct DonneesAxe {
    uint16_t position;
    uint16_t vitesse;
    uint16_t courant;   
};
#pragma pack(push,1) // Empeche le padding pour la reinterpretation dans decoder trame
struct  Trame { 
    uint8_t SYNC_1;
    uint8_t SYNC_2;
    uint8_t SEQ;
    DonneesAxe Axes[6];
};
#pragma pack(pop)
// Structure pour les statistiques (déjà complète)
struct Statistiques {
    size_t octets_lus = 0;
    size_t trames_valides = 0;
    size_t trames_alerte = 0;
    uint8_t sequence_min = 255;
    uint8_t sequence_max = 0;
    size_t octets_bruit = 0;
};
size_t trames_perdues = 0;
// ============================================================================
// Fonctions de conversion
// ============================================================================

/**
 * @brief Convertit une position brute en degrés
 * @param brut Valeur brute en centièmes de degré (int16_t)
 * @return Position en degrés (float)
 */
float position_en_degres(int16_t brut) {
    float position_deg=(float)brut/100.0f; // La valeur brut est divisé par 100
    return position_deg;
}

/**
 * @brief Convertit une vitesse brute en degrés par seconde
 * @param brut Valeur brute en dixièmes de degré/seconde (int16_t)
 * @return Vitesse en degrés/seconde (float)
 */
float vitesse_en_deg_s(int16_t brut) {
    float vitesse_deg=(float)brut/10.0f; // La valeur brut est divisé par 10
    return vitesse_deg;
}

/**
 * @brief Convertit un courant brut en ampères
 * @param brut Valeur brute en milliampères (uint16_t)
 * @return Courant en ampères (float)
 */
float courant_en_amperes(uint16_t brut) {
    float courant_amp=(float)brut/1000.0f; // La valeur brut est divisé par 1000
    return courant_amp;
}

// ============================================================================
// Fonctions de détection et décodage
// ============================================================================

/**
 * @brief Recherche les octets de synchronisation dans un tampon
 * 
 * Parcourt le tampon à partir de la position donnée et recherche la séquence
 * de synchronisation (0xAA 0x55). Retourne la position du premier octet de
 * sync si trouvé, ou -1 si non trouvé.
 * 
 * @param buffer Pointeur vers le tampon de données
 * @param taille Taille du tampon en octets
 * @param debut Position de départ pour la recherche
 * @return Position de la séquence de sync, ou -1 si non trouvée
 */
int trouver_sync(const uint8_t* buffer, size_t taille, size_t debut) {


for (size_t i = debut; i < taille - 1; i++) {
        if (buffer[i] == SYNC_H && buffer[i + 1] == SYNC_L) { //Cherche la suite d'octet 0xAA et 0x55
            return i;  // Retourne la position de 0xAA
        }
    }
    return -1;  // Non trouvé
}

/**
 * @brief Vérifie si une trame est valide
 * 
 * Une trame est considérée valide si ses octets de synchronisation sont
 * corrects. (0xAA et 0x55)
 * 
 * @param trame Pointeur vers la trame à vérifier
 * @return true si la trame est valide, false sinon
 */
bool trame_valide(const Trame* trame) {

    if (trame->SYNC_1==SYNC_H && trame->SYNC_2==SYNC_L){ //Vérifie que les octect SYNC_1 et SYNC_2 sont égal a ce qui est recherché
        return true;
    }
    else{
        return false; // Retourne False si ce n'est pas le cas
    }
}

/**
 * @brief Décode une trame depuis un tampon d'octets bruts
 *
 * Cette fonction extrait les données d'une trame à partir d'un pointeur
 * vers le tampon brut. Elle suppose que le pointeur pointe vers le début
 * d'une trame (premier octet de sync).
 *
 * @param buffer Pointeur vers le début de la trame dans le tampon
 * @param trame Structure où stocker les données décodées
 * @return true si le décodage a réussi, false sinon
 */
bool decoder_trame(const uint8_t* buffer, Trame& trame) {
    // TODO: Implémenter le décodage de la trame
    // Si vos structures sont bien définies et alignées, un simple
    // transtypage peut suffire.
    trame.SYNC_1 = buffer[0];
    trame.SYNC_2 = buffer[1];
    trame.SEQ = buffer[2];

    uint16_t temp = 0; // variable temporaire d'opération initialisé à 0 (vide)
    buffer +=3; // decalage buffer initial
    for (int NbAxe=0; NbAxe<6; NbAxe++){
        for(int donne = 0; donne < 6; donne+=2 ){
            temp = temp + buffer[donne+1];        // décalage little-endian (8 derniers bits sur 16)
            temp = (temp << 8) + buffer[donne];   // ajout des 8 premiers bits
                if(donne == 0){                   // injection des données décodés 
                    trame.Axes[NbAxe].position = temp;
                }
                else if (donne == 2){
                    trame.Axes[NbAxe].vitesse = temp;
                }
                else if (donne == 4){
                    trame.Axes[NbAxe].courant = temp;
                }
                temp = 0; // reinit de variable temporaire
            }
            buffer +=6; // decalage du pointeur buffer
        }
    return true;
    }

// ============================================================================
// Fonctions d'analyse
// ============================================================================

/**
 * @brief Vérifie si un axe est en alerte courant
 * @param axe Données de l'axe à vérifier
 * @param seuil Seuil de courant en ampères
 * @return true si le courant dépasse le seuil
 */
bool est_en_alerte(const DonneesAxe& axe, float seuil) {

    if(courant_en_amperes(axe.courant)>seuil){ //Vérifie que le courant de l'axe est sous le seuil
        return true;
    }
    else{
        return false;
    }
}

/**
 * @brief Analyse une trame et met à jour les statistiques
 * @param trame Trame à analyser
 * @param stats Statistiques à mettre à jour
 * @param seuil Seuil de courant pour les alertes
 * @return true si la trame contient au moins une alerte
 */
bool analyser_trame(const Trame& trame, Statistiques& stats, float seuil) {
    // Mettre à jour les statistiques de séquence


    if (trame.SEQ < stats.sequence_min) {
        stats.sequence_min = trame.SEQ; // Garde en mémoire le numéro de séquence minimum lu
    }
    if (trame.SEQ > stats.sequence_max) {
        stats.sequence_max = trame.SEQ; // Garde en mémoire le numéro de séquence maximum lu
    }   

    for (size_t i = 0; i < NB_AXES; i++) {
        if (est_en_alerte(trame.Axes[i], seuil)) { //Vérifie que le courant de chaque axe ne depasse pas le seuil
            stats.trames_alerte++; //Ajoute 1 au nombre de trame en alerte
            return true;
        }
    }   
    
    return false;
}

// ============================================================================
// Fonctions d'entrée/sortie
// ============================================================================

/**
 * @brief Lit les données depuis un fichier ou stdin dans un vecteur
 * @param source Nom du fichier ou "-" pour stdin
 * @return Vecteur contenant les octets lus
 */
std::vector<uint8_t> lire_donnees(const std::string& source) {
    std::vector<uint8_t> buffer;
    
    if (source == "-") {
        // Lecture depuis stdin
        //
        // INDICES:
        // - Utiliser std::cin.get() pour lire octet par octet
        // - Ou lire par blocs dans un tampon temporaire
        // - Arrêter quand std::cin.eof() est vrai
        
        uint8_t octet;
        while (std::cin.get(reinterpret_cast<char&>(octet))) {
            buffer.push_back(octet); //Ajoute un octet a la fin de buffer tant qu'il y en a
        }
        
        
    } else {
        // Lecture depuis un fichier

        //
        // INDICES:
        // - std::ifstream fichier(source, std::ios::binary)
        // - Vérifier que le fichier est ouvert avec is_open()
        // - Déterminer la taille avec seekg/tellg
        // - Redimensionner le buffer et lire avec read()
        std::ifstream fichier(source, std::ios::binary);
        if (!fichier.is_open()) {
            throw std::runtime_error("Impossible d'ouvrir le fichier " + source); //Renvoi une errreur si incapale d'ouvir le fichier
        }
        fichier.seekg(0, std::ios::end); //Envoi le curseur a la fin du fichier
        size_t taille = fichier.tellg(); //Écrit la position actuel du curseur dans taille
        fichier.seekg(0, std::ios::beg); //Envoi le curseur au début du fichier
        buffer.resize(taille); //Défini la taille du buffer
        fichier.read(reinterpret_cast<char*>(buffer.data()), taille); // Reinterpret les données du fichier et les envoies dans buffer
    }
    
    return buffer;
}

/**
 * @brief Écrit une ligne de rapport pour une trame
 * @param sortie Flux de sortie
 * @param trame Trame à rapporter
 * @param seuil Seuil pour les alertes
 */
void ecrire_rapport_trame(std::ostream& sortie, const Trame& trame, float seuil) {

    
    for (size_t i = 0; i < NB_AXES; i++) {
        // TODO: Écrire les données de chaque axe dans le format suivant:
        //   "  Axe N: XXX.XX° | XXX.X°/s | X.XXX A [ALERTE]"
        //
        // INDICES:
        // - Utiliser std::fixed et std::setprecision() pour le formatage
        // - Ajouter "[!ALERTE!]" si le courant dépasse le seuil
        
        sortie << "  Axe " << (i + 1) << ": ";
        sortie << std::fixed << std::setprecision(2); //Défini la facon d'écrire les données et limite le nombre de chiffre après la virgule a deux
        sortie << position_en_degres(trame.Axes[i].position) << "° | ";
        sortie << std::fixed << std::setprecision(1);
        sortie << vitesse_en_deg_s(trame.Axes[i].vitesse) << "°/s | ";
        sortie << std::fixed << std::setprecision(3);
        sortie << courant_en_amperes(trame.Axes[i].courant) << "A ";

        if (est_en_alerte(trame.Axes[i], seuil)) {
            sortie << "[!ALERTE!]"; // Si le seuil de courant est dépasser écrit ALERTE
        }
        
        sortie << "\n";
    }
    
    sortie << "\n";
}

/**
 * @brief Écrit le résumé des statistiques
 * @param sortie Flux de sortie
 * @param stats Statistiques à afficher
 */
void ecrire_statistiques(std::ostream& sortie, const Statistiques& stats) {
    sortie << "========================================\n";
    sortie << "STATISTIQUES\n";
    sortie << "========================================\n";
    sortie << "Octets lus          : " << stats.octets_lus << "\n";
    sortie << "Octets de bruit     : " << stats.octets_bruit << "\n";
    sortie << "Trames valides      : " << stats.trames_valides << "\n";
    sortie << "Trames avec alerte  : " << stats.trames_alerte << "\n";
    
    if (stats.trames_valides > 0) {
        sortie << "Séquence min        : " << static_cast<int>(stats.sequence_min) << "\n";
        sortie << "Séquence max        : " << static_cast<int>(stats.sequence_max) << "\n";
        

        //       basé sur les numéros de séquence (attention au wrap-around de 255 à 0)

        sortie << "Trames perdues      : " << trames_perdues << "\n";
    }
    
    sortie << "========================================\n";
}

// ============================================================================
// Fonction principale
// ============================================================================

void afficher_aide(const char* prog) {
    std::cerr << "Usage: " << prog << " <fichier_entree> [fichier_sortie] [seuil_courant]\n";
    std::cerr << "\n";
    std::cerr << "Arguments:\n";
    std::cerr << "  fichier_entree   Fichier binaire ou '-' pour stdin\n";
    std::cerr << "  fichier_sortie   Fichier de rapport (défaut: stdout)\n";
    std::cerr << "  seuil_courant    Seuil d'alerte en ampères (défaut: 5.0)\n";
    std::cerr << "\n";
    std::cerr << "Exemples:\n";
    std::cerr << "  " << prog << " donnees.bin\n";
    std::cerr << "  " << prog << " donnees.bin rapport.txt 4.5\n";
    std::cerr << "  ./simulateur | " << prog << " - rapport.txt\n";
}

int main(int argc, char* argv[]) {
    // ========================================================================
    // Analyse des arguments
    // ========================================================================
    if (argc < 2) {
        afficher_aide(argv[0]);
        return 1;
    }
    
    std::string fichier_entree = argv[1];
    std::string fichier_sortie = "";  // Vide = stdout
    float seuil_courant = 5.0f;       // Ampères
    
    if (argc >= 3) {
        fichier_sortie = argv[2];
    }
    if (argc >= 4) {
        try {
            seuil_courant = std::stof(argv[3]);
        } catch (...) {
            std::cerr << "Erreur: seuil de courant invalide\n";
            return 1;
        }
    }
    
    // ========================================================================
    // Lecture des données
    // ========================================================================
    
    std::vector<uint8_t> buffer = lire_donnees(fichier_entree);
    
    if (buffer.empty()) {
        std::cerr << "Erreur: aucune donnée lue\n";
        return 1;
    }
    
    // ========================================================================
    // Configuration de la sortie
    // ========================================================================
    
    std::ofstream fichier_out;
    std::ostream* sortie = &std::cout;
    
    if (!fichier_sortie.empty()) {
        fichier_out.open(fichier_sortie);
        if (!fichier_out.is_open()) {
            std::cerr << "Erreur: impossible de créer " << fichier_sortie << "\n";
            return 1;
        }
        sortie = &fichier_out;
    }
    
    // ========================================================================
    // Traitement des trames
    // ========================================================================
    
    Statistiques stats;
    stats.octets_lus = buffer.size();
    
    *sortie << "Analyse de télémétrie - Seuil d'alerte: " << seuil_courant << " A\n";
    *sortie << "========================================\n\n";
    
    //
    // INDICES:
    // - Parcourir le buffer pour trouver les trames (avec trouver_sync)
    // - Passer les trames trouvées à decoder_trame et trame_valide
    // - Analyser les trames valides avec analyser_trame
    // - Écrire le rapport de chaque trame avec ecrire_rapport_trame
    // - Mettre à jour les statistiques

uint8_t dernier_num_seq = 0;

for(size_t pos = 0; pos < buffer.size();) {
    int sync_pos = trouver_sync(buffer.data(), buffer.size(), pos); // Parcour le buffer pour trouver les octets de syncronisation

    if (sync_pos==-1){ //Arrete la boucle for si aucun octet de syncronisation est trouvé
        break;
    }

    Trame trame;
    if (decoder_trame(buffer.data() + sync_pos, trame)) {
        stats.trames_valides++; //Ajoute 1 a trame valide si decoder trame a réussi

        analyser_trame(trame, stats, seuil_courant);

        if (stats.trames_valides == 1) { //Trouve le numéro de séquence de la première trame decoder
            dernier_num_seq = trame.SEQ;
        } 
        else {
            uint8_t attendu = dernier_num_seq + 1;
            uint8_t seq_actuel = trame.SEQ;

            if (attendu != seq_actuel) { //Ajoute le nombre le nombre de trame perdu si les numéro de trame ne se suivent pas
                uint8_t diff = (seq_actuel-1 - dernier_num_seq);
                trames_perdues += diff;
            }
        }

        dernier_num_seq = trame.SEQ; //Change le dernier numéro de trame pour la prochaine exécution de la boucle for

        ecrire_rapport_trame(*sortie, trame, seuil_courant);
    }

    pos = sync_pos + 39; // Continuer la recherche après le sync actuel
}

stats.octets_bruit = stats.octets_lus-(stats.trames_valides*TAILLE_TRAME); //Calcul le nombre d'octet de bruit



    
    // ========================================================================
    // Affichage des statistiques
    // ========================================================================
    
    ecrire_statistiques(*sortie, stats);
    
    return 0;
}