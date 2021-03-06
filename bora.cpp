#include "Arduino.h"
#include "bora.h"

BORA::BORA(Client& wifi) {
    PubSubClient broker(wifi);
    this->broker = broker;

    this->broker_server = 'baboon.rmq.cloudamqp.com';
    this->broker_port = 1883;
    this->broker_user = 'vzriadoh:vzriadoh';
    this->broker_pass = 'Y98z8eUaIYiRv7VO_L3rgBPYMjGFAEyZ';
}

void BORA::begin(const char* secret_key) {
    this->secret_key = secret_key;

    this->broker.setServer("baboon.rmq.cloudamqp.com", 1883);
    this->broker.setCallback([this](char *topic, byte *payload, unsigned int length) {
        this->handleBrokerMessages(topic, payload, length);
    });

    this->connectBroker();
    this->initBroker();
}

void BORA::handleBrokerMessages(char* topic, byte* payload, unsigned int length) {
    String value;
    for(int i = 0; i < length; i++)  {
       char c = (char)payload[i];
       value += c;
    }

    this->values[(String)topic] = value;
}

void BORA::loop() {
    this->connectBroker();

    unsigned long time_now = millis();
    while(millis() < time_now + this->period) {
        this->broker.loop();
    }
}

void BORA::setPeriod(int period) {
    this->period = period;
}

String BORA::generatePostUrl(String variable, String value) {
    String secret_key = (String)this->secret_key;
    String generatedPostUrl = "/device/secret/" + secret_key + "/data/" + variable + "?value=" + value;

    return generatedPostUrl;
}

String BORA::generateTopic(String topic) {
    String secret_key = (String)this->secret_key;
    String separator = "/";
    String generatedTopic = secret_key + separator + topic;

    return generatedTopic.c_str();
}

void BORA::sendData(String variable, String value) {
    String last_data = this->values[this->generateTopic(variable)].as<String>();
    bool is_new_data = last_data != value;

    if (is_new_data) {
        restclient rest("server.bora-iot.com", 80);
        String generatedUrl = this->generatePostUrl(variable, value);
        rest.post(generatedUrl.c_str(), "0");
    }
}

void BORA::setServer(String server, int port, String user, String pass) {
    this->broker_server = server;
    this->broker_port = port;
    this->broker_user = user;
    this->broker_pass = pass;
}

bool BORA::isConnected() {
    return this->broker.connected();
}

int BORA::clientState() {
    return this->broker.state();
}

void BORA::connectBroker() {
    while (!this->broker.connected()) {
        if (this->broker.connect("BORA", "vzriadoh:vzriadoh", "Y98z8eUaIYiRv7VO_L3rgBPYMjGFAEyZ")) {
            String generatedTopic = this->generateTopic("#");
            this->broker.subscribe(generatedTopic.c_str());
        } else {
            delay(500);
        }
    }
}

void BORA::initBroker() {
    restclient rest("server.bora-iot.com", 80);
    String secret_key = (String)this->secret_key;
    String pathUrl = "/device/init_broker/" + secret_key;

    rest.post(pathUrl.c_str(), "0");
}

int BORA::digitalRead(int pin, String variable) {
    int value = Arduino_h::digitalRead(pin);
    this->sendData(variable, (String)value);

    return value;
}

int BORA::analogRead(int pin, String variable) {
    int value = Arduino_h::analogRead(pin);
    this->sendData(variable, (String)value);

    return value;
}

void BORA::analogWrite(char pin, int value, String variable) {
    this->sendData(variable, (String)value);
    Arduino_h::digitalWrite(pin, value);
}

void BORA::digitalWrite(char pin, int value, String variable) {
    this->sendData(variable, (String)value);
    Arduino_h::digitalWrite(pin, value);
}

String BORA::virtualWrite(String variable, String value) {
    this->sendData(variable, value);
    return value;
}

String BORA::virtualRead(char* variable) {
    return this->values[this->generateTopic(variable)].as<String>();
}