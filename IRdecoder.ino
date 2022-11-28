/*Cores fiação
Marron: GND/N
Laranja: Vin/VCC
Verde: OUT/S
*/
#include "CODES.h"
#include <Preferences.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include "BluetoothSerial.h"
Preferences preferences;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;

//#include <IRremote.hpp> //INCLUSÃO DE BIBLIOTECA DE INFRAVERMELHO
#define PIN_IRSend 4
#define PIN_IrReceiver 13  //sequencia da esquerda para direita: out, gnd, vcc
IRsend irsend(PIN_IRSend);

//Tensao da rede eletrica
int rede = 220;
#include "EmonLib.h"
//Pino do sensor SCT
#define PIN_SCT 34
EnergyMonitor emon1;

#include "DHT.h"  //INCLUSÃO DE BIBLIOTECA SENSOR DE UMIDADE E TEMPERATURA AM2302
#define PIN_DHT 25
#define DHTTYPE DHT22
DHT dht(PIN_DHT, DHTTYPE);

float tempIdeal = 23.5;
float minTempIdeal = 22;
float maxTempIdeal = 25;

float consumo_com=0.00, consumo_total=0.00, hora;

//#include <Sleep_n0m1.h>
//Sleep sleep;

void setup() {
	Serial.begin(115200);
	SerialBT.begin("Controle_AC"); //Bluetooth device name
	SerialBT.println("Enabled IRin");
	irsend.begin();
	dht.begin();

	emon1.current(PIN_SCT, 111.1);
	preferences.begin("ac-ifpi", false);
	irsend.sendCOOLIX(dataON);
	irsend.sendCOOLIX(data22);
	delay(5000);
}
int estado=0;
void loop() {
	SerialBT.print("Estado: ");
	SerialBT.print(estado);
	SerialBT.println();
	
	static unsigned long startTime = millis();
	long seconds = (millis() - startTime) / 1000;
	
	//Calcula a corrente
	double Irms = emon1.calcIrms(1480);
	float potencia = Irms/1000 * rede;
	consumo_com = consumo_com + potencia;
	SerialBT.print("Consumo: ");
	SerialBT.print(consumo_com);  // Consumo
	SerialBT.print(" W");
	SerialBT.println();
	SerialBT.print("Corrente: ");
	SerialBT.print(Irms);  // Consumo
	SerialBT.println(" mA");
	
	if (seconds >= 52200) {
		SerialBT.print("Tempo máximo atingido!");
		SerialBT.println();
		hora = seconds/3600;
		consumo_total = consumo_com*hora;
		preferences.putFloat("consumo-com", consumo_com);
		preferences.putFloat("consumo-khw", consumo_total);
		preferences.putULong64("tempo-com", seconds);
		SerialBT.println("Valor final salvo: ");
		SerialBT.print("Potencia Total: ");
		SerialBT.print(preferences.getFloat("consumo-com", 0));
		SerialBT.print("W");
		SerialBT.println();
		SerialBT.print("Consumo Energia: ");
		SerialBT.print(preferences.getFloat("consumo-khw", 0));
		SerialBT.print("KWh");
		SerialBT.println();
		SerialBT.print("Tempo Total: ");
		SerialBT.print(preferences.getULong64("tempo-com", 0));
		SerialBT.print(" segundos");
		preferences.end();
		irsend.sendCOOLIX(dataOFF);
		while (1) {  // loop infinito
		}
	}

	// Capturar humidade do sensor
	float hum_dht = dht.readHumidity();
	// Capturar temperatura do sensor em Celsius (padrão)
	float temp_dht = dht.readTemperature();
	SerialBT.print("Temperatura: ");
	SerialBT.print(temp_dht);
	SerialBT.print(" ˚");
	SerialBT.println();

	if (temp_dht >= maxTempIdeal) {  //se a temperaturaC lida for superior a 25 °C
		if(estado<1000){
			estado++;
		}else if(estado>=1000){
			irsend.sendCOOLIX(data20);
			estado=0;
		}
		
	} else if (temp_dht >= minTempIdeal && temp_dht <= maxTempIdeal) {
		estado=0;
			SerialBT.print("Temperatura agradável: ");
			SerialBT.print(temp_dht);
			SerialBT.print(" ˚");
			SerialBT.println();
	} else if (temp_dht < minTempIdeal) {
		if(estado<1000){
			estado++;
		}else if(estado>=1000){
			irsend.sendCOOLIX(data24);
			estado=0;
		}
			SerialBT.print("Alterou a TEMP para 25º");
	}
}