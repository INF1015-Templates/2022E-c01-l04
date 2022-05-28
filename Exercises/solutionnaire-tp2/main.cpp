#include "Jeu.hpp"
#include <iostream>
#include <fstream>
#include <cassert>
#include "cppitertools/range.hpp"
#include "gsl/span"
#include "bibliotheque_cours.hpp"
#include "verification_allocation.hpp"
#include "ListeDeveloppeurs.hpp"
#include "debogage_memoire.hpp"  //NOTE: Si on veut le numéro de ligne où l'allocation est faite, il faut ajouter ce include dans chaque .cpp qui utilise "new".  Attention que c'est incompatible avec le "placement new", ne pas utiliser cette entête si vous utilisez ce type de "new" dans les lignes qui suivent cette inclusion.

using namespace std;
using namespace iter;
using namespace gsl;

#pragma region "Fonctions de base pour vous aider"
typedef uint8_t UInt8;
typedef uint16_t UInt16;
UInt8 lireUint8(istream& fichier)
{
	UInt8 valeur = 0;
	fichier.read((char*)&valeur, sizeof(valeur));
	return valeur;
}
UInt16 lireUint16(istream& fichier)
{
	UInt16 valeur = 0;
	fichier.read((char*)&valeur, sizeof(valeur));
	return valeur;
}
string lireString(istream& fichier)
{
	string texte;
	texte.resize(lireUint16(fichier));
	fichier.read((char*)&texte[0], streamsize(sizeof(texte[0])) * texte.length());
	return texte;
}
gsl::span<Jeu*> spanListeJeux(const ListeJeux& liste)
{
	return gsl::span(liste.elements, liste.nElements);
}
gsl::span<Designer*> spanListeDesigners(const ListeDesigners& liste)
{
	return gsl::span(liste.elements, liste.nElements);
}
#pragma endregion

//TODO: Fonction qui cherche un designer par son nom dans une ListeJeux.
// Cette fonction renvoie le pointeur vers le designer si elle le trouve dans
// un des jeux de la ListeJeux. En cas contraire, elle renvoie un pointeur nul.
Designer* trouverDesigner(const ListeJeux& listeJeux, string nom)
{
	for (const Jeu* j : spanListeJeux(listeJeux)) {
		// Normalement on voudrait retourner un pointeur const, mais cela nous
		// empêcherait d'affecter le pointeur retourné lors de l'appel de cette
		// fonction.
		for (Designer* d : spanListeDesigners(j->designers)) {
			if (d->nom == nom)
				return d;
		}
	}
	return nullptr;
}


Designer* lireDesigner(istream& fichier, ListeJeux& listeJeux)
{
	Designer designer = {}; // On initialise une structure vide de type Designer.
	designer.nom = lireString(fichier);
	designer.anneeNaissance = lireUint16(fichier);
	designer.pays = lireString(fichier);
	// Rendu ici, les champs précédents de la structure designer sont remplis
	// avec la bonne information.

	//TODO: Ajouter en mémoire le designer lu. Il faut revoyer le pointeur créé.
	// Attention, valider si le designer existe déjà avant de le créer, sinon
	// on va avoir des doublons car plusieurs jeux ont des designers en commun
	// dans le fichier binaire. Pour ce faire, cette fonction aura besoin de
	// la liste de jeux principale en paramètre.
	// Afficher un message lorsque l'allocation du designer est réussie.
	Designer* designerExistant = trouverDesigner(listeJeux, designer.nom);
	if (designerExistant != nullptr)
		return designerExistant;

	//cout << designer.nom << endl;  //TODO: Enlever cet affichage temporaire servant à voir que le code fourni lit bien les jeux.
	cout << "\033[92m" << "Allocation en mémoire du designer " << designer.nom
				<< "\033[0m" << endl;
	return new Designer(designer); //TODO: Retourner le pointeur vers le designer crée.
}

//TODO: Fonction qui change la taille du tableau de jeux de ListeJeux.
// Cette fonction doit recevoir en paramètre la nouvelle capacité du nouveau
// tableau. Il faut allouer un nouveau tableau assez grand, copier ce qu'il y
// avait dans l'ancien, et éliminer l'ancien trop petit. N'oubliez pas, on copie
// des pointeurs de jeux. Il n'y a donc aucune nouvelle allocation de jeu ici !
void changerTailleListeJeux(ListeJeux& liste, unsigned nouvelleCapacite)
{
	assert(nouvelleCapacite >= liste.nElements); // On ne demande pas de supporter les réductions de nombre d'éléments.
	Jeu** nouvelleListeJeux = new Jeu* [nouvelleCapacite];
	// Pas nécessaire de tester si liste.elements est nullptr puisque si c'est le cas, nElements est nécessairement 0.
	for(int i : iter::range(liste.nElements))
		nouvelleListeJeux[i] = liste.elements[i];
	delete[] liste.elements;

	liste.elements = nouvelleListeJeux;
	liste.capacite = nouvelleCapacite;
}

