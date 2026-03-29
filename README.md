# Projet Splash

## 1. Présentation du projet

Dans ce projet, nous avons réalisé une version en C du jeu **Splash** demandé dans le sujet.

L’objectif était de développer un programme principal capable de charger 4 joueurs sous forme de bibliothèques dynamiques `.so`, puis de faire jouer ces 4 joueurs sur une grille de taille `100 x 100`.

Le projet a été réalisé sous Linux en langage C.

Notre programme permet :
- de charger 4 joueurs dynamiques ;
- de gérer une grille `100 x 100` ;
- de gérer les crédits d’action de chaque joueur ;
- de faire revenir les joueurs de l’autre côté lorsqu’ils sortent du plateau ;
- d’afficher graphiquement l’évolution de la partie ;
- d’avoir aussi un mode sans affichage pour faire des tests plus rapidement.

Nous avons essayé de faire un code simple, lisible et organisé, avec une séparation claire entre les différents fichiers.

---

## 2. Organisation du projet

### `splash.c`
C’est le fichier principal du projet.  
Il contient le `main()`.

Il sert à :
- lire les arguments donnés au lancement ;
- vérifier les bibliothèques des joueurs ;
- lancer la partie ;
- fermer proprement le programme à la fin.

### `actions.h`
Ce fichier contient les différentes actions possibles pour les joueurs.

Par exemple :
- avancer ;
- dash ;
- téléportation ;
- rester immobile.

Tous les joueurs utilisent ces mêmes codes d’action.

### `game.h` / `game.c`
Ces fichiers contiennent la logique principale du jeu.

Ils servent à :
- initialiser la partie ;
- placer les joueurs au départ ;
- gérer les tours ;
- lire les actions choisies par les joueurs ;
- appliquer les déplacements ;
- retirer les crédits ;
- arrêter la partie quand il n’y a plus de joueur actif.

C’est la partie la plus importante du projet.

### `board.h` / `board.c`
Ces fichiers servent à gérer la grille.

Ils permettent :
- de mémoriser quelle case appartient à quel joueur ;
- de mettre à jour les cases lorsque les joueurs avancent ;
- de calculer les scores.

Nous avons choisi de faire en sorte qu’une case appartienne au dernier joueur passé dessus.

### `player.h` / `player.c`
Ces fichiers servent à charger les joueurs dynamiquement.

Nous utilisons pour cela :
- `dlopen`
- `dlsym`
- `dlclose`

Le but est de charger les fichiers `.so` au moment de l’exécution, puis d’appeler leur fonction `get_action()`.

### `renderer.h` / `renderer_x11.c`
Ces fichiers servent à la partie graphique.

Nous avons choisi d’utiliser **X11** pour afficher :
- la grille ;
- les couleurs des cases ;
- les informations sur les joueurs ;
- les scores et crédits.

Nous avons volontairement fait un affichage simple, mais fonctionnel.

### `p1_square.c`
Joueur de test qui effectue des déplacements simples pour former un parcours en carré.

### `p2_snake.c`
Joueur de test qui balaie progressivement la carte.

### `p3_spiral.c`
Joueur de test qui suit un parcours en spirale.

### `p4_dash.c`
Joueur de test qui utilise plusieurs types d’actions pour varier les déplacements.

### `Makefile`
Ce fichier permet de compiler facilement tout le projet.

Il génère :
- l’exécutable principal `splash`
- les 4 bibliothèques dynamiques `.so`

### `.gitignore`
Ce fichier sert à éviter d’ajouter au dépôt les fichiers générés automatiquement à la compilation.

---

## 3. Fonctionnement général

Le programme principal charge d’abord 4 joueurs dynamiques passés en argument.

Chaque joueur possède :
- une position de départ ;
- un nombre de crédits ;
- une stratégie définie dans sa fonction `get_action()`.

Ensuite, à chaque tour :
1. le programme demande une action à chaque joueur ;
2. il vérifie si le joueur a assez de crédits ;
3. il applique l’action ;
4. il met à jour la grille ;
5. il met à jour le score ;
6. il passe au tour suivant.

La partie s’arrête lorsqu’aucun joueur ne peut encore agir.

---

## 4. Choix réalisés

### Gestion du plateau
Nous avons utilisé une grille `100 x 100`.

Chaque case stocke le joueur qui la possède actuellement.  
Quand un autre joueur passe dessus, la case change de propriétaire.

Cela permet de recalculer facilement les scores pendant toute la partie.

### Gestion des déplacements
Nous avons distingué plusieurs actions :
- déplacement simple ;
- dash ;
- téléportation ;
- immobile.

Le dash et la téléportation n’ont pas exactement le même comportement, ce qui permet d’avoir des stratégies différentes selon les joueurs.

### Retour sur les bords
Quand un joueur sort de la grille d’un côté, il revient automatiquement de l’autre côté.

Exemple :
- s’il dépasse à gauche, il revient à droite ;
- s’il dépasse en haut, il revient en bas.

Nous avons donc appliqué un fonctionnement de type **Pacman**.

### Affichage graphique
Nous avons développé un affichage graphique simple sous Linux avec X11.

L’objectif n’était pas de faire un moteur très avancé, mais un affichage clair permettant de visualiser :
- les zones contrôlées ;
- les positions ;
- les scores ;
- l’évolution de la partie.

### Mode headless
Nous avons aussi ajouté un mode `--headless`.

Ce mode permet de lancer le jeu sans ouvrir de fenêtre, ce qui est pratique pour vérifier rapidement le fonctionnement du programme.

---

## 5. Compilation

Dans le dossier du projet, il faut exécuter :
./splash --delay 15 --render-every 1 ./p1_square.so ./p2_snake.so ./p3_spiral.so ./p4_dash.so
make


## 6. Répartition du travail

Nous avons réparti le travail de la manière suivante :

### Mohamed
- réflexion sur l’architecture générale du projet ;
- mise en place de la logique principale du jeu ;
- gestion de la grille et des scores ;
- gestion des déplacements et des crédits.

### Sami
- chargement dynamique des joueurs ;
- partie affichage graphique ;
- création des joueurs de test ;
- Makefile, organisation du projet et relecture finale.

Nous avons ensuite relu et testé ensemble l’ensemble du projet afin de vérifier son bon fonctionnement.
```bash

