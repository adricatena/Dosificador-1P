#include <SPI.h>          
#include <MCUFRIEND_kbv.h>
#include <Keypad.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <FreeDefaultFonts.h>
#include <Adafruit_GFX.h>
#include <EEPROM.h>
#include "imagen2.h"

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

MCUFRIEND_kbv tft;

const byte filas = 4;
const byte columnas = 4;
char keys[filas][columnas] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'},
};

byte pinesFilas[filas] = {46, 47, 48, 49};
byte pinesColumnas[columnas] = {50, 51, 52, 53};

Keypad teclado = Keypad(makeKeymap(keys), pinesFilas, pinesColumnas, filas,columnas);
char tecla;

unsigned int flag = 0;
int ancho, alto, anchoUnidad, altoUnidad;

struct sReceta {
  unsigned int volumen;
  unsigned long tiempo;
  unsigned int numeroReceta;
  bool ultimaReceta;
};
const int maximoRecetas = 30;
int tamanoEstructura = 0;
sReceta aRecetas[maximoRecetas];
int recetasGuardadas = 0;
bool ultimaReceta = false;;
unsigned long t1 = 0;
unsigned long t2 = 0;
unsigned long tMarcha;
bool recetaNueva = false;
bool arregloVacio = false;
int pulsador = 40;
int salida = 30;
int error = -1;
bool agotoEspera = false;
unsigned long tiempoNuevo = 0;
int i, j, k;
String volumen, borrarReceta, marchaReceta;


void setup() {
  Serial.begin(9600);
  uint16_t ID = tft.readID();
  tft.begin(ID);
  
  tft.setRotation(1);
  ancho = tft.width();
  alto = tft.height();
  int anchoLogo = 184;
  int altoLogo = 84;
  tft.fillScreen(0xF7BF);
  tft.drawRGBBitmap((ancho-anchoLogo)/2, (alto-altoLogo)/2, logo1, anchoLogo, altoLogo);
  
  pinMode(pulsador, INPUT_PULLUP); // Pulsador
  pinMode(salida, OUTPUT); // Salida a rele
  
  //Iteracion para "resetear" EEPROM
  /*for(i = 0; i < EEPROM.length(); i++) {
    EEPROM.update(i, 255);
  }*/
  
  tamanoEstructura = sizeof(sReceta);
  for(i = 0; i < maximoRecetas; i++) {
    EEPROM.get(tamanoEstructura*i, aRecetas[i]);
    Serial.println(aRecetas[i].volumen);
    Serial.println(aRecetas[i].tiempo);
    Serial.println(aRecetas[i].numeroReceta);
    Serial.println(aRecetas[i].ultimaReceta);
    if(aRecetas[i].numeroReceta < 65535) {
      recetasGuardadas++;
    }
  }
  Serial.println(sizeof(sReceta));
  Serial.println(recetasGuardadas);
  
  i = 0;
  j = 0;
  k = 0;  
  
  delay(1000);
  //imprimirMensaje(int tamaño, int x, int y, uint16_t color, char *texto)
  imprimirMensaje(1, 0, 20, 0x1193, "PULSE CUALQUIER TECLA PARA COMENZAR");
}

