#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <RTClib.h>
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>
#define SS_pin 10
#define sensor 2
#define ledDatos 5
#define btn1 6
#define btn2 7

int temperatura;
int humedad;
double cont;
unsigned long time1;
unsigned long time2 = 0;
boolean estadoWiFi = false;
int conta = 0;

File report;

LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 RTC;
DHT dht(sensor, DHT11);

//Defino logo de estado wifi full
byte wifiFull[8] = {B11111, B00000, B01110, B00000, B01110, B00000, B00100, B00100};

//Defino GeneriXOs Icon
byte genchar1[8] = {B00000, B00000, B00000, B00000, B00000, B00001, B00011, B00111};
byte genchar2[8] = {B11111, B11111, B11111, B01111, B00000, B00000, B00000, B00000};
byte genchar3[8] = {B00000, B00000, B00110, B01001, B10001, B00001, B00001, B10010};
byte genchar4[8] = {B11001, B10101, B10011, B01110, B00000, B00000, B00000, B00000};

//Defino Termometer Icon
byte tempchar1[8] = {B00000, B00001, B00010, B00100, B00100, B00100, B00100, B00111};
byte tempchar2[8] = {B00111, B00111, B00111, B00111, B00111, B00111, B00111, B00011};
byte tempchar3[8] = {B00000, B10000, B01011, B00100, B00111, B00100, B00111, B11100};
byte tempchar4[8] = {B11111, B11100, B11111, B11100, B11111, B11100, B11111, B11000};

//Defino Humidity Icon
byte humchar1[8] = {B00000, B00001, B00011, B00011, B00111, B01111, B01111, B11111};
byte humchar2[8] = {B11111, B11111, B11111, B01111, B00011, B00000, B00000, B00000};
byte humchar3[8] = {B00000, B10000, B11000, B11000, B11100, B11110, B11110, B11111};
byte humchar4[8] = {B11111, B11111, B11111, B11110, B11100, B00000, B00000, B00000};

//Defino Time Icon
byte timechar1[8] = {B00000, B00000, B00000, B00011, B00100, B01000, B10010, B10001};
byte timechar2[8] = {B10000, B10000, B01000, B00100, B00011, B00000, B00000, B00000};
byte timechar3[8] = {B00000, B00000, B00000, B11000, B00100, B00010, B01001, B10001};
byte timechar4[8] = {B00001, B00001, B00010, B00100, B11000, B00000, B00000, B00000};

//Defino WiFi Icon
byte wifichar1[8] = {B00000, B00111, B00111, B11000, B11000, B00000, B00111, B01111};
byte wifichar2[8] = {B01100, B00000, B00011, B00111, B00100, B00000, B00001, B00001};
byte wifichar3[8] = {B00000, B11100, B11100, B00011, B00011, B00000, B11100, B11110};
byte wifichar4[8] = {B00110, B00000, B11000, B11100, B00100, B00000, B10000, B10000};


