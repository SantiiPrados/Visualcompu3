#include "mbed.h"

#define TRUE            1 //para dar un verdadero
#define NUMBOTONES      4 //numero botones
#define MAXLED          4 //numero de leds
#define TIMETOSTART     1000
#define TIMEMAX         1501
#define BASETIME        500
#define ESPERAR         0 //fase de espera al pulsador en la mef juego
#define JUEGO           1 //fase de ejecucion del juego en la mef juego
#define JUEGOTERMINADO  2 //fase final en la mef juego
#define TECLAS          3 //fase de herror en la mef juego
#define INTERVAL        40 //milisegundos a esperar para tomar el estado del pulsador

typedef enum{
    BUTTON_DOWN,    //0
    BUTTON_UP,      //1
    BUTTON_FALLING, //2
    BUTTON_RISING   //3
}_eButtonState;


typedef struct{
    uint8_t estado;
    int32_t timeDown;
    int32_t timeDiff;
}_sTeclas;

_sTeclas ourButton[NUMBOTONES];
uint16_t mask[]={0x0001,0x0002,0x0004,0x0008}; // 0001 , 0010,  0100, 1000

uint8_t estadoJuego=ESPERAR;
_eButtonState myButton;

void startMef(uint8_t indice); //Le dá un estado inicial a myButton
//buttonState Este parámetro le pasa a la MEF el estado del botón.
void actuallizaMef(uint8_t indice ); //MEF Actualiza el estado del botón cada vez que se invoca
void togleLed(uint8_t indice); //cambia el estado del led

BusIn botones(PB_6,PB_7,PB_8,PB_9);
BusOut leds(PB_12,PB_13,PB_14,PB_15);


DigitalOut LedBlue(PC_13); //lo uso para prender led integrado en bluepill

Timer miTimer; //Timer para esperar 40 milisegundos

int tiempoMs=0; //Almacena el tiempo del timmer una vez cumplido
int parpadeo=0;//Almacena tiempo