void loop() {
  switch(flag) {
    case 0: // Espera de tecla de inicio
      if(teclado.getKey() != NULL) {
        flag = 1;
      }
      break;
      
    case 1: // Menu Inicio
      anchoUnidad = ancho / 9;
      altoUnidad = alto / 13;      
      tft.fillScreen(WHITE);      
      //fillRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
      //imprimirMensaje(int tamaño, int x, int y, uint16_t color, char *texto)
      tft.fillRect(2*anchoUnidad, 2*altoUnidad, 5*anchoUnidad, 3*altoUnidad, BLUE);      
      imprimirMensaje(2, anchoUnidad*2.7, altoUnidad*4, BLACK, "1 - Recetas");      
      tft.fillRect(2*anchoUnidad, 8*altoUnidad, 5*anchoUnidad, 3*altoUnidad, BLUE);
      imprimirMensaje(2, anchoUnidad*2.9, altoUnidad*10*1, BLACK, "2 - Marcha");      
      flag = 10;
      break;
      
    case 10: // Espera de Tecla de Menu
      tecla = teclado.getKey();
      if(tecla != NULL) {
        if(tecla == '1') {
          flag = 2;
        }
        else if(tecla == '2') {
          flag = 4;
        }  
      }
      break;
      
     case 2: // Menu de Recetas
      anchoUnidad = ancho / 9;
      altoUnidad = alto / 13;      
      tft.fillScreen(WHITE);      
      //fillRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
      //imprimirMensaje(int tamaño, int x, int y, uint16_t color, char *texto)
      tft.fillRect(anchoUnidad, altoUnidad, 7*anchoUnidad, 2*altoUnidad, BLUE);      
      imprimirMensaje(2, anchoUnidad*1.6, altoUnidad*2+(altoUnidad/2), BLACK, "1 - Agregar receta");      
      tft.fillRect(anchoUnidad, 4*altoUnidad, 7*anchoUnidad, 2*altoUnidad, BLUE);
      imprimirMensaje(2, anchoUnidad*1.5, altoUnidad*5+(altoUnidad/2), BLACK, "2 - Mostrar recetas");
      tft.fillRect(anchoUnidad, 7*altoUnidad, 7*anchoUnidad, 2*altoUnidad, BLUE);
      imprimirMensaje(2, anchoUnidad*1.9, altoUnidad*8+(altoUnidad/2), BLACK, "3 - Borrar receta");
      tft.fillRect(anchoUnidad, 10*altoUnidad, 7*anchoUnidad, 2*altoUnidad, BLUE);
      imprimirMensaje(2, anchoUnidad*3.2, altoUnidad*11+(altoUnidad/2), BLACK, "D - Inicio");      
      flag = 200;
     break;

     case 200: // Espera de tecla de Recetas
      tecla = teclado.getKey();
      if(tecla != NULL) {
        if(tecla == '1') {    
          flag = 210;
        }
        else if(tecla == '2') {
          flag = 220;
        }
        else if(tecla == '3') {
          flag = 230;
        }
        else if(tecla == 'D') {
          flag = 1;
        }
       }
     break;

     case 210: // Menu agregar receta
      if(recetasGuardadas == maximoRecetas) {
        tft.fillScreen(WHITE);
        imprimirMensaje(3, 5, 30, RED, "Se alcanzo el limite de recetas guardadas.");
        imprimirMensaje(3, 5, 60, RED, "Elimine alguna receta.");
        delay(6000);
        flag = 2;
      }
      else {
        instruccionesAgregarReceta();        
        flag = 211;
      }

      case 211: // Espera para continuar a agregar receta o cancelar
        tecla = teclado.getKey();
        if(tecla == '*') {
          flag = 212; // Marcha para agregar receta
        }
        else if(tecla == 'C') {
          flag = 2; // Menu de recetas
        }
      break;

      case 212: // Marcha para agregar receta      
        tft.fillScreen(WHITE);
        tft.fillRect(2*anchoUnidad, 5*altoUnidad, 5*anchoUnidad, 3*altoUnidad, RED);  
        imprimirMensaje(2, anchoUnidad*3.2, altoUnidad*7, BLACK, "ESPERA");
        t1 = millis();
        t2 = t1;
        while((t2-t1) < 10000  && !digitalRead(pulsador)) {
          t2 = millis();
        }
        if((t2-t1) >= 10000) {
          recetaNueva = false;
          agotoEspera = true;
        }
        else if(digitalRead(pulsador)) {
          t1 = millis();
          digitalWrite(salida, HIGH);
          tft.fillRect(2*anchoUnidad, 5*altoUnidad, 5*anchoUnidad, 3*altoUnidad, GREEN); 
          imprimirMensaje(2, anchoUnidad*3.2, altoUnidad*7, BLACK, "MARCHA");          
          while(digitalRead(pulsador)) {   
          }          
          t2 = millis();
          digitalWrite(salida, LOW);
          tiempoNuevo = t2 - t1;
          recetaNueva = true;
          agotoEspera = false;
        }
        else {
          error = 210;
          recetaNueva = false;
          agotoEspera = false;
        }        
        if(recetaNueva) {
          tft.fillScreen(WHITE);
          imprimirMensaje(1, anchoUnidad*3.1, altoUnidad*2, BLACK, "RECETA NUEVA");
        
          tft.fillRect(2*anchoUnidad, 4*altoUnidad, 5*anchoUnidad, 3*altoUnidad, BLUE);
          imprimirMensaje(2, anchoUnidad*2.9, altoUnidad*6, BLACK, "A - Aceptar");
        
          tft.fillRect(2*anchoUnidad, 8*altoUnidad, 5*anchoUnidad, 3*altoUnidad, BLUE);
          imprimirMensaje(2, anchoUnidad*2.7, altoUnidad*10*1, BLACK, "C - Cancelar");
          flag = 213; // espera boton aceptar o cancelar
        }
        else if(agotoEspera) {
          tft.fillScreen(WHITE);
          imprimirMensaje(3, 5, 30, RED, "SE AGOTO EL TIEMPO DE ESPERA");
          delay(5000);
          flag = 2; // Menu de recetas
        }     
      break;

     case 213: // espera para boton aceptar o cancelar nueva receta
      tecla = teclado.getKey();
      if(tecla == 'A') {
        flag = 214; // secuencia para guardar receta en memoria
      }
      else if(tecla == 'C') {
        flag = 2; // Menu de recetas
     break;

     case 214: // Menu para agregar voluemn
      instruccionesAgregarVolumen();
      flag = 215; // ingreso de teclas para volumen
     break;

     case 215: // Espera de teclas en agregar volumen
      tecla = teclado.getKey();
      if(tecla > 47 && tecla < 58) {
        mostrarVolumen();
      }
      else if(tecla == 'B') {
        borrarNumero();
      }
      else if(tecla == 'A') {
        /*
        aRecetas[recetasGuardadas].volumen = volumen.toInt();
        aRecetas[recetasGuardadas].tiempo = tiempoNuevo;
        aRecetas[recetasGuardadas].numeroReceta = recetasGuardadas;
        aRecetas[recetasGuardadas].ultimaReceta = true;
        EEPROM.put(tamanoEstructura*recetasGuardadas, aRecetas[recetasGuardadas]);
        if(recetasGuardadas > 0) {
          aRecetas[recetasGuardadas-1].ultimaReceta = false;
          EEPROM.put(tamanoEstructura*(recetasGuardadas-1), aRecetas[recetasGuardadas-1]);
        }  */
        agregarReceta();      
        recetasGuardadas++;
        recetaGuardada();
        volumen = "";
        flag = 0; // espera para presionar algun boton        
      }          
     break;

     case 220: // Menu mostrar recetas
      if(recetasGuardadas == 0) {
        tft.fillScreen(WHITE);
        imprimirMensaje(3, 5, 30, RED, "No hay recetas guardadas.");
        imprimirMensaje(3, 5, 60, RED, "Agrege primero alguna receta.");
        delay(6000);
        flag = 2;                     
      }
      else {
        tft.fillScreen(WHITE);
        imprimirMensaje(1, 5, altoUnidad, BLACK, "RECETAS");
        imprimirMensaje(1, 5, altoUnidad*2, RED, "A - Menu Recetas");
        imprimirMensaje(3, anchoUnidad*6, altoUnidad*1.9, BLACK, "vol [mL]");
        flag = 221; // muestra de pantalla dinamica
      }
     break;

     case 221: // muestra de recetas (parte dinamica)      
      mostrarRecetas();
      flag = 222; // espera de mostrar mas o volver
     break;

     case 222: // espera de tecla para mostrar mas o volver
      tecla = teclado.getKey();
      if(tecla == 'A') {
        i = 0;
        j = 0;
        k = 0;
        flag = 2;
      }
      else if(tecla == '1') {
        j = 0;
        flag = 221; 
      }
      else if(tecla == '2') {
        j = 10;
        flag = 221;
      }
      else if(tecla == '3') {
        j = 20;
        flag = 221;
      }
      
     break;

     case 230: // Menu Borrar receta
      if(recetasGuardadas == 0) {
        tft.fillScreen(WHITE);
        imprimirMensaje(3, 5, 30, RED, "No hay recetas guardadas.");
        imprimirMensaje(3, 5, 60, RED, "Agrege primero alguna receta.");
        delay(6000);
        flag = 2;
      }
      else {
        menuBorrarReceta();
        flag = 231; // espera para ingreso de receta a eliminar
      }
     break;

     case 231: // ingreso de receta a eliminar o volver al menu
      tecla = teclado.getKey();
      if(tecla == 'C') {
        borrarReceta = "";
        flag = 1; // menu inicio
      }
      else if(tecla > 47 && tecla < 58) {
        mostrarTecla();
      }
      else if(tecla == 'B') {
        borrarTecla();
      }
      else if(tecla == 'A') {
        if(borrarReceta.toInt() > recetasGuardadas || borrarReceta.toInt() < 1) {
          tft.fillScreen(WHITE);
          imprimirMensaje(3, 10, altoUnidad*4, RED, "¡ERROR ingresando el numero de receta!");
          borrarReceta = "";
          delay(2000);
          flag = 230; // volver a menu borrar receta
        }
        else {
          reordenArreglo();
          recetaBorrada();
          borrarReceta = "";
          flag = 0; // espera de presionar cualquier tecla
        }
      }
     break;
          
     case 4: // Menu de Marcha
      menuMarcha();
      flag = 40; // espera para ingresar receta a marchar
     break;

     case 40: // ingresar receta a marchar
      tecla = teclado.getKey();
      if(tecla == 'C') {
        marchaReceta = "";
        flag = 1; // Menu inicio
      }
      else if(tecla > 47 && tecla < 58) {
        mostrarTecla2();
      }
      else if(tecla == 'B') {
        borrarTecla2();
      }
      else if(tecla =='A') {
        if(marchaReceta.toInt() > recetasGuardadas || marchaReceta.toInt() < 1 || aRecetas[marchaReceta.toInt()-1].numeroReceta == 65535) {
          tft.fillScreen(WHITE);
          imprimirMensaje(3, 10, altoUnidad*4, RED, "¡ERROR ingresando el numero de receta!");
          marchaReceta = "";
          delay(2000);
          flag = 4;
        }
        else {
          instruccionesMarcha();
          flag = 41; // espera para marcha o cancelar
        }
      }
      break;

      case 41: // Espera para continuar a marcha o cancelar
        tecla= teclado.getKey();
        if(tecla == '*') {
          flag = 42; // Espera a Marcha        
        }
        else if(tecla == 'C') {
          flag = 4; // menu de marcha
        }
      break;

      case 42:
        pantallaEspera();
        imprimirMensaje(3, anchoUnidad, altoUnidad*12, RED, "C - Cancelar o Salir");
        flag = 43;
      break;

      case 43: // Espera a marcha        
        tecla = teclado.getKey();
        if(tecla == 'C') {
          flag = 4;
        }
        else if(digitalRead(pulsador)) {
          delay(5);
          if(digitalRead(pulsador)) {
            flag = 44; // Marcha
          }
        }
      break;

      case 44: // Marcha
        tft.fillRect(2*anchoUnidad, 5*altoUnidad, 5*anchoUnidad, 3*altoUnidad, GREEN); 
        imprimirMensaje(2, anchoUnidad*3.2, altoUnidad*7, BLACK, "MARCHA");
        imprimirMensaje(3, anchoUnidad, altoUnidad*12, RED, "C - Cancelar o Salir");
        tMarcha = aRecetas[marchaReceta.toInt()-1].tiempo;
        tecla = teclado.getKey();
        t1 = millis();
        t2 = t1;
        digitalWrite(salida, HIGH);
        while((t2-t1) <= tMarcha && tecla != 'C') {
          tecla = teclado.getKey();
          t2 = millis();
        }
        digitalWrite(salida, LOW);
        flag = 42; // vuelve a espera
      break;
         
      default:
      break;            
    }
  }
}

