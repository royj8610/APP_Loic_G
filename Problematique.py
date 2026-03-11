import numpy as np
import matplotlib.pyplot as plt
 
 
# Dimensions du robot
 
l1 = 0.15
l2x = 0.05
l2y = 0.1
l3 = 0.5
l4x = 0.1
l4y = 0.02
l5 = 0.3
l6 = 0.02
 
# Dimensions de la pièce
 
hg_nom = 0.1
hd_nom = 0.05
lb_nom = 0.15
 
# Tolérances
 
tol_angle_deg = 0.2
tol_distance = 0.001
 
# Position de la caméra
 
lx_cam = 0.8
ly_cam = 0.7
 
#position de la piece
 
lpx = 0.5994
lpz = 0.1991
 
# Position angulaires des joints
 
q_manufacturier = np.array([0.1,np.pi/4,0.1,0.1,0.1,0.1])
q_ramasse_piece = np.array([-0.4,-1.2,0,0,-0.3708,0])
q_inspection = np.array([0,-0.3,0,0,0.5,-1.6])
 
# Points sur la tranche
TI = np.array([
    [0.158920, 0.013914, 0.028686],
    [0.157470, 0.021067, 0.008891],
    [0.153781, 0.039266, -0.040587],
    [0.152420, 0.045970, -0.060395],
    [0.150931, 0.053326, -0.080185]
])
 
# Défauts
DF = np.array([
    [0.153758, 0.039379, -0.020575],
    [0.145698, 0.079138, -0.039398],
    [0.153932, 0.038521, 0.009411],
    [0.152097, 0.047573, 0.035692],
    [0.146104, 0.077134, 0.030571]
])
 
 
# Fonctions de rotations
 
def R_1(theta):
    c = np.cos(theta)
    s = np.sin(theta)
    return np.array([[1,0,0],[0,c,-s],[0,s,c]])
 
def R_2(theta):
    c = np.cos(theta)
    s = np.sin(theta)
    return np.array([[c,0,s],[0,1,0],[-s,0,c]])
 
def R_3(theta):
    c = np.cos(theta)
    s = np.sin(theta)
    return np.array([[c,-s,0],[s,c,0],[0,0,1]])
 
# Calcul de la cinématique directe
 
def cinematique_directe(angles):
 
    theta1,theta2,theta3,theta4,theta5,theta6 = angles
 
    # Matrices de rotations
 
    wRa = R_2(theta1)
    aRb = R_3(theta2)
    bRc = R_3(theta3)
    cRd = R_1(theta4)
    dRe = R_3(theta5)
 
    # Vecteurs colonnes de translation
 
    _r_w_W0_A0 = np.array([0,l1,0])
    _r_a_A0_B0 = np.array([l2x,l2y,0])
    _r_b_B0_C0 = np.array([0,l3,0])
    _r_c_C0_D0 = np.array([l4x,l4y,0])
    _r_d_D0_E0 = np.array([l5,0,0])
    _r_e_E0_T0 = np.array([l6,0,0])
 
    # Vecteurs du robot
 
    rW = np.array([0,0,0])
    rA = rW + _r_w_W0_A0
    rB = rA + wRa @ _r_a_A0_B0
    rC = rB + (wRa @ aRb) @ _r_b_B0_C0
    rD = rC + (wRa @ aRb @ bRc) @ _r_c_C0_D0
    rE = rD + (wRa @ aRb @ bRc @ cRd) @ _r_d_D0_E0
    rT = rE + (wRa @ aRb @ bRc @ cRd @ dRe) @ _r_e_E0_T0
 
    return np.array([rW,rA,rB,rC,rD,rE,rT])
 
# Fonction d'extraction des coordonnées du tool
 
def position_tool (angles):
    return cinematique_directe(angles)[-1]
 
# Matrice de rotation 3x3 de T0 dans le repère monde"""
 
def rotation_tool(angles):
   
    theta1,theta2,theta3,theta4,theta5,theta6 = angles
    wRa = R_2(theta1)
    aRb = R_3(theta2)
    bRc = R_3(theta3)
    cRd = R_1(theta4)
    dRe = R_3(theta5)
    eRt = R_1(theta6)
    return wRa @ aRb @ bRc @ cRd @ dRe @ eRt
 
