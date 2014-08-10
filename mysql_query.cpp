#include <mysql.h>
#include <memory>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

namespace mylib {

class database
{
public:
    virtual ~database() {}
};

class mysql_database : public database
{
    MYSQL mysql_;

public:
    mysql_database()
    {
        mysql_init(&mysql_);
    }

    ~mysql_database() override
    {
        mysql_close(&mysql_);
    }
};

class database_service
{
    boost::asio::io_service io_service_;
    std::unique_ptr<boost::asio::io_service::work> work_;
    boost::thread thread_;
    database& db_;

public:
    database_service(database& db)
    :   io_service_()
    ,   work_()
    ,   thread_()
    ,   db_(db)
    {}

    void start()
    {
        io_service_.reset();
        work_.reset(new boost::asio::io_service::work(io_service_));
        thread_ = boost::thread([this] {
            io_service_.run();
        });
    }

    void stop()
    {
        work_.reset();
        thread_.join();
    }

    template <typename Functor>
    void execute_task(Functor functor)
    {
        io_service_.post([this, functor] {
            functor(db_);
        });
    }
};

class demo_accessor
{
public:
    demo_accessor()
    {}

    void query_something(database& db) const
    {
        // Do some query
    }
};

class demo_model
{
    database_service& db_service_;
    demo_accessor accessor_;

public:
    demo_model(database_service& db_service, demo_accessor const& accessor)
    :   db_service_(db_service)
    ,   accessor_(accessor)
    {}

    void refresh()
    {
        db_service_.execute_task([this](database& db) {
            accessor_.query_something(db);
        });
    }
};

} // namespace mylib

int main()
{
    mylib::mysql_database db;
    mylib::database_service database_service(db);

    using mylib::demo_accessor;
    mylib::demo_model model(database_service, demo_accessor());

    database_service.start();
    model.refresh();
    database_service.stop();
}
