#define SER 2 //multiplexe de la matrice
#define RCLK 3 
#define SRCLK 4

#define DATA 5 //multiplexe du 7 segment
#define RCLK7 6
#define SRCLK7 7

#define H 11 //bouton haut
#define B 10 //bouton bas
#define G 8 //bouton gauche
#define D 9 //bouton droit

#define haut 0
#define bas 1
#define gauche 2
#define droite 3

#define pot A0 //potentiomètre

#define longueur_max  64

byte dessin[8];//tableau de gestion de la matrice

const int digit[] = {A1,A2,A3,A4}; //pin des digits du 7 segments

byte numbit[] = {63,6,91,79,102,109,125,7,127,111}; //chiffres en segments

byte GAMEOVER[] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0b01111110,0xFF,0xFF,0b11000011,0b11001011,0b11101111,0b11101111,0b01101110,0x00,0b01111111,0xFF,0xFF,0b11001100,0b11001100,0xFF,0xFF,0b01111111,0x00,0xFF,0xFF,0b01110000,0b00110000,0b01110000,0xFF,0xFF,0x00, 0xFF,0xFF,0xFF,0b11011011,0b11011011,0b11011011,0b11000011,0b11000011,0x00,0x00,0x00,0b01111110,0xFF,0xFF,0b11000011,0b11000011,0xFF,0xFF,0b01111110,0x00,0b11111100,0b11111110,0xFF,0b00000011,0b00000011,0xFF,0b11111110,0b11111100,0x00, 0xFF,0xFF,0xFF,0b11011011,0b11011011,0b11011011,0b11000011,0b11000011,0x00, 0xFF,0xFF,0xFF,0b11011000,0b11011000,0xFF,0xFF,0b01110111, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

int tete; 
int queue;
int longueur_corps;
int DIRECTION;
unsigned int score;
int gameover;

unsigned long tmouv;
unsigned long t0;
unsigned long tg;
unsigned long tfin;

typedef struct position {
  int x;
  int y;
} Position;

Position nourriture;
Position corps[longueur_max];
//-----------------------------------------------------------------------------

void affiche_gameover()
{   tg = ((millis()-tfin)/125)%82;
    for (int i =0 ; i <= 7; i ++){
    digitalWrite ( RCLK, LOW ) ;
      shiftOut( SER , SRCLK , LSBFIRST, GAMEOVER[i+tg]);
      shiftOut( SER , SRCLK , LSBFIRST, 0xFF - bit(7-i));
    digitalWrite ( RCLK , HIGH ) ;
    }
}
//-----------------------------------------------------------------------------

void affiche_7(int s)
{
  for ( int i = 0; i <=3; i++){
    
    int m2 = pow(10,4-i) + 0.5; //le + 0.5 permet d'avoir 10000 ou 1000 en int au lieu de 9999 ou 999, du à un bug ARDUINO // la fonction pow fonctionne en floats.
    int p2 = pow(10,3-i) + 0.5;
    int S = (s%m2)/p2;
    
    digitalWrite(digit[i], LOW);
      digitalWrite( RCLK7 , LOW );
        shiftOut(DATA , SRCLK7 , MSBFIRST , numbit[S]);
      digitalWrite( RCLK7 , HIGH );
      digitalWrite( RCLK7 , LOW ); // cette partie permet de ne pas avoir de "déchet" sur le chiffre suivant donc l'affichage est plus "clair"
        shiftOut(DATA , SRCLK7 , MSBFIRST , 0x00);
      digitalWrite( RCLK7 , HIGH );
    
    for ( int j = 0; j <=3; j++)
    {
      digitalWrite(digit[j], HIGH);
    }
  }
}

//-----------------------------------------------------------------------------
void off7()
{
      for ( int j = 0; j <=3; j++)
      {
      digitalWrite(digit[j], HIGH);
      }
}

//-----------------------------------------------------------------------------

void affiche_matrice (int cx,int cy)
{ if ((cx == -1 ) && (cy == -1))// on éteint complètement la matrice si -1 -1 est appelé.
  {
    digitalWrite ( RCLK, LOW ) ;
      shiftOut( SER , SRCLK , LSBFIRST, 0x00);
      shiftOut( SER , SRCLK , LSBFIRST, 0xFF);
    digitalWrite ( RCLK , HIGH ) ;
  }
    dessin[cy] = bit(7-cx);
    
   
      byte p = bit(cy);
      digitalWrite ( RCLK, LOW ) ;// les deux multiplexeurs sont en série et sont donc sur la même horloge.
        shiftOut( SER , SRCLK , LSBFIRST, p);//envoie au deuxième multiplexeur la colonne à allumer.
        shiftOut( SER , SRCLK , LSBFIRST, ~(dessin[cy]));//envoie au premier multilplexeur la ligne a éteindre
      digitalWrite ( RCLK , HIGH ) ;//on va ainsi avoir les 2 multiplexeurs synchronisés et avoir un seul bit allumé à ce moment précis.
  
}

 //-----------------------------------------------------------------------------
 
bool appuyeBouton(int bouton) //Check quel bouton est appuyé
{ 
  int BA;
  if (digitalRead(bouton) == LOW){
    if (BA = 0) {
      BA = 1;
      return true;
    }
    else if (digitalRead(bouton) == HIGH) {      
        BA = 0;
        return false;
    }
  }
  else return false;
}

//-----------------------------------------------------------------------------

bool PositionValideNourriture() //On utilise bool pour recupérer un état True ou False
{ 
    if (nourriture.x < 0 || nourriture.y < 0) return false;
    for (int i = tete; i <= queue; i++) {
        if (corps[i].x == nourriture.x && corps[i].y == nourriture.y) return false;
    }
    return true;
}

