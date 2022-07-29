#include <ArduinoJson.h>//json 읽기 위한 라이브러리

void read_status() {
  String phpHost = "http://cloud.park-cloud.co19.kr/project/view_status.php";
  Serial.print("Connect to ");
  Serial.println(phpHost);

  http.begin(client, phpHost);
  http.setTimeout(1000);
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.printf("GET code : %d\n\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
      //sta, R, G, B;
      int targetsnum [4] = {0,};
      for (int i = 0; i < payload.length(); i++) {

        if (payload.charAt(i) == ':' && payload.charAt(i + 1) == '"') {
          int index = i + 2;
          //Serial.println("found! : " + String(index));
          count++;
          if (count == 1) {
            //status
            //Serial.println("found number 1");
            while (payload.charAt(index) != '"') {
              //Serial.print(payload.charAt(index));
              index++;
            }
            sta = payload.substring(i + 2, index);
          } else if (count == 2) {
            //Serial.println("found number 2");
            while (payload.charAt(index) != '"') {
              //Serial.print(payload.charAt(index));
              index++;
            }
            R = payload.substring(i + 2, index);
          } else if (count == 3) {
            //Serial.println("found number 3");
            while (payload.charAt(index) != '"') {
              //Serial.print(payload.charAt(index));
              index++;
            }
            G = payload.substring(i + 2, index);
          } else if (count == 4) {
            //Serial.println("found number 4");
            while (payload.charAt(index) != '"') {
              //Serial.print(payload.charAt(index));
              index++;
            }
            B = payload.substring(i + 2, index);
            count = 0;
          }
        }
      }
      Serial.print(sta + "\t");
      Serial.print(R + "\t");
      Serial.print(G + "\t");
      Serial.print(B + "\t");
      Serial.println("");
    }
  }
  else {
    Serial.printf("GET failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();

}

void insert_db() {
  //db 값 전달
  //          float humidity = DHTsensor.readHumidity();
  //          float temp = DHTsensor.readTemperature();
  //http://cloud.park-cloud.co19.kr/project/insert.php?temp=45&hum=25&pm1=150&pm2=200&pm3=20 다음과 같은 주소 형식으로 데이터를 보낼 예정
  //  int humidity = 20;
  //  int temp = 30;

  String phpHost = host + "/insert.php?temp=" + String(t) + "&hum=" + String(h) + "&pm1=" + String(pm1) + "&pm2=" + String(pm2) + "&pm3=" + String(pm3);
  Serial.print("Connect to ");
  Serial.println(phpHost);

  http.begin(client, phpHost);
  http.setTimeout(1000);
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.printf("GET code : %d\n\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    }
  }
  else {
    Serial.printf("GET failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}
