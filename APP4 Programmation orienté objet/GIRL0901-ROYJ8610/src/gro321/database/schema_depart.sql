-- Schéma de base de données - VERSION DE DÉPART
--
-- Les robots et leurs clients sont déjà correctement modélisés (3FN):
--   - clients: une seule rangée par client (aucune redondance);
--   - robots: une rangée par robot, relié à son client par clé étrangère.
-- Ce code, ainsi que son interface web CRUD, sert de référence fonctionnelle.
--
-- La table bons_travail, elle, est une preuve de concept laissée par le
-- collègue: ni arrimée au reste de la base, ni normalisée.

-- ============================================================================
-- CODE DE RÉFÉRENCE (NORMALISÉ ET FONCTIONNEL)
-- ============================================================================

-- Table des clients (entité séparée, normalisée)
CREATE TABLE IF NOT EXISTS clients (
    client_id INTEGER PRIMARY KEY AUTOINCREMENT,
    nom TEXT NOT NULL,
    contact TEXT NOT NULL,
    adresse TEXT NOT NULL
);

-- Table des robots (entité séparée, normalisée)
-- Reliée au client par une clé étrangère
CREATE TABLE IF NOT EXISTS robots (
    robot_id INTEGER PRIMARY KEY AUTOINCREMENT,
    modele TEXT NOT NULL,
    numero_serie TEXT NOT NULL UNIQUE,
    statut TEXT NOT NULL DEFAULT 'operationnel',
    client_id INTEGER NOT NULL,

    FOREIGN KEY (client_id) REFERENCES clients(client_id),
    CHECK (statut IN ('operationnel', 'en_maintenance', 'hors_service'))
);

-- ============================================================================
-- BONS DE TRAVAIL (PREUVE DE CONCEPT)
-- ============================================================================
-- PREUVE DE CONCEPT — à arrimer et à normaliser.
-- Cette table n'est reliée à rien: elle désigne le robot par son numéro de
-- série (TEXT), sans clé étrangère vers la table robots. Elle n'est pas non
-- plus normalisée: selon le type de bon, beaucoup de colonnes restent NULL.
CREATE TABLE IF NOT EXISTS bons_travail (
    bon_id INTEGER PRIMARY KEY AUTOINCREMENT,
    numero_serie TEXT NOT NULL,  -- PREUVE DE CONCEPT: pas de clé étrangère
    type_bon TEXT NOT NULL,
    date_creation TEXT NOT NULL,
    statut TEXT NOT NULL DEFAULT 'ouvert',

    -- Colonnes spécifiques selon le type (beaucoup de NULL!)
    symptomes TEXT,           -- Pour diagnostic
    diagnostic TEXT,          -- Pour diagnostic
    version_actuelle TEXT,    -- Pour mise à jour
    version_cible TEXT,       -- Pour mise à jour
    mise_a_jour_reussie INTEGER,  -- Pour mise à jour (0/1/NULL)
    composant TEXT,           -- Pour réparation
    probleme TEXT,            -- Pour réparation

    FOREIGN KEY (numero_serie) REFERENCES robots(numero_serie),
    CHECK (type_bon IN ('diagnostic', 'mise_a_jour', 'reparation')),
    CHECK (statut IN ('ouvert', 'en_cours', 'termine', 'annule'))
);

CREATE TABLE IF NOT EXISTS pieces (
    piece_id INTEGER PRIMARY KEY AUTOINCREMENT,
    nom TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS liste_pieces (
    bon_id INTEGER NOT NULL,
    piece_id INTEGER NOT NULL,
    quantite INTEGER NOT NULL,
    
    PRIMARY KEY (bon_id, piece_id),
    FOREIGN KEY (bon_id) REFERENCES bons_travail(bon_id),
    FOREIGN KEY (piece_id) REFERENCES pieces(piece_id)
);

-- Index pour améliorer les performances
CREATE INDEX IF NOT EXISTS idx_robots_client ON robots(client_id);
CREATE INDEX IF NOT EXISTS idx_bons_numero_serie ON bons_travail(numero_serie);
CREATE INDEX IF NOT EXISTS idx_bons_statut ON bons_travail(statut);