//-----------------------------------------------------------------------------

void SpawnNourriture() //Fait apparaitre la nourriture
{ 
  while (!PositionValideNourriture()) { //Quand position invalide, PositionValideNourriture est false
    nourriture = {random(8),random(8)}; //Stock un nombre aleatoire entre 0 et 7 pour les coordonnées de la nourriture
  }
}


//-----------------------------------------------------------------------------
void MiamMiam() //Fonction qui vérifie si la nourriture a été mangé
{ 
  if (corps[tete].x == nourriture.x && corps[tete].y == nourriture.y) { //Vérifie que la tête a bien les mêmes coordonnées que la nourriture
    if (longueur_corps < longueur_max) { //Vérifie que la longueur max du corps n'est pas atteinte
      longueur_corps ++; //Augmente la longueur du corps de 1
      queue ++; 
    }
    if (longueur_corps <= 10){
      score += 10;
    }
    if ((10 < longueur_corps)&& (longueur_corps <= 20)){
      score += 25;
    }
    if ((20 < longueur_corps)&& (longueur_corps <= 25)){
      score += 50;
    }
    if (longueur_corps > 25){
      score += 100;
    }
    nourriture = {-1,-1}; //Position non valide donc fait réapparaitre de la nourriture ailleurs
    SpawnNourriture();
  }
}

//-----------------------------------------------------------------------------

void CheckDirection() // comme son nom l'indique, cette fonction va retourner la valeur de la direction(haut bas gauche droite) à chaque fois qu'elle est appelée.
  {
  if (DIRECTION == haut || DIRECTION == bas) {
    if (appuyeBouton(D) == HIGH) {
      DIRECTION = droite;
    }
    if (appuyeBouton(G) == HIGH) {
      DIRECTION = gauche;
    }
  }
  if (DIRECTION == droite || DIRECTION == gauche) {
    if (appuyeBouton(H) == HIGH) {
      DIRECTION = haut;
    }
    if (appuyeBouton(B) == HIGH) {
      DIRECTION = bas;
    }
  }
  return DIRECTION;
}

//-----------------------------------------------------------------------------

void MouvementSnake() 
{

  for (int i = queue ; i >= 1  ; i--){
    corps[i].x = corps[i-1].x;
    corps[i].y = corps[i-1].y;
  }
    switch (DIRECTION) //permet de donner à la tête une nouvelle direction ou de suivre l'ancienne si aucune action n'est effectuée
  {
    case 0: //haut
      corps[tete].y ++;
    break;
    case 1: //bas
      corps[tete].y --;
    break;
    case 2: //gauche
      corps[tete].x --;
    break;
    case 3: //droite
      corps[tete].x ++;
    break;
  }
  //le snake va avancer puisque chaque point va prendre la place du point précédent et la tête va avancer d'une case dans la direction souhaitée
  if (corps[tete].x > 7){
    corps[tete].x = 0;
  }
  if (corps[tete].x < 0){
    corps[tete].x = 7;
  }
  if (corps[tete].y > 7){
    corps[tete].y = 0;
  }
  if (corps[tete].y < 0){
    corps[tete].y = 7;
  }
  //toute cette partie va permettre que le snake passe de l'autre côté de la matrice si il rentre dans un mur.
}

void raz() //Remet le jeu a zéro
{ 
  corps[0] = {3,3};
  corps[1] = {3,4};
  corps[2] = {3,5};
  longueur_corps = 3; //Longueur du corps de base
  tete = 0; //Position de base de la tête dans le tableau du corps
  queue = 2; //Position de base de la queue dans le tableau du corps
  score = 0; // Reset le score
  DIRECTION = gauche; //Direction de base
  nourriture = {4,4}; //Permet la réapparition de nourriture
  score = 0;
  gameover = 0;
}



int lecpot()
{
  int a = analogRead(pot);
  return a;
}

void setup() {
 pinMode( DATA , OUTPUT );
 pinMode( RCLK7 , OUTPUT );
 pinMode( SRCLK7 , OUTPUT );
 
 pinMode( SER , OUTPUT );
 pinMode( RCLK , OUTPUT );
 pinMode( SRCLK , OUTPUT );
 
 pinMode( pot , INPUT );

 pinMode( H , INPUT_PULLUP);
 pinMode( B , INPUT_PULLUP);
 pinMode( G , INPUT_PULLUP);
 pinMode( D , INPUT_PULLUP);
 
 for (int i = 0; i <= 3 ; i++ ){
   pinMode( digit[i], OUTPUT );
   digitalWrite(digit[i], HIGH);
 }
 raz();

// Serial.begin(9600);
}

void loop() {
    unsigned long t = millis();
 if (gameover == 0 ){
  CheckDirection();
  affiche_matrice(corps[tete].x,corps[tete].y);
    for(int n = tete+1; n <= queue;n++)
    {
      affiche_matrice(corps[n].x,corps[n].y);
      if ((corps[tete].x == corps[n].x) && (corps[tete].y == corps[n].y)) 
      {
      gameover = 1;
      tfin = t;
      }
    };
    affiche_matrice(nourriture.x,nourriture.y);
    affiche_7(score);
    tmouv = t - t0;
    if (tmouv > lecpot())//permet de régler la vitesse avec le potentiomètre en réglant le temps entre 2 mouvements.
    {
      MouvementSnake();
      MiamMiam();
      t0 = t;
    }
 }
 else //si le joueur a perdu
 {
  if(t%1000 < 500){
    affiche_7(score);
  }
  else off7();
  affiche_matrice(-1,-1);
  affiche_gameover();
 }
}