# Fonction d'affichage graphiq des joints du robot
 
def afficher_robot(angles):
 
    positions = cinematique_directe(angles)
 
    fig = plt.figure()
    ax = fig.add_subplot(projection='3d')
 
    # Boucle sur tous les points pour les placer et les relier
    for i in range(len(positions)):
        W_1 = positions[i, 0]  
        W_2 = positions[i, 2]  
        W_3 = positions[i, 1]  
        ax.scatter(W_1, W_2, W_3, color='red', s=50)
 
        # relier le point précédent au point actuel
        if i > 0:
           
            W1_prev = positions[i-1, 0]
            W2_prev = positions[i-1, 2]
            W3_prev = positions[i-1, 1]
 
            ax.plot([W1_prev, W_1],
                    [W2_prev, W_2],
                    [W3_prev, W_3],
                    color='blue', linewidth=2)
 
    ax.set_xlabel('W_1')
    ax.set_ylabel('W_3')
    ax.set_zlabel('W_2')
    ax.set_title("Configuration du robot")
    plt.show()
 
# Matrice de transformation
 
def transformation(R, p):
    T = np.eye(4)
    T[:3,:3] = R
    T[:3,3] = p
    return T
 
# Transforme un point de la caméra (_r_point) dans le repère pièce
 
def camera_vers_piece(_r_point):
 
    # Pose du tool au moment du ramassage
    wRt_pick = rotation_tool(q_ramasse_piece)
    _r_w_T0_W0_pick = position_tool(q_ramasse_piece)
 
    # Position connue de P0 dans W au moment du ramassage
    _r_w_P0_W0_pick = np.array([lpx, 0, lpz])
 
    # Vecteur T0 -> P0 exprimé dans le repère t
    _r_t_T0_P0 = wRt_pick.T @ (_r_w_P0_W0_pick - _r_w_T0_W0_pick)
 
    # Rotation de la pièce par rapport au tool (p1 = t2, p2 = t3, p3 = t1)
    tRp = np.array([[0, 0, 1],[1, 0, 0],[0, 1, 0]])
 
    # Transformation du repère t vers le repère p
    tTp = transformation(tRp, _r_t_T0_P0)
 
    # Pose du tool pendant l'inspection
    wRt_inspection = rotation_tool(q_inspection)
    _r_w_W0_T0_inspection = position_tool(q_inspection)
 
    # Transformation du repère t vers le repère w pendant l'inspection
    wTt_inspection = transformation(wRt_inspection, _r_w_W0_T0_inspection)
 
    # Transformation du repère p vers le repère w pendant l'inspection
    wTp_inspection = wTt_inspection @ tTp
 
    # Caméra fixe dans le monde
    # Rotation de 180° autour de W3
    wRv = R_3(np.pi)
    _r_w_V0_W0 = np.array([lx_cam, ly_cam, 0])
    wTv = transformation(wRv, _r_w_V0_W0)
 
    # Vecteur défaut exprimé dans V0 sous forme homogène
    _r_v_point_V0_h = np.append(_r_point, 1)
 
    # Transformation du point caméra vers le repère pièce
    _r_p_point_P0 = np.linalg.inv(wTp_inspection) @ wTv @ _r_v_point_V0_h
 
    return _r_p_point_P0[:3]
 
# Transformer tous les points TI
TI_piece = np.array([camera_vers_piece(points) for points in TI])
 
# Transformer tous les défauts DF
DF_piece = np.array([camera_vers_piece(points) for points in DF])
 
def est_dans_zone_interdite(x, y):
    val = 45*x**2 + 30*x*y + 85*y**2 - 10.8*x - 8.4*y + 0.684
    return val < 0
 
def verifier_defauts():
    au_moins_un = False
 
    for i, p in enumerate(DF_piece):
        if est_dans_zone_interdite(p[0], p[1]):
            print(f"DF{i+1} dans zone interdite !")
            au_moins_un = True
        else:
            print(f"DF{i+1} hors zone interdite")
 
    if not au_moins_un:
        print("Aucun défaut trouvé dans la zone interdite.")
 
