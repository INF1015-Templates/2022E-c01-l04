#include "Developpeur.hpp"
#include "Jeu.hpp"
#include "cppitertools/range.hpp"
#include <iostream>
#include "debogage_memoire.hpp"  //NOTE: Incompatible avec le "placement new", ne pas utiliser cette ent�te si vous utilisez ce type de "new" dans les lignes qui suivent cette inclusion.
using namespace std;

Developpeur::Developpeur(const string& nom)
{
	paireNomJeux_ = { nom, ListeJeux{} };
}

Developpeur::~Developpeur()
{
	delete[] paireNomJeux_.second.elements; // Les objets jeux n'appartiennent pas � la liste, donc on ne fait pas de detruireListeJeux (ou on pourrait avoir une version de detruireListeJeux qui ne d�truit pas les jeux).
}

void Developpeur::ajouterJeux(ListeJeux& listeJeux)
{
	changerTailleListeJeux(paireNomJeux_.second, compterJeuxDeveloppes(listeJeux));
	for (Jeu*& j : spanListeJeux(listeJeux)) // Ici, ListeJeux est externe � la classe
	{
		if (j->developpeur == getNom())
			ajouterJeu(paireNomJeux_.second, j);
	}
}

void Developpeur::afficher() const
{
	cout << "\n" << getNom() << " a d�velopp� les jeux suivants :" << endl;
	if (paireNomJeux_.second.nElements > 0) {
		for (Jeu*& j : spanListeJeux(paireNomJeux_.second))
			cout << "\t\033[33m" << j->titre << "\033[0m" << endl;
	}
	else
		cout << "\t\033[31m" << "Aucun jeu trouv�, r�essayez" << "\033[0m" << endl;
}

unsigned int Developpeur::compterJeuxDeveloppes(ListeJeux& listeJeux)
{
	unsigned int n = 0;
	for (Jeu*& j : spanListeJeux(listeJeux)) {
		if (j->developpeur == getNom())
			n++;
	}
	return n;
}
