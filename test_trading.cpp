#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include "json.hpp"
#include "Common.hpp"

using boost::asio::ip::tcp;

class TradingServerTest : public ::testing::Test {
protected:
    boost::asio::io_service io_service;
    tcp::resolver resolver;
    tcp::socket socket;

    TradingServerTest() : resolver(io_service), socket(io_service) {}

    void connectToServer() {
        tcp::resolver::query query(tcp::v4(), "127.0.0.1", std::to_string(port));
        tcp::resolver::iterator iterator = resolver.resolve(query);
        boost::asio::connect(socket, iterator);
    }

    std::string sendRequest(const nlohmann::json& request) {
        std::string message = request.dump();
        boost::asio::write(socket, boost::asio::buffer(message, message.size()));

        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, "\0");
        std::istream is(&response);
        std::string response_str(std::istreambuf_iterator<char>(is), {});
        return response_str;
    }
};


TEST_F(TradingServerTest, TestTradingScenario) {
    nlohmann::json request;
    request["ReqType"] = Requests::Free;
    request["Message"] = "bibip";
    connectToServer();
    sendRequest(request);
    
    // Регистрация пользователей
    request["ReqType"] = Requests::Registration;
    request["Message"] = "User1";
    std::string response1 = sendRequest(request);
    EXPECT_EQ(response1, "0");

    request["Message"] = "User2";
    std::string response2 = sendRequest(request);
    EXPECT_EQ(response2, "1");

    request["Message"] = "User3";
    std::string response3 = sendRequest(request);
    EXPECT_EQ(response3, "2");

    // Выставление заявок на покупку и продажу
    request["ReqType"] = Requests::Trading;
    request["UserId"] = "0";
    request["Message"] = "20:60:buy";
    connectToServer();
    std::string response4 = sendRequest(request);
    EXPECT_EQ(response4, "Your application is being processed");

    request["UserId"] = "1";
    request["Message"] = "30:61:buy";
    std::string response5 = sendRequest(request);
    EXPECT_EQ(response5, "Your application is being processed");

    request["UserId"] = "2";
    request["Message"] = "30:60:sell";
    std::string response6 = sendRequest(request);
    EXPECT_EQ(response6, "Your application is being processed");

    // Проверка статуса пользователей
    request["ReqType"] = Requests::Status;
    request["UserId"] = "0";
    std::string status1 = sendRequest(request);
    EXPECT_EQ(status1, "USD 0.000000, Money 0.000000\n");

    request["UserId"] = "1";
    std::string status2 = sendRequest(request);
    EXPECT_EQ(status2, "USD 30.000000, Money -1830.000000\n");

    request["UserId"] = "2";
    std::string status3 = sendRequest(request);
    EXPECT_EQ(status3, "USD -30.000000, Money 1830.000000\n");
}

TEST_F(TradingServerTest, RegisterUserAndPlaceOrders) {
    nlohmann::json request;
    request["ReqType"] = Requests::Free;
    request["Message"] = "bibip";
    connectToServer();
    sendRequest(request);
    // Регистрация пользователей
    request["ReqType"] = Requests::Registration;
    request["Message"] = "User1";
    std::string response1 = sendRequest(request);
    EXPECT_EQ(response1, "0");

    request["Message"] = "User2";
    std::string response2 = sendRequest(request);
    EXPECT_EQ(response2, "1");

    request["Message"] = "User3";
    std::string response3 = sendRequest(request);
    EXPECT_EQ(response3, "2");

    // Выставление заявок на покупку и продажу
    request["ReqType"] = Requests::Trading;
    request["UserId"] = "0";
    request["Message"] = "10:62:buy";
    std::string response4 = sendRequest(request);
    EXPECT_EQ(response4, "Your application is being processed");

    request["UserId"] = "1";
    request["Message"] = "20:63:buy";
    std::string response5 = sendRequest(request);
    EXPECT_EQ(response5, "Your application is being processed");

    request["UserId"] = "2";
    request["Message"] = "30:61:sell";
    std::string response6 = sendRequest(request);
    EXPECT_EQ(response6, "Your application is being processed");

    // Проверка статуса
    request["ReqType"] = Requests::Status;
    request["UserId"] = "0";
    std::string status1 = sendRequest(request);
    EXPECT_EQ(status1, "USD 10.000000, Money -620.000000\n");

    request["UserId"] = "1";
    std::string status2 = sendRequest(request);
    EXPECT_EQ(status2, "USD 20.000000, Money -1260.000000\n");

    request["UserId"] = "2";
    std::string status3 = sendRequest(request);
    EXPECT_EQ(status3, "USD -30.000000, Money 1880.000000\n");
}


