"""
Module CRUD pour les bons de travail - VERSION ÉTUDIANT.

OBJECTIF:
Implémenter les opérations CRUD pour les bons de travail en vous inspirant
de crud_clients.py et crud_robots.py qui sont fournis comme référence (code
complet du collègue).

La table bons_travail est une preuve de concept: elle n'est pas normalisée et
elle désigne le robot par son numéro de série (numero_serie), sans clé
étrangère vers la table robots. Un exercice supplémentaire sera de l'arrimer au
reste de la base et de la normaliser.
"""

from datetime import datetime

from ..models import BonTravail
from .connexion import get_connection


def lister_bons(statut=None, numero_serie=None, db_path="data/gro321.db"):
    """
    Liste les bons de travail avec filtres optionnels.

    Fonction fournie comme exemple de requête SELECT.

    Args:
        statut: Filtre par statut (optionnel)
        numero_serie: Filtre par numéro de série du robot (optionnel)
        db_path: Chemin vers la base de données

    Returns:
        Liste de dictionnaires contenant les informations des bons
    """
    with get_connection(db_path) as conn:
        cursor = conn.cursor()

        query = "SELECT * FROM bons_travail WHERE 1=1"
        params = []

        if statut:
            query += " AND statut = ?"
            params.append(statut)

        if numero_serie:
            query += " AND numero_serie = ?"
            params.append(numero_serie)

        query += " ORDER BY date_creation DESC"

        cursor.execute(query, params)

        bons = []
        for row in cursor.fetchall():
            bons.append(dict(row))

        return bons


def creer_bon_diagnostic(numero_serie, symptomes, db_path="data/gro321.db"):
    """
    Crée un nouveau bon de diagnostic dans la base de données.

    type_bon: 'diagnostic'

    Note : Vous pouvez obtenir la date/heure actuelle avec :
      datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    pour le champ "date_creation".
    
    Args:
        numero_serie: Numéro de série du robot concerné
        symptomes: Description des symptômes
        db_path: Chemin vers la base de données

    Returns:
        ID du bon créé
    """
    # TODO: Implémenter cette fonction

    with get_connection(db_path) as conn:
        cursor = conn.cursor()
        date_creation = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        cursor.execute(
            """
            INSERT INTO bons_travail (type_bon, numero_serie, symptomes, date_creation, statut)
            VALUES (?, ?, ?, ?, ?)
        """,
            ('diagnostic', numero_serie, symptomes, date_creation, 'en_attente'),
        )
        return cursor.lastrowid


def creer_bon_mise_a_jour(
    numero_serie, version_actuelle, version_cible, db_path="data/gro321.db"
):
    """
    Crée un nouveau bon de mise à jour dans la base de données.

    type_bon: 'mise_a_jour'

    Args:
        numero_serie: Numéro de série du robot concerné
        version_actuelle: Version logicielle actuelle
        version_cible: Version logicielle cible
        db_path: Chemin vers la base de données

    Returns:
        ID du bon créé
    """
    # TODO: Implémenter cette fonction
    pass


def creer_bon_reparation(numero_serie, composant, probleme, db_path="data/gro321.db"):
    """
    Crée un nouveau bon de réparation dans la base de données.

    type_bon = 'reparation'
  
    Args:
        numero_serie: Numéro de série du robot concerné
        composant: Composant à réparer
        probleme: Description du problème
        db_path: Chemin vers la base de données

    Returns:
        ID du bon créé
    """
    # TODO: Implémenter cette fonction
    pass


def lire_bon(bon_id, db_path="data/gro321.db"):
    """
    Récupère un bon de travail par son ID.

    Cet méthode nécessite d'avoir complété les classes dans bon_travail.py.

    Args:
        bon_id: Identifiant du bon
        db_path: Chemin vers la base de données

    Returns:
        Objet BonTravail (ou classe dérivée) ou None
    """
    # TODO: Implémenter cette fonction
    pass


def modifier_statut_bon(bon_id, nouveau_statut, db_path="data/gro321.db"):
    """
    Modifie le statut d'un bon de travail.

    Args:
        bon_id: Identifiant du bon
        nouveau_statut: Nouveau statut
        db_path: Chemin vers la base de données

    Returns:
        True si modifié, False sinon
    """
    # TODO: Implémenter cette fonction
    pass


def supprimer_bon(bon_id, db_path="data/gro321.db"):
    """
    Supprime un bon de travail.

    Args:
        bon_id: Identifiant du bon
        db_path: Chemin vers la base de données

    Returns:
        True si supprimé, False sinon
    """
    # TODO: Implémenter cette fonction
    pass


# Extraits des notes de cours pour référence rapide:
#
# 1. Structure générale d'une fonction CRUD:
#    with get_connection(db_path) as conn:
#        cursor = conn.cursor()
#        cursor.execute("SQL...", (params,))
#        return cursor.lastrowid  # ou autre résultat
#
# 2. Pour CREATE (INSERT):
#    cursor.execute("INSERT INTO table (col1, col2) VALUES (?, ?)", (val1, val2))
#    return cursor.lastrowid
#
# 3. Pour READ (SELECT):
#    cursor.execute("SELECT * FROM table WHERE id = ?", (id,))
#    row = cursor.fetchone()
#
# 4. Pour UPDATE:
#    cursor.execute("UPDATE table SET col = ? WHERE id = ?", (nouvelle_val, id))
#    return cursor.rowcount > 0
#
# 5. Pour DELETE:
#    cursor.execute("DELETE FROM table WHERE id = ?", (id,))
#    return cursor.rowcount > 0
#
# Consultez crud_clients.py et crud_robots.py pour des exemples complets.
