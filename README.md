# Sujet de programmation impérative

La page du sujet :

<https://www.labri.fr/perso/renault/working/teaching/projets/2024-25-S6-C-Coursidor.php>

La page sur thor :

<https://thor.enseirb-matmeca.fr/ruby/projects/pr105-coursidor>

Lancer une/des parties : 
	make puis ./install/server ./install/tristan.so ./install/steven.so [options]
       Les options sont :
	    -m[1-9]* pour la changer la tailler m du graphe de jeu
	    -t[TCH] pour changer le type du graphe : T=TRIANGULAR, C=CIRCULAR, H=HOLEY
	    -M[1-9]* pour le nombre de tour maximum
	    -p pour afficher chaque coup de la partie
	    -g[1-9]* pour définir le nombre de partie jouée successivement
	    -s[1-9]* pour définir la seed de l'aléatoire

       Si on veut observer la dernière partie jouée par le serveur:
	./src/generate-multi-graph.sh game.dot
	evince output.pdf

Lancer les tests :
       make test
      Pour obsever les parties des tests:
       make neato
       evince test_[nom_du_test].pdf