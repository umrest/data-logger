#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <boost/array.hpp>

#include <comm/CommunicationDefinitions.h>

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <iomanip>

class DataSaver {
    public:


    boost::asio::io_service    io_service;
    boost::asio::ip::tcp::socket socket;

    unsigned char socket_buffer[131]; // buffer from socket

    bool socket_connected = false;

    std::ofstream file_output;
    
    DataSaver(std::string ip, int port, std::string filename) : socket(io_service), file_output("../logs/" + filename + ".REST_DATA") {
       socket_reconnect();

    }
  

    void handle_socket_receive(const boost::system::error_code& ec, std::size_t bytes_transferred){
       
        if(!ec){
            std::cout << "Recieved data" << std::endl;
            if(socket_buffer[3] == (unsigned char)comm::CommunicationDefinitions::TYPE::ROBOT_STATE){
                // M,CAN_ID,%VBUS,CURRENT,ENCODERPOSITION,ENCODERVELOCITY
                unsigned char* data = socket_buffer + 4;
                for(int i = 0; i < 4; i++){

                    unsigned char *_CAN_ID = data;
                    short *_CURRENT = (short*)(data + 1);
                    long *_ENCODERPOSITION = (long*)(data + 3);
                    int *_ENCODERVELOCITY = (int*)(data + 11);
                    int8_t *_VBUS = (int8_t*)(data + 15);

                    int CAN_ID = *_CAN_ID;
                    int VBUS = *_VBUS;
                    float CURRENT = *_CURRENT / 100.0;
                    long ENCODERPOSITION = *_ENCODERPOSITION;
                    int ENCODERVELOCITY = *_ENCODERVELOCITY;

                
                    file_output << "M," << CAN_ID << "," << VBUS << "," << CURRENT << "," << ENCODERPOSITION << "," << ENCODERVELOCITY << std::endl;
                    file_output.flush();

                    data += 16;
                }

            }
            socket_read();
        }
        else{
            std::cout << "Socket: Disconnected" << std::endl;
            socket.close();
            socket_reconnect();
        }
    }

    void socket_read(){
        socket.async_read_some(boost::asio::buffer(socket_buffer),
            boost::bind(&DataSaver::handle_socket_receive,
                this, boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }


    void socket_reconnect(){
        boost::system::error_code ec;

        socket.connect(boost::asio::ip::tcp::endpoint( boost::asio::ip::address::from_string("127.0.0.1"), 8091 ), ec);

        if(!ec){
            socket_connected = true;
            std::cout << "Socket: Reconnect Succeeded" << std::endl;

            char identifier[128];
            identifier[0] = 250;
            identifier[1] = 5;

            boost::array<boost::asio::const_buffer, 2> d = {
            boost::asio::buffer(comm::CommunicationDefinitions::key, 3),
            boost::asio::buffer(identifier, 128) };
            
            socket.write_some(d);
            socket_read();
        }
        else{
            socket_connected = false;
            std::cout << "Socket: Reconnect Failed" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            socket_reconnect();
        }
    }

    void run(){
        io_service.run();
    }


};

int main(){
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");

    DataSaver ds("127.0.0.1", 8091, ss.str());
    ds.run();
}