String dataIn;
String dt[10];
int i;
boolean parsing=false;

//Inisialisasi WiFi & HTTPClient
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

//Inisialisasi SSID dan Password
const char* ssid     = "Yoga WiFi";
const char* password = "ag1266zb";

//Inisialisasi alamat database sever
const char* serverName = "https://pradana-yoga.000webhostapp.com/databaseSensor/post-esp-data.php";

//Inisialisasi API key
String apiKeyValue = "tPmAT5Ab3j7F9";


//Inisialisasi Telegram Bot
#include <UniversalTelegramBot.h>
#define BOTtoken "5341114487:AAFfu6vk-rxTOmCOJJPhGFpwtpUj7A6MHqQ" // diisi Token Bot (Dapat dari Telegram Botfather)

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

//Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

void setup() 
{
  Serial.begin(9600);
  client.setInsecure();
  dataIn="";

  delay(10);

//  Proses menghubungkan ke WiFi
    Serial.println();
    Serial.print("Mengubungkan ke Jaringan ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  
}

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));
}

void loop() 
{
  if(Serial.available()>0) 
  {
    char inChar = (char)Serial.read();
    dataIn += inChar;
    if (inChar == '\n') 
    {
      parsing = true;
    }
  }

  if(parsing)
  {
    parsingData();
    parsing=false;
    dataIn="";
  }
}

void parsingData()
{
  int j=0;

  //kirim data yang telah diterima sebelumnya
  Serial.print("data masuk : ");
  Serial.print(dataIn);
  Serial.print("\n");

  //inisialisasi variabel, (reset isi variabel)
  dt[j]="";
  //proses parsing data
  for(i=1;i<dataIn.length();i++)
  {
    //pengecekan tiap karakter dengan karakter (#) dan (,)
    if ((dataIn[i] == '#') || (dataIn[i] == ','))
    {
      //increment variabel j, digunakan untuk merubah index array penampung
      j++;
      dt[j]="";       //inisialisasi variabel array dt[j]
    }
    else
    {
    //proses tampung data saat pengecekan karakter selesai.
      dt[j] = dt[j] + dataIn[i];
    }
  }

  int Suhu = dt[0].toInt();
  int CO = dt[1].toInt();
  //kirim data hasil parsing
  Serial.print("Suhu : ");
  Serial.print(Suhu);
  Serial.print("\n");
  Serial.print("CO : ");
  Serial.print(CO);
  Serial.print("\n\n");
//Pengkondisian pengiriman data menurut tingkat Nilai CO


//  Posting data sensor ke database server
if(WiFi.status()== WL_CONNECTED)
   {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String httpRequestData = "api_key=" + apiKeyValue + "&suhu=" + Suhu
                          + "&co=" + CO + "";
    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);
    int httpResponseCode = http.POST(httpRequestData);
    if (httpResponseCode>0)
    {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
   }
   else
   {
    Serial.println("WiFi Disconnected");
   }
   delay(3000);


   //Kirim data ke Telegram
   if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    Serial.print(chat_id);
    
    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";
    
    //Cek Command untuk setiap aksi
    if (text == "/start") {
      String welcome = "Welcome  " + from_name + ".\n";
      welcome += "/statusSuhu : Status Suhu\n";
      welcome += "/statusCO : Status CO\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
    else if(text == "/statusSuhu"){
      String welcome = "Suhu Sekarang  " + dt[0] + "°C.\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }else if (text == "/statusCO"){
      String welcome = "CO Sekarang  " + dt[1] + "PPM.\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }

    if (CO >= 4){
    String welcome = "BERESIKO!\n";
    welcome += "CO Sekarang " + dt[1] + "PPM.\n";
    welcome += "Suhu Sekarang " + dt[0] + "°C.\n";
    bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }
}
