/* ************************************************** */
/* TARGET: ARDUINO NANO                               */
/* ************************************************** */
#include "Adafruit_TLC5947.h"


// ***********************************************************************
// ***********************************************************************
// ***********************************************************************
// ***********************************************************************


// ------------------------------------------------------------
// activer le mode de fonctionnement, commenter les autres modes
// #define MODE_TEST_LED    
//#define MODE_MULTI_LED_PROGRESSIF
#define MODE_CHENILLARD_UNE_PROGRESSIF

// ------------------------------------------------------------
// DUREE DES ETATS ON et OFF des LEDs pour le mde MODE_MULTI_LED_PROGRESSIF
// ATTENTION MAX doit etre superieur ou égal à MIN
// plus les temps sont long, plus la progression bers le On ou vers le OFF est rapide
#define ETAT_OFF_MIN  (25)       // temps minimum d'une led etainte,   si led ne sont pas assez souvent eteinte, on augmente cette valeur
#define ETAT_OFF_MAX  (100)      // temps maximum d'une led  eteinte,  S'il y a trop de led allumée en m^me temps tu augmente cette valeur 

#define ETAT_ON_MIN  (3)       // temps minimum d'une led  allumée ( sans effet flagrand)
#define ETAT_ON_MAX  (5)       // temps maximum d'une led  allumée  ( sans effet flagrand)

// ------------------------------------------------------------
// On active une ligne ou l'autre
//#define MODE_PROGRESSION_14  	// la progression se fait sur 14 steps
#define MODE_PROGRESSION_25  	// la progression se fait sur 25 steps

// ------------------------------------------------------------
// Si cette ligne est active sa ralentie tous, et c'est moins fluide ca prenf trop de temps
//#define SYNCHROISEE 			

// ------------------------------------------------------------
// Tempo pour le mode MODE_CHENILLARD_UNE_PROGRESSIF
#define DELAY_PROGRESSIF_ON  (3)  // Pause en ms entre chaque step d'allumage de la led
#define DELAY_PROGRESSIF_OFF (5)  // Pause en ms entre chaque step d'extinction de la led

// ***********************************************************************
// ***********************************************************************
// ***********************************************************************
// ***********************************************************************


// ------------------------------------------------------------
// Quel cible on utilise, (commenter la ligne inutile)
#define ARDUINO_NANO

// ------------------------------------------------------------
// Combien de hardWare sont enchainé ?
#define NUM_TLC5974 1	

// ------------------------------------------------------------
// Nombre de LED
 #define NB_LED (24)

// ------------------------------------------------------------
// DEFINITIONS DES PINs En FONCTION DE LA CIBLE 
 #ifdef ARDUINO_NANO
  #define PIN_EMBEDED_LED  (11)
  #define PIN_DATA (4)
  #define PIN_CLOCK (5)
  #define PIN_LATCH (6)
 #endif
 
 #ifdef ARDUINO_DIGISPARK
  #define PIN_EMBEDED_LED  (1)
  #define PIN_DATA (0)
  #define PIN_CLOCK (2)
  #define PIN_LATCH (3)
 #endif

// ------------------------------------------------------------
// Liste des valeur de luminosité des LED (8 intensités par led) 
// les 8 valeurs ne sont pas linéaire, c'est volontaire, pourmieux voirla progression lumineuse


#ifdef MODE_PROGRESSION_14
 #define NB_BRIGHTNESS_VALUE (14)
 uint16_t tabBrightness[NB_BRIGHTNESS_VALUE] = {0, 1, 2, 4, 8, 16 , 32, 64, 128, 256, 512, 1024, 2048, 4096 };
#endif

#ifdef MODE_PROGRESSION_25
 #define NB_BRIGHTNESS_VALUE (25)
 uint16_t tabBrightness[NB_BRIGHTNESS_VALUE] = {0,1,2, 3, 4, 6, 8, 12, 16,  24, 32,  48, 64,  96, 128, 192, 256, 384, 512, 768, 1024,  1536, 2048,  3072, 4096 };
#endif


// ------------------------------------------------------------
// Object depilotage du driver de LED
Adafruit_TLC5947 tlc = Adafruit_TLC5947(NUM_TLC5974, PIN_CLOCK, PIN_DATA, PIN_LATCH);

// ------------------------------------------------------------
// Structure d'etat d'une LED
typedef struct {   
                   uint16_t  brightness;     // luminosité de la led a envoyer
                   uint16_t  brightnessOld;  // derniere luminosité de la led envoyée
                   
                   int etat;				// etat de la led (off, allumage, progessi, ON, extinction progressive)
                   int tempo_etat_off;      //!  cmpteur LED OFF
                   int indexMontee;         //!  index de luminosité montante de la LED
				   int maintien;           //!  compteur de maintien de la LED Allumée
           
                   int indexDescente;       //!  index de luminosité descendente de la LED
                 } ST_ETAT_LED2;


// ------------------------------------------------------------
// un tableau de n structure d'etat. n étant le nb de led.
  ST_ETAT_LED2 tabLedPpm2[NB_LED]; 

  
