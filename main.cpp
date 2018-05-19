#include <iostream>
#include <sstream>
#include <string>

#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>

using std::cout;
using std::endl;
using std::make_unique;
using std::move;

int main()
{
    try {
        auto myUrl = make_unique<curlpp::options::Url>(
            "https://games.crossfit.com/competitions/api/v1/competitions/open/2018/"
            "leaderboards?page=4");
        curlpp::Easy myRequest;
        myRequest.setOpt(move(myUrl));
        std::ostringstream os;
        curlpp::options::WriteStream ws(&os);
        myRequest.setOpt(ws);
        myRequest.perform();
        cout << myRequest;
    } catch (curlpp::RuntimeError& e) {
        cout << e.what() << endl;
    } catch (curlpp::LogicError& e) {
        cout << e.what() << endl;
    }

    return 0;
}
