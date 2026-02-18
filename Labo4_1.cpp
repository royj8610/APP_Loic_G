#include <iostream>
#include <cstdint>

int main(int argc, char* argv[]) {
    int16_t valeur = 0x1234;
    int16_t* ptr = &valeur;  // ptr pointe vers valeur

    std::cout << "Valeur : " << valeur << std::endl;
    std::cout << "Adresse de valeur : " << ptr << std::endl;
    std::cout << "Valeur via pointeur : " << *ptr << std::endl;

    // Modification via le pointeur
    *ptr = 5678;
    std::cout << "Nouvelle valeur : " << valeur << std::endl;

    // TODO: Déclarez un tableau de 4 uint8_t initialisé à {0x12, 0x34, 0x56, 0x78}
    uint8_t tableau[4]={0x12, 0x34, 0x56, 0x78};
    uint8_t* pointeur = &tableau[0];  // pointeur vers le premier élément

    // Affichez l'adresse de chaque élément du tableau (indice: vous aurez normalement besoin d'un "cast")
    
    for (int i = 0; i < 4; i++) {
        std::cout << "Adresse de l'élément " << i << " : " << (void*)&tableau[i] << std::endl;
    }

    // Observez : les adresses sont-elles consécutives ?

    return 0;
}

/*
a) Remplacez le type des données du tableau par un type plus grand (par exemple uint32_t).
 Que se passe-t-il aux adresses des éléments ?

b) Lancez le programme avec le débogueur en ajoutant un point d’arrêt (breakpoint) sur la dernière ligne.
 Parcourez le tableau "Variables" et comparez avec ce que vous affichez dans le terminal.
Est-ce que les adresses correspondent ? Si ce n’est pas le cas, vérifiez comment vous faites votre affichage ou conversion de type.
*/
