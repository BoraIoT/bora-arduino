#ifndef BORA_h
#define BORA_h

#include <Arduino.h>
#include <Stream.h>
#include <Client.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <restclient.h>

class BORA {
    private:
        PubSubClient broker;
        const char* secret_key;

        String rest_server;
        String broker_server;
        int broker_port;
        String broker_user;
        String broker_pass;

    public:
        StaticJsonDocument<1000> values;
        unsigned long time_now = 0;
        bool has_new_data = false;
        int period = 2000;

        String generatePostUrl(String variable, String value);
        const char* generateTopic(String topic);
        void sendData(String variable, String value);

        BORA(Client& wifi);
        void begin(const char* secret_key);

        void initBroker();
        void setServer(String server, int port, String user, String pass);
        void handleBrokerMessages(char* topic, byte* payload, unsigned int length);
        void connectBroker();
        bool isConnected();
        int clientState();
        bool loop();

        int analogRead(int pin, String variable);
        int digitalRead(int pin, String variable);
        void analogWrite(char pin, int value, String variable);
        void digitalWrite(char pin, int value, String variable);
        String virtualRead(char* variable);
        String virtualWrite(String variable, String value);
};

#endif