void imprimirMensaje(int tamano, int x, int y, uint16_t color, String texto) {
  if(tamano > 2) {
    tft.setFont(&FreeSans12pt7b);
    tamano = 1;
  }
  else {
    tft.setFont(&FreeSans9pt7b);
  }
  tft.setTextSize(tamano);
  tft.setCursor(x, y);    
  tft.setTextColor(color);
  tft.print(texto);
  delay(100);
}

void instruccionesAgregarReceta() {
  tft.fillScreen(WHITE);
  imprimirMensaje(3, 5, 30, RED, "Ubique el envase en posicion de llenado.");
  imprimirMensaje(3, 5, 60, RED, "Asegurese de que no se generen perdidas.");
  imprimirMensaje(3, 5, 90, RED, "Apriete y mantenga apretado el pulsador");
  imprimirMensaje(3, 5, 115, RED, "hasta el punto que usted desee.");
  imprimirMensaje(1, 5, 140, RED, "(considere soltarlo un instante antes de parar)");
  imprimirMensaje(3, 5, 270, RED, "PULSE '*' PARA CONTINUAR");
  imprimirMensaje(3, 5, 300, RED, "PULSE 'C' PARA CANCELAR");
}

void instruccionesAgregarVolumen() {
  tft.fillScreen(WHITE);
  imprimirMensaje(1, anchoUnidad*3.1, altoUnidad*2, BLACK, "RECETA NUEVA");
  imprimirMensaje(3, anchoUnidad*0.1, altoUnidad*4, BLACK, "Ingrese el volumen del producto:");
  imprimirMensaje(2, anchoUnidad*5, altoUnidad*6.5, BLACK, "[mL]");
  imprimirMensaje(1, 5, altoUnidad*8, BLACK, "1 [L] = 1000 [mL]");
  imprimirMensaje(1, 5, altoUnidad*11, BLACK, "B - Borrar");
  imprimirMensaje(1, 5, altoUnidad*12, BLACK, "A - Aceptar");  
}