TEST_F(TradingServerTest, MixedOperationsPartialMatchAndNoMatch) {
    nlohmann::json request;
    request["ReqType"] = Requests::Free;
    request["Message"] = "bibip";
    connectToServer();
    sendRequest(request);

    // Регистрация пользователей
    request["ReqType"] = Requests::Registration;
    request["Message"] = "User1";
    std::string response1 = sendRequest(request);
    EXPECT_EQ(response1, "0");

    request["Message"] = "User2";
    std::string response2 = sendRequest(request);
    EXPECT_EQ(response2, "1");
    
    request["Message"] = "User3";
    std::string response3 = sendRequest(request);
    EXPECT_EQ(response3, "2");

    request["Message"] = "User4";
    std::string response4 = sendRequest(request);
    EXPECT_EQ(response4, "3");

    request["Message"] = "User5";
    std::string response5 = sendRequest(request);
    EXPECT_EQ(response5, "4");

    request["Message"] = "User6";
    std::string response6 = sendRequest(request);
    EXPECT_EQ(response6, "5");

    request["Message"] = "User7";
    std::string response7 = sendRequest(request);
    EXPECT_EQ(response7, "6");

    // Выставление заявок на покупку и продажу
    request["ReqType"] = Requests::Trading;

    request["UserId"] = "0";
    request["Message"] = "50:100:buy";
    std::string response8 = sendRequest(request);
    EXPECT_EQ(response8, "Your application is being processed");

    request["UserId"] = "1";
    request["Message"] = "40:110:sell";
    std::string response9 = sendRequest(request);
    EXPECT_EQ(response9, "Your application is being processed");

    request["UserId"] = "2";
    request["Message"] = "30:90:sell";
    std::string response10 = sendRequest(request);
    EXPECT_EQ(response10, "Your application is being processed");

    request["UserId"] = "3";
    request["Message"] = "20:95:buy";
    std::string response11 = sendRequest(request);
    EXPECT_EQ(response11, "Your application is being processed");

    request["UserId"] = "4";
    request["Message"] = "60:100:sell";
    std::string response12 = sendRequest(request);
    EXPECT_EQ(response12, "Your application is being processed");

    request["UserId"] = "5";
    request["Message"] = "50:105:sell";
    std::string response13 = sendRequest(request);
    EXPECT_EQ(response13, "Your application is being processed");

    request["UserId"] = "6";
    request["Message"] = "10:120:buy";
    std::string response14 = sendRequest(request);
    EXPECT_EQ(response13, "Your application is being processed");
    
    // Проверка статуса
    request["ReqType"] = Requests::Status;

    request["UserId"] = "0";	
    std::string status1 = sendRequest(request);
    EXPECT_EQ(status1, "USD 50.000000, Money -5000.000000\n");

    request["UserId"] = "1";
    std::string status2 = sendRequest(request);
    EXPECT_EQ(status2, "USD 0.000000, Money 0.000000\n");

    request["UserId"] = "2";
    std::string status3 = sendRequest(request);
    EXPECT_EQ(status3, "USD -30.000000, Money 3000.000000\n");

    request["UserId"] = "3";
    std::string status4 = sendRequest(request);
    EXPECT_EQ(status4, "USD 0.000000, Money 0.000000\n");

    request["UserId"] = "4";
    std::string status5 = sendRequest(request);
    EXPECT_EQ(status5, "USD -30.000000, Money 3000.000000\n");

    request["UserId"] = "5";
    std::string status6 = sendRequest(request);
    EXPECT_EQ(status6, "USD 0.000000, Money 0.000000\n");

    request["UserId"] = "6";
    std::string status7 = sendRequest(request);
    EXPECT_EQ(status7, "USD 10.000000, Money -1000.000000\n");
}

