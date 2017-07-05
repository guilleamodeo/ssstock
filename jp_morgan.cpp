/*
    jp_morgan.cpp

    Solution to JP Morgan's Super Simple Stocks assignment.

    Written by Guillermo Amodeo


        To compile using GCC to 'ssstock' :

            g++ -Wall -o ssstock jp_morgan.cpp -lm

        To compile using Microsoft C to to 'ssstock.exe':

            cl /Fessstock /TP /EHsc jp_morgan.cpp

        Tested on Mac OS X, Windows and Linux.

    NOTES:

        I simplified error checking for the sake of readability.
*/

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <math.h>
#include <time.h>
#include <sstream>
#include <iterator>

#define FIFTEEN_MINS    15 * 60

// Stock types

enum
{
    COMMON_STOCK = 0,   // Common stock
    PREF_STOCK,         // Preferred stock
};

// Trading operations

enum
{
    BUY_STOCK = 0,
    SELL_STOCK,
};


// A trade operation record (all members public to ease handling)

class trade_op
{
    public:
        time_t      stamp;
        std::string symbol;
        int         operation;
        int         quantity;
        double      price;

        trade_op(std::string &sy,int op,int qty,double pr)
        {
            stamp = time(NULL);
            symbol = sy;
            operation = (op == BUY_STOCK) ? BUY_STOCK : SELL_STOCK;
            quantity = qty;
            price = pr;
        }
};

// A rudimentary trading database

std::vector<trade_op> trade_db;

// An entry to the GBCE index

class stock
{
    private:

        std::string symbol;
        int         type;
        double      last_dividend;
        double      fixed_dividend;
        double      par_value;
        double      price;

    public:

        stock(const std::string &sy,int ty,double ld,double fd,double pv)
        {
            symbol = sy;
            type = (ty==PREF_STOCK) ? ty : COMMON_STOCK;
            last_dividend = ld;
            fixed_dividend = fd;

            // Initial price same than par value.

            par_value = price = pv;
        }

        std::string get_symbol() const
        {
            return symbol;
        }

        double get_price() const
        {
            return price;
        }

        // Calculate Dividend yield

        double get_dividend_yield() const
        {
            switch(type)
            {
                case COMMON_STOCK:
                    return (last_dividend / price);
                case PREF_STOCK:
                    return (fixed_dividend / price);
                default:
                    break;
            }
            return 0.0;
        }

        // Calculate Price/Earnings Ratio

        double get_pe_ratio() const
        {
            if(last_dividend)
                return (price / last_dividend);
            return 0.0;
        }

        // Set the price of the stock based on trading of a time interval

        double set_price(time_t interval)
        {
            int trades = 0;
            double tq=0.0,q=0.0;

            std::vector<trade_op>::const_iterator op = trade_db.begin();

            while (op != trade_db.end())
            {
                if(!symbol.compare(op->symbol))
                {
                    //  This is not the way to calculate time lapses
                    //  in the real world but its OK for this test.

                    if( interval >= (time(NULL) - op->stamp) )
                    {
                        tq += (op->quantity * op->price);
                         q += op->quantity;

                        trades++;
                    }
                }
                op++;
            }

            // Only modify price if there was trading

            if(trades)
                price = (tq / q);

            return price;
        }

        // Show an stock

        void show() const
        {
            std::cout << std::setw(3) << symbol << " ";
            std::cout << std::setw(4) << (type==PREF_STOCK ? "PREF" : "COMM") << " ";
            std::cout << std::setw(8) << last_dividend << " ";
            std::cout << std::setw(3) << fixed_dividend << " ";
            std::cout << std::setw(8) << par_value << " ";
            std::cout << std::setw(8) << price << std::endl;
        }
};


// Sample Table (values in pounds instead pennies to use doubles instead integers).

class the_index
{
    private:
        std::vector<stock> list;

    public:

        the_index()
        {
            list.push_back(stock("TEA",COMMON_STOCK,0.00,0, 1.00));
            list.push_back(stock("POP",COMMON_STOCK,0.08,0, 1.00));
            list.push_back(stock("ALE",COMMON_STOCK,0.23,0, 0.60));
            list.push_back(stock("GIN",PREF_STOCK,0.08,2, 1.00));
            list.push_back(stock("JOE",COMMON_STOCK,0.13,0, 2.50));
        }

        // Check if a symbol exists in the index

        bool exist(std::string symbol)
        {
            std::vector<stock>::const_iterator st = list.begin();

            while (st != list.end())
            {
                if(!symbol.compare(st->get_symbol()))
                    return true;
                st++;
            }
            return false;
        }

        // A function to show the index list

        void show() const
        {
            std::cout << std::setprecision(2) << std::fixed;

            std::cout << "=== ==== ======== ==== ======== ========" << std::endl;
            std::cout << "Sym Type Last Div Fix  PAR Val. T. Price" << std::endl;
            std::cout << "=== ==== ======== ==== ======== ========" << std::endl;

            std::vector<stock>::const_iterator st = list.begin();

            while (st != list.end())
            {
                st->show();
                st++;
            }
        }

        // A function to trade stock

        bool trade(std::string &symbol,int op,int num,double price)
        {
            /* Check if parameters correct */

            if(symbol.empty() || price < 0 || num < 0)
                return false;

            trade_db.push_back(trade_op(symbol,op,num,price));

            return true;
        }

        // A random trading operation

        void random_trade(const char *sym)
        {
            std::string symbol(sym);

            trade_db.push_back(
                trade_op(
                    symbol,
                    (rand() & 1) ? BUY_STOCK : SELL_STOCK,
                    1 + (rand() % 109),
                    0.41 + ((double)(rand() % 299) / 100.0)
                )
            );
        }

        // Calculate the index

        double get_index(void)
        {
            double tmp = 1.0,num;

            std::vector<stock>::const_iterator st = list.begin();

            while (st != list.end())
            {
                /* ignore 0 values from input but take them into account
                   in the n-root calculation. This throws the same result
                   than changing zeros to ones before each multiplication,
                   but its a lot faster. ;-)
                */
                num = st->get_price();
                if(num)
                    tmp*=num;
                st++;
            }

            num = (double) list.size();

            return pow(tmp, 1.00 / num );
        }


        void dividend_yield()
        {
            std::cout << std::setprecision(2) << std::fixed;

            std::vector<stock>::const_iterator st = list.begin();

            while (st != list.end())
            {
                std::cout << "Dividend Yield of " << st->get_symbol();
                std::cout << " is " << st->get_dividend_yield() << std::endl;
                st++;
            }
        }

        void pe_ratio()
        {
            std::cout << std::setprecision(2) << std::fixed;

            std::vector<stock>::const_iterator st = list.begin();

            while (st != list.end())
            {
                std::cout << "Price/Earnings Ratio of " << st->get_symbol();
                std::cout << " is " << st->get_pe_ratio() << std::endl;
                st++;
            }
        }

        void price()
        {
            std::cout << std::setprecision(2) << std::fixed;

            std::vector<stock>::iterator st = list.begin();

            while (st != list.end())
            {
                st->set_price(FIFTEEN_MINS);

                std::cout << "Price of " << st->get_symbol();
                std::cout << " is " << st->get_price() << std::endl;
                st++;
            }
        }

        // Show all trading operations

        void list_trade()
        {
            struct tm *td;

            std::vector<trade_op>::const_iterator op = trade_db.begin();

            std::cout << std::setprecision(2) << std::fixed;

            while (op != trade_db.end())
            {
                td = localtime(&(op->stamp));

                std::cout << "[" << std::setw(4) << (td->tm_year % 256) + 1900;
                std::cout << "-" << std::setw(2) << td->tm_mon+1;
                std::cout << "-" << std::setw(2) << td->tm_mday;
                std::cout << " " << std::setw(2) << td->tm_hour;
                std::cout << ":" << std::setw(2) << td->tm_min;
                std::cout << ":" << std::setw(2) << td->tm_sec;
                std::cout << "] " << (op->operation == BUY_STOCK ? "BOUGHT":"SOLD");
                std::cout << " " << op->quantity << " shares of " << op->symbol;
                std::cout << " at " << op->price << std::endl;

                op++;
            }

            std::cout << std::endl << trade_db.size() << " trading operations in the database" << std::endl ;
        }

};