void mostrarVolumen() {
  volumen.concat(tecla);
  imprimirMensaje(2, anchoUnidad*2, altoUnidad*6.5, BLACK, volumen);
}

void borrarNumero() {
  volumen = volumen.substring(0, volumen.length()-1);
  tft.fillRect(anchoUnidad, altoUnidad*4.3, anchoUnidad*3.9, altoUnidad*2.9, WHITE);
  imprimirMensaje(2, anchoUnidad*2, altoUnidad*6.5, BLACK, volumen);
}

void agregarReceta() {
  j = maximoRecetas-1;
  for(i = 0; i < maximoRecetas; i++) {
    if(aRecetas[i].numeroReceta == 65535 && j > i) {
      j = i;
    }
  }
  aRecetas[j].volumen = volumen.toInt();
  aRecetas[j].tiempo = tiempoNuevo;
  aRecetas[j].numeroReceta = j;
  aRecetas[j].ultimaReceta = true;
  EEPROM.put(tamanoEstructura*j, aRecetas[j]);
  /*
        aRecetas[recetasGuardadas].volumen = volumen.toInt();
        aRecetas[recetasGuardadas].tiempo = tiempoNuevo;
        aRecetas[recetasGuardadas].numeroReceta = recetasGuardadas;
        aRecetas[recetasGuardadas].ultimaReceta = true;
        EEPROM.put(tamanoEstructura*recetasGuardadas, aRecetas[recetasGuardadas]);
        if(recetasGuardadas > 0) {
          aRecetas[recetasGuardadas-1].ultimaReceta = false;
          EEPROM.put(tamanoEstructura*(recetasGuardadas-1), aRecetas[recetasGuardadas-1]);
        }  */
}