TEST_F(TradingServerTest, SameOperationPrice) {
    nlohmann::json request;
    request["ReqType"] = Requests::Free;
    request["Message"] = "bibip";
    connectToServer();
    sendRequest(request);

    // Регистрация пользователей
    request["ReqType"] = Requests::Registration;
    request["Message"] = "User1";
    std::string response1 = sendRequest(request);
    EXPECT_EQ(response1, "0");

    request["Message"] = "User2";
    std::string response2 = sendRequest(request);
    EXPECT_EQ(response2, "1"); 

    request["Message"] = "User3";
    std::string response3 = sendRequest(request);
    EXPECT_EQ(response3, "2");

    request["Message"] = "User4";
    std::string response4 = sendRequest(request);
    EXPECT_EQ(response4, "3"); 

    request["Message"] = "User5";
    std::string response5 = sendRequest(request);
    EXPECT_EQ(response5, "4");

    request["Message"] = "User6";
    std::string response6 = sendRequest(request);
    EXPECT_EQ(response6, "5");

    request["Message"] = "User7";
    std::string response7 = sendRequest(request);
    EXPECT_EQ(response7, "6");

    // Выставление заявок на покупку и продажу
    request["ReqType"] = Requests::Trading;

    request["UserId"] = "0";
    request["Message"] = "70:100:buy";
    std::string response8 = sendRequest(request);
    EXPECT_EQ(response8, "Your application is being processed");

    request["UserId"] = "1";
    request["Message"] = "30:100:sell";
    std::string response9 = sendRequest(request);
    EXPECT_EQ(response9, "Your application is being processed");

    request["UserId"] = "2";
    request["Message"] = "40:100:sell";
    std::string response10 = sendRequest(request);
    EXPECT_EQ(response10, "Your application is being processed");

    request["UserId"] = "3";
    request["Message"] = "10:100:buy";
    std::string response11 = sendRequest(request);
    EXPECT_EQ(response11, "Your application is being processed");

    request["UserId"] = "4";
    request["Message"] = "20:100:sell";
    std::string response12 = sendRequest(request);
    EXPECT_EQ(response12, "Your application is being processed");

    request["UserId"] = "5";
    request["Message"] = "40:100:sell";
    std::string response13 = sendRequest(request);
    EXPECT_EQ(response13, "Your application is being processed");

    request["UserId"] = "6";
    request["Message"] = "40:100:buy";
    std::string response14 = sendRequest(request);
    EXPECT_EQ(response13, "Your application is being processed");
    
    // Проверка статуса
    request["ReqType"] = Requests::Status;

    request["UserId"] = "0";
    std::string status1 = sendRequest(request);
    EXPECT_EQ(status1, "USD 70.000000, Money -7000.000000\n");

    request["UserId"] = "1";
    std::string status2 = sendRequest(request);
    EXPECT_EQ(status2, "USD -30.000000, Money 3000.000000\n");

    request["UserId"] = "2";
    std::string status3 = sendRequest(request);
    EXPECT_EQ(status3, "USD -40.000000, Money 4000.000000\n");

    request["UserId"] = "3";
    std::string status4 = sendRequest(request);
    EXPECT_EQ(status4, "USD 10.000000, Money -1000.000000\n");

    request["UserId"] = "4";
    std::string status5 = sendRequest(request);
    EXPECT_EQ(status5, "USD -20.000000, Money 2000.000000\n");

    request["UserId"] = "5";
    std::string status6 = sendRequest(request);
    EXPECT_EQ(status6, "USD -30.000000, Money 3000.000000\n");

    request["UserId"] = "6";
    std::string status7 = sendRequest(request);
    EXPECT_EQ(status7, "USD 40.000000, Money -4000.000000\n");
}