/* The GBCE index */

the_index gbce;

/* A function to process commands to test the code */

bool process_command(std::string cmdline)
{
    std::vector<std::string> cmd;
    std::istringstream f(cmdline);
    std::string s;

    while (getline(f, s, ' '))
        cmd.push_back(s);

    if(cmd.size() > 0)
    {

        if(!cmd[0].compare("quit"))
            return false;

        if(!cmd[0].compare("help"))
        {
            std::cout << std::endl;
            std::cout << "COMMANDS:" << std::endl << std::endl;
            std::cout << "    help   - Show this help." << std::endl;
            std::cout << "    index  - Show the list of stock and the All-share index." << std::endl;
            std::cout << "    trade  - Add random trading." << std::endl;
            std::cout << "    buy    - Buy stock. eg. buy 22 ALE 3.12" << std::endl;
            std::cout << "    sell   - Sell stock. eg. sell 22 ALE 3.12" << std::endl;
            std::cout << "    list   - Show trading database." << std::endl;
            std::cout << "    price  - Recalculate price of stock based on last 15 mins trade" << std::endl;
            std::cout << "    yield  - Show the dividend yield of all stock" << std::endl;
            std::cout << "    pe     - Show the P/E Ratio of all stock" << std::endl;
            std::cout << "    quit   - end the program\n" << std::endl;
        }
        else if(!cmd[0].compare("index"))
        {
            std::cout << std::setprecision(4) << std::fixed;
            std::cout << std::endl << "GBCE Index " << gbce.get_index() << std::endl << std::endl;

            gbce.show();

            std::cout << std::endl;

        }
        else if(!cmd[0].compare("trade"))
        {
            gbce.random_trade("TEA");
            gbce.random_trade("POP");
            gbce.random_trade("ALE");
            gbce.random_trade("GIN");
            gbce.random_trade("JOE");

            std::cout << "Done. " << trade_db.size() << " trading operations in the database" << std::endl;
        }
        else if(!cmd[0].compare("buy") || !cmd[0].compare("sell"))
        {
            if(cmd.size() > 3)
            {
                if(!gbce.exist(cmd[2]))
                {
                    std::cout << "ERROR: Unknown symbol " << cmd[2] << std::endl;
                }
                else
                {
                    int buy = cmd[0].compare("sell");
                    int qty = atoi(cmd[1].c_str());
                    double price = atof(cmd[3].c_str());


                    if(gbce.trade(cmd[2],(buy) ? BUY_STOCK : SELL_STOCK,qty,price))
                        std::cout << "Done. " << trade_db.size() << " Trading operations in the database" << std::endl;
                    else
                        std::cout << "ERROR: Cannot " << cmd[0] << " shares of " << cmd[2] << " at " << cmd[1] << std::endl;
                }
            }
            else
            {
                std::cout << "ERROR: syntax is '" << cmd[0] << " <quantity> <symbol> <price>'" << std::endl;
            }
        }
        else if(!cmd[0].compare("list"))
        {
            gbce.list_trade();
        }
        else if(!cmd[0].compare("price"))
        {
            gbce.price();
        }
        else if(!cmd[0].compare("yield"))
        {
            gbce.dividend_yield();
        }
        else if(!cmd[0].compare("pe"))
        {
            gbce.pe_ratio();
        }
        else
        {
            std::cout << "ERROR: Unknown command " << cmd[0] << std::endl;
        }
    }

    return true;
}



int main(int argc,char **argv)
{
    std::string cmd;

    std::cout << std::endl << "Super Simple Stocks" << std::endl << std::endl;
    std::cout << "Use 'help' for instructions" << std::endl << std::endl;

    do {
        std::cout << "->";
        getline(std::cin,cmd);
    } while(process_command(cmd));

    return 0;
}