void recetaGuardada() {
  String texto2 = String(j+1);
  tft.fillScreen(WHITE);
  imprimirMensaje(2, 10, altoUnidad*4, BLACK, "RECETA NUMERO");  
  imprimirMensaje(2, anchoUnidad*6.5, altoUnidad*4, RED, texto2);
  imprimirMensaje(2, 10, altoUnidad*6, BLACK, "GUARDADA CON EXITO");
  imprimirMensaje(1, 5, altoUnidad*11, BLACK, "PULSE CUALQUIER TECLA PARA CONTINUAR");
}

void mostrarRecetas() {
  String receta;
  tft.fillRect(5, altoUnidad*2.2, anchoUnidad*13, altoUnidad*13, WHITE);  
  for(i = 0; i < 10; i++) {
      if(aRecetas[j].numeroReceta < 65535) {
        receta = "RECETA NUMERO " + String(aRecetas[j].numeroReceta +1);
        imprimirMensaje(3, 10, altoUnidad*(i+3), BLACK, receta);
        imprimirMensaje(3, anchoUnidad*6, altoUnidad*(i+3), BLACK, String(aRecetas[j].volumen));
        j++;
      }
      else if(aRecetas[j].numeroReceta == 65535) {
        receta = "RECETA VACIA";
        imprimirMensaje(3, 10, altoUnidad*(i+3), BLACK, receta);
        j++;
      }
  }
  if(j > 20) {
    receta = "pag 3/3";
  }
  else if(j > 10) {
    receta = "pag 2/3";
  }
  else {
    receta = "pag 1/3";
  }
  imprimirMensaje(1, anchoUnidad*7, altoUnidad*13, RED, receta);
}

void menuBorrarReceta() {
  String texto = "Tiene " + String(recetasGuardadas) + " recetas guardadas";
  tft.fillScreen(WHITE);
  imprimirMensaje(3, 10, altoUnidad*3, BLACK, texto);
  imprimirMensaje(1, 10, altoUnidad*5, BLACK, "Ingrese el numero de receta a eliminar:");
  imprimirMensaje(1, 10, altoUnidad*10, BLACK, "A - Aceptar");
  imprimirMensaje(1, 10, altoUnidad*11, BLACK, "B - Borrar");
  imprimirMensaje(1, 10, altoUnidad*12, BLACK, "C - Cancelar");
}