def valider_tolérances_tranche(points_tranche):
 
    # Coordonnées dans le plan p1-p2
    p1 = points_tranche[:, 0]
    p2 = points_tranche[:, 1]
 
    # Ajustement par moindres carrés
 
    A = np.column_stack((p1, np.ones(len(p1))))
    a, b = np.linalg.lstsq(A, p2, rcond=None)[0]
 
    # Validation de l'angle
   
    angle_mesure_deg = abs(np.degrees(np.arctan(a)))
    angle_nominal_deg = abs(np.degrees(np.arctan2(hd_nom - hg_nom, lb_nom)))
 
    erreur_angle_deg = angle_mesure_deg - angle_nominal_deg
    conforme_angle = abs(erreur_angle_deg) <= tol_angle_deg
 
    # Validation du début tranche
 
    distance_mesuree = b
    erreur_distance = distance_mesuree - hg_nom
    conforme_distance = abs(erreur_distance) <= tol_distance
 
    # Affichage des résultats
 
    print("Validation des tolérances de la tranche")
    print()
    print("Validation de l'angle :")
    print(f"Angle mesuré          : {angle_mesure_deg:.4f} deg")
    print(f"Angle nominal         : {angle_nominal_deg:.4f} deg")
    print(f"Erreur angle          : {erreur_angle_deg:.4f} deg")
    print(f"Tolérance angle       : ±{tol_angle_deg:.4f} deg")
    print(f"Conforme angle        : {conforme_angle}")
    print()
    print("Validation du début de la tranche :")
    print(f"Distance mesurée      : {distance_mesuree:.6f} m")
    print(f"Distance nominale     : {hg_nom:.6f} m")
    print(f"Erreur distance       : {erreur_distance:.6f} m")
    print(f"Tolérance distance    : ±{tol_distance:.6f} m")
    print(f"Conforme distance     : {conforme_distance}")
    print()
       
def afficher_piece_et_points(points_tranche, points_défauts):
 
    fig, ax = plt.subplots()
 
    # Contour nominal de la pièce dans le plan p1-p2
    piece_p1 = [0, 0, lb_nom, lb_nom, 0]
    piece_p2 = [0, hg_nom, hd_nom, 0, 0]
 
    ax.plot(piece_p1, piece_p2, 'k-', linewidth=2, label="Contour pièce nominal")
 
    p1 = points_tranche[:,0]
    p2 = points_tranche[:,1]
 
    ax.scatter(p1, p2, color='blue', label="Points TI")
 
    A = np.column_stack((p1, np.ones(len(p1))))
    a, b = np.linalg.lstsq(A, p2, rcond=None)[0]
 
    x_line = np.linspace(min(p1), max(p1), 100)
    y_line = a*x_line + b
 
    ax.plot(x_line, y_line, 'b--', linewidth=2, label="Tranche moyenne")
 
    ax.scatter(points_défauts[:,0], points_défauts[:,1],
               marker='x', s=80, color='red', label="Défauts DF")
 
    for i, p in enumerate(points_tranche):
        ax.text(p[0], p[1], f"TI{i+1}", fontsize=9)
 
    for i, p in enumerate(points_défauts):
        ax.text(p[0], p[1], f"DF{i+1}", fontsize=9)
 
    ax.set_xlabel("p1 (longueur)")
    ax.set_ylabel("p2 (hauteur)")
    ax.set_title("Pièce et tranche moyenne mesurée")
    ax.axis("equal")
    ax.grid(True)
    ax.legend()
 
    plt.show()
 


# -----------------------------
# Jacobienne du point E0
# -----------------------------

def jacobienne_min_norme(angles):

    #Constante diemensions robot
    l3 = 0.5
    l4x = 0.1
    l4y = 0.02
    l5 = 0.3

    #Angles des joint 2 et 3
    theta2 = angles[1]
    theta3 = angles[2]

    #Dérivé de h
    h_theta2 = -l3*np.sin(theta2) - l4y*np.sin(theta2+theta3) + (l4x+l5)*np.cos(theta2+theta3)
    h_theta3 = -l4y*np.sin(theta2+theta3) + (l4x+l5)*np.cos(theta2+theta3)

    #Matrice Jacobienne
    J = np.array([[0, h_theta2, h_theta3, 0, 0, 0]])


    return J