int main()
{
    miTimer.start();
    uint16_t ledAuxRandom=0;
    //int tiempoRandom=0;
    int ledAuxRandomTime=0;
    int ledAuxJuegoStart=0;
    uint8_t varwin=0; //indica si el jugado gano o no
    uint8_t vardestello=0;//cuanta la cantidad de destellos del led
    uint8_t indiceAux=0;
    for(uint8_t indice=0; indice<NUMBOTONES;indice++){
        startMef(indice);
    }

    while(TRUE)
    {
        switch(estadoJuego){
            case ESPERAR:
                if ((miTimer.read_ms()-tiempoMs)>INTERVAL){
                    tiempoMs=miTimer.read_ms();
                    for(uint8_t indice=0; indice<NUMBOTONES;indice++){
                        actuallizaMef(indice);
                        if(ourButton[indice].timeDiff >= TIMETOSTART){
                            srand(miTimer.read_us());
                            estadoJuego=TECLAS;
                        }
                    }
                }
            break;
            case TECLAS:
                for( indiceAux=0; indiceAux<NUMBOTONES;indiceAux++){
                    actuallizaMef(indiceAux);
                    if(ourButton[indiceAux].estado!=BUTTON_UP){
                        break;
                    }
                }
                if(indiceAux==NUMBOTONES){
                    estadoJuego=JUEGO;
                    leds=15;
                    ledAuxJuegoStart=miTimer.read_ms();
                }
            break;
            case JUEGO:
                if(leds==0){
                    ledAuxRandom = rand() % (MAXLED);
                    ledAuxRandomTime = (rand() % (TIMEMAX))+BASETIME;
                    //tiempoRandom=miTimer.read_ms();
                    leds=mask[ledAuxRandom];
                }else{
                    if((miTimer.read_ms()-ledAuxJuegoStart)>TIMETOSTART) {
                        if(leds==15){
                            ledAuxJuegoStart=miTimer.read_ms();
                            leds=0;
                        }
                    }
                }
                
                togleLed(ledAuxRandom);
                if ((miTimer.read_ms()-tiempoMs)<=ledAuxRandomTime){
                    actuallizaMef(ledAuxRandom);
                    if(ourButton[ledAuxRandom].estado!=BUTTON_UP){
                        varwin=1;
                        tiempoMs=miTimer.read_ms();
                        estadoJuego=JUEGOTERMINADO;
                    }
                }else{
                    leds=0;
                    varwin=0;
                    tiempoMs=miTimer.read_ms();
                    estadoJuego=JUEGOTERMINADO;
                }

                /*if(ourButton[ledAuxRandom].estado!=BUTTON_UP){
                    varwin=1;
                    tiempoMs=miTimer.read_ms();
                    estadoJuego=JUEGOTERMINADO;
                }else{
                    varwin=0;
                    tiempoMs=miTimer.read_ms();
                    estadoJuego=JUEGOTERMINADO;
                }*/
                
            break;
            case JUEGOTERMINADO:
                if(varwin==1){
                    //destello todos los leds 3 veces por 500ms
                    if ((miTimer.read_ms()-tiempoMs>BASETIME)&&(vardestello<=4)){
                        tiempoMs=miTimer.read_ms();
                        if(vardestello==0||vardestello==2||vardestello==4){
                            leds=15;
                        }else{
                            leds=0;
                        }
                        if (vardestello<4){
                            vardestello++;
                        }else{
                            vardestello=0;
                            estadoJuego=ESPERAR;
                        }
                    }
                }else{
                    //destello 3 veces el led herrado por 500ms
                    if ((miTimer.read_ms()-tiempoMs>BASETIME)&&(vardestello<=4)){
                    if(vardestello==0||vardestello==2||vardestello==4){
                        leds=ledAuxRandom;
                    }else{
                        leds=0;
                    }
                        if (vardestello<4){
                            vardestello++;
                        }else{
                            vardestello=0;
                            estadoJuego=ESPERAR;
                        }
                    }
                }
            break;
            default:
                estadoJuego=ESPERAR;
        }

        if ((miTimer.read_ms()-parpadeo)>=BASETIME){
            parpadeo=miTimer.read_ms();
            LedBlue=!LedBlue;
            pc.printf("\r\n estadestellando");
        }

       /*
       if ((miTimer.read_ms()-tiempoMs)>INTERVAL){
           tiempoMs=miTimer.read_ms();
           for(uint8_t indice=0; indice<NROBOTONES;indice++){
               actuallizaMef(indice);
               if(ourButton[indice].timeDiff >= TIMETOSTART){
                    ourButton[indice].timeDiff=0;
                    srand(miTimer.read_us());
                    ledAuxRandom = rand() % (MAXLED);
                    togleLed(ledAuxRandom);
                    ledAuxRandomTime = (rand() % (TIMEMAX))+BASETIME;
                    tiempoRandom=miTimer.read_ms();
               }
           }
        }
        */
    }
    return 0;
}

void startMef(uint8_t indice){
    //le dice al boton del indice mandado que esta en estado no apretado
   ourButton[indice].estado=BUTTON_UP;
}

void actuallizaMef(uint8_t indice){

    switch (ourButton[indice].estado){
        case BUTTON_DOWN:
            if(botones.read() & mask[indice] )
            ourButton[indice].estado=BUTTON_RISING;
        
        break;
        case BUTTON_UP:
            if(!(botones.read() & mask[indice]))
                ourButton[indice].estado=BUTTON_FALLING;
        
        break;
        case BUTTON_FALLING:
            if(!(botones.read() & mask[indice]))
            {
                ourButton[indice].timeDown=miTimer.read_ms();
                ourButton[indice].estado=BUTTON_DOWN;
                //Flanco de bajada
            }
            else
                ourButton[indice].estado=BUTTON_UP;    

        break;
        case BUTTON_RISING:
            if(botones.read() & mask[indice]){
                ourButton[indice].estado=BUTTON_UP;
                //Flanco de Subida
                ourButton[indice].timeDiff=miTimer.read_ms()-ourButton[indice].timeDown;
            /*
                if(ourButton[indice].timeDiff >= TIMETOSTART)
                    togleLed(indice);
                    */
            }

            else
                ourButton[indice].estado=BUTTON_DOWN;
        
        break;
        
        default:
        startMef(indice);
            break;
    }
}

void togleLed(uint8_t indice){
   leds=mask[indice];
}
