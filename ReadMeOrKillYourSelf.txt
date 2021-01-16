******** PSX-P alpha 0.2 SRC public release by Yoshihiro ********

Please don't host this file and download it from http://www.pspgen.com


BUGS connus :
Dans PSX-P impossible de quitter l'émulateur avec HOME donc HOME désactiver part défaut.
Pour quitter, utilisez L + R + X

**** Nouveau et fonctionnement ****

GPU utiliser PEOPS Soft 1.17 de Pete Bernett avec la SDL ;

Ajout du FPS et de la résolution de la PSX en temps réel (désactivé dans release publique)

Optimisation du HW rootcounter pour le Vsync mult part 3 ;

Config.Bias ajouter peut être modifiée à la volée pour régler la Vitesse en Automatique 
pas terminé :=/ ;

Memorycard Fixer GFX Fixé pour la plupart des jeux ;=).

Fonctionnement de PSX-P dans le menu des roms ;

Bouton X pour lancer les *.BIN's / *.ISO's et aussi les *.EXE's / *.PSX's PlayStation ;

Bouton O pour lancer les iso's avec le Bios psx qui ce lance en premier Comme la console ;

Bouton START pour booter le bios et pouvoir éditer les memory cards ;

Bouton Triangle pour quitter émulateur sous l'écran de sélection des roms ;

Dossiers obligatoires dans le répertoire PSXP ;
./SaveState/*.sav \
./BIOS/scph1001.bin \
./MC/Mcd001.mcr \
./MC/Mcd001.mcr 


Ajout du save state et load state 

// Fonction in game seulement 
Revenir au XMB PSP : L + R + X ; 
Save state : L + R + START ;
Load State : L + R + SELECT ; 

Bouton PSX special L2 : le bouton Home ;
Bouton PSX special R2 : le bouton Note de musique ;


Pour activer la config CPU DYNAREC sur PSX-P il faut définir -DPSP_DYNAREC 
Dans le Makefile .


Dev officiel de PCSX Version PC : 
Écrit part :
codeur principal: linuzappz
Co-codeurs: shadow, Nocomp, Pete Bernett, nik3d ;

Greatz original de PCSX Version PC :

Duddie, Tratax, Kazzuya, JNS, Bobbi, Psychojak, Shunt
Keith, DarkWatcher, Xeven, Liquid, Dixon, Prafull"
Special thanks to:
Twin (we Love you twin0r), Roor (love for you too),
calb (Thanks for help :) ), now3d (for great help to my psxdev stuff :) ;


Yoshihiro Greetings: :

PacManFan, Groepaz from hitmen, Looser, MaGiXieN, Hlide, MrTuto_Alek, Snap06, Mathieulh, Dark_Alex, Fanjita, PSMonkey, Zx-81, Mickeyblue, Zeus, et toutes les personnes qui m'ont soutenu dans ce projet.

Official site : http://www.pspgen.com

Official Sponsor: http://www.gamefreax.com