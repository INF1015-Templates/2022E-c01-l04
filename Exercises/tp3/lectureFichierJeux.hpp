// Fonctions pour lire le fichier binaire.
#pragma once
#include "Liste.hpp"
#include "Jeu.hpp"
#include "Concepteur.hpp"
#include <iostream>

using UInt8  = uint8_t;
using UInt16 = uint16_t;

UInt8 lireUint8(std::istream& fichier);
UInt16 lireUint16(std::istream& fichier);
std::string lireString(std::istream& fichier);

std::shared_ptr<Concepteur> chercherConcepteur(Liste<Jeu>& listeJeux, std::string nom);
std::shared_ptr<Concepteur> lireConcepteur(Liste<Jeu>& lj, std::istream& f);
std::shared_ptr<Jeu> lireJeu(std::istream& f, Liste<Jeu>& lj);
Liste<Jeu> creerListeJeux(const std::string& nomFichier);