void setup () {

  pinMode(ledDatos, OUTPUT);
  pinMode(btn1, INPUT);
  pinMode(btn2, INPUT);
  Wire.begin(); // Inicia el puerto I2C
  RTC.begin(); // Inicia la comunicaci¢n con el RTC
  //RTC.adjust(DateTime(__DATE__, __TIME__)); // Establece la fecha y hora (Comentar una vez establecida la hora) se usa solo en la configuracion inicial

  dht.begin();
  lcd.init();
  lcd.backlight();

  /*INTRO*/
  crearIconos(genchar1, genchar2, genchar3, genchar4);
  lcd.setCursor(3, 0);
  lcd.print("  GeneriXOs");
  lcd.setCursor(3, 1);
  lcd.print("Sensor System");
  delay(5000);
  int estadoBTN;
  if (!estadoWiFi) {
    crearIconos(wifichar1, wifichar2, wifichar3, wifichar4);
    lcd.setCursor(2, 0);
    lcd.print(" Conectar WiFi");
    lcd.setCursor(2, 1);
    lcd.print("   Ok para Si   " );
    delay(1000);
    lcd.setCursor(2, 1);
    lcd.print("Espera para No");
    delay(1000);
    lcd.setCursor(2, 1);
    lcd.print("   Ok para Si   " );
    delay(1000);
    estadoBTN = digitalRead(btn1);
    lcd.setCursor(2, 1);
    lcd.print("Espera para No");
    delay(1000);
    estadoBTN = digitalRead(btn1);
    while (estadoBTN == 0 && conta <= 5000) {
      estadoBTN = digitalRead(btn1);
      conta++;
      delay(1);
    }
    if (estadoBTN == 1) {
      lcd.setCursor(2, 0);
      lcd.print(" Se conecto a ");
      lcd.setCursor(2, 1);
      lcd.print(" Nombre de red");
      estadoWiFi = true;
      delay(3000);
    } else {
      lcd.setCursor(2, 0);
      lcd.print(" No se conecto");
      lcd.setCursor(2, 1);
      lcd.print("    al WiFi   ");
      delay(3000);
    }
  }



  /*INICIAR ALMACENAMIENTO*/
  limpirarYZero();
  lcd.print("    Buscando    ");
  lcd.setCursor(0, 1);
  lcd.print(" Almacenamiento ");
  intermitenteRapido();

  if (!SD.begin(SS_pin)) {
    limpirarYZero();
    lcd.print("  Sin Memoria   ");
    lcd.setCursor(0, 1);
    lcd.print("    MicroSD!    ");
    intermitenteLento();
    lcd.clear();
    delay(1000);
    return;
  }
  limpirarYZero();
  lcd.print("Se hara respaldo");
  lcd.setCursor(0, 1);
  lcd.print("en la Micro SD");
  digitalWrite(ledDatos, HIGH);
  delay(2000);
  lcd.clear();
  delay(1000);


  /* CREANDO PRIMER REGISTRO*/
  crearRegistro();
}
void loop() {

  report = SD.open("report.csv", FILE_WRITE);
  DateTime now = RTC.now();
  temperatura = dht.readTemperature();
  humedad = dht.readHumidity();

  /*FECHA*/
  crearIconos(timechar1, timechar2, timechar3, timechar4);

  //logo de conexxion a red
  if (estadoWiFi) {
    lcd.createChar(5, wifiFull);
    lcd.setCursor(3, 1);
    lcd.write(5);
  }
  //fecha

  lcd.setCursor(6, 1);
  lcd.print(now.year(), DEC);
  lcd.print("/");
  if ((now.month()) <= 9) {
    lcd.print("0");
  }
  lcd.print(now.month(), DEC);
  lcd.print("/");
  if ((now.day()) <= 9) {
    lcd.print("0");
  }
  lcd.print(now.day(), DEC);

  lcd.setCursor(8, 0);
  if ((now.hour()) <= 9) {
    lcd.print("0");
  }
  lcd.print(now.hour(), DEC);
  lcd.print(":");
  if (now.minute() <= 9) {
    lcd.print("0");
  }
  lcd.print(now.minute(), DEC);

  for (int i = 0; i <= 7; i++) {
    lcd.setCursor(10, 0);
    lcd.print(":");
    delay(500);
    lcd.setCursor(10, 0);
    lcd.print(" ");
    delay(500);
  }
  delay(1000);


  /* TEMPERATURA*/

  crearIconos(tempchar1, tempchar2, tempchar3, tempchar4);

  //logo de conexxion a red
  if (estadoWiFi) {
    lcd.createChar(5, wifiFull);
    lcd.setCursor(3, 1);
    lcd.write(5);
  }
  //temperatura

  lcd.setCursor(3, 0);
  lcd.print("Temperatura A");
  lcd.setCursor(8, 1);
  lcd.print(temperatura);
  lcd.print((char)223);
  lcd.print("C");
  delay(3000);
  lcd.clear();
  delay(1000);


  /* HUMEDAD */

  crearIconos(humchar1, humchar2, humchar3, humchar4);
  //logo de conexxion a red
  if (estadoWiFi) {
    lcd.createChar(5, wifiFull);
    lcd.setCursor(3, 1);
    lcd.write(5);
  }
  //humedad
  lcd.setCursor(3, 0);
  lcd.print("Humedad Relat");
  lcd.setCursor(8, 1);
  lcd.print(humedad);
  lcd.print("%");
  delay(3000);
  lcd.clear();
  delay(1000);

  cont = cont + 0.34;

  /*REGISTRO*/
  if (validarSD()) {
    if (report) {
      digitalWrite(ledDatos, HIGH);
      if (cont >= 15) {
        crearRegistro();
      } else {
        limpirarYZero();
        lcd.print("Proximo Registro");
        lcd.setCursor(0, 1);
        lcd.print("   ");
        lcd.print(15 - (int)cont);
        lcd.print(" ");
        lcd.print("minutos  ");
        digitalWrite(ledDatos, LOW);
        delay(100);
        digitalWrite(ledDatos, HIGH);
        delay(2800);
        lcd.clear();
        delay(1000);
        report.close();
      }
    } else {
      limpirarYZero();
      lcd.print("Error en");
      lcd.setCursor(0, 1);
      lcd.print("el registro");
      intermitenteLento();
      lcd.clear();
      delay(1000);
    }
  } else {

  }


}
void crearRegistro() {
  DateTime now = RTC.now();
  temperatura = dht.readTemperature();
  humedad = dht.readHumidity();
  report = SD.open("report.csv", FILE_WRITE);
  report.print(now.year(), DEC);
  report.print("/");
  if ((now.month()) <= 9) {
    report.print("0");
  }
  report.print(now.month(), DEC);
  report.print("/");
  if ((now.day()) <= 9) {
    report.print("0");
  }
  report.print(now.day(), DEC);
  report.print(", ");

  if (now.hour() <= 9) {
    report.print("0");
  }
  report.print(now.hour(), DEC);
  report.print(":");
  if (now.minute() <= 9) {
    report.print("0");
  }
  report.print(now.minute(), DEC);
  report.print(":");
  if (now.second() <= 9) {
    report.print("0");
  }
  report.print(now.second(), DEC);
  report.print(", ");
  report.print(temperatura);
  report.print(", ");
  report.print(humedad);
  report.println("");
  limpirarYZero();
  lcd.print("    Creando   ");
  lcd.setCursor(0, 1);
  lcd.print("    Registro    ");
  intermitenteRapido();
  lcd.clear();
  delay(1000);
  cont = 0;
  report.close();

}
void crearIconos(byte *x1, byte *x2, byte *x3, byte *x4) {
  limpirarYZero();
  lcd.createChar(1, x1);
  lcd.createChar(2, x2);
  lcd.createChar(3, x3);
  lcd.createChar(4, x4);
  lcd.setCursor(0, 0);
  lcd.write(1);
  lcd.setCursor(0, 1);
  lcd.write(2);
  lcd.setCursor(1, 0);
  lcd.write(3);
  lcd.setCursor(1, 1);
  lcd.write(4);
}
void intermitenteRapido() {
  for (int i = 0; i <= 10; i++) {
    digitalWrite(ledDatos, HIGH);
    delay(100);
    digitalWrite(ledDatos, LOW);
    delay(100);
    digitalWrite(ledDatos, HIGH);
  }


}
void intermitenteLento() {
  for (int i = 0; i <= 4; i++) {
    digitalWrite(ledDatos, HIGH);
    delay(500);
    digitalWrite(ledDatos, LOW);
    delay(500);
  }
}
void limpirarYZero() {
  lcd.clear();
  lcd.setCursor(0, 0);
}

boolean validarSD() {
  if (!SD.begin(SS_pin)) {
    limpirarYZero();
    lcd.print("  Sin Memoria   ");
    lcd.setCursor(0, 1);
    lcd.print("    MicroSD!    ");
    intermitenteLento();
    lcd.clear();
    delay(1000);
    report.close();
    return false;
  } else {
    return true;
  }
}