TEST_F(TradingServerTest, DifferentPrices1) {
    nlohmann::json request;
    request["ReqType"] = Requests::Free;
    request["Message"] = "bibip";
    connectToServer();
    sendRequest(request);

    // Регистрация пользователей
    request["ReqType"] = Requests::Registration;
    request["Message"] = "User1";
    std::string response1 = sendRequest(request);
    EXPECT_EQ(response1, "0");

    request["Message"] = "User2";
    std::string response2 = sendRequest(request);
    EXPECT_EQ(response2, "1"); 

    request["Message"] = "User3";
    std::string response3 = sendRequest(request);
    EXPECT_EQ(response3, "2");

    request["Message"] = "User4";
    std::string response4 = sendRequest(request);
    EXPECT_EQ(response4, "3"); 

    request["Message"] = "User5";
    std::string response5 = sendRequest(request);
    EXPECT_EQ(response5, "4");

    request["Message"] = "User6";
    std::string response6 = sendRequest(request);
    EXPECT_EQ(response6, "5");

    request["Message"] = "User7";
    std::string response7 = sendRequest(request);
    EXPECT_EQ(response7, "6");

    // Выставление заявок на покупку и продажу
    request["ReqType"] = Requests::Trading;

    request["UserId"] = "0";
    request["Message"] = "25:60:buy";
    std::string response8 = sendRequest(request);
    EXPECT_EQ(response8, "Your application is being processed");

    request["UserId"] = "1";
    request["Message"] = "35:70:buy";
    std::string response9 = sendRequest(request);
    EXPECT_EQ(response9, "Your application is being processed");

    request["UserId"] = "2";
    request["Message"] = "30:60:sell";
    std::string response10 = sendRequest(request);
    EXPECT_EQ(response10, "Your application is being processed");

    request["UserId"] = "3";
    request["Message"] = "15:80:buy";
    std::string response11 = sendRequest(request);
    EXPECT_EQ(response11, "Your application is being processed");

    request["UserId"] = "4";
    request["Message"] = "20:70:sell";
    std::string response12 = sendRequest(request);
    EXPECT_EQ(response12, "Your application is being processed");

    request["UserId"] = "5";
    request["Message"] = "10:80:sell";
    std::string response13 = sendRequest(request);
    EXPECT_EQ(response13, "Your application is being processed");

    request["UserId"] = "6";
    request["Message"] = "30:60:sell";
    std::string response14 = sendRequest(request);
    EXPECT_EQ(response13, "Your application is being processed");
    
    // Проверка статуса
    request["ReqType"] = Requests::Status;

    request["UserId"] = "0";
    std::string status1 = sendRequest(request);
    EXPECT_EQ(status1, "USD 25.000000, Money -1500.000000\n");

    request["UserId"] = "1";
    std::string status2 = sendRequest(request);
    EXPECT_EQ(status2, "USD 35.000000, Money -2450.000000\n");

    request["UserId"] = "2";
    std::string status3 = sendRequest(request);
    EXPECT_EQ(status3, "USD -30.000000, Money 2100.000000\n");

    request["UserId"] = "3";
    std::string status4 = sendRequest(request);
    EXPECT_EQ(status4, "USD 15.000000, Money -1200.000000\n");

    request["UserId"] = "4";
    std::string status5 = sendRequest(request);
    EXPECT_EQ(status5, "USD -20.000000, Money 1550.000000\n");

    request["UserId"] = "5";
    std::string status6 = sendRequest(request);
    EXPECT_EQ(status6, "USD 0.000000, Money 0.000000\n");

    request["UserId"] = "6";
    std::string status7 = sendRequest(request);
    EXPECT_EQ(status7, "USD -25.000000, Money 1500.000000\n");
}


