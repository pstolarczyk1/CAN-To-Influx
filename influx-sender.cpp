
#include "influx-sender.h"


std::unique_ptr<influxdb::InfluxDB> influxdb_client = influxdb::InfluxDBFactory::Get(INFLUX_URI);


void send_to_influx(std::string point, std::string param, double value)
{
    try
    {
        influxdb_client->write(influxdb::Point{point}
            .addField(param, value)
        );
    }
    catch (const std::exception& ex)
    {
        std::cout << "Error sending " << point << "/" << param << ": " << value << std::endl;
        std::cout << ex.what() << std::endl;
    }
}



