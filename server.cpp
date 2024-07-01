#include <cstdlib>
#include <list>
#include <iostream>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include "json.hpp"
#include "Common.hpp"

using boost::asio::ip::tcp;

std::vector<size_t> find_all(const std::string& str, char target) {
    std::vector<size_t> positions;
    size_t pos = str.find(target);
    while (pos != std::string::npos) {
        positions.push_back(pos);
        pos = str.find(target, pos + 1);
    }
    return positions;
}

struct Balance {
    std::string userName;
    double usd;
    double money;
};

struct Record {
        int id;
        double usd;
        double price;
        std::string side;
        double position;

        Record()
        {
            id = 0;
            usd = 0;
            price = 0;
            side = "None";
            position = 0;
        }
        Record(const std::string& deal)
        {
            std::vector<size_t> delimiters = find_all(deal, ':');
            
            if (delimiters.size() != 2)
            {
            throw std::runtime_error("Invalid input format: expected at least two delimiters.");
            }
            try {
                usd = std::stof(deal.substr(0, delimiters[0]));
                price = std::stof(deal.substr(delimiters[0] + 1, delimiters[1] - delimiters[0] - 1));
            }
            catch (const std::exception& e) {
                std::cerr << e.what() << '\n';
                throw;
            }
            side = deal.substr(delimiters[1] + 1, deal.size() - delimiters[1] - 1);
            if (side != "buy" && side != "sell")
            {
            throw std::runtime_error("Unknuwn term.");
            }
        }
    };

std::list<Record> filterBuyOrders(const std::list<Record>& orders) {
    std::list<Record> buyOrders;
    std::copy_if(orders.begin(), orders.end(), std::back_inserter(buyOrders),
                 [](const Record& record) { return record.side == "buy"; });
    return buyOrders;
}

std::list<Record> filterSellOrders(const std::list<Record>& orders) {
    std::list<Record> sellOrders;
    std::copy_if(orders.begin(), orders.end(), std::back_inserter(sellOrders),
                 [](const Record& record) { return record.side == "sell"; });
    return sellOrders;
}

void sortOrdersByPriceAscending(std::list<Record>& orders) 
{
    orders.sort([](const Record& a, const Record& b) {return (a.price < b.price) || (a.price == b.price && a.position < b.position);});
}

void sortOrdersByPriceDescending(std::list<Record>& orders) 
{
    orders.sort([](const Record& a, const Record& b) {return (a.price > b.price) || (a.price == b.price && a.position < b.position);});
}



class Core
{
public:
    void Free()
    {
        mUsers.clear();
        mDeals.clear();
    }
    // "Регистрирует" нового пользователя и возвращает его ID.
    std::string RegisterNewUser(const std::string& aUserName)
    {
        size_t newUserId = mUsers.size();
        mUsers[newUserId].userName = aUserName;
        mUsers[newUserId].usd = 0;
        mUsers[newUserId].money = 0;

        return std::to_string(newUserId);
    }

    // Запрос имени клиента по ID
    std::string GetUserName(const std::string& aUserId)
    {
        const auto userIt = mUsers.find(std::stoi(aUserId));
        if (userIt == mUsers.cend())
        {
            return "Error! Unknown User";
        }
        else
        {
            return userIt->second.userName;
        }
    }
    std::string GetStatus(const std::string& aUserId)
    {
        return "USD " + std::to_string(mUsers[std::stoi(aUserId)].usd) + ", Money " + std::to_string(mUsers[std::stoi(aUserId)].money);
        // for (auto it = mDeals.begin(); it != mDeals.end(); ++it)
        //     if (it->id == std::stoi(aUserId))
        //         return std::to_string(it->usd) + "\n";
        return "beep";
    }

    // Добавление сделки
    std::string AddDeal(const std::string& aUserId, const std::string& deal) {
    try {
        Record new_deal(deal); // Создаем объект Record в блоке try

        new_deal.id = std::stoi(aUserId);
        new_deal.position = mDeals.size();
        mDeals.push_back(new_deal);
        Algorithm();
        return "Your application is being processed";
    } catch (std::exception& e) {
        return "Incorrect input\n";
    }
}

private:
    // Таблицы пользователь-баланс пользователь-сделка
    std::map<size_t, Balance> mUsers;
    std::list<Record> mDeals;

    void RemoveDealByPosition(int pos) 
    {
        for (auto it = mDeals.begin(); it != mDeals.end(); ++it) {
            if (it->position == pos) {
                mDeals.erase(it);
                return;
            }
        }
    }

    void ChangeDealByPosition(int pos, double diff) 
    {
        for (auto it = mDeals.begin(); it != mDeals.end(); ++it) {
            if (it->position == pos) {
                it->usd -= diff;
                return;
            }
        }
    }

