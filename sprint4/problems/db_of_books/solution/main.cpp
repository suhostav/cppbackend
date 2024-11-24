#include <iostream>
#include <fstream>
#include <optional>
#include <boost/json.hpp>
#include <pqxx/pqxx>

using namespace std::literals;

// #define DATA_FROM_FILE

using pqxx::operator"" _zv;
using std::string, std::cin, std::cout, std::endl;

class statics{
public:
    class keys{
    public:
        inline static string action{"action"};
        inline static string payload{"payload"};
    };
    class commands {
    public:
        inline static string add_book{"add_book"};
        inline static string all_books{"all_books"};
        inline static string exit{"exit"};
    };
    class fields {
    public:
        inline static string id{"id"};
        inline static string title{"title"};
        inline static string author{"author"};
        inline static string year{"year"};
        inline static string ISBN{"ISBN"};
    };
    class queries {
    public:
        inline static auto tag_add_book{"tag_add_book"_zv};
        inline static auto add_book{"INSERT INTO books (title, author, year, ISBN) VALUES ($1, $2, $3, $4);"_zv};
        inline static auto tag_add_book_no_isbn{"tag_add_book_no_isbn"_zv};
        inline static auto add_book_no_isbn{"INSERT INTO books (title, author, year) VALUES ($1, $2, $3);"_zv};
        inline static auto tag_all_books{"tag_all_books"_zv};
        inline static auto all_books{"SELECT * FROM books;"_zv};
    };
};

void prepare_table(pqxx::connection& conn){
    pqxx::work w(conn);
    w.exec(R"query(
        CREATE TABLE IF NOT EXISTS books (
            id SERIAL PRIMARY KEY, 
            title varchar(100) NOT NULL,
            author varchar(100) NOT NULL, 
            year integer NOT NULL,
            ISBN char(13) UNIQUE
    );)query"_zv);
    w.exec("DELETE FROM books;"_zv);
    w.commit();
}

void add_book(const boost::json::value& jline, pqxx::connection&conn){
    pqxx::work w(conn);
    boost::json::object payload{jline.as_object().at(statics::keys::payload).as_object()};
    string title{payload.at(statics::fields::title).as_string()};
    string author{payload.at(statics::fields::author).as_string()};
    int year{static_cast<int>(payload.at(statics::fields::year).as_int64())};
    string isbn{payload.at(statics::fields::ISBN).as_string()};
    if(isbn.size() > 0) {
        w.exec_prepared(statics::queries::tag_add_book, title, author, year, isbn);
    } else {
        w.exec_prepared(statics::queries::tag_add_book_no_isbn, title, author, year);
    }
    w.commit();
}

void get_books(pqxx::connection& conn){
        pqxx::read_transaction r(conn);
        boost::json::array jbooks;
        for(auto [id, title, author, year, isbn_opt] : r.query<int, string, string, int, std::optional<string>>(statics::queries::all_books)){
            boost::json::object jbook;
            jbook[statics::fields::id] = id;
            jbook[statics::fields::title] = title;
            jbook[statics::fields::author] = author;
            jbook[statics::fields::year] = year;
            if(isbn_opt.has_value()) {
                jbook[statics::fields::ISBN] = *isbn_opt;
            } else {
                jbook[statics::fields::ISBN] = "";
            }
            jbooks.push_back(jbook);
        }
        cout << boost::json::serialize(jbooks) << endl;
}

int main(int argc, const char* argv[]){
    try{
        if(argc == 1){
#ifdef DATA_FROM_FILE
            argv[1] = R"abc(postgres://postgres:Mys3Cr3t@localhost:30432/test_db)abc";
#endif
            std::cout << "Usage: db_example <conn-string>\n"sv;
#ifndef DATA_FROM_FILE
            return EXIT_SUCCESS;
#endif
        } else if(argc != 2){
            std::cerr << "Invalid command line\n"sv;
            return EXIT_FAILURE;
        }
        pqxx::connection conn{argv[1]};
        prepare_table(conn);
        conn.prepare(statics::queries::tag_add_book, statics::queries::add_book); 
        conn.prepare(statics::queries::tag_add_book_no_isbn, statics::queries::add_book_no_isbn); 

        std::string line;
#ifdef DATA_FROM_FILE
        std::ifstream in{"../queries.txt"};
        while(std::getline(in, line)){
#else
        while(std::getline(cin, line)){
#endif
            boost::json::value jline =  boost::json::parse(line);
            if(!jline.as_object().contains("action")){
                throw std::invalid_argument("Invalid input. No object with key <action>\n");
            }
            std::string command{jline.as_object()[statics::keys::action].as_string()};
            if(command == statics::commands::add_book){
                add_book(jline, conn);
            } else if(command == statics::commands::all_books){
                get_books(conn);
            } else if(command == statics::commands::exit){
                break;
            }
        }
    }  catch(const std::exception& e){
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch(...){
        cout << "Unknown error!\n";
    }
}