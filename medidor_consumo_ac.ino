#include <Preferences.h>
Preferences preferences;
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;

double consumo_sem=0.00, consumo_total=0.00, hora;

//Tensao da rede eletrica
int rede = 220;
#include "EmonLib.h"
#define PIN_SCT 34
EnergyMonitor emon1;


void setup() {
  Serial.begin(115200);
  SerialBT.begin("Controle_AC"); //Bluetooth device name
	SerialBT.println("Enabled IRin");
  
  emon1.current(PIN_SCT, 60);	

  preferences.begin("ac-ifpi", false); 
}

void loop() {
  static unsigned long startTime = millis();
	long seconds = (millis() - startTime)/1000;
  
	
	//Calcula a corrente
	double Irms = emon1.calcIrms(1480);
	double potencia = Irms*rede/1000;
	consumo_sem = consumo_sem + potencia;
  
	
  SerialBT.print("Tempo: ");
  SerialBT.println(seconds);
  SerialBT.print("Consumo: ");
	SerialBT.print(consumo_sem);  // Consumo
	SerialBT.print(" W");
	SerialBT.println();
	SerialBT.print("Corrente: ");
	SerialBT.print(Irms);  // Consumo
	SerialBT.println(" mA");

	if(seconds>=52200){
  //if(seconds>=60){
    hora = seconds/3600.00;
    consumo_total = (consumo_sem*hora)/1000.00;
    Serial.println("Tempo máximo atingido!");
		preferences.putDouble("consumo-sem", consumo_sem);
    preferences.putDouble("consumo-khw", consumo_total);
		preferences.putULong64("tempo-sem", seconds);
    Serial.print("Valor salvo em Preferences: ");
		while (1){                            // loop infinito  
      if (SerialBT.available()) {
        char valor = (char)SerialBT.read();
        if (valor=='A') {
            SerialBT.print("Potencia Total: ");
            SerialBT.print(preferences.getDouble("consumo-sem", 0));
            SerialBT.print("W");
            SerialBT.println();
            SerialBT.print("Consumo Energia: ");
            SerialBT.print(preferences.getDouble("consumo-khw", 0));
            SerialBT.print("KWh");
            SerialBT.println();
            SerialBT.print("Tempo Total: ");
            SerialBT.print(preferences.getULong64("tempo-sem", 0));
            SerialBT.print(" segundos");
        }
      }
		}
  }

}