//TODO: Fonction pour ajouter un Jeu à ListeJeux.
// Le jeu existant déjà en mémoire, on veut uniquement ajouter le pointeur vers
// le jeu existant. De plus, en cas de saturation du tableau elements, cette
// fonction doit doubler la taille du tableau elements de ListeJeux.
// Utilisez la fonction pour changer la taille du tableau écrite plus haut.
void ajouterJeu(ListeJeux& liste, Jeu* jeu)
{
	if(liste.nElements == liste.capacite)
		changerTailleListeJeux(liste, max(1U, liste.capacite * 2));
	liste.elements[liste.nElements++] = jeu;
}

//TODO: Fonction qui enlève un jeu de ListeJeux.
// Attention, ici il n'a pas de désallocation de mémoire. Elle enlève le
// pointeur de la ListeJeux, mais le jeu pointé existe encore en mémoire.
// Puisque l'ordre de la ListeJeux n'a pas être conservé, on peut remplacer le
// jeu à être retiré par celui présent en fin de liste et décrémenter la taille
// de celle-ci.
void enleverJeu(ListeJeux& liste, const Jeu* jeu)
{
	for (Jeu*& elem : spanListeJeux(liste)) {
		if (elem == jeu) {
			if (liste.nElements > 1)
				elem = liste.elements[liste.nElements - 1];
			liste.nElements--;
		}
	}
}

Jeu* lireJeu(istream& fichier, ListeJeux& listeJeux)
{
	Jeu jeu = {}; // On initialise une structure vide de type Jeu
	jeu.titre = lireString(fichier);
	jeu.anneeSortie = lireUint16(fichier);
	jeu.developpeur = lireString(fichier);
	jeu.designers.nElements = lireUint8(fichier);
	// Rendu ici, les champs précédents de la structure jeu sont remplis avec la
	// bonne information.

	//TODO: Ajouter en mémoire le jeu lu. Il faut revoyer le pointeur créé.
	// Attention, il faut aussi créer un tableau dynamique pour les designers
	// que contient un jeu. Servez-vous de votre fonction d'ajout de jeu car la
	// liste de jeux participé est une ListeJeu. Afficher un message lorsque
	// l'allocation du jeu est réussie.
	Jeu* ptrJeu = new Jeu(jeu);  // Ou allouer directement au début plutôt qu'en faire une copie ici.
	cout << "\033[96m" << "Allocation en mémoire du jeu " << jeu.titre
			  << "\033[0m" << endl;
	// cout << jeu.titre << endl;  //TODO: Enlever cet affichage temporaire servant à voir que le code fourni lit bien les jeux.
	ptrJeu->designers.elements = new Designer* [ptrJeu->designers.nElements];  // On n'a pas demandé de faire une réallocation dynamique pour les designers.
	for (Designer*& d : spanListeDesigners(ptrJeu->designers)) {
		d = lireDesigner(fichier, listeJeux);  //TODO: Mettre le designer dans la liste des designer du jeu.
		ajouterJeu(d->listeJeuxParticipes, ptrJeu); //TODO: Ajouter le jeu à la liste des jeux auquel a participé le designer.
	}
	return ptrJeu; //TODO: Retourner le pointeur vers le nouveau jeu.
}

ListeJeux creerListeJeux(const string& nomFichier)
{
	ifstream fichier(nomFichier, ios::binary);
	fichier.exceptions(ios::failbit);
	int nElements = lireUint16(fichier);
	ListeJeux listeJeux = {};
	for([[maybe_unused]] int n : iter::range(nElements))
	{
		ajouterJeu(listeJeux, lireJeu(fichier, listeJeux)); //TODO: Ajouter le jeu à la ListeJeux.
	}

	return listeJeux; //TODO: Renvoyer la ListeJeux.
}

//TODO: Fonction pour détruire un designer (libération de mémoire allouée).
// Lorsqu'on détruit un designer, on affiche son nom pour fins de débogage.
void detruireDesigner(Designer* d)
{
	cout << "\033[91m" << "Destruction du designer " << d->nom << "\033[0m"
			  << endl;
	delete[] d->listeJeuxParticipes.elements;
	delete d;
}

//TODO: Fonction qui détermine si un designer participe encore à un jeu.
bool encorePresentDansUnJeu(const Designer* d)
{
	return d->listeJeuxParticipes.nElements != 0;
}

//TODO: Fonction pour détruire un jeu (libération de mémoire allouée).
// Attention, ici il faut relâcher toute les cases mémoires occupées par un jeu.
// Par conséquent, il va falloir gérer le cas des designers (un jeu contenant
// une ListeDesigners). On doit commencer par enlever le jeu à détruire des jeux
// qu'un designer a participé (listeJeuxParticipes). Si le designer n'a plus de
// jeux présents dans sa liste de jeux participés, il faut le supprimer.  Pour
// fins de débogage, affichez le nom du jeu lors de sa destruction.
void detruireJeu(Jeu* jeu)
{
	for (Designer* d : spanListeDesigners(jeu->designers)) {
		enleverJeu(d->listeJeuxParticipes, jeu);
		if (!encorePresentDansUnJeu(d))
			detruireDesigner(d);
	}
	cout << "\033[31m" << "Destruction du jeu " << jeu->titre << "\033[0m"
			  << endl;
	delete[] jeu->designers.elements;
	delete jeu;
}

