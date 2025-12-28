#include <SoftwareSerial.h>

SoftwareSerial sim800(10, 11); // RX, TX

void setup() {
  Serial.begin(9600);
  while (!Serial);
  sim800.begin(9600);

  Serial.println("--- SUPER LITE TEST (eth0.me) ---");
  delay(2000);

  // 1. Очистка (на всякий случай)
  sendCmd("AT+HTTPTERM");
  sendCmd("AT+SAPBR=0,1");

  // 2. Настройка GPRS (MTS)
  Serial.println("\n[1] Настройка сети...");
  sendCmd("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  sendCmd("AT+SAPBR=3,1,\"APN\",\"internet.mts.ru\"");
  sendCmd("AT+SAPBR=3,1,\"USER\",\"mts\"");
  sendCmd("AT+SAPBR=3,1,\"PWD\",\"mts\"");

  // 3. Включение интернета с проверкой
  Serial.println("[2] Включаем интернет (ждем 5 сек)...");
  sim800.println("AT+SAPBR=1,1");
  delay(5000);
  readBuffer(); // Читаем ответ (OK или ERROR)

  // Проверяем, дали ли нам IP
  sim800.println("AT+SAPBR=2,1");
  delay(1000);
  
  // Читаем ответ в переменную, чтобы проверить наличие IP
  String ipResponse = "";
  while(sim800.available()) {
    ipResponse += (char)sim800.read();
  }
  Serial.println(ipResponse);

  // Если в ответе нет кавычек (значит нет IP), стоп.
  if (ipResponse.indexOf("\"") == -1) {
    Serial.println("!!! ОШИБКА: Интернет не подключился. Дальше идти нет смысла.");
    Serial.println("Проверьте баланс или питание.");
    while(true); // Вечная пауза
  }

  // 4. HTTP Запрос
  Serial.println("\n[3] Запрос внешнего IP...");
  sendCmd("AT+HTTPINIT");
  sendCmd("AT+HTTPPARA=\"CID\",1");
  
  // Короткая ссылка - влетит со свистом!
  sendCmd("AT+HTTPPARA=\"URL\",\"http://eth0.me\"");
  
  Serial.println(">>> GET запрос...");
  sim800.println("AT+HTTPACTION=0");
}

void loop() {
  if (sim800.available()) {
    String resp = sim800.readString();
    Serial.println(resp);

    // Если видим заветные 200 (ОК)
    if (resp.indexOf(",200,") > 0) {
      Serial.println("\n--- УСПЕХ! ВАШ IP: ---");
      sim800.println("AT+HTTPREAD");
      delay(1000);
      while(sim800.available()) Serial.write(sim800.read());
      Serial.println("\n----------------------");
      while(true); // Тест окончен
    }
    
    if (resp.indexOf(",60") > 0) {
       Serial.println("Снова ошибка соединения...");
    }
  }
}

void sendCmd(String cmd) {
  sim800.println(cmd);
  delay(500); // Короткая пауза для простых команд
  readBuffer();
}

void readBuffer() {
  while (sim800.available()) {
    Serial.write(sim800.read());
  }
}
