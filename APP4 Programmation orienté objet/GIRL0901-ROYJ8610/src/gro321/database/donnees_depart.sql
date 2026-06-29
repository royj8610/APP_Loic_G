-- Données d'exemple pour la base de départ
--
-- Les clients et les robots sont normalisés: chaque client n'apparait qu'une
-- seule fois et chaque robot reference son client par client_id.

-- ============================================================================
-- CLIENTS ET ROBOTS (CODE DE RÉFÉRENCE NORMALISÉ)
-- ============================================================================

-- Insertion des clients (une seule fois chacun)
INSERT INTO clients (client_id, nom, contact, adresse) VALUES
    (1, 'Restaurant Le Gourmet', '514-555-1234', '123 rue Principale, Montréal'),
    (2, 'Hôtel Plaza', '418-555-5678', '456 boulevard Royal, Québec'),
    (3, 'Café Central', '450-555-9012', '789 avenue Centrale, Laval');

-- Insertion des robots avec référence au client
INSERT INTO robots (robot_id, modele, numero_serie, statut, client_id) VALUES
    (1, 'ServBot-2000', 'SN-2024-001', 'operationnel', 1),
    (2, 'ServBot-2000', 'SN-2024-002', 'operationnel', 1),
    (3, 'ServBot-3000', 'SN-2024-003', 'en_maintenance', 1),
    (4, 'ServBot-2000', 'SN-2024-004', 'operationnel', 2),
    (5, 'ServBot-3000', 'SN-2024-005', 'operationnel', 2),
    (6, 'ServBot-2000', 'SN-2024-006', 'hors_service', 3);

-- ============================================================================
-- BONS DE TRAVAIL (PREUVE DE CONCEPT, NON NORMALISÉ)
-- ============================================================================
-- Le robot est désigné par son numéro de série (pas de clé étrangère).
-- Beaucoup de colonnes sont NULL selon le type de bon.

-- Bon de diagnostic
INSERT INTO bons_travail (numero_serie, type_bon, date_creation, statut, symptomes, diagnostic) VALUES
    ('SN-2024-003', 'diagnostic', '2024-06-01 10:30:00', 'termine',
     'Le robot tourne en rond et ne suit pas les trajectoires',
     'Capteur LIDAR avant défectueux');

-- Bons de mise à jour
INSERT INTO bons_travail (numero_serie, type_bon, date_creation, statut, version_actuelle, version_cible, mise_a_jour_reussie) VALUES
    ('SN-2024-001', 'mise_a_jour', '2024-06-05 14:00:00', 'termine', 'v1.2.3', 'v2.0.0', 1),
    ('SN-2024-002', 'mise_a_jour', '2024-06-05 14:30:00', 'en_cours', 'v1.2.3', 'v2.0.0', NULL);

-- Bon de réparation
INSERT INTO bons_travail (numero_serie, type_bon, date_creation, statut, composant, probleme) VALUES
    ('SN-2024-006', 'reparation', '2024-06-03 09:00:00', 'termine',
     'Moteur roue gauche',
     'Moteur ne répond plus aux commandes');

-- Autres bons en cours
INSERT INTO bons_travail (numero_serie, type_bon, date_creation, statut, symptomes) VALUES
    ('SN-2024-004', 'diagnostic', '2024-06-08 11:00:00', 'ouvert', 'Bruit anormal lors des virages');

INSERT INTO bons_travail (numero_serie, type_bon, date_creation, statut, composant, probleme) VALUES
    ('SN-2024-005', 'reparation', '2024-06-07 16:00:00', 'en_cours', 'Batterie', 'Autonomie réduite de 50%');

INSERT INTO pieces (piece_id, nom) VALUES
    (1, 'Moteur NEMA-17'),
    (2, 'Courroie');

INSERT INTO liste_pieces (bon_id, piece_id, quantite) VALUES
    (4, 1, 1),  -- Bon de réparation pour SN-2024-006 utilise 1 Moteur NEMA-17
    (4, 2, 1);  -- Bon de réparation pour SN-2024-006 utilise 1 Courroie
