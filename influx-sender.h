#ifndef INFLUX_SENDER__H_
#define INFLUX_SENDER__H_

#include <iostream>

#include "InfluxDB/InfluxDBFactory.h"

#define INFLUX_URI "http://iD6vR73KTYL0fIT5cnlu9HuNdOFyde4LLMwcElsDEZ4G-xLiUmSrz8rFAw2tJoazN-WI4BrrhXip11ivcEnZoQ==@localhost:8086?db=data"

extern std::unique_ptr<influxdb::InfluxDB> influxdb_client;

void send_to_influx(std::string point, std::string param, double value);



#endif // INFLUX_SENDER__H_