TEST_F(TradingServerTest, DifferentPrices2) {
    nlohmann::json request;
    request["ReqType"] = Requests::Free;
    request["Message"] = "bibip";
    connectToServer();
    sendRequest(request);

    // Регистрация пользователей
    request["ReqType"] = Requests::Registration;
    request["Message"] = "User1";
    std::string response1 = sendRequest(request);
    EXPECT_EQ(response1, "0");

    request["Message"] = "User2";
    std::string response2 = sendRequest(request);
    EXPECT_EQ(response2, "1"); 

    request["Message"] = "User3";
    std::string response3 = sendRequest(request);
    EXPECT_EQ(response3, "2");

    request["Message"] = "User4";
    std::string response4 = sendRequest(request);
    EXPECT_EQ(response4, "3"); 

    request["Message"] = "User5";
    std::string response5 = sendRequest(request);
    EXPECT_EQ(response5, "4");

    request["Message"] = "User6";
    std::string response6 = sendRequest(request);
    EXPECT_EQ(response6, "5");

    request["Message"] = "User7";
    std::string response7 = sendRequest(request);
    EXPECT_EQ(response7, "6");

    // Выставление заявок на покупку и продажу
    request["ReqType"] = Requests::Trading;

    request["UserId"] = "0";
    request["Message"] = "100:90:buy";
    std::string response8 = sendRequest(request);
    EXPECT_EQ(response8, "Your application is being processed");

    request["UserId"] = "1";
    request["Message"] = "40:80:sell";
    std::string response9 = sendRequest(request);
    EXPECT_EQ(response9, "Your application is being processed");

    request["UserId"] = "2";
    request["Message"] = "50:85:sell";
    std::string response10 = sendRequest(request);
    EXPECT_EQ(response10, "Your application is being processed");

    request["UserId"] = "3";
    request["Message"] = "30:95:buy";
    std::string response11 = sendRequest(request);
    EXPECT_EQ(response11, "Your application is being processed");

    request["UserId"] = "4";
    request["Message"] = "20:80:buy";
    std::string response12 = sendRequest(request);
    EXPECT_EQ(response12, "Your application is being processed");

    request["UserId"] = "5";
    request["Message"] = "60:100:sell";
    std::string response13 = sendRequest(request);
    EXPECT_EQ(response13, "Your application is being processed");

    request["UserId"] = "6";
    request["Message"] = "50:85:buy";
    std::string response14 = sendRequest(request);
    EXPECT_EQ(response13, "Your application is being processed");
    
    // Проверка статуса
    request["ReqType"] = Requests::Status;

    request["UserId"] = "0";
    std::string status1 = sendRequest(request);
    EXPECT_EQ(status1, "USD 90.000000, Money -8100.000000\n");

    request["UserId"] = "1";
    std::string status2 = sendRequest(request);
    EXPECT_EQ(status2, "USD -40.000000, Money 3600.000000\n");

    request["UserId"] = "2";
    std::string status3 = sendRequest(request);
    EXPECT_EQ(status3, "USD -50.000000, Money 4500.000000\n");

    request["UserId"] = "3";
    std::string status4 = sendRequest(request);
    EXPECT_EQ(status4, "USD 0.000000, Money 0.000000\n");

    request["UserId"] = "4";
    std::string status5 = sendRequest(request);
    EXPECT_EQ(status5, "USD 0.000000, Money 0.000000\n");

    request["UserId"] = "5";
    std::string status6 = sendRequest(request);
    EXPECT_EQ(status6, "USD 0.000000, Money 0.000000\n");

    request["UserId"] = "6";
    std::string status7 = sendRequest(request);
    EXPECT_EQ(status7, "USD 0.000000, Money 0.000000\n");
}

TEST_F(TradingServerTest, InvalidTradingRequests) {
    // Некорректные форматы запросов на торговлю
    nlohmann::json request;
    request["ReqType"] = Requests::Free;
    request["Message"] = "bibip";
    connectToServer();
    sendRequest(request);

    // Некорректный формат: 10:60:None
    request["ReqType"] = Requests::Trading;
    request["UserId"] = "0";
    request["Message"] = "10:60:None";
    std::string response1 = sendRequest(request);
    EXPECT_EQ(response1, "Incorrect input\n");

    // Некорректный формат: 10:100:buy:::
    request["Message"] = "10:100:buy:::";
    std::string response2 = sendRequest(request);
    EXPECT_EQ(response2, "Incorrect input\n");

    // Некорректный формат: :::
    request["Message"] = ":::";
    std::string response3 = sendRequest(request);
    EXPECT_EQ(response3, "Incorrect input\n");

    // Некорректный формат: :10:10
    request["Message"] = ":10:10";
    std::string response4 = sendRequest(request);
    EXPECT_EQ(response4, "Incorrect input\n");

    // Некорректный формат: buy:sell:buy
    request["Message"] = "buy:sell:buy";
    std::string response5 = sendRequest(request);
    EXPECT_EQ(response5, "Incorrect input\n");

    // Некорректный формат: 10::sell
    request["Message"] = "10::sell";
    std::string response6 = sendRequest(request);
    EXPECT_EQ(response6, "Incorrect input\n");
}


int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