void mostrarTecla() {
  borrarReceta.concat(tecla);
  imprimirMensaje(2, 10, altoUnidad*7, RED, borrarReceta);
}

void borrarTecla() {
  borrarReceta = borrarReceta.substring(0, borrarReceta.length()-1);
  tft.fillRect(8, altoUnidad*5.3, anchoUnidad*4, altoUnidad*3, WHITE);
  imprimirMensaje(2, 10, altoUnidad*7, RED, borrarReceta);
}

void reordenArreglo() {
  aRecetas[borrarReceta.toInt()-1].tiempo = 4294967295;
  aRecetas[borrarReceta.toInt()-1].volumen = 65535;
  aRecetas[borrarReceta.toInt()-1].numeroReceta = 65535;
  aRecetas[borrarReceta.toInt()-1].ultimaReceta = 255;
  EEPROM.put(tamanoEstructura*(borrarReceta.toInt()-1), aRecetas[borrarReceta.toInt()-1]);
  recetasGuardadas--;
  
  /*for(i = borrarReceta.toInt()-1; i < recetasGuardadas; i++) {
    aRecetas[i].tiempo = aRecetas[i+1].tiempo;
    aRecetas[i].volumen = aRecetas[i+1].volumen;
    aRecetas[i].ultimaReceta = aRecetas[i+1].ultimaReceta;
    EEPROM.put(tamanoEstructura*i, aRecetas[i]);
  }
  aRecetas[recetasGuardadas-1].tiempo = 4294967295;
  aRecetas[recetasGuardadas-1].volumen = 65535;
  aRecetas[recetasGuardadas-1].numeroReceta = 65535;
  aRecetas[recetasGuardadas-1].ultimaReceta = 255;
  EEPROM.put(tamanoEstructura*(recetasGuardadas-1), aRecetas[recetasGuardadas-1]);
  recetasGuardadas--;*/
}

void recetaBorrada() {
  tft.fillScreen(WHITE);
  imprimirMensaje(3, 10, altoUnidad*4, BLACK, "RECETA BORRADA CON EXITO");
  imprimirMensaje(1, 5, altoUnidad*11, BLACK, "PULSE CUALQUIER TECLA PARA CONTINUAR");
}

void menuMarcha() {
  String texto = "Tiene " + String(recetasGuardadas) + " recetas guardadas";
  tft.fillScreen(WHITE);
  imprimirMensaje(3, 10, altoUnidad*3, BLACK, texto);
  imprimirMensaje(1, 10, altoUnidad*5, BLACK, "Ingrese el numero de receta a marchar:");
  imprimirMensaje(1, 10, altoUnidad*10, BLACK, "A - Aceptar");
  imprimirMensaje(1, 10, altoUnidad*11, BLACK, "B - Borrar");
  imprimirMensaje(1, 10, altoUnidad*12, BLACK, "C - Cancelar");
}
void mostrarTecla2() {
  marchaReceta.concat(tecla);
  imprimirMensaje(2, 10, altoUnidad*7, RED, marchaReceta);
}

void borrarTecla2() {
  marchaReceta = marchaReceta.substring(0, marchaReceta.length()-1);
  tft.fillRect(8, altoUnidad*5.3, anchoUnidad*4, altoUnidad*3, WHITE);
  imprimirMensaje(2, 10, altoUnidad*7, RED, marchaReceta);
}

void instruccionesMarcha() {
  tft.fillScreen(WHITE);
  imprimirMensaje(3, 5, 30, RED, "Ubique el envase en posicion de llenado.");
  imprimirMensaje(3, 5, 60, RED, "Asegurese de que no se generen perdidas.");
  imprimirMensaje(3, 5, 90, RED, "Apriete el boton cuando este listo.");
  imprimirMensaje(3, 5, 270, RED, "PULSE '*' PARA CONTINUAR");
  imprimirMensaje(3, 5, 300, RED, "PULSE 'C' PARA CANCELAR");
}

void pantallaEspera() {
  tft.fillScreen(WHITE);
  tft.fillRect(2*anchoUnidad, 5*altoUnidad, 5*anchoUnidad, 3*altoUnidad, RED);
  imprimirMensaje(2, anchoUnidad*3.2, altoUnidad*7, BLACK, "ESPERA");
}