/* ************************************************************ */
// PGM SETUP, Lance une seul fois au demarage
/* ************************************************************ */
void setup() {
  int i=0; 

// Initialisation des pin en sortie
	pinMode(PIN_EMBEDED_LED, OUTPUT);
	pinMode(PIN_DATA, OUTPUT);
	pinMode(PIN_CLOCK, OUTPUT);
	pinMode(PIN_LATCH, OUTPUT);
  
// Initialisation de la structure de LED
	for ( i=0 ; i<NB_LED ; i++)
	{
		tabLedPpm2[i].brightness    			= 0 ;
		tabLedPpm2[i].brightnessOld 			= -1; // pour etre sur que ce soit different de '0'
		tabLedPpm2[i].etat 						= 0;

		tabLedPpm2[i].tempo_etat_off 			= random (ETAT_OFF_MIN, ETAT_OFF_MAX); ; // juste pour décaler l'allumage des led la premire fois
		
		tabLedPpm2[i].indexMontee 				= 0;
		tabLedPpm2[i].maintien 					= random (ETAT_ON_MIN, ETAT_ON_MAX); ;
		tabLedPpm2[i].indexDescente 			= 0 ;
	}
}


/* *********************************************************************** */
/* PGM PRINCCIPAL                                                          */
/* ilest executé en permanance                                             */
/* *********************************************************************** */
void loop() {
  
#ifdef MODE_TEST_LED
	testOut();
#endif  

#ifdef MODE_MULTI_LED_PROGRESSIF
    gestionLed();
#endif 

#ifdef MODE_CHENILLARD_UNE_PROGRESSIF
  gestionLed_chenillard();
#endif
 
}

/* *********************************************************************** */
/* pour tester les sorties, c'est un simple chenillard                     */
/* *********************************************************************** */
void testOut() {
	int i;
	for (i=0 ; i<NB_LED ; i++)
	{
		tlc.setPWM(i, 0x7ff );  // de 0 à 0x7ff, pour la luminosité
		tlc.write(); 
		delay(DELAY_PROGRESSIF_ON);

		tlc.setPWM(i, 0 ); 
		tlc.write(); 
		delay(1);
	}

}

/* *********************************************************************** */
/* gestionLed_chenillard()                                                 */ 
/* *********************************************************************** */  
void gestionLed_chenillard() {
	int pos = 0;
	int led = 0;

	for (led = 0; led < NB_LED; led += 1)
	{
	
		for (pos = 0; pos < NB_BRIGHTNESS_VALUE; pos += 1) { // goes from 0 degrees to 180 degrees
			tlc.setPWM(led,tabBrightness[pos]);
			tlc.write();
			delay(DELAY_PROGRESSIF_ON);                       // waits 15ms for the servo to reach the position
		}
		for (pos = NB_BRIGHTNESS_VALUE ; pos > 0; pos -= 1) { // goes from 180 degrees to 0 degrees
			tlc.setPWM(led,tabBrightness[pos-1]);
			tlc.write();
			delay(DELAY_PROGRESSIF_OFF);                       // waits 15ms for the servo to reach the position
		}
	}
	
}


/* *********************************************************************** */
/* gestionLed()                                                           */
// on parcours toutes les led une a une..
// Ppour chaque led 
// 1 )on regarde si la luminosité a commander est dierence de la derniere pilotée.
//    si oui -> on alume la led avec la nouvelle valeur de luminosité
//    si non -> on fait rien
// 
// 2) on calcule la future luminosité, pour le prochain passsage
// 
/* *********************************************************************** */
void gestionLed() {

   //Serial.println("TIMER");
    for (int i=0 ; i<NB_LED ; i++)
    {
      // Le fait de fair la comparaision avec l'ancienne valeur permet de gagner du temps 
#ifndef SYNCHROISEE      
      if ( tabLedPpm2[i].brightnessOld != tabLedPpm2[i].brightness )
      {
#endif        
        tlc.setPWM(i, tabLedPpm2[i].brightness);
        tlc.write();  
#ifndef SYNCHROISEE      		
       tabLedPpm2[i].brightnessOld = tabLedPpm2[i].brightness ;
      }
#endif      
      calcbright(i);
    }
}
/* ****************************************************************** */
// calcbright(int channel)
    // pour la led on regarde son etat doit elle etre :
    // - eteinte? 
    // - allumage progressif ?
	// - Allumé ?
// - extintion progressive ?

/* ****************************************************************** */
void calcbright(int channel)
{
  switch (tabLedPpm2[channel].etat)
    {
    case 0: // led eteinte pendant un certain temps !
		if ( tabLedPpm2[channel].tempo_etat_off-- <= 0){
			tabLedPpm2[channel].tempo_etat_off = random (ETAT_OFF_MIN, ETAT_OFF_MAX); ;
			tabLedPpm2[channel].etat = 1;
		}
	break;
    
    case 1:  // Allumage en progression
		tabLedPpm2[channel].brightness  = tabBrightness[tabLedPpm2[channel].indexMontee] ;
		if (++tabLedPpm2[channel].indexMontee >= NB_BRIGHTNESS_VALUE)
		{
			tabLedPpm2[channel].indexMontee = 0;
			tabLedPpm2[channel].etat = 2;
		}
	break;
      
    case 2:   // Maintenir en position
		if ( tabLedPpm2[channel].maintien-- <= 0){
			tabLedPpm2[channel].maintien = random (ETAT_ON_MIN, ETAT_ON_MAX); ;
			tabLedPpm2[channel].etat = 3;
			tabLedPpm2[channel].indexDescente = NB_BRIGHTNESS_VALUE-1 ;
		}  
		break;
      
    case 3: // descente extinction
		tabLedPpm2[channel].brightness  = tabBrightness[tabLedPpm2[channel].indexDescente] ;
		if ( tabLedPpm2[channel].indexDescente-- <= 0){
			tabLedPpm2[channel].indexDescente = 0;
			tabLedPpm2[channel].etat = 0;
		}  
		break;
    
      default: //Erreur ce cas ne devrait pas arrivée
		tabLedPpm2[channel].etat = 0;
		tabLedPpm2[channel].brightness = 15;    
       break;
    }
}