    void Algorithm()
    {
        if (mDeals.back().side == "sell")
        {
            std::list<Record> buyOrders = filterBuyOrders(mDeals);
            sortOrdersByPriceDescending(buyOrders);

            for (auto it = buyOrders.begin(); it != buyOrders.end(); ++it){
                if (mDeals.back().price > it->price)
                    break;

                else {
                    if (it->usd < mDeals.back().usd)
                    {
                        mUsers[it->id].usd  += it->usd;
                        mUsers[mDeals.back().id].usd -= it->usd;
                        mUsers[it->id].money  -= it->usd * it->price;
                        mUsers[mDeals.back().id].money += it->usd * it->price;

                        mDeals.back().usd -= it->usd;
                        RemoveDealByPosition(it->position);
                        
                    }

                    else if (it->usd > mDeals.back().usd)
                    {
                        mUsers[it->id].usd  += mDeals.back().usd;
                        mUsers[mDeals.back().id].usd -= mDeals.back().usd;
                        mUsers[it->id].money -= mDeals.back().usd * it->price;
                        mUsers[mDeals.back().id].money += mDeals.back().usd * it->price;

                        ChangeDealByPosition(it->position, mDeals.back().usd);
                        mDeals.pop_back();
                        break;
                    }

                    else if (it -> usd == mDeals.back().usd)
                    {
                        mUsers[it->id].usd  += it->usd;
                        mUsers[mDeals.back().id].usd -= it->usd;
                        mUsers[it->id].money  -= it->usd * it->price;
                        mUsers[mDeals.back().id].money += it->usd * it->price;

                        RemoveDealByPosition(it->position);
                        mDeals.pop_back();
                        break;
                    }
                }
            }
        }
        else if (mDeals.back().side == "buy")
        {
            std::list<Record> sellOrders = filterSellOrders(mDeals);
            sortOrdersByPriceAscending(sellOrders);

            for (auto it = sellOrders.begin(); it != sellOrders.end(); ++it) {
                if (it->price > mDeals.back().price)
                    break;

                else {
                    if (it->usd < mDeals.back().usd)
                    {
                        mUsers[it->id].usd  -= it->usd;
                        mUsers[mDeals.back().id].usd += it->usd;
                        mUsers[it->id].money  += it->usd * it->price;
                        mUsers[mDeals.back().id].money -= it->usd * it->price;

                        mDeals.back().usd -= it->usd;
                        RemoveDealByPosition(it->position);
                    }

                    else if (it -> usd > mDeals.back().usd)
                    {
                        mUsers[it->id].usd  -= mDeals.back().usd;
                        mUsers[mDeals.back().id].usd += mDeals.back().usd;
                        mUsers[it->id].money += mDeals.back().usd * it->price;
                        mUsers[mDeals.back().id].money -= mDeals.back().usd * it->price;

                        ChangeDealByPosition(it->position, mDeals.back().usd);
                        mDeals.pop_back();
                        break;
                    }

                    else if (it -> usd == mDeals.back().usd)
                    {
                        mUsers[it->id].usd  -= it->usd;
                        mUsers[mDeals.back().id].usd += it->usd;
                        mUsers[it->id].money  += it->usd * it->price;
                        mUsers[mDeals.back().id].money -= it->usd * it->price;

                        RemoveDealByPosition(it->position);
                        mDeals.pop_back();
                        break;
                    }
                }
            }
        }
    }
};

Core& GetCore()
{
    static Core core;
    return core;
}

class session
{
public:
    session(boost::asio::io_service& io_service)
        : socket_(io_service)
    {
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&session::handle_read, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    // Обработка полученного сообщения.
    void handle_read(const boost::system::error_code& error,
        size_t bytes_transferred)
    {
        if (!error)
        {
            data_[bytes_transferred] = '\0';

            // Парсим json, который пришёл нам в сообщении.
            auto j = nlohmann::json::parse(data_);
            auto reqType = j["ReqType"];

            std::string reply = "Error! Unknown request type";
            if (reqType == Requests::Registration)
            {
                // Это реквест на регистрацию пользователя.
                // Добавляем нового пользователя и возвращаем его ID.
                reply = GetCore().RegisterNewUser(j["Message"]);
            }
            else if (reqType == Requests::Hello)
            {
                // Это реквест на приветствие.
                // Находим имя пользователя по ID и приветствуем его по имени.
                reply = "Hello, " + GetCore().GetUserName(j["UserId"]) + "!\n";
            }
            else if (reqType == Requests::Trading)
            {
                reply = GetCore().AddDeal(j["UserId"], j["Message"]);
            }
            else if (reqType == Requests::Status)
            {
                reply = GetCore().GetStatus(j["UserId"]) + "\n";
            }
            else if (reqType == Requests::Free)
            {
                GetCore().Free();
                reply = "bibip\n";
            }

            boost::asio::async_write(socket_,
                boost::asio::buffer(reply, reply.size()),
                boost::bind(&session::handle_write, this,
                    boost::asio::placeholders::error));
        }
        else
        {
            delete this;
        }
    }

    void handle_write(const boost::system::error_code& error)
    {
        if (!error)
        {
            socket_.async_read_some(boost::asio::buffer(data_, max_length),
                boost::bind(&session::handle_read, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            delete this;
        }
    }

private:
    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
};

class server
{
public:
    server(boost::asio::io_service& io_service)
        : io_service_(io_service),
        acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
    {
        std::cout << "Server started! Listen " << port << " port" << std::endl;

        session* new_session = new session(io_service_);
        acceptor_.async_accept(new_session->socket(),
            boost::bind(&server::handle_accept, this, new_session,
                boost::asio::placeholders::error));
    }

    void handle_accept(session* new_session,
        const boost::system::error_code& error)
    {
        if (!error)
        {
            new_session->start();
            new_session = new session(io_service_);
            acceptor_.async_accept(new_session->socket(),
                boost::bind(&server::handle_accept, this, new_session,
                    boost::asio::placeholders::error));
        }
        else
        {
            delete new_session;
        }
    }

private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
};

int main()
{
    try
    {
        boost::asio::io_service io_service;
        static Core core;

        server s(io_service);

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
