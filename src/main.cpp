/*
  Titre      : DEMO TMP36 - MKR 1000 - Thingsboard
  Auteur     : André Roussel
  Date       : 04/11/2021
  Description: lecture du TMP36 et envoie de cette valeur a une fréquences spécifique sur la plateforme IDO Thingsbaord
  Droits     : Reproduction permise pour usage pédagogique
  Version    : 0.0.4
*/

#include <Arduino.h>
#include "WIFIConnector_MKR1000.h" //inclusion d'un libairie pour la focntionnalite wifi
#include "MQTTConnector.h"         //inclusion d'un libairie pour la focntionnalite MQTT.h

/*
  Variables de base pour la lecture de la 
  sortie du TMP36 a partir d'une broche digitale
*/

const int ANALOG_PIN = A1; // Utilisation de la broche A1 pour lecture analogue
const int LED_BLEU = 3;
const int LED_ROUGE = 2;
int AnalogValue = 0; // Variable pour contenir la valeur lue de la broche analogue

/*
  Variables pour obtenir une valeur de tension a partir du résultat du convertisseur
  analogique digitale CAD, de 0 à 1023 en 0 à 3.3V.  A ajuster selon la tension maximale
  permise par le microncontrolleur et la resolution de la broche 8,10,12,...  bits
*/

const float PIN_BASE_VOLTAGE = 3.3;    // Tension de base maximale pour la broche analogue du uC utilisé
const int PIN_ANALOG_MAX_VALUE = 1023; // Valeur maximale de la lecture de la broche analogue
float PinVoltage = 0;                  // Variable pour la transformation de la lecture analogue a une tension

/*
  Variables pour obtenir une mesure de température en 
  degré Celcius a partir d'une valeur en V pour le TMP36
*/

const float TENSION_DECALAGE = 500; // Tension de décalage pour le TMP36 en mV
float TMP36Temperature = 0;         // Varaiable pour contenir la temperature
int MvParDegreeCelcius = 10;        // Nombre de mV par degré Celcius

// Variable de controle générales tel les délais, etc...

const int MS_DELAI = 1000; // Nombre de milliseconde de délais

// La fonction setup sert, entre autre chose, a configurer les broches du uC

void setup()
{

  Serial.begin(9600); // Activation du port série pour affichage dans le moniteur série

  wifiConnect(); //Branchement au réseau WIFI
  MQTTConnect(); //Branchement au broker MQTT

  pinMode(ANALOG_PIN, INPUT); // Pour une bonne lecture, la broche analogique doit être placé en mode entré explicitment

  pinMode(LED_BLEU, OUTPUT);
  pinMode(LED_ROUGE, OUTPUT);
}

// Boucle infinie qui permet au uC de faire une ou plusieurs taches sans arrêt

void loop()
{
  // put your main code here, to run repeatedly:

  AnalogValue = analogRead(ANALOG_PIN); // Lecture de la broche analogue et sauvegarde de valeur

  /*
      Par défaut, sur le Arduino MKR1000, la lecture d'une broche analogue donneras une valeur de 0 a 1023.
      Nous pouvons alors utilisé la règle de 3 pour obtenir la tension lu par la broche analogue
      c'est-à-dire, sachant qu'une lecture de 1023 est égale a 3.3V, par regle de 3,
      la tension sera donnè par Tension = (lecture *3.3)/1023
  */

  PinVoltage = (AnalogValue * PIN_BASE_VOLTAGE) / PIN_ANALOG_MAX_VALUE; // Transfert de la lecture de la broche en tension

  /* 
    Calcul de la temnpérature a partir de la tension de lecture en Volts
      1. transfert de la valeur de tension de Volts en mV en multipliant la tension obtenue en lecture par 1000
      2. Soustraction de 500 mV comme facteur de correction
      3. Le tout divisé par 10 mV par degré Celcius pour obtenir des degré Celcius
  */

  TMP36Temperature = (PinVoltage * 1000 - TENSION_DECALAGE) / MvParDegreeCelcius;

  if (TMP36Temperature >= 25)
  {
    // C'est ici que on va allumer la climatisation et eteindre le chauffage
    digitalWrite(LED_ROUGE, LOW);
    digitalWrite(LED_BLEU, HIGH);
  }
  else if (TMP36Temperature <= 21)
  {
    // c'est ici que on va allumer le chauffage et eteindre la climatisation
    digitalWrite(LED_ROUGE, HIGH);
    digitalWrite(LED_BLEU, LOW);
  }
  else
  {
    // c'est ici que on va eteindre le chauffage et eteindre la climatisation
    digitalWrite(LED_ROUGE, LOW);
    digitalWrite(LED_BLEU, LOW);
  }

  // Impression dans le moniteur série

  Serial.print("La valeur obtene par la broche analogue est ");
  Serial.println(AnalogValue);

  Serial.print("La valeur de tension obtenue par la broche analogue est ");
  Serial.print(PinVoltage);
  Serial.println(" V");

  Serial.print("La valeur de température est alors  ");
  Serial.print(TMP36Temperature);
  Serial.println(" degré Celcius");

  appendPayload("Temperature", TMP36Temperature); //Ajout de la donnée température au message MQTT
  sendPayload();                                  //Envoie du message via le protocole MQTT

  delay(MS_DELAI); // Délai de sorte a ce qu'on puisse lire les valeurs et ralentir le uC
                   // Note: l'utilisation d'un délai est généralement une mauvaise pratique mais utilisable dans le cas de ce démo
}