//TODO: Fonction pour détruire une ListeJeux et tous ses jeux.
void detruireListeJeux(ListeJeux& liste)
{
	for(Jeu* j : spanListeJeux(liste))
		detruireJeu(j);
	delete[] liste.elements;
}

void afficherDesigner(const Designer& d)
{
	cout << "\t" << d.nom << ", " << d.anneeNaissance << ", " << d.pays
			  << endl;
}

//TODO: Fonction pour afficher les infos d'un jeu ainsi que ses designers.
// Servez-vous de la fonction afficherDesigner ci-dessus.
void afficherJeu(const Jeu& j)
{
	cout << "Titre : " << "\033[94m" << j.titre << "\033[0m" << endl;
	cout << "Parution : " << "\033[94m" << j.anneeSortie << "\033[0m"
			  << endl;
	cout << "Développeur :  " << "\033[94m" << j.developpeur << "\033[0m"
			  << endl;
	cout << "Designers du jeu :" << "\033[94m" << endl;
	for(const Designer* d : spanListeDesigners(j.designers))
		afficherDesigner(*d);
	cout << "\033[0m";
}

//TODO: Fonction pour afficher tous les jeux de ListeJeux, séparés par un ligne.
// Servez-vous de la fonction d'affichage d'un jeu crée ci-dessus. Votre ligne
// de séparation doit être différent de celle utilisée dans le main.
void afficherListeJeux(const ListeJeux& listeJeux)
{
	static const string ligneSeparation = "\n\033[95m"
	"══════════════════════════════════════════════════════════════════════════"
	"\033[0m\n";
	cout << ligneSeparation << endl;
	for(const Jeu* j : spanListeJeux(listeJeux))
	{
		afficherJeu(*j);
		cout << ligneSeparation << endl;
	}
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	#pragma region "Bibliothèque du cours"
	// Permet sous Windows les "ANSI escape code" pour changer de couleur
	// https://en.wikipedia.org/wiki/ANSI_escape_code ; les consoles Linux/Mac
	// les supportent normalement par défaut.
	bibliotheque_cours::activerCouleursAnsi(); 
	#pragma endregion

	//int* fuite = new int;  // Pour vérifier que la détection de fuites fonctionne; un message devrait dire qu'il y a une fuite à cette ligne.

	ListeJeux lj = creerListeJeux("jeux.bin"); //TODO: Appeler correctement votre fonction de création de la liste de jeux.

	static const string ligneSeparation = "\n\033[35m════════════════════════════════════════\033[0m\n";
	cout << ligneSeparation << endl;
	cout << "Premier jeu de la liste :" << endl;
	//TODO: Afficher le premier jeu de la liste (en utilisant la fonction).  Devrait être Chrono Trigger.
	afficherJeu(*lj.elements[0]);
	cout << ligneSeparation << endl;

	//TODO: Appel à votre fonction d'affichage de votre liste de jeux.
	afficherListeJeux(lj);
	
	//TODO: Faire les appels à toutes vos fonctions/méthodes pour voir qu'elles fonctionnent et avoir 0% de lignes non exécutées dans le programme (aucune ligne rouge dans la couverture de code; c'est normal que les lignes de "new" et "delete" soient jaunes).  Vous avez aussi le droit d'effacer les lignes du programmes qui ne sont pas exécutée, si finalement vous pensez qu'elle ne sont pas utiles.

	// PARTIE 2
	ListeDeveloppeurs ld;
	// Création des développeurs externes à la liste des développeur
	Developpeur* nintendo = new Developpeur("Nintendo");
	Developpeur* square = new Developpeur("Square");
	Developpeur* bidon = new Developpeur("Bidon");
	Developpeur* konami = new Developpeur("Konami");
	// On ajoute les jeux respectifs de ListeJeux développé par le développeur
	nintendo->ajouterJeux(lj);
	square->ajouterJeux(lj);
	konami->ajouterJeux(lj);
	// On ajoute les développeurs à la ListeDeveloppeur car ils sont externes
	ld.ajouter(nintendo);
	ld.ajouter(square);
	ld.ajouter(konami);
	ld.ajouter(bidon);
	// On affiche la liste des développeurs, leurs jeux sont aussi affichés; Bidon ne devrait avoir aucun jeu.
	ld.afficher();

	cout << endl << "On retire " << bidon->getNom() << endl;
	ld.retirer(bidon); // Retire sans détruire.
	ld.afficher();
	cout << "Il existe encore: " << bidon->getNom() << endl;

	delete nintendo;
	delete square;
	delete konami;
	delete bidon;

	//TODO: Détruire tout avant de terminer le programme.  Devrait afficher "Aucune fuite detectee." a la sortie du programme; il affichera "Fuite detectee:" avec la liste des blocs, s'il manque des delete.
	detruireListeJeux(lj);
}