def jacobienne_mouv_verticale(angles):

    #Constante diemensions robot
    l3 = 0.5
    l4x = 0.1
    l4y = 0.02
    l5 = 0.3

    #Angles des joint 2 et 3
    theta2 = angles[1]
    theta3 = angles[2]

    #Dérivé de h
    h_theta2 = -l3*np.sin(theta2) - l4y*np.sin(theta2+theta3) + (l4x+l5)*np.cos(theta2+theta3)
    h_theta3 = -l4y*np.sin(theta2+theta3) + (l4x+l5)*np.cos(theta2+theta3)

    #Dérivé de r
    r_theta2 = -l3*np.cos(theta2) - l4y*np.cos(theta2+theta3) - (l4x+l5)*np.sin(theta2+theta3)
    r_theta3 = -l4y*np.cos(theta2+theta3) - (l4x+l5)*np.sin(theta2+theta3)

    #Matrice Jacobienne
    J = np.array([[0, h_theta2, h_theta3, 0, 0, 0],
                  [0, r_theta2, r_theta3, 0, 0, 0]])


    return J


# -----------------------------
# Solution a) : minimiser la norme des vitesses des joints
# -----------------------------
def solution_min_norme(angles, v):
    """
    Calcule les vitesses angulaires des joints 1-2-3
    pour obtenir une vitesse du point E0 donnée 'v'
    en minimisant la norme totale des vitesses des joints.
    """
    
    # Calcul de la Jacobienne
    J = jacobienne_min_norme(angles)

    # vitesse verticale
    vz = np.array([v])   

    # Résolution via pseudo-inverse 6x1 @ 1x1 donne 6x1
    Vit_joint = np.linalg.pinv(J) @ vz

    return Vit_joint


# -----------------------------
# Solution b) : mouvement purement vertical
# -----------------------------
def solution_verticale(angles, vz):
    """
    Calcule les vitesses angulaires des joints 1-2-3
    pour obtenir un mouvement purement vertical
    du point E0 (vitesse v_y = 1 m/s, vx=vz=0)
    """
    
# Calcul de la Jacobienne
    J = jacobienne_mouv_verticale(angles)

    # Création du vecteur vitesse 2x1 pour un mouvement vertical
    v = np.array([[vz],
                  [0]])

    # Calcul des vitesses des joints via pseudo-inverse 6x2 @ 2x1 donne 6x1
    Vit_joint = np.linalg.pinv(J) @ v

    # Retourne un vecteur 1D
    return Vit_joint

# -----------------------------
# Solution c) : mouvement vertical uniquement avec le joint 2
# -----------------------------

#Impossible d'obtenir un mouvement purement vertical avec seulement 1 joint



# -----------------------------
# Configurations de test
# -----------------------------
qT1 = np.array([-0.4,-1.2,0,0,-0.3708,0])  # Configuration 1
qT2 = np.array([0,0,1.521,0,0,0])          # Configuration 2
v = 1                       # Vitesse verticale désirée

# -----------------------------
# Exécution des tests
# -----------------------------
print("")
print("----------- Configuration 1 -----------\n")

Solution=solution_min_norme(qT1,v)
print("a) Min vitesse total joint en Rad/s")
for i, val in enumerate(Solution):
    print(f"theta{i+1} = {val}")
print("")

Solution=solution_verticale(qT1,1)
print("b) Mouvement purement vertical")
for i, val in enumerate(Solution):
    print(f"theta{i+1} = {0 if val<0.001 and -0.001<val else val}")
print("")

print("c) Mouvement purement vertical seulement joint 2")
print("Impossible!")
print("")

print("----------- Configuration 2 -----------\n")

Solution=solution_min_norme(qT2,v)
print("a) Min vitesse total joint en Rad/s")
for i, val in enumerate(Solution):
    print(f"theta{i+1} = {val}")
print("")

Solution=solution_verticale(qT2, 1)
print("b) Mouvement purement vertical")
for i, val in enumerate(Solution):
    print(f"theta{i+1} = {0 if val<0.001 and -0.001<val else val}")
print("")


print("c) Mouvement purement vertical seulement joint 2")
print("Impossible!")
print("")

valider_tolérances_tranche(TI_piece)
verifier_defauts()
afficher_piece_et_points(TI_piece, DF_piece)
afficher_robot(q_manufacturier)